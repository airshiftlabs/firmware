#ifndef AIRSHIFT_I2C_H
#define AIRSHIFT_I2C_H

#include "airshift_header_common.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t   airshift_i2c_init();
esp_err_t   airshift_i2c_release();

esp_err_t   airshift_i2c_read( uint8_t device_address, uint8_t *read_buffer, size_t buffer_size );
esp_err_t   airshift_i2c_write( uint8_t device_address, uint8_t *write_buffer, size_t buffer_size );

#ifdef __cplusplus
}
#endif

#endif // AIRSHIFT_I2C_H
