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

#ifndef Pegasus_AuthHandle_h
#define Pegasus_AuthHandle_h

#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

#ifndef PEGASUS_PAM_SESSION_SECURITY
// empty class representation for all authentication mechanisms not using an
// authentication handle
class PEGASUS_COMMON_LINKAGE AuthHandle
{
public:
    AuthHandle() {};
    ~AuthHandle() {};

    void destroy() {};
};

#else

class PEGASUS_COMMON_LINKAGE AuthHandle
{
public:
    AuthHandle();

    ~AuthHandle(){};

    void destroy();

    // PAM Handle pointer
    // Intentionally keeping definition as void* to avoid
    // #include <security/pam_appl.h> in this header file
    void * hdl;
};
#endif

PEGASUS_NAMESPACE_END

#endif   /* Pegasus_AuthHandle_h*/
