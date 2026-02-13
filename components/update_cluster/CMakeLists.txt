idf_component_register(
    SRCS battery_measurement.c humidity_measurement.c temp_measurement.c waterleak_check.c current_time.c co2_measurement.c iaq_measurement.c bvoc_measurement.c gas_resistance_measurement.c builtin_light_flash.c
    INCLUDE_DIRS include
    PRIV_REQUIRES espressif__esp-zigbee-lib driver create_cluster
)
