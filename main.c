/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* ML include files. */
#include "main.h"
#include "conv.h"
#include "layers.h"
#include "block.h"
//#include "timemeasure.h"



/* TI includes. */
#include "driverlib.h"

//uint8_t fram_weights[ARRAY_SIZE * ARRAY_SIZE] __attribute__((section(".fram_section")));
//uint8_t fram_weights_2[ARRAY_SIZE * ARRAY_SIZE] __attribute__((section(".fram_section")));
//volatile _q15 a= _Q15(0.12345678);

#pragma PERSISTENT(MODEL_BLOCK_TEMP)
int16_t MODEL_BLOCK_TEMP[100] = {0};

#pragma PERSISTENT(SE_FC)
int16_t SE_FC[10] = {0};

volatile uint32_t ulRunTimeCounterOverflows = 0;


void main(){
    boardSetup();
    int16_t* block = MODEL_BLOCK_TEMP;
    int16_t * fc = SE_FC;

    while(1){
        inputFeatures.numRows = INPUT_NUM_ROWS;
        inputFeatures.numCols = INPUT_NUM_COLS;
        inputFeatures.data = input_buffer;

        outputLabels.numRows = OUTPUT_NUM_LABELS;
        outputLabels.numCols = LEA_RESERVED;   // one more column is reserved for LEA
        outputLabels.data = output_buffer;


        conv(&outputLabels, &inputFeatures, conv1Params, conv1scale);

        SE_Block(32, 32, 0, 16, fc, block, 16, 1);

        max_pooling_layer(2,2,32,32, block, 16, 193, 15);

        conv(&outputLabels, &inputFeatures, conv2Params, conv2scale);

        SE_Block(16, 16, 0, 32, fc, block, 32, 2);

        max_pooling_layer(2,2,16,16,block, 32, 246, 15);

        conv(&outputLabels, &inputFeatures, conv3Params, conv3scale);

        SE_Block(8, 8, 0, 64, fc, block, 64, 3);

        max_pooling_layer(2,2,8,8,block, 64, 451, 15);

        dense_koo(MODEL_ARRAY_TEMP,MODEL_ARRAY_OUTPUT);


        //conv(&outputLabels, &inputFeatures, conv2Params);


        /* this is for the convolution test*/
//        inputFeatures.numRows = 4;
//        inputFeatures.numCols = 4;
//        inputFeatures.data = input_buffer;
//
//        outputLabels.numRows = OUTPUT_NUM_LABELS;
//        outputLabels.numCols = LEA_RESERVED;   // one more column is reserved for LEA
//        outputLabels.data = output_buffer;
//        TimerMeasure_start();
//        conv(&outputLabels, &inputFeatures);
//        TimerMeasure_stop();
//        volatile unsigned long elapsedTime = TimerMeasure_getElapsedTime_ms();
    }

}


