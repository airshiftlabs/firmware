#ifndef AIRSHIFT_EVENT_H
#define AIRSHIFT_EVENT_H

#include "airshift_header_common.h"

#include <esp_event.h>

#ifdef __cplusplus
extern "C" {
#endif

ESP_EVENT_DECLARE_BASE( AIRSHIFT_EVENT_MATTER );
ESP_EVENT_DECLARE_BASE( AIRSHIFT_EVENT_GENERAL );

// Added numbers for easier readability ...
typedef enum {
    AIRSHIFT_EVENT_MATTER_COMMISSIONING_WINDOW_OPENED   = 1,
    AIRSHIFT_EVENT_MATTER_COMMISSIONING_WINDOW_CLOSED   = 2,
    AIRSHIFT_EVENT_MATTER_IP_EVENT_STA_GOT_IP           = 3
} airshift_event_matter_t;

typedef enum {
    AIRSHIFT_EVENT_GENERAL_REBOOT_NEEDED                = 999
} airshift_event_general_t;

esp_err_t airshift_event_init();
esp_err_t airshift_event_release();

#ifdef __cplusplus
}
#endif

#endif // AIRSHIFT_EVENT_H
