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

#ifndef Pegasus_AutoPThreadSecurity_h
#define Pegasus_AutoPThreadSecurity_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/ProviderManager2/Linkage.h>

#ifdef PEGASUS_ZOS_SECURITY
// This include file will not be provided in the OpenGroup CVS for now.
// Do NOT try to include it in your compile
# include <Pegasus/ProviderManager2/ProviderManagerzOS_inline.h>
#endif

PEGASUS_NAMESPACE_BEGIN

// Auto class to encapsulate enabling and disabling
// of the pthread_security on z/OS
// For all other platforms this should be an empty class
// Targets: avoid ifdefs and keep code readable(clean)
#ifndef PEGASUS_ZOS_SECURITY

// not z/OS == empty class
class AutoPThreadSecurity
{
public:
    AutoPThreadSecurity(const OperationContext& context, bool reverse=false) {};
};

#else

class AutoPThreadSecurity
{
public:
    AutoPThreadSecurity(const OperationContext& context, bool reverse=false):
        _reverse(reverse)
    {
        if (!_reverse)
        {
            int err_num=enablePThreadSecurity(context);
            if (err_num!=0)
            {
                // need a new CIMException for this
                throw CIMException(
                    CIM_ERR_ACCESS_DENIED,
                    String(strerror(err_num)));
            }
        }
        else
        {
            // remember the context, it will be needed in the destructor
            internalOpContextReference = &(context);
            revDisablePThreadSecurity();        
        }
    };

    ~AutoPThreadSecurity()
    {
        if (!_reverse)
        {
            disablePThreadSecurity();
        }
        else
        {
            revEnablePThreadSecurity(*internalOpContextReference);
        }
    };
private:
    const OperationContext * internalOpContextReference;
    bool _reverse;
};

#endif

PEGASUS_NAMESPACE_END

#endif
