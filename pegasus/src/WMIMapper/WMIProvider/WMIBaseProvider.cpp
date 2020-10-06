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
// Modified By:     Adriano Zanuz (adriano.zanuz@hp.com)
//               Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//               Mateus Baur, Hewlett-Packard Company (jair.santos@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////
// WMIBaseProvider.cpp: implementation of the WMIBaseProvider class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "WMICollector.h"
#include "WMIBaseProvider.h"
#include "WMIClassProvider.h"
#include "WMIInstanceProvider.h"
#include "WMIQueryProvider.h"

#include "WMIProperty.h"
#include "WMIString.h"
#include "WMIValue.h"
#include "WMIQualifier.h"
#include "WMIQualifierSet.h"
#include "WMIType.h"
#include "WMIException.h"

PEGASUS_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WMIBaseProvider::WMIBaseProvider()
{
}

WMIBaseProvider::~WMIBaseProvider()
{
}

/////////////////////////////////////////////////////////////////////////////
// WMIBaseProvider::initialize
//
// ///////////////////////////////////////////////////////////////////////////
void WMIBaseProvider::initialize(bool bLocal)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIBaseProvider::initialize()");

    initCollector(bLocal);

    PEG_METHOD_EXIT();
}

/////////////////////////////////////////////////////////////////////////////
// WMIBaseProvider::terminate
//
// ///////////////////////////////////////////////////////////////////////////
void WMIBaseProvider::terminate(void)
{
    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIBaseProvider::terminate()");

    cleanup();

    PEG_METHOD_EXIT();
}


/////////////////////////////////////////////////////////////////////////////
// WMIBaseProvider::setup
//
/////////////////////////////////////////////////////////////////////////////
void WMIBaseProvider::setup(const String & nameSpace,
                            const String & userName,
                            const String & password)
{
    m_sNamespace = nameSpace;
    m_sUserName = userName;
    m_sPassword = password;

    if (!m_bInitialized)
    {
        initCollector();
    }

    if (m_bInitialized)
    {
        _collector->setNamespace(m_sNamespace);

        if (m_sUserName != String::EMPTY)
            _collector->setUserName(m_sUserName);

        if (m_sPassword != String::EMPTY)
            _collector->setPassword(m_sPassword);
    }
}

/////////////////////////////////////////////////////////////////////////////
// WMIBaseProvider::initCollector
//
// ///////////////////////////////////////////////////////////////////////////
void WMIBaseProvider::initCollector(bool bLocal)
{

    if (!m_bInitialized)
    {
        _collector = new WMICollector(bLocal);
        m_bInitialized = _collector->setup();
    }

}

/////////////////////////////////////////////////////////////////////////////
// WMIBaseProvider::cleanup
//
// ///////////////////////////////////////////////////////////////////////////
void WMIBaseProvider::cleanup()
{
    if (m_bInitialized)
    {
        _collector->terminate();
        delete _collector;
        _collector = NULL;
        m_bInitialized = false;
    }
}

/////////////////////////////////////////////////////////////////////////////
// WMIBaseProvider::getCIMInstance - retrieves a CIMInstance object
//
// ///////////////////////////////////////////////////////////////////////////
CIMInstance WMIBaseProvider::getCIMInstance(
    const String& nameSpace,
    const String& userName,
    const String& password,
    const CIMObjectPath &instanceName,
    const CIMPropertyList &propertyList)
{

    CIMInstance cimInstance;
    CIMStatusCode errorCode = CIM_ERR_SUCCESS;
    String errorDescription;
    WMIInstanceProvider provider;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIBaseProvider::getCIMInstance()");

    try
    {
        // This fix uses the current boolean value stored in collector
        // to initialize it.
        provider.initialize(_collector->isLocalConnection());

        cimInstance = provider.getInstance(nameSpace,
                                           userName,
                                           password,
                                           instanceName,
                                           false,
                                           false,
                                           false,
                                           propertyList);
        provider.terminate();
    }
    catch(CIMException& exception)
    {
        provider.terminate();
        errorCode = exception.getCode();
        errorDescription = exception.getMessage();
        throw PEGASUS_CIM_EXCEPTION(errorCode, errorDescription);
    }
    catch(Exception& exception)
    {
        provider.terminate();
        errorCode = CIM_ERR_FAILED;
        errorDescription = exception.getMessage();
        throw PEGASUS_CIM_EXCEPTION(errorCode, errorDescription);
    }
    catch(...)
    {
        provider.terminate();
        throw CIMException(CIM_ERR_FAILED);
    }

    PEG_METHOD_EXIT();

    return cimInstance;
}


/////////////////////////////////////////////////////////////////////////////
// WMIBaseProvider::getCIMClass - retrieves a CIMClass object
//
// ///////////////////////////////////////////////////////////////////////////
CIMClass WMIBaseProvider::getCIMClass(const String& nameSpace,
                                        const String& userName,
                                        const String& password,
                                        const String& className,
                                        const CIMPropertyList &propertyList)
{
    CIMClass cimClass;
    CIMStatusCode errorCode = CIM_ERR_SUCCESS;
    String errorDescription;
    WMIClassProvider provider;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIBaseProvider::getCIMClass()");

    try
    {
        // This fix uses the current boolean value stored in collector
        // to initialize it.
        provider.initialize(_collector->isLocalConnection());

        cimClass = provider.getClass(nameSpace, userName, password,
            className, false, true, true, propertyList);

        provider.terminate();
    }
    catch(CIMException& exception)
    {
        provider.terminate();
        errorCode = exception.getCode();
        errorDescription = exception.getMessage();
        throw PEGASUS_CIM_EXCEPTION(errorCode, errorDescription);
    }
    catch(Exception& exception)
    {
        provider.terminate();
        errorCode = CIM_ERR_FAILED;
        errorDescription = exception.getMessage();
        throw PEGASUS_CIM_EXCEPTION(errorCode, errorDescription);
    }
    catch(...)
    {
        provider.terminate();
        throw CIMException(CIM_ERR_FAILED);
    }

    PEG_METHOD_EXIT();

    return cimClass;
}

/////////////////////////////////////////////////////////////////////////////
// WMIBaseProvider::execCIMQuery - retrieves a query result
//
// ///////////////////////////////////////////////////////////////////////////
Array<CIMObject> WMIBaseProvider::execCIMQuery(
    const String& nameSpace,
    const String& userName,
    const String& password,
    const String& queryLanguage,
    const String& query,
    const CIMPropertyList& propertyList,
    Boolean includeQualifiers,
    Boolean includeClassOrigin)
{
    Array<CIMObject> objects;

    CIMInstance cimInstance;
    CIMStatusCode errorCode = CIM_ERR_SUCCESS;
    String errorDescription;
    WMIQueryProvider provider;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIBaseProvider::execCIMQuery()");

    try
    {
        // This fix uses the current boolean value stored in collector
        // to initialize it.
        provider.initialize(_collector->isLocalConnection());

        objects = provider.execQuery(nameSpace,
                    userName,
                    password,
                    queryLanguage,
                    query,
                    propertyList,
                    includeQualifiers,
                    includeClassOrigin);

        provider.terminate();
    }
    catch(CIMException& exception)
    {
        provider.terminate();
        errorCode = exception.getCode();
        errorDescription = exception.getMessage();
        throw PEGASUS_CIM_EXCEPTION(errorCode, errorDescription);
    }
    catch(Exception& exception)
    {
        provider.terminate();
        errorCode = CIM_ERR_FAILED;
        errorDescription = exception.getMessage();
        throw PEGASUS_CIM_EXCEPTION(errorCode, errorDescription);
    }
    catch(...)
    {
        provider.terminate();
        throw CIMException(CIM_ERR_FAILED);
    }

    PEG_METHOD_EXIT();

    return objects;
}


//////////////////////////////////////////////////////////////////////////////
// WMIBaseProvider::getQueryString - builds the query string from the
//        input parameters for Associator and Reference commands
//
// ///////////////////////////////////////////////////////////////////////////
String WMIBaseProvider::getQueryString(const CIMObjectPath &objectName,
        const String &sQueryCommand,
        const String &assocClass,
        const String &resultClass,
        const String &role,
        const String &resultRole)
{
    bool hasWHERE = false;
    bool isInst;

    //first we need to get the object name
    String sObjName = getObjectName(objectName);

    // check if is an instance name
    Uint32 pos = sObjName.find(qString(Q_PERIOD));
    isInst = (PEG_NOT_FOUND != pos);

    CMyString sQuery;

    int strLength = CMyString(sQueryCommand).GetLength() +
        CMyString(sObjName).GetLength() + 1;
    sQuery.Format(
        CMyString(sQueryCommand),
        strLength, static_cast<LPCTSTR>(CMyString(sObjName)));

    //set up any optional parameters
    if (!((0 == assocClass.size()) && (0 == resultClass.size()) &&
          (0 == role.size()) && (0 == resultRole.size())))
    {
        // we have optional parameters, append the appropriate ones
        sQuery += qChar(Q_WHERE);
        hasWHERE = true;

        if (0 != assocClass.size())
        {
            sQuery += qChar(Q_ASSOC_CLS);
            sQuery += assocClass;
        }

        if (0 != resultClass.size())
        {
            sQuery += qChar(Q_RESULT_CLASS);
            sQuery += resultClass;
        }

        if (0 != role.size())
        {
            sQuery += qChar(Q_ROLE);
            sQuery += role;
        }

        if (0 != resultRole.size())
        {
            sQuery += qChar(Q_RESULT_ROLE);
            sQuery += resultRole;
        }
    }

    // check if an instance
    if (!isInst)
    {
        // have a class, add "SchemaOnly"
        if (!hasWHERE)
        {
            sQuery += qChar(Q_WHERE);
        }

        sQuery += qChar(Q_SCHEMA);
    }

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
        "WMIBaseProvider::getQueryString() - Query is %s", (LPCTSTR)sQuery));

    String s = (LPCTSTR)sQuery;
    return s;
}

//////////////////////////////////////////////////////////////////////////////
// WMIBaseProvider::getObjectName - extracts the String object name from
//        CIMObjectPath
//        removes namespace
//
// Possible input Object Path formats:
// 1. Fully-qualified path
//     example: \\hostname:port\root\cimv2:ClassName.Key1="Value",Key2="Value"
//
// 2. No hostname & port (implies current host)
//     example: root\cimv2:ClassName.Key1="Value",Key2="Value"
//
// 3. No namespace (implies current namespace):
//     example: ClassName.Key1="Value",Key2="Value"
//
// 4. Reference instance
//     example: ClassName.Key1=R"root\cimv2:RefClass.Key="RefValue""
//
// In all cases, this method needs to return only the class name and keys from
// the input object path (need to strip any hostname, port, and namespace).
// For example, the return for cases #1-3, above, should be:
//    ClassName.Key1="Value",Key2="Value"
//
// Also, for "reference" keys, the reference indicator (R) needs to be
// removed. Therefore, the output from case #4, above, would be:
//    ClassName.Key1="root\cimv2:RefClass.Key="RefValue""
//
// ///////////////////////////////////////////////////////////////////////////
String WMIBaseProvider::getObjectName( const CIMObjectPath& objectName)
{
    String sObjName;
    String sObjNameLower;
    bool bHaveReference = false;

    PEG_METHOD_ENTER(TRC_WMIPROVIDER,"WMIBaseProvider::getObjectName()");

    sObjName = objectName.toString();
    sObjNameLower = sObjName;
    sObjNameLower.toLower();

    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
        "WMIBaseProvider::getObjectName() - ObjectName: %s",
        sObjName.getCString()));

    Uint32 pos;

    // 1. if Object name initiates with a hostname then remove it
    if ((sObjName.subString(0, 4) != "root") &&
        (sObjNameLower.subString(0, 2) != "//") &&
        (sObjNameLower.subString(0, 2) != "\\\\"))
    {
        pos = sObjNameLower.find("root");

        if (sObjNameLower.find("=") > pos) {

            if (PEG_NOT_FOUND != pos)
            {
                sObjName.remove(0, pos);
                 sObjNameLower.remove(0, pos);
            }
        }
    }

    //2. Remove the machine name and port if it exists
    if ((sObjNameLower.subString(0, 2) == "//") ||
        (sObjNameLower.subString(0, 2) == "\\\\"))
    {
        pos = sObjNameLower.find("root");

        if (PEG_NOT_FOUND != pos)
        {
            sObjName.remove(0, pos);
            sObjNameLower.remove(0, pos);
        }

        //3. After ensuring that all stuff before root was removed,
        //   get the class/instance name.
        pos = sObjName.find(qString(Q_COLON));

        if (PEG_NOT_FOUND != pos)
        {
            sObjName.remove(0, pos + 1);
        }
    }
    else
    {
        //   get the class/instance name.
        if (sObjNameLower.subString(0, 4) == "root")
        {
            pos = sObjName.find(qString(Q_COLON));

            if (PEG_NOT_FOUND != pos)
            {
                sObjName.remove(0, pos + 1);
            }
        }
    }


    PEG_TRACE((TRC_WMIPROVIDER, Tracer::LEVEL3,
        "WMIBaseProvider::getObjectName() - ObjectName: %s",
        sObjName.getCString()));

    PEG_METHOD_EXIT();

    return sObjName;
}

PEGASUS_NAMESPACE_END
