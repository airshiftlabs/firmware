#ifndef AIRSHIFT_DISPLAY_H
#define AIRSHIFT_DISPLAY_H

#include "airshift_header_common.h"

#include <lvgl.h>
#include <lvgl_helpers.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t   airshift_display_init();
esp_err_t   airshift_display_release();

#ifdef __cplusplus
}
#endif

#endif // AIRSHIFT_DISPLAY_H
