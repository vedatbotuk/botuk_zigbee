#include "time_sync.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "update_cluster.h"
#include <time.h>

void time_sync_task(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        time_t now;
        struct tm timeinfo;

        time(&now);
        localtime_r(&now, &timeinfo);
        zb_update_current_time(now);
        zb_update_local_time(now);

        vTaskDelay(pdMS_TO_TICKS(1800000));
    }
}
