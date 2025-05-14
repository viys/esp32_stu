idf_component_register(SRCS "uart.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_driver_uart
                    PRIV_REQUIRES esp_driver_gpio
                    )
