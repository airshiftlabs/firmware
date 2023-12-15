idf_component_register(SRCS "airshift_pms7003.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "../../main/include"
                    REQUIRES driver freertos airshift_common)
