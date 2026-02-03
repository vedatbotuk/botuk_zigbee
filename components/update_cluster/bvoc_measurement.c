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

static const char *TAG_ZB_UPDATE_BVOC = "UPDATE_BVOC_CLUSTER";

void zb_update_bvoc(float bvoc)
{
    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_zcl_status_t state = esp_zb_zcl_set_attribute_val(
        DEVICE_ENDPOINT,
        ESP_ZB_ZCL_CLUSTER_ID_VOC_MEASUREMENT,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        ESP_ZB_ZCL_ATTR_VOC_MEASUREMENT_MEASURED_VALUE_ID,
        &bvoc,
        false);
    esp_zb_lock_release();

    /* Check for error */
    if (state != ESP_ZB_ZCL_STATUS_SUCCESS)
    {
        ESP_LOGE(TAG_ZB_UPDATE_BVOC, "Setting BVOC attribute failed with %x", state);
        return;
    }
    ESP_LOGI(TAG_ZB_UPDATE_BVOC, "Set BVOC attribute success");
    return;
}

void zb_report_bvoc()
{
    static esp_zb_zcl_report_attr_cmd_t bvoc_measurement_cmd_req = {};
    bvoc_measurement_cmd_req.zcl_basic_cmd.src_endpoint = DEVICE_ENDPOINT;
    bvoc_measurement_cmd_req.address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
    bvoc_measurement_cmd_req.clusterID = ESP_ZB_ZCL_CLUSTER_ID_VOC_MEASUREMENT;
    bvoc_measurement_cmd_req.attributeID = ESP_ZB_ZCL_ATTR_VOC_MEASUREMENT_MEASURED_VALUE_ID;
    bvoc_measurement_cmd_req.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;
    bvoc_measurement_cmd_req.manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC;

    /* Request sending new BVOC value */
    esp_zb_lock_acquire(portMAX_DELAY);
    esp_err_t state = esp_zb_zcl_report_attr_cmd_req(&bvoc_measurement_cmd_req);
    esp_zb_lock_release();

    /* Check for error */
    if (state != ESP_ZB_ZCL_STATUS_SUCCESS)
    {
        ESP_LOGE(TAG_ZB_UPDATE_BVOC, "Report BVOC attribute report command failed!");
        return;
    }

    ESP_LOGI(TAG_ZB_UPDATE_BVOC, "Report BVOC attribute success");
    return;
}
