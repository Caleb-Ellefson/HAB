#pragma once
#include <zephyr/device.h>
#include "flight_data.h"

int read_bme280(const struct device *bme280, flight_data_t *data);
