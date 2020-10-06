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

#ifndef _Pegasus_Common_CIMObjectPathRep_h
#define _Pegasus_Common_CIMObjectPathRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/HostLocator.h>

PEGASUS_NAMESPACE_BEGIN

class CIMObjectPathRep
{
public:
    CIMObjectPathRep(): _refCounter(1)
    {
    }

    CIMObjectPathRep(const CIMObjectPathRep& x)
        : _refCounter(1), _host(x._host), _nameSpace(x._nameSpace),
        _className(x._className), _keyBindings(x._keyBindings)
    {
    }

    CIMObjectPathRep(
        const String& host,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const Array<CIMKeyBinding>& keyBindings)
        : _refCounter(1), _host(host), _nameSpace(nameSpace),
        _className(className), _keyBindings(keyBindings)
    {
    }

    ~CIMObjectPathRep()
    {
    }

    CIMObjectPathRep& operator=(const CIMObjectPathRep& x)
    {
        if (&x != this)
        {
            _host = x._host;
            _nameSpace = x._nameSpace;
            _className = x._className;
            _keyBindings = x._keyBindings;
        }
        return *this;
    }

    static Boolean isValidHostname(const String& hostname)
    {
        HostLocator addr(hostname);

        return addr.isValid();
    }

    // reference counter as member to avoid
    // virtual function resolution overhead
    AtomicInt _refCounter;
    //
    // Contains port as well (e.g., myhost:1234).
    //
    String _host;

    CIMNamespaceName _nameSpace;
    CIMName _className;
    Array<CIMKeyBinding> _keyBindings;
};

PEGASUS_NAMESPACE_END

#endif /* _Pegasus_Common_CIMObjectPathRep_h */
