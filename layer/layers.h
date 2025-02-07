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


// Standard Neural Network Functions

matrix *filter_simple(matrix *result, matrix *input, matrix *filter, uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols);
matrix *maxpooling(matrix* result, matrix *input, uint16_t pool_numRows, uint16_t pool_numCols);
matrix_8 *maxpooling_with_importance(matrix_8* result, matrix_8 *input, uint16_t pool_numRows, uint16_t pool_numCols, uint16_t filter_num, int16_t *block, int16_t scale, uint16_t shift);
matrix *flatten(matrix* result, matrix *input, uint16_t num_filter);
matrix_8 *padding_same(matrix_8 *result, matrix_8 *input, matrix_8 *filter, uint16_t stride_numRows, uint16_t stride_numCols, Convscale convscale);
matrix_8 *maxpooling_filters(matrix_8 *result, matrix_8 *input, uint16_t numFilters, uint16_t pool_numRows, uint16_t pool_numCols, int16_t *block, int16_t scale, uint16_t shift);
matrix_8 *filters_sum(int8_t task_num, int8_t conv_num, matrix_8 *result, matrix_8 *input, matrix_8 *filter, uint16_t numChannels, int16_t b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols, uint16_t padding, uint16_t conv_numRows, uint16_t conv_numCols, Convscale convscale);
matrix_8 *conv2d(int8_t task_num, int8_t conv_num, matrix_8 *result, matrix_8 *input, matrix_8 *filter, uint16_t numFilters, uint16_t numChannels, int16_t *b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols, uint16_t padding, Convscale convscale);
matrix *apply_leakyrelu(matrix *result, matrix *input, uint16_t precision);
matrix *dense(matrix *result, matrix *input, matrix *W, matrix *b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision);
matrix *conv2d_importance(matrix *result, matrix *input, matrix *filter, uint16_t numFilters, uint16_t numChannels, int16_t *b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols, uint16_t padding);

#endif




#endif /* LAYERS_H_ */
