/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

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

#pragma PERSISTENT(fail)
uint8_t fail = 0;

#pragma PERSISTENT(check)
volatile uint8_t check = 0;

#pragma PERSISTENT(restore_tick)
TickType_t restore_tick = 0;

#pragma PERSISTENT(elapsed_ticks)
TickType_t elapsed_ticks = 0;


#pragma PERSISTENT(end_time)
AB1805 end_time = {
    ._ss_pin = 10,
    ._hundredth = 0,
    ._year = 24,
    ._month = 1,
    ._date = 20,
    ._day = 6,         // 요일 (예: 6 = 토요일)
    ._hour = 12,
    ._minutes = 30,
    ._seconds = 0,

    ._alarm_year = 24,
    ._alarm_month = 1,
    ._alarm_date = 20,
    ._alarm_day = 6,
    ._alarm_hour = 12,
    ._alarm_minutes = 35,
    ._alarm_seconds = 0
};


#pragma PERSISTENT(task1Params)
struct TaskParameters task1Params = {
    .releaseTime = 0,
    .deadline = TASK1_DEADLINE,
    .currentJob = 0,
    .totalJobs = 3,
    .state = 0};
#pragma PERSISTENT(task2Params)
struct TaskParameters task2Params = {
//    .jobs = {
//        {.executionTime = 10},
//        {.executionTime = 20},
//        {.executionTime = 30}},
    .releaseTime = 0,
    .deadline = TASK2_DEADLINE,
    .currentJob = 0,
    .totalJobs = 3,
    .state = 0};

volatile uint8_t start = 0;
void task_start( void );
void UartStart( uint16_t usStackSize, UBaseType_t uxPriority );

uint32_t GetElapsedTicks() { //elapsed tick = current time - end time.
    AB1805 current_time;
    AB1805_get_datetime(&current_time);  // 현재 RTC 시간 읽기
    AB1805 *temp = &end_time;
    uint32_t elapsed_ms = 0;

    elapsed_ms += (current_time._hour - temp->_hour) * 3600000;
    elapsed_ms += (current_time._minutes - temp->_minutes) * 60000;
    elapsed_ms += (current_time._seconds - temp->_seconds) * 1000;
    elapsed_ms += (current_time._hundredth - temp->_hundredth) * 10;

    uint32_t elapsed_ticks = elapsed_ms / portTICK_PERIOD_MS;

    TickType_t * a = &restore_tick;
    *a = xTaskGetTickCount();

    elapsed_ticks -= *a;
    //_DBGUART("Elapsed tick: %l\r\n", elapsed_ticks);
    return elapsed_ticks;
}


int main( void )
{
    boardSetup();
    AB1805_init();

    uint8_t * c = &fail;

    /*This is the very first start.
     * Save the start time.
     * If start is not the first don't set the time.
     */
    if(*c == 0){
        AB1805_set_datetime(25, 1, 6, 1, 12, 34, 56);   //uint8_t year, uint8_t month, uint8_t date, uint8_t day, uint8_t hour, uint8_t minutes, uint8_t seconds)
        AB1805_get_datetime(&end_time); //save the start time
        *c = 1;
    }
    else{
        //TickType_t *t = &elapsed_ticks;
        //*t = GetElapsedTicks();  // 1 tick = 10ms
        //AB1805 rtc;
        //AB1805_get_datetime(&rtc); //save the start time
        //_DBGUART("Date and Time: %d/%d/%d %d:%d:%d.....%d ms\r\n", rtc._year, rtc._month, rtc._date, rtc._hour, rtc._minutes, rtc._seconds, rtc._hundredth * 10 );   //10ms 마다 값을 불러오도록 가능.
        //_DBGUART("E tick: %l\r\n", *t);  //전원이 꺼졌다가 다시 켜지기까지 흐른 tick
        _DBGUART("1\r\n");
    }

//    while (1) {
//        AB1805 rtc;
//        AB1805_get_datetime(&rtc);
//        uartinit();
//        _DBGUART("2\r\n");
//        _DBGUART("Date and Time: %d/%d/%d %d:%d:%d.....%d ms\r\n",
//                 rtc._year, rtc._month, rtc._date, rtc._hour, rtc._minutes, rtc._seconds, rtc._hundredth * 10 );   //10ms 마다 값을 불러오도록 가능.
//
//        __delay_cycles(160000); // 10ms 대기
//
//    }
    //adc_setup();

    _DBGUART("start\r\n");
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
        xEventGroupClearBits(xEventGroup, TASK1_BIT | TASK2_BIT | SAVE | RESTORE);
        xTaskCreate(Task1, "Task1", configMINIMAL_STACK_SIZE*30, &task1Params, mainNEW_TASK_PRIORITY, &task1Params.handle);
        xTaskCreate(Task2, "Task2", configMINIMAL_STACK_SIZE*30, &task2Params, mainNEW_TASK_PRIORITY, &task2Params.handle);
        xTaskCreate(SchedulerTask, "edf", configMINIMAL_STACK_SIZE*10, NULL, mainSCHEDULER_PRIORITY, NULL );
        //xTaskCreate(prvOutputTask, "OutputTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
        /* Start the tasks and timer running. */
        vTaskStartScheduler();   //  tick = elapsed_tick + restore_tick
    }
    for( ;; );
}

