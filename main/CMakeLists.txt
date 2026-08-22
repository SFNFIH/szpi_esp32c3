if(DEFINED ENV{ESP_MATTER_PATH})
    set(MAIN_SRCS
        "app_main.cpp"
        "app_driver.cpp"
        "app_reset.cpp"
    )
    set(MAIN_PRIV_INCLUDES "." "${ESP_MATTER_PATH}/examples/common/utils")

    idf_component_register(SRCS ${MAIN_SRCS}
                        INCLUDE_DIRS "."
                        PRIV_INCLUDE_DIRS ${MAIN_PRIV_INCLUDES}
                        REQUIRES board szpi_drivers nvs_flash esp_matter
                        PRIV_REQUIRES esp_timer)

    target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
else()
    idf_component_register(SRCS "main.c"
                        INCLUDE_DIRS "."
                        REQUIRES board szpi_drivers)
endif()
