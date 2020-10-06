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
// Modified By: Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//              Terry Martin, Hewlett-Packard Company (terry.martin@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

// WMIMethodProvider.cpp: implementation of the WMIMethodProvider class.
//
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// WMIMethodProvider::
//
// ///////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/CIMInstance.h>

#include "Stdafx.h"

#include "WMIValue.h"
#include "WMIString.h"
#include "WMICollector.h"
#include "WMIBaseProvider.h"
#include "WMIClassProvider.h"
#include "WMIMethodProvider.h"

PEGASUS_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WMIMethodProvider::WMIMethodProvider()
{
    _collector = NULL;
    m_bInitialized = false;

}

WMIMethodProvider::~WMIMethodProvider()
{

}


 //////////////////////////////////////////////////////////////////////////////
// ATTN:
// The  following public methods are not yet implemented
//
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// WMIMethodProvider::invokeMethod
//
// ///////////////////////////////////////////////////////////////////////////
   /// invokeMethod
CIMValue WMIMethodProvider::invokeMethod(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const CIMObjectPath& instanceName,
        const String& methodName,
        const Array<CIMParamValue>& inParameters,
        Array<CIMParamValue>& outParameters)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIMethodProvider::invokeMethod()");

    setup(nameSpace, userName, password);

    // WMI Objects
    CComPtr<IWbemServices>        pServices = NULL;
    CComPtr<IWbemClassObject>    pClass = NULL;
    CComPtr<IWbemClassObject>    pOutInst = NULL;
    CComPtr<IWbemClassObject>    pInClass = NULL;
    CComPtr<IWbemClassObject>    pInInst = NULL;

    // Return Value
    CIMValue cimRetVal;

    // String variables
    String        strTmp;
    CComBSTR    bstrClassPath;
    CComBSTR    bstrMethodName = L"Create";

    HRESULT hr;

    // Connect to WMI
    if (!_collector->Connect(&pServices))
    {
        if (pServices)
            pServices.Release();

        throw CIMException(CIM_ERR_ACCESS_DENIED);
    }


    // Get the class for the method definition.
    bstrClassPath = instanceName.getClassName().getString().getCString();
    hr = pServices->GetObject(bstrClassPath, 0, NULL, &pClass, NULL);

    if (FAILED(hr))
    {
        if (pServices)
            pServices.Release();

        if (pClass)
            pClass.Release();

        switch(hr)
        {
            case WBEM_E_NOT_FOUND:
                throw CIMException(CIM_ERR_NOT_FOUND);
                break;
            default:
                throw CIMException(CIM_ERR_FAILED);
                break;
        }
    }


    // Get the input-argument class object and create an instance.
    bstrMethodName = methodName.getCString();
    hr = pClass->GetMethod(bstrMethodName, 0, &pInClass, NULL);

    if (FAILED(hr))
    {
        if (pServices)
            pServices.Release();

        if (pClass)
            pClass.Release();

        if (pInClass)
            pInClass.Release();

        switch(hr)
        {
            case WBEM_E_NOT_FOUND:
                throw CIMException(CIM_ERR_METHOD_NOT_FOUND);
                break;
            default:
                throw CIMException(CIM_ERR_FAILED);
                break;
        }
    }


    // Check if pInClass is NULL (it will occur when has no parameters)
    if (pInClass)
    {
        hr = pInClass->SpawnInstance(0, &pInInst);

        if (pInClass)
            pInClass.Release();

        if (FAILED(hr))
        {
            if (pServices)
                pServices.Release();

            if (pClass)
                pClass.Release();

            if (pInInst)
                pInInst.Release();

            throw CIMException(CIM_ERR_FAILED);
        }

        // Get the input parameters
        for (Uint32 i = 0; i < inParameters.size(); i++)
        {
            // Get parameter name
            CComBSTR bstrParamName;
            bstrParamName = inParameters[i].getParameterName().getCString();

            // Get parameter value
            WMIValue wmiParamValue(inParameters[i].getValue());
            CComVariant vParamValue;
            wmiParamValue.getAsVariant(
                    &vParamValue,
                    nameSpace,
                    userName,
                    password);

            if ((wmiParamValue.getType() == CIMTYPE_STRING) &&
                (!wmiParamValue.isArray()))
            {
                wmiParamValue.get(strTmp);
                vParamValue.bstrVal[strTmp.size()] = 0;
            }

            // Set parameter on the WMI
            hr = pInInst->Put(bstrParamName, 0, &vParamValue, 0);
            vParamValue.Clear();

            if (FAILED(hr))
            {
                if (pServices)
                    pServices.Release();

                if (pClass)
                    pClass.Release();

                if (pInInst)
                    pInInst.Release();

                throw CIMException(CIM_ERR_FAILED);
            }
        }
    }

    // Get the instance definition and Call the method.
    bstrClassPath = getObjectName(instanceName).getCString();

    hr = pServices->ExecMethod(bstrClassPath,
                               bstrMethodName,
                               0,
                               NULL,
                               pInInst,
                               &pOutInst,
                               NULL);
    if (pServices)
        pServices.Release();

    if (pClass)
        pClass.Release();

    if (pInInst)
        pInInst.Release();

    if (FAILED(hr))
    {
        if (pOutInst)
            pOutInst.Release();

        throw CIMException(CIM_ERR_FAILED);
    }

    // Check if pOutInst is NULL (it will ocurr when don't exist any return)
    if (pOutInst)
    {
        // Get output parameters
        hr = pOutInst->BeginEnumeration(WBEM_FLAG_LOCAL_ONLY);
        while (true)
        {
            // Get parameter name and value
            CComBSTR bstrParamName;
            CComVariant vParamValue;
            CIMTYPE cimType;
            hr = pOutInst->Next(0, &bstrParamName,
                &vParamValue, &cimType, NULL);

            // Check errors
            if (WBEM_S_NO_MORE_DATA == hr) break;
            if (FAILED(hr))
            {
                if (pOutInst)
                    pOutInst.Release();

                vParamValue.Clear();

                throw CIMException(CIM_ERR_FAILED);
            }


            // Convert to CIMParamValue
            WMIValue wmiParamValue(vParamValue, cimType);
            CMyString strAux; strAux = bstrParamName;
            CIMParamValue cimParamValue(String(strAux.Copy()), wmiParamValue);

            // Insert Parameter into Array
            // if it isn't the return value
            if (strAux.Compare("ReturnValue"))
                cimRetVal = cimParamValue.getValue();
            else
                outParameters.append(cimParamValue);

            vParamValue.Clear();
        }
        hr = pOutInst->EndEnumeration();
    }

    if (pOutInst)
        pOutInst.Release();

    PEG_METHOD_EXIT();

    // Return the last output parameter
    return cimRetVal;
}

PEGASUS_NAMESPACE_END
