# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

#FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(SRCS "main.c"
                            "../lib/access_protocol/access_protocol.c"
                            "../lib/uart/uart.c"
                       PRIV_REQUIRES esp_http_server)
# (SRCS ${app_sources})
