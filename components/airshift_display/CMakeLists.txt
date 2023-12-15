idf_component_register(SRCS "airshift_display.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "../../main/include"
                    REQUIRES driver lvgl lvgl_esp32_drivers)
