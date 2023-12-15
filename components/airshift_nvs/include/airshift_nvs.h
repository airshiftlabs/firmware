#ifndef AIRSHIFT_NVS_H
#define AIRSHIFT_NVS_H

#include "airshift_header_common.h"

#include <nvs_flash.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t   airshift_nvs_init();
size_t      airshift_nvs_available( size_t *total_entries, size_t *used_entries );
esp_err_t   airshift_nvs_release();
esp_err_t   airshift_nvs_format();

esp_err_t   airshift_nvs_write_string( const char* namespace, const char* key, const char* value );
esp_err_t   airshift_nvs_write_int16( const char* namespace, const char* key, int16_t value );
esp_err_t   airshift_nvs_write_int32( const char* namespace, const char* key, int32_t value );
esp_err_t   airshift_nvs_write_blob( const char* namespace, const char* key, const void* value, size_t length );

esp_err_t   airshift_nvs_get_string( const char* namespace, const char* key, char* out_value, size_t* length );
esp_err_t   airshift_nvs_get_int16( const char* namespace, const char* key, int16_t* out_value );
esp_err_t   airshift_nvs_get_int32( const char* namespace, const char* key, int32_t* out_value );
esp_err_t   airshift_nvs_get_blob( const char* namespace, const char* key, void* out_value, size_t* length );

esp_err_t   airshift_nvs_erase_value( const char* namespace, const char* key );
esp_err_t   airshift_nvs_erase_all( const char* namespace );

bool        airshift_nvs_key_exists( const char* namespace, const char* key );

esp_err_t   airshift_nvs_get_or_create_int16( const char* namespace, const char* key, int16_t* out_value );
esp_err_t   airshift_nvs_get_or_create_int32( const char* namespace, const char* key, int32_t* out_value );

#ifdef __cplusplus
}
#endif

#endif // AIRSHIFT_NVS_H
