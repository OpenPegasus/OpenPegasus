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
// Modified By: Paulo Sehn (paulo_sehn@hp.com)
//              Adriano Zanuz (adriano.zanuz@hp.com)
//              Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//              Terry Martin, Hewlett-Packard Company (terry.martin@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////
/// WMIClassProvider.cpp: implementation of the WMIClassProvider class.
//
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::
//
// ///////////////////////////////////////////////////////////////////////////
#include "Stdafx.h"
#include <vector>

#include "WMICollector.h"
#include "WMIBaseProvider.h"
#include "WMIClassProvider.h"

#include "WMIProperty.h"
#include "WMIString.h"
#include "WMIValue.h"
#include "WMIQualifier.h"
#include "WMIQualifierSet.h"
#include "WMIType.h"
#include "WMIException.h"
#include "WMIFlavor.h"

#include "MyString.h"

using namespace std;

PEGASUS_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WMIClassProvider::WMIClassProvider(void)
{
    _collector = NULL;
    m_bInitialized = false;
}

WMIClassProvider::~WMIClassProvider(void)
{
    cleanup();
}

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::getClass
//
/// virtual class CIMClass. From the operations class
// ///////////////////////////////////////////////////////////////////////////
CIMClass WMIClassProvider::getClass(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const String& className,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIClassProvider::getClass()");

    CComPtr<IWbemClassObject> pClass;

    setup(nameSpace,userName,password);

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
        "getClass - localOnly %x, includeQualifiers %x, includeClassOrigin %x",
        localOnly, includeQualifiers, includeClassOrigin));

    if (!m_bInitialized)
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMIClassProvider::getClass - m_bInitilized= %x, "
                "throw CIM_ERR_FAILED exception",
            m_bInitialized));

        throw CIMException(CIM_ERR_FAILED);
    }

    try
    {
        if (!(_collector->getObject(&pClass, className)))
        {
            if (pClass)
                pClass.Release();

            throw CIMException(CIM_ERR_NOT_FOUND);
        }
        else if (_collector->isInstance(pClass))
        {
            if (pClass)
                pClass.Release();

            throw CIMException(CIM_ERR_INVALID_PARAMETER);
        }
    }
    catch (CIMException &e)
    {
        if (pClass)
            pClass.Release();

        switch(e.getCode())
        {
            case CIM_ERR_INVALID_CLASS:
                throw CIMException(CIM_ERR_NOT_FOUND);
                break;
            default: throw;
        }
    }

    String superClass = _collector->getSuperClass(pClass);
    CIMName objName = className;
    CIMClass cimClass = CIMClass(objName);

    if (0 != superClass.size())
    {
        CIMName superclassName = superClass;
        cimClass.setSuperClassName(superclassName);
    }

    if (!(_collector->getCIMClass(pClass,
                cimClass,
                localOnly,
                includeQualifiers,
                includeClassOrigin,
                propertyList)))
    {
        throw CIMException(CIM_ERR_NOT_FOUND);
    }

    if (pClass)
        pClass.Release();

    PEG_METHOD_EXIT();

    return cimClass;
}

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::enumerateClasses
//
// ///////////////////////////////////////////////////////////////////////////
Array<CIMClass> WMIClassProvider::enumerateClasses(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const String& className,
        Boolean deepInheritance ,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIClassProvider::enumerateClasses()");

    HRESULT hr;
    long lCount = 0;
    DWORD dwReturned;
    CComPtr<IEnumWbemClassObject>    pClassEnum;
    CComPtr<IWbemClassObject>        pClass;
    Array<CIMClass>                    cimClasses;

    setup(nameSpace,userName,password);

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
        "enumerateClasses - deepInheritance %x, localOnly %x, "
        "includeQualifiers %x, includeClassOrigin %x",
        deepInheritance, localOnly, includeQualifiers, includeClassOrigin));

    if (!m_bInitialized)
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMIClassProvider::enumerateClasses - m_bInitilized= %x, "
                "throw CIM_ERR_FAILED exception",
            m_bInitialized));

        throw CIMException(CIM_ERR_FAILED);
    }

    if (!(_collector->getClassEnum(&pClassEnum, className, deepInheritance)))
    {
        if (pClassEnum)
            pClassEnum.Release();

        throw CIMException(CIM_ERR_FAILED);
    }

    // set proxy security on pClassEnum
    bool bSecurity = _collector->setProxySecurity(pClassEnum);

    //
    //TODO:  What happens if bSecurity is false
    //

    // get the classes and append them to the array
    hr = pClassEnum->Next(WBEM_INFINITE, 1, &pClass, &dwReturned);

    while (SUCCEEDED(hr) && (1 == dwReturned))
    {
        String className = _collector->getClassName(pClass);
        String superClass = _collector->getSuperClass(pClass);

        CIMName objName = className;

        CIMClass tempClass = CIMClass(objName);

        if (0 != superClass.size())
        {
            CIMName superclassName = superClass;
            tempClass.setSuperClassName(superclassName);
        }

        if (_collector->getCIMClass(pClass,
                    tempClass,
                    localOnly,
                    includeQualifiers,
                    includeClassOrigin))
        {
            lCount++;
            cimClasses.append(tempClass);
        }

        if (pClass)
            pClass.Release();

        hr = pClassEnum->Next(WBEM_INFINITE, 1, &pClass, &dwReturned);
    }

    if (pClassEnum)
        pClassEnum.Release();

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL4,
        "WMIClassProvider::enumerateClasses() - Class count is %d", lCount));

    if (lCount == 0)
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL2,
            "WMIClassProvider::enumerateClasses() - no classes found, "
            "hResult value is %x", hr));
    }

    PEG_METHOD_EXIT();

    return cimClasses;
}

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::enumerateClassNames
//
// ///////////////////////////////////////////////////////////////////////////
Array<CIMName> WMIClassProvider::enumerateClassNames(
        const String& nameSpace,
        const String& userName,
        const String& password,
        const String& className,
        Boolean deepInheritance)
{
    HRESULT hr;
    long lCount = 0;
    DWORD dwReturned;

    CComPtr<IEnumWbemClassObject>    pClassEnum;
    CComPtr<IWbemClassObject>        pClass;

    Array<CIMName> classNames;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,
        "WMIClassProvider::enumerateClassNames()");

    setup(nameSpace,userName,password);

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL4,
        "enumerateClassNames - deepInheritance %x", deepInheritance));

    if (!m_bInitialized)
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMIClassProvider::enumerateClassNames - m_bInitilized= %x, "
                "throw CIM_ERR_FAILED exception",
            m_bInitialized));

        throw CIMException(CIM_ERR_FAILED);
    }

    // retrieve class enumeration object
    if (!(_collector->getClassEnum(&pClassEnum, className, deepInheritance)))
    {
        if (pClassEnum)
            pClassEnum.Release();

        throw CIMException(CIM_ERR_FAILED);
    }

    // set proxy security on pClassEnum
    bool bSecurity = _collector->setProxySecurity(pClassEnum);

    // get the classe names and append them to the array
    hr = pClassEnum->Next(WBEM_INFINITE, 1, &pClass, &dwReturned);

    while (SUCCEEDED(hr) && (1 == dwReturned))
    {
        String className = _collector->getClassName(pClass);
        lCount++;
        CIMName cimclassname = className;
        classNames.append(cimclassname);

        if (pClass)
            pClass.Release();

        hr = pClassEnum->Next(WBEM_INFINITE, 1, &pClass, &dwReturned);
    }

    if (pClassEnum)
        pClassEnum.Release();

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
        "WMIClassProvider::enumerateClassNames() - "
        "Class count is %d", lCount));

    if (lCount == 0)
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMIClassProvider::enumerateClassNames() - no classes found, "
            "hResult value is %x", hr));
    }

    PEG_METHOD_EXIT();

    return classNames;
}

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::deleteClass
//
// ///////////////////////////////////////////////////////////////////////////
void WMIClassProvider::deleteClass(const String& nameSpace,
                                   const String& userName,
                                   const String& password,
                                   const String& className)
{
    HRESULT hr;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,
        "WMIClassProvider::deleteClass()");

    CComPtr<IWbemServices> pServices;

    //Connect to namespace
    setup(nameSpace,userName,password);

    bool bConnected = _collector->Connect(&pServices);

    if (!bConnected)
    {
        if (pServices)
            pServices.Release();

        throw CIMException(CIM_ERR_ACCESS_DENIED);
    }

    //Convert the parameters to make the WMI call
    CComBSTR bsClassName = className.getCString();

    LONG lFlags = 0L;

    //Perform the WMI operation
    hr = pServices->DeleteClass(bsClassName,
                                lFlags,
                                NULL,
                                NULL);
    if (pServices)
        pServices.Release();

    //Handle the WMI operation result
    if (FAILED(hr))
    {
        CMyString msg;
        msg.Format("Failed to delete class [%s]. Error: 0x%X",
            255, static_cast<char const *>(className.getCString()), hr);

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
            "WMIClassProvider::deleteClass() - %s", (LPCTSTR)msg));

        switch (hr)
        {
            case WBEM_E_ACCESS_DENIED:
                throw CIMException(CIM_ERR_ACCESS_DENIED);
                break;
            case WBEM_E_FAILED:
                throw CIMException(CIM_ERR_FAILED);
                break;
            case WBEM_E_INVALID_PARAMETER:
                throw CIMException(CIM_ERR_FAILED, "WMI Invalid Parameter");
                break;
            case WBEM_E_INVALID_CLASS:
                throw CIMException(CIM_ERR_NOT_FOUND);
                break;
            case WBEM_E_NOT_FOUND:
                throw CIMException(CIM_ERR_NOT_FOUND);
                break;
            case WBEM_E_CLASS_HAS_CHILDREN:
                throw CIMException(CIM_ERR_CLASS_HAS_CHILDREN);
                break;
            default:
                throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
        }
    }

    PEG_METHOD_EXIT();
}

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::createClass
//
// ///////////////////////////////////////////////////////////////////////////
void WMIClassProvider::createClass(const String& nameSpace,
                                   const String& userName,
                                   const String& password,
                                   const CIMClass& newClass,
                                   Boolean updateClass)
{

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,
        "WmiClassProvider::createClass()");

    setup(nameSpace, userName, password);

    if (!m_bInitialized)
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMIClassProvider::createClass - m_bInitilized= %x, "
            "throw CIM_ERR_FAILED exception",
            m_bInitialized));

        throw CIMException(CIM_ERR_FAILED);
    }

    // Check if the class does not exist and if if has a valid
    // superclass
    performInitialCheck(newClass, updateClass);
    bool hasSuperClass =
        (newClass.getSuperClassName().getString() != String::EMPTY);

    // gets the pointers
    CComPtr<IWbemServices> pServices;
    CComPtr<IWbemClassObject> pNewClass;

    if (!_collector->Connect(&pServices))
    {
        if (pServices)
            pServices.Release();

        throw CIMException (CIM_ERR_FAILED);
    }

    try
    {
        // starts the class creation by name and class qualifiers
        createClassNameAndClassQualifiers(
            newClass,
            pServices,
            &pNewClass,
            hasSuperClass);

        // create properties
        createProperties(
            newClass,
            pServices,
            pNewClass);

        // create methods
        createMethods(
            newClass,
            pServices,
            pNewClass);
    }
    catch (CIMException&)
    {
        if (pServices)
            pServices.Release();

        if (pNewClass)
            pNewClass.Release();

        throw;
    }

    // Store the new class into WMI
    LONG lFlags = 0L;

    //if updateClass is set, we are trying a modifyclass
    if (updateClass) lFlags = WBEM_FLAG_UPDATE_ONLY;

    HRESULT hr = pServices->PutClass(pNewClass, lFlags, NULL, NULL);

    if (pServices)
        pServices.Release();

    if (pNewClass)
        pNewClass.Release();

    if (FAILED(hr))
    {
        CMyString msg;
        msg.Format(
            "It is not possible to create the class [%s]. Error: 0x%X",
            255,
            static_cast<char const *>
                (newClass.getClassName().getString().getCString()),
            hr);

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
                          "WMIClassProvider::createClass() - %s",
                          (LPCTSTR)msg));

        switch(hr)
        {
            case E_ACCESSDENIED:
                throw CIMException(CIM_ERR_ACCESS_DENIED);
                break;
            case WBEM_E_ACCESS_DENIED:
                throw CIMException(CIM_ERR_ACCESS_DENIED);
                break;
            case WBEM_E_CLASS_HAS_CHILDREN:
                throw CIMException(CIM_ERR_CLASS_HAS_CHILDREN);
                break;
            case WBEM_E_CLASS_HAS_INSTANCES:
                throw CIMException(CIM_ERR_CLASS_HAS_INSTANCES);
                break;
            case WBEM_E_NOT_FOUND:
                throw CIMException(CIM_ERR_NOT_FOUND);
                break;
            case WBEM_E_INVALID_CLASS:
                throw CIMException(CIM_ERR_INVALID_PARAMETER);
                break;
            default:
                throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
        }
    }

    PEG_METHOD_EXIT();

    return;
}

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::modifyClass
//
// ///////////////////////////////////////////////////////////////////////////
void WMIClassProvider::modifyClass(const String& nameSpace,
                                   const String& userName,
                                   const String& password,
                                   const CIMClass& modifiedClass)
{

    PEG_METHOD_ENTER(TRC_WMIPROVIDER, "WMIClassProvider::modifyClass()");

    setup(nameSpace, userName, password);

    if (!m_bInitialized)
    {
        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMIClassProvider::modifyClass - m_bInitilized= %x, "
            "throw CIM_ERR_FAILED exception",
            m_bInitialized));

        throw CIMException(CIM_ERR_FAILED);
    }

    // check if class does exist
    if (!classAlreadyExists(modifiedClass.getClassName().getString()))
    {
        throw CIMException(CIM_ERR_NOT_FOUND);
    }

    // check if superclass does exist
    if (!classAlreadyExists(modifiedClass.getSuperClassName().getString()))
    {
        // superclass doesn't exist
        throw CIMException(CIM_ERR_INVALID_SUPERCLASS);
    }

    //
    // By Jair
    //
    // If the class already has a superclass and it isn't explicitly
    // defined in the <modifiedClass> definition, we must set it
    // before changing the control to the createClass method. This way
    // we don't change the Class definition, as specified in DSP0200.
    //
    CIMClass updatedClass = modifiedClass;

    if ((updatedClass.getSuperClassName().getString()).size() == 0)
    {
        // set the superclass

        CComPtr<IWbemClassObject> pClass;

        if (!(_collector->getObject(&pClass,
            updatedClass.getClassName().getString())))
        {
            if (pClass)
                pClass.Release();

            throw CIMException(CIM_ERR_NOT_FOUND);
        }

        String superClass = _collector->getSuperClass(pClass);

        if (0 != superClass.size())
        {
            CIMName superclassName = superClass;
            updatedClass.setSuperClassName(superclassName);
        }

        if (pClass)
            pClass.Release();
    }

    //update the class using createClass
    createClass(nameSpace,
                userName,
                password,
                updatedClass,
                true);

    PEG_METHOD_EXIT();

    return;
}

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::classAlreadyExists
//
// ///////////////////////////////////////////////////////////////////////////
Boolean WMIClassProvider::classAlreadyExists (const String& className)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,
        "WmiClassProvider::classAlreadyExists()");

    CComPtr<IWbemClassObject> pClass;

    try
    {
        if (_collector->getObject(&pClass, className))
        {
            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL2,
                "WMIClassProvider::classAlreadyExists() - the class %s "
                "already exists", className.getCString()));

            if (pClass)
                pClass.Release();

            return true;
        }
    }
    catch(CIMException &e)
    {
        if (pClass)
            pClass.Release();

        switch(e.getCode())
        {
            case CIM_ERR_NOT_FOUND: return false; break;
            default: throw;
        }
    }

    if (pClass)
        pClass.Release();

    PEG_METHOD_EXIT();

    return false;
}

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::performInitialCheck
//
// ///////////////////////////////////////////////////////////////////////////
void WMIClassProvider::performInitialCheck(const CIMClass& newClass,
                                           Boolean updateClass)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,
        "WmiClassProvider::performInitialCheck()");

    // check if class already exists, just for createClass calls
    if ((classAlreadyExists(newClass.getClassName().getString())) &&
        (!updateClass))
    {
        PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL2,
            "WMIClassProvider::performInitialCheck - class already exists,"
            " throw CIM_ERR_ALREADY_EXISTS exception");

        throw CIMException(CIM_ERR_ALREADY_EXISTS);
    }

    // check if newClass has a superclass
    if (newClass.getSuperClassName().getString() != String::EMPTY)
    {
        // verifies if the superclass exists
        if (!classAlreadyExists(newClass.getSuperClassName().getString()))
        {
            // superclass doesn't exist, trace and throw error
            String tmp = newClass.getSuperClassName().getString();

            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
                "WMIClassProvider::performInitialCheck() - the "
                "superclass %s wasn't yet registered", tmp.getCString()));

            throw CIMException(CIM_ERR_INVALID_SUPERCLASS);
        }
    }

    PEG_METHOD_EXIT();

    return;
}

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::createClassNameAndClassQualifiers
//
// ///////////////////////////////////////////////////////////////////////////
void WMIClassProvider::createClassNameAndClassQualifiers(
    const CIMClass& newClass,
    IWbemServices *pServices,
    IWbemClassObject **pNewClass,
    const bool hasSuperClass)
{
    HRESULT hr;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,
        "WmiClassProvider::createClassNameAndClassQualifiers()");

    // if the class has a superclass, we need to spwan a derived
    if (hasSuperClass)
    {
        // get the superclass name
        CComPtr<IWbemClassObject> pSuperClass;
        String tmp = newClass.getSuperClassName().getString();
        CComBSTR bs = tmp.getCString();

        hr = pServices->GetObject(
                bs,
                NULL,
                NULL,
                &pSuperClass,
                NULL);

        bs.Empty();

        if (FAILED(hr))
        {
            if (pSuperClass)
                pSuperClass.Release();

            CMyString msg;
            msg.Format("Failed to get a pointer to Superclass [%s]. Error: "
                "0x%X", 255, static_cast<char const *>(tmp.getCString()), hr);

            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
                "WMIClassProvider::createClassNameAndClassQualifiers() - %s",
                (LPCTSTR)msg));

            throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
        }

        //Creates the new class
        pSuperClass->SpawnDerivedClass(NULL, pNewClass);
        if (pSuperClass)
            pSuperClass.Release();
    }
    else
    {
        // we are creating a base class
        hr = pServices->GetObject(NULL,
                                  NULL,
                                  NULL,
                                  pNewClass,
                                  NULL);

        if (FAILED(hr))
        {
            CMyString msg;
            msg.Format("Failed to get a pointer to a new class. Error: 0x%X",
                hr);

            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
                "WMIClassProvider::createClassNameAndClassQualifiers() - %s",
                (LPCTSTR)msg));

            throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
        }
    }

    // create the class name
    CComVariant v;
    v = newClass.getClassName().getString().getCString();
    hr = (*pNewClass)->Put(L"__CLASS", 0, &v, 0);

    v.Clear();

    if (FAILED(hr))
    {
        CMyString msg;
        msg.Format(
            "Failed to add class name on class [%s]. Error: 0x%X",
            255,
            static_cast<char const *>
                (newClass.getClassName().getString().getCString()),
            hr);

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMIClassProvider::createClassNameAndClassQualifiers() - %s",
            (LPCTSTR)msg));

        if (*pNewClass)
            (*pNewClass)->Release();

        throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
    }

    // get a pointer to work with qualifiers
    CComPtr<IWbemQualifierSet> pNewClassQualifier;
    hr = (*pNewClass)->GetQualifierSet(&pNewClassQualifier);

    if (FAILED(hr))
    {
        CMyString msg;
        msg.Format(
            "Failed to get the Qualifier set pointer of class [%s]."
                " Error: 0x%X",
            255,
            static_cast<char const *>
                (newClass.getClassName().getString().getCString()),
            hr);

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WMIClassProvider::createClassNameAndClassQualifiers() - %s",
            (LPCTSTR)msg));

        if (*pNewClass)
            (*pNewClass)->Release();

        if (pNewClassQualifier)
            pNewClassQualifier.Release();

        throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
    }

    // check the class qualifiers and create them if they are valid
    // we are taking care of the class qualifiers and not
    // methods/properties qualifiers :D
    for (Uint32 i = 0; i < newClass.getQualifierCount(); i++)
    {
        try
        {
            WMIQualifier qualifier(newClass.getQualifier(i).clone());

            createQualifier(qualifier, pNewClassQualifier);
        }
        catch (CIMException&)
        {
            if (*pNewClass)
                (*pNewClass)->Release();

            if (pNewClassQualifier)
                pNewClassQualifier.Release();

            throw;
        }
    }

    if (pNewClassQualifier)
        pNewClassQualifier.Release();

    PEG_METHOD_EXIT();

    return;
}


/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::createProperties creates all properties including keys
//                                      add the qualifiers too
// ///////////////////////////////////////////////////////////////////////////
void WMIClassProvider::createProperties(const CIMClass& newClass,
                                        IWbemServices *pServices,
                                        IWbemClassObject *pNewClass)
{
    HRESULT hr;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER, "WmiClassProvider::createProperties()");

    // create the properties but don't create the keys again
    CIMProperty prop;

    for (Uint32 i = 0; i < newClass.getPropertyCount(); i++)
    {
        prop = newClass.getProperty(i).clone();

        // create the properties
        try
        {
            createProperty(prop, pNewClass);
        }
        catch (CIMException&)
        {
            throw;
        }

        // get a pointer to work with qualifiers
        CComPtr<IWbemQualifierSet> pQual;
        CComBSTR bs = prop.getName().getString().getCString();

        hr = pNewClass->GetPropertyQualifierSet(bs, &pQual);
        bs.Empty();

        if (FAILED(hr))
        {
            CMyString msg;
            msg.Format(
                "Failed get Qualifier set of [%s]. Error: 0x%X",
                255,
                static_cast<char const *>
                    (prop.getName().getString().getCString()),
                hr);

            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
                "WMIClassProvider::createProperties() - %s", (LPCTSTR)msg));

            if (pQual)
                pQual.Release();

            throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
        }

        // set the qualifiers to the property
        for (Uint32 j = 0; j < prop.getQualifierCount(); j++)
        {
            WMIQualifier qualifier(prop.getQualifier(j));
            try
            {
                createQualifier(qualifier, pQual);
            }
            catch (CIMException&)
            {
                if (pQual)
                    pQual.Release();

                throw;
            }
        }

        // set the CLASSORIGIN qualifier if it wasn't set yet
        String strClassorigin = prop.getClassOrigin().getString();

        if (strClassorigin.size() == 0)
        {
            strClassorigin = newClass.getClassName().getString();
        }

        WMIFlavor flavor(CIMFlavor::DEFAULTS);

        /*
        v.vt = VT_BSTR;
        v.bstrVal = strClassorigin.getCString();
        */
        CComVariant v;
        v = strClassorigin.getCString();

        hr = pQual->Put(L"CLASSORIGIN", &v, flavor.getAsWMIValue());
        v.Clear();

        if (pQual)
            pQual.Release();

        if (FAILED(hr))
        {
            CMyString msg;
            msg.Format(
                "Failed to add CLASSORIGIN qualifier in [%s]. Error: 0x%X",
                255,
                static_cast<char const *>
                    (prop.getName().getString().getCString()),
                hr);

            PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
                          "WMIClassProvider::createProperties () - %s",
                          (LPCTSTR)msg));

            throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
        }
    }

    PEG_METHOD_EXIT();

    return;
}

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::createMethods  create methods
//
/////////////////////////////////////////////////////////////////////////////
void WMIClassProvider::createMethods(const CIMClass& newClass,
                                     IWbemServices *pServices,
                                     IWbemClassObject *pNewClass)
{

    PEG_METHOD_ENTER(TRC_WMIPROVIDER, "WMIClassProvider::createMethods ()");

    // create all methods
    for (Uint32 i = 0; i < newClass.getMethodCount(); i++)
    {
        CIMConstMethod method;
        method = newClass.getMethod(i);

        try
        {
            createMethod(method, pServices, pNewClass);
        }
        catch (CIMException&)
        {
            throw;
        }
    }

    PEG_METHOD_EXIT();

    return;
}

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::createProperty   creates one property
//                                      doesn't create the qualifiers
/////////////////////////////////////////////////////////////////////////////
void WMIClassProvider::createProperty(const CIMProperty &keyProp,
                                      IWbemClassObject *pNewClass)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER, "WmiClassProvider::createProperty()");

    // create the property
    //Get the CIMTYPE of the parameter
    CIMTYPE type = CIMTypeToWMIType(keyProp.getType());

    //If the property is an array, add CIM_FLAG_ARRAY to CIMType
    if (keyProp.isArray())
    {
        type |= CIM_FLAG_ARRAY;
    }

    // add the property to the class
    CComBSTR bs = keyProp.getName().getString().getCString();
    HRESULT hr = pNewClass->Put(bs, NULL, NULL, type);

    bs.Empty();

    //handle the error, if failed
    if (FAILED(hr))
    {
        CMyString msg;
        msg.Format(
            "Failed to add property [%s]. Error: 0x%X",
            255,
            static_cast<char const *>
                (keyProp.getName().getString().getCString()),
            hr);

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
                      "WMIClassProvider::createProperty () - %s",
                      (LPCTSTR)msg));

        throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
    }

    PEG_METHOD_EXIT();

    return;
}

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::createQualifier  creates a qualifiers
//
/////////////////////////////////////////////////////////////////////////////
void WMIClassProvider::createQualifier (const WMIQualifier &qualifier,
                                        IWbemQualifierSet *pQual)
{
    HRESULT hr;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER, "WmiClassProvider::createQualifier()");

    String sName = qualifier.getName().getString();
    CComBSTR bs = sName.getCString();
    WMIValue value(qualifier.getValue());
    WMIFlavor flavor(qualifier.getFlavor());

    CComVariant v;
    value.getAsVariant(&v);

    // key is created using a special call to wmi
    //if (!stricmp("key", ))
    if (String::equalNoCase("key", sName))
    {
        hr = pQual->Put(bs, &v, NULL);
    }
    else
    {
        hr = pQual->Put(bs, &v, flavor.getAsWMIValue());
    }
    v.Clear();
    bs.Empty();

    if (FAILED(hr))
    {
        CMyString msg;
        msg.Format(
            "It is not possible to add the qualifier [%s] to "
                "the object! Error: 0x%X",
            255,
            static_cast<char const *>(sName.getCString()),
            hr);

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
            "WmiClassProvider::createQualifier() - %s", (LPCTSTR)msg));

        throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
    }

    PEG_METHOD_EXIT();

    return;
}

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::createMethod  create a method
//
/////////////////////////////////////////////////////////////////////////////
void WMIClassProvider::createMethod (CIMConstMethod &method,
                                     IWbemServices *pServices,
                                     IWbemClassObject *pNewClass)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER, "WmiClassProvider::createMethod()");

    // the parameters
    HRESULT hr = S_OK;
    CComPtr<IWbemClassObject> pinParameters;
    CComPtr<IWbemClassObject> poutParameters;

    // Get pointers to use for in & out params:
    hr = pServices->GetObject(L"__PARAMETERS",
                              NULL,
                              NULL,
                              &pinParameters,
                              NULL);
    if (FAILED(hr))
    {
        CMyString msg;
        msg.Format(
            "Failed to get a paramter pointer while "
                "creating method: %s! Error: 0x%X",
            255,            
            static_cast<char const *>
                (method.getName().getString().getCString()),
            hr);

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
                      "WMIClassProvider::createMethod() - %s", (LPCTSTR)msg));

        if (pinParameters)
            pinParameters.Release();
        if (poutParameters)
            poutParameters.Release();

        throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
    }

    // Get pointers to use for in & out params:
    hr = pServices->GetObject(L"__PARAMETERS",
                              NULL,
                              NULL,
                              &poutParameters,
                              NULL);

    if (FAILED(hr))
    {
        CMyString msg;
        msg.Format(
            "Failed to get a paramter pointer while "
                "creating method: %s! Error: 0x%X",
            255,
            static_cast<char const *>
                (method.getName().getString().getCString()),
            hr);

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
                      "WMIClassProvider::createMethod() - %s", (LPCTSTR)msg));

        if (pinParameters)
            pinParameters.Release();
        if (poutParameters)
            poutParameters.Release();

        throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
    }

    // create the parameters
    for (Uint32 i = 0; i < method.getParameterCount(); i++)
    {
        CIMConstParameter param;
        param = method.getParameter(i);

        // Is this an in or out parameter, or both?
        if (param.findQualifier(CIMName("in")) != -1)
        {
            try
            {
                createParam(param, pinParameters);
            }
            catch (CIMException&)
            {
                if (pinParameters)
                    pinParameters.Release();
                if (poutParameters)
                    poutParameters.Release();

                throw;
            }
        }

        if (param.findQualifier(CIMName("out")) != -1)
        {
            try
            {
                createParam(param, poutParameters);
            }
            catch (CIMException&)
            {
                if (pinParameters)
                    pinParameters.Release();
                if (poutParameters)
                    poutParameters.Release();

                throw;
            }
        }
    }

    // create the method
    CComBSTR bs = method.getName().getString().getCString();
    hr = pNewClass->PutMethod(bs, NULL, pinParameters, poutParameters);

    if (pinParameters)
        pinParameters.Release();

    if (poutParameters)
        poutParameters.Release();

    if (FAILED(hr))
    {
        CMyString msg;
        msg.Format(
            "It is not possible to add the method [%s] ! Error: 0x%X",
            255,
            static_cast<char const *>
                (method.getName().getString().getCString()),
            hr);

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
                      "WMIClassProvider::createMethod() - %s", (LPCTSTR)msg));

        throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
    }


    // create the qualifiers for the method
    // get a pointer to work with qualifiers
    CComPtr<IWbemQualifierSet> pQual;
    hr = pNewClass->GetMethodQualifierSet(bs, &pQual);

    if (FAILED(hr))
    {
        CMyString msg;
        msg.Format(
            "It is not possible to get the qualifier "
                "set of the method [%s] ! Error: 0x%X",
            255,
            static_cast<char const *>
                (method.getName().getString().getCString()),
            hr);

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
                "WMIClassProvider::createMethod() - %s", (LPCTSTR)msg));

        if (pQual)
            pQual.Release();

        throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
    }

    // create the qualifiers for the method
    for (Uint32 i = 0; i < method.getQualifierCount(); i++)
    {
        CIMQualifier cimQual = method.getQualifier(i).clone();
        WMIQualifier qualifier(cimQual);

        try
        {
            createQualifier(qualifier, pQual);
        }
        catch (CIMException&)
        {
            if (pQual)
                pQual.Release();

            throw;
        }
    }

    if (pQual)
        pQual.Release();

    PEG_METHOD_EXIT();

    return;
}

/////////////////////////////////////////////////////////////////////////////
// WMIClassProvider::createParam  create a parameter
//
/////////////////////////////////////////////////////////////////////////////
void WMIClassProvider::createParam(const CIMConstParameter &param,
                                   IWbemClassObject *pNewClass)
{

    PEG_METHOD_ENTER(TRC_WMIPROVIDER, "WmiClassProvider::createParam()");

    //Get the CIMTYPE of the parameter
    CIMTYPE type = CIMTypeToWMIType(param.getType());

    //If the property is an array, add CIM_FLAG_ARRAY to CIMType
    if (param.isArray())
    {
        type |= CIM_FLAG_ARRAY;
    }

    // add the property to the class
    CComBSTR bs = param.getName().getString().getCString();
    HRESULT hr = pNewClass->Put(bs, NULL, NULL, type);

    //handle the error, if failed
    if (FAILED(hr))
    {
        CMyString msg;
        msg.Format(
            "It is not possible to add parameter [%s] "
                "to the parameters list! Error: 0x%X",
            255,
            static_cast<char const *>
                (param.getName().getString().getCString()),
            hr);

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
                      "WMIClassProvider::createParam() - %s", (LPCTSTR)msg));

        throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
    }

    // create the qualifiers for this parameter
    // get a pointer to work with qualifiers
    CComPtr<IWbemQualifierSet> pQual;
    hr = pNewClass->GetPropertyQualifierSet(bs, &pQual);

    if (FAILED(hr))
    {
        CMyString msg;
        msg.Format(
            "It is not possible to get a qualifier "
                "set for parameter [%s]! Error: 0x%X",
            255,
            static_cast<char const *>
                (param.getName().getString().getCString()),
            hr);

        PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL1,
                      "WMIClassProvider::createParam() - %s", (LPCTSTR)msg));

        if (pQual)
            pQual.Release();

        throw CIMException(CIM_ERR_FAILED, (LPCTSTR)msg);
    }

    // create the qualifiers for this parameter
    for (Uint32 i = 0; i < param.getQualifierCount(); i++)
    {
        CIMQualifier cimQual = param.getQualifier(i).clone();
        WMIQualifier qualifier(cimQual);

        try
        {
            createQualifier(qualifier, pQual);
        }
        catch (CIMException&)
        {
            if (pQual)
                pQual.Release();

            throw;
        }
    }

    if (pQual)
        pQual.Release();

    PEG_METHOD_EXIT();

    return;
}

PEGASUS_NAMESPACE_END
