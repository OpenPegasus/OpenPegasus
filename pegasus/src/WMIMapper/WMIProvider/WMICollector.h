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
// Modified By:    Adriano Zanuz (adriano.zanuz@hp.com)
//              Terry Martin, Hewlett-Packard Company (terry.martin@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////


#ifndef Pegasus_WMICollector_h
#define Pegasus_WMICollector_h

#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMClass.h>

#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMParamValue.h>

//#include <Ntsecapi.h>
#include "config.h"

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_WMIPROVIDER_LINKAGE WMICollector
{
public:
    WMICollector(bool bLocal = FALSE);
    virtual ~WMICollector(void);

    virtual void terminate(void);

    bool getInstanceEnum(
        IEnumWbemClassObject **ppInstances,
        const String & sClassName,
        Boolean deepInheritance);

    bool getClassEnum(
        IEnumWbemClassObject **ppClasses,
        const String & sSuperClass,
        Boolean deepInheritance);

    bool getQueryResult(
        IEnumWbemClassObject **ppInstances,
        const String & query,
        const String & queryLanguage);

    void setNamespace(const char * sNamespace)
    {
        m_bsNamespace = (LPCTSTR)sNamespace;
    }

    void setNamespace(const String & sNamespace);
    void setUserName(const String & sUserName);
    void setPassword(const String & sPassword);
    bool Connect(IWbemServices **ppServices);
    bool setup();

    bool getObject(IWbemClassObject **ppObject, const String & sObjectName);

    bool getCIMInstance(IWbemClassObject *pObject,
        CIMInstance & cimInst,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList = CIMPropertyList(),
        Boolean getKeyProperties = FALSE);

    bool getCIMObject(IWbemClassObject *pObject,
        CIMObject & cimObj,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList = CIMPropertyList(),
        Boolean getKeyProperties = FALSE);


    bool getCIMClass(IWbemClassObject *pObject,
        CIMClass & cimClass,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList = CIMPropertyList());

    static String getClassName(IWbemClassObject *pObject);
    String getSuperClass(IWbemClassObject *pClass);
    String getRelativePath(IWbemClassObject *pObject);

    bool isInstance(IWbemClassObject *pObject);

    bool getObjectProperties(IWbemClassObject * pObject,
        CIMObject & cimObj,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        Boolean bGetKeyProperties = FALSE);

    static CIMProperty getProperty(IWbemClassObject *pClass,
        const CComBSTR & bsName,
        const CComVariant & vValue, // this will be NULL for class objects
        CIMTYPE type,
        Boolean includeClassOrigin,
        Boolean includeQualifiers,
        Boolean bPropagated);

    bool getClassMethods(IWbemClassObject *pObject,
        CIMClass & cimClass,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin);

    static CIMMethod getMethod(IWbemClassObject *pClass,
        const CComBSTR &bsName,
        const CComPtr<IWbemClassObject> &inParameters,
        const CComPtr<IWbemClassObject> &outParameters,
        Boolean includeClassOrigin,
        Boolean includeQualifiers,
        Boolean bPropagated);

    bool setProxySecurity(IUnknown * pProxy);

    bool isLocalNamespace()
        {return m_bIsLocalNamespace;}

    bool isLocalConnection()
        { return m_bLocalConnection; }

private:
    static String getStringProperty(
        IWbemClassObject *pObject,
        const CComBSTR & bsPropertyName);

    bool isArrayType(VARTYPE vt)
        {return (vt & VT_ARRAY) ? true : false;}

    bool isReferenceType(VARTYPE vt)
        {return (vt & VT_BYREF) ? true : false;}

    void logonUser();

    void revertToSelf();

    /*
    LSA_HANDLE GetPolicyHandle();
    bool GetAccountSid(LPTSTR, LPTSTR, PSID*);
    void AddPrivileges(PSID, LSA_HANDLE);
    void InitLsaString(PLSA_UNICODE_STRING, LPWSTR);
    */

    bool    m_bInitialized;
    bool    m_bIsLocalNamespace;
    bool    m_bImpersonate;
    bool    m_bLocalConnection;

    // WMI interfaces
    CComBSTR                m_bsNamespace;
    CComBSTR                m_bsUserName;
    CComBSTR                m_bsPassword;
    CComBSTR                m_bsDomain;
};

PEGASUS_NAMESPACE_END

#endif
