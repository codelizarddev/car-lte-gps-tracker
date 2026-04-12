idf_component_register(
    SRCS "modem.c"
    INCLUDE_DIRS "."
    PRIV_INCLUDE_DIRS "../../config"
    REQUIRES driver log
)
