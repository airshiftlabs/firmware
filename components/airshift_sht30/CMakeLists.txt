idf_component_register(SRCS "airshift_sht30.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "../../main/include"
                    REQUIRES driver freertos airshift_i2c airshift_common)
