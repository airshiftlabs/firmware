#include "airshift_sht30.h"
#include "airshift_i2c.h"
#include "airshift_common.h"

#define SHT30_DEVICE_ADDRESS        0x44

#define SHT30_COMMAND_SOFT_RESET    0x30a2
#define SHT30_COMMAND_BREAK         0x3093
#define SHT30_COMMAND_READ_STATUS   0xf32d
#define SHT30_COMMAND_CLEAR_STATUS  0x3041
#define SHT30_COMMAND_PERIODIC_10_H 0x2737
#define SHT30_COMMAND_FETCH_DATA    0xe000

static const char* TAG = "airshift_sht30";

// Forward declarations
static esp_err_t    write_command( uint16_t command );
static esp_err_t    read_command( uint16_t command, uint8_t *read_buffer, size_t buffer_size );
static uint8_t      calc_crc8( uint8_t *data, int len );

// Public functions
esp_err_t airshift_sht30_init()
{
    esp_err_t ret = ESP_FAIL;

    ESP_LOGI( TAG, "airshift_sht30_init" );

    // issue a break command ...
    ESP_GOTO_ON_ERROR( write_command( SHT30_COMMAND_BREAK ), error, TAG, "write_command -> SHT30_COMMAND_BREAK failed" );

    // issue a soft reset  ...
    ESP_GOTO_ON_ERROR( write_command( SHT30_COMMAND_SOFT_RESET ), error, TAG, "write_command -> SHT30_COMMAND_SOFT_RESET failed" );

    // set measurement mode ...
    ESP_GOTO_ON_ERROR( write_command( SHT30_COMMAND_PERIODIC_10_H ), error, TAG, "write_command -> SHT30_COMMAND_PERIODIC_10_H failed" );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_sht30_init failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    return ret;
}

esp_err_t airshift_sht30_release()
{
    ESP_LOGI( TAG, "airshift_sht30_release" );

    return ESP_OK;
}

esp_err_t airshift_sht30_get( sht30_data_t *sht30_data )
{
    esp_err_t   ret             = ESP_FAIL;
    uint8_t     read_buffer[6]  = { 0 };

    ESP_LOGI( TAG, "airshift_sht30_get" );

    // reset output ...
    memset( sht30_data, 0, sizeof( sht30_data_t ) );

    // read_command ...
    ESP_GOTO_ON_ERROR( read_command( SHT30_COMMAND_FETCH_DATA, read_buffer, sizeof( read_buffer ) ), error, TAG, "read_command failed" );

    // validate temperature data ...
    ESP_GOTO_ON_FALSE( ( calc_crc8( read_buffer, 2 ) == read_buffer[2] ), ret, error, TAG, "invalid temperature crc" );

    // validate humidity data ...
    ESP_GOTO_ON_FALSE( ( calc_crc8( &read_buffer[3], 2 ) == read_buffer[5] ), ret, error, TAG, "invalid humidity crc" );

    // data good, return our results ...
    sht30_data->temperature = ( 175.0f * (float)airshift_make_word( read_buffer[0], read_buffer[1] ) / 65535.0f - 45.0f );
    sht30_data->humidity    = ( 100.0f * ( (float)airshift_make_word( read_buffer[3], read_buffer[4] ) / 65535.0f ) );

    ESP_LOGI( TAG, "airshift_sht30_get -> temperature: %f, humidity: %f", sht30_data->temperature, sht30_data->humidity );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_sht30_get failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    return ret;
}

// Private functions
static esp_err_t write_command( uint16_t command )
{
    uint8_t write_buffer[2] = { 0 };

    ESP_LOGI( TAG, "write_command: 0x%02hx", command );

	write_buffer[0] = command >> 8;
	write_buffer[1] = (uint8_t)command;

    return airshift_i2c_write( SHT30_DEVICE_ADDRESS, write_buffer, sizeof( write_buffer ) );
}

static esp_err_t read_command( uint16_t command, uint8_t *read_buffer, size_t buffer_size )
{
    esp_err_t ret = ESP_FAIL;

    ESP_LOGI( TAG, "read_command: 0x%02hx", command );

    // write_command ...
    ESP_GOTO_ON_ERROR( write_command( command ), error, TAG, "write_command failed" );

    // airshift_i2c_read ...
    ESP_GOTO_ON_ERROR( airshift_i2c_read( SHT30_DEVICE_ADDRESS, read_buffer, buffer_size ), error, TAG, "airshift_i2c_read failed" );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "read_command failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    return ret;
}

static uint8_t calc_crc8( uint8_t *data, int len )
{
    const uint8_t   POLYNOMIAL  = 0x31;
	uint8_t         crc         = 0xFF;
	
	for( int i = 0; i < len; ++i )
    {
        crc ^= *data++;
        
        for( int j = 0; j < 8; ++j )
        {
            crc = ( crc & 0x80 ) ? ( crc << 1 ) ^ POLYNOMIAL : ( crc << 1 );
        }
    }
    
    return crc;
}
