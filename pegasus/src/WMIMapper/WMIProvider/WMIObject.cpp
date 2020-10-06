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

#include "StdAfx.h"

#include <Pegasus/Common/CIMInstance.h>

#include "WMIObject.h"
#include "WMIProperty.h"
#include "WMIValue.h"
#include "WMICollector.h"

PEGASUS_NAMESPACE_BEGIN


//////////////////////////////////////////////////////////////////////
// WMIObjectProvider
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
WMIObjectProvider::WMIObjectProvider(void)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIObjectProvider::constructor()");

    _collector = NULL;
    m_bInitialized = false;

    PEG_METHOD_EXIT();
}

WMIObjectProvider::~WMIObjectProvider(void)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIObjectProvider::destructor()");

    cleanup();

    PEG_METHOD_EXIT();
}



bool WMIObjectProvider::getObject(
    IWbemClassObject **ppObject,
    const CIMObjectPath& objectName,
    const String& nameSpace,
    const String& userName,
    const String& password)

{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIObjectProvider::getObject");

    setup(nameSpace, userName, password);

    String sObjectName = getObjectName(objectName);

    if (!m_bInitialized)
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
            "WMIObjectProvider::getObject - m_bInitilized= %x, "
            "throw CIM_ERR_FAILED exception",
            m_bInitialized));

        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "Collector initialization failed.");
    }

    // retrieve instance object
    if (!(_collector->getObject(ppObject, sObjectName)))
    {
        PEG_TRACE((
            TRC_WMIPROVIDER,
            Tracer::LEVEL3,
            "WMIObjectProvider::getObject(): Object not Found"));

        throw CIMException(CIM_ERR_NOT_FOUND);
    }
    else if (_collector->isInstance(*ppObject))
    {
        PEG_TRACE((
            TRC_WMIPROVIDER,
            Tracer::LEVEL3,
            "WMIObjectProvider::getObject(): Cannot be an instance"));

        throw CIMException(CIM_ERR_INVALID_PARAMETER);
    }


    PEG_METHOD_EXIT();
    return true;
}

//////////////////////////////////////////////////////////////////////
// WMIObject
//////////////////////////////////////////////////////////////////////

WMIObject::WMIObject(const WMIObject & pObject) : CIMObject(pObject)
{

}

WMIObject::WMIObject(const CIMObject & pObject) : CIMObject(pObject)
{

}

WMIObject::WMIObject(const CComPtr <IWbemClassObject>& pObject)
{

    WMICollector tmpCollector;

    CIMObject tmpObj(CIMInstance(CIMName(tmpCollector.getClassName(pObject))));

    tmpCollector.getCIMObject(
        pObject,
        tmpObj,
        true,
        true,
        true);


   *this = tmpObj;

}

VARIANT WMIObject::toVariant(const String& nameSpace,
                             const String& userName,
                             const String& password)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIObject::toVariant()");

    VARIANT mVar;
    HRESULT hr;
    CComPtr<IWbemClassObject> pObj;
    CComPtr<IWbemLocator>    pLocator;
    CComPtr<IWbemServices> pServices;
    bool boolIsLocal;
    WMIObjectProvider provider;

    if (nameSpace == String::EMPTY)
    {
        PEG_TRACE((TRC_WMIPROVIDER,
                   Tracer::LEVEL3,
                   "namespace cannot be empty"));

        throw CIMException(CIM_ERR_FAILED);
    }

    if ((userName == String::EMPTY) && (password == String::EMPTY))
    {
        boolIsLocal = true;
    }
    else if ((userName != String::EMPTY) && (password != String::EMPTY))
    {
        boolIsLocal = false;
    }
    else
    {
        PEG_TRACE((TRC_WMIPROVIDER,
                   Tracer::LEVEL3,
                   "userName and password must be both /"
                   "empty or both non empty"));

        throw CIMException(CIM_ERR_FAILED);
    }

    CIMObjectPath mObjPath(getPath());

    provider.getObject(
        &pObj,
        mObjPath,
        nameSpace,
        userName,
        password);


     // If Object is an instance, then copy all the property values
     if (isInstance())
     {
         for (Uint32 i = 0; i < getPropertyCount(); i++)
         {
             WMIProperty mProperty(getProperty(i));

             CIMName mName(mProperty.getName());
             WMIValue mValue(mProperty.getValue());

             CComBSTR bstrPropName;
             bstrPropName = mName.getString().getCString();

             CComVariant mPropVar;
             mValue.getAsVariant(&mPropVar, nameSpace, userName, password);

             hr = pObj->Put(bstrPropName, 0, &mPropVar, 0);

             if (hr != WBEM_S_NO_ERROR)
             {

                 PEG_TRACE((TRC_WMIPROVIDER,
                 Tracer::LEVEL3, "WMIObject::toVariant() Input /"
                 "Property Copy Error. Property [ %s ] discarded."));
             }
         }
     }

    IWbemClassObject *pFinalObj = NULL;
    pObj->Clone(&pFinalObj);
    mVar.vt = VT_UNKNOWN;
    mVar.punkVal = pFinalObj;

    PEG_METHOD_EXIT();
    return mVar;
}



PEGASUS_NAMESPACE_END
