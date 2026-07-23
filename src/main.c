#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_LORA
#include "lora.h"
#endif

LOG_MODULE_REGISTER(main);

int main(void) {

#ifdef CONFIG_LORA

    if (lora_client_init() < 0) {
        LOG_ERR("Failed to initialize LoRa client subsystem");
        return 0;
    }

    while (1) {
        static const char msg[] = "Hello World";
        
        lora_client_send(msg, sizeof(msg));
        k_sleep(K_SECONDS(5));
    }
#endif

    return 0;
}