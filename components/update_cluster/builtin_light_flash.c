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

#include <stdint.h>
#include "esp_zigbee_core.h"
#include "esp_log.h"
#include "create_cluster.h"
#include "update_cluster.h"

static const char *TAG_ZB_UPDATE_BUILTIN_LIGHT = "UPDATE_BUILTIN_LIGHT_FLASH_CLUSTER";

void zb_update_builtin_light_flash_red(uint8_t flash_status)
{
    if (connection_status())
    {
        esp_zb_lock_acquire(portMAX_DELAY);
        esp_zb_zcl_status_t state = esp_zb_zcl_set_attribute_val(
            DEVICE_ENDPOINT,
            ESP_ZB_ZCL_CLUSTER_ID_RED_LIGHT_ON_OFF,
            ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
            ESP_ZB_ZCL_ATTR_RED_LIGHT_ON_OFF_ID,
            &flash_status,
            false);
        esp_zb_lock_release();

        /* Check for error */
        if (state != ESP_ZB_ZCL_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Setting builtin light flash RED attribute failed with %x", state);
            return;
        }
        ESP_LOGI(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Set builtin light flash RED attribute success");
        return;
    }
    else
    {
        ESP_LOGW(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Device is not connected! Could not update builtin light flash value");
    }
}

void zb_update_builtin_light_flash_yellow(uint8_t flash_status)
{
    if (connection_status())
    {
        esp_zb_lock_acquire(portMAX_DELAY);
        esp_zb_zcl_status_t state = esp_zb_zcl_set_attribute_val(
            DEVICE_ENDPOINT,
            ESP_ZB_ZCL_CLUSTER_ID_YELLOW_LIGHT_ON_OFF,
            ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
            ESP_ZB_ZCL_ATTR_YELLOW_LIGHT_ON_OFF_ID,
            &flash_status,
            false);
        esp_zb_lock_release();

        /* Check for error */
        if (state != ESP_ZB_ZCL_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Setting builtin light flash YELLOW attribute failed with %x", state);
            return;
        }
        ESP_LOGI(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Set builtin light flash YELLOW attribute success");
        return;
    }
    else
    {
        ESP_LOGW(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Device is not connected! Could not update builtin light flash value");
    }
}

void zb_update_builtin_light_flash_green(uint8_t flash_status)
{
    if (connection_status())
    {
        esp_zb_lock_acquire(portMAX_DELAY);
        esp_zb_zcl_status_t state = esp_zb_zcl_set_attribute_val(
            DEVICE_ENDPOINT,
            ESP_ZB_ZCL_CLUSTER_ID_GREEN_LIGHT_ON_OFF,
            ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
            ESP_ZB_ZCL_ATTR_GREEN_LIGHT_ON_OFF_ID,
            &flash_status,
            false);
        esp_zb_lock_release();

        /* Check for error */
        if (state != ESP_ZB_ZCL_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Setting builtin light flash GREEN attribute failed with %x", state);
            return;
        }
        ESP_LOGI(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Set builtin light flash GREEN attribute success");
        return;
    }
    else
    {
        ESP_LOGW(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Device is not connected! Could not update builtin light flash value");
    }
}

void zb_update_builtin_light_flash_white(uint8_t flash_status)
{
    if (connection_status())
    {
        esp_zb_lock_acquire(portMAX_DELAY);
        esp_zb_zcl_status_t state = esp_zb_zcl_set_attribute_val(
            DEVICE_ENDPOINT,
            ESP_ZB_ZCL_CLUSTER_ID_WHITE_LIGHT_ON_OFF,
            ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
            ESP_ZB_ZCL_ATTR_WHITE_LIGHT_ON_OFF_ID,
            &flash_status,
            false);
        esp_zb_lock_release();

        /* Check for error */
        if (state != ESP_ZB_ZCL_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Setting builtin light flash WHITE attribute failed with %x", state);
            return;
        }
        ESP_LOGI(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Set builtin light flash WHITE attribute success");
        return;
    }
    else
    {
        ESP_LOGW(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Device is not connected! Could not update builtin light flash value");
    }
}

void zb_report_builtin_light_flash_red()
{
    if (connection_status())
    {
        static esp_zb_zcl_report_attr_cmd_t red_light_flash_cmd_req = {};
        red_light_flash_cmd_req.zcl_basic_cmd.src_endpoint = DEVICE_ENDPOINT;
        red_light_flash_cmd_req.address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
        red_light_flash_cmd_req.clusterID = ESP_ZB_ZCL_CLUSTER_ID_RED_LIGHT_ON_OFF;
        red_light_flash_cmd_req.attributeID = ESP_ZB_ZCL_ATTR_RED_LIGHT_ON_OFF_ID;
        red_light_flash_cmd_req.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;
        red_light_flash_cmd_req.manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC;

        /* Request sending new red light flash value */
        esp_zb_lock_acquire(portMAX_DELAY);
        esp_err_t state = esp_zb_zcl_report_attr_cmd_req(&red_light_flash_cmd_req);
        esp_zb_lock_release();

        /* Check for error */
        if (state != ESP_ZB_ZCL_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Report builtin light flash RED attribute report command failed!");
            return;
        }

        ESP_LOGI(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Report builtin light flash RED attribute success");
        return;
    }
    else
    {
        ESP_LOGW(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Device is not connected! Could not report builtin light flash value");
    }
}

void zb_report_builtin_light_flash_yellow()
{
    if (connection_status())
    {
        static esp_zb_zcl_report_attr_cmd_t yellow_light_flash_cmd_req = {};
        yellow_light_flash_cmd_req.zcl_basic_cmd.src_endpoint = DEVICE_ENDPOINT;
        yellow_light_flash_cmd_req.address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
        yellow_light_flash_cmd_req.clusterID = ESP_ZB_ZCL_CLUSTER_ID_YELLOW_LIGHT_ON_OFF;
        yellow_light_flash_cmd_req.attributeID = ESP_ZB_ZCL_ATTR_YELLOW_LIGHT_ON_OFF_ID;
        yellow_light_flash_cmd_req.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;
        yellow_light_flash_cmd_req.manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC;

        /* Request sending new yellow light flash value */
        esp_zb_lock_acquire(portMAX_DELAY);
        esp_err_t state = esp_zb_zcl_report_attr_cmd_req(&yellow_light_flash_cmd_req);
        esp_zb_lock_release();

        /* Check for error */
        if (state != ESP_ZB_ZCL_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Report builtin light flash YELLOW attribute report command failed!");
            return;
        }

        ESP_LOGI(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Report builtin light flash YELLOW attribute success");
        return;
    }
    else
    {
        ESP_LOGW(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Device is not connected! Could not report builtin light flash value");
    }
}

void zb_report_builtin_light_flash_green()
{
    if (connection_status())
    {
        static esp_zb_zcl_report_attr_cmd_t green_light_flash_cmd_req = {};
        green_light_flash_cmd_req.zcl_basic_cmd.src_endpoint = DEVICE_ENDPOINT;
        green_light_flash_cmd_req.address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
        green_light_flash_cmd_req.clusterID = ESP_ZB_ZCL_CLUSTER_ID_GREEN_LIGHT_ON_OFF;
        green_light_flash_cmd_req.attributeID = ESP_ZB_ZCL_ATTR_GREEN_LIGHT_ON_OFF_ID;
        green_light_flash_cmd_req.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;
        green_light_flash_cmd_req.manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC;

        /* Request sending new green light flash value */
        esp_zb_lock_acquire(portMAX_DELAY);
        esp_err_t state = esp_zb_zcl_report_attr_cmd_req(&green_light_flash_cmd_req);
        esp_zb_lock_release();

        /* Check for error */
        if (state != ESP_ZB_ZCL_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Report builtin light flash GREEN attribute report command failed!");
            return;
        }

        ESP_LOGI(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Report builtin light flash GREEN attribute success");
        return;
    }
    else
    {
        ESP_LOGW(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Device is not connected! Could not report builtin light flash value");
    }
}

void zb_report_builtin_light_flash_white()
{
    if (connection_status())
    {
        static esp_zb_zcl_report_attr_cmd_t white_light_flash_cmd_req = {};
        white_light_flash_cmd_req.zcl_basic_cmd.src_endpoint = DEVICE_ENDPOINT;
        white_light_flash_cmd_req.address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
        white_light_flash_cmd_req.clusterID = ESP_ZB_ZCL_CLUSTER_ID_WHITE_LIGHT_ON_OFF;
        white_light_flash_cmd_req.attributeID = ESP_ZB_ZCL_ATTR_WHITE_LIGHT_ON_OFF_ID;
        white_light_flash_cmd_req.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;
        white_light_flash_cmd_req.manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC;

        /* Request sending new white light flash value */
        esp_zb_lock_acquire(portMAX_DELAY);
        esp_err_t state = esp_zb_zcl_report_attr_cmd_req(&white_light_flash_cmd_req);
        esp_zb_lock_release();

        /* Check for error */
        if (state != ESP_ZB_ZCL_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Report builtin light flash WHITE attribute report command failed!");
            return;
        }

        ESP_LOGI(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Report builtin light flash WHITE attribute success");
        return;
    }
    else
    {
        ESP_LOGW(TAG_ZB_UPDATE_BUILTIN_LIGHT, "Device is not connected! Could not report builtin light flash value");
    }
}