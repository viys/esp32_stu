idf_component_register(SRCS "led.c"
                    PRIV_REQUIRES esp_driver_gpio
                    PRIV_REQUIRES esp_driver_ledc
                    INCLUDE_DIRS ".")
