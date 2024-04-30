#include "airshift_matter.h"
#include "airshift_event.h"

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

static const char* TAG = "airshift_matter";

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

// Forward declarations
static esp_err_t    attribute_callback( attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data );
static esp_err_t    identification_callback( identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id, uint8_t effect_variant, void *priv_data );
static void         event_callback( const ChipDeviceEvent *event, intptr_t arg );

static node_t       *node_                          = NULL;
static endpoint_t   *endpoint_                      = NULL;
static bool         commissioning_session_started_  = false;

// Public functions
esp_err_t airshift_matter_init()
{
    esp_err_t                       ret                         = ESP_FAIL;
    node::config_t                  node_config                 = {};
    air_quality_sensor::config_t    air_quality_sensor_config   = {};

    ESP_LOGI( TAG, "airshift_matter_init" );

    // reset variables ...
    node_                           = NULL;
    endpoint_                       = NULL;
    commissioning_session_started_  = false;
    
    node_ = node::create( &node_config, attribute_callback, identification_callback );

    ESP_GOTO_ON_FALSE( ( node_ != NULL ), ret, error, TAG, "node::create failed" );

    endpoint_ = air_quality_sensor::create( node_, &air_quality_sensor_config, ENDPOINT_FLAG_NONE, NULL );

    ESP_GOTO_ON_FALSE( ( endpoint_ != NULL ), ret, error, TAG, "air_quality_sensor::create failed" );

    ESP_GOTO_ON_ERROR( esp_matter::start( event_callback ), error, TAG, "esp_matter::start failed" );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_matter_init failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    // clean up on failure ...
    ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_matter_release() );

    return ret;
}

esp_err_t airshift_matter_release()
{
    ESP_LOGI( TAG, "airshift_matter_release" );

    if( ( node_ != NULL ) && ( endpoint_ != NULL ) )
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT( destroy( node_, endpoint_ ) );
    }

    // reset variables ...
    node_                           = NULL;
    endpoint_                       = NULL;
    commissioning_session_started_  = false;

    return ESP_OK;
}

// Private functions
static esp_err_t attribute_callback( attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data )
{
    ESP_LOGE( TAG, "attribute_update_callback: attribute_id: %lu", attribute_id );

    return ESP_OK;
}

static esp_err_t identification_callback( identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id, uint8_t effect_variant, void *priv_data )
{
    ESP_LOGE( TAG, "identification_callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant );

    return ESP_OK;
}

static void event_callback( const ChipDeviceEvent *event, intptr_t arg )
{
    switch( event->Type )
    {
        case chip::DeviceLayer::DeviceEventType::kESPSystemEvent:
        {
            if( event->Platform.ESPSystemEvent.Base == WIFI_EVENT )
            {
                switch( event->Platform.ESPSystemEvent.Id )
                {
                    case WIFI_EVENT_STA_START:
                    {
                        ESP_LOGI( TAG, "WIFI_EVENT_STA_START" );
                        break;
                    }
                    case WIFI_EVENT_STA_CONNECTED:
                    {
                        ESP_LOGI( TAG, "WIFI_EVENT_STA_CONNECTED" );
                        break;
                    }
                    case WIFI_EVENT_STA_DISCONNECTED:
                    {
                        ESP_LOGI( TAG, "WIFI_EVENT_STA_DISCONNECTED" );
                        break;
                    }
                    default:
                    {
                        ESP_LOGE( TAG, "event->Platform.ESPSystemEvent.Base == WIFI_EVENT: ESPSystemEvent.Id: %lu", event->Platform.ESPSystemEvent.Id );
                        break;
                    }
                }
            }

            if( event->Platform.ESPSystemEvent.Base == IP_EVENT )
            {
                switch( event->Platform.ESPSystemEvent.Id )
                {
                    case IP_EVENT_STA_GOT_IP:
                    {
                        ESP_LOGI( TAG, "IP_EVENT_STA_GOT_IP" );

                        ESP_ERROR_CHECK_WITHOUT_ABORT( esp_event_post( AIRSHIFT_EVENT_MATTER, AIRSHIFT_EVENT_MATTER_IP_EVENT_STA_GOT_IP, NULL, 0, portMAX_DELAY ) );

                        break;
                    }
                    case IP_EVENT_GOT_IP6:
                    {
                        ESP_LOGI( TAG, "IP_EVENT_GOT_IP6" );
                        break;
                    }
                    default:
                    {
                        ESP_LOGE( TAG, "event->Platform.ESPSystemEvent.Base == IP_EVENT: ESPSystemEvent.Id: %lu", event->Platform.ESPSystemEvent.Id );
                        break;
                    }
                }
            }
            
            break;
        }
        case chip::DeviceLayer::DeviceEventType::kWiFiConnectivityChange:
        {
            ESP_LOGI( TAG, "kWiFiConnectivityChange" );
            break;
        }
        case chip::DeviceLayer::DeviceEventType::kInternetConnectivityChange:
        {
            ESP_LOGI( TAG, "kInternetConnectivityChange" );
            break;
        }
        case chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished:
        {
            ESP_LOGI( TAG, "kCHIPoBLEConnectionEstablished" );
            break;
        }
        case chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionClosed:
        {
            ESP_LOGI( TAG, "kCHIPoBLEConnectionClosed" );
            break;
        }
        case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        {
            ESP_LOGI( TAG, "kInterfaceIpAddressChanged" );
            break;
        }
        case chip::DeviceLayer::DeviceEventType::kCHIPoBLEAdvertisingChange:
        {
            ESP_LOGI( TAG, "kCHIPoBLEAdvertisingChange" );
            break;
        }
        case chip::DeviceLayer::DeviceEventType::kOperationalNetworkEnabled:
        {
            ESP_LOGI( TAG, "kOperationalNetworkEnabled" );
            break;
        }
        case chip::DeviceLayer::DeviceEventType::kDnssdInitialized:
        {
            ESP_LOGI( TAG, "kDnssdInitialized" );
            break;
        }
        case chip::DeviceLayer::DeviceEventType::kServerReady:
        {
            ESP_LOGI( TAG, "kServerReady" );
            break;
        }
        case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        {
            ESP_LOGI( TAG, "kCommissioningSessionStarted" );

            commissioning_session_started_ = true;

            break;
        }
        case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        {
            ESP_LOGI( TAG, "kCommissioningWindowOpened" );

            ESP_ERROR_CHECK_WITHOUT_ABORT( esp_event_post( AIRSHIFT_EVENT_MATTER, AIRSHIFT_EVENT_MATTER_COMMISSIONING_WINDOW_OPENED, NULL, 0, portMAX_DELAY ) );

            break;
        }
        case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        {
            ESP_LOGI( TAG, "kCommissioningWindowClosed" );

            ESP_ERROR_CHECK_WITHOUT_ABORT( esp_event_post( AIRSHIFT_EVENT_MATTER, AIRSHIFT_EVENT_MATTER_COMMISSIONING_WINDOW_CLOSED, (void *)&commissioning_session_started_, sizeof( bool ), portMAX_DELAY ) );

            break;
        }
        case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        {
            ESP_LOGI( TAG, "kCommissioningComplete" );
            break;
        }
        case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        {
            ESP_LOGI( TAG, "kCommissioningSessionStopped" );

            commissioning_session_started_ = false;

            break;
        }
        case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
        {
            ESP_LOGI( TAG, "kFabricWillBeRemoved" );
            break;
        }
        case chip::DeviceLayer::DeviceEventType::kFabricRemoved:
        {
            ESP_LOGI( TAG, "kFabricRemoved" );

            ESP_ERROR_CHECK_WITHOUT_ABORT( esp_event_post( AIRSHIFT_EVENT_GENERAL, AIRSHIFT_EVENT_GENERAL_REBOOT_NEEDED, NULL, 0, portMAX_DELAY ) );

            break;
        }
        case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
        {
            ESP_LOGI( TAG, "kFabricCommitted" );
            break;
        }
        default:
        {
            ESP_LOGE( TAG, "event_callback: event type: %d", event->Type );
            break;
        }
    }
}
