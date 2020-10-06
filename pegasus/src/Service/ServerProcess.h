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

#ifndef Pegasus_Server_Process_h
#define Pegasus_Server_Process_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Service/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    This abstract class has virtual methods for information that varies across
    applications. The rest of the methods are called from the application
    (i.e. cimserver.cpp), and need to be defined by every operating system
    implementation.

    This version will not touch the method names, as it is fairly risky to do
    so now.  However, the goal is to eventually standardize the interface so
    we pull out as much OS-specific function as possible from the main
    cimserver file.

    Not all operating systems need to fully implement all these methods.
    Stick methods which do not apply to your OS in a "No-ops" section at the
    top.

    See PEP#222 for more information.
*/

class PEGASUS_SERVICE_LINKAGE ServerProcess
{
public:

    ServerProcess();

    virtual ~ServerProcess();

    virtual const char* getProductName() const = 0;

    virtual const char* getExtendedName() const = 0;

    virtual const char* getDescription() const = 0;

    virtual const char* getVersion() const = 0;

    virtual const char* getProcessName() const = 0;

    virtual int cimserver_run(
        int argc,
        char** argv,
        Boolean shutdownOption,
        Boolean debugOutputOption) = 0;

    virtual void cimserver_stop() = 0;

    int platform_run(
        int argc,
        char** argv,
        Boolean shutdownOption,
        Boolean debugOutputOption);

    int cimserver_fork();

    void notify_parent(int id);

    void cimserver_set_process(void* p);

    void cimserver_exitRC(int rc);

    int cimserver_initialize();

    // Currently (07/27/06) this function is only used by
    // pegasus/src/Pegasus/DynListener/Service/cimlistener.cpp
    // in the cimlistener it is used to wait for a signal
    // to shutdown

    // if PEGASUS_HAS_SIGNALS is defined this function waits in a sigwait
    // for either a SIGHUP or a SIGTERM and does not return before

    // if PEGASUS_HAS_SIGNALS is NOT defined this function is a noop function
    // returning immediately with -1
    int cimserver_wait();

    String getHome();
};

PEGASUS_NAMESPACE_END

#endif // Pegasus_Server_Process_h
