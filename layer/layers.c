/*
 * layers.c
 *
 *  Created on: 2024. 10. 23.
 *      Author: kooo
 */

#include "layers.h"
#include "myuart.h"

#pragma PERSISTENT(PADDING_BUFFER)
static int8_t PADDING_BUFFER[PADDING_BUFFER_LENGTH] = {0};

#pragma PERSISTENT(FILTER_BUFFER)
static int16_t FILTER_BUFFER[FILTER_BUFFER_LENGTH] = {0};

extern int16_t MODEL_BLOCK_TEMP[100];

int8_t clamp_to_int8(int16_t value) {
    if (value > INT8_MAX) return INT8_MAX;
    if (value < INT8_MIN) return INT8_MIN;
    return (int8_t)value;
}

matrix *dense(matrix *result, matrix *input, matrix *W, matrix *b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision) {
    /**
     * Implementation of a dense feed-forward layer using matrix operations.
     */
    result = matrix_multiply_reduce(result, W, input, precision);

    // Only add bias if given
    if (b != NULL_PTR) {
        result = matrix_add(result, result, b);
    }

    result = apply_elementwise(result, result, activation, precision);
    return result;
}

matrix *maxpooling(matrix* result, matrix *input, uint16_t pool_numRows, uint16_t pool_numCols){
    /**
     * Implementation of maxpooling layer
     */
    uint16_t result_numRows = input->numRows / pool_numRows;
    uint16_t result_numCols = input->numCols / pool_numCols;
    uint16_t i, j, x, y, kx, ky, input_offset, result_offset;
    int16_t max;
    for (i = 0; i < result_numRows; i ++){
        for (j = 0; j < result_numCols; j ++){

            // (i, j) is the coordinate of each element after maxpooling
            // (x, y) is the coordinate of top-left element among all corresponding points in the original input matrix

            x = i * pool_numRows;
            y = j * pool_numCols;

            max = -32768;
            for (kx = 0; kx < pool_numRows; kx ++){
                for (ky = 0; ky < pool_numCols; ky ++){
                    // traverse the entire sub-block that are related to this pooling
                    input_offset = (x + kx) * input->numCols + (y + ky);
                    if (max < input->data[input_offset]){
                        max = input->data[input_offset];  // if a bigger number found, update max
                    }

                }
            }
            result_offset = i * result_numCols + j;
            result->data[result_offset] = max;

        }
    }
    return result;
}

matrix_8 *maxpooling_with_importance(matrix_8* result, matrix_8 *input, uint16_t pool_numRows, uint16_t pool_numCols, uint16_t filter_num, int16_t *block, int16_t scale, uint16_t shift){
    /**
     * Implementation of maxpooling layer
     */
    uint16_t result_numRows = input->numRows / pool_numRows;
    uint16_t result_numCols = input->numCols / pool_numCols;
    uint16_t i, j, x, y, kx, ky, input_offset, result_offset;
    volatile int16_t max;
    int16_t A_zero_point = -128;
    int16_t B_zero_point = -128;
    int16_t C_zero_point = -128;
    int16_t importance = block[filter_num-1] - B_zero_point;
//    int16_t scale = 193;  //shift: 15
//    uint16_t shift = 15;
    int32_t temp = 0;
    //koo: get the importance value from the model_block_temp

    for (i = 0; i < result_numRows; i ++){
        for (j = 0; j < result_numCols; j ++){

            // (i, j) is the coordinate of each element after maxpooling
            // (x, y) is the coordinate of top-left element among all corresponding points in the original input matrix

            x = i * pool_numRows;
            y = j * pool_numCols;

            max = -32768;
            for (kx = 0; kx < pool_numRows; kx ++){
                for (ky = 0; ky < pool_numCols; ky ++){
                    // traverse the entire sub-block that are related to this pooling
                    input_offset = (x + kx) * input->numCols + (y + ky);
                    temp = (int16_t)input->data[input_offset] - A_zero_point;
                    temp *= importance;
                    temp *= scale;
                    temp >>= shift;
                    temp+=C_zero_point;
                    if (max < temp) max = temp;
                }
            }

            if (max > 127) max = 127;
            if (max < -128) max = -128;

            result_offset = i * result_numCols + j;
            result->data[result_offset] = (int8_t)max;

        }
    }
    return result;
}


matrix_8 *maxpooling_filters(matrix_8 *result, matrix_8 *input, uint16_t numFilters, uint16_t pool_numRows, uint16_t pool_numCols, int16_t *block, int16_t scale, uint16_t shift){
    /**
     * Iteration for each filter
     * one conv2d layer usually has multiple filters, we do maxpooling one by one
     */

    uint16_t i, filter_offset, result_offset, filter_length = input->numRows * input->numCols, result_length = result->numRows * result->numCols;
    int8_t *filterData = input->data, *resultData = result->data;

    for (i = numFilters; i > 0; i --){
        filter_offset = (i - 1) * filter_length;
        result_offset = (i - 1) * result_length;

        input->data = filterData + filter_offset;
        result->data = resultData + result_offset;

        /* process one filter at a time */
        //maxpooling(result, input, pool_numRows, pool_numCols);

        /* koo: process one filter at a time and multiply the importance value*/
        maxpooling_with_importance(result, input, pool_numRows, pool_numCols, i, block, scale, shift);
    }
    return result;
}

matrix *flatten(matrix* result, matrix *input, uint16_t num_filter){
    /**
     * Implementation of flatten layer for CNN
     * the result of conv2d_maxpooling or conv2d_filter is saved in the order of filter by filter
     * however, the flatten result should be in this following way according to Tensorflow
     * f0[0], f1[0], ..., fn[0], f0[1], f1[1], ..., fn[1], ..., f0[n], f1[n], fn[n]
     */
    uint16_t i, j, input_offset, result_offset = 0;
    uint16_t filter_length;
    filter_length = input->numCols * input->numRows;
    for (i = 0; i < filter_length; i ++ ){
        for (j = 0; j < num_filter; j ++){
            input_offset = i + j * filter_length;   // get the ith element of the jth filter
            result->data[result_offset++] = input->data[input_offset];   // append it to result
            result->data[result_offset++] = 0; // for LEA, we have to append 0 to each number
        }
    }
    return result;
}

matrix_8 *padding_same(matrix_8 *result, matrix_8 *input, matrix_8 *filter, uint16_t stride_numRows, uint16_t stride_numCols, Convscale convscale) {
    uint16_t input_numRows = input->numRows, input_numCols = input->numCols, filter_numRows = filter->numRows, filter_numCols = filter->numCols;
    uint16_t pad_along_numRows, pad_along_numCols, i, input_offset, padding_offset;
    int8_t * idx = PADDING_BUFFER;
    if (input_numRows % stride_numRows) {
        pad_along_numRows = filter_numRows - input_numRows % stride_numRows;
    }
    else {
        pad_along_numRows = filter_numRows - stride_numRows;
    }
    if (input_numCols % stride_numCols) {
        pad_along_numCols = filter_numCols - input_numCols % stride_numCols;
    }
    else {
        pad_along_numCols = filter_numCols - stride_numCols;
    }

    result->numRows = input->numRows + pad_along_numRows;
    result->numCols = input->numCols + pad_along_numCols;

    //memset(PADDING_BUFFER, convscale.x_zero_point, result->numRows * result->numCols * sizeof(dtype));
    for (i = 0; i < result->numRows * result->numCols; i++) {
        idx[i] = (int8_t)convscale.x_zero_point;
    }

    for (i = 0; i < input_numRows; i ++) {
        input_offset = i * input_numCols;
        padding_offset = ((pad_along_numRows >> 1) + i) * result->numCols + (pad_along_numCols >> 1);
        dma_load(PADDING_BUFFER + padding_offset, input->data + input_offset, input_numCols);
    }

    result->data = PADDING_BUFFER;
    return result;
}

matrix *filter_simple(matrix *result, matrix *input, matrix *filter, uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols){
    /**
     * Implementation of one filter of a conv2d layer
     */
    uint16_t input_numRows = input->numRows;
    uint16_t input_numCols = input->numCols;
    uint16_t filter_numRows = filter->numRows;
    uint16_t filter_numCols = filter->numCols;
    uint16_t i, j, m, n, input_offset, filter_offset, result_offset = 0;
    int16_t mult_result, sum = 0, mult1, mult2;

    for (i = 0; i <= input_numRows - filter_numRows; i += stride_numRows){
        for (j = 0; j <= input_numCols - filter_numCols; j += stride_numCols){
            // (i,j) is the coordinate of the top-left element of the moving filter
            sum = 0;
            for (m = i; m < i + filter_numRows; m ++){
                for (n = j; n < j + filter_numCols; n ++){  // calculate element-wise matrix product between the filter and corresponding section in the input image
                    input_offset = m * input_numRows + n;
                    filter_offset = (m - i) * filter_numCols + (n - j);
                    mult_result = (int16_t)input->data[input_offset] * (int16_t)filter->data[filter_offset];
                    sum += mult_result;  // ATTENTION *** potential overflow issue ***
                }
            }
            result->data[result_offset++] = clamp_to_int8(sum);  // add bias
        }
    }
    return result;
}


matrix_8 *filters_sum(matrix_8 *result, matrix_8 *input, matrix_8 *filter, uint16_t numChannels, int16_t b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols, uint16_t padding, uint16_t conv_numRows, uint16_t conv_numCols, Convscale convscale){
    int8_t *filter_head = filter->data;
    int8_t *input_head = input->data;
    uint16_t i, result_length = result->numRows * result->numCols, input_length = input->numRows * input->numCols, filter_length = filter->numRows * filter->numCols, input_numRows = input->numRows, input_numCols = input->numCols;
    matrix temp = {FILTER_BUFFER, result->numRows, result->numCols};
    int32_t t_result[1024];
    matrix_32 temp_result = {t_result, result->numRows, result->numCols};
    //memset(result->data, 0, result_length * sizeof(dtype));
    memset(t_result, 0, result_length * sizeof(int32_t));

    for (i = numChannels; i > 0; i--){
        input->data = input_head + input_length * (i - 1);
        filter->data = filter_head + filter_length * (i - 1);
        if (padding == 1) {
            padding_same(input, input, filter, stride_numRows, stride_numCols, convscale);
        }

        temp.numRows = conv_numRows;
        temp.numCols = conv_numCols;
        filter_im2col(&temp, input, filter, precision, stride_numRows, stride_numCols, convscale);
        //filter_simple(&temp, input, filter, precision, stride_numRows, stride_numCols);
        //matrix_add(result, result, &temp);
        matrix32_add(&temp_result, &temp_result, &temp);
        input->numRows = input_numRows;
        input->numCols = input_numCols;
        input->data = input_head;
    }
    int32_t scale = convscale.scale;//83;
    int32_t y_zero_point = convscale.y_zero_point;//-128;
    for (i = result_length; i > 0; i --){
        temp_result.data[i - 1] = temp_result.data[i - 1] + b;
        volatile int64_t t = temp_result.data[i - 1];
        t *= scale;
        t>>=convscale.shift;
        t+=y_zero_point;
        if (t < -128) t = -128;
        if (t > 127) t = 127;
        result->data[i - 1] = (int8_t)t;
    }
//    for (i = result_length; i > 0; i --){
//        result->data[i - 1] = result->data[i - 1] + b;
//        int32_t temp = result->data[i - 1];
//        temp *= scale;
//        temp>>=convscale.shift;
//        temp+=y_zero_point;
//        if (temp < -128) temp = -128;
//        if (temp > 127) temp = 127;
//        result->data[i - 1] = (int8_t)temp;
//    }

    //result = apply_elementwise(result, result, activation, precision);
    return result;
}

matrix_8 *conv2d(matrix_8 *result, matrix_8 *input, matrix_8 *filter, uint16_t numFilters, uint16_t numChannels, int16_t *b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols, uint16_t padding, Convscale convscale){
    uint16_t i, result_length = result->numRows * result->numCols, filter_length = filter->numRows * filter->numCols * numChannels;
    int8_t *filter_head = filter->data, *result_head = result->data;
    uint16_t conv_numRows = (input->numRows - filter->numRows + 2 * padding) / stride_numRows + 1;  //koo: ¹Ù²Ù±â
    uint16_t conv_numCols = (input->numCols - filter->numCols + 2 * padding) / stride_numCols + 1;
    for (i = numFilters; i > 0; i --){

        filter->data = filter_head + (i - 1) * filter_length;
        result->data = result_head + (i - 1) * result_length;
        filters_sum(result, input, filter, numChannels, b[i - 1], activation, precision, stride_numRows, stride_numCols, padding, conv_numRows, conv_numCols, convscale);

    }
    return result;
}

matrix *apply_leakyrelu(matrix *result, matrix *input, uint16_t precision){
    result = apply_elementwise(result, input, &fp_leaky_relu, precision);
    return result;
}




