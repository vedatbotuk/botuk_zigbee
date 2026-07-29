#include "macros.h"
#include "create_cluster.h"
#include "esp_log.h"
#include "esp_zigbee_core.h"

static const char *TAG = "ZIGBEE_CLUSTERS";

void register_device_clusters(esp_zb_cluster_list_t *cluster_list)
{
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
}
