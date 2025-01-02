/*
 * block.h
 *
 *  Created on: 2024. 10. 25.
 *      Author: kooo
 */

#include <stdint.h>
#include <stdio.h>
#include <DSPLib.h>
#include <math.h>
#include <stdbool.h>
#include "math/matrix.h"
#include "msp430.h"
#include "DSPLib.h"

#ifndef BLOCK_H_
#define BLOCK_H_


//matrix* adaptive_avg_pool(matrix* result, matrix* input, uint16_t numchannel);
void global_avg_pool(int16_t row, int16_t col, int16_t block_number, int16_t channel, int16_t *block, int se_idx, int task_num);
void SE_Block(int16_t row, int16_t col, int16_t block_number, int16_t channel, int16_t *fc, int16_t *block, int fc_num, int se_idx, int task_num);

#endif /* BLOCK_H_ */
