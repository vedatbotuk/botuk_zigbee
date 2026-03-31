/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 *
 * Zigbee buzzer driver example
 *
 * This example code is in the Public Domain (or CC0 licensed, at your option.)
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include "esp_log.h"
#include "buzzer_driver.h"
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

void buzzer_driver_init(bool power)
{
    // GPIO configuration for an output
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,       // No interrupts for the pin
        .mode = GPIO_MODE_OUTPUT,             // Set pin as output
        .pin_bit_mask = GPIO_OUTPUT_PIN_BUZZER_SEL,  // Configure the desired pin
        .pull_down_en = GPIO_PULLDOWN_ENABLE, // Enable pull-down
        .pull_up_en = GPIO_PULLUP_DISABLE     // Disable pull-up
    };

    gpio_config(&io_conf); // Apply the configuration
    gpio_sleep_sel_dis(GPIO_BUZZER);
    gpio_set_level(GPIO_BUZZER, power);
}

void buzzer_driver_deinit()
{
    gpio_set_level(GPIO_BUZZER, 0);  // optional: set low
    gpio_reset_pin(GPIO_BUZZER);     // reset configuration
}

void gpio_buzzer_driver_loop(void *arg)
{
    uint32_t wait_time = *(uint32_t *)arg;
    buzzer_driver_init(false);

    while (1)
    {
        gpio_hold_dis(GPIO_BUZZER);
        gpio_set_level(GPIO_BUZZER, true);
        gpio_hold_en(GPIO_BUZZER);
        vTaskDelay(pdMS_TO_TICKS(5));

        gpio_hold_dis(GPIO_BUZZER);
        gpio_set_level(GPIO_BUZZER, false);
        gpio_hold_en(GPIO_BUZZER);
        vTaskDelay(pdMS_TO_TICKS(wait_time));
    }
}