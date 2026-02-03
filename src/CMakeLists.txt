file(GLOB_RECURSE PROJECT_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/*.c"
)
target_sources(app PRIVATE ${PROJECT_SOURCES})

# target_sources(app PRIVATE
#     main.c
#     llextc/llextc.c
#     llextcc/llextc_priv.c
#     llextcc/llextc_utils.c
#     llextcc/llextc_container_api.c
#     oci/parser.c
#     neta/wifi.c
#     neta/http.c
# )
