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
// Author: Thilo Boehm (tboehm@de.ibm.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef AutoRestartMgr_ZOS_ZSERIES_IBM_h
#define AutoRestartMgr_ZOS_ZSERIES_IBM_h

////////////////////////////////////////////////////////////////////////////////
// internal prototypes for the assembler services called
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "OS_UPSTACK" {
#endif


////////////////////////////////////////////////////////////////////////////////
// assembler routine which registers with ARM
////////////////////////////////////////////////////////////////////////////////

#pragma map    (__register_arm,"REGARM")
void __register_arm (char * elemname,  // elementname (16bytes) right-padded
                                     // with blanks
                  char * buffer,     // must point to a local buffer which has
                                     // a length of least 128 bytes
                  int * retcode,     // contains returncode on return
                  int * reasoncode); // contains reasoncode on return

////////////////////////////////////////////////////////////////////////////////
// assembler routine put element as READY with ARM
////////////////////////////////////////////////////////////////////////////////

#pragma map    (__ready_arm,"READYARM")
void __ready_arm ( char * buffer,   // must point to a local buffer which has
                                    // a length of least 128 bytes
                 int * retcode,     // contains returncode on return
                 int * reasoncode); // contains reasoncode on return

////////////////////////////////////////////////////////////////////////////////
// assembler routine which deregisters element from ARM
////////////////////////////////////////////////////////////////////////////////

#pragma map    (__deregister_arm,"DEREGARM")
void __deregister_arm ( char * buffer, // must point to a local buffer which has
                                    // a length of least 128 bytes
                 int * retcode,     // contains returncode on return
                 int * reasoncode); // contains reasoncode on return

#ifdef __cplusplus
}
#endif

#endif  /* ifndef ARM_H */
