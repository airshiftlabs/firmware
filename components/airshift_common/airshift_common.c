#include "airshift_common.h"
#include <esp_mac.h>

static const char* TAG = "airshift_common";

// Forward declarations

// Public functions
uint64_t seconds_to_microseconds( uint32_t seconds )
{
    ESP_LOGI( TAG, "seconds_to_microseconds -> %lu", seconds );

    return ( (uint64_t)seconds * (uint64_t)1000 * (uint64_t)1000 );
}

uint16_t airshift_make_word( uint8_t h, uint8_t l )
{
    return ( h << 8 ) | l;
}

esp_err_t airshift_get_mac_address( char* mac_address, const char* delimiter )
{
    esp_err_t   ret         = ESP_FAIL;
    uint8_t     mac_addr[6] = { 0 };

    ESP_LOGI( TAG, "airshift_get_mac_address" );

    if( mac_address == NULL ){ goto error; }

    ESP_GOTO_ON_ERROR( esp_read_mac( mac_addr, ESP_MAC_WIFI_STA ), error, TAG, "esp_read_mac failed" );

    if( delimiter != NULL )
    {
        sprintf( mac_address, "%02X%s%02X%s%02X%s%02X%s%02X%s%02X", mac_addr[0], delimiter, mac_addr[1], delimiter, mac_addr[2], delimiter, mac_addr[3], delimiter, mac_addr[4], delimiter, mac_addr[5] );
    }
    else
    {
        sprintf( mac_address, "%02X%02X%02X%02X%02X%02X", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5] );
    }

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_get_mac_address failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    return ret;
}

// Private functions
