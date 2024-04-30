#ifndef AIRSHIFT_SENSEAIR_H
#define AIRSHIFT_SENSEAIR_H

#include "airshift_header_common.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t   airshift_senseair_init();
esp_err_t   airshift_senseair_release();

esp_err_t   airshift_senseair_get( uint32_t *value );

#ifdef __cplusplus
}
#endif

#endif // AIRSHIFT_SENSEAIR_H
