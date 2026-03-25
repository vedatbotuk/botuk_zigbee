/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 *
 * Zigbee light driver example
 *
 * This example code is in the Public Domain (or CC0 licensed, at your option.)
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include "esp_log.h"
#include "led_strip.h"
#include "light_driver.h"
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

static led_strip_handle_t s_led_strip;
static uint8_t s_red = 255, s_green = 255, s_blue = 255, s_level = 255;

void light_driver_set_color_RGB(uint8_t red, uint8_t green, uint8_t blue)
{
    float ratio = (float)s_level / 255;
    s_red = red;
    s_green = green;
    s_blue = blue;
    ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, 0, red * ratio, green * ratio, blue * ratio));
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
}

void light_driver_set_power(bool power)
{
    ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, 0, s_red * power, s_green * power, s_blue * power));
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
}

void light_driver_set_level(uint8_t level)
{
    s_level = level;
    float ratio = (float)s_level / 255;
    ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, 0, s_red * ratio, s_green * ratio, s_blue * ratio));
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
}

void light_driver_init(bool power)
{
#if HW_VERSION == 128 || HW_VERSION == 127
    if (s_led_strip != NULL)
    {
        ESP_LOGW("light_driver_init", "LED strip already initialized");
        return;
    }
    led_strip_config_t led_strip_conf = {
        .max_leds = CONFIG_EXAMPLE_STRIP_LED_NUMBER,
        .strip_gpio_num = CONFIG_EXAMPLE_STRIP_LED_GPIO,
    };
    led_strip_rmt_config_t rmt_conf = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz

    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&led_strip_conf, &rmt_conf, &s_led_strip));

    light_driver_set_power(power);
#endif
#if HW_VERSION == 126 || HW_VERSION == 125
    // GPIO configuration for an output
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,       // No interrupts for the pin
        .mode = GPIO_MODE_OUTPUT,             // Set pin as output
        .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,  // Configure the desired pin
        .pull_down_en = GPIO_PULLDOWN_ENABLE, // Enable pull-down
        .pull_up_en = GPIO_PULLUP_DISABLE     // Disable pull-up
    };

    gpio_config(&io_conf); // Apply the configuration
    gpio_sleep_sel_dis(GPIO_LIGHT_RED);
    gpio_set_level(GPIO_LIGHT_RED, power);
#if HW_VERSION == 125
    gpio_sleep_sel_dis(GPIO_LIGHT_YELLOW);
    gpio_set_level(GPIO_LIGHT_YELLOW, power);
#endif
#endif
}

void light_driver_deinit()
{
#if HW_VERSION == 128 || HW_VERSION == 127
    if (s_led_strip)
    {
        // Turn off all LEDs
        led_strip_clear(s_led_strip);
        led_strip_refresh(s_led_strip);

        // Delete RMT device (VERY IMPORTANT)
        led_strip_del(s_led_strip);
        // ESP_LOGI("light_driver_deinit", "LED strip deinitialized and resources freed");
        s_led_strip = NULL;
    }
#endif
#if HW_VERSION == 126 || HW_VERSION == 125
        gpio_set_level(GPIO_LIGHT_RED, 0);  // optional: set low
        gpio_reset_pin(GPIO_LIGHT_RED);     // reset configuration
#if HW_VERSION == 125
        gpio_set_level(GPIO_LIGHT_YELLOW, 0);  // optional: set low
        gpio_reset_pin(GPIO_LIGHT_YELLOW);     // reset configuration
#endif
#endif

}

void light_driver_set_red_light(void *arg)
{
    s_red = 255;
    s_green = 0;
    s_blue = 0;
    uint8_t level = 255;
    light_driver_loop(level);
}

void light_driver_set_yellow_light(void *arg)
{
    s_red = 255;
    s_green = 150;
    s_blue = 0;
    uint8_t level = 120;
    light_driver_loop(level);
}

void light_driver_set_green_light(void *arg)
{
    s_red = 0;
    s_green = 255;
    s_blue = 0;
    uint8_t level = 120;
    light_driver_loop(level);
}

void light_driver_set_white_light(void *arg)
{
    s_red = 255;
    s_green = 255;
    s_blue = 255;
    uint8_t level = 120;
    light_driver_loop(level);
}

void light_driver_loop(uint8_t level)
{
    light_driver_init(true);
#if HW_VERSION == 128
    light_driver_set_color_RGB(s_green, s_red, s_blue);
#else
    light_driver_set_color_RGB(s_red, s_green, s_blue);
#endif
    while (1)
    {
        light_driver_init(false);
        vTaskDelay(pdMS_TO_TICKS(10));
        light_driver_set_level(level);
        vTaskDelay(pdMS_TO_TICKS(10));

        light_driver_set_level(0);
        vTaskDelay(pdMS_TO_TICKS(10));
        light_driver_deinit();
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void gpio_light_driver_loop_red(void *arg)
{
    light_driver_init(false);

    while (1)
    {
        gpio_hold_dis(GPIO_LIGHT_RED);
        gpio_set_level(GPIO_LIGHT_RED, true);
        gpio_hold_en(GPIO_LIGHT_RED);
        vTaskDelay(pdMS_TO_TICKS(10));

        gpio_hold_dis(GPIO_LIGHT_RED);
        gpio_set_level(GPIO_LIGHT_RED, false);
        gpio_hold_en(GPIO_LIGHT_RED);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void gpio_light_driver_loop_yellow(void *arg)
{
    light_driver_init(false);

    while (1)
    {
        gpio_hold_dis(GPIO_LIGHT_YELLOW);
        gpio_set_level(GPIO_LIGHT_YELLOW, true);
        gpio_hold_en(GPIO_LIGHT_YELLOW);
        vTaskDelay(pdMS_TO_TICKS(10));

        gpio_hold_dis(GPIO_LIGHT_YELLOW);
        gpio_set_level(GPIO_LIGHT_YELLOW, false);
        gpio_hold_en(GPIO_LIGHT_YELLOW);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}