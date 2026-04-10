idf_component_register(
    SRCS "modem.c"
    INCLUDE_DIRS "."
    REQUIRES driver uart esp_driver_gpio
)
