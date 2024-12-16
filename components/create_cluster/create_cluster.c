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
#include <stdio.h>
#include "create_cluster.h"
#include "esp_log.h"
#include "zcl/esp_zigbee_zcl_power_config.h"
#include "esp_ota_ops.h"
#include <stdio.h>
#include <string.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

static char *manufacturer = "\x05"
                            "Botuk";
static char *model = "\x05" TOSTRING(MODEL_ID_MAP);
static const char *TAG_CREATE_CLUSTER = "Create_Cluster";
static RTC_DATA_ATTR uint8_t lastBatteryPercentageRemaining = 0x8C;
static uint8_t test_attr;

static void convert_version(const char *version_string, char *firmware_version, size_t buffer_size)
{
    int length = strlen(version_string);

    if (length > buffer_size - 2)
    {
        length = buffer_size - 2; // Adjust length to fit within the buffer
    }

    // Set the first element to the length
    firmware_version[0] = length;

    // Copy the characters of the version string into the array starting from index 1
    for (int i = 0; i < length; i++)
    {
        firmware_version[i + 1] = version_string[i];
    }

    // Optional: fill the rest of the array with zeros if desired
    for (int i = length + 1; i < buffer_size; i++)
    {
        firmware_version[i] = 0;
    }
}

void create_basic_cluster(esp_zb_cluster_list_t *esp_zb_cluster_list)
{
#ifdef BATTERY_FEATURES
    uint8_t power_source = 3;
#else
    uint8_t power_source = 1;
#endif

    uint8_t running_version = 1;
    char firmware_version[16];
    char firmware_date[10];
    convert_version(FIRMWARE_VERSION, firmware_version, sizeof(firmware_version));
    convert_version(CURRENT_DATE, firmware_date, sizeof(firmware_date));

    /* basic cluster create with fully customized */
    esp_zb_attribute_list_t *esp_zb_basic_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_BASIC);
    esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, manufacturer);
    esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, model);
    esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_SW_BUILD_ID, firmware_version);
    esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID, &test_attr);
    esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID, &power_source);
    esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_APPLICATION_VERSION_ID, &running_version);
    esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_DATE_CODE_ID, firmware_date);
    esp_zb_cluster_update_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID, &test_attr);
    esp_zb_cluster_list_add_basic_cluster(esp_zb_cluster_list, esp_zb_basic_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    // Log the model, manufacturer, and firmware version
    ESP_LOGI(TAG_CREATE_CLUSTER, "Model: %s", model);
    ESP_LOGI(TAG_CREATE_CLUSTER, "Manufacturer: %s", manufacturer);
    ESP_LOGI(TAG_CREATE_CLUSTER, "Firmware Version: %s", firmware_version);
    ESP_LOGI(TAG_CREATE_CLUSTER, "Firmware Date: %s", firmware_date);
}

void create_identify_cluster(esp_zb_cluster_list_t *esp_zb_cluster_list)
{
    /* identify cluster create with fully customized */
    esp_zb_attribute_list_t *esp_zb_identify_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY);
    esp_zb_identify_cluster_add_attr(esp_zb_identify_cluster, ESP_ZB_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID, &test_attr);
    /* create client role of the cluster */
    esp_zb_cluster_list_add_identify_cluster(esp_zb_cluster_list, esp_zb_identify_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
}

void create_temp_cluster(esp_zb_cluster_list_t *esp_zb_cluster_list)
{
    /* Default temperature value*/
    int16_t undefined_value;
    // TODO: Are following values correct?
    uint16_t temperature_max = 5000;
    uint16_t temperature_min = -5000;
    esp_zb_attribute_list_t *esp_zb_temperature_meas_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT);
    esp_zb_temperature_meas_cluster_add_attr(esp_zb_temperature_meas_cluster, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, &undefined_value);
    esp_zb_temperature_meas_cluster_add_attr(esp_zb_temperature_meas_cluster, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MIN_VALUE_ID, &temperature_min);
    esp_zb_temperature_meas_cluster_add_attr(esp_zb_temperature_meas_cluster, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MAX_VALUE_ID, &temperature_max);
    esp_zb_cluster_list_add_temperature_meas_cluster(esp_zb_cluster_list, esp_zb_temperature_meas_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
}

void create_hum_cluster(esp_zb_cluster_list_t *esp_zb_cluster_list)
{
    uint16_t undefined_value;
    uint16_t humidity_max = 10000;
    uint16_t humidity_min = 0;
    esp_zb_attribute_list_t *esp_zb_hum_meas_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT);
    esp_zb_humidity_meas_cluster_add_attr(esp_zb_hum_meas_cluster, ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID, &undefined_value);
    esp_zb_humidity_meas_cluster_add_attr(esp_zb_hum_meas_cluster, ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MIN_VALUE_ID, &humidity_min);
    esp_zb_humidity_meas_cluster_add_attr(esp_zb_hum_meas_cluster, ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MAX_VALUE_ID, &humidity_max);
    esp_zb_cluster_list_add_humidity_meas_cluster(esp_zb_cluster_list, esp_zb_hum_meas_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
}

void create_waterleak_cluster(esp_zb_cluster_list_t *esp_zb_cluster_list)
{
    esp_zb_ias_zone_cluster_cfg_t ias_zone_cluster_cfg = {
        .zone_state = ESP_ZB_ZCL_IAS_ZONE_ZONESTATE_ENROLLED,
        .zone_type = ESP_ZB_ZCL_IAS_ZONE_ZONETYPE_WATER_SENSOR,
        .ias_cie_addr = ESP_ZB_ZCL_ZONE_IAS_CIE_ADDR_DEFAULT,
    };
    esp_zb_attribute_list_t *esp_zb_ias_zone_cluster = esp_zb_ias_zone_cluster_create(&ias_zone_cluster_cfg);
    esp_zb_cluster_list_add_ias_zone_cluster(esp_zb_cluster_list, esp_zb_ias_zone_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
}

void create_battery_cluster(esp_zb_cluster_list_t *esp_zb_cluster_list)
{
    // TODO: add power_cluster for battery
    esp_zb_power_config_cluster_cfg_t power_cfg = {0};
    uint8_t batteryRatedVoltage = 90;
    uint8_t batteryMinVoltage = 70;
    uint8_t batteryQuantity = 1;
    uint8_t batterySize = 0x02;
    uint16_t batteryAhrRating = 50000;
    uint8_t batteryAlarmMask = 0;
    uint8_t batteryVoltage = 90;

    esp_zb_attribute_list_t *esp_zb_power_cluster = esp_zb_power_config_cluster_create(&power_cfg);
    esp_zb_power_config_cluster_add_attr(esp_zb_power_cluster, ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_ID, &batteryVoltage);
    esp_zb_power_config_cluster_add_attr(esp_zb_power_cluster, ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_SIZE_ID, &batterySize);
    esp_zb_power_config_cluster_add_attr(esp_zb_power_cluster, ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_QUANTITY_ID, &batteryQuantity);
    esp_zb_power_config_cluster_add_attr(esp_zb_power_cluster, ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_RATED_VOLTAGE_ID, &batteryRatedVoltage);
    esp_zb_power_config_cluster_add_attr(esp_zb_power_cluster, ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_ALARM_MASK_ID, &batteryAlarmMask);
    esp_zb_power_config_cluster_add_attr(esp_zb_power_cluster, ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_MIN_THRESHOLD_ID, &batteryMinVoltage);
    esp_zb_power_config_cluster_add_attr(esp_zb_power_cluster, ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_A_HR_RATING_ID, &batteryAhrRating);
    esp_zb_power_config_cluster_add_attr(esp_zb_power_cluster, ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID, &lastBatteryPercentageRemaining);
    // remove 8 first cluster: shift the pointer to the 8th cluster
    for (int i = 0; i < 7; i++)
    {
        esp_zb_power_cluster = esp_zb_power_cluster->next;
    }
    esp_zb_cluster_list_add_power_config_cluster(esp_zb_cluster_list, esp_zb_power_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
}

void create_ota_cluster(esp_zb_cluster_list_t *esp_zb_cluster_list)
{
    /* OTA Upgrade Cluster*/
    esp_zb_ota_cluster_cfg_t ota_cluster_cfg = {
        .ota_upgrade_file_version = OTA_UPGRADE_RUNNING_FILE_VERSION,
        .ota_upgrade_downloaded_file_ver = OTA_UPGRADE_DOWNLOADED_FILE_VERSION,
        .ota_upgrade_manufacturer = OTA_UPGRADE_MANUFACTURER,
        .ota_upgrade_image_type = OTA_UPGRADE_IMAGE_TYPE,
    };
    esp_zb_attribute_list_t *esp_zb_ota_client_cluster = esp_zb_ota_cluster_create(&ota_cluster_cfg);
    /** add client parameters to ota client cluster */
    esp_zb_zcl_ota_upgrade_client_variable_t variable_config = {
        .timer_query = ESP_ZB_ZCL_OTA_UPGRADE_QUERY_TIMER_COUNT_DEF,
        .hw_version = OTA_UPGRADE_HW_VERSION,
        .max_data_size = 64,
    };
    esp_zb_ota_cluster_add_attr(esp_zb_ota_client_cluster, ESP_ZB_ZCL_ATTR_OTA_UPGRADE_CLIENT_DATA_ID, (void *)&variable_config);
    esp_zb_cluster_list_add_ota_cluster(esp_zb_cluster_list, esp_zb_ota_client_cluster, ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE);
}

void create_light_switch_cluster(esp_zb_cluster_list_t *esp_zb_cluster_list)
{
    uint16_t undefined_value;
    esp_zb_attribute_list_t *esp_zb_light_swt_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_ON_OFF);
    esp_zb_on_off_cluster_add_attr(esp_zb_light_swt_cluster, ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID, &undefined_value);
    esp_zb_cluster_list_add_on_off_cluster(esp_zb_cluster_list, esp_zb_light_swt_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
}

void create_time_cluster(esp_zb_cluster_list_t *esp_zb_cluster_list)
{
    uint32_t undefined_value;
    esp_zb_attribute_list_t *esp_zb_time_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_TIME);
    esp_zb_time_cluster_add_attr(esp_zb_time_cluster, ESP_ZB_ZCL_ATTR_TIME_TIME_ID, &undefined_value);
    esp_zb_time_cluster_add_attr(esp_zb_time_cluster, ESP_ZB_ZCL_ATTR_TIME_TIME_STATUS_ID, &undefined_value);
    esp_zb_time_cluster_add_attr(esp_zb_time_cluster, ESP_ZB_ZCL_ATTR_TIME_TIME_ZONE_ID, &undefined_value);
    esp_zb_time_cluster_add_attr(esp_zb_time_cluster, ESP_ZB_ZCL_ATTR_TIME_DST_START_ID, &undefined_value);
    esp_zb_time_cluster_add_attr(esp_zb_time_cluster, ESP_ZB_ZCL_ATTR_TIME_DST_END_ID, &undefined_value);
    esp_zb_time_cluster_add_attr(esp_zb_time_cluster, ESP_ZB_ZCL_ATTR_TIME_DST_SHIFT_ID, &undefined_value);
    esp_zb_time_cluster_add_attr(esp_zb_time_cluster, ESP_ZB_ZCL_ATTR_TIME_STANDARD_TIME_ID, &undefined_value);
    esp_zb_time_cluster_add_attr(esp_zb_time_cluster, ESP_ZB_ZCL_ATTR_TIME_LOCAL_TIME_ID, &undefined_value);
    esp_zb_time_cluster_add_attr(esp_zb_time_cluster, ESP_ZB_ZCL_ATTR_TIME_LAST_SET_TIME_ID, &undefined_value);
    esp_zb_time_cluster_add_attr(esp_zb_time_cluster, ESP_ZB_ZCL_ATTR_TIME_VALID_UNTIL_TIME_ID, &undefined_value);
    esp_zb_cluster_list_add_time_cluster(esp_zb_cluster_list, esp_zb_time_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
}