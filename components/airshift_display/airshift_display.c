#include "airshift_display.h"

#include <esp_timer.h>

#define HORIZONTAL_RESOLUTION   240
#define VERTICAL_RESOLUTION     240

#define TICK_TIMER_MS           5000

static const char* TAG = "airshift_display";

// Forward declarations
static esp_err_t    display_driver_init();
static esp_err_t    tick_timer_init();
static void         tick_timer_callback( void *data );

// Public functions
esp_err_t airshift_display_init()
{
    ESP_LOGI( TAG, "airshift_display_init" );

    lv_init();

    lvgl_driver_init();

    ESP_RETURN_ON_ERROR( display_driver_init(), TAG, "display_driver_init failed" );

    return ESP_OK;
}

esp_err_t airshift_display_release()
{
    ESP_LOGI( TAG, "airshift_display_release" );

    return ESP_OK;
}

// Private functions
static esp_err_t display_driver_init()
{
    esp_err_t                   ret                 = ESP_FAIL;
    static lv_disp_draw_buf_t   lv_disp_draw_buf    = { 0 };
    static lv_disp_drv_t        lv_disp_drv         = { 0 };
    lv_color_t                  *lv_color_buf_1     = NULL;
    lv_color_t                  *lv_color_buf_2     = NULL;
    lv_disp_t                   *lv_disp            = NULL;

    ESP_LOGI( TAG, "display_driver_init" );

    lv_color_buf_1 = heap_caps_malloc( DISP_BUF_SIZE * sizeof( lv_color_t ), MALLOC_CAP_DMA );

    ESP_GOTO_ON_FALSE( ( lv_color_buf_1 != NULL ), ret, error, TAG, "heap_caps_malloc -> lv_color_buf_1 failed" );

    lv_color_buf_2 = heap_caps_malloc( DISP_BUF_SIZE * sizeof( lv_color_t ), MALLOC_CAP_DMA );

    ESP_GOTO_ON_FALSE( ( lv_color_buf_2 != NULL ), ret, error, TAG, "heap_caps_malloc -> lv_color_buf_2 failed" );
    
    lv_disp_draw_buf_init( &lv_disp_draw_buf, lv_color_buf_1, lv_color_buf_2, DISP_BUF_SIZE );
    
    lv_disp_drv_init( &lv_disp_drv );
    
    lv_disp_drv.flush_cb           = disp_driver_flush;
    lv_disp_drv.hor_res            = HORIZONTAL_RESOLUTION;
    lv_disp_drv.ver_res            = VERTICAL_RESOLUTION;
    lv_disp_drv.physical_hor_res   = -1;
    lv_disp_drv.physical_ver_res   = -1;
    lv_disp_drv.antialiasing       = true;
    lv_disp_drv.screen_transp      = false;
    lv_disp_drv.draw_buf           = &lv_disp_draw_buf;
    
    lv_disp = lv_disp_drv_register( &lv_disp_drv );

    ESP_GOTO_ON_FALSE( ( lv_disp != NULL ), ret, error, TAG, "lv_disp_drv_register failed" );

    ESP_GOTO_ON_ERROR( tick_timer_init(), error, TAG, "tick_timer_init failed" );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "display_driver_init failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

	return ret;
}

static esp_err_t tick_timer_init()
{
    esp_err_t                       ret                     = ESP_FAIL;
    esp_timer_handle_t              esp_timer_handle        = NULL;
    const esp_timer_create_args_t   esp_timer_create_args   = { .callback = tick_timer_callback, .name = "tick_timer_init", .arg = NULL, .dispatch_method = ESP_TIMER_TASK, .skip_unhandled_events = true };

    ESP_LOGI( TAG, "tick_timer_init" );

    ESP_GOTO_ON_ERROR( esp_timer_create( &esp_timer_create_args, &esp_timer_handle ), error, TAG, "esp_timer_create failed" );

    ESP_GOTO_ON_ERROR( esp_timer_start_periodic( esp_timer_handle, TICK_TIMER_MS ), error, TAG, "esp_timer_start_once failed" );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "tick_timer_init failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

	return ret;
}

static void tick_timer_callback( void *data )
{
    lv_tick_inc( ( TICK_TIMER_MS / 1000 ) );
}
