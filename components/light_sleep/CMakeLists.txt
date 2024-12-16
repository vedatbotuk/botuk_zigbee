idf_component_register(
    SRCS light_sleep.c
    INCLUDE_DIRS include
    PRIV_REQUIRES nvs_flash esp_timer driver espressif__esp-zigbee-lib
)
