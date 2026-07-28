/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * DWT (Data Watchpoint and Trace) driver for i.MX RT1180 CM33.
 *
 * The DWT cycle counter provides a free-running 32-bit count of CPU cycles
 * that can be used for high-resolution delay and profiling.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-01-01     RT-Thread    first version
 */

#ifndef __DRV_DWT_H__
#define __DRV_DWT_H__

#include <rtthread.h>
#include <stdint.h>

#ifdef BSP_USING_DWT

/* Initialize the DWT cycle counter. Must be called before any DWT APIs. */
int dwt_init(void);

/*
 * Delay for the given number of microseconds.
 * Blocks the calling thread using the DWT cycle counter - no RTOS involvement.
 * Safe to call from interrupt context.
 *
 * @param us  number of microseconds to delay (0 is a no-op)
 */
void dwt_delay_us(rt_uint32_t us);

/*
 * Return the current DWT cycle count.
 * Wraps every (2^32 / CPU_Hz) seconds (~17.9 s at 240 MHz).
 */
rt_uint32_t dwt_get_cycles(void);

#endif /* BSP_USING_DWT */

#endif /* __DRV_DWT_H__ */
