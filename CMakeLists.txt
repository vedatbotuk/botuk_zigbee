# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/components
    )

# Path to the settings file
set(SETTINGS_FILE ${CMAKE_SOURCE_DIR}/settings.conf)

# Ensure that changes in settings.conf trigger a reconfiguration
configure_file(${SETTINGS_FILE} ${CMAKE_BINARY_DIR}/settings.conf COPYONLY)

# Function to read the settings file and extract values
function(parse_settings key result)
    file(STRINGS ${SETTINGS_FILE} content)

    # Iterate through each line of the settings file
    foreach(line IN LISTS content)
        # Check if the line starts with the requested key
        if("${line}" MATCHES "^${key}=")
            string(REPLACE "${key}=" "" value "${line}")
            set(${result} "${value}" PARENT_SCOPE)
        endif()
    endforeach()
endfunction()

# Check if the settings file exists
if(EXISTS ${SETTINGS_FILE})
    # Extract specific settings from the file
    parse_settings("FIRMWARE_VERSION" FIRMWARE_VERSION)
    parse_settings("DEV_PROD" DEV_PROD)
    parse_settings("MODEL_ID" MODEL_ID)
    parse_settings("DEVICE_ENDPOINT" DEVICE_ENDPOINT)
    parse_settings("ED_KEEP_ALIVE" ED_KEEP_ALIVE)
    parse_settings("HW_VERSION" HW_VERSION)
    parse_settings("VERSION" VERSION)
    parse_settings("MANUFACTURER" MANUFACTURER)
    parse_settings("ZIGBEE_CHANNEL" ZIGBEE_CHANNEL)
    parse_settings("DEEP_SLEEP_TIME" DEEP_SLEEP_TIME)
    parse_settings("TESTING" TESTING)
    parse_settings("SIMULATE" SIMULATE)
    parse_settings("SIMULATE_ZIGBEE_CHANNEL" SIMULATE_ZIGBEE_CHANNEL)

    # Print the values for debugging
    message(STATUS "FIRMWARE_VERSION: ${FIRMWARE_VERSION}")
    message(STATUS "DEV_PROD: ${DEV_PROD}")
    message(STATUS "MODEL_ID: ${MODEL_ID}")
    message(STATUS "DEVICE_ENDPOINT: ${DEVICE_ENDPOINT}")
    message(STATUS "ED_KEEP_ALIVE: ${ED_KEEP_ALIVE}")
    message(STATUS "HW_VERSION: ${HW_VERSION}")
    message(STATUS "VERSION: ${VERSION}")
    message(STATUS "MANUFACTURER: ${MANUFACTURER}")
    message(STATUS "ZIGBEE_CHANNEL: ${ZIGBEE_CHANNEL}")
    message(STATUS "DEEP_SLEEP_TIME: ${DEEP_SLEEP_TIME}")
    message(STATUS "TESTING: ${TESTING}")
    message(STATUS "SIMULATE: ${SIMULATE}")
    message(STATUS "SIMULATE_ZIGBEE_CHANNEL: ${SIMULATE_ZIGBEE_CHANNEL}")

    # Add other settings as preprocessor definitions
    add_definitions(-DFIRMWARE_VERSION="${FIRMWARE_VERSION}")
    add_definitions(-DDEVICE_ENDPOINT=${DEVICE_ENDPOINT})
    add_definitions(-DED_KEEP_ALIVE=${ED_KEEP_ALIVE})
    if(TESTING EQUAL "1")
        if(SIMULATE EQUAL "1")
            add_definitions(-DSIMULATE=${SIMULATE})
            message(STATUS "Simulating active, dummy payload will be created.")               
        endif()
        add_definitions(-DZIGBEE_CHANNEL=${SIMULATE_ZIGBEE_CHANNEL})
    else()
        add_definitions(-DZIGBEE_CHANNEL=${ZIGBEE_CHANNEL})
    endif()


    add_definitions(-DDEEP_SLEEP_TIME=${DEEP_SLEEP_TIME})

else()
    message(FATAL_ERROR "Settings file not found: ${SETTINGS_FILE}")
endif()

#### ######## ####
#### Get Time ####
# Get time for FIRMWARE_DATE
string(TIMESTAMP CURRENT_DATE "%Y%m%d")
message(STATUS "Current date: ${CURRENT_DATE}")
add_definitions(-DCURRENT_DATE="${CURRENT_DATE}")



#### ######### ####
#### FUNKTIONS ####
# Funktion zur Konvertierung einer Binärzahl in eine Dezimalzahl
function(binary_to_decimal binary_number result)
    set(decimal_value 0)
    set(base 1)

    # Binärzahl von rechts nach links verarbeiten
    string(LENGTH "${binary_number}" binary_length)
    math(EXPR last_index "${binary_length} - 1")
    
    foreach(index RANGE ${last_index} 0 -1)
        string(SUBSTRING "${binary_number}" ${index} 1 digit)
        if(digit EQUAL "1")
            math(EXPR decimal_value "${decimal_value} + ${base}")
        endif()
        math(EXPR base "${base} * 2")
    endforeach()

    # Ergebnis zurückgeben
    set(${result} "${decimal_value}" PARENT_SCOPE)
endfunction()

# Funktion zur Formatierung des Ergebnisses auf eine feste Länge
function(format_decimal decimal_number length result)
    string(LENGTH "${decimal_number}" decimal_length)
    if(decimal_length LESS length)
        math(EXPR zeros_to_add "${length} - ${decimal_length}")
        string(REPEAT "0" ${zeros_to_add} leading_zeros)
        set(formatted_result "${leading_zeros}${decimal_number}")
    else()
        set(formatted_result "${decimal_number}")
    endif()

    # Ergebnis zurückgeben
    set(${result} "${formatted_result}" PARENT_SCOPE)
endfunction()
#### FUNKTIONS ####
#### ######### ####

#### Convert Macros ####
#### ############## ####
add_definitions(-DSENSOR_MAP=0b${MODEL_ID})
binary_to_decimal(${MODEL_ID} MODEL_ID_DECIMAL)
format_decimal(${MODEL_ID_DECIMAL} 5 MODEL_ID_FORMATED)
add_definitions(-DMODEL_ID_MAP=${MODEL_ID_FORMATED})
add_definitions(-DDEVICE_ENDPOINT=${DEVICE_ENDPOINT})
#### Convert Macros ####
#### ############## ####

#### Check MODEL_ID ####
#### #################### ####
# Sixth and Seventh bit cannot be at the same time 1.
# Both should be 0 or one of them should be 1.
string(SUBSTRING ${MODEL_ID} 5 1 SIXTH_BIT)
string(SUBSTRING ${MODEL_ID} 6 1 SEVENTH_BIT)
if(SIXTH_BIT EQUAL "1" AND SEVENTH_BIT EQUAL "1")
    message(FATAL_ERROR "Sixth and Seventh bits cannot both be 1 at the same time in MODEL_ID. Please check the MODEL_ID and correct it.")
endif()

string(SUBSTRING ${MODEL_ID} 0 1 FIRST_BIT)
string(SUBSTRING ${MODEL_ID} 5 1 SIXTH_BIT)
if(FIRST_BIT EQUAL "0" AND SIXTH_BIT EQUAL "1")
    message(FATAL_ERROR "First bit cannot be 0 while Sixth bit is 1 in MODEL_ID. Please check the MODEL_ID and correct it.")
endif()
#### Check MODEL_ID ####
#### #################### ####

#### Set specific sdkconfig ####
#### ###################### ####
# Set the default configurations for the device
set(SET_SDKCONFIG_DEFAULTS "sdkconfig_defaults/sdkconfig.main")
message(STATUS "Setting SET_SDKCONFIG_DEFAULTS to sdkconfig.main")

# Check the first number of MODEL_ID
string(SUBSTRING ${MODEL_ID} 0 1 FIRST_NUM)
if(FIRST_NUM EQUAL "1")
    list(APPEND SET_SDKCONFIG_DEFAULTS "sdkconfig_defaults/sdkconfig.end_device")
    message(STATUS "Appending SET_SDKCONFIG_DEFAULTS to sdkconfig.end_device")
else()
    list(APPEND SET_SDKCONFIG_DEFAULTS "sdkconfig_defaults/sdkconfig.router")
    message(STATUS "Appending SET_SDKCONFIG_DEFAULTS to sdkconfig.router")
endif()

# Check the sixth number of MODEL_ID
string(SUBSTRING ${MODEL_ID} 5 1 SIXTH_NUM)
if(SIXTH_NUM EQUAL "1")
    list(APPEND SET_SDKCONFIG_DEFAULTS "sdkconfig_defaults/sdkconfig.light_sleep")
    message(STATUS "Appending sdkconfig.light_sleep to SET_SDKCONFIG_DEFAULTS")

    # Check the tenth number of MODEL_ID
    string(SUBSTRING ${MODEL_ID} 9 1 TENTH_NUM)
    if(TENTH_NUM EQUAL "1")
        list(APPEND SET_SDKCONFIG_DEFAULTS "sdkconfig_defaults/sdkconfig.light_sleep_gpio_on")
        message(STATUS "Appending sdkconfig.light_sleep_gpio_on to SET_SDKCONFIG_DEFAULTS")
    else()
        list(APPEND SET_SDKCONFIG_DEFAULTS "sdkconfig_defaults/sdkconfig.light_sleep_gpio_off")
        message(STATUS "Appending sdkconfig.light_sleep_gpio_off to SET_SDKCONFIG_DEFAULTS")
    endif()
endif()

# Check the seventh number of MODEL_ID
string(SUBSTRING ${MODEL_ID} 6 1 SEVENTH_NUM)
if(SEVENTH_NUM EQUAL "1")
    list(APPEND SET_SDKCONFIG_DEFAULTS "sdkconfig_defaults/sdkconfig.deep_sleep")
    message(STATUS "Appending sdkconfig.deep_sleep to SET_SDKCONFIG_DEFAULTS")
endif()

# Check production-specific configurations to the file for prod devices
if(DEV_PROD STREQUAL "prod")
    list(APPEND SET_SDKCONFIG_DEFAULTS "sdkconfig_defaults/sdkconfig.prod")
    message(STATUS "Appending sdkconfig.prod to SET_SDKCONFIG_DEFAULTS")
    add_compile_options(-msave-restore)
    message(STATUS "Added -msave-restore to compile options")
endif()

# Create a new file with the name "sdkconfig.defaults"
set(SDKCONFIG_DEVICE_FILE "${CMAKE_SOURCE_DIR}/sdkconfig.defaults")

# Write the configurations to the file
file(WRITE ${SDKCONFIG_DEVICE_FILE} 
"#
# Automatically generated file; DO NOT EDIT.
# Espressif IoT Development Framework (ESP-IDF) Configuration
#
")

# Append each SET_SDKCONFIG_DEFAULTS file to the sdkconfig.defaults file
foreach(SDKCONFIG_DEFAULTS ${SET_SDKCONFIG_DEFAULTS})
    file(READ ${SDKCONFIG_DEFAULTS} SDKCONFIG_CONTENT)
    file(APPEND ${SDKCONFIG_DEVICE_FILE} "\n###\n### Contents of ${SDKCONFIG_DEFAULTS}\n${SDKCONFIG_CONTENT}")
endforeach()

message(STATUS "Created sdkconfig.defaults with default configurations")
#### Set specific sdkconfig ####
#### ###################### ####

#### OTA MACROS ####
#### ########## ####
add_definitions(-DESP_OTA_CLIENT_ENDPOINT=${DEVICE_ENDPOINT})
add_definitions(-DOTA_UPGRADE_HW_VERSION=${HW_VERSION})
add_definitions(-DOTA_UPGRADE_DOWNLOADED_FILE_VERSION=${VERSION})
add_definitions(-DOTA_UPGRADE_RUNNING_FILE_VERSION=${VERSION})
add_definitions(-DOTA_UPGRADE_IMAGE_TYPE=${MODEL_ID_DECIMAL})
add_definitions(-DOTA_UPGRADE_MANUFACTURER=${MANUFACTURER})
#### OTA MACROS ####
#### ########## ####

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(FILE_NAME "device_${MODEL_ID_FORMATED}_v${VERSION}${DEV_PROD}")
project(${FILE_NAME})

#### ###################### ####
#### Create OTA-Update File ####
# Check the seccond number of MODEL_ID
string(SUBSTRING ${MODEL_ID} 1 1 SECOND_NUM)
if(SECOND_NUM EQUAL "1")
    message(STATUS "OTA is not enabled, won't create OTA file.")
    set(MY_DIR ${CMAKE_SOURCE_DIR}/ota)
    file(MAKE_DIRECTORY ${MY_DIR})

    set(PYTHON_SCRIPT ${CMAKE_SOURCE_DIR}/scripts/create_ota.py)
    add_custom_target(RunPostBuildScript ALL
        COMMAND ${CMAKE_COMMAND} -E echo  "Running Python script..."
        COMMAND ${CMAKE_COMMAND} -E env python3 ${PYTHON_SCRIPT} -m ${MANUFACTURER} -i ${MODEL_ID_DECIMAL} -v ${VERSION} ${CMAKE_BINARY_DIR}/${FILE_NAME}.bin > ${CMAKE_SOURCE_DIR}/ota/${FILE_NAME}.ota
    )
    add_dependencies(RunPostBuildScript ${FILE_NAME}.elf)
endif()
#### Create OTA-Update File ####
#### ###################### ####
