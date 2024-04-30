idf_component_register(SRCS "airshift_i2c.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "../../main/include"
                    REQUIRES driver freertos)
