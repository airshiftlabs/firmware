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

static TFT_eSPI tft = TFT_eSPI(TFT_WIDTH, TFT_HEIGHT);

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[TFT_WIDTH * TFT_HEIGHT / 10];

lv_obj_t *arc_co2;
lv_obj_t *label_co2_value;
lv_obj_t *label_co2_name;
lv_obj_t *label_temp;
lv_obj_t *label_pm;
lv_obj_t *label_air;

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
static void lvgl_ui_init(void);

/******************************************************************************/

/**
 * @brief  Task for update lvgl
 */
static void lvgl_task(void *arg){
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * TFT_HEIGHT / 10);

    /* Initialize the display */
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /* Initialize the (dummy) input device driver */
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_NONE;
    lv_indev_drv_register(&indev_drv);

    /* Create UI */
    lvgl_ui_init();

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

/**
 * @brief  Initialize UI from squareline output
 */
static void lvgl_ui_init(void){
    lv_obj_t *ui_home = lv_scr_act();

    lv_obj_set_style_bg_color(ui_home, lv_color_hex(0x707585), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_home, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_home, lv_color_hex(0x455060), LV_PART_MAIN | LV_STATE_DEFAULT );

    arc_co2 = lv_arc_create(ui_home);
    lv_obj_set_width(arc_co2, 150);
    lv_obj_set_height(arc_co2, 150);
    lv_obj_set_x(arc_co2, 0 );
    lv_obj_set_y(arc_co2, 35 );
    lv_obj_set_align(arc_co2, LV_ALIGN_TOP_MID );
    lv_obj_set_style_radius(arc_co2, 350, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(arc_co2, lv_color_hex(0x1E232D), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(arc_co2, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(arc_co2, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(arc_co2, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(arc_co2, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(arc_co2, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(arc_co2, lv_color_hex(0x0F1215), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_arc_opa(arc_co2, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(arc_co2, 8, LV_PART_MAIN| LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(arc_co2, lv_color_hex(0x36B9F6), LV_PART_INDICATOR | LV_STATE_DEFAULT );
    lv_obj_set_style_arc_opa(arc_co2, 255, LV_PART_INDICATOR| LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(arc_co2, 8, LV_PART_INDICATOR| LV_STATE_DEFAULT);

    label_co2_value = lv_label_create(arc_co2);
    lv_obj_set_width( label_co2_value, LV_SIZE_CONTENT);
    lv_obj_set_height( label_co2_value, LV_SIZE_CONTENT);
    lv_obj_set_align( label_co2_value, LV_ALIGN_CENTER );
    lv_label_set_text(label_co2_value,"1000");
    lv_obj_set_style_text_color(label_co2_value, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(label_co2_value, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_co2_value, &lv_font_montserrat_28, LV_PART_MAIN| LV_STATE_DEFAULT);

    label_co2_name = lv_label_create(arc_co2);
    lv_obj_set_width( label_co2_name, LV_SIZE_CONTENT);
    lv_obj_set_height( label_co2_name, LV_SIZE_CONTENT);
    lv_obj_set_x( label_co2_name, 0 );
    lv_obj_set_y( label_co2_name, 50 );
    lv_obj_set_align( label_co2_name, LV_ALIGN_CENTER );
    lv_label_set_text(label_co2_name,"CO2");
    lv_obj_set_style_text_color(label_co2_name, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(label_co2_name, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    label_temp = lv_label_create(ui_home);
    lv_obj_set_width( label_temp, LV_SIZE_CONTENT);
    lv_obj_set_height( label_temp, LV_SIZE_CONTENT);
    lv_obj_set_x( label_temp, -45 );
    lv_obj_set_y( label_temp, 195 );
    lv_obj_set_align( label_temp, LV_ALIGN_TOP_MID );
    lv_label_set_text(label_temp,"T: 36");
    lv_obj_set_style_text_color(label_temp, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(label_temp, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    label_pm = lv_label_create(ui_home);
    lv_obj_set_width( label_pm, LV_SIZE_CONTENT);
    lv_obj_set_height( label_pm, LV_SIZE_CONTENT);
    lv_obj_set_x( label_pm, 40 );
    lv_obj_set_y( label_pm, 195 );
    lv_obj_set_align( label_pm, LV_ALIGN_TOP_MID );
    lv_label_set_text(label_pm,"PM: 34");
    lv_obj_set_style_text_color(label_pm, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(label_pm, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    label_air = lv_label_create(ui_home);
    lv_obj_set_width( label_air, LV_SIZE_CONTENT);
    lv_obj_set_height( label_air, LV_SIZE_CONTENT);
    lv_obj_set_x( label_air, 0 );
    lv_obj_set_y( label_air, 10 );
    lv_obj_set_align( label_air, LV_ALIGN_TOP_MID );
    lv_label_set_text(label_air,"Air Sensor");
    lv_obj_set_style_text_color(label_air, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(label_air, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
}

/******************************************************************************/

/**
 * @brief  Display image screen
 */
void lvgl_gui_sensor_start(void){
    lv_init();
    lvgl_tick_init();

    /* Clear LCD */
    tft.fillScreen(TFT_BLACK);

    xTaskCreate(lvgl_task, "LVGL", 10240, NULL, 4, NULL);
}

/**
 * @brief  Print sensor datas
 */
void lvgl_gui_print(int co2, float temp, int pm2){
    static char temp_str[8];
    static char pm_str[8];
    static char co2_str[8];

    uint8_t temp_value = (uint8_t) temp;
    temp_value = (temp_value > TEMP_MAX) ? TEMP_MAX : temp_value;
    int co2_value = (co2 > CO2_MAX) ? CO2_MAX : co2;
    int pm_value = (pm2 > PM2_MAX) ? PM2_MAX : pm2;

    lv_arc_set_value(arc_co2, co2_value * 100 / CO2_MAX);
    sprintf(co2_str, "%d", co2_value);
    lv_label_set_text(label_co2_value, co2_str);

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