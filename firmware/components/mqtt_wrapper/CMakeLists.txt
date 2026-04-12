idf_component_register(
    SRCS "mqtt_client_wrapper.c" "mqtt_payloads.c"
    INCLUDE_DIRS "."
    REQUIRES mqtt relay gnss power_monitor driver esp_log freertos
)
