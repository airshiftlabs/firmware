
#include <Arduino.h>
#include <mqtt_client.h>
#include "mqtt_api.h"

#define MQTT_BROKER "mqtt://mqtt.airshift.app"
#define MQTT_PORT 1883

static const char *TAG = "MQTT";

static char client_id[32];
static esp_mqtt_client_handle_t mqtt_client;
static bool mqtt_broker_connected = false;

static esp_err_t mqtt_client_event_handler(esp_mqtt_event_handle_t event) {
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            mqtt_broker_connected = true;
            ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "MQTT_EVENT_CONNECTED");
            break;

        case MQTT_EVENT_DISCONNECTED:
            mqtt_broker_connected = false;
            ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "MQTT_EVENT_DATA");
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "MQTT_EVENT_ERROR");
            break;

        default:
            break;
    }
    return ESP_OK;
}

void mqtt_api_publish(int co2, float temp, int pm2, int rh) {
    char topic[64];
    char message[32];
    int32_t msg_id;

    if (mqtt_broker_connected) {
        /* CO2 */
        sprintf(topic, "co2/%s", client_id);
        sprintf(message, "%d", co2);
        esp_mqtt_client_publish(mqtt_client, topic, message, strlen(message), 0, 0);

        /* Temperature */
        sprintf(topic, "temp/%s", client_id);
        sprintf(message, "%.01f", temp);
        esp_mqtt_client_publish(mqtt_client, topic, message, strlen(message), 0, 0);

        /* Humidity */
        sprintf(topic, "rh/%s", client_id);
        sprintf(message, "%d", rh);
        esp_mqtt_client_publish(mqtt_client, topic, message, strlen(message), 0, 0);

        /* PM */
        sprintf(topic, "pm2.5/%s", client_id);
        sprintf(message, "%d", pm2);
        msg_id = esp_mqtt_client_publish(mqtt_client, topic, message, strlen(message), 0, 0);
    }
}

void mqtt_api_init(void) {
    /* Use wifi mac default */
    uint8_t mac_addr[6];
    esp_read_mac(mac_addr, ESP_MAC_WIFI_STA);
    sprintf(client_id, "%02X%02X%02X%02X%02X%02X", mac_addr[0],
            mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    esp_mqtt_client_config_t mqtt_cfg;
    memset(&mqtt_cfg, 0, sizeof(mqtt_cfg));
    mqtt_cfg.uri = MQTT_BROKER;
    mqtt_cfg.port = MQTT_PORT;
    mqtt_cfg.client_id = client_id;
    mqtt_cfg.event_handle = mqtt_client_event_handler;
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    /* Start MQTT client */
    ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "Start MQTT client");
    esp_mqtt_client_start(mqtt_client);
}
