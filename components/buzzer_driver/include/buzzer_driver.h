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


#pragma once

#include <stdbool.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO configuration for buzzer */
#define GPIO_BUZZER 11
#define GPIO_OUTPUT_PIN_BUZZER_SEL ((1ULL << GPIO_BUZZER))

/**
* @brief buzzer driver init, be invoked where you want to use buzzer
* @param power power on/off
*/
void buzzer_driver_init(bool power);

/**
 * @brief Deinitialize the buzzer driver, free resources
 */
void buzzer_driver_deinit();

/**
* @brief Buzzer driver loop, to be used in a separate task when buzzer needs to be turned on
* @param arg not used
*/
void gpio_buzzer_driver_loop(void *arg);
