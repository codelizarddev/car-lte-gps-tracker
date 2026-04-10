/**
 * unity_config.h — Unity test framework configuration for host (PC) build
 */
#pragma once

#define UNITY_OUTPUT_CHAR(c)     putchar(c)
#define UNITY_OUTPUT_FLUSH()     fflush(stdout)
