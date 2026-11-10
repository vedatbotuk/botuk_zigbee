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

add_test(NAME random_utils COMMAND test_random_utils)

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