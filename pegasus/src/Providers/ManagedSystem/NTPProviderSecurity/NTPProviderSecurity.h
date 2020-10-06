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

#ifndef Pegasus_Security_h
#define Pegasus_Security_h

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
//Pegasus includes
#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
#include <Pegasus/Provider/ProviderException.h>
#include <Pegasus/Common/OperationContext.h>

// Security includes
#if !defined (PEGASUS_OS_VMS)
#include <sys/getaccess.h>
#endif
#include <grp.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

//------------------------------------------------------------------------------
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

// Options operation to verify access
static const String SEC_OPT_READ("r");
static const String SEC_OPT_WRITE("w");
static const String SEC_OPT_READ_WRITE("rw");
static const String SEC_OPT_EXECUTE("x");
static const String SEC_OPT_ALL("rwx");

// Parameter option to Operation context
static const Uint32 CONTEXT_ID = 1;

//------------------------------------------------------------------------------
// Class [NTPProviderSecurity] Definition
//------------------------------------------------------------------------------
class NTPProviderSecurity
{

public:
    NTPProviderSecurity(const OperationContext & context);
    virtual ~NTPProviderSecurity(void);

public:
    //
    // Public Functions - Interface
    //

    // This function retrieves the permissions of file by username
    Boolean checkAccess(
        const String filename,
        const String chkoper);

private:
    //
    // Class attributes
    //
    String secUsername;
};

#endif
