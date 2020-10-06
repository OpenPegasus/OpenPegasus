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

#include <ctest.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <Pegasus/Common/Tracer.h>

//
// This functions has been duplicated to 
// src/slp/slp_client/src/cmd-utils/slp_client/slp_client.cpp to solve 
// a circular build dependency.
// To be able to build the library pegslp_client prior pegcommon it was 
// necessary to copy this function.
// 


void __pegasus_assert_zOS(const char* file, int line, const char* cond)
{
    // a buffer to compose the messages
    char msgBuffer[1024];

    sprintf(msgBuffer,"PEGASUS_ASSERT: Assertation \'%s\' failed",cond);
    fprintf(stderr,"\n%s in file %s ,line %d\n",msgBuffer,file,line);

    // generate stacktace
    ctrace(msgBuffer);

    // flush trace buffers
    Pegasus::Tracer::flushTrace();

    // If env vars are set, a SYSM dump is generated.
    kill(getpid(),SIGDUMP);
}


