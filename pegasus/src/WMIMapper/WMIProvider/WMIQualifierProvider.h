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
// WMIQualifierProvider.h: interface for the WMIQualifierProvider class.
//
//////////////////////////////////////////////////////////////////////

#ifndef Pegasus_WMIQualifierProvider_h
#define Pegasus_WMIQualifierProvider_h

#include <WMIMapper/WMIProvider/WMIBaseProvider.h>

PEGASUS_NAMESPACE_BEGIN

class WMICollector;

class PEGASUS_WMIPROVIDER_LINKAGE WMIQualifierProvider :
    public WMIBaseProvider
{
public:
    WMIQualifierProvider();
    virtual ~WMIQualifierProvider();

///////////////////////////////////////////////////////////////////////////////
//    ATTN:
//    The following public methods have not been implemented
//    yet.
///////////////////////////////////////////////////////////////////////////////

    /// getQualifier
    virtual CIMQualifierDecl getQualifier(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const String& qualifierName);

    /// setQualifier
    virtual void setQualifier(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const CIMQualifierDecl& qualifierDecl);

    /// virtual deleteQualifier
    virtual void deleteQualifier(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const String& qualifierName);

    /// enumerateQualifiers
    virtual Array<CIMQualifierDecl> enumerateQualifiers(
        const String& nameSpace,
        const String& userName,
        const String& password);

protected:

private:

};


PEGASUS_NAMESPACE_END


#endif // !Pegasus_WMIQualifierProvider_h
