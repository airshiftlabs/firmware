idf_component_register(SRCS "airshift_nvs.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "../../main/include"
                    REQUIRES nvs_flash)
