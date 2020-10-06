//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Signal_h
#define Pegasus_Signal_h

#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Mutex.h>

#ifdef PEGASUS_HAS_SIGNALS

# include <signal.h>
typedef siginfo_t PEGASUS_SIGINFO_T;
# define PEGASUS_SIGHUP    SIGHUP
# define PEGASUS_SIGABRT   SIGABRT
# define PEGASUS_SIGPIPE   SIGPIPE
# define PEGASUS_SIGTERM   SIGTERM
# define PEGASUS_SIGUSR1   SIGUSR1
# define PEGASUS_SIGCHLD   SIGCHLD
# define PEGASUS_SIGDANGER SIGDANGER

#else // PEGASUS_HAS_SIGNALS

typedef void PEGASUS_SIGINFO_T;
# define PEGASUS_SIGHUP    1
# define PEGASUS_SIGABRT   11
# define PEGASUS_SIGPIPE   13
# define PEGASUS_SIGTERM   15
# define PEGASUS_SIGUSR1   16
# define PEGASUS_SIGCHLD   18
# define PEGASUS_SIGDANGER 33

#endif // PEGASUS_HAS_SIGNALS


extern "C" {
    typedef void (* signal_handler)(int, PEGASUS_SIGINFO_T *, void *);
}

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_COMMON_LINKAGE SignalHandler
{
public:
    SignalHandler();

    ~SignalHandler(); // deactivate all registered handlers

    // these functions should throw exceptions

    /**
        Registers a signal handler.
        @param signum The signal number for which to register the handler.
        @sighandler The signal handler to register
        @exception IndexOutOfBoundsException if signum is outside the range
            of signals supported by the SignalHandler (see PEGASUS_NSIG).
    */
    void registerHandler(unsigned signum, signal_handler sighandler);

    /**
        Activates a signal handler.
        @param signum The signal number for which to activate the handler.
        @exception IndexOutOfBoundsException if signum is outside the range
            of signals supported by the SignalHandler (see PEGASUS_NSIG).
    */
    void activate(unsigned signum);

    /**
        Deactivates a signal handler.
        @param signum The signal number for which to deactivate the handler.
        @exception IndexOutOfBoundsException if signum is outside the range
            of signals supported by the SignalHandler (see PEGASUS_NSIG).
    */
    void deactivate(unsigned signum);

    void deactivateAll();

    /**
        Sets a signal action to "ignore".
        @param signum The signal number to ignore.
        @exception IndexOutOfBoundsException if signum is outside the range
            of signals supported by the SignalHandler (see PEGASUS_NSIG).
    */
    static void ignore(unsigned signum);

    /**
        Resets a signal action to its default.
        @param signum The signal number for which to reset the signal action.
        @exception IndexOutOfBoundsException if signum is outside the range
            of signals supported by the SignalHandler (see PEGASUS_NSIG).
    */
    static void defaultAction(unsigned signum);

private:

#ifdef PEGASUS_HAS_SIGNALS
    enum
    {
        PEGASUS_NSIG = 33
    };

    static void verifySignum(unsigned signum);

    typedef struct {
        int signum;
        int active;
        signal_handler sh;
        struct sigaction oldsa;
    } register_handler;

    register_handler reg_handler[PEGASUS_NSIG + 1];
    Mutex reg_mutex;

    void deactivate_i(register_handler &rh);

    register_handler &getHandler(unsigned sigum);
#endif

};

PEGASUS_COMMON_LINKAGE SignalHandler * getSigHandle();

PEGASUS_NAMESPACE_END

#endif // Pegasus_Signal_h
