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
//%///////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/CIMInstance.h>

#include "StdAfx.h"

#include "WMIType.h"
#include "WMIFlavor.h"
#include "WMIValue.h"
#include "WMIString.h"
#include "WMIProperty.h"
#include "WMIMethod.h"
#include "WMICollector.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
WMICollector::WMICollector(bool bLocal)
{
    m_bInitialized = false;
    m_bIsLocalNamespace = false;
    m_bImpersonate = false;
    m_bLocalConnection = bLocal;
}

WMICollector::~WMICollector(void)
{
}

void WMICollector::terminate(void)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMICollector::terminate()");

    if (m_bInitialized)
    {
        CoUninitialize();
        m_bInitialized = false;

        if (m_bImpersonate)
        {
            revertToSelf();
            m_bImpersonate = false;
        }

        m_bIsLocalNamespace = false;
    }

    PEG_METHOD_EXIT();
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::setup - calls CoInitialize stuff
//
/////////////////////////////////////////////////////////////////////////////
bool WMICollector::setup()
{
    HRESULT hr;
    static bool bCallInitSec = true;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMICollector::setup()");

    if (!m_bInitialized)
    {
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
            "WMICollector::setup() - return from CoInitializeEx() is %x", hr));

        m_bInitialized = (SUCCEEDED(hr));

        if (m_bInitialized)
        {

            if (bCallInitSec)
            {
                bCallInitSec = false;
                hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
                        RPC_C_AUTHN_LEVEL_DEFAULT,
                        RPC_C_IMP_LEVEL_IMPERSONATE,
                        NULL,
                        EOAC_DYNAMIC_CLOAKING,
                        0);
            }
        }
    }

    PEG_METHOD_EXIT();
    return m_bInitialized;
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::Connect- attempt to establish a WMI connection with the local
//        host
//
/////////////////////////////////////////////////////////////////////////////
bool WMICollector::Connect(IWbemServices **ppServices)
{
    //
    // CComPtr is a smart pointer, therefore, it doesn't need to be explicitely
    // released.
    //
    CComPtr<IWbemLocator> pLocator;
    CComPtr<IWbemServices> pServices;
    HRESULT hr = S_OK;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER, "WMICollector::Connect()");

    // get the Locator object
    hr = pLocator.CoCreateInstance(CLSID_WbemLocator);

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
        "WMICollector::Connect() - return from CoCreateInstance() is %x", hr));

    if (SUCCEEDED(hr))
    {
        if (m_bIsLocalNamespace)
        {
            PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL4,
                "WMICollector::Connect() - m_bIsLocalNamespace is true");

            //Does not impersonate if is being called from a client app
            //Impersonate if it is being called from the Mapper service
            if (!m_bLocalConnection)
            {
                PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL4,
                    "WMICollector::Connect() - m_bLocalConnection is false, "
                    "call logonUser()");
                logonUser();
            }

            hr = pLocator->ConnectServer(m_bsNamespace,
                                         NULL,
                                         NULL,
                                         NULL, 0L, NULL, NULL,
                                         &pServices);
        }
        else
        {

            PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL4,
                "WMICollector::Connect() - m_bIsLocalNamespace is false");
            //---------------------------------------------------
            //by Jair
            //adjusting the username in order to add the domain
            //reference
            CComBSTR bsUser = m_bsDomain;
            bsUser.Append("\\");
            bsUser.Append(m_bsUserName);

            //---------------------------------------------------

            hr = pLocator->ConnectServer(m_bsNamespace,
                                         bsUser,
                                         m_bsPassword,
                                         NULL, 0L, NULL, NULL,
                                         &pServices);
        }

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
            "WMICollector::Connect() - return from "
            "ConnectServer() is %x", hr));

        if (FAILED(hr))
        {
            _com_error myError(hr);
            switch(hr)
            {
                case E_ACCESSDENIED:
                    PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL1,
                        "WMICollector::Connect() connectServer() - throw "
                        "CIM_ERR_ACCESS_DENIED for E_ACCESSDENIED error");
                    throw CIMException(CIM_ERR_ACCESS_DENIED);
                case WBEM_E_ACCESS_DENIED:
                    PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL1,
                        "WMICollector::Connect() - connectServer() - throw "
                        "CIM_ERR_ACCESS_DENIED error");
                    throw CIMException(CIM_ERR_ACCESS_DENIED);
                default:
                    PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL1,
                        "WMICollector::Connect() - connectServer() - throw "
                        "CIM_ERR_INVALID_NAMESPACE error");
                    throw CIMException(CIM_ERR_INVALID_NAMESPACE,
                        myError.ErrorMessage());
            }
        }

        if (SUCCEEDED(hr))
        {
            // set security attributes on pServices
            if (setProxySecurity(pServices))
            {
                pServices.CopyTo(ppServices);
            }
            else
            {
                *ppServices = NULL;
            }
        }
    }

    PEG_METHOD_EXIT();

    return (SUCCEEDED(hr));
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::getInstanceEnum - fetch a pointer to an enumeration of the
//        instances of sClassname
//
/////////////////////////////////////////////////////////////////////////////
bool WMICollector::getInstanceEnum(
    IEnumWbemClassObject **ppInstances,
    const String & sClassName,
    Boolean deepInheritance)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMICollector::getInstanceEnum()");

    HRESULT hr;
    long lFlags = WBEM_FLAG_FORWARD_ONLY;

    //
    // CComPtr is a smart pointer, therefore, it doesn't need to be explicitely
    // released.
    //
    CComPtr<IWbemServices> pServices;
    CComPtr<IEnumWbemClassObject> p_inst;

    bool bConnected = Connect(&pServices);

    if (!bConnected)
    {
        PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMICollector::getInstanceEnum,() - bConnected is false - "
            "throw Connect failed error");

        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_ACCESS_DENIED, "Connect failed.");
    }

    CComBSTR bsClassName = sClassName.getCString();

    deepInheritance ? lFlags |= WBEM_FLAG_DEEP : lFlags |= WBEM_FLAG_SHALLOW;

    // retrieve instances
    hr = pServices->CreateInstanceEnum(
        bsClassName,
        lFlags,
        NULL,
        &p_inst);

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
        "WMICollector::getInstanceEnum() return from CreateInstanceEnum()-"
            " hr value is %x", hr));

    if (SUCCEEDED(hr))
    {
        p_inst.CopyTo(ppInstances);

        // set security attributes on *ppInstances
        bool bSecurity = setProxySecurity(*ppInstances);
    }
    else
    {
        *ppInstances = NULL;
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMICollector::getInstanceEnum() - hr value is %x", hr));

        switch(hr)
        {
            case WBEM_E_ACCESS_DENIED:
                throw CIMException(CIM_ERR_ACCESS_DENIED);
            case WBEM_E_INVALID_CLASS:
                throw CIMException(CIM_ERR_INVALID_CLASS);
            default:
                throw CIMException(CIM_ERR_FAILED,
                    "[getInstanceEnum] general");
        }
    }

    PEG_METHOD_EXIT();

    return SUCCEEDED(hr);
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::getClassEnum - fetch a pointer to an enumeration of the
//        classes satisfying the criteria
//
/////////////////////////////////////////////////////////////////////////////
bool WMICollector::getClassEnum(
    IEnumWbemClassObject **ppClasses,
    const String & sSuperClass,
    Boolean deepInheritance)
{
    HRESULT hr;

    long lFlags = WBEM_FLAG_FORWARD_ONLY;
    CComBSTR bsSuperClass = NULL;

    CComPtr<IWbemServices> pServices;
    CComPtr<IEnumWbemClassObject> p_class;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMICollector::getClassEnum()");

    bool bConnected = Connect(&pServices);

    if (!bConnected)
    {
        PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMICollector::getClassEnum,() - bConnected is false - "
                "throw Connect failed error");

        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_ACCESS_DENIED, "Connect failed.");
    }

    //
    // workaround because there is no NULL for String class...
    //
    if (0 != sSuperClass.size())
    {
        bsSuperClass = sSuperClass.getCString();
    }

    deepInheritance ? lFlags |= WBEM_FLAG_DEEP : lFlags |= WBEM_FLAG_SHALLOW;

    if (0 != sSuperClass.size())
    {
        // retrieve classes
        hr = pServices->CreateClassEnum(
            bsSuperClass,
            lFlags,
            NULL,
            &p_class);
    }
    else
    {
        // retrieve classes
        hr = pServices->CreateClassEnum(
            NULL,
            lFlags,
            NULL,
            &p_class);
    }

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
        "WMICollector::getClassEnum() returns from CreateClassEnum() -"
        " hr value is %x", hr));

    if (SUCCEEDED(hr))
    {
        p_class.CopyTo(ppClasses);

        // set security attributes on *ppClasses
        bool bSecurity = setProxySecurity(*ppClasses);
    }
    else
    {
        *ppClasses = NULL;

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMICollector::getClassEnum() - hr value is %x", hr));

        switch(hr)
        {
            case WBEM_E_ACCESS_DENIED:
                PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL1,
                    "WMICollector::getClassEnum() - createClassEnum() returns "
                    "ACCESS_DENIED- throw CIM_ERROR_ACCESS_DENIED error");
                throw CIMException(CIM_ERR_ACCESS_DENIED);
            case WBEM_E_INVALID_CLASS:
                PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL1,
                    "WMICollector::getClassEnum() - createClassEnum() returns "
                    "INVALID_CLASS- throw CIM_INVALID_CLASS error");
                throw CIMException(CIM_ERR_INVALID_CLASS);
            default:
                PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL1,
                    "WMICollector::getClassEnum() - createClassEnum() returns "
                    "default - throw getClassEnum general error");
                throw CIMException(CIM_ERR_FAILED, "[getClassEnum] general");
        }
    }

    PEG_METHOD_EXIT();

    return SUCCEEDED(hr);
}


/////////////////////////////////////////////////////////////////////////////
// WMICollector::getQueryResult - fetch a point to the enumeration of
//        instances returned from an ExecQuery request
//
/////////////////////////////////////////////////////////////////////////////
bool WMICollector::getQueryResult(
    IEnumWbemClassObject **ppInstances,
    const String &query,
    const String &queryLanguage)
{
    HRESULT hr;

    CComPtr<IWbemServices> pServices;
    CComPtr<IEnumWbemClassObject> p_inst;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER, "WMICollector::getQueryResult()");

    bool bConnected = Connect(&pServices);

    if (!bConnected)
    {
        PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMICollector::getQueryResult,() - bConnected is false - "
                "throw Connect failed error");

        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_ACCESS_DENIED, "Connect failed.");
    }

    //if QueryLanguage is not WQL, throws an exception error informing
    //that the query language is not supported
    if (!String::equalNoCase("WQL", queryLanguage))
    {
        throw CIMException(CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED);
    }

    CComBSTR bsQuery = query.getCString();
    CComBSTR bsQueryLanguage = queryLanguage.getCString();

    // retrieve query result
    hr = pServices->ExecQuery(
        bsQueryLanguage,
        bsQuery,
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_ENSURE_LOCATABLE,
        NULL,
        &p_inst);

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
        "WMICollector::getQueryResult(), return from ExecQuery "
            "- hr value is %x", hr));

    if (SUCCEEDED(hr))
    {
        p_inst.CopyTo(ppInstances);

        // set security attributes on *ppInstances
        bool bSecurity = setProxySecurity(*ppInstances);
    }
    else
    {
        *ppInstances = NULL;

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMICollector::getQueryResult() - hr value is %x", hr));

        switch(hr)
        {
            case WBEM_E_INVALID_QUERY:
                throw CIMException(CIM_ERR_INVALID_QUERY);
            case WBEM_E_INVALID_QUERY_TYPE:
                throw CIMException(CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED);
            default:
                throw CIMException(CIM_ERR_FAILED, "[getQueryResult] general");
        }
    }

    PEG_METHOD_EXIT();

    return (SUCCEEDED(hr));
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::getObject - fetch a pointer to the object
//        represented by sObjectName
//        Can be either a class or an instance
//
/////////////////////////////////////////////////////////////////////////////
bool WMICollector::getObject(
    IWbemClassObject **ppObject,
    const String & sObjectName)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMICollector::getObject()");

    HRESULT hr;
    CComPtr<IWbemServices>    pServices;
    CComPtr<IWbemClassObject> p_obj;

    bool bConnected = Connect(&pServices);

    if (!bConnected)
    {
        PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMICollector::getObject,() - bConnected is false - "
                "throw Connect failed error");

        throw CIMException(CIM_ERR_ACCESS_DENIED);
    }

    CComBSTR bsObjectName = sObjectName.getCString();
    LONG lFlags =
        WBEM_FLAG_USE_AMENDED_QUALIFIERS | WBEM_FLAG_RETURN_WBEM_COMPLETE;

    // retrieve class object
    hr = pServices->GetObject(
        bsObjectName,
        lFlags,
        NULL,
        &p_obj,
        NULL);

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
        "WMICollector::getObject() - GetObject result is %x", hr));

    if (SUCCEEDED(hr))
    {
        p_obj.CopyTo(ppObject);

        // set security attributes on result
        bool bSecurity = setProxySecurity(*ppObject);
    }
    else
    {
        *ppObject = NULL;

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
            "WMICollector::getObject() - GetObject result is %x", hr));

        //generate error
        switch(hr)
        {
            case WBEM_E_INVALID_CLASS:
                throw CIMException(CIM_ERR_INVALID_CLASS);
            case WBEM_E_NOT_FOUND:
                throw CIMException(CIM_ERR_NOT_FOUND);
            default:
                throw CIMException(CIM_ERR_FAILED, "[getObject] general");
        }
    }

    PEG_METHOD_EXIT();
    return SUCCEEDED(hr);
}


/////////////////////////////////////////////////////////////////////////////
// getProperties - retrieves selected
//        properties for a CIMClass or
//        CIMInstance object
//
/////////////////////////////////////////////////////////////////////////////
template<class CONTAINER>
void getProperties(
    IWbemClassObject *pClass,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    CONTAINER & container)
{
    HRESULT hr = S_OK;
    String sMessage;

    CComBSTR    bsName;            // of the property
    CComVariant vValue;            // of the property
    long        lFlavor;        // of the property
    CIMTYPE     type;
    CIMProperty property;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER, "getProperties()");

    Uint32 size = propertyList.size();

    for (Uint32 i = 0; ((i < size) && SUCCEEDED(hr)); i++)
    {
        String sPropName = propertyList[i].getString();

        bsName.Empty();
        vValue.Clear();
        sMessage = "Get()";

        bsName = sPropName.getCString();

        // fetch the property
        hr = pClass->Get(
            bsName,
            0,
            &vValue,
            &type,
            &lFlavor);

        // process each property
        if (SUCCEEDED(hr))
        {
            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL4,
                "getProperties() - CIMTYPE[%x] - WMITYPE[%x]",
                type,
                vValue.vt));

            bool bPropagated =
                (lFlavor & WBEM_FLAVOR_ORIGIN_PROPAGATED) ? true : false;

            try
            {
                property = WMICollector::getProperty(
                    pClass, bsName, vValue,
                    type, includeClassOrigin,
                    includeQualifiers, bPropagated);
            }
            catch (TypeMismatchException& e)
            {
                // ATTN:
                // unsupported for now - do some tracing...
                String sClass = WMICollector::getClassName(pClass);
                PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL2,
                    "getProperties() - Ignoring invalid type for %s in %s."
                    "  %s, unsupported WMI CIM type is %x",
                    sPropName, sClass, e.getMessage(), type));

                continue;
            }
            vValue.Clear();

            if (bPropagated && localOnly)
            {
                PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL4,
                    "getProperties() - Property %s was defined in a "
                    "superclass", (LPCTSTR)sPropName.getCString()));
            }
            else
            {
                // try to add it
                try
                {
                    container.addProperty(property);
                }
                catch( AlreadyExistsException& e )
                {
                    // ignore this
                    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL2,
                        "getProperties() - Property %s is already defined. %s",
                        sPropName, e.getMessage()));
                }
                catch (Exception& e)
                {
                    // ignore this
                    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL2,
                        "getAllProperties() - Ignoring AddedReferenceToClass."
                        "  %s", e.getMessage()));

                }
                catch (...)
                {
                    throw CIMException(CIM_ERR_FAILED,
                        "[getProperties] general");
                }
            }

        }
        else if (WBEM_E_NOT_FOUND == hr)
        {    // we are supposed to keep going...
            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
                "getProperties() - %s property not found",
                (LPCTSTR)sPropName.getCString()));

            hr = S_OK;
        }
    }

    if (FAILED(hr))
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "getProperties() - failed, return result is %x", hr));

        throw CIMException(CIM_ERR_FAILED, "[getProperties] general 2");
    }

    PEG_METHOD_EXIT();
}

/////////////////////////////////////////////////////////////////////////////
// getAllProperties - retrieves all the
//        properties for a CIMClass or
//        CIMInstance object
//
/////////////////////////////////////////////////////////////////////////////
template<class CONTAINER>
bool getAllProperties(
    IWbemClassObject *pClass,
    long lFlags,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    CONTAINER & container)
{
    HRESULT hr;
    String sMessage;

    CComBSTR bsName;     // of the property
    CComVariant vValue;  // of the property
    long lFlavor;        // of the property
    CIMTYPE type;
    CIMProperty property;

    bool bFound = false;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"getAllProperties()");

    hr = pClass->BeginEnumeration(lFlags);
    sMessage = "BeginEnumeration()";

    if (SUCCEEDED(hr))
    {
        bsName.Empty();
        vValue.Clear();
        sMessage = "Next()";

        hr = pClass->Next(0, &bsName, &vValue, &type, &lFlavor);
    }

    // process each property
    while (SUCCEEDED(hr))
    {
        if (WBEM_S_NO_MORE_DATA == hr)
        {
            break;
        }

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
            "getAllProperties() - CIMTYPE[%x] - WMITYPE[%x]",
            type,
            vValue.vt));

        bFound = true;

        CMyString sPropName; sPropName = bsName;

        bool bPropagated =
            (lFlavor & WBEM_FLAVOR_ORIGIN_PROPAGATED) ? true : false;

        try
        {
            property = WMICollector::getProperty(
                pClass, bsName, vValue,
                type, includeClassOrigin,
                includeQualifiers, bPropagated);
        }
        catch( TypeMismatchException & e )
        {
            // ATTN:
            // unsupported for now - do some tracing...
            String sClass = WMICollector::getClassName(pClass);
            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL2,
                "getAllProperties() - Ignoring invalid type for %s in %s."
                    "  %s, unsupported WMI CIM type is %x",
                (LPCTSTR)sPropName, sClass, e.getMessage(), type));

            bsName.Empty();
            vValue.Clear();

            sMessage = "Next()";
            hr = pClass->Next(0, &bsName, &vValue, &type, &lFlavor);

            continue;
        }

        try
        {
            container.addProperty(property);
        }
        catch (AlreadyExistsException& e)
        {
            // ignore this
            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL2,
                "getAllProperties() - Property %s is already defined",
                (LPCTSTR)sPropName));
            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL2,
                "getAllProperties() - %s", e.getMessage()));

        }
        catch (Exception& e)
        {
            // ignore this
            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL2,
                "getAllProperties() - Ignoring AddedReferenceToClass.  %s",
                e.getMessage()));

        }
        catch (...)
        {
            throw CIMException(CIM_ERR_FAILED, "[getAllProperties] general 1");
        }

        bsName.Empty();
        vValue.Clear();

        sMessage = "Next()";
        hr = pClass->Next(0, &bsName, &vValue, &type, &lFlavor);
    }
    pClass->EndEnumeration();
    vValue.Clear();

    if (FAILED(hr))
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "getAllProperties() - %s result is %x",
            sMessage.getCString(), hr));
        throw CIMException(CIM_ERR_FAILED, "[getAllProperties] general 2");
    }

    PEG_METHOD_EXIT();

    return bFound;
}

/////////////////////////////////////////////////////////////////////////////
// getAllMethods - retrieves all the
//        methods for a CIMClass or
//        CIMInstance object
//
/////////////////////////////////////////////////////////////////////////////
bool getAllMethods(
    IWbemClassObject *pClass,
    long lFlags,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    CIMClass & container)
{
    HRESULT hr;
    String sMessage;

    CComBSTR                  bsName;         // of the method
    CComPtr<IWbemClassObject> inParameters;   // of the method
    CComPtr<IWbemClassObject> outParameters;  // of the method
    CIMMethod                 method;

    bool bFound = false;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"getAllMethods()");

    hr = pClass->BeginMethodEnumeration(lFlags);
    sMessage = "BeginMethodEnumeration()";

    if (WBEM_E_INVALID_PARAMETER == hr)
    {
        // Windows 2000 does not accept any flags for BeginMethodEnumeration()
        // (e.g., WBEM_FLAG_LOCAL_ONLY, which is used when localOnly==true)
        // so try again with no flags (assuming Windows 2000 here):
        hr = pClass->BeginMethodEnumeration(0L);
    }

    if (SUCCEEDED(hr))
    {
        bsName.Empty();
        sMessage = "NextMethod()";

        hr = pClass->NextMethod(0, &bsName, &inParameters, &outParameters);
    }

    // process each method
    while (SUCCEEDED(hr))
    {
        if (WBEM_S_NO_MORE_DATA == hr)
        {
            break;
        }

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
                "getAllMethods() - name [%S]",
                bsName));

        bFound = true;

        CMyString sMethodName; sMethodName = bsName;

        // TODO: investigate propigation in terms of Methods.
        // Is this applicable?  Without a flavor,
        // where would this come from?
        //(lFlavor & WBEM_FLAVOR_ORIGIN_PROPAGATED) ? true : false;
        bool bPropagated = false;

        method = WMICollector::getMethod(
            pClass,
            bsName,
            inParameters,
            outParameters,
            includeClassOrigin,
            includeQualifiers,
            bPropagated);

        try
        {
            container.addMethod(method);
        }
        catch (AlreadyExistsException& e)
        {
            // ignore this
            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL2,
                "getAllMethods() - Method %s is already defined",
                (LPCTSTR)sMethodName));
            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL2, "getAllMethods() - %s",
                e.getMessage()));

        }
        catch (Exception& e)
        {
            // ignore this
            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL2,
                "getAllMethods() - Ignoring AddedReferenceToClass.  %s",
                e.getMessage()));

        }
        catch (...)
        {
            throw CIMException(CIM_ERR_FAILED, "[getAllProperties] general 1");
        }

        bsName.Empty();

        if (inParameters)
            inParameters.Release();

        if (outParameters)
            outParameters.Release();

        sMessage = "NextMethod()";
        hr =  pClass->NextMethod(0, &bsName, &inParameters, &outParameters);
    }
    pClass->EndMethodEnumeration();

    if (inParameters)
        inParameters.Release();

    if (outParameters)
        outParameters.Release();

    if (FAILED(hr))
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "getAllMethods() - %s result is %x", sMessage.getCString(), hr));
        throw CIMException(CIM_ERR_FAILED, "[getAllMethods] general 2");
    }

    PEG_METHOD_EXIT();
    return bFound;
}

/////////////////////////////////////////////////////////////////////////////
// getClassQualifiers - retrieves the
//        class qualifier definitions for a
//        class or instance and adds
//        them to the CIMClass or CIMInstance
//
/////////////////////////////////////////////////////////////////////////////
template<class CONTAINER>
void getClassQualifiers(
    IWbemClassObject *pClass,
    CONTAINER & container)
{
    HRESULT hr;
    String sMessage;

    CComPtr<IWbemQualifierSet> pQualifiers;

    CComBSTR bsName = NULL;  // of the qualifier
    CComVariant vValue;      // of the qualifier
    long lFlavor;            // of the qualifier
    bool bPropagated;        // true if propated from a superclass
    CIMQualifier qualifier;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"getClassQualifiers()");

    // get the qualifiers enumerator
    hr = pClass->GetQualifierSet(&pQualifiers);

    sMessage = "GetQualifier()";

    if (SUCCEEDED(hr))
    {
        hr = pQualifiers->BeginEnumeration(0);
        sMessage = "BeginEnumeration()";
    }

    if (SUCCEEDED(hr))
    {
        bsName.Empty();
        vValue.Clear();
        sMessage = "Next()";

        hr = pQualifiers->Next(0, &bsName, &vValue, &lFlavor);
    }

    // process each qualifier
    while (SUCCEEDED(hr))
    {
        if (WBEM_S_NO_MORE_DATA == hr)
        {
            break;
        }

        bPropagated = (lFlavor & WBEM_FLAVOR_ORIGIN_PROPAGATED) ? true : false;

        qualifier = CIMQualifier(
            WMIString(bsName), WMIValue(vValue),
            WMIFlavor(lFlavor), bPropagated);

        bsName.Empty();
        vValue.Clear();

        try
        {
            container.addQualifier(qualifier);
        }
        catch (...)
        {
            throw CIMException(CIM_ERR_FAILED,
                "[getClassQualifiers] general 1");
        }

        bsName.Empty();
        vValue.Clear();

        hr = pQualifiers->Next(0, &bsName, &vValue, &lFlavor);
    }

    pQualifiers->EndEnumeration();

    bsName.Empty();
    vValue.Clear();

    if (FAILED(hr))
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "getClassQualifiers() - %s result is %x",
            sMessage.getCString(), hr));

        throw CIMException(CIM_ERR_FAILED);
    }

    PEG_METHOD_EXIT();
}

////////////////////////////////////////////////////////////////////////////
// WMICollector::getCIMObject - set up a getCIMObject structure
//
/////////////////////////////////////////////////////////////////////////////
bool WMICollector::getCIMObject(
    IWbemClassObject *pObject,
    CIMObject & cimObj,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    Boolean getKeyProperties)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMICollector::getCIMObject()");

    // first get the qualifiers if wanted
    if (includeQualifiers)
    {
        getClassQualifiers(pObject, cimObj);
    }

    // then the properties
    getObjectProperties(
        pObject, cimObj, localOnly,
        includeQualifiers, includeClassOrigin,
        propertyList, getKeyProperties);

    PEG_METHOD_EXIT();
    return true;
}

////////////////////////////////////////////////////////////////////////////
// WMICollector::getCIMInstance - set up a CIMInstance structure
//
/////////////////////////////////////////////////////////////////////////////
bool WMICollector::getCIMInstance(
    IWbemClassObject *pObject,
    CIMInstance & cimInst,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    Boolean getKeyProperties)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMICollector::getCIMInstance()");

    CIMObject cimObj = cimInst;

    getCIMObject(pObject, cimObj, localOnly, includeQualifiers,
                    includeClassOrigin, propertyList, getKeyProperties);

    CIMInstance newInstance(cimObj);
    cimInst = newInstance;

    PEG_METHOD_EXIT();
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::getCIMClass - set up a
//        CIMClass structure
//
/////////////////////////////////////////////////////////////////////////////
bool WMICollector::getCIMClass(
    IWbemClassObject *pObject,
    CIMClass & cimClass,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMICollector::getCIMClass()");

    CIMObject cimObj = cimClass;

    getCIMObject(pObject, cimObj, localOnly, includeQualifiers,
                    includeClassOrigin, propertyList);

    CIMClass newClass(cimObj);
    cimClass = newClass;

    // Get methods for classes only
    getClassMethods(
        pObject, cimClass, localOnly, includeQualifiers, includeClassOrigin);

    PEG_METHOD_EXIT();
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::getClassName - return the value of the __CLASS
//        property of pObject
//
/////////////////////////////////////////////////////////////////////////////
String WMICollector::getClassName(IWbemClassObject *pObject)
{
    CComBSTR bsClass = "__CLASS";

    return getStringProperty(pObject, bsClass);
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::getRelativePath - return the value of the __RELPATH
//        property of pClass
//
/////////////////////////////////////////////////////////////////////////////
String WMICollector::getRelativePath(IWbemClassObject *pObject)
{
    CComBSTR bsRelPath = "__RELPATH";

    return getStringProperty(pObject, bsRelPath);
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::getSuperClass - return the value of the __SUPERCLASS
//        property of pClass
//
/////////////////////////////////////////////////////////////////////////////
String WMICollector::getSuperClass(IWbemClassObject *pClass)
{
    CComBSTR bsSuperClass = "__SUPERCLASS";

    return getStringProperty(pClass, bsSuperClass);
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::isInstance - returns true if pObject represents a
//        WMI instance - false if it is a class
//
/////////////////////////////////////////////////////////////////////////////
bool WMICollector::isInstance(IWbemClassObject *pObject)
{
    String sClass = getClassName(pObject);
    String sRelPath = getRelativePath(pObject);

    return (!String::equal(sClass, sRelPath));
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::getStringProperty - helper function to retrieve specific
//        string properties from a class or instance object
//
/////////////////////////////////////////////////////////////////////////////
String WMICollector::getStringProperty(
    IWbemClassObject *pObject,
    const CComBSTR &bsPropertyName)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER, "WMICollector::getStringProperty()");

    HRESULT hr;
    CComVariant vValue;
    CComBSTR bs;
    CMyString s = "";
    CMyString name; name = bsPropertyName;

    hr = pObject->Get(bsPropertyName, 0, &vValue, NULL, NULL);
    if (SUCCEEDED(hr))
    {
        if (VT_NULL != vValue.vt)
        {    // there is a value... if not we will return an empty string...
            bs = vValue.bstrVal;

            if (0 != bs.Length())
            {
                s = bs;
            }
        }

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
            "WMICollector::getStringProperty() - Value for %s is %s",
            (LPCTSTR)name, (LPCTSTR)s));
    }
    else
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMICollector::getStringProperty() - get result for %s is %x",
            (LPCTSTR)name, hr));
    }

    PEG_METHOD_EXIT();

    if (VT_NULL == vValue.vt)
    {
        vValue.Clear();
        return String::EMPTY;
    }
    else
    {
        vValue.Clear();
        return String((LPCTSTR)s);
    }
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::getObjectProperties - retrieves the
//        object property definitions and values, if any,
//        and adds them to the CIMObject instance
//
/////////////////////////////////////////////////////////////////////////////
bool WMICollector::getObjectProperties(
    IWbemClassObject *pObject,
    CIMObject & cimObj,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    Boolean bGetKeyProperties)
{
     PEG_METHOD_ENTER(TRC_WMIPROVIDER, "WMICollector::getObjectProperties()");

    long lFlags = (localOnly) ? WBEM_FLAG_LOCAL_ONLY :
        WBEM_FLAG_NONSYSTEM_ONLY;

    if (propertyList.isNull())
    {    // we want all the properties...
        getAllProperties(
            pObject, lFlags,
            includeQualifiers, includeClassOrigin,
            cimObj);
    }

    else if (0 != propertyList.size())
    {    // just get the ones requested
        getProperties(
            pObject, localOnly, includeQualifiers,
            includeClassOrigin, propertyList, cimObj);
    }

    // else we have an empty list and don't want any

    // if being called from getInstance, need to be sure that
    //    the key properties are retrieved...
    if (bGetKeyProperties)
    {
        getAllProperties(
            pObject, WBEM_FLAG_KEYS_ONLY,
            includeQualifiers, includeClassOrigin,
            cimObj);
    }

    PEG_METHOD_EXIT();
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::getProperty
//        create a CIMProperty object from
//        WMI data
//
/////////////////////////////////////////////////////////////////////////////
CIMProperty WMICollector::getProperty(
    IWbemClassObject *pClass,
    const CComBSTR &bsName,
    const CComVariant &vValue,
    CIMTYPE type,
    Boolean includeClassOrigin,
    Boolean includeQualifiers,
    Boolean bPropagated)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER, "WMICollector::getProperty()");

    HRESULT hr = S_OK;
    String sMessage;
    CIMProperty property;
    CMyString sPropName; sPropName = bsName;
    long iSize = sPropName.GetLength();

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
        "WMICollector::getProperty() - Property Name is %s, type is %x, "
        "CIMTYPE is %x", (LPCTSTR)sPropName, vValue.vt, type));

    // put this in to check XP - @bbp
    if (VT_BSTR == vValue.vt)
    {
        CComBSTR bs;
        bs.Empty();
        bs.Append(vValue.bstrVal);
        sPropName = bs;
        iSize = sPropName.GetLength();
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
            "WMICollector::getProperty() - Property Value is %s, size is %x",
            (LPCTSTR)sPropName, iSize));
    }

    CComPtr<IWbemQualifierSet> pQualifiers;

    // get the qualifier set for the properties
    sMessage = "GetPropertyQualifierSet()";
    hr = pClass->GetPropertyQualifierSet(bsName, &pQualifiers);

    if (SUCCEEDED(hr))
    {
        property = WMIProperty(bsName, vValue, type,
            pQualifiers, includeQualifiers);

        // set the property qualifier
        property.setPropagated(bPropagated);
    }

    // check if requested ClassOrigin qualifier
    // ATTN:
    // This is a bug in Pegasus.  It will put it
    //    the current class for GetClass if it isn't
    //    there.
    if ((includeClassOrigin /*|| bPropagated*/) && SUCCEEDED(hr))
    {
        CComBSTR        bsClassOrigin;
        sMessage = "GetPropertyOrigin()";

        hr = pClass->GetPropertyOrigin(bsName, & bsClassOrigin);

        if (SUCCEEDED(hr))
        {
            property.setClassOrigin(WMIString(bsClassOrigin));
        }
    }

    if (pQualifiers)
        pQualifiers.Release();

    if (FAILED(hr))
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMICollector::getProperty() - %s result is %x", sMessage, hr));

        throw CIMException(CIM_ERR_FAILED);
    }

    PEG_METHOD_EXIT();
    return property;
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::getClassMethods - retrieves the
//        class method definitions and parameters, if any,
//        and adds them to the CIMClass instance
//
/////////////////////////////////////////////////////////////////////////////
bool WMICollector::getClassMethods(
    IWbemClassObject *pObject,
    CIMClass & cimClass,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin)
{
     PEG_METHOD_ENTER(TRC_WMIPROVIDER, "WMICollector::getClasslMethods()");

    long lFlags = (localOnly) ? WBEM_FLAG_LOCAL_ONLY : 0L;

    // we want all the methods...
    getAllMethods(pObject, lFlags, includeQualifiers,
        includeClassOrigin, cimClass);

    PEG_METHOD_EXIT();
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::getMethod
//        create a CIMMethod object from
//        WMI data
//
/////////////////////////////////////////////////////////////////////////////
CIMMethod WMICollector::getMethod(
    IWbemClassObject *pClass,
    const CComBSTR &bsName,
    const CComPtr<IWbemClassObject> &inParameters,
    const CComPtr<IWbemClassObject> &outParameters,
    Boolean includeClassOrigin,
    Boolean includeQualifiers,
    Boolean bPropagated)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER, "WMICollector::getMethod()");

    HRESULT hr = S_OK;
    String sMessage;
    CIMMethod method;
    CMyString sMethodName; sMethodName = bsName;
    long iSize = sMethodName.GetLength();

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
        "WMICollector::getMethod() - Method Name is %s",
        (LPCTSTR)sMethodName));


    CComPtr<IWbemQualifierSet> pQualifiers;

    // get the qualifier set for the method
    sMessage = "GetMethodQualifierSet()";
    hr = pClass->GetMethodQualifierSet(bsName, &pQualifiers);

    if (SUCCEEDED(hr))
    {
        method = WMIMethod(bsName, inParameters, outParameters,
            pQualifiers, includeQualifiers);

        // set the method qualifier
        method.setPropagated(bPropagated);
    }

    // check if requested ClassOrigin qualifier
    // ATTN:
    // This is a bug in Pegasus.  It will put it
    //    the current class for GetClass if it isn't
    //    there.
    if ((includeClassOrigin /*|| bPropagated*/) && SUCCEEDED(hr))
    {
        CComBSTR        bsClassOrigin;
        sMessage = "GetMethodOrigin()";

        hr = pClass->GetMethodOrigin(bsName, & bsClassOrigin);

        if (SUCCEEDED(hr))
        {
            method.setClassOrigin(WMIString(bsClassOrigin));
        }
    }

    if (pQualifiers)
        pQualifiers.Release();

    if (FAILED(hr))
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMICollector::getMethod() - %s result is %x", sMessage, hr));

        throw CIMException(CIM_ERR_FAILED);
    }

    PEG_METHOD_EXIT();
    return method;
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::setNamespace
//
//
/////////////////////////////////////////////////////////////////////////////
void WMICollector::setNamespace(const String & sNamespace)
{
    String s;

    String sLower = sNamespace;
    sLower.toLower();
    Uint32 pos = sLower.find("root");

    m_bIsLocalNamespace = (0 == pos);

    if (m_bIsLocalNamespace)
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL4,
            "WMICollector::setNamespace() - Namespace %s is local",
            sNamespace.getCString()));
        s = sNamespace;
    }
    else
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL4,
            "WMICollector::setNamespace() - Namespace %s is remote",
            sNamespace.getCString()));

        // by Jair
        // adjust the namespace to accept DNS fully qualified names
        // and IP addresses.
        String str = sNamespace;
        for (Uint32 i = 0; i < pos - 1; i++)
        {
            if (str[i] == '/')
            {
                // have a '.' encoded as '/' -- convert back to '.':
                str[i] = '.';
            }
            else if (str[i] == '_')
            {
                if (str[i + 1] == '_' &&
                    iswxdigit(str[i + 2]) &&
                    iswxdigit(str[i + 3]))
                {
                    // have a non-alpha char (punctuation, etc.) encoded as
                    // "__XX", where XX is the hex representation of the char
                    String hexStr = str.subString(i+2, 2);
                    Char16 punctChar = (Char16)wcstol(
                        (const wchar_t *)hexStr.getChar16Data(), NULL, 16);
                    // replace the first underscore with the non-alpha char:
                    str[i] = punctChar;
                    // remove the second underscore + 2 hex chars:
                    str.remove(i+1, 3);
                    pos -= 3;
                }
                else if (iswdigit(str[i + 1]))
                {
                    if ((i == 0) || (str[i - 1] == '.'))
                    {
                        // have a leading digit pre-pended with an underscore
                        // remove the underscore:
                        str.remove(i, 1);
                        pos--;
                    }
                }
            }
        }

        //prepend "//" to namespace
        s = "//";
        s.append(str);
    }

    m_bsNamespace = s.getCString();
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::setUserName
//
//
/////////////////////////////////////////////////////////////////////////////
void WMICollector::setUserName(const String & sUserName)
{
    String sUser = sUserName;
    String sDomain = ".";

    Uint32 pos = sUser.find("/");

    if (PEG_NOT_FOUND == pos)
        pos = sUser.find("\\");

    if (pos != PEG_NOT_FOUND)
    {
        sDomain = sUser.subString(0, pos);
        sUser.remove(0, pos + 1);
    }

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL4,
        "WMICollector::setUserName() - UserName [%s], Domain [%s]",
        sUser.getCString(), sDomain.getCString()));

    m_bsUserName = sUser.getCString();
    m_bsDomain = sDomain.getCString();
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::setPassword
//
//
// ///////////////////////////////////////////////////////////////////////////
void WMICollector::setPassword(const String & sPassword)
{
    m_bsPassword = sPassword.getCString();
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::setProxySecurity
//
//
/////////////////////////////////////////////////////////////////////////////
bool WMICollector::setProxySecurity(IUnknown * pProxy)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMICollector::setProxySecurity()");

    HRESULT hr;

    if (m_bIsLocalNamespace)
    {
        PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL4,
            "WMICollector::setProxySecurity() - m_bIsLocalNamespace is true");

        hr = CoSetProxyBlanket(
            pProxy,
            RPC_C_AUTHN_WINNT,    // NTLM authentication service
            RPC_C_AUTHZ_NONE,     // default authorization service...
            NULL,                 // no mutual authentication
            RPC_C_AUTHN_LEVEL_PKT,      // authentication level
            RPC_C_IMP_LEVEL_IMPERSONATE, // impersonation level
            NULL,
            EOAC_DYNAMIC_CLOAKING);     // enable dynamic cloaking, so
                                        // impersonation token is propagated
                                        // to WMI
    }
    else
    {
        PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL4,
            "WMICollector::setProxySecurity() - m_bIsLocalNamespace is false");

        // set security attributes on pProxy
        COAUTHIDENTITY authident;

        memset((void *)&authident,0,sizeof(COAUTHIDENTITY));

        authident.UserLength = (ULONG)m_bsUserName.Length();
        authident.User = (USHORT *) m_bsUserName.m_str;
        authident.PasswordLength = (ULONG)m_bsPassword.Length();
        authident.Password = (USHORT *) m_bsPassword.m_str;
        authident.DomainLength = (ULONG)m_bsDomain.Length();
        authident.Domain = (USHORT *) m_bsDomain.m_str;
        authident.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

        hr = CoSetProxyBlanket(
                pProxy,
                RPC_C_AUTHN_WINNT,    // NTLM authentication service
                RPC_C_AUTHZ_NONE,     // default authorization service...
                NULL,                 // no mutual authentication
                RPC_C_AUTHN_LEVEL_PKT_PRIVACY,  // authentication level
                RPC_C_IMP_LEVEL_IMPERSONATE,    // impersonation level
                &authident,
                EOAC_NONE); // EOAC_DYNAMIC_CLOAKING seems to fail on remote
                            // WMI calls, or when COAUTHIDENTITY info is sent
                            // (as in this case), so going back to EOC_NONE for
                            // this case only.
    }

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
        "WMICollector::setProxySecurity() - return from "
        "CoSetProxyBlanket() is %x", hr));

    PEG_METHOD_EXIT();
    return SUCCEEDED(hr);
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::logonUser
//
//
/////////////////////////////////////////////////////////////////////////////
void WMICollector::logonUser()
{
    CMyString sUserName, sDomain, sPassword;

    sUserName = (LPWSTR)m_bsUserName.m_str;
    sDomain   = (LPWSTR)m_bsDomain.m_str;
    sPassword = (LPWSTR)m_bsPassword.m_str;

    LPTSTR pszUserName  = (LPTSTR)(LPCTSTR)sUserName;
    LPTSTR pszDomain    = (LPTSTR)(LPCTSTR)sDomain;
    LPTSTR pszPassword  = (LPTSTR)(LPCTSTR)sPassword;

    //Logon and impersonate the user
    if (!RevertToSelf())
    {
        DWORD error = GetLastError();

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
                "WMICollector::logonUser() - return from RevertToSelf() is %d",
                error));

        throw CIMException(CIM_ERR_ACCESS_DENIED, "RevertToSelf()");
    }

    HANDLE htok = 0;
    if (!LogonUser(
            pszUserName,
            pszDomain,
            pszPassword,
            LOGON32_LOGON_INTERACTIVE,
            LOGON32_PROVIDER_DEFAULT,
            &htok))
    {
        DWORD error = GetLastError();

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMICollector::logonUser() - return from LogonUser() is %d",
            error));

        Logger::put(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
            "Failed to login user \"$0/$1\". Invalid username or password.",
            pszDomain, pszUserName);

        throw CIMException(CIM_ERR_ACCESS_DENIED, "LogonUser()");
    }

    if (!ImpersonateLoggedOnUser(htok))
    {
        DWORD error = GetLastError();
        CloseHandle(htok);

        Logger::put(
            Logger::STANDARD_LOG, System::CIMSERVER, Logger::WARNING,
            "Failed to impersonate logged-in user \"$0/$1\".",
            pszDomain, pszUserName);

        throw CIMException(CIM_ERR_ACCESS_DENIED, "ImpersonateLoggedOnUser()");
    }

    CloseHandle(htok);
    m_bImpersonate = true;
}

/////////////////////////////////////////////////////////////////////////////
// WMICollector::revertToSelf
//
//
/////////////////////////////////////////////////////////////////////////////
void WMICollector::revertToSelf()
{
    if (!RevertToSelf())
    {
        throw CIMException(CIM_ERR_FAILED, "RevertToSelf()");
    }
}

PEGASUS_NAMESPACE_END
