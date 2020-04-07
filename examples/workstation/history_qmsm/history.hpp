//.$file${.::history.hpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: history.qm
// File:  ${.::history.hpp}
//
// This code has been generated by QM 5.0.0 <www.state-machine.com/qm/>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This code is covered by the following QP license:
// License #   : QPCPP-EVAL
// Issued to   : Institution or an individual evaluating the QP/C++ framework
// Framework(s): qpcpp
// Support ends: 2020-12-31
// Product(s)  :
// This license is available only for evaluation purposes and
// the generated code is still licensed under the terms of GPL.
// Please submit request for extension of the evaluaion period at:
// <www.state-machine.com/licensing/#RequestForm>
//
//.$endhead${.::history.hpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#ifndef HISTORY_HPP
#define HISTORY_HPP

enum ToastOvenSignals {
    OPEN_SIG = QP::Q_USER_SIG,
    CLOSE_SIG,
    TOAST_SIG,
    BAKE_SIG,
    OFF_SIG,
    TERMINATE_SIG // terminate the application
};

extern QP::QMsm * const the_oven; // opaque pointer to the oven MSM

#endif // HISTORY_HPP