#ifndef AIRSHIFT_UI_COMMON_H
#define AIRSHIFT_UI_COMMON_H

#include "airshift_header_common.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t   airshift_ui_init();
esp_err_t   airshift_ui_release();

esp_err_t   airshift_ui_display_qrcode();
esp_err_t   airshift_ui_display_main();

void        airshift_ui_set_co2( uint16_t value );
void        airshift_ui_set_temperature( float value );
void        airshift_ui_set_particulate_matter( uint16_t value );

#ifdef __cplusplus
}
#endif

#endif // AIRSHIFT_UI_COMMON_H