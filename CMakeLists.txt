cmake_minimum_required(VERSION 3.16)

if(DEFINED ENV{ESP_MATTER_PATH})
    if(NOT DEFINED ENV{ESP_MATTER_DEVICE_PATH})
        if("${IDF_TARGET}" STREQUAL "esp32c3" OR "${IDF_TARGET}" STREQUAL "")
            set(ENV{ESP_MATTER_DEVICE_PATH} $ENV{ESP_MATTER_PATH}/device_hal/device/esp32c3_devkit_m)
        else()
            message(FATAL_ERROR "Matter build on this repo currently supports esp32c3 only")
        endif()
    endif()

    set(ESP_MATTER_PATH $ENV{ESP_MATTER_PATH})
    set(MATTER_SDK_PATH ${ESP_MATTER_PATH}/connectedhomeip/connectedhomeip)

    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    include(${ESP_MATTER_PATH}/examples/common/cmake_common/components_include.cmake)
    include($ENV{ESP_MATTER_DEVICE_PATH}/esp_matter_device.cmake)

    set(EXTRA_COMPONENT_DIRS
        "${ESP_MATTER_PATH}/examples/common"
        "${MATTER_SDK_PATH}/config/esp32/components"
        "${ESP_MATTER_PATH}/components"
        "${ESP_MATTER_PATH}/device_hal/device"
        ${extra_components_dirs_append})

    set(SDKCONFIG_DEFAULTS "sdkconfig.defaults;sdkconfig.defaults.matter")

    project(szpi_esp32c3)

    idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++17;-Os;-DCHIP_HAVE_CONFIG_H;-Wno-overloaded-virtual" APPEND)
    idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
    idf_build_set_property(COMPILE_OPTIONS "-Wno-format-nonliteral;-Wno-format-security" APPEND)
else()
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(szpi_esp32c3)
endif()
