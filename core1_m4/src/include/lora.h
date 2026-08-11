#ifndef LORA_H_
#define LORA_H_

/**
 * @brief LoRa broadcast thread entry function.
 *
 * Reads curr_flight_data from shared memory via HSEM and
 * broadcasts it at ~25Hz over LoRa.
 */
void lora_thread_entry(void *p1, void *p2, void *p3);

#endif /* LORA_H_ */
