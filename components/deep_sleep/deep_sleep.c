/*
 * selforganized_802.15.4_network_with_esp32
 * Copyright (c) 2024 Vedat Botuk.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "deep_sleep.h"
#include "esp_check.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "time.h"
#include "sys/time.h"
#include "driver/rtc_io.h"
#include "driver/uart.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_pm.h"
#include "esp_private/esp_clk.h"

static const char *TAG_SLEEP = "ESP_ZB_DEEP_SLEEP";
static RTC_DATA_ATTR struct timeval s_sleep_enter_time;
static esp_timer_handle_t s_oneshot_timer;

/********************* Define functions **************************/
static void s_oneshot_timer_callback(void *arg)
{
    /* Enter deep sleep */
    ESP_LOGI(TAG_SLEEP, "Enter deep sleep for %dmin\n", DEEP_SLEEP_TIME);
    gettimeofday(&s_sleep_enter_time, NULL);
    esp_deep_sleep_start();
}

void start_deep_sleep()
{
    ESP_ERROR_CHECK(esp_timer_start_once(s_oneshot_timer, before_deep_sleep_time_sec * 1000000));
}

void zb_deep_sleep_init()
{
    const esp_timer_create_args_t s_oneshot_timer_args = {
        .callback = &s_oneshot_timer_callback,
        .name = "one-shot"};

    ESP_ERROR_CHECK(esp_timer_create(&s_oneshot_timer_args, &s_oneshot_timer));

    struct timeval now;
    gettimeofday(&now, NULL);
    int sleep_time_ms = (now.tv_sec - s_sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - s_sleep_enter_time.tv_usec) / 1000;
    esp_sleep_wakeup_cause_t wake_up_cause = esp_sleep_get_wakeup_cause();
    switch (wake_up_cause)
    {
    case ESP_SLEEP_WAKEUP_TIMER:
    {
        ESP_LOGI(TAG_SLEEP, "Wake up from timer. Time spent in deep sleep and boot: %dms", sleep_time_ms);
        break;
    }
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    default:
        ESP_LOGI(TAG_SLEEP, "Not a deep sleep reset");
        break;
    }

    const int wakeup_time_sec = DEEP_SLEEP_TIME * 60;
    ESP_LOGI(TAG_SLEEP, "Enabling timer wakeup, %dmin", DEEP_SLEEP_TIME);
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000));
}
