idf_component_register(SRCS "airshift_mqtt.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "../../main/include"
                    REQUIRES esp_netif mqtt airshift_common)
