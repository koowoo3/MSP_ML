/*
 * scheduler.c
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

/* Local includes. */
#include "myuart.h"
#include "task_common.h"

//void prvSchedulerTask( void *pvParameters );
QueueHandle_t xSchedulerMessages;
EventGroupHandle_t xEventGroup;
struct TaskParameters *taskArray[mainTASK_ARRAY_SIZE];

void SchedulerTask( void *pvParameters )
{
    struct Message msg;
    TickType_t earliestDeadline;
    //struct TaskParameters *nextTask = NULL;
    volatile int num = 0;

    for ( ;; )
    {    /* Wait until something arrives in the queue
            but, I made this always active. infinite -> 0 */
        if ( xQueueReceive( xSchedulerMessages, &msg, 0 ) )
        {
            switch ( msg.status )
            {
            case statusCOMPLETED:
                _DBGUART( "SCHEDULER: A task completed\r\n" );
                break;
            case statusCREATED:
                addTaskToArray( msg.params );
                num++;
                break;
            case statusRUNNING:
                break;
            default:
                //_DBGUART( "SCHEDULER: scrub\r\n" );
                break;
            }
        }


        /* EDF */
        if(num ==2){
            earliestDeadline = 0xFFFFFFFFUL;
            struct TaskParameters *nextTask = NULL;
            TickType_t currentTick = xTaskGetTickCount();//xTaskGetTickCountFromISR();//
            _DBGUART( "cur tick : %l\r\n", currentTick);
            for (int i = 0; i < mainTASK_ARRAY_SIZE; i++) {
                if (taskArray[i] && taskArray[i]->releaseTime <= currentTick &&
                    taskArray[i]->deadline < earliestDeadline ) {
                    earliestDeadline = taskArray[i]->deadline;
                    nextTask = taskArray[i];
                }
            }

            if (nextTask) {
                if (nextTask == &task1Params) {
                    _DBGUART("Scheduler: Triggering Task1\r\n");
                    xEventGroupSetBits(xEventGroup, TASK1_BIT);
                } else if (nextTask == &task2Params) {
                    _DBGUART("Scheduler: Triggering Task2\r\n");
                    xEventGroupSetBits(xEventGroup, TASK2_BIT);
                }
            }
        }
    }
}

void addTaskToArray( struct TaskParameters *params )
{
    for ( int i = 0; i < mainTASK_ARRAY_SIZE; i++ )
    {
        if ( taskArray[i] == NULL )
        {
            taskArray[i] = params;
            return;
        }
    }
}

void removeTaskFromArray( struct TaskParameters *params )
{
    for ( int i = 0; i < mainTASK_ARRAY_SIZE; i++ )
    {
        if ( taskArray[i] == params )
        {
            taskArray[i] = NULL;
            return;
        }
    }
}

