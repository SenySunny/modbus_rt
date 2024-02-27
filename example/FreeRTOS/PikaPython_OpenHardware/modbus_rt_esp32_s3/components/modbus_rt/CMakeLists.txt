set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-char-subscripts -Wno-maybe-uninitialized")

file(GLOB_RECURSE SOURCES *.c)
idf_component_register(
                SRCS ${SOURCES}
                INCLUDE_DIRS 
                "agile_modbus" 
                "modbus_rt" 
                "platform/freertos" 
                "slave_util" 
                "../../main"
                "${IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos"
                WHOLE_ARCHIVE
                REQUIRES
                driver
                fatfs
                esp_wifi
                nvs_flash
                app_update
                esp_http_client
                esp_https_ota
                )