#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include "flight_data.h"

#ifdef CONFIG_LORA
#include <zephyr/drivers/lora.h>
#include <stm32_ll_hsem.h>

#include "lora.h"

LOG_MODULE_REGISTER(lora);

#define IPC_HSEM_CHANNEL  1

static const struct device *const lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));

#define LORA_STACK_SIZE   1024
#define LORA_BROADCAST_RATE_HZ 25

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

void lora_thread_entry(void *p1, void *p2, void *p3)
{
    int ret;
    struct lora_modem_config config;

    if (!device_is_ready(lora_dev)) {
        LOG_ERR("LoRa device not ready");
        return;
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
        return;
    }

    LOG_INF("LoRa initialized @ 915MHz, SF7, 125kHz");

    while (1) {
        lock_ipc_hsem();

        flight_data_t data;
        memcpy(&data, (const void *)curr_flight_data, sizeof(data));

        unlock_ipc_hsem();

        ret = lora_send(lora_dev, (uint8_t *)&data, sizeof(data));
        if (ret < 0) {
            LOG_WRN("LoRa send failed: %d", ret);
        } else {
            LOG_DBG("Broadcast sent (%zu bytes)", sizeof(data));
        }

        k_sleep(K_MSEC(1000 / LORA_BROADCAST_RATE_HZ));
    }
}

K_THREAD_DEFINE(lora_thread_id, LORA_STACK_SIZE,
                lora_thread_entry, NULL, NULL, NULL,
                5, 0, 0);
#endif
