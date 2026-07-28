/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-19     RT-Thread    the first version for IMXRT1180 (BBNSM RTC)
 *
 * The IMXRT1180 SoC does not have the SNVS peripheral found on older i.MX RT
 * parts. Instead it integrates the BBNSM (Battery-Backed Non-Secure Monitor)
 * block, which provides a 47-bit seconds counter (BBNSM_RTC_MS[14:0] <<32 |
 * BBNSM_RTC_LS[31:0]).  This driver maps that counter to/from Unix time_t so
 * that the standard RT-Thread RTC device interface works without change.
 */

#include <rtthread.h>
#include <rtdevice.h>
#ifdef BSP_USING_RTC

#define LOG_TAG  "drv.rtc"
#include <drv_log.h>

#include "drv_rtc.h"
#include <sys/time.h>

/* BBNSM base address for IMXRT1189 (same for all RT118x variants). */
#ifndef BBNSM_BASE
#define BBNSM_BASE  (0x44440000u)
#endif

/* Minimal BBNSM register layout - only the fields we need. */
typedef struct {
    volatile uint32_t BBNSM_VID;       /* 0x00 - version */
    volatile uint32_t BBNSM_FEATURES;  /* 0x04 */
    volatile uint32_t BBNSM_CTRL;      /* 0x08 */
    uint32_t          _res0;
    volatile uint32_t BBNSM_INT_EN;    /* 0x10 */
    volatile uint32_t BBNSM_EVENTS;    /* 0x14 */
    uint32_t          _res1[3];
    volatile uint32_t BBNSM_PAD_CTRL;  /* 0x24 */
    uint32_t          _res2[6];
    volatile uint32_t BBNSM_RTC_LS;    /* 0x40 - lower 32 bits of RTC counter */
    volatile uint32_t BBNSM_RTC_MS;    /* 0x44 - upper 15 bits of RTC counter */
} BBNSM_Regs;

/* BBNSM_CTRL bit fields */
#define BBNSM_CTRL_RTC_EN_DISABLE   (0x1u << 0)  /* write 01 to stop RTC */
#define BBNSM_CTRL_RTC_EN_ENABLE    (0x2u << 0)  /* write 10 to start RTC */

/* Upper 15 bits mask for RTC_MS register */
#define BBNSM_RTC_MS_MASK           (0x7FFFu)

#define BBNSM  ((BBNSM_Regs *)BBNSM_BASE)

/* ----------------------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------------------*/

/*
 * bbnsm_rtc_enable - start the RTC counter if it is not already running.
 *
 * The BBNSM uses a "write-once sticky" encoding: write 0b10 to enable,
 * write 0b01 to disable. Reading back 0b10 means enabled.
 */
static void bbnsm_rtc_enable(void)
{
    uint32_t ctrl = BBNSM->BBNSM_CTRL;
    /* clear the 2-bit RTC_EN field then write "enable" value */
    ctrl &= ~(0x3u);
    ctrl |= BBNSM_CTRL_RTC_EN_ENABLE;
    BBNSM->BBNSM_CTRL = ctrl;
}

/*
 * bbnsm_get_seconds - read the 47-bit RTC counter as a 64-bit value.
 *
 * The hardware guarantees a coherent read when RTC_LS is read first and
 * RTC_MS is read immediately after (RTC_LS latches RTC_MS on its read).
 */
static uint64_t bbnsm_get_seconds(void)
{
    uint32_t ls, ms;
    /* read LS first - this latches MS */
    ls = BBNSM->BBNSM_RTC_LS;
    ms = BBNSM->BBNSM_RTC_MS & BBNSM_RTC_MS_MASK;
    return ((uint64_t)ms << 32) | (uint64_t)ls;
}

/*
 * bbnsm_set_seconds - write a new value into the 47-bit RTC counter.
 *
 * The procedure recommended by the RM is:
 *   1. Stop the RTC counter.
 *   2. Write RTC_LS then RTC_MS.
 *   3. Re-enable the RTC counter.
 */
static void bbnsm_set_seconds(uint64_t secs)
{
    uint32_t ctrl;

    /* 1. Stop counter */
    ctrl = BBNSM->BBNSM_CTRL;
    ctrl &= ~(0x3u);
    ctrl |= BBNSM_CTRL_RTC_EN_DISABLE;
    BBNSM->BBNSM_CTRL = ctrl;

    /* 2. Write new value (LS first, then MS) */
    BBNSM->BBNSM_RTC_LS = (uint32_t)(secs & 0xFFFFFFFFu);
    BBNSM->BBNSM_RTC_MS = (uint32_t)((secs >> 32) & BBNSM_RTC_MS_MASK);

    /* 3. Re-enable counter */
    bbnsm_rtc_enable();
}

/* ----------------------------------------------------------------------------
 * RT-Thread device callbacks
 * --------------------------------------------------------------------------*/

static rt_err_t imxrt1180_rtc_init(rt_device_t dev)
{
    bbnsm_rtc_enable();
    return RT_EOK;
}

static rt_err_t imxrt1180_rtc_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t imxrt1180_rtc_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_ssize_t imxrt1180_rtc_read(rt_device_t dev, rt_off_t pos, void *buf, rt_size_t size)
{
    return -RT_EINVAL;
}

static rt_ssize_t imxrt1180_rtc_write(rt_device_t dev, rt_off_t pos, const void *buf, rt_size_t size)
{
    return -RT_EINVAL;
}

static rt_err_t imxrt1180_rtc_control(rt_device_t dev, int cmd, void *args)
{
    RT_ASSERT(dev != RT_NULL);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
    {
        /* return Unix timestamp (seconds since 1970-01-01 00:00:00 UTC) */
        *(time_t *)args = (time_t)bbnsm_get_seconds();
        break;
    }
    case RT_DEVICE_CTRL_RTC_SET_TIME:
    {
        bbnsm_set_seconds((uint64_t)(*(time_t *)args));
        break;
    }
    default:
        return -RT_EINVAL;
    }

    return RT_EOK;
}

/* ----------------------------------------------------------------------------
 * Device object and registration
 * --------------------------------------------------------------------------*/

static struct rt_device s_rtc_device =
{
    .type    = RT_Device_Class_RTC,
    .init    = imxrt1180_rtc_init,
    .open    = imxrt1180_rtc_open,
    .close   = imxrt1180_rtc_close,
    .read    = imxrt1180_rtc_read,
    .write   = imxrt1180_rtc_write,
    .control = imxrt1180_rtc_control,
};

int rt_hw_rtc_init(void)
{
    rt_err_t ret;

    ret = rt_device_register(&s_rtc_device, "rtc", RT_DEVICE_FLAG_RDWR);
    if (ret != RT_EOK)
    {
        LOG_E("rtc device register failed: %d", ret);
        return ret;
    }

    rt_device_open(&s_rtc_device, RT_DEVICE_OFLAG_RDWR);

    LOG_D("rtc init ok (BBNSM)");
    return RT_EOK;
}

INIT_DEVICE_EXPORT(rt_hw_rtc_init);

#endif /* BSP_USING_RTC */
