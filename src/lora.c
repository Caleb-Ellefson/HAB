#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/logging/log.h>

#include "lora.h"
LOG_MODULE_REGISTER(lora_client, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *const lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));

int lora_client_init(void) {
    struct lora_modem_config config;
    int ret;

    if (!device_is_ready(lora_dev)) {
        LOG_ERR("LoRa device not ready");
        return -ENODEV;
    }

    config.frequency = 915000000; 
    config.bandwidth = BW_125_KHZ;     
    config.datarate = SF_7;           
    config.coding_rate = CR_4_5; 
    config.preamble_len = 8;
    config.tx_power = 14;
    config.tx = true;

    ret = lora_config(lora_dev, &config);
    if (ret < 0) {
        LOG_ERR("LoRa config failed: %d", ret);
        return ret;
    }

    LOG_INF("LoRa client initialized successfully");
    return 0;
}

int lora_client_send(const void *buf, size_t len) {
    int ret = lora_send(lora_dev, (uint8_t *)buf, len);
    if (ret < 0) {
        LOG_ERR("LoRa send failed: %d", ret);
        return ret;
    }
    
    LOG_INF("Message sent (%zu bytes)", len);
    return 0;
}