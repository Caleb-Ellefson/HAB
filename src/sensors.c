#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/rtc.h>
#include "flight_data.h"

#ifdef CONFIG_BME280

#define SENSOR_THREAD_STACK_SIZE 1024
#define SENSOR_THREAD_PRIORITY   5

const struct device *bme280 = DEVICE_DT_GET_ANY(bosch_bme280);
struct k_thread sensor_thread_data;

int read_bme280(const struct device *bme280, flight_data_t *data) 
{

    if (!device_is_ready(bme280)) {
        printk("BME280 is not ready\n");
        return -ENODEV;
    }

    int ret = sensor_sample_fetch(bme280);
    if (ret < 0) {
        printk("Failed to get sample (%d)\n", ret);
        return ret;
    }

    struct sensor_value temp, press, hum;
    sensor_channel_get(bme280, SENSOR_CHAN_AMBIENT_TEMP, &temp);
    sensor_channel_get(bme280, SENSOR_CHAN_PRESS, &press);
    sensor_channel_get(bme280, SENSOR_CHAN_HUMIDITY, &hum);

    data->f_internal_tmp = sensor_value_to_float(&temp);
    data->f_pressure = sensor_value_to_float(&press) * 10.0f;
    data->f_humidity = sensor_value_to_float(&hum);

    const struct device *rtc = DEVICE_DT_GET(DT_ALIAS(rtc));

    if (device_is_ready(rtc)) {
        struct rtc_time rtc_tm;

        rtc_get_time(rtc, &rtc_tm);
        data->rtc_time = rtc_tm;
    }

#ifdef CONFIG_DEBUG
    printk("[%04u-%02u-%02u %02u:%02u:%02u] ",
           (unsigned)(data->rtc_time.tm_year + 1900),
           (unsigned)(data->rtc_time.tm_mon + 1),
           (unsigned)data->rtc_time.tm_mday,
           (unsigned)data->rtc_time.tm_hour,
           (unsigned)data->rtc_time.tm_min,
           (unsigned)data->rtc_time.tm_sec);
    printk("temp: %.1f pressure: %.1f humidity: %.1f\n",
           (double)data->f_internal_tmp,
           (double)data->f_pressure,
           (double)data->f_humidity);
#endif
    return 0;
}

void update_flight_data(const flight_data_t *local_data) 
{
    k_mutex_lock(&flight_data_mutex, K_FOREVER);

    curr_flight_data.f_internal_tmp = local_data->f_internal_tmp;
    curr_flight_data.f_pressure     = local_data->f_pressure;
    curr_flight_data.f_humidity     = local_data->f_humidity;
    curr_flight_data.rtc_time       = local_data->rtc_time;
    
    k_mutex_unlock(&flight_data_mutex);
}

void sensor_thread_entry(void *p1, void *p2, void *p3) {
    flight_data_t local_sample;

    while (1) {
        if (read_bme280(bme280, &local_sample) == 0) {
            update_flight_data(&local_sample);
        }
        k_msleep(100);
    }
}

/* Spawn thread at boot */
K_THREAD_DEFINE(sensor_thread_id, SENSOR_THREAD_STACK_SIZE,
                sensor_thread_entry, NULL, NULL, NULL,
                SENSOR_THREAD_PRIORITY, 0, 0);
#endif