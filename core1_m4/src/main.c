#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stm32wlxx_ll_pwr.h>
#include "flight_data.h"

#define IPC_NODE DT_NODELABEL(ipc_shm)
#define IPC_SHM_BASE DT_REG_ADDR(IPC_NODE)

LOG_MODULE_REGISTER(main);

/* Shared section of SRAM */
volatile flight_data_t *curr_flight_data =
    (volatile flight_data_t *)DT_REG_ADDR(IPC_NODE);

int main(void)
{
	/* Release CPU2 boot lock / boot CPU2 from CPU1 */
	LL_PWR_EnableBootC2();

	return 0;
}
