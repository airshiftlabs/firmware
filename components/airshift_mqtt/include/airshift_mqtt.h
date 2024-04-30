#ifndef AIRSHIFT_MQTT_H
#define AIRSHIFT_MQTT_H

#include "airshift_header_common.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t   airshift_mqtt_init();
esp_err_t   airshift_mqtt_release();

esp_err_t   airshift_mqtt_start();
esp_err_t   airshift_mqtt_publish( const char *topic, const char *message, size_t message_len );
const char  *airshift_mqtt_get_client_id();

#ifdef __cplusplus
}
#endif

#endif // AIRSHIFT_MQTT_H
