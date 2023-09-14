/*
 *  main.cpp
 *
 *  Created on: Jul 6, 2023
 */

/******************************************************************************/

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/

#include <Arduino.h>
#include <Matter.h>
#include <ESP32Utils.h>
#include <app/server/OnboardingCodesUtil.h>
#include <AirGradient.h>
#include <FastLED.h>
// #include <Setup70e_ESP32_GC9A01.h>
#include "board_conf.h"
#include "lvgl_gui.h"
#include "mqtt_api.h"

using namespace chip;
using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace esp_matter::endpoint;

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

#define NUM_LEDS 4

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

static AirGradient ag = AirGradient();
static CRGB leds[NUM_LEDS];

const char *TAG = "MAIN";

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/

/******************************************************************************/
/*                                FUNCTIONS                                   */
/******************************************************************************/

/******************************************************************************/

/* There is possibility to listen for various device events, related for example to setup process */
static void matter_event_cb(const ChipDeviceEvent *event, intptr_t arg) {
    switch (event->Type) {
        case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
            // Serial.println( "Interface IP Address changed");
            Serial.println("on_identification");

            break;

        case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
            Serial.println("Commissioning complete");
            break;

        case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
            Serial.println("Commissioning failed, fail safe timer expired");
            break;

        case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
            Serial.println("Commissioning session started");
            break;

        case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
            Serial.println("Commissioning session stopped");
            break;

        case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
            Serial.println("Commissioning window opened");
            break;

        case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
            Serial.println("Commissioning window closed");
            break;

        case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
            Serial.println("Fabric removed successfully");
            // if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
            //     chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
            //     constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
            //     if (!commissionMgr.IsCommissioningWindowOpen()) {
            //         /* After removing last fabric, this example does not remove the Wi-Fi credentials
            //          * and still has IP connectivity so, only advertising on DNS-SD.
            //          */
            //         CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
            //                                                                     chip::CommissioningWindowAdvertisement::kDnssdOnly);
            //         if (err != CHIP_NO_ERROR) {
            //             ESP_LOGE(TAG, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
            //         }
            //     }
            // }
            // break;
        }

        case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
            Serial.println("Fabric will be removed");
            break;

        case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
            Serial.println("Fabric is updated");
            break;

        case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
            Serial.println("Fabric is committed");
            break;
        default:
            Serial.println("Unknown event");
            break;
    }
}

static esp_err_t on_identification(identification::callback_type_t type, uint16_t endpoint_id,
                                   uint8_t effect_id, void *priv_data) {
    Serial.println("on_identification");
    return ESP_OK;
}

/* Listener on attribute update requests
 * In this example, when update is requested, path (endpoint, cluster and attribute) is checked
 */
static esp_err_t on_attribute_update(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                     uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data) {
    Serial.println("on_attribute_update");
    return ESP_OK;
}

static void matter_init(void) {
    /* Setup Matter node */
    node::config_t node_config;
    node_t *node = node::create(&node_config, on_attribute_update, on_identification);

    /* Start Matter device */
    esp_matter::start(matter_event_cb);

    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::init();
}

static void update_led(int co2) {
    for (size_t i = 0; i < NUM_LEDS; i++) {
        // 400 ppm: average outdoor air level.
        // 400–1,000 ppm: typical level found in occupied spaces with good air exchange.
        // 1,000–2,000 ppm: level associated with complaints of drowsiness and poor air.
        // 2,000–5,000 ppm: level associated with headaches, sleepiness, and stagnant, stale, stuffy air. Poor concentration, loss of attention, increased heart rate and slight nausea may also be present.
        // 5,000 ppm: this indicates unusual air conditions where high levels of other gases could also be present. Toxicity or oxygen deprivation could occur. This is the permissible exposure limit for daily workplace exposures.
        // 40,000 ppm: this level is immediately harmful due to oxygen deprivation.

        if (co2 < 1000) {
            // Excellent
            leds[i] = CRGB::Green;
        } else if (co2 < 1500) {
            // Mediocre
            leds[i] = CRGB::Yellow;
        } else if (co2 < 2000) {
            // Unhealthy
            leds[i] = CRGB::Orange;
        } else if (co2 < 2500) {
            // Very unhealthy
            leds[i] = CRGB::Red;
        } else {
            // Hazardous
            // blink if co2 is too high
            if (millis() % 1000 < 500) {
                leds[i] = CRGB::Black;
            } else {
                leds[i] = CRGB::Purple;
            }
        }

        FastLED.show();
        // todo fade to values
    }
}

/******************************************************************************/

void setup() {
    /* Enable debug logging */
    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "Power up");

    /* LCD blacklight init */
    pinMode(TFT_BACKLIGHT, OUTPUT);
    digitalWrite(TFT_BACKLIGHT, HIGH);
    delay(1000);
    digitalWrite(TFT_BACKLIGHT, LOW);
    delay(1000);
    digitalWrite(TFT_BACKLIGHT, HIGH);

    /* LED init */
    FastLED.addLeds<SK6812, DATA_PIN, GRB>(leds, NUM_LEDS);

    /* LCD init */
    lvgl_gui_init();

    /* Sensors init */
    ag.TMP_RH_Init(0x44, CONFIG_SHT30_PIN_SDA, CONFIG_SHT30_PIN_SCL);
    ag.CO2_Init(CONFIG_CO2_PIN_RX, CONFIG_CO2_PIN_TX);
    ag.PMS_Init(CONFIG_PMS_PIN_RX, CONFIG_PMS_PIN_TX);

    /* Matter start */
    matter_init();

    bool connected = false;
    int retries = 0;

    // chip::DeviceLayer::Internal::ESP32Utils::StartWiFiLayer();
    // chip::DeviceLayer::Internal::ESP32Utils::EnableStationMode();

    /* Print codes needed to setup Matter device */
    char payloadBuffer[256];
    chip::MutableCharSpan qrCode(payloadBuffer);
    if (GetQRCode(qrCode, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE)) == CHIP_NO_ERROR) {
        // GetQRCodeUrl(url, sizeof(url), qrCode);
        Serial.print("Matter QR Code: ");
        Serial.println(qrCode.data());
    }

    do {
        chip::DeviceLayer::Internal::ESP32Utils::IsStationConnected(connected);
        if (connected) {
            break;
        }
        Serial.println("Waiting for provisioning. " + String(retries) + " retries.");
        retries++;
        delay(1000);
    } while ((!connected) && (retries < 60));

    if (!connected) {
        Serial.println("!!!WARNING: Not provisioned after 60s. Resetting device.");
        esp_matter::factory_reset();
    }

    Serial.println("Finished provisioning");

    lvgl_gui_sensor_start();

    /* MQTT init */
    mqtt_api_init();
}

void loop() {
    static uint32_t last_print_ms = millis();
    int PM2, Co2;

    TMP_RH result = ag.periodicFetchData();
    PM2 = ag.getPM2_Raw();
    Co2 = ag.getCO2();

    /* Print and publish every 10 seconds */
    uint32_t elapsed = (millis() - last_print_ms) & 0xFFFFFFFF;
    if (elapsed > 10000) {
        last_print_ms = millis();
        ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "----");
        ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "Relative Humidity in %: %d", result.rh);
        ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "Temperature in Celsius: %.01f", result.t);
        ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "Temperature in Fahrenheit: %.01f", (result.t * 9 / 5) + 32);
        ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "PM2.5 in ug/m3: %d", PM2);
        ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "Co2: %d", Co2);
        ESP_LOG_LEVEL(ESP_LOG_INFO, TAG, "----");
        mqtt_api_publish(Co2, result.t, PM2, result.rh);
    }

    /* Display the sensors result on the LCD */
    lvgl_gui_print(Co2, result.t, PM2);
    update_led(Co2);
}
