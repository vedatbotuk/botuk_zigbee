#include "device_handlers.h"

#include <sys/time.h>

#include "create_cluster.h"
#include "esp_check.h"
#include "esp_log.h"
#include "macros.h"
#include "update_cluster.h"

#ifdef OTA_UPDATE
#include "ota.h"
#endif

#if defined SWITCH_FEATURES || defined IRRIGATION_FEATURES
#include "switch.h"
#endif

#ifdef BUILTIN_LIGHT
#include "ha/esp_zigbee_ha_standard.h"
#include "light_driver.h"
#include "buzzer_driver.h"

static TaskHandle_t flash_task_handle = NULL;
#if HW_VERSION == 124 || HW_VERSION == 123
static TaskHandle_t buzzer_task_handle = NULL;
static uint32_t buzzer_wait_red = 10000;
#if HW_VERSION == 123
static uint32_t buzzer_wait_yellow = 60000;
#endif
#endif
#endif

static const char *TAG = "DEVICE_HANDLERS";

#ifdef BUILTIN_LIGHT
static void zb_stop_builtin_light_flash_task(const char *light_name)
{
    if (flash_task_handle != NULL)
    {
        vTaskDelete(flash_task_handle);
        flash_task_handle = NULL;
        ESP_LOGI(TAG, "Stopped flashing %s", light_name);
    }
}

static void zb_start_builtin_light_flash_task(TaskFunction_t task_function, const char *task_name, const char *light_name, UBaseType_t priority)
{
    if (flash_task_handle == NULL)
    {
        ESP_LOGI(TAG, "Started flashing %s", light_name);
    }
    else
    {
        zb_stop_builtin_light_flash_task(light_name);
        ESP_LOGI(TAG, "Already flashing another light, stopping flash task and starting to flash %s", light_name);
    }

    xTaskCreate(task_function, task_name, 2048, NULL, priority, &flash_task_handle);
}

static void zb_deinit_builtin_light_driver(void)
{
#if HW_VERSION == 126 || HW_VERSION == 125 || HW_VERSION == 124 || HW_VERSION == 123
    gpio_light_driver_deinit();
#elif HW_VERSION == 128 || HW_VERSION == 127
    light_driver_deinit();
#endif
}

static void zb_reset_builtin_light_attributes(void)
{
#if HW_VERSION == 128 || HW_VERSION == 127
    zb_update_builtin_light_flash_red(0);
    zb_update_builtin_light_flash_yellow(0);
    zb_update_builtin_light_flash_green(0);
    zb_update_builtin_light_flash_white(0);
#elif HW_VERSION == 126 || HW_VERSION == 125 || HW_VERSION == 124 || HW_VERSION == 123
    zb_update_builtin_light_flash_red(0);
#if HW_VERSION == 125 || HW_VERSION == 123
    zb_update_builtin_light_flash_yellow(0);
#endif
#endif
}

#if HW_VERSION == 124 || HW_VERSION == 123
static void zb_stop_builtin_light_buzzer(void)
{
    if (buzzer_task_handle != NULL)
    {
        vTaskDelete(buzzer_task_handle);
        buzzer_task_handle = NULL;
        ESP_LOGI(TAG, "Stopped buzzer");
    }
    buzzer_driver_deinit();
}
#endif
#endif

#if defined SWITCH_FEATURES
static esp_err_t zb_switch_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message)
{
    esp_err_t ret = ESP_OK;
    bool switch_state = 0;

    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)", message->info.status);
    ESP_LOGI(TAG, "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)", message->info.dst_endpoint, message->info.cluster,
             message->attribute.id, message->attribute.data.size);
    if (message->info.dst_endpoint == DEVICE_ENDPOINT)
    {
        if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF)
        {
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID && message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL)
            {
#ifdef BATTERY_FEATURES
                if (get_battery_level() < 10)
                {
                    ESP_LOGW(TAG, "Battery level is too low to change the switch state false.");
                    switch_driver_set_power(0);
                }
                else
                {
                    switch_state = message->attribute.data.value ? *(bool *)message->attribute.data.value : switch_state;
                    ESP_LOGI(TAG, "Switch sets to %s", switch_state ? "On" : "Off");
                    switch_driver_set_power(switch_state);
                    zb_update_switch(switch_state);
                }
#else
                switch_state = message->attribute.data.value ? *(bool *)message->attribute.data.value : switch_state;
                ESP_LOGI(TAG, "Switch sets to %s", switch_state ? "On" : "Off");
                switch_driver_set_power(switch_state);
                zb_update_switch(switch_state);
#endif
            }
        }
    }
    return ret;
}
#endif

#if defined BUILTIN_LIGHT
static esp_err_t zb_builtin_light_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message)
{
    esp_err_t ret = ESP_OK;
    bool light_state = 0;

    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)", message->info.status);
    ESP_LOGI(TAG, "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)", message->info.dst_endpoint, message->info.cluster,
             message->attribute.id, message->attribute.data.size);

    if (message->info.dst_endpoint == DEVICE_ENDPOINT)
    {
        switch (message->info.cluster)
        {
#if HW_VERSION == 126 || HW_VERSION == 125 || HW_VERSION == 124 || HW_VERSION == 123
        case ESP_ZB_ZCL_CLUSTER_ID_RED_LIGHT_ON_OFF:
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_RED_LIGHT_ON_OFF_ID &&
                message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL)
            {
                light_state = message->attribute.data.value ? *(bool *)message->attribute.data.value : light_state;
                if (light_state)
                {
                    zb_start_builtin_light_flash_task(gpio_light_driver_loop_red, "light_driver_set_red_light", "red light", 1);
#if HW_VERSION == 125 || HW_VERSION == 123
                    zb_update_builtin_light_flash_yellow(0);
#endif
#if HW_VERSION == 124 || HW_VERSION == 123
                    if (buzzer_task_handle != NULL)
                    {
                        vTaskDelete(buzzer_task_handle);
                        ESP_LOGI(TAG, "Already buzzing for another light, stopping buzzer task and starting to buzz for red light");
                    }
                    xTaskCreate(gpio_buzzer_driver_loop, "buzzer_driver_loop", 2048, &buzzer_wait_red, 5, &buzzer_task_handle);
#endif
                    zb_update_builtin_light_flash_red(1);
                }
                else
                {
                    zb_stop_builtin_light_flash_task("red light");
                    zb_deinit_builtin_light_driver();
                    zb_reset_builtin_light_attributes();
#if HW_VERSION == 124 || HW_VERSION == 123
                    zb_stop_builtin_light_buzzer();
#endif
                }
            }
            break;
#if HW_VERSION == 125 || HW_VERSION == 123
        case ESP_ZB_ZCL_CLUSTER_ID_YELLOW_LIGHT_ON_OFF:
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_YELLOW_LIGHT_ON_OFF_ID &&
                message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL)
            {
                light_state = message->attribute.data.value ? *(bool *)message->attribute.data.value : light_state;
                if (light_state)
                {
                    zb_start_builtin_light_flash_task(gpio_light_driver_loop_yellow, "light_driver_set_yellow_light", "yellow light", 1);
                    zb_update_builtin_light_flash_red(0);
#if HW_VERSION == 123
                    if (buzzer_task_handle != NULL)
                    {
                        vTaskDelete(buzzer_task_handle);
                        ESP_LOGI(TAG, "Already buzzing for another light, stopping buzzer task and starting to buzz for yellow light");
                    }
                    xTaskCreate(gpio_buzzer_driver_loop, "gpio_buzzer_driver_loop", 2048, &buzzer_wait_yellow, 5, &buzzer_task_handle);
#endif
                }
                else
                {
                    zb_stop_builtin_light_flash_task("yellow light");
                    zb_deinit_builtin_light_driver();
                    zb_reset_builtin_light_attributes();
#if HW_VERSION == 123
                    zb_stop_builtin_light_buzzer();
#endif
                }
            }
            break;
#endif
#elif HW_VERSION == 128 || HW_VERSION == 127
        case ESP_ZB_ZCL_CLUSTER_ID_RED_LIGHT_ON_OFF:
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_RED_LIGHT_ON_OFF_ID &&
                message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL)
            {
                light_state = message->attribute.data.value ? *(bool *)message->attribute.data.value : light_state;
                if (light_state)
                {
                    zb_start_builtin_light_flash_task(light_driver_set_red_light, "light_driver_set_red_light", "red light", 5);
                    zb_update_builtin_light_flash_yellow(0);
                    zb_update_builtin_light_flash_green(0);
                    zb_update_builtin_light_flash_white(0);
                }
                else
                {
                    zb_stop_builtin_light_flash_task("red light");
                    zb_deinit_builtin_light_driver();
                    zb_reset_builtin_light_attributes();
                }
            }
            break;
        case ESP_ZB_ZCL_CLUSTER_ID_YELLOW_LIGHT_ON_OFF:
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_YELLOW_LIGHT_ON_OFF_ID &&
                message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL)
            {
                light_state = message->attribute.data.value ? *(bool *)message->attribute.data.value : light_state;
                if (light_state)
                {
                    zb_start_builtin_light_flash_task(light_driver_set_yellow_light, "light_driver_set_yellow_light", "yellow light", 4);
                    zb_update_builtin_light_flash_red(0);
                    zb_update_builtin_light_flash_green(0);
                    zb_update_builtin_light_flash_white(0);
                }
                else
                {
                    zb_stop_builtin_light_flash_task("yellow light");
                    zb_deinit_builtin_light_driver();
                    zb_reset_builtin_light_attributes();
                }
            }
            break;
        case ESP_ZB_ZCL_CLUSTER_ID_GREEN_LIGHT_ON_OFF:
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_GREEN_LIGHT_ON_OFF_ID &&
                message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL)
            {
                light_state = message->attribute.data.value ? *(bool *)message->attribute.data.value : light_state;
                if (light_state)
                {
                    zb_start_builtin_light_flash_task(light_driver_set_green_light, "light_driver_set_green_light", "green light", 5);
                    zb_update_builtin_light_flash_red(0);
                    zb_update_builtin_light_flash_yellow(0);
                    zb_update_builtin_light_flash_white(0);
                }
                else
                {
                    zb_stop_builtin_light_flash_task("green light");
                    zb_deinit_builtin_light_driver();
                    zb_reset_builtin_light_attributes();
                }
            }
            break;
        case ESP_ZB_ZCL_CLUSTER_ID_WHITE_LIGHT_ON_OFF:
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_WHITE_LIGHT_ON_OFF_ID &&
                message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL)
            {
                light_state = message->attribute.data.value ? *(bool *)message->attribute.data.value : light_state;
                if (light_state)
                {
                    zb_start_builtin_light_flash_task(light_driver_set_white_light, "light_driver_set_white_light", "white light", 5);
                    zb_update_builtin_light_flash_red(0);
                    zb_update_builtin_light_flash_yellow(0);
                    zb_update_builtin_light_flash_green(0);
                }
                else
                {
                    zb_stop_builtin_light_flash_task("white light");
                    zb_deinit_builtin_light_driver();
                    zb_reset_builtin_light_attributes();
                }
            }
            break;
#endif
        default:
            ESP_LOGI(TAG, "Message data: cluster(0x%x), attribute(0x%x)  ", message->info.cluster, message->attribute.id);
        }
    }
    return ret;
}
#endif

#if defined IRRIGATION_FEATURES
static esp_err_t zb_irrigation_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message)
{
    esp_err_t ret = ESP_OK;
    bool relay_state = 0;

    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)", message->info.status);
    ESP_LOGI(TAG, "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)", message->info.dst_endpoint, message->info.cluster,
             message->attribute.id, message->attribute.data.size);

    if (message->info.dst_endpoint == DEVICE_ENDPOINT)
    {
        if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_IRRIGATION_ON_OFF)
        {
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_IRRIGATION_ON_OFF_ID && message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL)
            {
                relay_state = message->attribute.data.value ? *(bool *)message->attribute.data.value : relay_state;
                ESP_LOGI(TAG, "Relay sets to %s", relay_state ? "On" : "Off");
                switch_relay_set_power(relay_state);
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
        tv.tv_sec = *(uint32_t *)attribute->data.value + 946684800 - 1080;
        settimeofday(&tv, NULL);
        zb_update_current_time(*(uint32_t *)attribute->data.value);
    }
}

static esp_err_t zb_read_attr_resp_handler(const esp_zb_zcl_cmd_read_attr_resp_message_t *message)
{
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)", message->info.status);

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

esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
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
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
    {
        const esp_zb_zcl_set_attr_value_message_t *set_attr_msg = (const esp_zb_zcl_set_attr_value_message_t *)message;
        uint16_t cluster_id = set_attr_msg->info.cluster;

#ifdef SWITCH_FEATURES
        if (cluster_id == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF)
        {
            return zb_switch_attribute_handler(set_attr_msg);
        }
#endif

#ifdef IRRIGATION_FEATURES
        if (cluster_id == ESP_ZB_ZCL_CLUSTER_ID_IRRIGATION_ON_OFF)
        {
            return zb_irrigation_attribute_handler(set_attr_msg);
        }
#endif

#ifdef BUILTIN_LIGHT
        if (cluster_id == ESP_ZB_ZCL_CLUSTER_ID_RED_LIGHT_ON_OFF ||
            cluster_id == ESP_ZB_ZCL_CLUSTER_ID_YELLOW_LIGHT_ON_OFF ||
            cluster_id == ESP_ZB_ZCL_CLUSTER_ID_GREEN_LIGHT_ON_OFF ||
            cluster_id == ESP_ZB_ZCL_CLUSTER_ID_WHITE_LIGHT_ON_OFF)
        {
            return zb_builtin_light_attribute_handler(set_attr_msg);
        }
#endif

        ESP_LOGW(TAG, "No handler found for Cluster ID: 0x%x", cluster_id);
        break;
    }
    default:
        ESP_LOGW(TAG, "Receive Zigbee action(0x%x) callback", callback_id);
        break;
    }
    return ret;
}
