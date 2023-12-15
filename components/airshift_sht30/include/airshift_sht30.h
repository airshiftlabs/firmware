#ifndef AIRSHIFT_SHT30_H
#define AIRSHIFT_SHT30_H

#include "airshift_header_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float temperature;
    float humidity;
} sht30_data_t;

esp_err_t   airshift_sht30_init();
esp_err_t   airshift_sht30_release();

esp_err_t   airshift_sht30_get( sht30_data_t *sht30_data );

#ifdef __cplusplus
}
#endif

#endif // AIRSHIFT_SHT30_H
