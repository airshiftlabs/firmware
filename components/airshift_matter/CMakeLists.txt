idf_component_register(SRCS "airshift_matter.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "../../main/include"
                    REQUIRES esp_matter airshift_event)
