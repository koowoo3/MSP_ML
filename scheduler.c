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
extern int16_t ML_num[2][3][3];

const int16_t INITIAL_VALUES[3][3] = {
    {16,  3, 1024},  // first
    {32, 16, 256},   // sec
    {64, 32, 64}     // third
};

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
                //_DBGUART( "SCHEDULER: A task completed\r\n" );
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


        if(num == 2){   //Task parameter setting.

            int i;
            for(i=0; i<2; i++){
                int16_t (*layer)[3][3] = ML_num;
                struct TaskParameters * t = taskArray[i];

                if(t->state ==1 || t->currentJob>=5){ // if state is idle initialize the value or the task is done
                    for(int j = 0; j<3; j++){
                        for(int k = 0; k<3; k++){
                            layer[i][j][k] = INITIAL_VALUES[j][k];
                        }
                    }
                    t->currentJob = 0;
                    TickType_t Time = xTaskGetTickCount();
                    if(i == 0){
                        if(Time > t->releaseTime + TASK1_PERIOD){
                            t->releaseTime = Time;
                            t->deadline = t->releaseTime + TASK1_PERIOD;
                        }
                    }
                    else{
                        if(Time > t->releaseTime + TASK2_PERIOD){
                            t->releaseTime = Time;
                            t->deadline = t->releaseTime + TASK2_PERIOD;
                        }
                    }
                }

                if(t->state == 0){ //this is the first start.

                    for(int j = 0; j<3; j++){
                        for(int k = 0; k<3; k++){
                            layer[i][j][k] = INITIAL_VALUES[j][k];
                        }
                    }
                    t->state = 1; //task is now idle.
                    TickType_t Time = xTaskGetTickCount();
                    if(i == 0){
                        t->releaseTime = Time;
                        t->deadline = t->releaseTime + TASK1_PERIOD;
                    }
                    else{
                        t->releaseTime = Time;
                        t->deadline = t->releaseTime + TASK2_PERIOD;
                    }
                }
            }
        }

        /* EDF */
        if(num ==2){
            earliestDeadline = 0xFFFFFFFFUL;
            struct TaskParameters *nextTask = NULL;
            TickType_t currentTick = xTaskGetTickCount();//xTaskGetTickCountFromISR();//
            //_DBGUART( "cur tick : %l\r\n", currentTick);
            for (int i = 0; i < mainTASK_ARRAY_SIZE; i++) {
                if (taskArray[i] && taskArray[i]->releaseTime <= currentTick &&
                    taskArray[i]->deadline < earliestDeadline ) {
                    earliestDeadline = taskArray[i]->deadline;
                    nextTask = taskArray[i];
                }
            }

            if (nextTask) {
                if (nextTask == &task1Params) {
                    //_DBGUART("Scheduler: Triggering Task1\r\n");
                    xEventGroupSetBits(xEventGroup, TASK1_BIT);
                } else if (nextTask == &task2Params) {
                    //_DBGUART("Scheduler: Triggering Task2\r\n");
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

