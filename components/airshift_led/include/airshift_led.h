#ifndef AIRSHIFT_LED_H
#define AIRSHIFT_LED_H

#include "airshift_header_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    AIRSHIFT_LED_POSITION_ONE,
    AIRSHIFT_LED_POSITION_TWO,
    AIRSHIFT_LED_POSITION_THREE,
    AIRSHIFT_LED_POSITION_FOUR
} airshift_led_position_t;

typedef enum
{
    AIRSHIFT_LED_COLOR_GREEN = 1,
    AIRSHIFT_LED_COLOR_YELLOW,
    AIRSHIFT_LED_COLOR_ORANGE,
    AIRSHIFT_LED_COLOR_RED,
    AIRSHIFT_LED_COLOR_BLACK,
    AIRSHIFT_LED_COLOR_PURPLE
} airshift_led_color_t;

esp_err_t   airshift_led_init();
esp_err_t   airshift_led_release();

esp_err_t   airshift_led_set( airshift_led_position_t airshift_led_position, airshift_led_color_t airshift_led_color );
esp_err_t   airshift_led_reset();

#ifdef __cplusplus
}
#endif

#endif // AIRSHIFT_LED_H
