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
//! @brief QV/C++ port to ARM Cortex-M, cooperative QV kernel, IAR-ARM
//!
#ifndef QV_PORT_HPP
#define QV_PORT_HPP

#if (__ARM_ARCH == 6) // if ARMv6-M...

    // macro to put the CPU to sleep inside QV::onIdle()
    #define QV_CPU_SLEEP() do { \
        __WFI();                \
        QF_INT_ENABLE();        \
    } while (false)

    #define QV_ARM_ERRATUM_838869() ((void)0)

#else // Cortex-M3/M4/M7(v7-M)

    // macro to put the CPU to sleep inside QV::onIdle()
    #define QV_CPU_SLEEP() do { \
        QF_PRIMASK_DISABLE();   \
        QF_INT_ENABLE();        \
        __WFI();                \
        QF_PRIMASK_ENABLE();    \
    } while (false)

    // initialization of the QV kernel for Cortex-M3/M4/M4F
    #define QV_INIT() QV_init()
    extern "C" void QV_init(void);

    // The following macro implements the recommended workaround for the
    // ARM Erratum 838869. Specifically, for Cortex-M3/M4/M7 the DSB
    // (memory barrier) instruction needs to be added before exiting an ISR.
    // This macro should be inserted at the end of ISRs.
    //
    #define QV_ARM_ERRATUM_838869() __DSB()

#endif // ARMv6-M

#include "qv.hpp"   // QV platform-independent public interface

#endif // QV_PORT_HPP

