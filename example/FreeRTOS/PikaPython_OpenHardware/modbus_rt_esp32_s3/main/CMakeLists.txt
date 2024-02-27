# Embed the server root certificate into the final binary
idf_build_get_property(project_dir PROJECT_DIR)
file(GLOB_RECURSE SOURCES *.c)
idf_component_register(SRCS ${SOURCES}
                WHOLE_ARCHIVE
                INCLUDE_DIRS 
                "."
                "${IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos"
                )