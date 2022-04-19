//! @file
//! @brief QUTEST port for Win32
//! @ingroup ports
//! @cond
//============================================================================
//! Last updated for version 6.9.3
//! Last updated on  2021-03-16
//!
//!                    Q u a n t u m  L e a P s
//!                    ------------------------
//!                    Modern Embedded Software
//!
//! Copyright (C) 2005-2021 Quantum Leaps. All rights reserved.
//!
//! This program is open source software: you can redistribute it and/or
//! modify it under the terms of the GNU General Public License as published
//! by the Free Software Foundation, either version 3 of the License, or
//! (at your option) any later version.
//!
//! Alternatively, this program may be distributed and modified under the
//! terms of Quantum Leaps commercial licenses, which expressly supersede
//! the GNU General Public License and are specifically designed for
//! licensees interested in retaining the proprietary status of their code.
//!
//! This program is distributed in the hope that it will be useful,
//! but WITHOUT ANY WARRANTY; without even the implied warranty of
//! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//! GNU General Public License for more details.
//!
//! You should have received a copy of the GNU General Public License
//! along with this program. If not, see <www.gnu.org/licenses>.
//!
//! Contact information:
//! <www.state-machine.com/licensing>
//! <info@state-machine.com>
//============================================================================
//! @endcond
//!

#ifndef Q_SPY
    #error "Q_SPY must be defined for QUTest application"
#endif // Q_SPY

#define QP_IMPL         // this is QP implementation
#include "qf_port.hpp"  // QF port
#include "qassert.h"    // QP embedded systems-friendly assertions
#include "qs_port.hpp"  // include QS port

#include "safe_std.h"   // portable "safe" <stdio.h>/<string.h> facilities
#include <stdlib.h>
#include <time.h>
#include <conio.h>

// Minimum required Windows version is Windows-XP or newer (0x0501)
#ifdef WINVER
#undef WINVER
#endif
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define WINVER _WIN32_WINNT_WINXP
#define _WIN32_WINNT _WIN32_WINNT_WINXP

#include <ws2tcpip.h>

#define QS_TX_SIZE     (8*1024)
#define QS_RX_SIZE     (2*1024)
#define QS_TX_CHUNK    QS_TX_SIZE
#define QS_TIMEOUT_MS  10

namespace QP {

//Q_DEFINE_THIS_MODULE("qutest_port")

// local variables ...........................................................
static SOCKET l_sock = INVALID_SOCKET;

//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[QS_TX_SIZE];   // buffer for QS-TX channel
    static uint8_t qsRxBuf[QS_RX_SIZE]; // buffer for QS-RX channel
    char hostName[128];
    char const *serviceName = "6601";   // default QSPY server port
    char const *src;
    char *dst;
    int status;

    struct addrinfo *result = NULL;
    struct addrinfo *rp = NULL;
    struct addrinfo hints;
    BOOL   sockopt_bool;
    ULONG  ioctl_opt;
    WSADATA wsaData;

    // initialize the QS transmit and receive buffers
    initBuf(qsBuf, sizeof(qsBuf));
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // initialize Windows sockets version 2.2
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
        fprintf(stderr,
            "<TARGET> ERROR Windows Sockets cannot be initialized\n");
        goto error;
    }

    // extract hostName from 'arg' (hostName:port_remote)...
    src = (arg != (void const *)0)
          ? (char const *)arg
          : "localhost"; // default QSPY host
    dst = hostName;
    while ((*src != '\0')
           && (*src != ':')
           && (dst < &hostName[sizeof(hostName) - 1]))
    {
        *dst++ = *src++;
    }
    *dst = '\0'; // zero-terminate hostName

    // extract serviceName from 'arg' (hostName:serviceName)...
    if (*src == ':') {
        serviceName = src + 1;
    }
    //printf("<TARGET> Connecting to QSPY on Host=%s:%s...\n",
    //       hostName, serviceName);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    status = getaddrinfo(hostName, serviceName, &hints, &result);
    if (status != 0) {
        fprintf(stderr,
            "<TARGET> ERROR   cannot resolve host Name=%s:%s,Err=%d\n",
                    hostName, serviceName, status);
        goto error;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        l_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (l_sock != INVALID_SOCKET) {
            if (connect(l_sock, rp->ai_addr,
                        static_cast<int>(rp->ai_addrlen)) == SOCKET_ERROR)
            {
                closesocket(l_sock);
                l_sock = INVALID_SOCKET;
            }
            break;
        }
    }

    freeaddrinfo(result);

    // socket could not be opened & connected?
    if (l_sock == INVALID_SOCKET) {
        fprintf(stderr, "<TARGET> ERROR   cannot connect to QSPY at "
            "host=%s:%s\n",
            hostName, serviceName);
        goto error;
    }

    // set the socket to non-blocking mode
    ioctl_opt = 1;
    if (ioctlsocket(l_sock, FIONBIO, &ioctl_opt) != NO_ERROR) {
        fprintf(stderr, "<TARGET> ERROR   Failed to set non-blocking socket "
            "WASErr=%d\n", WSAGetLastError());
        goto error;
    }

    // configure the socket to reuse the address and not to linger
    sockopt_bool = TRUE;
    setsockopt(l_sock, SOL_SOCKET, SO_REUSEADDR,
               (const char *)&sockopt_bool, sizeof(sockopt_bool));
    sockopt_bool = TRUE;
    setsockopt(l_sock, SOL_SOCKET, SO_DONTLINGER,
               (const char *)&sockopt_bool, sizeof(sockopt_bool));

    //printf("<TARGET> Connected to QSPY at Host=%s:%d\n",
    //       hostName, port_remote);
    onFlush();

    return true;  // success

error:
    return false; // failure
}
//............................................................................
void QS::onCleanup(void) {
    if (l_sock != INVALID_SOCKET) {
        closesocket(l_sock);
        l_sock = INVALID_SOCKET;
    }
    WSACleanup();
    //printf("<TARGET> Disconnected from QSPY\n");
}
//............................................................................
void QS::onReset(void) {
    onCleanup();
    exit(0);
}
//............................................................................
void QS::onFlush(void) {
    uint16_t nBytes;
    uint8_t const *data;

    if (l_sock == INVALID_SOCKET) { // socket NOT initialized?
        fprintf(stderr, "<TARGET> ERROR   invalid TCP socket\n");
        return;
    }

    nBytes = QS_TX_CHUNK;
    while ((data = getBlock(&nBytes)) != (uint8_t *)0) {
        for (;;) { // for-ever until break or return
            int nSent = send(l_sock, (char const *)data, (int)nBytes, 0);
            if (nSent == SOCKET_ERROR) { // sending failed?
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK) {
                    // sleep for the timeout and then loop back
                    // to send() the SAME data again
                    //
                    Sleep(QS_TIMEOUT_MS);
                }
                else { // some other socket error...
                    fprintf(stderr, "<TARGET> ERROR   sending data over TCP,"
                           "WASErr=%d\n", err);
                    return;
                }
            }
            else if (nSent < (int)nBytes) { // sent fewer than requested?
                Sleep(QS_TIMEOUT_MS); // sleep for the timeout

                // adjust the data and loop back to send() the rest
                data   += nSent;
                nBytes -= (uint16_t)nSent;
            }
            else {
                break; // break out of the for-ever loop
            }
        }
        // set nBytes for the next call to QS::getBlock()
        nBytes = QS_TX_CHUNK;
    }
}
//............................................................................
void QS::onTestLoop() {
    fd_set readSet;
    FD_ZERO(&readSet);

    rxPriv_.inTestLoop = true;
    while (rxPriv_.inTestLoop) {
        FD_SET(l_sock, &readSet);

        // selective, timed blocking on the TCP/IP socket...
        struct timeval timeout = {
            (long)0, (long)(QS_TIMEOUT_MS * 1000)
        };
        int status = select(0, &readSet, (fd_set *)0, (fd_set *)0, &timeout);
        if (status == SOCKET_ERROR) {
            fprintf(stderr,
                "<TARGET> ERROR socket select,WSAErr=%d",
                WSAGetLastError());
            onCleanup();
            exit(-2);
        }
        else if (FD_ISSET(l_sock, &readSet)) { // socket ready?
            status = recv(l_sock,
                          (char *)QS::rxPriv_.buf, (int)QS::rxPriv_.end, 0);
            if (status > 0) { // any data received?
                QS::rxPriv_.tail = 0U;
                QS::rxPriv_.head = status; // # bytes received
                QS::rxParse(); // parse all received bytes
            }
        }

        // flush the QS TX buffer
        onFlush();

        int ch = 0;
        while (_kbhit()) { // any key pressed?
            ch = _getch();
        }
        switch (ch) {
            case 'x':      // 'x' pressed?
            case 'X':      // 'X' pressed?
            case '\033': { // ESC pressed?
                onCleanup();
                exit(1);
                break;
            }
        }
    }
    // set inTestLoop to true in case calls to QS_onTestLoop() nest,
    // which can happen through the calls to QS_TEST_PAUSE().
    rxPriv_.inTestLoop = true;
}

} // namespace QP

