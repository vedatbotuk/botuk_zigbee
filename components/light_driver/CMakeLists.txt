idf_component_register(SRC_DIRS "src"
                       INCLUDE_DIRS "include"
                       REQUIRES
                       led_strip driver espressif__esp-zigbee-lib
)