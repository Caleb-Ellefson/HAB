#ifndef LORA_H_
#define LORA_H_

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Initializes the LoRa hardware interface and configuration.
 * @return 0 on success, or a negative error code on failure.
 */
int lora_client_init(void);

/**
 * @brief Transmits a message payload over LoRa.
 * 
 * @param buf Pointer to data buffer.
 * @param len Size of data buffer in bytes.
 * @return 0 on success, or a negative error code on failure.
 */
int lora_client_send(const void *buf, size_t len);

#endif /* LORA_CLIENT_H_ */