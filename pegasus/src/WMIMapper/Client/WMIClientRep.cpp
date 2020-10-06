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
// Author: Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//
// Modified By: Terry Martin, Hewlett-Packard Company (terry.martin@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

#include "WMIClientRep.h"

///////////////////////////////////////////////////////////////////////////////
// Need to include these before the WMI Provider headers
#include <atlbase.h>
#include <comdef.h>
#include <wbemcli.h>

///////////////////////////////////////////////////////////////////////////////
// WMI Provider interface headers
#include <WMIMapper/WMIProvider/WMIInstanceProvider.h>
#include <WMIMapper/WMIProvider/WMIClassProvider.h>
#include <WMIMapper/WMIProvider/WMIAssociatorProvider.h>
#include <WMIMapper/WMIProvider/WMIReferenceProvider.h>
#include <WMIMapper/WMIProvider/WMIQualifierProvider.h>
#include <WMIMapper/WMIProvider/WMIMethodProvider.h>
#include <WMIMapper/WMIProvider/WMIQueryProvider.h>
///////////////////////////////////////////////////////////////////////////////

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

static Boolean verifyServerCertificate(SSLCertificateInfo &certInfo)
{
    //ATTN-NB-03-05132002: Add code to handle server certificate verification.
    return true;
}

///////////////////////////////////////////////////////////////////////////////
WMIClientRep::WMIClientRep(Uint32 timeoutMilliseconds):
    MessageQueue(PEGASUS_QUEUENAME_CLIENT),
    _timeoutMilliseconds(timeoutMilliseconds),
    _connected(false)
{
}

///////////////////////////////////////////////////////////////////////////////
WMIClientRep::~WMIClientRep()
{
   disconnect();
}

///////////////////////////////////////////////////////////////////////////////
void WMIClientRep::handleEnqueue()
{
}

///////////////////////////////////////////////////////////////////////////////
void WMIClientRep::connectLocal()
{
    //
    // If already connected, bail out!
    //
    if (_connected)
        throw AlreadyConnectedException();

    _connected = true;
}

///////////////////////////////////////////////////////////////////////////////
void WMIClientRep::disconnect()
{
    if (_connected)
        _connected = false;
}

///////////////////////////////////////////////////////////////////////////////
CIMClass WMIClientRep::getClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    CIMClass cimClass;
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
           WMIClassProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        cimClass = provider.getClass(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            className.getString(),
            localOnly,
            includeQualifiers,
            includeClassOrigin,
            propertyList);

        //terminate the provider
        provider.terminate();

    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "getClass() failed!");
        throw cimException;
    }

    return(cimClass);
}

///////////////////////////////////////////////////////////////////////////////
CIMInstance WMIClientRep::getInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    CIMInstance cimInstance;
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIInstanceProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        cimInstance = provider.getInstance(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            instanceName,
            localOnly,
            includeQualifiers,
            includeClassOrigin,
            propertyList);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "getInstance() failed!");
        throw cimException;
    }

    return(cimInstance);
}

///////////////////////////////////////////////////////////////////////////////
void WMIClientRep::deleteClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIClassProvider provider;
           provider.initialize(TRUE);

        //Performs the WMI call
        provider.deleteClass(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            className.getString());

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "deleteClass() failed!");
        throw cimException;
    }
}

///////////////////////////////////////////////////////////////////////////////
void WMIClientRep::deleteInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName)
{
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIInstanceProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        provider.deleteInstance(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            instanceName);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "deleteInstance() failed!");
        throw cimException;
    }
}

///////////////////////////////////////////////////////////////////////////////
void WMIClientRep::createClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass)
{
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIClassProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        provider.createClass(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            newClass);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "createClass() failed!");
        throw cimException;
    }
}

///////////////////////////////////////////////////////////////////////////////
CIMObjectPath WMIClientRep::createInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& newInstance)
{
    CIMObjectPath instanceName;
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIInstanceProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        instanceName = provider.createInstance(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            newInstance);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "createInstance() failed!");
        throw cimException;
    }

    return (instanceName);
}

///////////////////////////////////////////////////////////////////////////////
void WMIClientRep::modifyClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& modifiedClass)
{
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIClassProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        provider.modifyClass(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            modifiedClass);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "modifyClass() failed!");
        throw cimException;
    }
}

///////////////////////////////////////////////////////////////////////////////
void WMIClientRep::modifyInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& modifiedInstance,
    Boolean includeQualifiers,
    const CIMPropertyList& propertyList)
{
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIInstanceProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        provider.modifyInstance(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            modifiedInstance,
            includeQualifiers,
            propertyList);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "modifyInstance() failed!");
        throw cimException;
    }
}

///////////////////////////////////////////////////////////////////////////////
Array<CIMClass> WMIClientRep::enumerateClasses(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin)
{
    Array<CIMClass> cimClasses;
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIClassProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        cimClasses = provider.enumerateClasses(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            className.getString(),
            deepInheritance,
            localOnly,
            includeQualifiers,
            includeClassOrigin);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "enumerateClasses() failed!");
        throw cimException;
    }

    return(cimClasses);
}

///////////////////////////////////////////////////////////////////////////////
Array<CIMName> WMIClientRep::enumerateClassNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance)
{
    Array<CIMName> classNames;
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIClassProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        classNames = provider.enumerateClassNames(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            className.getString(),
            deepInheritance);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "enumerateClassNames() failed!");
        throw cimException;
    }

    return(classNames);
}

///////////////////////////////////////////////////////////////////////////////
Array<CIMInstance> WMIClientRep::enumerateInstances(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    Array<CIMInstance> cimInstances;
    CIMPropertyList myPropertyList(propertyList);
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIInstanceProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        cimInstances = provider.enumerateInstances(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            className.getString(),
            deepInheritance,
            localOnly,
            includeQualifiers,
            includeClassOrigin,
            myPropertyList);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "enumerateInstances() failed!");
        throw cimException;
    }

    return(cimInstances);
}

///////////////////////////////////////////////////////////////////////////////
Array<CIMObjectPath> WMIClientRep::enumerateInstanceNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    Array<CIMObjectPath> instanceNames;
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIInstanceProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        instanceNames = provider.enumerateInstanceNames(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            className.getString());

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "enumerateInstanceNames() failed!");
        throw cimException;
    }

    return(instanceNames);
}

///////////////////////////////////////////////////////////////////////////////
Array<CIMObject> WMIClientRep::execQuery(
    const CIMNamespaceName& nameSpace,
    const String& queryLanguage,
    const String& query)
{
    Array<CIMObject> cimObjects;
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIQueryProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        cimObjects = provider.execQuery(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            queryLanguage,
            query);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "execQuery() failed!");
        throw cimException;
    }

    return(cimObjects);
}

///////////////////////////////////////////////////////////////////////////////
Array<CIMObject> WMIClientRep::associators(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    Array<CIMObject> cimObjects;
    CIMPropertyList myPropertyList(propertyList);
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIAssociatorProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        cimObjects = provider.associators(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            objectName,
            assocClass.getString(),
            resultClass.getString(),
            role,
            resultRole,
            includeQualifiers,
            includeClassOrigin,
            myPropertyList);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "associators() failed!");
        throw cimException;
    }

    return(cimObjects);
}

///////////////////////////////////////////////////////////////////////////////
Array<CIMObjectPath> WMIClientRep::associatorNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole)
{
    Array<CIMObjectPath> objectNames;
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIAssociatorProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        objectNames = provider.associatorNames(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            objectName,
            assocClass.getString(),
            resultClass.getString(),
            role,
            resultRole);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "associatorNames() failed!");
        throw cimException;
    }

    return(objectNames);
}

///////////////////////////////////////////////////////////////////////////////
Array<CIMObject> WMIClientRep::references(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    Array<CIMObject> cimObjects;
    CIMPropertyList myPropertyList(propertyList);
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIReferenceProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        cimObjects = provider.references(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            objectName,
            resultClass.getString(),
            role,
            includeQualifiers,
            includeClassOrigin,
            myPropertyList);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "references() failed!");
        throw cimException;
    }

    return(cimObjects);
}

///////////////////////////////////////////////////////////////////////////////
Array<CIMObjectPath> WMIClientRep::referenceNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role)
{
    Array<CIMObjectPath> objectNames;
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIReferenceProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        objectNames = provider.referenceNames(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            objectName,
            resultClass.getString(),
            role);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "referenceNames() failed!");
        throw cimException;
    }

    return(objectNames);
}

///////////////////////////////////////////////////////////////////////////////
CIMValue WMIClientRep::getProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName)
{
    CIMValue value;
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIInstanceProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        value = provider.getProperty(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            instanceName,
            propertyName.getString());

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "getProperty() failed!");
        throw cimException;
    }

    return(value);
}

///////////////////////////////////////////////////////////////////////////////
void WMIClientRep::setProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName,
    const CIMValue& newValue
)
{
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIInstanceProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        provider.setProperty(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            instanceName,
            propertyName.getString(),
            newValue);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "setProperty() failed!");
        throw cimException;
    }
}

///////////////////////////////////////////////////////////////////////////////
CIMQualifierDecl WMIClientRep::getQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    CIMQualifierDecl cimQualifierDecl;
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIQualifierProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        cimQualifierDecl = provider.getQualifier(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            qualifierName.getString());

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "getQualifier() failed!");
        throw cimException;
    }

    return(cimQualifierDecl);
}

///////////////////////////////////////////////////////////////////////////////
void WMIClientRep::setQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMQualifierDecl& qualifierDeclaration)
{
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIQualifierProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        provider.setQualifier(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            qualifierDeclaration);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "setQualifier() failed!");
        throw cimException;
    }
}

///////////////////////////////////////////////////////////////////////////////
void WMIClientRep::deleteQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIQualifierProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        provider.deleteQualifier(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            qualifierName.getString());

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "deleteQualifier() failed!");
        throw cimException;
    }
}

///////////////////////////////////////////////////////////////////////////////
Array<CIMQualifierDecl> WMIClientRep::enumerateQualifiers(
    const CIMNamespaceName& nameSpace)
{
    Array<CIMQualifierDecl> qualifierDeclarations;
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIQualifierProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        qualifierDeclarations = provider.enumerateQualifiers(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "enumerateQualifiers() failed!");
        throw cimException;
    }

    return(qualifierDeclarations);
}

///////////////////////////////////////////////////////////////////////////////
CIMValue WMIClientRep::invokeMethod(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    Array<CIMParamValue>& outParameters)
{
    CIMValue retValue;
    CIMException cimException;

    try
    {
        //Initializes the WMI Provider Interface
        WMIMethodProvider provider;
        provider.initialize(TRUE);

        //Performs the WMI call
        retValue = provider.invokeMethod(
            nameSpace.getString(),
            String::EMPTY,
            String::EMPTY,
            instanceName,
            methodName.getString(),
            inParameters,
            outParameters);

        //terminate the provider
        provider.terminate();
    }
    catch(CIMException&)
    {
        throw;
    }
    catch(Exception& exception)
    {
       cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
           exception.getMessage());
       throw cimException;
    }
    catch(...)
    {
        cimException = PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "invokeMethod() failed!");
        throw cimException;
    }

    return(retValue);
}

PEGASUS_NAMESPACE_END
