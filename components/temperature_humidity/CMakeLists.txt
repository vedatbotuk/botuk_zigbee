 idf_component_register(
    SRCS "temperature_humidity.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES driver update_cluster espressif__esp-zigbee-lib update_cluster dht22 macros random_utils
    )
