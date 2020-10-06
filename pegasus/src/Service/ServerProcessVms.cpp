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
// Author: Sean Keenan (sean.keenan@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <Pegasus/Common/Signal.h>

#define MAX_WAIT_TIME 25

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

Boolean handleSigUsr1 = false;

String newPortNumber = "";
String pegasusTrace  = "";


void sigUsr1Handler(int s_n, PEGASUS_SIGINFO_T * s_info, void * sig)
{
    handleSigUsr1 = true;
}

//constructor
ServerProcess::ServerProcess() {}

//destructor
ServerProcess::~ServerProcess() {}

// no-ops
int ServerProcess::cimserver_fork(void) { return 0; }
void ServerProcess::cimserver_set_process(void* p) {}
void ServerProcess::cimserver_exitRC(int rc) {}
int ServerProcess::cimserver_initialize(void) { return 1; }
int ServerProcess::cimserver_wait(void) { return 1; }
String ServerProcess::getHome(void) { return String::EMPTY; }

// notify parent process to terminate so user knows that cimserver
// is ready to serve CIM requests.
void ServerProcess::notify_parent(int id)
{
  pid_t ppid = getppid();
  if (id)
   kill(ppid, SIGTERM);
  else
   kill(ppid, PEGASUS_SIGUSR1);
}

// Platform specific run
int ServerProcess::platform_run(
    int argc,
    char** argv,
    Boolean shutdownOption,
    Boolean debugOutputOption)
{
//  newPortNumber = "";
//  pegasusTrace = "";
    return cimserver_run(argc, argv, shutdownOption, debugOutputOption);
}


