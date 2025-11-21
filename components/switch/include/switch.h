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

#pragma once

#include <stdbool.h>
#include <esp_err.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* light intensity level */
#define SWITCH_DEFAULT_ON 1
#define SWITCH_DEFAULT_OFF 0

#define GPIO_OUTPUT_PIN_1 26
#define GPIO_OUTPUT_PIN_2 27
#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_PIN_1) | (1ULL << GPIO_OUTPUT_PIN_2))

  /**
   * @brief Set switch power (on/off).
   *
   * @param  power  The switch power to be set
   */
  void switch_driver_set_power(bool power);

  /**
   * @brief switch driver init, be invoked where you want to use switch
   *
   * @param power power on/off
   */
  esp_err_t switch_driver_init(bool power);

#ifdef __cplusplus
} // extern "C"
#endif