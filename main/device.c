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
#include "update_cluster.h"
#include "create_cluster.h"
#include "signal_handler.h"
#include <time.h>
#include <sys/time.h>
#include "random_utils.h"

#ifdef OTA_UPDATE
#include "ota.h"
#endif

#ifdef LIGHT_SLEEP
#include "light_sleep.h"
#endif

#ifdef DEEP_SLEEP
#include "deep_sleep.h"
#endif

#ifdef BATTERY_FEATURES
#include "battery_read.h"
#endif

#ifdef WATERLEAK_FEATURES
#include "waterleak.h"
#endif

#if defined TEMPERATURE_FEATURES || defined HUMIDITY_FEATURES
#include "temperature_humidity.h"
#endif

#ifdef SWITCH_FEATURES
#include "switch.h"
#endif

static const char *TAG = "DEVICE";

bool connected = false;

/********************* Define functions **************************/
void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    create_signal_handler(*signal_struct);
}

#if !defined DEEP_SLEEP
#if defined TEMPERATURE_FEATURES || defined HUMIDITY_FEATURES
void measure_temp_hum()
{
    while (1)
    {
        connected = connection_status();
        if (connected)
        {
#ifdef TEMPERATURE_FEATURES
            check_temperature();
#endif
#ifdef HUMIDITY_FEATURES
            check_humidity();
#endif
        }
        else
        {
            ESP_LOGW(TAG, "Device is not connected! Could not measure the temperature and humidity");
        }
#if !defined TESTING
        vTaskDelay(pdMS_TO_TICKS(60000)); // 300000 ms = 5 minutes
#else
        vTaskDelay(pdMS_TO_TICKS(30000)); // 30000 ms = 30 seconds
#endif
    }
}
#endif

#ifdef BATTERY_FEATURES
void measure_battery()
{
    while (1)
    {
        connected = connection_status();
        if (connected)
        {
            get_battery_level();
        }
        else
        {
            ESP_LOGW(TAG, "Device is not connected! Could not measure the battery level");
        }
#if !defined TESTING
        vTaskDelay(pdMS_TO_TICKS(600000)); // 900000 ms = 15 minutes
#else
        vTaskDelay(pdMS_TO_TICKS(60000)); // 60000 ms = 1 minutes
#endif
    }
}
#endif

#ifdef WATERLEAK_FEATURES
void waterleak_loop()
{
    while (1)
    {
        connected = connection_status();
        if (connected)
        {
            check_waterleak();
        }
        else
        {
            ESP_LOGW(TAG, "Device is not connected! Could not measure the waterleak status");
        }
        vTaskDelay(pdMS_TO_TICKS(10000)); // 10000 ms = 10 seconds
    }
}
#endif
#endif

#ifdef SWITCH_FEATURES
static esp_err_t zb_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message)
{
    esp_err_t ret = ESP_OK;
    bool light_state = 0;

    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)",
                        message->info.status);
    ESP_LOGI(TAG, "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)", message->info.dst_endpoint, message->info.cluster,
             message->attribute.id, message->attribute.data.size);
    if (message->info.dst_endpoint == DEVICE_ENDPOINT)
    {
        if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF)
        {
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID && message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL)
            {
                light_state = message->attribute.data.value ? *(bool *)message->attribute.data.value : light_state;
                ESP_LOGI(TAG, "Light sets to %s", light_state ? "On" : "Off");
                light_driver_set_power(light_state);
            }
        }
    }
    return ret;
}
#endif

static void esp_app_zb_attribute_handler(uint16_t cluster_id, const esp_zb_zcl_attribute_t *attribute)
{
    if (cluster_id == ESP_ZB_ZCL_CLUSTER_ID_TIME)
    {
        ESP_LOGI(TAG, "Server time recieved %lu", *(uint32_t *)attribute->data.value);
        struct timeval tv;
        tv.tv_sec = *(uint32_t *)attribute->data.value + 946684800 - 1080; // after adding OTA cluster time shifted to 1080 sec... strange issue ...
        settimeofday(&tv, NULL);
        zb_update_current_time(*(uint32_t *)attribute->data.value);
    }
}

static esp_err_t zb_read_attr_resp_handler(const esp_zb_zcl_cmd_read_attr_resp_message_t *message)
{
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)",
                        message->info.status);

    ESP_LOGI(TAG, "Read attribute response: from address(0x%x) src endpoint(%d) to dst endpoint(%d) cluster(0x%x)",
             message->info.src_address.u.short_addr, message->info.src_endpoint,
             message->info.dst_endpoint, message->info.cluster);

    esp_zb_zcl_read_attr_resp_variable_t *variable = message->variables;
    while (variable)
    {
        ESP_LOGI(TAG, "Read attribute response: status(%d), cluster(0x%x), attribute(0x%x), type(0x%x), value(%d)",
                 variable->status, message->info.cluster,
                 variable->attribute.id, variable->attribute.data.type,
                 variable->attribute.data.value ? *(uint8_t *)variable->attribute.data.value : 0);
        if (variable->status == ESP_ZB_ZCL_STATUS_SUCCESS)
        {
            esp_app_zb_attribute_handler(message->info.cluster, &variable->attribute);
        }

        variable = variable->next;
    }

    return ESP_OK;
}

static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    esp_err_t ret = ESP_OK;
    switch (callback_id)
    {
    case ESP_ZB_CORE_CMD_READ_ATTR_RESP_CB_ID:
        ret = zb_read_attr_resp_handler((esp_zb_zcl_cmd_read_attr_resp_message_t *)message);
        break;
#ifdef OTA_UPDATE
    case ESP_ZB_CORE_OTA_UPGRADE_VALUE_CB_ID:
        ret = zb_ota_upgrade_status_handler(*(esp_zb_zcl_ota_upgrade_value_message_t *)message);
        break;
#endif
#ifdef SWITCH_FEATURES
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
        ret = zb_attribute_handler((esp_zb_zcl_set_attr_value_message_t *)message);
        break;
#endif
    default:
        ESP_LOGW(TAG, "Receive Zigbee action(0x%x) callback", callback_id);
        break;
    }
    return ret;
}

static void esp_zb_task(void *pvParameters)
{
    /* initialize Zigbee stack */
#ifdef END_DEVICE
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
    ESP_LOGI(TAG, "Enable END_DEVICE");
#endif
#ifdef ROUTER_DEVICE
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZR_CONFIG();
    ESP_LOGI(TAG, "Enable ROUTER_DEVICE");
#endif
    /* The order in the following 3 lines must not be changed. */
#ifdef LIGHT_SLEEP
    esp_zb_sleep_enable(true);
#endif
    esp_zb_init(&zb_nwk_cfg);
#ifdef LIGHT_SLEEP
    esp_zb_sleep_set_threshold(2000);
    ESP_LOGI(TAG, "Enable LIGHT_SLEEP");
#endif
#ifdef ROUTER_DEVICE
    esp_zb_set_tx_power(20);
#endif
#ifdef END_DEVICE
    esp_zb_set_tx_power(5);
#endif

    /* create cluster lists for this endpoint */
    esp_zb_cluster_list_t *esp_zb_cluster_list = esp_zb_zcl_cluster_list_create();

    create_basic_cluster(esp_zb_cluster_list);
    create_identify_cluster(esp_zb_cluster_list);
    create_time_cluster(esp_zb_cluster_list);
#ifdef TEMPERATURE_FEATURES
    create_temp_cluster(esp_zb_cluster_list);
    ESP_LOGI(TAG, "Create SENSOR_TEMPERATURE Cluster");

#endif
#ifdef HUMIDITY_FEATURES
    create_hum_cluster(esp_zb_cluster_list);
    ESP_LOGI(TAG, "Create SENSOR_HUMIDITY Cluster");

#endif
#ifdef WATERLEAK_FEATURES
    create_waterleak_cluster(esp_zb_cluster_list);
    ESP_LOGI(TAG, "Create WATERLEAK Cluster");

#endif
#ifdef BATTERY_FEATURES
    create_battery_cluster(esp_zb_cluster_list);
    ESP_LOGI(TAG, "Create BATTERY Cluster");

#endif
#ifdef OTA_UPDATE
    create_ota_cluster(esp_zb_cluster_list);
    ESP_LOGI(TAG, "Create OTA_UPDATE Cluster");
#endif

#ifdef SWITCH_FEATURES
    create_light_switch_cluster(esp_zb_cluster_list);
#endif

    esp_zb_ep_list_t *esp_zb_ep_list = esp_zb_ep_list_create();
    esp_zb_endpoint_config_t endpoint_config = {
        .endpoint = DEVICE_ENDPOINT,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = ESP_ZB_HA_TEST_DEVICE_ID,
        .app_device_version = 0};
    esp_zb_ep_list_add_ep(esp_zb_ep_list, esp_zb_cluster_list, endpoint_config);

    esp_zb_device_register(esp_zb_ep_list);
    esp_zb_core_action_handler_register(zb_action_handler);
    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);
    ESP_ERROR_CHECK(esp_zb_start(false));
    esp_zb_stack_main_loop();
}

static void update_rtc_time()
{
    while (1)
    {
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        // ESP_LOGI(TAG, "Current time: %s", asctime(&timeinfo));
        connected = connection_status();
        if (connected)
        {
            zb_update_current_time(now);
            zb_update_local_time(now);
        }
        vTaskDelay(pdMS_TO_TICKS(60000)); // 60000 ms = 1 minutes
    }
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
    ESP_LOGI(TAG, "Deferred driver initialization %s", light_driver_init(LIGHT_DEFAULT_OFF) ? "failed" : "successful");
#endif
#if !defined DEEP_SLEEP
#if defined TEMPERATURE_FEATURES || defined HUMIDITY_FEATURES
    xTaskCreate(measure_temp_hum, "measure_temp_hum", 4096, NULL, 5, NULL);
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
#endif
    xTaskCreate(esp_zb_task, "Zigbee_main", 4 * 1024, NULL, 10, NULL);
    xTaskCreate(update_rtc_time, "update_rtc_time", 4096, NULL, 5, NULL);
}
