#include "zigbee_network.h"

#include "create_cluster.h"
#include "esp_check.h"
#include "esp_log.h"
#include "macros.h"
#include "device.h"

#ifdef LIGHT_SLEEP
#include "light_sleep.h"
#endif

static const char *TAG = "ZIGBEE_NETWORK";

void zigbee_network_task(void *pvParameters)
{
    (void)pvParameters;

#ifdef END_DEVICE
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
    ESP_LOGI(TAG, "Enable END_DEVICE");
#endif
#ifdef ROUTER_DEVICE
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZR_CONFIG();
    ESP_LOGI(TAG, "Enable ROUTER_DEVICE");
#endif

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

    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();

    create_basic_cluster(cluster_list);
    create_identify_cluster(cluster_list);
    create_time_cluster(cluster_list);
#if defined DHT22 || defined BME680
    create_hum_cluster(cluster_list);
    ESP_LOGI(TAG, "Create SENSOR_HUMIDITY Cluster");
    create_temp_cluster(cluster_list);
    ESP_LOGI(TAG, "Create SENSOR_TEMPERATURE Cluster");
#endif
#if defined BME680
    create_gas_resistance_cluster(cluster_list);
    ESP_LOGI(TAG, "Create SENSOR_GAS Cluster");
    create_iaq_cluster(cluster_list);
    ESP_LOGI(TAG, "Create IAQ Cluster");
    create_iaq_accuracy_cluster(cluster_list);
    ESP_LOGI(TAG, "Create IAQ Accuracy Cluster");
    create_co2_cluster(cluster_list);
    ESP_LOGI(TAG, "Create CO2 Cluster");
    create_bvoc_cluster(cluster_list);
    ESP_LOGI(TAG, "Create BVOC Cluster");
    create_pressure_cluster(cluster_list);
    ESP_LOGI(TAG, "Create SENSOR_PRESSURE Cluster");
#endif
#ifdef WATERLEAK_FEATURES
    create_waterleak_cluster(cluster_list);
    ESP_LOGI(TAG, "Create WATERLEAK Cluster");
#endif
#ifdef BATTERY_FEATURES
    create_battery_cluster(cluster_list);
    ESP_LOGI(TAG, "Create BATTERY Cluster");
#endif
#ifdef OTA_UPDATE
    create_ota_cluster(cluster_list);
    ESP_LOGI(TAG, "Create OTA_UPDATE Cluster");
#endif
#ifdef SWITCH_FEATURES
    create_light_switch_cluster(cluster_list);
    ESP_LOGI(TAG, "Create LIGHT_SWITCH Cluster");
#endif
#if defined IRRIGATION_FEATURES
    create_irrigation_cluster(cluster_list);
    ESP_LOGI(TAG, "Create IRRIGATION_CONTROL Cluster");
#endif
#ifdef BUILTIN_LIGHT
    create_builtin_light_red(cluster_list);
    create_builtin_light_yellow(cluster_list);
    create_builtin_light_green(cluster_list);
    create_builtin_light_white(cluster_list);
#endif

    esp_zb_ep_list_t *endpoint_list = esp_zb_ep_list_create();
    esp_zb_endpoint_config_t endpoint_config = {
        .endpoint = DEVICE_ENDPOINT,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = ESP_ZB_HA_TEST_DEVICE_ID,
        .app_device_version = 0};
    esp_zb_ep_list_add_ep(endpoint_list, cluster_list, endpoint_config);

    esp_zb_device_register(endpoint_list);
    esp_zb_core_action_handler_register(zb_action_handler);
    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);
    ESP_ERROR_CHECK(esp_zb_start(false));
    esp_zb_stack_main_loop();
}
