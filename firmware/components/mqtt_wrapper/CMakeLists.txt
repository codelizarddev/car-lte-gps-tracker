idf_component_register(
    SRCS "mqtt_client_wrapper.c" "mqtt_payloads.c"
    INCLUDE_DIRS "."
    PRIV_INCLUDE_DIRS "../../config"
    REQUIRES mqtt relay gnss power driver log freertos
)
