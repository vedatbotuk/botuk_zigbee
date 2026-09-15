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

#include "macros.h"
#include "device.h"
#include "nvs_flash.h"
#include "esp_check.h"
#include "esp_log.h"
#include "signal_handler.h"
#include "time_sync.h"
#include "zigbee_network.h"

#ifdef LIGHT_SLEEP
#include "light_sleep.h"
#endif

#if defined DEEP_SLEEP
#include "deep_sleep.h"
#endif

#ifdef BATTERY_FEATURES
#include "battery_read.h"
#endif

#ifdef WATERLEAK_FEATURES
#include "waterleak.h"
#endif

#ifdef DHT22
#include "temperature_humidity.h"
#endif

#ifdef BME680
#include "air_quality.h"
#endif

#if defined SWITCH_FEATURES || defined IRRIGATION_FEATURES
#include "switch.h"
#endif

static const char *TAG = "DEVICE";

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    create_signal_handler(*signal_struct);
}

void app_main(void)
{
    ESP_LOGI(TAG, "--- Application Start ---");
    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));

#ifdef LIGHT_SLEEP
    ESP_ERROR_CHECK(esp_zb_power_save_init());
#endif
#ifdef DEEP_SLEEP
    zb_deep_sleep_init();
#endif
#ifdef SWITCH_FEATURES
    ESP_LOGI(TAG, "Deferred driver initialization %s", switch_driver_init(SWITCH_DEFAULT_OFF) ? "failed" : "successful");
#endif
#if defined DHT22
    xTaskCreate(measure_temp_hum, "measure_temp_hum", 4096, NULL, 5, NULL);
#endif
#if defined BME680
#if !defined SIMULATE
    xTaskCreate(bsec_task, "bsec", 10240, NULL, 5, NULL);
#else
    xTaskCreate(sim_bsec_task, "sim_bsec", 4096, NULL, 5, NULL);
#endif
#endif
#ifdef BATTERY_FEATURES
    xTaskCreate(measure_battery, "measure_battery", 4096, NULL, 4, NULL);
#endif
#ifdef WATERLEAK_FEATURES
    if (button_init() == ESP_OK)
    {
        ESP_LOGI(TAG, "Button init successful");
        xTaskCreate(waterleak_loop, "waterleak_loop", 4096, NULL, 3, NULL);
    }
    else
    {
        ESP_LOGE(TAG, "Button init failed");
    }
#endif
    vTaskDelay(pdMS_TO_TICKS(1000)); // delay to ensure all tasks are up before starting Zigbee task
    xTaskCreate(zigbee_network_task, "Zigbee_main", 4096, NULL, 10, NULL);
    xTaskCreate(time_sync_task, "time_sync", 4096, NULL, 3, NULL);
}
