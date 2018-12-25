#ifndef __GPS_H__
#define __GPS_H__

#include "esp_err.h"
#include "esp_types.h"
#include "time.h"

typedef struct {
    double latitude;
    double longtitude;
    struct tm time;
    unsigned int satellites;
    double speed;
} gps_info_t;

extern gps_info_t* gps_info;

esp_err_t gps_init();

#endif // __GPS_H__