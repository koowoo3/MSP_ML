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
#include "task_common.h"

/*koo custom include files*/
#include "rtc.h"

//void vApplicationMallocFailedHook( void );
//void vApplicationIdleHook( void );
//void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
//void vApplicationTickHook( void );

struct TaskParameters task1Params = {
    .jobs = {
        {.executionTime = 20},
        {.executionTime = 30},
        {.executionTime = 30}},
    .releaseTime = 0,
    .deadline = TASK1_DEADLINE,
    .currentJob = 0,
    .totalJobs = 3};

struct TaskParameters task2Params = {
    .jobs = {
        {.executionTime = 10},
        {.executionTime = 20},
        {.executionTime = 30}},
    .releaseTime = 0,
    .deadline = TASK2_DEADLINE,
    .currentJob = 0,
    .totalJobs = 3};

void task_start( void );

int main( void )
{
    boardSetup();
    AB1805 rtc;
    AB1805_init();

    AB1805_set_datetime(24, 8, 19, 1, 12, 34, 56);
    while (1) {
        AB1805_get_datetime(&rtc);

        _DBGUART("Date and Time: %d/%d/%d %d:%d:%d.....%d ms\r\n",
                 rtc._year, rtc._month, rtc._date, rtc._hour, rtc._minutes, rtc._seconds, rtc._hundredth * 10 );   //10ms 마다 값을 불러오도록 가능.

        __delay_cycles(160000); // 10ms 대기

    }

    //task_start();
    return 0;
}






void task_start( void )
{
    /* Create the queue. */
    xSchedulerMessages = xQueueCreate( 10, sizeof( struct Message ) );

    if( xSchedulerMessages != NULL )
    {
        /* Start the two tasks as described in the comments at the top of this
        file. */

        xEventGroup = xEventGroupCreate();
        xEventGroupClearBits(xEventGroup, TASK1_BIT | TASK2_BIT);

        xTaskCreate(Task1,
                    "Task1",
                    configMINIMAL_STACK_SIZE*20,
                    &task1Params,
                    mainNEW_TASK_PRIORITY,
                    &task1Params.handle);

        xTaskCreate(Task2,
                    "Task2",
                    configMINIMAL_STACK_SIZE*20,
                    &task2Params,
                    mainNEW_TASK_PRIORITY,
                    &task2Params.handle);

        xTaskCreate( SchedulerTask,
                    "edf",
                    configMINIMAL_STACK_SIZE*20,
                    NULL,
                    mainSCHEDULER_PRIORITY,
                    NULL );
        //xTaskCreate(prvOutputTask, "OutputTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

        /* Start the tasks and timer running. */
        vTaskStartScheduler();
    }

    for( ;; );
}

//void vApplicationMallocFailedHook( void )
//{
//    /* Called if a call to pvPortMalloc() fails because there is insufficient
//    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
//    internally by FreeRTOS API functions that create tasks, queues, software
//    timers, and semaphores.  The size of the FreeRTOS heap is set by the
//    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
//
//    /* Force an assert. */
//    configASSERT( ( volatile void * ) NULL );
//}
///*-----------------------------------------------------------*/
//
//void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
//{
//    ( void ) pcTaskName;
//    ( void ) pxTask;
//
//    /* Run time stack overflow checking is performed if
//    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
//    function is called if a stack overflow is detected.
//    See http://www.freertos.org/Stacks-and-stack-overflow-checking.html */
//
//    /* Force an assert. */
//    configASSERT( ( volatile void * ) NULL );
//}
///*-----------------------------------------------------------*/
//
//void vApplicationIdleHook( void )
//{
//    __bis_SR_register( LPM4_bits + GIE );
//    __no_operation();
//}
///*-----------------------------------------------------------*/
//
//void vApplicationTickHook( void )
//{
//    #if( mainCREATE_SIMPLE_BLINKY_DEMO_ONLY == 0 )
//    {
//        /* Call the periodic event group from ISR demo. */
//        vPeriodicEventGroupsProcessing();
//
//        /* Call the code that 'gives' a task notification from an ISR. */
//        xNotifyTaskFromISR();
//    }
//    #endif
//}
///*-----------------------------------------------------------*/
//
///* The MSP430X port uses this callback function to configure its tick interrupt.
//This allows the application to choose the tick interrupt source.
//configTICK_VECTOR must also be set in FreeRTOSConfig.h to the correct
//interrupt vector for the chosen tick interrupt source.  This implementation of
//vApplicationSetupTimerInterrupt() generates the tick from timer A0, so in this
//case configTICK_VECTOR is set to TIMER0_A0_VECTOR. */
//void vApplicationSetupTimerInterrupt( void )
//{
//const unsigned short usACLK_Frequency_Hz = 32768;
//
//    /* Ensure the timer is stopped. */
//    TA0CTL = 0;
//
//    /* Run the timer from the ACLK. */
//    TA0CTL = TASSEL_1;
//
//    /* Clear everything to start with. */
//    TA0CTL |= TACLR;
//
//    /* Set the compare match value according to the tick rate we want. */
//    TA0CCR0 = usACLK_Frequency_Hz / configTICK_RATE_HZ;
//
//    /* Enable the interrupts. */
//    TA0CCTL0 = CCIE;
//
//    /* Start up clean. */
//    TA0CTL |= TACLR;
//
//    /* Up mode. */
//    TA0CTL |= MC_1;
//}

//#pragma PERSISTENT(MODEL_BLOCK_TEMP)
//int16_t MODEL_BLOCK_TEMP[100] = {0};
//
//#pragma PERSISTENT(SE_FC)
//int16_t SE_FC[10] = {0};
//
//
//void main(){
//    boardSetup();
//    int16_t* block = MODEL_BLOCK_TEMP;
//    int16_t * fc = SE_FC;
//    int8_t ans;
//    while(1){
//        inputFeatures.numRows = INPUT_NUM_ROWS;
//        inputFeatures.numCols = INPUT_NUM_COLS;
//        inputFeatures.data = input_buffer;
//
//        outputLabels.numRows = OUTPUT_NUM_LABELS;
//        outputLabels.numCols = LEA_RESERVED;   // one more column is reserved for LEA
//        outputLabels.data = output_buffer;
//
//
//        conv(&outputLabels, &inputFeatures, conv1Params, conv1scale); // matrix *output, matrix *input, ConvLayerParams convparams, Convscale convscale
//
//        SE_Block(32, 32, 0, 16, fc, block, 16, 1); //row, col, block_number, channel, *fc, *block, fc_num, se_idx
//
//        max_pooling_layer(2,2,32,32, block, 16, 193, 15); //pool_numRows, pool_numCols, input_numRows, input_numCols, *block, numFil, scale, shift
//
//        conv(&outputLabels, &inputFeatures, conv2Params, conv2scale);
//
//        SE_Block(16, 16, 0, 32, fc, block, 32, 2);
//
//        max_pooling_layer(2,2,16,16,block, 32, 246, 15);
//
//        conv(&outputLabels, &inputFeatures, conv3Params, conv3scale);
//
//        SE_Block(8, 8, 0, 64, fc, block, 64, 3);
//
//        max_pooling_layer(2,2,8,8,block, 64, 451, 15);
//
//        ans = dense_koo(MODEL_ARRAY_TEMP,MODEL_ARRAY_OUTPUT);
//
//    }
//
//}


