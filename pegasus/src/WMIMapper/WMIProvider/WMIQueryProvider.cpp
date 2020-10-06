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

// WMIQueryProvider.cpp: implementation of the WMIQueryProvider class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"


#include "WMICollector.h"
#include "WMIBaseProvider.h"
#include "WMIClassProvider.h"
#include "WMIInstanceProvider.h"
#include "WMIAssociatorProvider.h"
#include "WMIQueryProvider.h"

#include "WMIObjectPath.h"
#include "WMIProperty.h"
#include "WMIString.h"
#include "WMIValue.h"
#include "WMIQualifier.h"
#include "WMIQualifierSet.h"
#include "WMIType.h"
#include "WMIException.h"

//////////////////////////////////////////////////////////////////////////////
// WMIQueryProvider::
//
// ///////////////////////////////////////////////////////////////////////////
PEGASUS_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WMIQueryProvider::WMIQueryProvider()
{
    _collector = NULL;
    m_bInitialized = false;

}

WMIQueryProvider::~WMIQueryProvider()
{
    cleanup();

}

/////////////////////////////////////////////////////////////////////////////
// WMIQueryProvider::execQuery
//
// ///////////////////////////////////////////////////////////////////////////
Array<CIMObject> WMIQueryProvider::execQuery(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const String& queryLanguage,
        const String& query,
        const CIMPropertyList& propertyList,
        Boolean includeQualifiers,
        Boolean includeClassOrigin)
{
    HRESULT hr;
    long lCount = 0;
    DWORD dwReturned;
    bool bInst;

    CComPtr<IEnumWbemClassObject>    pObjEnum;
    CComPtr<IWbemClassObject>        pObject;

    Array<CIMObject> objects;
    CIMName className;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIQueryProvider::execQuery()");

    setup(nameSpace,userName,password);

    if (!m_bInitialized)
    {
        throw CIMException(CIM_ERR_FAILED);
    }

    // retrieve results
    if (!(_collector->getQueryResult(&pObjEnum, query, queryLanguage)))
    {
        if (pObjEnum)
            pObjEnum.Release();

        throw CIMException(CIM_ERR_FAILED);
    }

    //set proxy security on pObjEnum
    bool bSecurity = _collector->setProxySecurity(pObjEnum);

    //get the results and append them to the array
    hr = pObjEnum->Next(WBEM_INFINITE, 1, &pObject, &dwReturned);

    if (SUCCEEDED(hr) && (1 == dwReturned))
    {
        bInst = _collector->isInstance(pObject);

        CComVariant mVar;
        CComBSTR bs;
        CMyString mstr;

        HRESULT hRes = pObject->Get(L"__CLASS", 0, &mVar, 0, 0);
        if (SUCCEEDED(hRes)) {
           bs = mVar.bstrVal;
           mstr = bs;
           className = CIMName((LPCTSTR)mstr);
        }
        mVar.Clear();

    }


    while (SUCCEEDED(hr) && (1 == dwReturned))
    {
        // collect the information about the current object
        if (bInst)
        {
            //get class from the returned instance
            //it will avoid "type mismatch" exceptions
            //when deepInheritance is true and instances
            //of subclasses are returned
            CComVariant vTmpClassName;
            String strTmpClassName;
            if (pObject->Get(L"__CLASS", 0, &vTmpClassName, NULL, NULL)
                == S_OK)
            {
                strTmpClassName = WMIString(vTmpClassName);
            }

            CIMInstance tempInst(strTmpClassName);

            if (_collector->getCIMInstance(
                    pObject, tempInst,
                    false, includeQualifiers, includeClassOrigin,
                    propertyList,
                    true))  //get key properties
            {
                lCount++;

                // build the object path
                CComVariant v;
                hr = pObject->Get(L"__PATH",
                                    0,
                                    &v,
                                    NULL,
                                    NULL);

                WMIObjectPath tempRef(v.bstrVal);
                tempInst.setPath(tempRef);
                v.Clear();

                objects.append(CIMObject(tempInst));
            }
        }
        else
        {
            // we are collecting a class
            CIMClass cimClass;
            String superClass = _collector->getSuperClass(pObject);
            CIMName objName = className;

            cimClass = CIMClass(objName);

            if (0 != superClass.size())
            {
                CIMName superClassName = superClass;
                cimClass.setSuperClassName(superClassName);
            }

            if (_collector->getCIMClass(pObject,
                                        cimClass,
                                        false,
                                        includeQualifiers,
                                        includeClassOrigin,
                                        propertyList))
            {
                lCount++;
                objects.append(CIMObject(cimClass));
            }
        }

        if (pObject)
            pObject.Release();

        hr = pObjEnum->Next(WBEM_INFINITE, 1, &pObject, &dwReturned);

        if (SUCCEEDED(hr) && (1 == dwReturned))
        {
            bInst = _collector->isInstance(pObject);
        }
    }

    if (pObjEnum)
        pObjEnum.Release();

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL4,
        "WMIQueryProvider::execQuery() - Result count is %d", lCount));

    if (lCount == 0)
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL2,
            "WMIQueryProvider::execQuery() - hResult value is %x", hr));
    }

    PEG_METHOD_EXIT();

    return objects;
}

PEGASUS_NAMESPACE_END

