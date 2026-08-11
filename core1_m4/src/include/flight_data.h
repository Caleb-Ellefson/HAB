#pragma once
#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
} ipc_time_t;

typedef struct __attribute__((packed)) {
    float      f_internal_tmp;
    float      f_ext_tmp;
    float      f_pressure;
    float      f_humidity;
    float      f_altitude;
    float      f_ACCEL_X;
    float      f_ACCEL_Y;
    float      f_ACCEL_Z;
    double     d_latitude;
    double     d_longitude;
    ipc_time_t rtc_time;
} flight_data_t;

/* shared RAM */
extern volatile flight_data_t *curr_flight_data;