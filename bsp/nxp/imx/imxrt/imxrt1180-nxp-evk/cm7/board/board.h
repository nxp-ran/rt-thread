/*
 * Copyright 2021-2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include "clock_config.h"
#include "pin_mux.h"
#include "fsl_common.h"
#include "fsl_rgpio.h"
#include "fsl_clock.h"

#if defined(__ARMCC_VERSION)
extern int Image$$ARM_LIB_HEAP$$ZI$$Base;
extern int Image$$ARM_LIB_HEAP$$ZI$$Limit;
#define HEAP_BEGIN  ((void *)&Image$$ARM_LIB_HEAP$$ZI$$Base)
#define HEAP_END    ((void*)&Image$$ARM_LIB_HEAP$$ZI$$Limit)
#elif defined(__ICCARM__)
#pragma section="HEAP"
#define HEAP_BEGIN    (__section_begin("HEAP"))
#define HEAP_END      (__section_end("HEAP"))
#elif defined(__GNUC__)
extern int __HeapBase;
extern int __HeapLimit;
#define HEAP_BEGIN  ((void *)&__HeapBase)
#define HEAP_END  ((void *)&__HeapLimit)
#endif

/*! @brief The board flash size */
#define BOARD_FLASH_SIZE (0x1000000U)

/*! @brief The Ethernet PHY addresses. */
#define BOARD_EP0_PHY_ADDR       (0x03U)
#define BOARD_SWT_PORT0_PHY_ADDR (0x02U)
#define BOARD_SWT_PORT1_PHY_ADDR (0x05U)
#define BOARD_SWT_PORT2_PHY_ADDR (0x04U)
#define BOARD_SWT_PORT3_PHY_ADDR (0x07U)

/*! @brief The Ethernet PHY type of the board */
#define BOARD_USE_NETC_PHY_RTL8201

/* USB PHY configuration */
#define BOARD_USB_PHY_D_CAL     (0x07U)
#define BOARD_USB_PHY_TXCAL45DP (0x06U)
#define BOARD_USB_PHY_TXCAL45DM (0x06U)

/*! @brief The Ethernet port used by network examples, default use 1G port. */
#ifndef BOARD_NETWORK_USE_100M_ENET_PORT
#define BOARD_NETWORK_USE_100M_ENET_PORT (0U)
#endif

void BOARD_NETC_Init(void);
void BOARD_RequestTRDC(bool bRequestAON, bool bRequestWakeup, bool bReqeustMega);
void BOARD_CommonSetting(void);
void PHY_Reset(void);

void rt_hw_board_init(void);

#endif
