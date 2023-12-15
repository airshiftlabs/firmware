#include "airshift_pms7003.h"
#include "airshift_common.h"

#include <driver/uart.h>
#include <driver/gpio.h>

#define UART_PORT           UART_NUM_2
#define TX_PIN              GPIO_NUM_18
#define RX_PIN              GPIO_NUM_8
#define RECEIVE_BUFFER_SIZE 128
#define START_CHARACTER_1   0x42
#define START_CHARACTER_2   0x4d

static const char* TAG = "airshift_pms7003";

// Forward declarations

// Public functions
esp_err_t airshift_pms7003_init()
{
    esp_err_t           ret             = ESP_FAIL;
    const uart_config_t uart_config     = { .baud_rate = 9600, .data_bits = UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE, .stop_bits = UART_STOP_BITS_1, .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, .source_clk = UART_SCLK_APB };

    ESP_LOGI( TAG, "airshift_pms7003_init" );

    // uart_driver_install
    ESP_GOTO_ON_ERROR( uart_driver_install( UART_PORT, ( RECEIVE_BUFFER_SIZE * 2 ), 0, 0, NULL, 0 ), error, TAG, "uart_driver_install failed" );

    // uart_param_config
    ESP_GOTO_ON_ERROR( uart_param_config( UART_PORT, &uart_config ), error, TAG, "uart_param_config failed" );

    // uart_set_pin
    ESP_GOTO_ON_ERROR( uart_set_pin( UART_PORT, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE ), error, TAG, "uart_set_pin failed" );

    // default to waking up the device, can be put to sleep later ...
    ESP_GOTO_ON_ERROR( airshift_pms7003_wakeup(), error, TAG, "airshift_pms7003_wakeup failed" );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_pms7003_init failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    // Cleanup on any error ...
    airshift_pms7003_release();

    return ret;
}

esp_err_t airshift_pms7003_release()
{
    ESP_LOGI( TAG, "airshift_pms7003_release" );

    ESP_ERROR_CHECK_WITHOUT_ABORT( uart_driver_delete( UART_PORT ) );

    return ESP_OK;
}

esp_err_t airshift_pms7003_sleep()
{
    uint8_t command[] = { 0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73 };

    ESP_LOGI( TAG, "airshift_pms7003_sleep" );

    uart_write_bytes( UART_PORT, command, sizeof( command ) );

    return ESP_OK;
}

esp_err_t airshift_pms7003_wakeup()
{
    uint8_t command[] = { 0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74 };

    ESP_LOGI( TAG, "airshift_pms7003_wakeup" );

    uart_write_bytes( UART_PORT, command, sizeof( command ) );

    return ESP_OK;
}

esp_err_t airshift_pms7003_get( pms7003_data_t *pms7003_data )
{
    esp_err_t   ret                                 = ESP_FAIL;
    uint8_t     read_buffer[RECEIVE_BUFFER_SIZE]    = { 0 };
    int         bytes_read                          = 0;
    uint8_t     passive_mode[]                      = { 0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71 };

    ESP_LOGI( TAG, "airshift_pms7003_get" );

    // reset output ...
    memset( pms7003_data, 0, sizeof( pms7003_data_t ) );

    // set passive mode ...
    uart_write_bytes( UART_PORT, passive_mode, sizeof( passive_mode ) );

    // uart_read_bytes ...
    bytes_read = uart_read_bytes( UART_PORT, read_buffer, sizeof( read_buffer ), ( 1000 / portTICK_PERIOD_MS ) );

    ESP_GOTO_ON_FALSE( ( bytes_read > 0 ), ret, error, TAG, "uart_read_bytes no bytes read" );

    for( size_t i = 0; i < bytes_read; i++ )
    {
        // must make sure enough sapce for data left ...
        if( ( bytes_read - i ) < sizeof( pms7003_data_t ) )
        {
            ESP_LOGE( TAG, "pms7003 data not found: %d, %d", bytes_read, i );

            goto error;
        }

        // look for start ...
        if( ( read_buffer[i] == START_CHARACTER_1 ) && ( read_buffer[i + 1] == START_CHARACTER_2 ) )
        {
            uint16_t calculated_checksum = 0;

            pms7003_data->frame_length  = airshift_make_word( read_buffer[i + 2], read_buffer[i + 3] );

            pms7003_data->pm_sp_ug_1_0  = airshift_make_word( read_buffer[i + 4], read_buffer[i + 5] );
            pms7003_data->pm_sp_ug_2_5  = airshift_make_word( read_buffer[i + 6], read_buffer[i + 7] );
            pms7003_data->pm_sp_ug_10_0 = airshift_make_word( read_buffer[i + 8], read_buffer[i + 9] );

            pms7003_data->pm_ae_ug_1_0  = airshift_make_word( read_buffer[i + 10], read_buffer[i + 11] );
            pms7003_data->pm_ae_ug_2_5  = airshift_make_word( read_buffer[i + 12], read_buffer[i + 13] );
            pms7003_data->pm_ae_ug_10_0 = airshift_make_word( read_buffer[i + 14], read_buffer[i + 15] );

            pms7003_data->pm_raw_0_3    = airshift_make_word( read_buffer[i + 16], read_buffer[i + 17] );
            pms7003_data->pm_raw_0_5    = airshift_make_word( read_buffer[i + 18], read_buffer[i + 19] );
            pms7003_data->pm_raw_1_0    = airshift_make_word( read_buffer[i + 20], read_buffer[i + 21] );
            pms7003_data->pm_raw_2_5    = airshift_make_word( read_buffer[i + 22], read_buffer[i + 23] );
            pms7003_data->pm_raw_5_0    = airshift_make_word( read_buffer[i + 24], read_buffer[i + 25] );
            pms7003_data->pm_raw_10_0   = airshift_make_word( read_buffer[i + 26], read_buffer[i + 27] );

            pms7003_data->checksum      = airshift_make_word( read_buffer[i + 30], read_buffer[i + 31] );

            // calculate checksum ...
            for( size_t x = i; x < ( i + 30 ); x++ )
            {
                calculated_checksum += read_buffer[x];
            }

            ESP_LOGI( TAG, "pms7003_data: frame_length: %hu, checksum: %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu, calculated_checksum: %hu",
                pms7003_data->frame_length,
                pms7003_data->checksum,
                pms7003_data->pm_sp_ug_1_0,
                pms7003_data->pm_sp_ug_2_5,
                pms7003_data->pm_sp_ug_10_0,
                pms7003_data->pm_ae_ug_1_0,
                pms7003_data->pm_ae_ug_2_5,
                pms7003_data->pm_ae_ug_10_0,
                pms7003_data->pm_raw_0_3, 
                pms7003_data->pm_raw_0_5, 
                pms7003_data->pm_raw_1_0, 
                pms7003_data->pm_raw_2_5, 
                pms7003_data->pm_raw_5_0, 
                pms7003_data->pm_raw_10_0, 
                calculated_checksum );

            break;
        }
    }

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_pms7003_get failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    return ret;
}

// Private functions