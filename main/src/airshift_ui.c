#include "airshift_ui.h"
#include "airshift_display.h"

LV_IMG_DECLARE( qrcode )

static const char* TAG = "airshift_ui";

// Forward declarations

static lv_obj_t *arc_co2_           = NULL;
static lv_obj_t *label_co2_value_   = NULL;
static lv_obj_t *label_temp_        = NULL;
static lv_obj_t *label_pm_          = NULL;

// Public functions
esp_err_t airshift_ui_init()
{
    ESP_LOGI( TAG, "airshift_ui_init" );

    return ESP_OK;
}

esp_err_t airshift_ui_release()
{
    ESP_LOGI( TAG, "airshift_ui_release" );

    return ESP_OK;
}

esp_err_t airshift_ui_display_qrcode()
{
    lv_obj_t *image = NULL;

    ESP_LOGI( TAG, "airshift_ui_display_qrcode" );

    lv_obj_set_style_bg_opa( lv_scr_act(), LV_OPA_100, LV_PART_MAIN );
    lv_obj_set_style_bg_color( lv_scr_act(), lv_color_white(), LV_PART_MAIN );
    
    image = lv_img_create( lv_scr_act() );

    lv_img_set_src( image, &qrcode );

    lv_obj_center( image );

    return ESP_OK;
}

esp_err_t airshift_ui_display_main()
{
    ESP_LOGI( TAG, "airshift_ui_display_main" );

    lv_obj_t *ui_home = lv_scr_act();
    lv_obj_t *label_co2_name;
    lv_obj_t *label_air;

    lv_obj_set_style_bg_color(ui_home, lv_color_hex(0x707585), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(ui_home, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_home, lv_color_hex(0x455060), LV_PART_MAIN | LV_STATE_DEFAULT );

    arc_co2_ = lv_arc_create(ui_home);
    lv_arc_set_value( arc_co2_, 0 );
    lv_obj_set_width(arc_co2_, 150);
    lv_obj_set_height(arc_co2_, 150);
    lv_obj_set_x(arc_co2_, 0 );
    lv_obj_set_y(arc_co2_, 35 );
    lv_obj_set_align(arc_co2_, LV_ALIGN_TOP_MID );
    lv_obj_set_style_radius(arc_co2_, 350, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(arc_co2_, lv_color_hex(0x1E232D), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(arc_co2_, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(arc_co2_, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(arc_co2_, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(arc_co2_, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(arc_co2_, 8, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(arc_co2_, lv_color_hex(0x0F1215), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_arc_opa(arc_co2_, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(arc_co2_, 8, LV_PART_MAIN| LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(arc_co2_, lv_color_hex(0x36B9F6), LV_PART_INDICATOR | LV_STATE_DEFAULT );
    lv_obj_set_style_arc_opa(arc_co2_, 255, LV_PART_INDICATOR| LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(arc_co2_, 8, LV_PART_INDICATOR| LV_STATE_DEFAULT);

    label_co2_value_ = lv_label_create(arc_co2_);
    lv_obj_set_width( label_co2_value_, LV_SIZE_CONTENT);
    lv_obj_set_height( label_co2_value_, LV_SIZE_CONTENT);
    lv_obj_set_align( label_co2_value_, LV_ALIGN_CENTER );
    lv_label_set_text(label_co2_value_,"n/a");
    lv_obj_set_style_text_color(label_co2_value_, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(label_co2_value_, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_co2_value_, &lv_font_montserrat_28, LV_PART_MAIN| LV_STATE_DEFAULT);

    label_co2_name = lv_label_create(arc_co2_);
    lv_obj_set_width( label_co2_name, LV_SIZE_CONTENT);
    lv_obj_set_height( label_co2_name, LV_SIZE_CONTENT);
    lv_obj_set_x( label_co2_name, 0 );
    lv_obj_set_y( label_co2_name, 50 );
    lv_obj_set_align( label_co2_name, LV_ALIGN_CENTER );
    lv_label_set_text(label_co2_name,"CO2");
    lv_obj_set_style_text_color(label_co2_name, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(label_co2_name, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    label_temp_ = lv_label_create(ui_home);
    lv_obj_set_width( label_temp_, LV_SIZE_CONTENT);
    lv_obj_set_height( label_temp_, LV_SIZE_CONTENT);
    lv_obj_set_x( label_temp_, -45 );
    lv_obj_set_y( label_temp_, 195 );
    lv_obj_set_align( label_temp_, LV_ALIGN_TOP_MID );
    lv_label_set_text(label_temp_,"T: n/a");
    lv_obj_set_style_text_color(label_temp_, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(label_temp_, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    label_pm_ = lv_label_create(ui_home);
    lv_obj_set_width( label_pm_, LV_SIZE_CONTENT);
    lv_obj_set_height( label_pm_, LV_SIZE_CONTENT);
    lv_obj_set_x( label_pm_, 40 );
    lv_obj_set_y( label_pm_, 195 );
    lv_obj_set_align( label_pm_, LV_ALIGN_TOP_MID );
    lv_label_set_text(label_pm_,"PM: n/a");
    lv_obj_set_style_text_color(label_pm_, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(label_pm_, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    label_air = lv_label_create(ui_home);
    lv_obj_set_width( label_air, LV_SIZE_CONTENT);
    lv_obj_set_height( label_air, LV_SIZE_CONTENT);
    lv_obj_set_x( label_air, 0 );
    lv_obj_set_y( label_air, 10 );
    lv_obj_set_align( label_air, LV_ALIGN_TOP_MID );
    lv_label_set_text(label_air,"Air Sensor");
    lv_obj_set_style_text_color(label_air, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(label_air, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

    return ESP_OK;
}

void airshift_ui_set_co2( uint16_t value )
{
    char    label_text[10]  = { 0 };
    float   percentage      = ( ( (float)value / (float)2500 ) * (float)100 ); // 2500 (unhealthy) == 100%

    ESP_LOGI( TAG, "airshift_ui_set_co2" );

    snprintf( label_text, sizeof( label_text ), "%hu", value );

    lv_label_set_text( label_co2_value_, label_text );

    lv_arc_set_value( arc_co2_, (int)percentage ); 
}

void airshift_ui_set_temperature( float value )
{
    char label_text[15] = { 0 };

    ESP_LOGI( TAG, "airshift_ui_set_temperature" );

    snprintf( label_text, sizeof( label_text ), "T: %.0f C", value );

    lv_label_set_text( label_temp_, label_text );
}

void airshift_ui_set_particulate_matter( uint16_t value )
{
    char label_text[15] = { 0 };

    ESP_LOGI( TAG, "airshift_ui_set_particulate_matter" );

    snprintf( label_text, sizeof( label_text ), "PM: %hu", value );

    lv_label_set_text( label_pm_, label_text );
}

// Private functions
