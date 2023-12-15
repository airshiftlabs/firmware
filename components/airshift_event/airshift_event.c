#include "airshift_event.h"

static const char* TAG = "airshift_event";

ESP_EVENT_DEFINE_BASE( AIRSHIFT_EVENT_MATTER );
ESP_EVENT_DEFINE_BASE( AIRSHIFT_EVENT_GENERAL );

// Forward declarations

// Public functions
esp_err_t airshift_event_init()
{
    esp_err_t ret = ESP_FAIL;

    ESP_LOGI( TAG, "airshift_event_init" );
    
    ESP_GOTO_ON_ERROR( esp_event_loop_create_default(), error, TAG, "esp_event_loop_create_default failed" );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_event_init failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    return ret;
}

esp_err_t airshift_event_release()
{
    esp_err_t ret = ESP_FAIL;

    ESP_LOGI( TAG, "airshift_event_release" );
    
    ESP_GOTO_ON_ERROR( esp_event_loop_delete_default(), error, TAG, "esp_event_loop_delete_default failed" );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_event_release failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    return ret;
}

// Private functions

