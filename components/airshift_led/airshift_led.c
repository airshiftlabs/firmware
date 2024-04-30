#include "airshift_led.h"
#include <driver/gpio.h>
#include <led_strip.h>

static const char* TAG = "airshift_led";

// Forward declarations

static led_strip_handle_t led_strip_handle_ = NULL;

// Public functions
esp_err_t airshift_led_init()
{
    esp_err_t               ret                     = ESP_FAIL;
    led_strip_config_t      led_strip_config        = { .strip_gpio_num = GPIO_NUM_39, .max_leds = 4 };
    led_strip_rmt_config_t  led_strip_rmt_config    = { .resolution_hz = 10 * 1000 * 1000 };

    ESP_LOGI( TAG, "airshift_led_init" );

    // led_strip_new_rmt_device ...
    ESP_GOTO_ON_ERROR( led_strip_new_rmt_device( &led_strip_config, &led_strip_rmt_config, &led_strip_handle_ ), error, TAG, "led_strip_new_rmt_device failed" );
    
    // airshift_led_reset ...
    ESP_GOTO_ON_ERROR( airshift_led_reset(), error, TAG, "airshift_led_reset failed" );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_led_init failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    // clean up on failure ...
    ESP_ERROR_CHECK_WITHOUT_ABORT( airshift_led_release() );

    return ret;
}

esp_err_t airshift_led_release()
{
    ESP_LOGI( TAG, "airshift_led_release" );

    if( led_strip_handle_ != NULL )
    {
        led_strip_del( led_strip_handle_ );

        led_strip_handle_ = NULL;
    }

    return ESP_OK;
}

esp_err_t airshift_led_set( airshift_led_position_t airshift_led_position, airshift_led_color_t airshift_led_color )
{
    uint32_t red    = 0;
    uint32_t green  = 0;
    uint32_t blue   = 0;

    ESP_LOGV( TAG, "airshift_led_set" ); // set to verbose, logs too much when blinking ...

    switch( airshift_led_color )
    {
        case AIRSHIFT_LED_COLOR_GREEN:{     red = 0x00, green = 0x80, blue = 0x00; break; }
        case AIRSHIFT_LED_COLOR_YELLOW:{    red = 0xFF, green = 0xFF, blue = 0x00; break; }
        case AIRSHIFT_LED_COLOR_ORANGE:{    red = 0xFF, green = 0x45, blue = 0x00; break; }
        case AIRSHIFT_LED_COLOR_RED:{       red = 0xFF, green = 0x00, blue = 0x00; break; }
        case AIRSHIFT_LED_COLOR_BLACK:{     red = 0x00, green = 0x00, blue = 0x00; break; }
        case AIRSHIFT_LED_COLOR_PURPLE:{    red = 0x80, green = 0x00, blue = 0x80; break; }
    }

    // led_strip_set_pixel ...
    ESP_RETURN_ON_ERROR( led_strip_set_pixel( led_strip_handle_, airshift_led_position, red, green, blue ), TAG, "led_strip_set_pixel failed" );

    // led_strip_refresh ...
    ESP_RETURN_ON_ERROR( led_strip_refresh( led_strip_handle_ ), TAG, "led_strip_refresh failed" );

    return ESP_OK;
}

esp_err_t airshift_led_reset()
{
    ESP_LOGI( TAG, "airshift_led_reset" );

    ESP_RETURN_ON_ERROR( led_strip_clear( led_strip_handle_ ), TAG, "led_strip_clear failed" );

    return ESP_OK;
}

// Private functions
