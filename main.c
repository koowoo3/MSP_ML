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

volatile unsigned int SlowToggle_Period = 20000-1;
volatile unsigned int FastToggle_Period = 1000-1;


//void vApplicationMallocFailedHook( void );
//void vApplicationIdleHook( void );
//void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
//void vApplicationTickHook( void );

struct TaskParameters task1Params = {
//    .jobs = {
//        {.executionTime = 20},
//        {.executionTime = 30},
//        {.executionTime = 30}},
    .releaseTime = 0,
    .deadline = TASK1_DEADLINE,
    .currentJob = 0,
    .totalJobs = 3};

struct TaskParameters task2Params = {
//    .jobs = {
//        {.executionTime = 10},
//        {.executionTime = 20},
//        {.executionTime = 30}},
    .releaseTime = 0,
    .deadline = TASK2_DEADLINE,
    .currentJob = 0,
    .totalJobs = 3};

void task_start( void );
void UartStart( uint16_t usStackSize, UBaseType_t uxPriority );

int main( void )
{
    boardSetup();
    AB1805 rtc;
    AB1805_init();

    AB1805_set_datetime(25, 1, 6, 1, 12, 34, 56);   //uint8_t year, uint8_t month, uint8_t date, uint8_t day, uint8_t hour, uint8_t minutes, uint8_t seconds)
//    while (1) {
//        AB1805_get_datetime(&rtc);
//
//        _DBGUART("Date and Time: %d/%d/%d %d:%d:%d.....%d ms\r\n",
//                 rtc._year, rtc._month, rtc._date, rtc._hour, rtc._minutes, rtc._seconds, rtc._hundredth * 10 );   //10ms 마다 값을 불러오도록 가능.
//
//        __delay_cycles(160000); // 10ms 대기
//
//    }

    task_start();
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

        //UartStart(( configMINIMAL_STACK_SIZE * 2 ),4);

        xTaskCreate(Task1,
                    "Task1",
                    configMINIMAL_STACK_SIZE*30,
                    &task1Params,
                    mainNEW_TASK_PRIORITY,
                    &task1Params.handle);

        xTaskCreate(Task2,
                    "Task2",
                    configMINIMAL_STACK_SIZE*30,
                    &task2Params,
                    mainNEW_TASK_PRIORITY,
                    &task2Params.handle);

        xTaskCreate( SchedulerTask,
                    "edf",
                    configMINIMAL_STACK_SIZE*10,
                    NULL,
                    mainSCHEDULER_PRIORITY,
                    NULL );
        //xTaskCreate(prvOutputTask, "OutputTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

        /* Start the tasks and timer running. */
        vTaskStartScheduler();
    }

    for( ;; );
}
