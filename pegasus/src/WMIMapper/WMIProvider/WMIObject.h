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
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_WMIObject_h
#define Pegasus_WMIObject_h

#include <Pegasus/Common/CIMObject.h>

#include "WMIBaseProvider.h"

PEGASUS_NAMESPACE_BEGIN

class WMICollector;

class PEGASUS_WMIPROVIDER_LINKAGE WMIObjectProvider : public WMIBaseProvider
{
public:
    WMIObjectProvider(void);
    virtual ~WMIObjectProvider(void);

    virtual bool getObject(
        IWbemClassObject **ppObject,
        const CIMObjectPath& objectName,
        const String& nameSpace,
        const String& userName = String::EMPTY,
        const String& password = String::EMPTY);
};

class PEGASUS_WMIPROVIDER_LINKAGE WMIObject : public CIMObject
{
public:
  WMIObject(const WMIObject & pObject);

  WMIObject(const CIMObject & pObject);

  WMIObject(const CComPtr <IWbemClassObject>& pObject);

    // returns a variant type from the WMI
    // username and password not present, then Local Namespace is assumed
    VARIANT toVariant(const String& nameSpace,
                      const String& userName = String::EMPTY,
                      const String& password = String::EMPTY);

protected:
   WMIObject(void) { };

};

#define PEGASUS_ARRAY_T WMIObject
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif
