#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/rtc.h>
#include "flight_data.h"
#include <stm32_ll_hsem.h>

#define IPC_HSEM_CHANNEL  16

#define SENSOR_THREAD_STACK_SIZE 1024
#define SENSOR_THREAD_PRIORITY   5

static void lock_ipc_hsem(void)
{
    while (LL_HSEM_2StepLock(HSEM, IPC_HSEM_CHANNEL, 0) != 0) {
        k_yield();
    }
}

static void unlock_ipc_hsem(void)
{
    LL_HSEM_ReleaseLock(HSEM, IPC_HSEM_CHANNEL, 0);
}

#ifdef CONFIG_BME280
static const struct device *get_bme280(void)
{
    static const struct device *bme280;
    static bool initialized;

    if (!initialized) {
        bme280 = DEVICE_DT_GET_ANY(bosch_bme280);
        initialized = true;
    }
    return bme280;
}
#endif

#ifdef CONFIG_ADXL375
static const struct device *get_adxl375(void)
{
    static const struct device *adxl375;
    static bool initialized;

    if (!initialized) {
        adxl375 = DEVICE_DT_GET_ANY(adxl375);
        initialized = true;
    }
    return adxl375;
}
#endif

#ifdef CONFIG_BME280
int read_bme280(const struct device *bme280, float *out_tmp, float *out_press, float *out_hum)
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

    *out_tmp = sensor_value_to_float(&temp);
    *out_press = sensor_value_to_float(&press) * 10.0f;
    *out_hum = sensor_value_to_float(&hum);

    return 0;
}
#endif

#ifdef CONFIG_ADXL375
int read_adxl375(const struct device *adxl375, float *out_x, float *out_y, float *out_z)
{
    if (!device_is_ready(adxl375)) {
        printk("ADXL375 is not ready\n");
        return -ENODEV;
    }

    int ret = sensor_sample_fetch(adxl375);
    if (ret < 0) {
        printk("Failed to get sample (%d)\n", ret);
        return ret;
    }

    struct sensor_value accel[3];
    sensor_channel_get(adxl375, SENSOR_CHAN_ACCEL_XYZ, accel);

    *out_x = sensor_value_to_float(&accel[0]);
    *out_y = sensor_value_to_float(&accel[1]);
    *out_z = sensor_value_to_float(&accel[2]);

    return 0;
}
#endif

#ifdef CONFIG_RTC
int read_rtc_time(ipc_time_t *out_ipc_time)
{
    const struct device *rtc = DEVICE_DT_GET(DT_ALIAS(rtc));
    struct rtc_time rtc_tm;

    if (!device_is_ready(rtc)) {
        return -ENODEV;
    }

    int ret = rtc_get_time(rtc, &rtc_tm);
    if (ret == 0) {
        out_ipc_time->year    = (uint16_t)(rtc_tm.tm_year + 1900);
        out_ipc_time->month   = (uint8_t)(rtc_tm.tm_mon + 1);
        out_ipc_time->day     = (uint8_t)rtc_tm.tm_mday;
        out_ipc_time->hours   = (uint8_t)rtc_tm.tm_hour;
        out_ipc_time->minutes = (uint8_t)rtc_tm.tm_min;
        out_ipc_time->seconds = (uint8_t)rtc_tm.tm_sec;
    }

    return ret;
}
#endif

void update_flight_data(const flight_data_t *local_data)
{
    lock_ipc_hsem();

#ifdef CONFIG_BME280
    curr_flight_data->f_internal_tmp = local_data->f_internal_tmp;
    curr_flight_data->f_pressure     = local_data->f_pressure;
    curr_flight_data->f_humidity     = local_data->f_humidity;
#endif

#ifdef CONFIG_ADXL375
    curr_flight_data->f_ACCEL_X      = local_data->f_ACCEL_X;
    curr_flight_data->f_ACCEL_Y      = local_data->f_ACCEL_Y;
    curr_flight_data->f_ACCEL_Z      = local_data->f_ACCEL_Z;
#endif

#ifdef CONFIG_RTC
    curr_flight_data->rtc_time       = local_data->rtc_time;
#endif

    unlock_ipc_hsem();
}

#ifdef CONFIG_DEBUG
static void debug_print_bme280(const flight_data_t *data)
{
    printk("temp: %.1f pressure: %.1f humidity: %.1f\n",
           (double)data->f_internal_tmp,
           (double)data->f_pressure,
           (double)data->f_humidity);
}

static void debug_print_adxl375(const flight_data_t *data)
{
    printk("X: %.3f Y: %.3f Z: %.3f m/s^2\n",
           (double)data->f_ACCEL_X,
           (double)data->f_ACCEL_Y,
           (double)data->f_ACCEL_Z);
}

static void debug_print_time(const ipc_time_t *t)
{
    printk("[%04d-%02d-%02d %02d:%02d:%02d] ",
           t->year, t->month, t->day, t->hours, t->minutes, t->seconds);
}
#endif

void sensor_thread_entry(void *p1, void *p2, void *p3)
{
    flight_data_t local_sample = {0};

    while (1) {
#ifdef CONFIG_BME280
        float tmp, press, hum;

        if (read_bme280(get_bme280(), &tmp, &press, &hum) == 0) {
            local_sample.f_internal_tmp = tmp;
            local_sample.f_pressure     = press;
            local_sample.f_humidity     = hum;
        }
#endif

#ifdef CONFIG_ADXL375
        float ax, ay, az;

        if (read_adxl375(get_adxl375(), &ax, &ay, &az) == 0) {
            local_sample.f_ACCEL_X = ax;
            local_sample.f_ACCEL_Y = ay;
            local_sample.f_ACCEL_Z = az;
        }
#endif

#ifdef CONFIG_RTC
        ipc_time_t ipc_tm;

        if (read_rtc_time(&ipc_tm) == 0) {
            local_sample.rtc_time = ipc_tm;
        }
#endif

        update_flight_data(&local_sample);

#ifdef CONFIG_DEBUG
#ifdef CONFIG_RTC
        debug_print_time(&local_sample.rtc_time);
#endif
#ifdef CONFIG_BME280
        debug_print_bme280(&local_sample);
#endif
#ifdef CONFIG_ADXL375
        debug_print_adxl375(&local_sample);
#endif
#endif
        k_msleep(1);
    }
}

/* Spawn thread on start */
K_THREAD_DEFINE(sensor_thread_id, SENSOR_THREAD_STACK_SIZE,
                sensor_thread_entry, NULL, NULL, NULL,
                SENSOR_THREAD_PRIORITY, 0, 0);