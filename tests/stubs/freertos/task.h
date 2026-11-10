#ifndef TEST_FREERTOS_TASK_H
#define TEST_FREERTOS_TASK_H

#include "freertos/FreeRTOS.h"

void vTaskDelay(TickType_t ticks);
void vTaskDelete(void *task);

#endif
