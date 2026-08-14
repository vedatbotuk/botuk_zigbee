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

#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Pin definition for the button input.
 *        Adjust GPIO_NUM_X to match your hardware layout.
 *
 */
#define BUTTON_INPUT_PIN GPIO_NUM_12

/**
 * @brief Check whether the button is currently pressed.
 * 
 * @return true if button is pressed (active low), false otherwise.
 */
bool button_is_pressed(void);

/**
 * @brief Initialize the button GPIO pin as an input with internal pull-up.
 * 
 * @return ESP_OK on success.
 */
esp_err_t button_init(void);

/**
 * @brief Infinite task loop that monitors button state changes and logs events.
 */
void button_loop();

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_H */