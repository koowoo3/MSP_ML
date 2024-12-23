/*
 * max_pooling.c
 *
 *  Created on: 2024. 11. 14.
 *      Author: kooo
 */
#include "../neural_network_parameters.h"
#include "layers.h"

void max_pooling_layer(uint16_t pool_numRows, uint16_t pool_numCols, uint16_t input_numRows, uint16_t input_numCols, int16_t *block, int16_t numFil, int16_t scale, uint16_t shift) //koo: need to add matrix *result, matrix *input
{
    pool_numRows = 2;
    pool_numCols = 2;
    //uint16_t padding = 0; //koo: maybe paddding is not used.
    uint16_t numFilters = numFil;  //koo: need to make numFilters to parameters.

    matrix output, input;  //input¿∫ temp, output¿∫ model.output?
    input.numRows = input_numRows;
    input.numCols = input_numCols;
    input.data = MODEL_ARRAY_TEMP;

    output.numRows = input.numRows / pool_numRows;
    output.numCols = input.numCols / pool_numCols;
    output.data = MODEL_ARRAY_OUTPUT;

    maxpooling_filters(&output, &input, numFilters, pool_numRows, pool_numCols, block, scale, shift);

    dma_load(MODEL_ARRAY_TEMP, output.data, output.numRows * output.numCols * numFilters);

}


