#ifndef AIRSHIFT_COMMON_H
#define AIRSHIFT_COMMON_H

#include "airshift_header_common.h"

#ifdef __cplusplus
extern "C" {
#endif

uint64_t    seconds_to_microseconds( uint32_t seconds );
uint16_t    airshift_make_word( uint8_t h, uint8_t l );
esp_err_t   airshift_get_mac_address( char* mac_address, const char* delimiter );

#ifdef __cplusplus
}
#endif

#endif // AIRSHIFT_COMMON_H
