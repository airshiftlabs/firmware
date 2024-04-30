#include "airshift_nvs.h"

static const char* TAG = "airshift_nvs";

// Forward declarations
esp_err_t commit_and_close( nvs_handle_t nvs_handle );

// Public functions
esp_err_t airshift_nvs_init()
{
    esp_err_t ret = ESP_FAIL;

    ESP_LOGI( TAG, "airshift_nvs_init" );
    
    ret = nvs_flash_init();
    
    if( ( ret == ESP_ERR_NVS_NO_FREE_PAGES ) || ( ret == ESP_ERR_NVS_NEW_VERSION_FOUND ) )
    {
        ESP_GOTO_ON_ERROR( nvs_flash_erase(), error, TAG, "nvs_flash_erase failed" );

        ret = nvs_flash_init();
    }

    ESP_GOTO_ON_ERROR( ret, error, TAG, "nvs_flash_init failed" );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_nvs_init failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    return ret;
}

size_t airshift_nvs_available( size_t *total_entries, size_t *used_entries )
{
    esp_err_t   ret         = ESP_FAIL;
    nvs_stats_t nvs_stats   = { 0 };
   
    ESP_GOTO_ON_ERROR( nvs_get_stats( NULL, &nvs_stats ), error, TAG, "nvs_get_stats failed" );

    *total_entries  = nvs_stats.total_entries;
    *used_entries   = nvs_stats.used_entries;

    ESP_LOGI( TAG, "airshift_nvs_available -> total_entries: %d, used_entries: %d", *total_entries, *used_entries );
    
    return nvs_stats.free_entries;

error:

    ESP_LOGE( TAG, "airshift_nvs_available failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    return 0;
}

esp_err_t airshift_nvs_release()
{
    esp_err_t ret = ESP_FAIL;

    ESP_LOGI( TAG, "airshift_nvs_release" );
    
    ESP_GOTO_ON_ERROR( nvs_flash_deinit(), error, TAG, "nvs_flash_deinit failed" );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_nvs_release failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    return ret;
}

esp_err_t airshift_nvs_format()
{
    esp_err_t ret = ESP_FAIL;

    ESP_LOGI( TAG, "airshift_nvs_format" );

    ESP_GOTO_ON_ERROR( nvs_flash_erase(), error, TAG, "nvs_flash_erase failed" );

    return airshift_nvs_init();

error:

    ESP_LOGE( TAG, "airshift_nvs_format failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    return ret;
}

esp_err_t airshift_nvs_write_string( const char* namespace, const char* key, const char* value )
{
    esp_err_t       ret         = ESP_FAIL;
    nvs_handle_t    nvs_handle  = 0;

    ESP_LOGI( TAG, "airshift_nvs_write_string" );

    ESP_GOTO_ON_ERROR( nvs_open( namespace, NVS_READWRITE, &nvs_handle ), error, TAG, "nvs_open failed" );

    ESP_GOTO_ON_ERROR( nvs_set_str( nvs_handle, key, value ), error, TAG, "nvs_set_str failed" );

    return commit_and_close( nvs_handle );

error:

    ESP_LOGE( TAG, "airshift_nvs_write_string failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    // Cleanup in case of error after open ...
    if( nvs_handle != 0 )
    {
        nvs_close( nvs_handle );
    }

    return ret;
}

esp_err_t airshift_nvs_write_int16( const char* namespace, const char* key, int16_t value )
{
    esp_err_t       ret         = ESP_FAIL;
    nvs_handle_t    nvs_handle  = 0;

    ESP_LOGI( TAG, "airshift_nvs_write_int16" );

    ESP_GOTO_ON_ERROR( nvs_open( namespace, NVS_READWRITE, &nvs_handle ), error, TAG, "nvs_open failed" );

    ESP_GOTO_ON_ERROR( nvs_set_i16( nvs_handle, key, value ), error, TAG, "nvs_set_i16 failed" );

    return commit_and_close( nvs_handle );

error:

    ESP_LOGE( TAG, "airshift_nvs_write_int16 failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    // Cleanup in case of error after open ...
    if( nvs_handle != 0 )
    {
        nvs_close( nvs_handle );
    }

    return ret;
}

esp_err_t airshift_nvs_write_int32( const char* namespace, const char* key, int32_t value )
{
    esp_err_t       ret         = ESP_FAIL;
    nvs_handle_t    nvs_handle  = 0;

    ESP_LOGI( TAG, "airshift_nvs_write_int32" );

    ESP_GOTO_ON_ERROR( nvs_open( namespace, NVS_READWRITE, &nvs_handle ), error, TAG, "nvs_open failed" );

    ESP_GOTO_ON_ERROR( nvs_set_i32( nvs_handle, key, value ), error, TAG, "nvs_set_i32 failed" );

    return commit_and_close( nvs_handle );

error:

    ESP_LOGE( TAG, "airshift_nvs_write_int32 failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    // Cleanup in case of error after open ...
    if( nvs_handle != 0 )
    {
        nvs_close( nvs_handle );
    }

    return ret;
}

esp_err_t airshift_nvs_write_blob( const char* namespace, const char* key, const void* value, size_t length )
{
    esp_err_t       ret         = ESP_FAIL;
    nvs_handle_t    nvs_handle  = 0;

    ESP_LOGI( TAG, "airshift_nvs_write_blob" );

    ESP_GOTO_ON_ERROR( nvs_open( namespace, NVS_READWRITE, &nvs_handle ), error, TAG, "nvs_open failed" );

    ESP_GOTO_ON_ERROR( nvs_set_blob( nvs_handle, key, value, length ), error, TAG, "nvs_set_blob failed" );

    return commit_and_close( nvs_handle );

error:

    ESP_LOGE( TAG, "airshift_nvs_write_blob failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    // Cleanup in case of error after open ...
    if( nvs_handle != 0 )
    {
        nvs_close( nvs_handle );
    }

    return ret;
}

esp_err_t airshift_nvs_get_string( const char* namespace, const char* key, char* out_value, size_t* length )
{
    esp_err_t       ret         = ESP_FAIL;
    nvs_handle_t    nvs_handle  = 0;

    ESP_LOGI( TAG, "airshift_nvs_get_string" );

    ESP_GOTO_ON_ERROR( nvs_open( namespace, NVS_READONLY, &nvs_handle ), error, TAG, "nvs_open failed" );

    ESP_GOTO_ON_ERROR( nvs_get_str( nvs_handle, key, out_value, length ), error, TAG, "nvs_get_str failed" );

    nvs_close( nvs_handle );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_nvs_get_string failed: %s ( 0x%x ) -> %s", esp_err_to_name( ret ), ret, key );

    // Cleanup in case of error after open ...
    if( nvs_handle != 0 )
    {
        nvs_close( nvs_handle );
    }

    return ret;
}

esp_err_t airshift_nvs_get_int16( const char* namespace, const char* key, int16_t* out_value )
{
    esp_err_t       ret         = ESP_FAIL;
    nvs_handle_t    nvs_handle  = 0;

    ESP_LOGI( TAG, "airshift_nvs_get_int16" );

    ESP_GOTO_ON_ERROR( nvs_open( namespace, NVS_READONLY, &nvs_handle ), error, TAG, "nvs_open failed" );

    ESP_GOTO_ON_ERROR( nvs_get_i16( nvs_handle, key, out_value ), error, TAG, "nvs_get_i16 failed" );

    ESP_LOGI( TAG, "airshift_nvs_get_int16 -> %d", *out_value );

    nvs_close( nvs_handle );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_nvs_get_int16 failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    // Cleanup in case of error after open ...
    if( nvs_handle != 0 )
    {
        nvs_close( nvs_handle );
    }

    return ret;
}

esp_err_t airshift_nvs_get_int32( const char* namespace, const char* key, int32_t* out_value )
{
    esp_err_t       ret         = ESP_FAIL;
    nvs_handle_t    nvs_handle  = 0;

    ESP_LOGI( TAG, "airshift_nvs_get_int32" );

    ESP_GOTO_ON_ERROR( nvs_open( namespace, NVS_READONLY, &nvs_handle ), error, TAG, "nvs_open failed" );

    ESP_GOTO_ON_ERROR( nvs_get_i32( nvs_handle, key, out_value ), error, TAG, "nvs_get_i32 failed" );

    nvs_close( nvs_handle );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_nvs_get_int32 failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    // Cleanup in case of error after open ...
    if( nvs_handle != 0 )
    {
        nvs_close( nvs_handle );
    }

    return ret;
}

esp_err_t airshift_nvs_get_blob( const char* namespace, const char* key, void* out_value, size_t* length )
{
    esp_err_t       ret         = ESP_FAIL;
    nvs_handle_t    nvs_handle  = 0;

    ESP_LOGI( TAG, "airshift_nvs_get_blob" );

    ESP_GOTO_ON_ERROR( nvs_open( namespace, NVS_READONLY, &nvs_handle ), error, TAG, "nvs_open failed" );

    ESP_GOTO_ON_ERROR( nvs_get_blob( nvs_handle, key, out_value, length ), error, TAG, "nvs_get_blob failed" );

    nvs_close( nvs_handle );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "airshift_nvs_get_blob failed: %s ( 0x%x ) -> %s", esp_err_to_name( ret ), ret, key );

    // Cleanup in case of error after open ...
    if( nvs_handle != 0 )
    {
        nvs_close( nvs_handle );
    }

    return ret;
}

esp_err_t airshift_nvs_erase_value( const char* namespace, const char* key )
{
    esp_err_t       ret         = ESP_FAIL;
    nvs_handle_t    nvs_handle  = 0;

    ESP_LOGI( TAG, "airshift_nvs_erase_value" );

    ESP_GOTO_ON_ERROR( nvs_open( namespace, NVS_READWRITE, &nvs_handle ), error, TAG, "nvs_open failed" );

    ESP_GOTO_ON_ERROR( nvs_erase_key( nvs_handle, key ), error, TAG, "nvs_erase_key failed" );

    return commit_and_close( nvs_handle );

error:

    ESP_LOGE( TAG, "airshift_nvs_erase_value failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    // Cleanup in case of error after open ...
    if( nvs_handle != 0 )
    {
        nvs_close( nvs_handle );
    }

    return ret;
}

esp_err_t airshift_nvs_erase_all( const char* namespace )
{
    esp_err_t       ret         = ESP_FAIL;
    nvs_handle_t    nvs_handle  = 0;

    ESP_LOGI( TAG, "airshift_nvs_erase_all" );

    ESP_GOTO_ON_ERROR( nvs_open( namespace, NVS_READWRITE, &nvs_handle ), error, TAG, "nvs_open failed" );

    ESP_GOTO_ON_ERROR( nvs_erase_all( nvs_handle ), error, TAG, "nvs_erase_all failed" );

    return commit_and_close( nvs_handle );

error:

    ESP_LOGE( TAG, "airshift_nvs_erase_all failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    // Cleanup in case of error after open ...
    if( nvs_handle != 0 )
    {
        nvs_close( nvs_handle );
    }

    return ret;
}

bool airshift_nvs_key_exists( const char* namespace, const char* key )
{
    bool            exists          = false;
    esp_err_t       ret             = ESP_FAIL;
    nvs_iterator_t  nvs_iterator    = NULL;

    ESP_LOGI( TAG, "airshift_nvs_key_exists -> %s -> %s", namespace, key );

    ESP_GOTO_ON_ERROR( nvs_entry_find( NVS_DEFAULT_PART_NAME, namespace, NVS_TYPE_ANY, &nvs_iterator ), error, TAG, "nvs_entry_find failed" );
    
    while( nvs_iterator != NULL )
    {
        nvs_entry_info_t entry_info;
        
        nvs_entry_info( nvs_iterator, &entry_info );

        if( strcmp( entry_info.key, key ) == 0 )
        {
            exists = true;

            break;
        }

        ESP_GOTO_ON_ERROR( nvs_entry_next( &nvs_iterator ), error, TAG, "nvs_entry_next failed" );
    };

    goto cleanup;

error:

    ESP_LOGE( TAG, "airshift_nvs_key_exists failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

cleanup:

    // Cleanup ...
    if( nvs_iterator != NULL )
    {
        nvs_release_iterator( nvs_iterator );
    }

    return exists;
}

esp_err_t airshift_nvs_get_or_create_int16( const char* namespace, const char* key, int16_t* out_value )
{
    if( airshift_nvs_key_exists( namespace, key ) == true )
    {
        return airshift_nvs_get_int16( namespace, key, out_value );
    }
    else
    {
        return airshift_nvs_write_int16( namespace, key, (int32_t)*out_value );
    }
}

esp_err_t airshift_nvs_get_or_create_int32( const char* namespace, const char* key, int32_t* out_value )
{
    if( airshift_nvs_key_exists( namespace, key ) == true )
    {
        return airshift_nvs_get_int32( namespace, key, out_value );
    }
    else
    {
        return airshift_nvs_write_int32( namespace, key, (int32_t)*out_value );
    }
}

// Private functions
esp_err_t commit_and_close( nvs_handle_t nvs_handle )
{
    esp_err_t ret = ESP_FAIL;

    ESP_LOGI( TAG, "commit_and_close" );

    ESP_GOTO_ON_ERROR( nvs_commit( nvs_handle ), error, TAG, "nvs_commit failed" );

    nvs_close( nvs_handle );

    return ESP_OK;

error:

    ESP_LOGE( TAG, "commit_and_close failed: %s ( 0x%x )", esp_err_to_name( ret ), ret );

    // Cleanup in case of error with commit ...
    nvs_close( nvs_handle );

    return ret;
}