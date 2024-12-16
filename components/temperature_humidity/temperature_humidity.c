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

#include "dht22.h"
#include "temperature_humidity.h"
#include "esp_log.h"
#include "update_cluster.h"
#include "macros.h"
#include "random_utils.h"

static const char *TAG_TEMP_HUM = "TEMPERATURE_HUMIDITY_CHECK";

#if !defined SIMULATE
static float temperature = 0.0f;
static float humidity = 0.0f;
static bool check_first_implemented = false;
static bool first_implemented_temperature = false;
static bool first_implemented_humidity = false;

static esp_err_t read_dht22_data()
{
    esp_err_t result = dht_read_float_data(DHT_TYPE_AM2301, GPIO_NUM_0, &humidity, &temperature);
    if (result == ESP_OK)
    {
        ESP_LOGI(TAG_TEMP_HUM, "Temperature : %.1f ℃", temperature);
        ESP_LOGI(TAG_TEMP_HUM, "Humidity : %.1f %%", humidity);
    }
    else
    {
        ESP_LOGW(TAG_TEMP_HUM, "Could not read data from DHT22 Sensor.");
    }
    return result;
}
#endif

void check_temperature()
{
#if !defined SIMULATE
    if (check_first_implemented == false || first_implemented_temperature == true)
    {
        read_dht22_data();
        check_first_implemented = true;
        first_implemented_temperature = true;
    }
#else
    int temperature = random_utils_generate(41); // Generate a random temperature between 0 and 30
    ESP_LOGI(TAG_TEMP_HUM, "SIMULATE Temperature : %u ℃", temperature);
#endif
    zb_update_temp((int16_t)(temperature * 100));
#ifdef DEEP_SLEEP
    zb_report_temp();
#endif
}

void check_humidity()
{
#if !defined SIMULATE
    if (check_first_implemented == false || first_implemented_humidity == true)
    {
        read_dht22_data();
        check_first_implemented = true;
        first_implemented_humidity = true;
    }
#else
    int humidity = random_utils_generate(101); // Generate a random temperature between 0 and 100
    ESP_LOGI(TAG_TEMP_HUM, "SIMULATE Humidity : %u %%", humidity);
#endif
    zb_update_hum((uint16_t)(humidity * 100));
#ifdef DEEP_SLEEP
    zb_report_hum();
#endif
}