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

static const char *TAG_ZB_UPDATE_CUREENT_TIME = "UPDATE_CUREENT_TIME";

void zb_update_current_time(uint32_t current_time)
{
  if (connection_status())
  {
    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_zcl_status_t state = esp_zb_zcl_set_attribute_val(
        DEVICE_ENDPOINT,
        ESP_ZB_ZCL_CLUSTER_ID_TIME,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        ESP_ZB_ZCL_ATTR_TIME_TIME_ID,
        &current_time,
        false);
    esp_zb_lock_release();

    /* Check for error */
    if (state != ESP_ZB_ZCL_STATUS_SUCCESS)
    {
      ESP_LOGE(TAG_ZB_UPDATE_CUREENT_TIME, "Setting current time attribute failed with %x", state);
    }
    else
    {
      ESP_LOGI(TAG_ZB_UPDATE_CUREENT_TIME, "Setting current time attribute success");
    }
  }
  else
  {
    ESP_LOGW(TAG_ZB_UPDATE_CUREENT_TIME, "Device is not connected! Could not update the current time");
  }
}

void zb_update_local_time(uint32_t local_time)
{
  if (connection_status())
  {
    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_zcl_status_t state = esp_zb_zcl_set_attribute_val(
        DEVICE_ENDPOINT,
        ESP_ZB_ZCL_CLUSTER_ID_TIME,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        ESP_ZB_ZCL_ATTR_TIME_LOCAL_TIME_ID,
        &local_time,
        false);
    esp_zb_lock_release();

    /* Check for error */
    if (state != ESP_ZB_ZCL_STATUS_SUCCESS)
    {
      ESP_LOGE(TAG_ZB_UPDATE_CUREENT_TIME, "Setting current local time attribute failed with %x", state);
    }
    else
    {
      ESP_LOGI(TAG_ZB_UPDATE_CUREENT_TIME, "Setting current local time attribute success");
    }
  }
  else
  {
    ESP_LOGW(TAG_ZB_UPDATE_CUREENT_TIME, "Device is not connected! Could not update the local time");
  }
}