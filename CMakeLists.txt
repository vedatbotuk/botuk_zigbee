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
    add_definitions(-DHW_VERSION=${HW_VERSION})
    add_definitions(-DDEVICE_ENDPOINT=${DEVICE_ENDPOINT})
    add_definitions(-DED_KEEP_ALIVE=${ED_KEEP_ALIVE})
    if(TESTING EQUAL "1")
        add_definitions(-DTESTING=${TESTING})
        add_definitions(-DZIGBEE_CHANNEL=${SIMULATE_ZIGBEE_CHANNEL})
    else()
        add_definitions(-DZIGBEE_CHANNEL=${ZIGBEE_CHANNEL})
    endif()
    
    if(SIMULATE EQUAL "1")
        add_definitions(-DSIMULATE=${SIMULATE})
        message(STATUS "Simulating active, dummy payload will be created.")               
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
add_definitions(-DMODEL_ID_MAP=${MODEL_ID_FORMATED}_${HW_VERSION})
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
string(SUBSTRING ${MODEL_ID} 4 1 FIFTH_NUM)
if(SEVENTH_NUM EQUAL "1" OR FIFTH_NUM EQUAL "1")
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
set(FILE_NAME "${MODEL_ID_FORMATED}_${HW_VERSION}_${VERSION}_${DEV_PROD}")
project(${FILE_NAME})

#### ###################### ####
#### Create OTA-Update File ####

# Check the second number of MODEL_ID
string(SUBSTRING ${MODEL_ID} 1 1 SECOND_NUM)

if(SECOND_NUM STREQUAL "1")
    message(STATUS "OTA is enabled for this model. Creating OTA-Update file after build.")

    set(MY_DIR ${CMAKE_SOURCE_DIR}/ota)
    file(MAKE_DIRECTORY ${MY_DIR})
    file(MAKE_DIRECTORY ${MY_DIR}/patches)

    set(IMAGES_DIR ${CMAKE_SOURCE_DIR}/botuk-zigbee-images)

    # --------------------------------------------------
    # Determine environment (dev vs prod) from DEV_PROD
    # --------------------------------------------------
    string(TOLOWER "${DEV_PROD}" ENV_SUFFIX)
    if(NOT ENV_SUFFIX)
        set(ENV_SUFFIX "dev") # Fallback if DEV_PROD is not set
        message(WARNING "DEV_PROD is not set! Defaulting to 'dev' binaries.")
    endif()

    # --------------------------------------------------
    # Get version folders (verX.YYY_*)
    # --------------------------------------------------
    file(GLOB VERSION_DIRS RELATIVE ${IMAGES_DIR} "${IMAGES_DIR}/ver*")

    set(SORTED_VERSION_DIRS)

    foreach(DIR ${VERSION_DIRS})
        # Extract version number (e.g., 0.103)
        string(REGEX REPLACE "^ver([0-9]+\\.[0-9]+)_.*" "\\1" VERSION_NUM ${DIR})

        # Pad version for correct sorting (0.103 -> 000103)
        string(REPLACE "." "" VERSION_NUM_PADDED ${VERSION_NUM})
        string(LENGTH ${VERSION_NUM_PADDED} LEN)
        math(EXPR PAD_LEN "6 - ${LEN}")

        set(PADDING "")
        if(PAD_LEN GREATER 0)
            foreach(i RANGE 1 ${PAD_LEN})
                set(PADDING "${PADDING}0")
            endforeach()
        endif()

        set(SORT_KEY "${PADDING}${VERSION_NUM_PADDED}")

        # Store as "key|value" to prevent CMake from flattening the list
        list(APPEND SORTED_VERSION_DIRS "${SORT_KEY}|${DIR}")
    endforeach()

    # Sort by key (string sort works now)
    list(SORT SORTED_VERSION_DIRS)
    list(REVERSE SORTED_VERSION_DIRS)

    # Extract only directory names
    set(VERSION_DIRS)

    foreach(ITEM ${SORTED_VERSION_DIRS})
        # ITEM is "key|dir". Replace "|" with ";" to turn it into a 2-element CMake list
        string(REPLACE "|" ";" ITEM_LIST "${ITEM}")
        list(GET ITEM_LIST 1 DIR_NAME)
        list(APPEND VERSION_DIRS ${DIR_NAME})
    endforeach()

    # Take last 5 versions
    list(LENGTH VERSION_DIRS TOTAL_VERSIONS)
    if(TOTAL_VERSIONS GREATER 5)
        list(SUBLIST VERSION_DIRS 0 5 VERSION_DIRS)
    endif()

    message(STATUS "Using base versions: ${VERSION_DIRS}")

    # 1. Extract the NEW version info once outside the loop
    string(REPLACE "_" ";" NEW_PARTS "${FILE_NAME}")
    list(GET NEW_PARTS 2 NEW_VER)

    set(NEW_BINARY ${CMAKE_BINARY_DIR}/${FILE_NAME}.bin)

    # --------------------------------------------------
    # Loop over base versions
    # --------------------------------------------------
    foreach(VERSION_DIR ${VERSION_DIRS})

        set(FULL_PATH ${IMAGES_DIR}/${VERSION_DIR})
        
        # Grab specifically the dev or prod binary based on CMake variable
        file(GLOB BIN_FILES "${FULL_PATH}/*_${ENV_SUFFIX}.bin")

        list(LENGTH BIN_FILES BIN_COUNT)
        if(BIN_COUNT GREATER 0)
            list(GET BIN_FILES 0 BASE_BINARY)
            get_filename_component(BASE_NAME ${BASE_BINARY} NAME_WE)

            # 2. Extract BASE version info immediately
            string(REPLACE "_" ";" BASE_PARTS "${BASE_NAME}")
            list(GET BASE_PARTS 0 PREFIX_1)
            list(GET BASE_PARTS 1 PREFIX_2)
            list(GET BASE_PARTS 2 BASE_VER)
            list(GET BASE_PARTS 3 SUFFIX)

            # 3. ONLY proceed if the versions are different
            if(NOT BASE_VER STREQUAL NEW_VER)
                message(STATUS "Creating delta: ${BASE_VER} -> ${NEW_VER} (${ENV_SUFFIX})")

                set(PATCH_FILE ${MY_DIR}/patches/${FILE_NAME}_from_${BASE_NAME}.patch)

                add_custom_target(run_delta_ota_patch_gen_${BASE_NAME} ALL
                    COMMAND ${CMAKE_COMMAND} -E echo "Generating patch from ${BASE_NAME}"
                    COMMAND ${CMAKE_COMMAND} -E env python3 ${CMAKE_SOURCE_DIR}/scripts/esp_delta_ota_patch_gen.py
                        create_patch
                        --chip esp32-h2
                        --base_binary ${BASE_BINARY}
                        --new_binary ${NEW_BINARY}
                        --patch_file_name ${PATCH_FILE}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "Generating Delta OTA patch from ${BASE_NAME}"
                )

                if(TARGET gen_project_binary)
                    add_dependencies(run_delta_ota_patch_gen_${BASE_NAME} gen_project_binary)
                else()
                    add_dependencies(run_delta_ota_patch_gen_${BASE_NAME} ${FILE_NAME}.elf)
                endif()

                # Construct output target name
                set(CUSTOM_OTA_NAME "${PREFIX_1}_${PREFIX_2}_${BASE_VER}_to_${NEW_VER}_${SUFFIX}.ota")
                set(OTA_OUTPUT ${MY_DIR}/${CUSTOM_OTA_NAME})

                add_custom_target(run_post_build_${BASE_NAME} ALL
                    COMMAND ${CMAKE_COMMAND} -E env python3 ${CMAKE_SOURCE_DIR}/scripts/create_ota.py
                        -m ${MANUFACTURER}
                        -i ${MODEL_ID_DECIMAL}
                        -v ${VERSION}
                        ${PATCH_FILE}
                        > ${OTA_OUTPUT}                 
                )

                add_dependencies(run_post_build_${BASE_NAME} run_delta_ota_patch_gen_${BASE_NAME})
            else()
                message(STATUS "Skipping OTA generation: Base version (${BASE_VER}) is the same as New version.")
            endif()

        else()
            message(WARNING "No _${ENV_SUFFIX}.bin file found in ${FULL_PATH}. Skipping OTA generation.")
        endif()

    endforeach()

else()
    message(STATUS "OTA is not enabled for this model. No OTA-Update file will be created.")
endif()
#### Create OTA-Update File ####
#### ###################### ####

#### ########################################## ####
#### Copy Binary to botuk-zigbee-images Repo    ####

# Define the destination directory based on your version variables
set(IMAGE_DEST_DIR "${CMAKE_SOURCE_DIR}/botuk-zigbee-images/${FIRMWARE_VERSION}_${VERSION}")

# Add a post-build command to create the directory and copy the binary
add_custom_command(
    TARGET gen_project_binary POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${IMAGE_DEST_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/${FILE_NAME}.bin ${IMAGE_DEST_DIR}/${FILE_NAME}.bin
    COMMENT "Copying ${FILE_NAME}.bin to ${IMAGE_DEST_DIR}"
)
#### ########################################## ####