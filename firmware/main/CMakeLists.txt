idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    PRIV_INCLUDE_DIRS "../config"
    REQUIRES
        modem
        gnss
        mqtt_wrapper
        relay
        power
        nvs_flash
)
