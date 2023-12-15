#include "airshift_i2c.h"
#include <driver/i2c.h>

#define I2C_MASTER_FREQ_HZ      400000
#define I2C_MASTER_TIMEOUT_MS   5000

static const char* TAG = "airshift_i2c";

// Forward declarations

// Public functions
esp_err_t airshift_i2c_init()
{
    esp_err_t       ret             = ESP_FAIL;
    i2c_config_t    i2c_config      = { .mode = I2C_MODE_MASTER, .sda_io_num = GPIO_NUM_37, .scl_io_num = GPIO_NUM_38, .sda_pullup_en = GPIO_PULLUP_ENABLE, .scl_pullup_en = GPIO_PULLUP_ENABLE, .master.clk_speed = I2C_MASTER_FREQ_HZ };

    ESP_LOGI( TAG, "airshift_i2c_init" );

    ESP_GOTO_ON_ERROR( i2c_param_config( I2C_NUM_0, &i2c_config ), error, TAG, "i2c_param_config failed" );

    ESP_GOTO_ON_ERROR( i2c_driver_install( I2C_NUM_0, i2c_config.mode, 0, 0, 0 ), error, TAG, "i2c_param_config failed" );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_i2c_init failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    return ret;
}

esp_err_t airshift_i2c_release()
{
    ESP_LOGI( TAG, "airshift_i2c_release" );

    // Check errors, but do not fail / abort, we're just stopping ...

    ESP_ERROR_CHECK_WITHOUT_ABORT( i2c_driver_delete( I2C_NUM_0 ) );

    return ESP_OK;
}

esp_err_t airshift_i2c_read( uint8_t device_address, uint8_t *read_buffer, size_t buffer_size )
{
    ESP_LOGI( TAG, "airshift_i2c_read" );

    return i2c_master_read_from_device( I2C_NUM_0, device_address, read_buffer, buffer_size, ( I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS ) );
}

esp_err_t airshift_i2c_write( uint8_t device_address, uint8_t *write_buffer, size_t buffer_size )
{
    ESP_LOGI( TAG, "airshift_i2c_read" );

    return i2c_master_write_to_device( I2C_NUM_0, device_address, write_buffer, buffer_size, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS );
}

// Private functions

