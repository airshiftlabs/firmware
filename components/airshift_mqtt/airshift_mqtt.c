#include "airshift_mqtt.h"
#include "airshift_common.h"

#include <esp_netif.h>
#include <mqtt_client.h>

#define MQTT_URI    "mqtt://mqtt.airshift.app"
// #define MQTT_URI    "mqtt://192.168.1.250"
#define MQTT_PORT   1883

static const char* TAG = "airshift_mqtt";

// Forward declarations
static void mqtt_event_handler( void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data );

static char                     client_id_[32]          = { 0 };
static esp_mqtt_client_handle_t esp_mqtt_client_handle_ = NULL;

// Public functions
esp_err_t airshift_mqtt_init()
{
    esp_err_t                   ret                     = ESP_FAIL;
    esp_mqtt_client_config_t    esp_mqtt_client_config  = { 0 };

    ESP_LOGI( TAG, "airshift_mqtt_init" );

    ESP_GOTO_ON_ERROR( esp_netif_init(), error, TAG, "esp_netif_init failed" );

    ESP_GOTO_ON_ERROR( airshift_get_mac_address( client_id_, NULL ), error, TAG, "get_mac_address failed" );

    esp_mqtt_client_config.broker.address.uri       = MQTT_URI;
    esp_mqtt_client_config.broker.address.port      = MQTT_PORT;
    esp_mqtt_client_config.credentials.client_id    = client_id_;

    esp_mqtt_client_handle_ = esp_mqtt_client_init( &esp_mqtt_client_config );

    ESP_GOTO_ON_FALSE( ( esp_mqtt_client_handle_ != NULL ), ret, error, TAG, "esp_mqtt_client_init failed" );

    ESP_GOTO_ON_ERROR( esp_mqtt_client_register_event( esp_mqtt_client_handle_, MQTT_EVENT_ANY, mqtt_event_handler, NULL ), error, TAG, "esp_mqtt_client_register_event failed" );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_mqtt_init failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    // clean up on failure ...
    ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_mqtt_release() );

    return ret;
}

esp_err_t airshift_mqtt_release()
{
    ESP_LOGI( TAG, "airshift_mqtt_release" );

    if( esp_mqtt_client_handle_ != NULL )
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT( esp_mqtt_client_unregister_event( esp_mqtt_client_handle_, MQTT_EVENT_ANY, mqtt_event_handler ) );

        ESP_ERROR_CHECK_WITHOUT_ABORT( esp_mqtt_client_stop( esp_mqtt_client_handle_ ) );

        esp_mqtt_client_handle_ = NULL;
    }

    return ESP_OK;
}

esp_err_t airshift_mqtt_start()
{
    ESP_LOGI( TAG, "airshift_mqtt_start" );

    if( esp_mqtt_client_handle_ != NULL )
    {
        return esp_mqtt_client_start( esp_mqtt_client_handle_ );
    }

    return ESP_FAIL;
}

esp_err_t airshift_mqtt_publish( const char *topic, const char *message, size_t message_len )
{
    ESP_LOGI( TAG, "airshift_mqtt_publish -> topic: %s, message: %s", topic, message );

    if( esp_mqtt_client_handle_ != NULL )
    {
        return esp_mqtt_client_publish( esp_mqtt_client_handle_, topic, message, message_len, 1, 0 );
    }

    return ESP_FAIL;
}

const char *airshift_mqtt_get_client_id()
{
    return client_id_;
}

// Private functions
static void mqtt_event_handler( void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data )
{
    switch( event_id )
    {
        case MQTT_EVENT_BEFORE_CONNECT:
        {
            ESP_LOGI( TAG, "MQTT_EVENT_BEFORE_CONNECT" );
            break;
        }
        case MQTT_EVENT_CONNECTED:
        {
            ESP_LOGI( TAG, "MQTT_EVENT_CONNECTED" );
            break;
        }
        case MQTT_EVENT_DISCONNECTED:
        {
            ESP_LOGI( TAG, "MQTT_EVENT_DISCONNECTED" );
            break;
        }
        case MQTT_EVENT_PUBLISHED:
        {
            ESP_LOGI( TAG, "MQTT_EVENT_PUBLISHED" );
            break;
        }
        default:
        {
            ESP_LOGE( TAG, "mqtt_event_handler -> event_id: %ld", event_id );
            break;
        }
    }
}
