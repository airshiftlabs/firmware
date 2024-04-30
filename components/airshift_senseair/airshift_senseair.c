#include "airshift_senseair.h"

#include <driver/uart.h>
#include <driver/gpio.h>

#define UART_PORT           UART_NUM_1
#define TX_PIN              GPIO_NUM_7
#define RX_PIN              GPIO_NUM_6
#define RECEIVE_BUFFER_SIZE 128

static const char* TAG = "airshift_senseair";

// Forward declarations

// Public functions
esp_err_t airshift_senseair_init()
{
    esp_err_t           ret             = ESP_FAIL;
    const uart_config_t uart_config     = { .baud_rate = 9600, .data_bits = UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE, .stop_bits = UART_STOP_BITS_1, .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, .source_clk = UART_SCLK_APB };

    ESP_LOGI( TAG, "airshift_senseair_init" );

    // uart_driver_install
    ESP_GOTO_ON_ERROR( uart_driver_install( UART_PORT, ( RECEIVE_BUFFER_SIZE * 2 ), 0, 0, NULL, 0 ), error, TAG, "uart_driver_install failed" );

    // uart_param_config
    ESP_GOTO_ON_ERROR( uart_param_config( UART_PORT, &uart_config ), error, TAG, "uart_param_config failed" );

    // uart_set_pin
    ESP_GOTO_ON_ERROR( uart_set_pin( UART_PORT, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE ), error, TAG, "uart_set_pin failed" );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_senseair_init failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    // Cleanup on any error ...
    airshift_senseair_release();

    return ret;
}

esp_err_t airshift_senseair_release()
{
    ESP_LOGI( TAG, "airshift_senseair_release" );

    ESP_ERROR_CHECK_WITHOUT_ABORT( uart_driver_delete( UART_PORT ) );

    return ESP_OK;
}

esp_err_t airshift_senseair_get( uint32_t *value )
{
    esp_err_t   ret                                 = ESP_FAIL;
    uint8_t     read_buffer[RECEIVE_BUFFER_SIZE]    = { 0 };
    int         bytes_read                          = 0;
    uint8_t     read_command[]                      = { 0xfe, 0x04, 0x00, 0x03, 0x00, 0x01, 0xd5, 0xc5 };

    ESP_LOGI( TAG, "airshift_senseair_get" );

    // reset output ...
    *value = 0;

    // send read request ...
    uart_write_bytes( UART_PORT, read_command, sizeof( read_command ) );

    // uart_read_bytes ...
    bytes_read = uart_read_bytes( UART_PORT, read_buffer, sizeof( read_buffer ), ( 1000 / portTICK_PERIOD_MS ) );

    // response must be 7 bytes ...
    ESP_GOTO_ON_FALSE( ( bytes_read == 7 ), ret, error, TAG, "invalid response received" );

    // format output ...
    *value = ( 256 * read_buffer[3] ) + read_buffer[4];

    ESP_LOGI( TAG, "airshift_senseair_get: %lu", *value  );
    
    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_senseair_get failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    return ret;
}

// Private functions
