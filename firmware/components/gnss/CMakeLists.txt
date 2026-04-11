idf_component_register(
    SRCS "gnss.c" "gnss_pure.c"
    INCLUDE_DIRS "."
    REQUIRES modem driver esp_log
)
