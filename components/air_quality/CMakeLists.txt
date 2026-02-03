idf_component_register(SRCS "air_quality.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES bme68x bsec update_cluster espressif__esp-zigbee-lib macros random_utils esp_timer nvs_flash espressif__esp-zigbee-lib
                       )
