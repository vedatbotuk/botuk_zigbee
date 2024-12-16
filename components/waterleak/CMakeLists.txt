idf_component_register(
    SRCS "waterleak.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES driver update_cluster espressif__esp-zigbee-lib update_cluster random_utils
    )
