/*
 * task1.c
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

#pragma PERSISTENT(MODEL_BLOCK_TEMP_1)
int16_t MODEL_BLOCK_TEMP_1[100] = {0};

#pragma PERSISTENT(SE_FC_1)
int16_t SE_FC_1[10] = {0};

#pragma PERSISTENT(Task1_Input)
int8_t Task1_Input[20000] = {0};

volatile uint32_t ulRunTimeCounterOverflows = 0;

/* Task 1 Implementation */
//void Task1(void *pvParameters) {
//    struct Message msg;
//    struct TaskParameters *params = (struct TaskParameters *)pvParameters;
//
//    msg.params = params;
//    msg.status = statusCREATED;
//    params->releaseTime = xTaskGetTickCount();
//    params->deadline = params->releaseTime + TASK1_DEADLINE;
//    xQueueSend(xSchedulerMessages, &msg, 0);
//
//    for (;;) {
//
//        xEventGroupWaitBits(xEventGroup, TASK1_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
//
//        TickType_t startTime = xTaskGetTickCount();
//        TickType_t executionTime = pdMS_TO_TICKS(params->jobs[params->currentJob].executionTime);
//
//        _DBGUART( "1 start : %l\r\n", startTime);
//        _DBGUART( "task1 job %d\r\n", params->currentJob);
//
//        volatile TickType_t inf_time;
//        while (1) {
//            inf_time =  xTaskGetTickCount() - startTime;
//            if(inf_time >= executionTime ) break;
//        }
//
//        TickType_t endTime = xTaskGetTickCount();
//
//        if (endTime > params->deadline) {
//            _DBGUART("Task1: DEADLINE MISS at %l (Deadline was %l)\r\n", endTime, params->deadline);
//        }
//        if (params->currentJob == 2 ) {
//            params->releaseTime = params->deadline; //next release time     = msg.params->startTime  + TASK2_PERIOD;
//            params->deadline = params->deadline + TASK1_PERIOD;
//            params->currentJob = 0;
//        }
//        else{
//            params->currentJob++;
//        }
//
//        msg.status = statusCOMPLETED;
//        xQueueSend(xSchedulerMessages, &msg, 0);
//
//    }
//}

//
//
uint8_t EUSCI_A_UART_receiveData (uint16_t baseAddress);

void Task1(void *pvParameters){

    int16_t* block = MODEL_BLOCK_TEMP_1;
    int16_t * fc = SE_FC_1;
    int8_t ans;

    struct Message msg;
    struct TaskParameters *params = (struct TaskParameters *)pvParameters;

    msg.params = params;
    msg.status = statusCREATED;
    params->releaseTime = xTaskGetTickCount();
    params->deadline = params->releaseTime + TASK1_DEADLINE;
    xQueueSend(xSchedulerMessages, &msg, 0);

    while(1){

        xEventGroupWaitBits(xEventGroup, TASK1_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        int8_t * data = Task1_Input;

        if(params->currentJob == 0){
            params->releaseTime = xTaskGetTickCount();

            int receivedCount = 0;
            while (receivedCount < DATA_SIZE) {
                int8_t c = EUSCI_A_UART_receiveData(EUSCI_A0_BASE);
                //_DBGUART("%d", c);
                data[receivedCount] = c;
                receivedCount++;
            }

            //dma_load(Task1_Input, input_buffer, INPUT_LENGTH);

            inputFeatures.numRows = INPUT_NUM_ROWS;
            inputFeatures.numCols = INPUT_NUM_COLS;
            inputFeatures.data = Task1_Input;

            outputLabels.numRows = OUTPUT_NUM_LABELS;
            outputLabels.numCols = LEA_RESERVED;   // one more column is reserved for LEA
            outputLabels.data = output_buffer;

            params->deadline = params->releaseTime + TASK1_DEADLINE;
        }

        else if(params->currentJob == 1){
        //uart 작업 추가하기.
        conv(&outputLabels, &inputFeatures, conv1Params, conv1scale, 1); // matrix *output, matrix *input, ConvLayerParams convparams, Convscale convscale, task num

        SE_Block(32, 32, 0, 16, fc, block, 16, 1, 1); //row, col, block_number, channel, *fc, *block, fc_num, se_idx

        max_pooling_layer(2,2,32,32, block, 16, 193, 15, 1); //pool_numRows, pool_numCols, input_numRows, input_numCols, *block, numFil, scale, shift
        }

        else if(params->currentJob == 2){

        conv(&outputLabels, &inputFeatures, conv2Params, conv2scale, 1);

        SE_Block(16, 16, 0, 32, fc, block, 32, 2, 1);

        max_pooling_layer(2,2,16,16,block, 32, 246, 15, 1);
        }

        else if(params->currentJob == 3){
        conv(&outputLabels, &inputFeatures, conv3Params, conv3scale, 1);

        SE_Block(8, 8, 0, 64, fc, block, 64, 3, 1);

        max_pooling_layer(2,2,8,8,block, 64, 451, 15, 1);
        }

        else if(params->currentJob == 4){

        ans = dense_koo(Task1_Input,MODEL_ARRAY_TEMP);
        _DBGUART("class is the %d\r\n", ans);

        }

        params->currentJob++;
        if(params->currentJob >=5){
            params->currentJob = 0;
            TickType_t endTime = xTaskGetTickCount();

            if (endTime > params->deadline) {
                _DBGUART("Task1: DEADLINE MISS at %l (Deadline was %l)\r\n", endTime, params->deadline);
            }
        }

        msg.status = statusCOMPLETED;
        xQueueSend(xSchedulerMessages, &msg, 0);

    }

}


