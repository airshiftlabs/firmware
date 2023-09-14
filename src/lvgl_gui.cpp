/*
 *  lvgl_gui.c
 *
 *  Created on: May 5, 2023
 */
/* http://tomeko.net/online_tools/file_to_hex.php?lang=en */

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <lv_widgets/lv_gauge.h>
#include <TJpg_Decoder.h>
#include "qrcode.h"
#include "lvgl_gui.h"
#include "board_conf.h"

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

#define LVGL_TICK_HANDLER                5
#define TEMP_MAX                         100
#define CO2_MAX                          2000
#define PM2_MAX                          1000

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

static TFT_eSPI tft = TFT_eSPI();

lv_obj_t *gauge_co2;
lv_obj_t *label_temp;
lv_obj_t *label_pm;

static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/



/******************************************************************************/
/*                                FUNCTIONS                                   */
/******************************************************************************/

static void lvgl_task(void *arg);
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
static void lv_tick_inc_cb(void *data);
static void lvgl_tick_init(void);

/******************************************************************************/

/**
 * @brief  Task for update lvgl
 */
static void lvgl_task(void *arg){
    while(1){
        lv_task_handler();    /* Let the GUI do its work */
        delay(LVGL_TICK_HANDLER);
    }
}

/**
 * @brief  Display flushing
 */
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p){
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors(&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/**
 * @brief  Increase tick for lvgl task
 */
static void lv_tick_inc_cb(void *data){
    uint32_t tick_inc_period_ms = *((uint32_t *) data);
    lv_tick_inc(tick_inc_period_ms);
}

/**
 * @brief  Create timer to increase tick
 */
static void lvgl_tick_init(void){
    static const uint32_t tick_inc_period_ms = LVGL_TICK_HANDLER;
    static esp_timer_create_args_t periodic_timer_args;

    periodic_timer_args.callback = lv_tick_inc_cb;
    periodic_timer_args.name = "tick";     /* name is optional, but may help identify the timer when debugging */
    periodic_timer_args.arg = (void *) &tick_inc_period_ms;
    periodic_timer_args.dispatch_method = ESP_TIMER_TASK;
    periodic_timer_args.skip_unhandled_events = true;

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));

    /* The timer has been created but is not running yet. Start the timer now */
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, tick_inc_period_ms * 1000));
}

/******************************************************************************/

/**
 * @brief  Display image screen
 */
void lvgl_gui_sensor_start(void){
    lv_init();
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
    lvgl_tick_init();

    /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_NONE;
    lv_indev_drv_register(&indev_drv);

    /* Describe the color for the needles */
    static lv_color_t needle_colors[3];
    needle_colors[0] = LV_COLOR_BLUE;
    needle_colors[1] = LV_COLOR_RED;
    needle_colors[2] = LV_COLOR_GREEN;

    /* Clear LCD */
    tft.fillScreen(TFT_BLACK);

    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label, "Air Sensor");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

    /* Gauge CO2 */
    gauge_co2 = lv_gauge_create(lv_scr_act(), NULL);
    lv_gauge_set_needle_count(gauge_co2, 1, &needle_colors[1]);
    lv_obj_set_size(gauge_co2, 170, 170);
    lv_obj_align(gauge_co2, NULL, LV_ALIGN_IN_TOP_MID, 0, 30);
    lv_gauge_set_range(gauge_co2, 0, CO2_MAX);
    lv_gauge_set_scale(gauge_co2, 270, 17, 5);

    lv_obj_t *label_co2 = lv_label_create(gauge_co2, NULL);
    lv_label_set_text(label_co2, "CO2");
    lv_obj_align(label_co2, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);

    label_temp = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label_temp, "T: 20");
    lv_obj_align(label_temp, NULL, LV_ALIGN_IN_TOP_LEFT, 70, 205);

    label_pm = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label_pm, "PM: 123");
    lv_obj_align(label_pm, NULL, LV_ALIGN_IN_TOP_LEFT, 120, 205);

    xTaskCreate(lvgl_task, "LVGL", 4096, NULL, 4, NULL);
}

/**
 * @brief  Print sensor datas
 */
void lvgl_gui_print(int co2, float temp, int pm2){
    static char temp_str[8];
    static char pm_str[8];

    uint8_t temp_value = (uint8_t) temp;
    temp_value = (temp_value > TEMP_MAX) ? TEMP_MAX : temp_value;
    int co2_value = (co2 > CO2_MAX) ? CO2_MAX : co2;
    int pm_value = (pm2 > PM2_MAX) ? PM2_MAX : pm2;

    /* Display LCD */
    lv_gauge_set_value(gauge_co2, 0, co2_value);

    sprintf(temp_str, "T: %d", temp_value);
    lv_label_set_text(label_temp, temp_str);
    sprintf(pm_str, "PM: %d", pm2);
    lv_label_set_text(label_pm, pm_str);
}

/* Output data to LCD callback */
static bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (y >= tft.height()) {
        return false;
    }

    tft.pushImage(x, y, w, h, bitmap);
    return true;
}

/**
 * @brief  Initialize LVGL for gui
 */
void lvgl_gui_init(void){
    pinMode(CONFIG_TFT_PIN_RST, OUTPUT);
    digitalWrite(CONFIG_TFT_PIN_RST, LOW);
    delay(10);
    digitalWrite(CONFIG_TFT_PIN_RST, HIGH);
    delay(1000);

    tft.init();
    tft.setRotation(2);

    tft.setSwapBytes(true);
    TJpgDec.setJpgScale(1);
    TJpgDec.setCallback(tft_output);

    tft.startWrite();
    TJpgDec.drawJpg(0, 0, qrcode, sizeof(qrcode));
    tft.endWrite();
    tft.setSwapBytes(false);
}