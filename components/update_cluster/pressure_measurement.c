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
#include "update_cluster.h"

static const char *TAG_ZB_UPDATE_PRESSURE = "UPDATE_PRESSURE_CLUSTER";

void zb_update_pressure(int16_t pressure)
{
    if (connection_status())
    {
        esp_zb_lock_acquire(portMAX_DELAY);
        esp_zb_zcl_status_t state = esp_zb_zcl_set_attribute_val(
            DEVICE_ENDPOINT,
            ESP_ZB_ZCL_CLUSTER_ID_PRESSURE_MEASUREMENT,
            ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
            ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_VALUE_ID,
            &pressure,
            false);
        esp_zb_lock_release();

        /* Check for error */
        if (state != ESP_ZB_ZCL_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG_ZB_UPDATE_PRESSURE, "Setting pressure attribute failed with %x", state);
            return;
        }
        ESP_LOGI(TAG_ZB_UPDATE_PRESSURE, "Set pressure attribute success");
        return;
    }
    else
    {
        ESP_LOGW(TAG_ZB_UPDATE_PRESSURE, "Device is not connected! Could not measure the pressure level");
    }
}

void zb_report_pressure()
{
    if (connection_status())
    {
        static esp_zb_zcl_report_attr_cmd_t pressure_measurement_cmd_req = {};
        pressure_measurement_cmd_req.zcl_basic_cmd.src_endpoint = DEVICE_ENDPOINT;
        pressure_measurement_cmd_req.address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
        pressure_measurement_cmd_req.clusterID = ESP_ZB_ZCL_CLUSTER_ID_PRESSURE_MEASUREMENT;
        pressure_measurement_cmd_req.attributeID = ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_VALUE_ID;
        pressure_measurement_cmd_req.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;
        pressure_measurement_cmd_req.manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC;

        /* Request sending new phase voltage */
        esp_zb_lock_acquire(portMAX_DELAY);
        esp_err_t state = esp_zb_zcl_report_attr_cmd_req(&pressure_measurement_cmd_req);
        esp_zb_lock_release();

        /* Check for error */
        if (state != ESP_ZB_ZCL_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG_ZB_UPDATE_PRESSURE, "Report pressure attribute report command failed!");
            return;
        }

        ESP_LOGI(TAG_ZB_UPDATE_PRESSURE, "Report pressure attribute success");
        return;
    }
    else
    {
        ESP_LOGW(TAG_ZB_UPDATE_PRESSURE, "Device is not connected! Could not report the pressure level");
    }
}