/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * DWT (Data Watchpoint and Trace) driver for i.MX RT1180 CM33.
 *
 * Uses the ARM CoreDebug / DWT registers (CMSIS headers) to enable the
 * free-running 32-bit cycle counter.  The counter is used to implement a
 * microsecond-resolution busy-wait delay and a raw cycle-count accessor.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-01-01     RT-Thread    first version
 */

#include "drv_dwt.h"

#ifdef BSP_USING_DWT

#include <rtdevice.h>
#include "MIMXRT1189_cm33.h"   /* provides CoreDebug, DWT, SystemCoreClock */

/*
 * dwt_init - enable the DWT cycle counter.
 *
 * Must be called once before dwt_delay_us() or dwt_get_cycles().
 * Registered as an auto-init board driver so it runs during
 * rt_components_board_init().
 */
int dwt_init(void)
{
    /* Enable the DWT unit via the DEMCR trace-enable bit. */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    /* Reset the cycle counter then enable it. */
    DWT->CYCCNT = 0U;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;

    return 0;
}
INIT_BOARD_EXPORT(dwt_init);

/*
 * dwt_get_cycles - return the raw 32-bit DWT cycle count.
 */
rt_uint32_t dwt_get_cycles(void)
{
    return DWT->CYCCNT;
}

/*
 * dwt_delay_us - busy-wait for the requested number of microseconds.
 *
 * Computation avoids 64-bit arithmetic: the maximum delay representable
 * without overflow is (UINT32_MAX / MHz) us, which is > 17 s at 240 MHz -
 * sufficient for all practical busy-delay use-cases.
 *
 * @param us  delay in microseconds; 0 is a no-op.
 */
void dwt_delay_us(rt_uint32_t us)
{
    rt_uint32_t cycles_per_us;
    rt_uint32_t start;
    rt_uint32_t delta;

    if (us == 0U)
    {
        return;
    }

    /* SystemCoreClock is in Hz; divide to get cycles per microsecond. */
    cycles_per_us = SystemCoreClock / 1000000U;
    delta         = cycles_per_us * us;
    start         = DWT->CYCCNT;

    /* Unsigned subtraction handles the 32-bit wrap correctly. */
    while ((DWT->CYCCNT - start) < delta)
    {
        /* busy wait */
    }
}

#endif /* BSP_USING_DWT */
