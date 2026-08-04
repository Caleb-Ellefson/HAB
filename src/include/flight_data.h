#pragma once
#include <stdint.h>
#include <zephyr/kernel.h>

/* Remove padding */
typedef struct __attribute__((packed)){
    float f_internal_tmp;
    float f_ext_tmp;
    float f_pressure;
    float f_humidity;
    float f_altitude;
    double d_latitude;
    double d_longitude;
    uint32_t timestamp;
} flight_data_t;

extern flight_data_t curr_flight_data;
extern struct k_mutex flight_data_mutex;