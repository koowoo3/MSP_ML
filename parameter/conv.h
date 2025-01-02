/*
 * conv.h
 *
 *  Created on: 2024. 10. 23.
 *      Author: kooo
 */

#ifndef CONV_H_
#define CONV_H_
#define FIXED_POINT_PRECISION 10
#include <stdint.h>
#include <../math/matrix.h>
//#include "layers.h"

#ifndef conv_w
#define conv_w

typedef struct {
    uint16_t activation;        //(2: Relu)
    uint16_t numFilters;
    uint16_t numChannels;
    uint16_t filter_numRows;
    uint16_t filter_numCols;
    uint16_t stride_numRows;
    uint16_t stride_numCols;
    uint16_t filters_length;
    uint16_t padding;
    uint16_t convnum;
    uint16_t input_numRows;
    uint16_t input_numCols;


} ConvLayerParams;

typedef struct {
    int16_t scale;
    int16_t shift;
    int16_t x_zero_point;
    int16_t w_zero_point;
    int16_t y_zero_point;
} Convscale;

void conv(matrix_8 *output, matrix_8 *input, ConvLayerParams convparams, Convscale convscale, int8_t task_num);


extern ConvLayerParams conv1Params;
extern ConvLayerParams conv2Params;
extern ConvLayerParams conv3Params;
extern Convscale conv1scale;
extern Convscale conv2scale;
extern Convscale conv3scale;
extern int8_t conv1_weight[];
extern int16_t conv1_bias[];
extern int8_t conv2_weight[];
extern int16_t conv2_bias[];
extern int8_t conv3_weight[];
extern int16_t conv3_bias[];


#endif


#endif /* CONV_H_ */
