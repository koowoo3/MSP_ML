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
void SE_Block(int16_t row, int16_t col, int16_t block_number, int16_t channel, int16_t *fc, int16_t *block, int fc_num, int se_idx, int task_num){

    global_avg_pool(row, col, block_number, channel, block, se_idx, task_num);
    se1_fc1(block, fc, fc_num, se_idx);
    se1_fc2(fc, block, fc_num, se_idx); //koo: output is the importance value
    int8_t num = 8<<se_idx;
    //masking(num, num);
    return;
}

//void masking(uint8_t num, uint8_t n){//sigmoid num, nth
//    int16_t* block = MODEL_BLOCK_TEMP;
//    int8_t* imp = importance;
//    int8_t copy[64];
//    for (int i = 0; i < num; i++) {
//        copy[i] = block[i];
//    }
//
//    // Find the nth largest value using a selection algorithm on the copy
//    int threshold;
//    for (int i = 0; i < n; i++) {
//        int max_index = i;
//        for (int j = i + 1; j < num; j++) {
//            if (copy[j] > copy[max_index]) {
//                max_index = j;
//            }
//        }
//        // Swap the max element with the i-th element
//        int temp = copy[i];
//        copy[i] = copy[max_index];
//        copy[max_index] = temp;
//
//        // After the nth iteration, the nth largest element is at copy[n-1]
//        if (i == n - 1) {
//            threshold = copy[i];
//        }
//    }
//
//    for (int i = 0; i < num; i++) {
//        if (block[i] >= threshold) {
//            imp[i] = 1;
//        }
//        else imp[i] = 0;
//    }
//
//    return;
//}




