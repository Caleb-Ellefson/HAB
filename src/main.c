#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lora_client);

int main(void) {
	const struct device *const lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));
	struct lora_modem_config config;
	int ret;

	if (!device_is_ready(lora_dev)) {
		LOG_ERR("LoRa device not ready");
		return 0;
	}

	/* Regional configuration (915MHz for North America) */
	config.frequency = 915000000; 
	config.bandwidth = BW_125_KHZ;     
	config.datarate = SF_7;           
	config.coding_rate = CR_4_5; 
	config.preamble_len = 8;
	config.tx_power = 14;
	config.tx = true;

	ret = lora_config(lora_dev, &config);
	if (ret < 0) {
		LOG_ERR("LoRa config failed");
		return 0;
	}

	LOG_INF("LoRa Client Started");

	while (1) {
		static char msg[] = "Hello World";
		ret = lora_send(lora_dev, msg, sizeof(msg));
		if (ret < 0) {
			LOG_ERR("LoRa send failed");
		} else {
			LOG_INF("Message sent!");
		}
		k_sleep(K_SECONDS(5));
	}
	return 0;
}
