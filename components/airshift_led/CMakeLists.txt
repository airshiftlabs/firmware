idf_component_register(SRCS "airshift_led.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "../../main/include"
                    REQUIRES driver freertos led_strip)
