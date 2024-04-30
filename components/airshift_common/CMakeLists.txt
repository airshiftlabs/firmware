idf_component_register(SRCS "airshift_common.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "../../main/include"
                    REQUIRES airshift_nvs airshift_event)
