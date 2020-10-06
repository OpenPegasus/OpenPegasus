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
// Author: Kevin Howard, Hewlett-Packard Company (kevin.d.howard@hp.com)
//
// Modified By:    Terry Martin, Hewlett-Packard Company (terry.martin@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "WMIType.h"
#include "WMIMethod.h"
#include "WMIString.h"
#include "WMIValue.h"
#include "WMIQualifier.h"
#include "WMIQualifierSet.h"

#include "QueryStrings.h"

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////
//
// addQualifiersToCIMParameter
//
// Add qualifiers from a WMI class to a CIMParameter.
//
///////////////////////////////////////////////////////////////
void addQualifiersToCIMParameter(const CIMQualifierList& cimQualifierList,
                                 CIMParameter& cimParam,
                                 Boolean includeQualifiers)
{
    static const CIMName idName("ID");
    static const CIMName inName("in");
    static const CIMName outName("out");

    for (Uint32 i = 0, n = cimQualifierList.getCount(); i < n; i++)
    {
        CIMQualifier cimQual = cimQualifierList.getQualifier(i);

        // Add this qualifier if includeQualifiers is true or if
        // this is an "ID", "in", or "out" qualifier:
        if (includeQualifiers ||
            cimQual.getName().equal(inName) ||
            cimQual.getName().equal(outName) ||
            cimQual.getName().equal(idName))
        {
            cimParam.addQualifier(cimQualifierList.getQualifier(i));
        }
    }
}

////////////////////////////////////////////////////////////////
//
// cimParamFromWMIParam
//
///////////////////////////////////////////////////////////////
CIMParameter cimParamFromWMIParam(
        const String& strParamName,
        CIMTYPE wmiType,
        const CComPtr<IWbemQualifierSet>& pQualifiers,
        Boolean includeQualifiers)
{
    // Need to convert the qualifiers, even if includeQualifiers is FALSE
    // still need to get "in" and "out", as a min
    CIMQualifierList qualifierList;
    if (pQualifiers)
    {
        WMIQualifierSet(pQualifiers).cloneTo(qualifierList);
    }

    // Check for a reference type, and get the reference class if necessary:
    String referenceClass = String::EMPTY;
    if ((wmiType & ~CIM_FLAG_ARRAY) == CIM_REFERENCE)
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
    CIMName refClassName;
    if (0 != referenceClass.size())
    {
        refClassName = referenceClass;
    }

    // Check for Array type, and get the array size, if necessary:
    Boolean isArray = false;
    Uint32 arraySize = 0;
    if (wmiType & CIM_FLAG_ARRAY)
    {
        isArray = true;

        // If there is a MAX qualifier on this array, use it as the size:
        Uint32 pos = qualifierList.find(CIMName("MAX"));
        if (PEG_NOT_FOUND != pos)
        {
            qualifierList.getQualifier(pos).getValue().get(arraySize);
        }

        // remove the array flag from the type to get the base type:
        wmiType &= ~CIM_FLAG_ARRAY;
    }

    // NOTE: Currently no mapping of WMI "object" types, so this could
    // throw if the param is of type "object" or other un-supported
    // WMI type:
    CIMType cimType;
    try
    {
        cimType = WMITypeToCIMType(wmiType);
    }
    catch (TypeMismatchException tme)
    {
        // Don't want method enumeration to fail, just because
        // we don't handle the type, so making this type "reference"
        // for now and making the reference class name "UNKNOWN_TYPE":
        cimType = CIMTYPE_REFERENCE;
        refClassName = CIMName("UNKNOWN_TYPE");
    }

    // Build the CIMParameter:
    CIMParameter cimParam = CIMParameter(strParamName,
                                         cimType,
                                         isArray,
                                         arraySize,
                                         refClassName);

    // Now, add the parameter's qualifiers ("in" and "out" as a minimum):
    addQualifiersToCIMParameter(qualifierList, cimParam, includeQualifiers);

    // Finally, return the newly created CIMParameter:
    return cimParam;
}

////////////////////////////////////////////////////////////////
//
// addWMIParametersToCIMMethod
//
// Add parameters from a WMI class to a CIMMethod.
//
///////////////////////////////////////////////////////////////
void addWMIParametersToCIMMethod(
        const CComPtr<IWbemClassObject>& wmiParameters,
        CIMMethod& method,
        Boolean includeQualifiers)
{
    // Check if wmiParameters is NULL (this will ocurr when there are none)
    HRESULT hr;
    if (wmiParameters)
    {
        // Enumerate all output parameters
        hr = wmiParameters->BeginEnumeration(WBEM_FLAG_LOCAL_ONLY);
        while(true)
        {
            // Get current parameter name and value
            CComBSTR bstrParamName;
            CComVariant vParamValue;
            CIMTYPE wmiType;
            hr = wmiParameters->Next(0, &bstrParamName,
                &vParamValue, &wmiType, NULL);

            // Check errors
            if (WBEM_S_NO_MORE_DATA == hr)
            {
                break;
            }
            if (FAILED(hr))
            {
                bstrParamName.Empty();
                vParamValue.Clear();
                throw CIMException(CIM_ERR_FAILED);
            }

            // Convert to CIMParameter
            BSTR tmpBstr = (BSTR)bstrParamName.Copy();
            String parameterName(_bstr_t(tmpBstr, FALSE));
            SysFreeString(tmpBstr);

            // Add the parameter to this method if it is not the return value
            // and it does not already exist in the method.
            // If the parameter already exists (i.e., an in & out parameter),
            // then there is no need to re-add or modify it here
            // (the in version is an exact copy of the out version):
            String strRetVal("ReturnValue");
            if (!(String::equalNoCase(parameterName, strRetVal)) &&
                method.findParameter(parameterName) == PEG_NOT_FOUND)
            {
                // Get the qualifier list for this param from WMI:
                CComPtr<IWbemQualifierSet> pParamQualifiers;
                HRESULT hr =
                    wmiParameters->GetPropertyQualifierSet(bstrParamName,
                                                           &pParamQualifiers);

                // create the CIMParameter
                CIMParameter cimParam =
                    cimParamFromWMIParam(parameterName,
                                         wmiType,
                                         pParamQualifiers,
                                         includeQualifiers);

                // Now, add the new parameter to the CIMMethod:
                method.addParameter(cimParam);

                if (pParamQualifiers)
                    pParamQualifiers.Release();
            }

            bstrParamName.Empty();
            vParamValue.Clear();
        }
        hr = wmiParameters->EndEnumeration();
    }
}


//////////////////////////////////////////////////////////////////////
//
// WMIMethod constructors
//
//////////////////////////////////////////////////////////////////////
WMIMethod::WMIMethod(const CIMMethod & method) : CIMMethod(method)
{
}


WMIMethod::WMIMethod(const BSTR & name,
                     const CComPtr<IWbemClassObject>& inParameters,
                     const CComPtr<IWbemClassObject>& outParameters,
                     IWbemQualifierSet * pQualifierSet,
                     Boolean includeQualifiers)
{
    CIMQualifierList    qualifierList;
    CComBSTR            bsName = name;
    CComVariant            vValue = NULL;
    CIMTYPE                returnValueType;
    HRESULT            hr;

    WMIQualifierSet(pQualifierSet).cloneTo(qualifierList);

    // Get method return value
    String referenceClass = String::EMPTY;

    CComBSTR propertyName = L"ReturnValue";

// modified to correct bug JAGaf25827
// JAGaf25827 - new code begin
    if (outParameters)
    {
        hr = outParameters->Get(
            propertyName,
            0,
            &vValue,
            &returnValueType,
            NULL);
    }
    else
    {
        hr = WBEM_E_NOT_FOUND;
    }

    //    not found. Maybe it is a 'void' return value
    if (hr == WBEM_E_NOT_FOUND) {
        vValue = NULL;
        returnValueType = CIM_UINT32;
    }
    else if (hr != WBEM_S_NO_ERROR) {
        // Error: throw?
        throw Exception("WMIMethod::WMIMethod Get ReturnValue Failed.");
    }
    vValue.Clear();

    // the WMI 'CIMTYPE' qualifier stores a string that contains the reference
    // class name cimtype_qualifier ::= ["ref:" + <reference_class_name>]
    // NOTE: CIMMethod does not seem to store the reference class anywhere,
    // but it seems like it should, so this code is here in case a reference
    // class member is ever added to the CIMMethod class in the future:
    if (CIM_REFERENCE == returnValueType)
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
    // propagated qualifier ::= [<class_origin>"."]<method_name>
    {
        Uint32 pos = qualifierList.find(CIMName("Propagated"));

        if (PEG_NOT_FOUND != pos)
        {
            qualifierList.getQualifier(pos).getValue().get(classOrigin);

            // strip on the ".<method_name> portion if there...
            if ((pos = classOrigin.find(".")) != PEG_NOT_FOUND)
            {
                classOrigin.remove(pos);
            }
        }
    }

    // NOTE: Currently no mapping of WMI "object" types, so this could
    // throw if the param is of type "object" or other un-supported
    // WMI type:
    CIMType cimType;
    try
    {
        cimType = WMITypeToCIMType(returnValueType);
    }
    catch (TypeMismatchException tme)
    {
        // Don't want method enumeration to fail, just because
        // we don't handle the type, so making this type "reference"
        // for now and making the reference class name "UNKNOWN_TYPE":
        cimType = CIMTYPE_REFERENCE;
        referenceClass = "UNKNOWN_TYPE";
    }

    // Build the method
    CIMName cimRef;
    CIMName cimClsOrigin;
    BSTR tmpBstr = (BSTR)bsName.Copy();
    String s(_bstr_t(tmpBstr, FALSE));
    SysFreeString(tmpBstr);
    bsName.Empty();

    if (0 != referenceClass.size())
    {
        cimRef = referenceClass;
    }

    if (0 != classOrigin.size())
    {
        cimClsOrigin = classOrigin;
    }

    *this = CIMMethod(
        CIMName(s),
        cimType,
        cimClsOrigin,
        (classOrigin.size() != 0));


    // Add the qualifiers
    if (includeQualifiers)
    {
        Uint32 i, n;

        for (i = 0, n = qualifierList.getCount(); i < n; i++)
        {
            addQualifier(qualifierList.getQualifier(i));
        }
    }

    // Enumerate the parameters of the WMI method in and out classes here,
    // adding them to the CIMMethos as appropriate.
    // NOTE: In WMI parameters method parameters are defined as in, out, or
    // both, meaning that they cold show up in both the inParameters and
    // outParameters lists. The addWMIParametersToCIMMethod() function will
    // check for the existing parameters by name, and will not attempt to
    // re-add any existing params (there is no need, since the "in" and "out"
    // versions of an in/out param are simply identical copies)
    addWMIParametersToCIMMethod(outParameters, *this, includeQualifiers);
    addWMIParametersToCIMMethod(inParameters, *this, includeQualifiers);
}


PEGASUS_NAMESPACE_END
