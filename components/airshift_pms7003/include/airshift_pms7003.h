#ifndef AIRSHIFT_PMS7003_H
#define AIRSHIFT_PMS7003_H

#include "airshift_header_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint16_t frame_length;

    uint16_t pm_sp_ug_1_0;
    uint16_t pm_sp_ug_2_5;
    uint16_t pm_sp_ug_10_0;

    uint16_t pm_ae_ug_1_0;
    uint16_t pm_ae_ug_2_5;
    uint16_t pm_ae_ug_10_0;

    uint16_t pm_raw_0_3;
    uint16_t pm_raw_0_5;
    uint16_t pm_raw_1_0;
    uint16_t pm_raw_2_5;
    uint16_t pm_raw_5_0;
    uint16_t pm_raw_10_0;

    uint16_t reserved;
    uint16_t checksum;
} pms7003_data_t;

esp_err_t   airshift_pms7003_init();
esp_err_t   airshift_pms7003_release();

esp_err_t   airshift_pms7003_sleep();
esp_err_t   airshift_pms7003_wakeup();

esp_err_t   airshift_pms7003_get( pms7003_data_t *pms7003_data );

#ifdef __cplusplus
}
#endif

#endif // AIRSHIFT_PMS7003_H
