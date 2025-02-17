/*
 * task2.c
 *
 *  Created on: 2024. 12. 30.
 *      Author: kooo
 */

#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "event_groups.h"

#include <stdlib.h>
//#include <pthread.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* ML include files. */
#include "conv.h"
#include "layers.h"
#include "block.h"

/* Local includes. */
#include "myuart.h"
#include "task_common.h"

#pragma PERSISTENT(MODEL_BLOCK_TEMP_2)
int16_t MODEL_BLOCK_TEMP_2[100] = {0};

#pragma PERSISTENT(SE_FC_2)
int16_t SE_FC_2[10] = {0};

#pragma PERSISTENT(Task2_Input)
int8_t Task2_Input[20000] = {0};

/* Task 2 Implementation */
//void Task2(void *pvParameters) {
//    struct Message msg;
//    struct TaskParameters *params = (struct TaskParameters *)pvParameters;
//    msg.params = params;
//
//    msg.status = statusCREATED;
//    params->releaseTime = xTaskGetTickCount();
//    params->deadline = params->releaseTime + TASK2_DEADLINE;
//    xQueueSend(xSchedulerMessages, &msg, 0);
//
//    for (;;) {
//
//        xEventGroupWaitBits(
//            xEventGroup,   /* 이벤트 그룹 */
//            TASK2_BIT,     /* 기다릴 비트 */
//            pdTRUE,        /* 읽은 후 비트를 클리어 */
//            pdFALSE,       /* 특정 비트 하나라도 대기 */
//            portMAX_DELAY  /* 무한 대기 */
//        );
//
//        TickType_t startTime = xTaskGetTickCount();
//        TickType_t executionTime = pdMS_TO_TICKS(params->jobs[params->currentJob].executionTime);
//
//        _DBGUART( "2 start : %l\r\n", startTime);
//        _DBGUART( "task2 job %d\r\n", params->currentJob);
//        while ((xTaskGetTickCount() - startTime) < executionTime) {
//        }
//
//        TickType_t endTime = xTaskGetTickCount();
//
//        if (endTime > params->deadline) {
//            _DBGUART("Task2: DEADLINE MISS at %l (Deadline was %l)\r\n", endTime, params->deadline);
//        }
//        if (params->currentJob == 2 ) {
//            params->releaseTime = params->deadline; //next release time     = msg.params->startTime  + TASK2_PERIOD;
//            params->deadline = params->deadline + TASK2_PERIOD;
//            params->currentJob = 0;
//        }
//        else{
//            params->currentJob++;
//        }
//
//        msg.status = statusCOMPLETED;
//        xQueueSend(xSchedulerMessages, &msg, 0);
//    }

//}

void Task2(void *pvParameters){

    int16_t* block = MODEL_BLOCK_TEMP_2;
    int16_t * fc = SE_FC_2;
    int8_t ans;

    struct Message msg;
    struct TaskParameters *params = (struct TaskParameters *)pvParameters;

    msg.params = params;
    msg.status = statusCREATED;

    if(params->state == 0){  // task is uninitilized.
        //params->state = 1; //task is now idle.
        params->releaseTime = xTaskGetTickCount();
        params->deadline = params->releaseTime + TASK2_DEADLINE;
    }
    xQueueSend(xSchedulerMessages, &msg, 0);

    while(1){

        xEventGroupWaitBits(xEventGroup, TASK2_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        params->state = 2; // now task is running
        int8_t * data = Task2_Input;

        _DBGUART("T2 Start\r\n");
        if(params->currentJob == 0){
             //next release time     = msg.params->startTime  + TASK2_PERIOD;

            int receivedCount = 0;
//            while (receivedCount < DATA_SIZE) {
//                int8_t c = EUSCI_A_UART_receiveData(EUSCI_A0_BASE);
//                data[receivedCount] = c;
//                receivedCount++;
//            }
            dma_load(Task2_Input, input_buffer, INPUT_LENGTH);

            inputFeatures.numRows = INPUT_NUM_ROWS;
            inputFeatures.numCols = INPUT_NUM_COLS;
            inputFeatures.data = Task2_Input;

            outputLabels.numRows = OUTPUT_NUM_LABELS;
            outputLabels.numCols = LEA_RESERVED;   // one more column is reserved for LEA
            outputLabels.data = output_buffer;

            params->deadline = params->releaseTime + TASK2_DEADLINE;
        }

        else if(params->currentJob == 1){
            _DBGUART("T2Job1 Start\r\n");
        conv(&outputLabels, &inputFeatures, conv1Params, conv1scale, 2, 1); // matrix *output, matrix *input, ConvLayerParams convparams, Convscale convscale, task num

        SE_Block(32, 32, 0, 16, fc, block, 16, 1, 2); //row, col, block_number, channel, *fc, *block, fc_num, se_idx

        max_pooling_layer(2,2,32,32, block, 16, 193, 15, 2); //pool_numRows, pool_numCols, input_numRows, input_numCols, *block, numFil, scale, shift
        }

        else if(params->currentJob == 2){
            _DBGUART("T2Job2 Start\r\n");
        conv(&outputLabels, &inputFeatures, conv2Params, conv2scale, 2, 2);

        SE_Block(16, 16, 0, 32, fc, block, 32, 2, 2);

        max_pooling_layer(2,2,16,16,block, 32, 246, 15, 2);
        }

        else if(params->currentJob == 3){
            _DBGUART("T2Job3 Start\r\n");
        conv(&outputLabels, &inputFeatures, conv3Params, conv3scale, 2, 3);

        SE_Block(8, 8, 0, 64, fc, block, 64, 3, 2);

        max_pooling_layer(2,2,8,8,block, 64, 451, 15, 2);
        }

        else if(params->currentJob == 4){
            _DBGUART("T2Job4 Start\r\n");
        ans = dense_koo(Task2_Input,MODEL_ARRAY_TEMP);
        _DBGUART("class is the %d\r\n", ans);
        }

        TickType_t Time = xTaskGetTickCount();
        if(params->deadline < Time){
            _DBGUART("T2 D_miss\r\n");
            params->state = 1; // if deadline is missed start again the task.
        }

        params->currentJob++;
        if(params->currentJob >=5){
            params->state = 1; // now state is idle.
        }



        msg.status = statusCOMPLETED;
        xQueueSend(xSchedulerMessages, &msg, 0);

    }

}
