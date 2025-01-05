/*
 * task_common.h
 *
 *  Created on: 2024. 12. 30.
 *      Author: kooo
 */

#ifndef TASK_COMMON_H_
#define TASK_COMMON_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

// Shared Queue
extern QueueHandle_t xSchedulerMessages;

// Shared Event Group
extern EventGroupHandle_t xEventGroup;

enum TaskStatus { statusCOMPLETED, statusRUNNING, statusCREATED };

struct Message
{
    enum TaskStatus status;
    struct TaskParameters *params;
};

struct Job {
    TickType_t executionTime;
};

struct TaskParameters {
    //struct Job jobs[3];
    TickType_t releaseTime;
    TickType_t deadline;
    int currentJob;
    int totalJobs;
    TaskHandle_t handle;
};

#define mainSCHEDULER_PRIORITY 2
#define mainNEW_TASK_PRIORITY 3

#define TASK1_BIT (1 << 0) // Task1 실행 비트
#define TASK2_BIT (1 << 1) // Task2 실행 비트

/* Task 1 and Task 2 Definitions */
#define TASK1_PERIOD 2000  // ( T = D)
#define TASK1_DEADLINE 2000

#define TASK2_PERIOD 3000  // ( T = D)
#define TASK2_DEADLINE 3000

#define DATA_SIZE 32*32*3

extern struct TaskParameters task1Params;
extern struct TaskParameters task2Params;

// Task Array
#define mainTASK_ARRAY_SIZE 2
extern struct TaskParameters *taskArray[mainTASK_ARRAY_SIZE];

// Task Functions
void addTaskToArray(struct TaskParameters *params);
void removeTaskFromArray(struct TaskParameters *params);

void Task1(void *pvParameters);
void Task2(void *pvParameters);
void SchedulerTask(void *pvParameters);

#endif /* TASK_COMMON_H_ */
