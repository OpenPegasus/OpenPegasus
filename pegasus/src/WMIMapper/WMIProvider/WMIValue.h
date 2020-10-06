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
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By:    Barbara Packard (barbara_packard@hp.com)
//                Paulo Sehn        (paulo_sehn@hp.com)
//                Adriano Zanuz    (adriano.zanuz@hp.com)
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_WMIValue_h
#define Pegasus_WMIValue_h

#include <Pegasus/Common/CIMValue.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_WMIPROVIDER_LINKAGE WMIValue : public CIMValue
{
public:
   WMIValue(const CIMValue & value);
   WMIValue(const VARIANT & value, const CIMTYPE type);
   WMIValue(const VARTYPE vt, void *pVal);
   WMIValue(const VARTYPE vt, void *pVal, const CIMTYPE Type);    // for arrays
   WMIValue(const CComVariant & vValue);

   VARIANT toVariant();
   operator VARIANT(void) const;

   // returns a variant type from the WMIVAlue
   // username and password not present, then Local Namespace is assumed
   void getAsVariant(CComVariant *var,
                     const String& nameSpace = String::EMPTY, // used only to...
                     const String& userName = String::EMPTY, // retrieve from...
                     const String& password = String::EMPTY);// WMIObject Values

protected:
   WMIValue(void) { };

private:
    CIMValue getCIMValueFromVariant(
        VARTYPE vt, void *pVal, const CIMTYPE Type = CIM_ILLEGAL);
    CIMValue getArrayValue(
        const CComVariant & vValue, const CIMTYPE Type = CIM_ILLEGAL);
    CIMValue getValue(
        const CComVariant & vValue, const CIMTYPE Type = CIM_ILLEGAL);

    bool isArrayType(VARTYPE vt)
        {return (vt & VT_ARRAY) ? true : false;}

    bool isReferenceType(VARTYPE vt)
        {return (vt & VT_BYREF) ? true : false;}


};

PEGASUS_NAMESPACE_END

#endif
