/*
 * conv.c
 *
 *  Created on: 2024. 10. 23.
 *      Author: kooo
 */
#include "conv.h"
#include "layers.h"
#include <stdint.h>
#include "math/fixed_point_ops.h"

//#pragma LOCATION(MODEL_ARRAY_OUTPUT, 0x10000)
//#pragma PERSISTENT(MODEL_ARRAY_OUTPUT)
//int8_t MODEL_ARRAY_OUTPUT[] = {0};

#pragma PERSISTENT(MODEL_ARRAY_TEMP)
int8_t MODEL_ARRAY_TEMP[] = {0};

void conv(matrix_8 *output, matrix_8 *input, ConvLayerParams convparams, Convscale convscale, int8_t task_num, int8_t conv_num){

    uint16_t activation, numChannels, filter_numRows, filter_numCols, stride_numRows, stride_numCols, filters_length, padding;
    uint16_t numFilters;

    int16_t *bias_array;


    output->data = MODEL_ARRAY_TEMP;

    // extract and prepare layer parameters
    activation = convparams.activation; //2; //Relu (2)
    numFilters = convparams.numFilters; //16;
    numChannels = convparams.numChannels; //3;
    filter_numRows = convparams.filter_numRows; //3;
    filter_numCols = convparams.filter_numCols; //3;
    stride_numRows = convparams.stride_numRows; //1;
    stride_numCols = convparams.stride_numCols; //1;
    filters_length = convparams.filters_length; //27;
    padding = convparams.padding;  //1;

    input->numRows = convparams.input_numRows;
    input->numCols = convparams.input_numCols;
    output->numRows = convparams.input_numRows;
    output->numCols = convparams.input_numCols;

    // prepare output
    if (padding == 1){
        output->numRows = input->numRows / stride_numRows;
        if (input->numRows % stride_numRows > 0){
            output->numRows ++;
        }
        output->numCols = input->numCols / stride_numCols;
        if (input->numCols % stride_numRows > 0){
            output->numCols ++;
        }
    }
    else {
        output->numRows = (input->numRows - filter_numRows + 1) / stride_numRows;
        if ((input->numRows - filter_numRows + 1) % stride_numRows > 0){
            output->numRows ++;
        }
        output->numCols = (input->numCols - filter_numCols + 1) / stride_numCols;
        if ((input->numCols - filter_numCols + 1) % stride_numCols > 0){
            output->numCols ++;
        }
    }
    int8_t *filters_array;
    // extract and prepare weights
    if(convparams.convnum == 1){
        filters_array = conv1_weight;
        bias_array = conv1_bias;
    }
    else if(convparams.convnum == 2){
        if(task_num == 1)
            input->data = Task1_Input;
        else if(task_num == 2)
            input->data = Task2_Input;
        filters_array = conv2_weight;
        bias_array = conv2_bias;
    }
    else if(convparams.convnum == 3){
        if(task_num == 1)
            input->data = Task1_Input;
        else if(task_num == 2)
            input->data = Task2_Input;
        filters_array = conv3_weight;
        bias_array = conv3_bias;
    }
    matrix_8 filters = {filters_array, filter_numRows, filter_numCols};




    // execute conv2d layer
    if (activation == 2){ //relu
        conv2d(task_num, conv_num, output, input, &filters, numFilters, numChannels, bias_array, &fp_relu, FIXED_POINT_PRECISION, stride_numRows, stride_numCols, padding, convscale);
    }
//    else if (activation == 1){ // sigmoid
//        conv2d(output, input, &filters, numFilters, numChannels, bias_array, &fp_sigmoid, FIXED_POINT_PRECISION, stride_numRows, stride_numCols, padding);
//    }
//    else{
//        conv2d(output, input, &filters, numFilters, numChannels, bias_array, &fp_linear, FIXED_POINT_PRECISION, stride_numRows, stride_numCols, padding);
//    }
    if(task_num == 1)
        dma_load(Task1_Input, output->data, output->numRows * output->numCols * numFilters);
    else if(task_num == 2)
        dma_load(Task2_Input, output->data, output->numRows * output->numCols * numFilters);
}



