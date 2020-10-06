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
// Author: Barbara Packard (barbara_packard@hp.com)
//
// Modified By:
//
//%////////////////////////////////////////////////////////////////////////////

// WMIAssociatorProvider.h: interface for the WMIAssociatorProvider class.
//
//////////////////////////////////////////////////////////////////////

// WMIQueryProvider.h: interface for the WMIQueryProvider class.
//
//////////////////////////////////////////////////////////////////////

#ifndef Pegasus_WMIQueryProvider_h
#define Pegasus_WMIQueryProvider_h

#include <WMIMapper/WMIProvider/WMIBaseProvider.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_WMIPROVIDER_LINKAGE WMIQueryProvider : public WMIBaseProvider
{
public:
    WMIQueryProvider();
    virtual ~WMIQueryProvider();

    /// execQuery
    virtual Array<CIMObject> execQuery(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const String& queryLanguage,
        const String& query,
        const CIMPropertyList& propertyList = CIMPropertyList(),
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false) ;

protected:

private:

};

PEGASUS_NAMESPACE_END

#endif // #ifndef Pegasus_WMIQueryProvider_h
