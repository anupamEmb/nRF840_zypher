/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Hello World for nRF52832 using Zephyr RTOS.
 * Logs a greeting message once on boot, then sleeps.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int main(void)
{
	LOG_INF("Hello World from nRF52832!");

	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
