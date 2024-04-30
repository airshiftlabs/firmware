#include "airshift_header_common.h"

#include <esp_timer.h>

// Components ...
#include "airshift_event.h"
#include "airshift_nvs.h"
#include "airshift_common.h"
#include "airshift_i2c.h"
#include "airshift_matter.h"
#include "airshift_display.h"
#include "airshift_pms7003.h"
#include "airshift_senseair.h"
#include "airshift_sht30.h"
#include "airshift_led.h"
#include "airshift_ui.h"
#include "airshift_mqtt.h"

// Component handlers ...

static const char* TAG = "airshift_main";

// Forward declarations
static esp_err_t	init();
static void			release();
static void			airshift_event_handler( void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data );
static void			start_restart_timer( uint64_t timeout_us );
static void			restart_timer_callback( void* arguments );
static void			sensor_polling_task( void* arguments );
static void			mqtt_publish( int co2, float temp, int pm2, float rh );
static void			update_leds( int co2 );
static void			blink_leds_task( void* arguments );

static const esp_timer_create_args_t	restart_timer_args_ = { .callback = &restart_timer_callback, .name = "restart-timer" };
static esp_timer_handle_t				restart_timer_		= NULL;
static bool								blink_leds_			= false;

// Public functions
void app_main( void )
{
	esp_reset_reason_t			reset_reason	= esp_reset_reason();
	esp_sleep_wakeup_cause_t	wakeup_cause	= esp_sleep_get_wakeup_cause();
	uint64_t					wakeup_status	= esp_sleep_get_ext1_wakeup_status();
	struct timeval				utc_time		= { 0 };

	gettimeofday( &utc_time, NULL );

	ESP_LOGI( TAG, "esp_reset_reason: %d", reset_reason );
	ESP_LOGI( TAG, "esp_wakeup_cause: %d", wakeup_cause );
	ESP_LOGI( TAG, "esp_wakeup_status: %llu", wakeup_status );
	ESP_LOGI( TAG, "utc_time: %lld", utc_time.tv_sec );

	init(); // TODO: Need to handle init failures somehow, flashing lights, etc... depending on the failure...

	// Log some debug / startup information ...
	ESP_LOGI( TAG, "AirShift firmware running ..." );

	// Main event loop ...
	while( true )
	{
		lv_task_handler();
		
		vTaskDelay( 1000 / portTICK_PERIOD_MS );
	}
}

// Private functions
static esp_err_t init()
{
	esp_err_t ret = ESP_FAIL;

	ESP_LOGI( TAG, "init" );

	ESP_GOTO_ON_ERROR( airshift_event_init(), error, TAG, "airshift_event_init failed" );
	
	ESP_GOTO_ON_ERROR( esp_event_handler_instance_register( AIRSHIFT_EVENT_GENERAL, ESP_EVENT_ANY_ID, airshift_event_handler, NULL, NULL ), error, TAG, "esp_event_handler_instance_register failed" );
	ESP_GOTO_ON_ERROR( esp_event_handler_instance_register( AIRSHIFT_EVENT_MATTER, ESP_EVENT_ANY_ID, airshift_event_handler, NULL, NULL ), error, TAG, "esp_event_handler_instance_register failed" );

	ESP_GOTO_ON_ERROR( airshift_nvs_init(), error, TAG, "airshift_nvs_init failed" );

	ESP_GOTO_ON_ERROR( airshift_i2c_init(), error, TAG, "airshift_i2c_init failed" );

	ESP_GOTO_ON_ERROR( airshift_led_init(), error, TAG, "airshift_led_init failed" );

	ESP_GOTO_ON_ERROR( airshift_display_init(), error, TAG, "airshift_display_init failed" );

	ESP_GOTO_ON_ERROR( airshift_ui_init(), error, TAG, "airshift_ui_init failed" );

	ESP_GOTO_ON_ERROR( airshift_pms7003_init(), error, TAG, "airshift_pms7003_init failed" );

	ESP_GOTO_ON_ERROR( airshift_senseair_init(), error, TAG, "airshift_senseair_init failed" );

	ESP_GOTO_ON_ERROR( airshift_sht30_init(), error, TAG, "airshift_sht30_init failed" );

	ESP_GOTO_ON_ERROR( airshift_matter_init(), error, TAG, "airshift_matter_init failed" );

	ESP_GOTO_ON_ERROR( airshift_mqtt_init(), error, TAG, "airshift_mqtt_init failed" );
	
	return ESP_OK;

error:

    ESP_LOGE( TAG, "device_init failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

	// Any part of device initialization has failed, let's cleanup, so we can try again ...
	release();

	return ret;
}

static void release()
{
	ESP_LOGI( TAG, "release" );

	ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_mqtt_release() );

	ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_matter_release() );

	ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_sht30_release() );
	
	ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_senseair_release() );

	ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_pms7003_release() );

	ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_ui_release() );

	ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_display_release() );

	ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_led_release() );

	ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_i2c_release() );

	ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_nvs_release() );
	
	ESP_ERROR_CHECK_WITHOUT_ABORT( esp_event_handler_unregister( AIRSHIFT_EVENT_GENERAL, ESP_EVENT_ANY_ID, &airshift_event_handler ) );
	
	ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_event_release() );

	if( ( restart_timer_ != NULL ) && ( esp_timer_is_active( restart_timer_ ) == true ) )
	{
		ESP_ERROR_CHECK_WITHOUT_ABORT( esp_timer_stop( restart_timer_ ) );
		ESP_ERROR_CHECK_WITHOUT_ABORT( esp_timer_delete( restart_timer_ ) );
	}
}

static void airshift_event_handler( void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data )
{
	ESP_LOGI( TAG, "%s: airshift_event_handler -> event id: %ld", base, event_id );

	switch( event_id )
	{
		case AIRSHIFT_EVENT_MATTER_COMMISSIONING_WINDOW_OPENED:
		{
			// need to show qrcode for provisioning ...
			ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_ui_display_qrcode() );

			break;
		}
		case AIRSHIFT_EVENT_MATTER_COMMISSIONING_WINDOW_CLOSED:
		{
			bool commissioning_session_started = *((bool *)event_data);

			ESP_LOGE( TAG, "AIRSHIFT_EVENT_MATTER_COMMISSIONING_WINDOW_CLOSED: %d", commissioning_session_started );

			// device was not provisioned in time, reboot to start process over again ...
			if( commissioning_session_started != true )
			{
				start_restart_timer( seconds_to_microseconds( 2 ) );
			}

			break;
		}
		case AIRSHIFT_EVENT_MATTER_IP_EVENT_STA_GOT_IP:
		{
			// create task to poll sensors ...
			xTaskCreate( sensor_polling_task, "sensor_polling_task", 8192, NULL, 5, NULL );

			break;
		}
		case AIRSHIFT_EVENT_GENERAL_REBOOT_NEEDED:
		{
			start_restart_timer( seconds_to_microseconds( 2 ) );

			break;
		}
	}
}

static void start_restart_timer( uint64_t timeout_us )
{
	ESP_LOGI( TAG, "start_restart_timer" );

	ESP_ERROR_CHECK_WITHOUT_ABORT( esp_timer_create( &restart_timer_args_, &restart_timer_ ) );

	ESP_ERROR_CHECK_WITHOUT_ABORT( esp_timer_start_once( restart_timer_, timeout_us ) );
}

static void restart_timer_callback( void* arguments )
{
	ESP_LOGI( TAG, "restart_timer_callback" );

	release();

	esp_restart();
}

static void sensor_polling_task( void* arguments )
{
	pms7003_data_t	pms7003_data	= { 0 };
	uint32_t		co2_value		= 0;
	sht30_data_t	sht30_data		= { 0 };

	ESP_LOGI( TAG, "sensor_polling_task" );

	// display main user interface ....
	airshift_ui_display_main();

	// start mqtt ...
	airshift_mqtt_start();

	// upon startup, give all sensors 30 seconds to warm up? Just a random number here, may need to be tweaked ...
	vTaskDelay( 30000 / portTICK_PERIOD_MS );

	while( true )
	{
		// read sensor data ...
		airshift_pms7003_get( &pms7003_data );

		airshift_senseair_get( &co2_value );

		airshift_sht30_get( &sht30_data );

		// ... update ui with sensor data ...
		airshift_ui_set_co2( (uint16_t)co2_value );

		airshift_ui_set_temperature( sht30_data.temperature );

		airshift_ui_set_particulate_matter( pms7003_data.pm_sp_ug_2_5 );

		// ... update leds ...
		update_leds( co2_value );

		// ... send sensor data via mqtt ...
		mqtt_publish( co2_value, sht30_data.temperature, pms7003_data.pm_sp_ug_2_5, sht30_data.humidity );

		vTaskDelay( 10000 / portTICK_PERIOD_MS ); // sleep 10 seconds before next reading ...
	}

	vTaskDelete( NULL );
}

static void mqtt_publish( int co2, float temp, int pm2, float rh )
{
	char		topic[64]	= { 0 };
    char		message[32]	= { 0 };
	const char	*client_id	= airshift_mqtt_get_client_id();

	ESP_LOGI( TAG, "mqtt_publish" );

	/* CO2 */
	sprintf( topic, "co2/%s", client_id );
	sprintf( message, "%d", co2 );

	airshift_mqtt_publish( topic, message, strlen( message ) );

	/* Temperature */
	sprintf( topic, "temp/%s", client_id );
	sprintf( message, "%.01f", temp );
	
	airshift_mqtt_publish( topic, message, strlen( message ) );

	/* Humidity */
	sprintf( topic, "rh/%s", client_id );
	sprintf( message, "%.0f", rh );
	
	airshift_mqtt_publish( topic, message, strlen( message ) );

	/* PM */
	sprintf( topic, "pm2.5/%s", client_id );
	sprintf( message, "%d", pm2 );
	
	airshift_mqtt_publish( topic, message, strlen( message ) );
}

static void update_leds( int co2 )
{
	airshift_led_color_t airshift_led_color = AIRSHIFT_LED_COLOR_BLACK;

	ESP_LOGI( TAG, "update_leds" );

	// 400 ppm: average outdoor air level.
	// 400–1,000 ppm: typical level found in occupied spaces with good air exchange.
	// 1,000–2,000 ppm: level associated with complaints of drowsiness and poor air.
	// 2,000–5,000 ppm: level associated with headaches, sleepiness, and stagnant, stale, stuffy air. Poor concentration, loss of attention, increased heart rate and slight nausea may also be present.
	// 5,000 ppm: this indicates unusual air conditions where high levels of other gases could also be present. Toxicity or oxygen deprivation could occur. This is the permissible exposure limit for daily workplace exposures.
	// 40,000 ppm: this level is immediately harmful due to oxygen deprivation.

	for( short i = AIRSHIFT_LED_POSITION_ONE; i <= AIRSHIFT_LED_POSITION_FOUR; i++ )
	{
		if( co2 < 1000 )
		{
			// Excellent
			airshift_led_color	= AIRSHIFT_LED_COLOR_GREEN;
			blink_leds_			= false;
        }
		else if( co2 < 1500 )
		{
            // Mediocre
			airshift_led_color	= AIRSHIFT_LED_COLOR_YELLOW;
			blink_leds_			= false;
        }
		else if( co2 < 2000 )
		{
            // Unhealthy
			airshift_led_color	= AIRSHIFT_LED_COLOR_ORANGE;
			blink_leds_			= false;
        }
		else if( co2 < 2500 )
		{
            // Very unhealthy
			airshift_led_color	= AIRSHIFT_LED_COLOR_RED;
			blink_leds_			= false;
        }
		else
		{
            // Hazardous. Blink if co2 is too high
			airshift_led_color = AIRSHIFT_LED_COLOR_PURPLE;
        }

		airshift_led_set( i, airshift_led_color );
    }

	// start blink task, if not already running ...
	if( airshift_led_color == AIRSHIFT_LED_COLOR_PURPLE )
	{
		if( blink_leds_ != true )
		{
			blink_leds_ = true;

			xTaskCreate( blink_leds_task, "blink_leds_task", 4096, NULL, 10, NULL );
		}
	}
}

static void blink_leds_task( void* arguments )
{
	bool					off					= true;
	airshift_led_color_t	airshift_led_color	= AIRSHIFT_LED_COLOR_BLACK;

	ESP_LOGI( TAG, "blink_leds_task" );

	while( blink_leds_ )
	{
		vTaskDelay( 500 / portTICK_PERIOD_MS );

		if( off == true )
		{
			airshift_led_color = AIRSHIFT_LED_COLOR_BLACK;
		}
		else
		{
			airshift_led_color = AIRSHIFT_LED_COLOR_YELLOW;
		}

		for( short i = AIRSHIFT_LED_POSITION_ONE; i <= AIRSHIFT_LED_POSITION_FOUR; i++ )
		{
			airshift_led_set( i, airshift_led_color );
		}

		off = !off;
	}

	vTaskDelete( NULL );
}

// chip-tool payload generate-qrcode --discriminator 3840 --setup-pin-code 20202021 --vendor-id 0xFFF1 --product-id 0x8000 --version 0 --commissioning-mode 0 --rendezvous 2

// modified to turn off CHIP logging: /Volumes/RaidData/BuildTools/espressif/esp-matter/connectedhomeip/connectedhomeip/config/esp32/components/chip/CMakeLists.txt