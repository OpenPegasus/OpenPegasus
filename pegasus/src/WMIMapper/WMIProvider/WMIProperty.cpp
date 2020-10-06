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
//              Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//              Terry Martin, Hewlett-Packard Company (terry.martin@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "WMIProperty.h"
#include "WMIString.h"
#include "WMIValue.h"
#include "WMIQualifier.h"
#include "WMIQualifierSet.h"
#include "WMIProperty.h"

#include "QueryStrings.h"

PEGASUS_NAMESPACE_BEGIN

WMIProperty::WMIProperty(const CIMProperty & property) : CIMProperty(property)
{
}

WMIProperty::WMIProperty(const BSTR & name,
                         const VARIANT & value,
                         const CIMTYPE type,
                         IWbemQualifierSet * pQualifierSet,
                         Boolean includeQualifiers)
{
    CIMQualifierList qualifierList;
    CComBSTR bsName = name;
    CComVariant vValue = value;
    WMIQualifierSet(pQualifierSet).cloneTo(qualifierList);

    // get additional property information
    String referenceClass = String::EMPTY;

    // the WMI 'CIMTYPE' qualifier stores a string that contains the reference
    // class name cimtype_qualifier ::= ["ref:" + <reference_class_name>]
    if (CIM_REFERENCE == type)
    {
    // strip "ref:"
        Uint32 pos = qualifierList.find(CIMName("CIMTYPE"));

        if (PEG_NOT_FOUND != pos)
        {
            qualifierList.getQualifier(pos).getValue().get(referenceClass);

            //strip off "ref:" or, if not found, erase the whole string
            if ((pos = referenceClass.find(qString(Q_COLON))) != PEG_NOT_FOUND)
            {
                referenceClass.remove(0, pos + 1);
            }
        }
    }

    String classOrigin = String::EMPTY;

    // the 'Propagated" qualifier stores a string containing the class origin
    // propagated qualifier ::= [<class_origin>"."]<property_name>
    {
        Uint32 pos = qualifierList.find(CIMName("Propagated"));

        if (PEG_NOT_FOUND != pos)
        {
            qualifierList.getQualifier(pos).getValue().get(classOrigin);

            // strip on the ".<property_name> portion if there...
            if ((pos = classOrigin.find(".")) != PEG_NOT_FOUND)
            {
                classOrigin.remove(pos);
            }
        }
    }

    // build the property
    BSTR tmpBstr = (BSTR)bsName.Copy();
    String s(_bstr_t(tmpBstr, FALSE));
    SysFreeString(tmpBstr);
    // old way -- memory leak?:
    // String s = WMIString(bsName);
    CIMName cimRef;
    CIMName cimClsOrigin;

    if (0 != referenceClass.size())
    {
        cimRef = referenceClass;
    }

    if (0 != classOrigin.size())
    {
        cimClsOrigin = classOrigin;
    }

    *this = CIMProperty(CIMName(s), WMIValue(vValue, type), 0, cimRef,
                cimClsOrigin, (classOrigin.size() != 0));

    vValue.Clear();

    // add the qualifiers
    if (includeQualifiers)
    {
        Uint32 i, n;

        for (i = 0, n = qualifierList.getCount(); i < n; i++)
        {
            addQualifier(qualifierList.getQualifier(i));
        }
    }

}

WMIProperty::WMIProperty(
        const BSTR & name,
        const VARIANT & value,
        const CIMTYPE type)
{
    CComBSTR bsName = name;
    CComVariant vValue = value;

    // build the property
    BSTR tmpBstr = (BSTR)bsName.Copy();
    String s(_bstr_t(tmpBstr, FALSE));
    SysFreeString(tmpBstr);
    *this = CIMProperty(CIMName(s), WMIValue(vValue, type));
    // old way -- memory leak?:
    // *this = CIMProperty(WMIString(bsName), WMIValue(vValue, type));

    vValue.Clear();
}


PEGASUS_NAMESPACE_END
