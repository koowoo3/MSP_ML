/*
 * block.c
 *
 *  Created on: 2024. 10. 25.
 *      Author: kooo
 */

#include "block.h"
#include "dense.h"

/* koo: block_number is for checkpoint
 * row, col is result of convolution
 * channel is result of convolution
 * fc num is same as channel
 * */
void SE_Block(int16_t row, int16_t col, int16_t block_number, int16_t channel, int16_t *fc, int16_t *block, int fc_num, int se_idx){

    global_avg_pool(row, col, block_number, channel, block, se_idx);
    se1_fc1(block, fc, fc_num, se_idx);
    se1_fc2(fc, block, fc_num, se_idx);

    return;
}


