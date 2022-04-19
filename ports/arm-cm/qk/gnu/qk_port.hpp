// QP/C++ Real-Time Embedded Framework (RTEF)
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-04-10
//! @version Last updated for: @ref qpcpp_7_0_0
//!
//! @file
//! @brief QK/C++ port to ARM Cortex-M, preempive QK kernel, GNU-ARM
//!
#ifndef QK_PORT_HPP
#define QK_PORT_HPP

// determination if the code executes in the ISR context
#define QK_ISR_CONTEXT_() (QK_get_IPSR() != 0U)

__attribute__((always_inline))
static inline uint32_t QK_get_IPSR(void) {
    uint32_t regIPSR;
    __asm volatile ("mrs %0,ipsr" : "=r" (regIPSR));
    return regIPSR;
}

// QK interrupt entry and exit
#define QK_ISR_ENTRY() ((void)0)

#define QK_ISR_EXIT()  do {                                   \
    QF_INT_DISABLE();                                         \
    if (QK_sched_() != 0U) {                                  \
        *Q_UINT2PTR_CAST(uint32_t, 0xE000ED04U) = (1U << 28U);\
    }                                                         \
    QF_INT_ENABLE();                                          \
    QK_ARM_ERRATUM_838869();                                  \
} while (false)

#if (__ARM_ARCH == 6) // Cortex-M0/M0+/M1 (v6-M, v6S-M)?
    #define QK_ARM_ERRATUM_838869() ((void)0)
#else // Cortex-M3/M4/M7 (v7-M)
    // The following macro implements the recommended workaround for the
    // ARM Erratum 838869. Specifically, for Cortex-M3/M4/M7 the DSB
    // (memory barrier) instruction needs to be added before exiting an ISR.
    //
    #define QK_ARM_ERRATUM_838869() \
        __asm volatile ("dsb" ::: "memory")
#endif

// Use NMI ARM Cortex-M exception to return to thread mode (default SVC)
//#define QK_ARM_CM_USE_NMI 1

// initialization of the QK kernel
#define QK_INIT() QK_init()
extern "C" void QK_init(void);
extern "C" void QK_thread_ret(void);

#include "qk.hpp" // QK platform-independent public interface

#endif // QK_PORT_HPP

