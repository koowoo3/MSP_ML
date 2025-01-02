/*
 * dense.c
 *
 *  Created on: 2024. 10. 25.
 *      Author: kooo
 */
#include "dense.h"
#include "math/fixed_point_ops.h"
#include <math.h>
#define VECTOR_SIZE 16
DSPLIB_DATA(lea_result, 4)
_iq31 lea_result = 0; // 결과 저장 변수 (_iq31 형식)

//int16_t *dma_load(int16_t *result, int16_t *data, uint16_t n) {
//    /*
//     * Transfer data in n bytes block to result using DMA
//     * This is the correct version for 20-bit address
//     */
//    // Configure DMA channel 0
//    __data16_write_addr((unsigned short)(__MSP430_BASEADDRESS_DMA__ + DMA_CHANNEL_0 + OFS_DMA0SA), data);
//    __data16_write_addr((unsigned short)(__MSP430_BASEADDRESS_DMA__ + DMA_CHANNEL_0 + OFS_DMA0DA), result);
//    DMA0SZ = n;                                      // Block size
//    DMA0CTL = DMADT_5 | DMASRCINCR_3 | DMADSTINCR_3; // Rpt, inc
//    DMA0CTL |= DMAEN;                                // Enable DMA0
//    DMA0CTL |= DMAREQ;
//    return result;
//}

Se_block_param SE_1_1={
   .a_zero_point = -128,
   .b_zero_point = 0,
   .y_zero_point = -128,
   .scale = 11,
   .shift = 9
};

Se_block_param SE_1_2={
   .a_zero_point = -128,
   .b_zero_point = 0,
   .y_zero_point = -45,
   .scale = 21,
   .shift = 12
};

Se_block_param SE_2_1={
   .a_zero_point = -128,
   .b_zero_point = 0,
   .y_zero_point = -128,
   .scale = 22,
   .shift = 11
};

Se_block_param SE_2_2={
   .a_zero_point = -128,
   .b_zero_point = 0,
   .y_zero_point = -21,
   .scale = 18,
   .shift = 12
};

Se_block_param SE_3_1={
   .a_zero_point = -128,
   .b_zero_point = 0,
   .y_zero_point = -21,
   .scale = 44,
   .shift = 11
};

Se_block_param SE_3_2={
   .a_zero_point = -128,
   .b_zero_point = 0,
   .y_zero_point = -79,
   .scale = 15,
   .shift = 12
};

Sigmoid_param Sig_1={
     .x_scale = 0.017243651673197746,
     .x_zero_point = -45,
     .y_scale = 0.0037302905693650246, //268.075623,
     .y_zero_point = -128
};

Sigmoid_param Sig_2={
     .x_scale = 0.033834051340818405,
     .x_zero_point = -21,
     .y_scale = 0.003895083675161004,//256.733894,
     .y_zero_point = -128
};

Sigmoid_param Sig_3={
     .x_scale = 0.18789245188236237,
     .x_zero_point = -79,
     .y_scale = 0.003921568859368563,//254.999984,
     .y_zero_point = -128
};


void se1_fc1(int16_t* input, int16_t* result, int fc_num, int se_idx){

    Se_block_param se_block;
    int8_t * weight;
    switch(se_idx){
    case 1: se_block = SE_1_1;
            weight= se1_fc1_weight;
            break;
    case 2: se_block = SE_2_1;
            weight= se2_fc1_weight;
            break;
    case 3: se_block = SE_3_1;
            weight= se3_fc1_weight;
            break;
    }

    int16_t a_zero_point = se_block.a_zero_point;
    int16_t b_zero_point = se_block.b_zero_point;
    int16_t y_zero_point = se_block.y_zero_point;
    int16_t scale = se_block.scale;
    int16_t shift = se_block.shift;


    //int weight_num = 16;
    int i,j;
    int n= 1<<(se_idx-1); //1 , 2 , 4
    int32_t sum=0;
    for(i=0; i< n; i++){
        sum=0;
        for(j=0; j<fc_num; j++){
            int16_t a = input[j]-a_zero_point;
            volatile int16_t b = (int16_t)weight[i+(n*j)]- b_zero_point;
            sum += a*b;
        }
        sum = (sum * scale) >> shift;
        sum = sum + y_zero_point;
        if (sum < -128) sum = -128;
        if (sum > 127) sum = 127;

        result[i] = (int8_t)sum;
    }
}
/*dequantize X to Q15
 * result = round(Sigmoid(X) / y_scale) + y_zero_point  // y_scale is too small. You need to make it 1/y_scale and multiply.
 */
int8_t sigmoid(int8_t input, int sig_idx){
    //    volatile int16_t temp = _Q15(x_dequantized);
    //    temp = fp_sigmoid(temp, 15);
    //    volatile double y_float = (double)temp / 32768.0f;
    //
    //    y_float = (y_float * y_scale);
    //    if (y_float >= 0) {
    //        y_float= (int32_t)(y_float + 0.5f);  // 양수인 경우 소수점 반올림
    //    } else {
    //        y_float= (int32_t)(y_float - 0.5f);  // 음수인 경우 소수점 반올림
    //    }
    //    volatile int32_t y_quantized = y_float;
    ////    volatile int32_t y_quantized = round(y_float * y_scale);
    //    y_quantized+= y_zero_point;
    Sigmoid_param sig;
    switch(sig_idx){
    case 1: sig = Sig_1; break;
    case 2: sig = Sig_2; break;
    case 3: sig = Sig_3; break;
    }
    double x_scale = sig.x_scale;
    int x_zero_point = sig.x_zero_point; // Input scale and zero point
    double y_scale = sig.y_scale;
    int y_zero_point = sig.y_zero_point; // Output scale and zero point
    volatile double x_dequantized = (input - x_zero_point) * x_scale;

    volatile double y_float = 1.0f / (1.0f + exp(-x_dequantized));
    double y_rec = 1 / y_scale;
    int32_t y_quantized = round(y_float * y_rec);
    y_quantized+= y_zero_point;
    // Clamp to int8 range
    if (y_quantized < -128) y_quantized = -128;
    if (y_quantized > 127) y_quantized = 127;

    return (int8_t)y_quantized;
}
void se1_fc2(int16_t* input, int16_t* output, int fc_num, int se_idx){

    Se_block_param se_block;
    volatile int8_t * weight = se1_fc2_weight;
    switch(se_idx){
    case 1: se_block = SE_1_2;
            weight= se1_fc2_weight;
            break;
    case 2: se_block = SE_2_2;
            weight= se2_fc2_weight;
            break;
    case 3: se_block = SE_3_2;
            weight= se3_fc2_weight;
            break;
    }

    int16_t a_zero_point = se_block.a_zero_point;
    int16_t b_zero_point = se_block.b_zero_point;
    int16_t y_zero_point = se_block.y_zero_point;
    int16_t scale = se_block.scale; //se1_fc2- shift : 12
    int16_t shift = se_block.shift;

    //int weight_num = 16;
    int i,j;
    int n = 1<<(se_idx-1);
    for(i=0; i<fc_num; i++){
        int32_t acc = 0;
        for(j=0; j<n; j++){

            volatile int32_t result = ((int16_t)input[j] - a_zero_point) * ((int16_t)weight[j*fc_num+i]-b_zero_point);
           acc+=result;
        }
            volatile int32_t y_quantized = (acc * scale) >> shift;
            y_quantized = y_quantized + y_zero_point;
            // Clamp the result to uint8 range
            if (y_quantized < -128) y_quantized = -128;
            if (y_quantized > 127) y_quantized = 127;

            output[i] = (int8_t)y_quantized; //output = MODEL_BLOCK_TEMP
            //output[i] = fp_sigmoid(output[i]);
            output[i] = (int8_t)sigmoid(output[i], se_idx);
    }

    return;
}

int8_t dense_koo(int8_t* input, int8_t* output) {

    //float a_scale = 0.03069718f;
    int a_zero_point = -128;
    //float b_scale = 0.00821415f;
    int b_zero_point = 0;
    //float y_scale = 0.17527385f;
    int y_zero_point = 5;

    volatile int32_t sum = 0;  // 더 큰 범위로 누적
    int8_t* input_idx = input;
    int8_t* weight = fc_weight;
    int16_t* bias = fc_bias;

    int8_t ans = 0;
    int8_t max = -128;
    for (int j = 0; j < 10; j++) {
        int32_t acc = 0;

        for (int p = 0; p < 1024; p++) {
            int32_t a_val = (int32_t)input_idx[p] - a_zero_point; // Dequantize A
            int32_t b_val = (int32_t)weight[p + j* 1024] - b_zero_point; // Dequantize B
            acc += a_val * b_val; // Multiply and accumulate
        }

        // Add bias if provided
        acc += bias[j];

        // Requantize the accumulated value
//        float scaled_value = acc * (a_scale * b_scale / y_scale);
//        int32_t quantized_value = (int32_t)(scaled_value + y_zero_point);

        acc = (acc * 11) >> 13;
        acc = acc + y_zero_point;
        if (acc < -128) acc = -128;
        if (acc > 127) acc = 127;

        // Clamp the result to the int8_t range
        acc = acc < INT8_MIN ? INT8_MIN : (acc > INT8_MAX ? INT8_MAX : acc);

        // Store the result
        output[j] = (int8_t)acc;

        if(output[j]>max){
            max = output[j];
            ans = j;
        }
    }
    return ans;
}



//void dense_koo(int16_t* input, int16_t* output) {
//    volatile int32_t sum = 0;  // 더 큰 범위로 누적
//    int16_t* input_idx = input;
//    int16_t* weight = fc_weight;
//
//    for (int i = 0; i < 10; i++) {  //koo: numweight 는 가중치의 총 개수 (10 * 1024)니 10
//        sum = 0;
//
//        for (int j = 0; j < 64; j++) {  //koo: numChannel은 입력 채널의 수 (16 * 64)이니 64
//
//            dma_load(MULTIPLY_BUFFER, input_idx, VECTOR_SIZE); //result of conv3
//            dma_load(MULTIPLY_BUFFER + VECTOR_SIZE, weight, VECTOR_SIZE); //weight data
//
//            msp_mac_q15_params params;
//            params.length = VECTOR_SIZE;
//            msp_status status = msp_mac_q15(&params, MULTIPLY_BUFFER, MULTIPLY_BUFFER + VECTOR_SIZE, &lea_result);
//            msp_checkStatus(status);
//
//            if (sum > MAX_Q31 - lea_result) {
//                sum = MAX_Q31;
//            } else if (sum < MIN_Q31 - lea_result) {
//                sum = MIN_Q31;
//            } else {
//                sum += lea_result;
//            }
//
//            input_idx += VECTOR_SIZE;
//            weight += VECTOR_SIZE;
//        }
//        output[i] = (_q15)(sum >> 15);
//    }
//}


