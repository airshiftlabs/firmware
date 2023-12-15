#ifndef AIRSHIFT_MATTER_H
#define AIRSHIFT_MATTER_H

#include "airshift_header_common.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t   airshift_matter_init();
esp_err_t   airshift_matter_release();

#ifdef __cplusplus
}
#endif

#endif // AIRSHIFT_MATTER_H
