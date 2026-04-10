idf_component_register(
    SRCS "mqtt_client_wrapper.c"
    INCLUDE_DIRS "."
    REQUIRES mqtt relay gnss power_monitor esp_log freertos
)
