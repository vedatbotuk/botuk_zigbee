cmake_minimum_required(VERSION 3.16)
project(random_utils_tests C)

enable_testing()

add_executable(test_random_utils
    test_random_utils.c
    ../components/random_utils/random_utils.c
)
target_include_directories(test_random_utils PRIVATE
    stubs
    ../components/random_utils/include
)

add_executable(test_air_quality
    test_air_quality.c
    ../components/air_quality/air_quality.c
)
target_include_directories(test_air_quality PRIVATE
    stubs
    ../components/air_quality/include
    ../components/update_cluster/include
    ../components/macros
    ../components/random_utils/include
)
target_compile_definitions(test_air_quality PRIVATE TESTING=1)
add_test(NAME air_quality COMMAND test_air_quality)

add_executable(test_battery
    test_battery.c
    ../components/battery/battery_read.c
)
target_include_directories(test_battery PRIVATE
    stubs
    ../components/battery/include
    ../components/update_cluster/include
    ../components/random_utils/include
    ../components/macros
)
target_compile_definitions(test_battery PRIVATE SIMULATE=1 DEEP_SLEEP=1)
add_test(NAME battery COMMAND test_battery)

function(add_feature_map_test target sensor_map test_case)
    add_executable(${target} test_macros.c)
    target_include_directories(${target} PRIVATE ../components/macros)
    target_compile_definitions(${target} PRIVATE
        SENSOR_MAP=${sensor_map}
        EXPECTED_CASE=${test_case}
    )
    add_test(NAME ${target} COMMAND ${target})
endfunction()

add_feature_map_test(test_macros_router 0 0)
add_feature_map_test(test_macros_mixed_features 33120 1)
add_feature_map_test(test_macros_end_device_ota 49152 2)
add_feature_map_test(test_macros_all_features 65504 3)