# 1. Register the component first. 
# This tells IDF about dependencies during the "Script" phase.
idf_component_register(INCLUDE_DIRS "include"
                       REQUIRES bme68x)

# 2. Only define the prebuilt library during the actual project generation.
# We check if the target for this component exists yet.
if(TARGET ${COMPONENT_LIB})
    add_prebuilt_library(libalgobsec "lib/libalgobsec.a"
                        PRIV_REQUIRES bme68x esp_timer)
    
    # Link it to the component (use INTERFACE if you have no SRCS)
    target_link_libraries(${COMPONENT_LIB} INTERFACE libalgobsec)
endif()
