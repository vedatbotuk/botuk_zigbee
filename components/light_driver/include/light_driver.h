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


#pragma once

#include <stdbool.h>
#include <math.h>
#include "led_strip.h"

#ifdef __cplusplus
extern "C" {
#endif

/* light intensity level */
#define LIGHT_DEFAULT_ON  1
#define LIGHT_DEFAULT_OFF 0

/* LED strip configuration */
#define CONFIG_EXAMPLE_STRIP_LED_GPIO   8
#define CONFIG_EXAMPLE_STRIP_LED_NUMBER 1

/**
* @brief Set light power (on/off).
*
* @param  power  The light power to be set
*/
void light_driver_set_power(bool power);

/**
* @brief color light driver init, be invoked where you want to use color light
*
* @param power power on/off
*/
void light_driver_init(bool power);

/**
 * @brief Deinitialize the light driver, free resources
 */
void light_driver_deinit();

/**
* @brief Set light level
*
* @param  level  The light level to be set
*/
void light_driver_set_level(uint8_t level);

/**
* @brief Set light color from RGB
*
* @param  red    The red color to be set
* @param  green  The green color to be set
* @param  blue   The blue color to be set
*/
void light_driver_set_color_RGB(uint8_t red, uint8_t green, uint8_t blue);

/* @brief Flash task for light driver
* @param  arg  The argument to be set
*/
void flash_task(void *arg);

/* @brief Set red light on/off
* @param  on  The red light on/off to be set
*/
void light_driver_set_red_light(void *arg);

/* @brief Set yellow light on/off
* @param  on  The yellow light on/off to be set
*/
void light_driver_set_yellow_light(void *arg);

/* @brief Set green light on/off
* @param  on  The green light on/off to be set
*/
void light_driver_set_green_light(void *arg);

/* @brief Set white light on/off
* @param  on  The white light on/off to be set
*/
void light_driver_set_white_light(void *arg);

#ifdef __cplusplus
} // extern "C"
#endif
