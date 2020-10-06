/*
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
*/

#ifndef _Executor_Policy_h
#define _Executor_Policy_h

#include <stdlib.h>
#include "Defines.h"
#include "Messages.h"

/*
**==============================================================================
**
** Policy
**
**     This structure defines a policy rule.
**
**==============================================================================
*/

struct Policy
{
    enum ExecutorMessageCode messageCode;
    const char* arg1;
    const char* arg2;
    unsigned long flags;
};

EXECUTOR_LINKAGE
int CheckPolicy(
    const struct Policy* policyTable,
    size_t policyTableSize,
    enum ExecutorMessageCode messageCode,
    const char* arg1,
    const char* arg2,
    unsigned long* flags);

EXECUTOR_LINKAGE
int CheckOpenFilePolicy(const char* path, int mode, unsigned long* flags);

EXECUTOR_LINKAGE
int CheckRemoveFilePolicy(const char* path);

EXECUTOR_LINKAGE
int CheckRenameFilePolicy(const char* oldPath, const char* newPath);

EXECUTOR_LINKAGE
void DumpPolicyHelper(
    FILE* outputStream,
    const struct Policy* policyTable,
    size_t policyTableSize,
    int expandMacros);

EXECUTOR_LINKAGE
void DumpPolicy(FILE* outputStream, int expandMacros);

#endif /* _Executor_Policy_h */
