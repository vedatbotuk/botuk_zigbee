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

#include "button.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "switch.h"

static const char *TAG = "BUTTON_DRIVER";

bool button_is_pressed(void)
{
    // Returns true if the pin is pulled to GND (active low)
    return gpio_get_level(BUTTON_INPUT_PIN) == 0;
}

esp_err_t button_init(void)
{
    // Reset pin configuration to default state before configuring
    gpio_reset_pin(BUTTON_INPUT_PIN);
    
    // Set the pin as an input
    gpio_set_direction(BUTTON_INPUT_PIN, GPIO_MODE_INPUT);
    
    // Enable the internal pull-up resistor
    gpio_set_pull_mode(BUTTON_INPUT_PIN, GPIO_PULLUP_ONLY);
    
    ESP_LOGI(TAG, "Button initialized on GPIO %d", BUTTON_INPUT_PIN);
    
    return ESP_OK;
}

void button_loop()
{
    bool last_pressed = false;

    // Infinite task loop
    while (1)
    {
        bool pressed = button_is_pressed();
        
        // Detect state changes
        if (pressed != last_pressed)
        {
            last_pressed = pressed;
            ESP_LOGI(TAG, "Button on GPIO 12 %s", pressed ? "PRESSED" : "RELEASED");
            switch_driver_set_power(pressed);
        }

        // 50ms delay serves as rudimentary software debouncing 
        // and prevents the task from starving the CPU
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}