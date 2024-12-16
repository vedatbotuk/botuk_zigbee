idf_component_register(
    SRCS battery_measurement.c humidity_measurement.c temp_measurement.c waterleak_check.c current_time.c
    INCLUDE_DIRS include
    PRIV_REQUIRES espressif__esp-zigbee-lib driver
)
