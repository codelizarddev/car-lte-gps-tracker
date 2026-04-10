idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        modem
        gnss
        mqtt_wrapper
        relay
        power_monitor
        nvs_flash
)
