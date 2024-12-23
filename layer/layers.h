/*
 * layers.h
 *
 *  Created on: 2024. 10. 23.
 *      Author: kooo
 */

#ifndef LAYERS_H_
#define LAYERS_H_

#include <stdint.h>
#include "../math/matrix_ops.h"
#include "../math/matrix.h"
#include "../math/fixed_point_ops.h"
#include "../utils/utils.h"
#include "../neural_network_parameters.h"
#include "../parameter/conv.h"

#ifndef LAYERS_GUARD
#define LAYERS_GUARD

//typedef struct {
//    uint16_t activation;        //(2: Relu)
//    uint16_t numFilters;
//    uint16_t numChannels;
//    uint16_t filter_numRows;
//    uint16_t filter_numCols;
//    uint16_t stride_numRows;
//    uint16_t stride_numCols;
//    uint16_t filters_length;
//    uint16_t padding;
//    uint16_t convnum;
//    uint16_t input_numRows;
//    uint16_t input_numCols;
//
//
//} ConvLayerParams;
//
//typedef struct {
//    int16_t scale;
//    int16_t shift;
//    int16_t x_zero_point;
//    int16_t w_zero_point;
//    int16_t y_zero_point;
//} Convscale;


// Standard Neural Network Functions

matrix *filter_simple(matrix *result, matrix *input, matrix *filter, uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols);
matrix *maxpooling(matrix* result, matrix *input, uint16_t pool_numRows, uint16_t pool_numCols);
matrix *maxpooling_with_importance(matrix* result, matrix *input, uint16_t pool_numRows, uint16_t pool_numCols, uint16_t filter_num, int16_t *block, int16_t scale, uint16_t shift);
matrix *flatten(matrix* result, matrix *input, uint16_t num_filter);
matrix *padding_same(matrix *result, matrix *input, matrix *filter, uint16_t stride_numRows, uint16_t stride_numCols, Convscale convscale);
matrix *maxpooling_filters(matrix *result, matrix *input, uint16_t numFilters, uint16_t pool_numRows, uint16_t pool_numCols, int16_t *block, int16_t scale, uint16_t shift);
matrix *filters_sum(matrix *result, matrix *input, matrix *filter, uint16_t numChannels, int16_t b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols, uint16_t padding, uint16_t conv_numRows, uint16_t conv_numCols, Convscale convscale);
matrix *conv2d(matrix *result, matrix *input, matrix *filter, uint16_t numFilters, uint16_t numChannels, int16_t *b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols, uint16_t padding, Convscale convscale);
matrix *apply_leakyrelu(matrix *result, matrix *input, uint16_t precision);
matrix *dense(matrix *result, matrix *input, matrix *W, matrix *b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision);
matrix *conv2d_importance(matrix *result, matrix *input, matrix *filter, uint16_t numFilters, uint16_t numChannels, int16_t *b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols, uint16_t padding);

#endif




#endif /* LAYERS_H_ */
