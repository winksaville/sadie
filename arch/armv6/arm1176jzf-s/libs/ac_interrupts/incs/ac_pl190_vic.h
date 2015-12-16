/*
 * Copyright 2015 Wink Saville
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SADIE_PLATFORM_VERSATILEPB_LIBS_AC_INTERRUPTS_INCS_AC_PL190_VIC_H
#define SADIE_PLATFORM_VERSATILEPB_LIBS_AC_INTERRUPTS_INCS_AC_PL190_VIC_H

#include <ac_inttypes.h>
#include <ac_bits.h>

// PIC interrupt bits
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0225d/index.html
// Its better to download the full manual
// [here](http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf)
// and see Section "3.10 Interrupts" page 3-49 Figure 3-23 "External and internal interrupt sources".
#define PIC_ALL             ((ac_u32)0xFFFFFFFF)
#define PIC_WATCHDOG        AC_BIT(ac_u32, 0)
#define PIC_SOFTWARE_INT    AC_BIT(ac_u32, 1)
#define PIC_COMMS_RX        AC_BIT(ac_u32, 2)
#define PIC_COMMS_TX        AC_BIT(ac_u32, 3)
#define PIC_TIMERS0_1       AC_BIT(ac_u32, 4)
#define PIC_TIMERS2_3       AC_BIT(ac_u32, 5)
#define PIC_GPIO0           AC_BIT(ac_u32, 6)
#define PIC_GPIO1           AC_BIT(ac_u32, 7)
#define PIC_GPIO2           AC_BIT(ac_u32, 8)
#define PIC_GPIO3           AC_BIT(ac_u32, 9)
#define PIC_RTC             AC_BIT(ac_u32, 10)
#define PIC_SSP             AC_BIT(ac_u32, 11)
#define PIC_UART0           AC_BIT(ac_u32, 12)
#define PIC_UART1           AC_BIT(ac_u32, 13)
#define PIC_UART2           AC_BIT(ac_u32, 14)
#define PIC_SCIO            AC_BIT(ac_u32, 15)
#define PIC_CLCDC           AC_BIT(ac_u32, 16)
#define PIC_DMA             AC_BIT(ac_u32, 17)
#define PIC_PWRFAIL         AC_BIT(ac_u32, 18)
#define PIC_MBX             AC_BIT(ac_u32, 19)
#define PIC_GND             AC_BIT(ac_u32, 20)
#define PIC_DOC             AC_BIT(ac_u32, 21)
#define PIC_MMCIO_0         AC_BIT(ac_u32, 22)
#define PIC_23_RESERVED     AC_BIT(ac_u32, 23)
#define PIC_AACI            AC_BIT(ac_u32, 24)
#define PIC_EHTERNET        AC_BIT(ac_u32, 25)
#define PIC_USB             AC_BIT(ac_u32, 26)
#define PIC_27_EXP          AC_BIT(ac_u32, 27)
#define PIC_28_EXP          AC_BIT(ac_u32, 28)
#define PIC_29_RESERVED     AC_BIT(ac_u32, 29)
#define PIC_30_RESERVED     AC_BIT(ac_u32, 30)
#define PIC_SIC             AC_BIT(ac_u32, 31)

#endif
