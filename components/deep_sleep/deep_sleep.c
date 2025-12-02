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
#include "driver/gpio.h"

static const char *TAG_SLEEP = "ESP_ZB_DEEP_SLEEP";
static RTC_DATA_ATTR struct timeval s_sleep_enter_time;
static esp_timer_handle_t s_oneshot_timer;
// #if !defined DEEP_SLEEP
// static const int real_deep_sleep_time = DEEP_SLEEP_TIME * 6;
// #else
// static const int real_deep_sleep_time = DEEP_SLEEP_TIME;
// #endif

static const int protected_pins[] = {
    // common flash/psram pins (ESP32 family); conservative for ESP32-H2 modules:
    6, 7, 8, 9, 10, 11,
    15, 16, 17, 18, 19, 20, 21};

/********************* Define functions **************************/
static bool is_protected(int pin)
{
    for (size_t i = 0; i < sizeof(protected_pins) / sizeof(protected_pins[0]); ++i)
    {
        if (protected_pins[i] == pin)
            return true;
    }
    return false;
}

void disable_all_gpios_safe(void)
{
    for (int pin = 0; pin < GPIO_NUM_MAX; ++pin)
    {
        if (!GPIO_IS_VALID_GPIO(pin))
        {
            continue; // not a usable gpio on this chip
        }
        if (is_protected(pin))
        {
            // Skip pins that are likely wired to flash or are reserved
            continue;
        }

        // Disable any pad-hold (in case enabled before)
        gpio_hold_dis(pin);

        // Make it input, disable pulls -> hi-z
        gpio_set_direction(pin, GPIO_MODE_INPUT);
// If your IDF supports gpio_set_pull_mode use it:
#if defined(GPIO_PULLUP_ONLY) || defined(GPIO_FLOATING)
        // some IDF versions:
        gpio_set_pull_mode(pin, GPIO_FLOATING);
#else
        // fallback to explicit disable
        gpio_pullup_dis(pin);
        gpio_pulldown_dis(pin);
#endif

        // Optionally reset pin config to default (safer on some IDF versions)
        // gpio_reset_pin(pin); // DO NOT call this if you're not 100% sure it's safe for your board
    }
}

static void s_oneshot_timer_callback(void *arg)
{
    /* Enter deep sleep */
    ESP_LOGI(TAG_SLEEP, "Enter deep sleep for %dmin\n", DEEP_SLEEP_TIME);
    gettimeofday(&s_sleep_enter_time, NULL);
    esp_deep_sleep_start();
}

void start_deep_sleep()
{
    // Disable all GPIOs before sleeping
    disable_all_gpios_safe();
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
