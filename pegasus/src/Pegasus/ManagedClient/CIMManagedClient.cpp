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
//%/////////////////////////////////////////////////////////////////////////////

#include "CIMManagedClient.h"

PEGASUS_NAMESPACE_BEGIN

// class constructor
CIMManagedClient::CIMManagedClient()
{
    // *shrugs*
    // using this constructor doesn't seem to clever
    // remind, if you use it, don't forget to set the ConnectionManager
    // via setConnectionManager()
    setPegasusDefaultPort();
    _cccm = 0;
}

CIMManagedClient::CIMManagedClient(CIMClientConnectionManager* cccm)
{
    CDEBUG("Using given CIMClientConnectionManager*");
    setPegasusDefaultPort();
    _cccm = cccm;
}

void CIMManagedClient::setConnectionManager(CIMClientConnectionManager* cccm)
{
    // delete _cccm;
    _cccm = cccm;
}

CIMClientConnectionManager* CIMManagedClient::getConnectionManager(void)
{
    return _cccm;
}

CIMManagedClient::~CIMManagedClient()
{
    // ubs, we shouldn't delete the ClientConnectionManager,
    // as we don't know what it is anyway
    // delete _cccm;
}

// l10n start
void CIMManagedClient::setRequestAcceptLanguages(
    const String& host,
    const String& port,
    const AcceptLanguageList& langs
)
{
    CIMClientRep * _rep;
    _rep = getTargetCIMOM(host, port, CIMNamespaceName());
    _rep->setRequestAcceptLanguages(langs);
}

AcceptLanguageList CIMManagedClient::getRequestAcceptLanguages(
    const String& host,
    const String& port
) const
{
    CIMClientRep * _rep;
    _rep = getTargetCIMOM(host, port, CIMNamespaceName());
    return _rep->getRequestAcceptLanguages();
}

void CIMManagedClient::setRequestContentLanguages(
    const String& host,
    const String& port,
    const ContentLanguageList& langs
)
{
    CIMClientRep * _rep;
    _rep = getTargetCIMOM(host, port, CIMNamespaceName());
    _rep->setRequestContentLanguages(langs);
}

ContentLanguageList CIMManagedClient::getRequestContentLanguages(
    const String& host,
    const String& port) const
{
    CIMClientRep * _rep;
    _rep = getTargetCIMOM(host, port, CIMNamespaceName());
    return _rep->getRequestContentLanguages();
}

ContentLanguageList CIMManagedClient::getResponseContentLanguages(
    const String& host,
    const String& port) const
{
    CIMClientRep * _rep;
    _rep = getTargetCIMOM(host, port, CIMNamespaceName());
    return _rep->getResponseContentLanguages();
}

void CIMManagedClient::setRequestDefaultLanguages(
    const String& host,
    const String& port)
{
    CIMClientRep * _rep;
    _rep = getTargetCIMOM(host, port, CIMNamespaceName());
    _rep->setRequestDefaultLanguages();
}
// l10n end

Array<CIMObjectPath> CIMManagedClient::enumerateInstanceNames(
    const String& host,
    const String& port,
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    // test if host and namespace are provided
    hasHostandNameSpace(host, nameSpace);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(host, port, nameSpace);
    Array<CIMObjectPath> returnedInstanceNames =
        _rep->enumerateInstanceNames(nameSpace,className).getInstanceNames();
    for (Uint32 i = 0; i < returnedInstanceNames.size(); i++)
    {
        returnedInstanceNames[i].setHost(_getHostwithPort(host, port));
        returnedInstanceNames[i].setNameSpace(nameSpace);
    }
    return returnedInstanceNames;
}

Array<CIMInstance> CIMManagedClient::enumerateInstances(
    const String& host,
    const String& port,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList
)
{
    // test if host and namespace are provided
    hasHostandNameSpace(host, nameSpace);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(host, port, nameSpace);

    Array<CIMInstance> returnedNamedInstances = _rep->enumerateInstances(
        nameSpace,
        className,
        deepInheritance,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList).getInstances();

    // adding host, port and namespace to every Instance
    // ensures object path is full specified
    for (Uint32 i = 0; i < returnedNamedInstances.size(); i++)
    {
        CIMObjectPath chgObjectPath = CIMObjectPath(
                                          returnedNamedInstances[i].getPath());

        chgObjectPath.setHost(_getHostwithPort(host, port));
        chgObjectPath.setNameSpace(nameSpace);

        returnedNamedInstances[i].setPath(chgObjectPath);
    }
    return returnedNamedInstances;
}

Array<CIMObject> CIMManagedClient::execQuery(
    const String& host,
    const String& port,
    const CIMNamespaceName& nameSpace,
    const String& queryLanguage,
    const String& query
)
{
    // test if host and namespace are provided
    hasHostandNameSpace(host, nameSpace);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(host, port, nameSpace);
    Array<CIMObject> returnedCimObjects = _rep->execQuery(
        nameSpace,
        queryLanguage,
        query).getObjects();
    // check if all objects contain a full specified object path
    for (Uint32 i = 0; i < returnedCimObjects.size(); i++)
    {
        CIMObjectPath chgObjectPath = CIMObjectPath(
                                          returnedCimObjects[i].getPath());
        /**if there is...
            no cimom and no namespace specified, set the current as default
            no namespace, but a cimom specified, add the current namespace
            no cimom, but a namespace specified, add the current cimom

          if there are both cimom and namespace specified, do nothing
          thus, only do something if either cimom or namespace is missing
        */

        // test for empty cimom
        if (chgObjectPath.getHost() == String::EMPTY)
        {
            chgObjectPath.setHost(_getHostwithPort(host, port));
        }
        // test for empty namespace
        if (chgObjectPath.getNameSpace().isNull())
        {
            chgObjectPath.setNameSpace(nameSpace);
        }
        returnedCimObjects[i].setPath(chgObjectPath);
    }
    return returnedCimObjects;
}


Array<CIMClass> CIMManagedClient::enumerateClasses(
    const String& host,
    const String& port,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin
)
{
    // test if host and namespace are provided
    hasHostandNameSpace(host, nameSpace);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(host, port, nameSpace);

    Array<CIMClass> returnedCimClasses = _rep->enumerateClasses(
        nameSpace,
        className,
        deepInheritance,
        localOnly,
        includeQualifiers,
        includeClassOrigin);

    // adding host, port and namespace to every Instance to make sure
    // object path is full specified
    for (Uint32 i = 0; i < returnedCimClasses.size(); i++)
    {
        CIMObjectPath chgObjectPath = CIMObjectPath(
                                          returnedCimClasses[i].getPath());

        chgObjectPath.setHost(_getHostwithPort(host,port));
        chgObjectPath.setNameSpace(nameSpace);

        returnedCimClasses[i].setPath(chgObjectPath);
    }
    return returnedCimClasses;
}

Array<CIMName> CIMManagedClient::enumerateClassNames(
    const String& host,
    const String& port,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance
)
{
    // test if host and namespace are provided
    hasHostandNameSpace(host, nameSpace);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(host, port, nameSpace);
    return _rep->enumerateClassNames(
        nameSpace,
        className,
        deepInheritance);
}

CIMClass CIMManagedClient::getClass(
    const String& host,
    const String& port,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList
)
{
    // test if host and namespace are provided
    hasHostandNameSpace(host, nameSpace);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(host, port, nameSpace);
    CIMClass returnedCimClass = _rep->getClass(
        nameSpace,
        className,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList);

    CIMObjectPath chgObjectPath = CIMObjectPath(returnedCimClass.getPath());

    chgObjectPath.setHost(_getHostwithPort(host, port));
    chgObjectPath.setNameSpace(nameSpace);

    returnedCimClass.setPath(chgObjectPath);
    return returnedCimClass;
}

void CIMManagedClient::createClass(
    const String& host,
    const String& port,
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass
)
{
    // test if host and namespace are provided
    hasHostandNameSpace(host, nameSpace);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(host, port, nameSpace);
    _rep->createClass(
        nameSpace,
        newClass);
}

void CIMManagedClient::deleteClass(
    const String& host,
    const String& port,
    const CIMNamespaceName& nameSpace,
    const CIMName& className
)
{
    // test if host and namespace are provided
    hasHostandNameSpace(host, nameSpace);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(host, port, nameSpace);
    _rep->deleteClass(
        nameSpace,
        className);
}

void CIMManagedClient::deleteQualifier(
    const String& host,
    const String& port,
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName
)
{
    // test if host and namespace are provided
    hasHostandNameSpace(host, nameSpace);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(host, port, nameSpace);
    _rep->deleteQualifier(
        nameSpace,
        qualifierName);
}

CIMQualifierDecl CIMManagedClient::getQualifier(
    const String& host,
    const String& port,
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName
)
{
    // test if host and namespace are provided
    hasHostandNameSpace(host, nameSpace);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(host, port, nameSpace);
    return _rep->getQualifier(
        nameSpace,
        qualifierName);
}

Array<CIMQualifierDecl> CIMManagedClient::enumerateQualifiers(
    const String& host,
    const String& port,
    const CIMNamespaceName& nameSpace
)
{
    // test if host and namespace are provided
    hasHostandNameSpace(host, nameSpace);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(host, port, nameSpace);
    return _rep->enumerateQualifiers(
        nameSpace);
}


void CIMManagedClient::modifyClass(
    const String& host,
    const String& port,
    const CIMNamespaceName& nameSpace,
    const CIMClass& modifiedClass
)
{
    // test if host and namespace are provided
    hasHostandNameSpace(host, nameSpace);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(host, port, nameSpace);
    _rep->modifyClass(
        nameSpace,
        modifiedClass);
}

void CIMManagedClient::setQualifier(
    const String& host,
    const String& port,
    const CIMNamespaceName& nameSpace,
    const CIMQualifierDecl& qualifierDeclaration
)
{
    // test if host and namespace are provided
    hasHostandNameSpace(host, nameSpace);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(host, port, nameSpace);
    _rep->setQualifier(
        nameSpace,
        qualifierDeclaration);
}

CIMValue CIMManagedClient::getProperty(
    const CIMObjectPath& instanceName,
    const CIMName& propertyName
)
{
    hasHostandNameSpace(instanceName);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(instanceName);
    return _rep->getProperty(
        instanceName.getNameSpace(),
        instanceName,
        propertyName);
}

void CIMManagedClient::setProperty(
    const CIMObjectPath& instanceName,
    const CIMName& propertyName,
    const CIMValue& newValue
)
{
    hasHostandNameSpace(instanceName);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(instanceName);
    _rep->setProperty(
        instanceName.getNameSpace(),
        instanceName,
        propertyName,
        newValue);
}


void CIMManagedClient::deleteInstance(
    const CIMObjectPath& instanceName
)
{
    hasHostandNameSpace(instanceName);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(instanceName);
    _rep->deleteInstance(
        instanceName.getNameSpace(),
        instanceName);
}

CIMObjectPath CIMManagedClient::createInstance(
    const CIMInstance& newInstance
)
{
    hasHostandNameSpace(newInstance);
    // save name space for later usage
    CIMNamespaceName reqNameSpace = CIMNamespaceName(
                                        newInstance.getPath().getNameSpace());
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(newInstance.getPath());
    CIMObjectPath returnedObjectPath = _rep->createInstance(
                    reqNameSpace,
                    newInstance);
    // put host and namespace back into object path,
    // so it is definite fully specified
    returnedObjectPath.setHost(newInstance.getPath().getHost());
    returnedObjectPath.setNameSpace(reqNameSpace);
    return returnedObjectPath;
}

CIMInstance CIMManagedClient::getInstance(
    const CIMObjectPath& instanceName,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList
)
{
    hasHostandNameSpace(instanceName);
    CIMNamespaceName reqNameSpace = CIMNamespaceName(
                                        instanceName.getNameSpace());
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(instanceName);
    CIMInstance returnedCimInstance = _rep->getInstance(
        reqNameSpace,
        instanceName,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList).getInstance();
/*
    CIMObjectPath chgObjectPath = CIMObjectPath(returnedCimInstance.getPath());

    chgObjectPath.setHost(instanceName.getHost());
    chgObjectPath.setNameSpace(reqNameSpace);

    returnedCimInstance.setPath(chgObjectPath);
*/
    // changed to take over the entire CIMObjectPath that was given ...
    returnedCimInstance.setPath(instanceName);

    return returnedCimInstance;
}

void CIMManagedClient::modifyInstance(
    const CIMInstance& modifiedInstance,
    Boolean includeQualifiers,
    const CIMPropertyList& propertyList
)
{
    hasHostandNameSpace(modifiedInstance);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(modifiedInstance.getPath());
    _rep->modifyInstance(
        modifiedInstance.getPath().getNameSpace(),
        modifiedInstance,
        includeQualifiers,
        propertyList);
}


CIMValue CIMManagedClient::invokeMethod(
    const CIMObjectPath& instanceName,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    Array<CIMParamValue>& outParameters
)
{
    hasHostandNameSpace(instanceName);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(instanceName);
    return _rep->invokeMethod(
        instanceName.getNameSpace(),
        instanceName,
        methodName,
        inParameters,
        outParameters);
}

Array<CIMObjectPath> CIMManagedClient::associatorNames(
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole
)
{
    hasHostandNameSpace(objectName);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(objectName);
    Array<CIMObjectPath> retAssocNames = _rep->associatorNames(
        objectName.getNameSpace(),
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole).getInstanceNames();

    for (Uint32 i = 0; i < retAssocNames.size(); i++)
    {
        try
        {
            // check if all object paths are fully qualified
            hasHostandNameSpace(retAssocNames[i]);
        }
        catch (TypeMismatchException&)
        {
            // should throw nasty exception about missing namespace ....
            // TODO: prepare exception for this
            MessageLoaderParms retTypeMismatchMessage = MessageLoaderParms(
                "Client.CIMClientRep.TYPEMISMATCH_PORTMISMATCH",
                "Returned CIM object path incomplete specified.");
            throw TypeMismatchException(retTypeMismatchMessage);
        }
    }
    return retAssocNames;
}

Array<CIMObject> CIMManagedClient::associators(
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList
)
{
    hasHostandNameSpace(objectName);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(objectName);
    Array<CIMObject> retAssoc = _rep->associators(
        objectName.getNameSpace(),
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole,
        includeQualifiers,
        includeClassOrigin,
        propertyList).getObjects();
    for (Uint32 i = 0; i < retAssoc.size(); i++)
    {
        // check if all object paths are fully qualified
        try
        {
            hasHostandNameSpace(retAssoc[i].getPath());
        }
        catch (TypeMismatchException&)
        {
            // should throw nasty exception about missing namespace ....
            // TODO: prepare exception for this
            MessageLoaderParms retTypeMismatchMessage = MessageLoaderParms(
                "Client.CIMClientRep.TYPEMISMATCH_PORTMISMATCH",
                "Returned CIM object path incomplete specified.");
            throw TypeMismatchException(retTypeMismatchMessage);
        }
    }
    return retAssoc;
}

Array<CIMObject> CIMManagedClient::references(
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList
)
{
    hasHostandNameSpace(objectName);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(objectName);
    Array<CIMObject> retRefer = _rep->references(
        objectName.getNameSpace(),
        objectName,
        resultClass,
        role,
        includeQualifiers,
        includeClassOrigin,
        propertyList).getObjects();
    for (Uint32 i = 0; i < retRefer.size(); i++)
    {
        // check if all object paths are fully qualified
        try
        {
            hasHostandNameSpace(retRefer[i].getPath());
        }
        catch (TypeMismatchException&)
        {
            // should throw nasty exception about missing namespace ....
            // TODO: prepare exception for this
            MessageLoaderParms retTypeMismatchMessage = MessageLoaderParms(
                "Client.CIMClientRep.TYPEMISMATCH_PORTMISMATCH",
                "Returned CIM object path incomplete specified.");
            throw TypeMismatchException(retTypeMismatchMessage);
        }
    }
    return retRefer;
}

Array<CIMObjectPath> CIMManagedClient::referenceNames(
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role
)
{
    hasHostandNameSpace(objectName);
    CIMClientRep *   _rep;
    _rep = getTargetCIMOM(objectName);
    Array<CIMObjectPath> retReferNames = _rep->referenceNames(
        objectName.getNameSpace(),
        objectName,
        resultClass,
        role).getInstanceNames();
    for (Uint32 i = 0; i < retReferNames.size(); i++)
    {
        try
        {
            // check if all object paths are fully qualified
            hasHostandNameSpace(retReferNames[i]);
        }
        catch (TypeMismatchException&)
        {
            // should throw nasty exception about missing namespace ....
            // TODO: prepare exception for this
            MessageLoaderParms retTypeMismatchMessage = MessageLoaderParms(
                "Client.CIMClientRep.TYPEMISMATCH_PORTMISMATCH",
                "Returned CIM object path incomplete specified.");
            throw TypeMismatchException(retTypeMismatchMessage);
        }
    }
    return retReferNames;
}



void CIMManagedClient::hasHostandNameSpace(
    const String& _host,
    const CIMNamespaceName& _nameSpace)
{
    MessageLoaderParms typeMismatchMessage;
    if (_host == String::EMPTY)
    {
        // should throw nasty exception about missing hostname ....
        // TODO: prepare exception for this
        typeMismatchMessage = MessageLoaderParms(
            "Client.CIMClientRep.TYPEMISMATCH_PORTMISMATCH",
            "Failed validation of CIM object path: no host name specified");
        throw TypeMismatchException(typeMismatchMessage);
    }
    if (_nameSpace.isNull())
    {
        // should throw nasty exception about missing namespace ....
        // TODO: prepare exception for this
        typeMismatchMessage = MessageLoaderParms(
            "Client.CIMClientRep.TYPEMISMATCH_PORTMISMATCH",
            "Failed validation of CIM object path: no namespace specified");
        throw TypeMismatchException(typeMismatchMessage);
    }
    CDEBUG("No exception thrown, seems the host and namespace are there.");
}

void CIMManagedClient::hasHostandNameSpace(const CIMObjectPath& inObjectPath)
{
    CDEBUG("hasHostandNameSpace(inObjectPath.getHost(),"
               " inObjectPath.getNameSpace())="
           << inObjectPath.getHost()
           << ","
           << inObjectPath.getNameSpace());
    hasHostandNameSpace(inObjectPath.getHost(), inObjectPath.getNameSpace());
}

void CIMManagedClient::hasHostandNameSpace(const CIMInstance& inInstance)
{
    hasHostandNameSpace(inInstance.getPath());
}

CIMClientRep* CIMManagedClient::getTargetCIMOM(
    const String& _host,
    const String& _port,
    const CIMNamespaceName& _nameSpace) const
{
    CIMClientRep *   targetCIMOM;
    if (strtoul((const char*) _port.getCString(), NULL, 0) == 0)
    {
        CDEBUG("no port given explicitly, thus we use the default port:"
                   << _pegasusDefaultPort);
        // no port given explicitly, thus we use the default port of 5988
        // lets determine if there is a connection for this objectpath
        targetCIMOM = _cccm->getConnection(
                          _host,
                          _pegasusDefaultPort,
                          _nameSpace);
    } else {
        // lets determine if a connection for this object path available
        targetCIMOM = _cccm->getConnection(_host, _port, _nameSpace);
    }
    // damn, somehow we missed to construct a CIMClientRep for this connection
    // shouldn't be possible at all, but one never knows everything
    if (targetCIMOM == 0)
    {
        // throw some crazy exception
        // TODO: invent exception
        MessageLoaderParms typeMismatchMessage = MessageLoaderParms(
            "Client.CIMClientRep.TYPEMISMATCH_PORTMISMATCH",
            "No valid CIMOM connection configured for: ($0:$1) ",
            _host,
            _port);
        throw TypeMismatchException(typeMismatchMessage);
    }
    CDEBUG("targetCIMOM=" << targetCIMOM);
    return targetCIMOM;
}

CIMClientRep* CIMManagedClient::getTargetCIMOM(
    const CIMObjectPath& inObjectPath) const
{
    String inHost, inPort;
    CIMNamespaceName inNameSpace;
    inNameSpace = inObjectPath.getNameSpace();

    // need to create our own Host String object,
    // as we split it in two pieces
    // thus change it ...
    inHost = String(inObjectPath.getHost());

    // test if a host is given at all not necessary here
    // if there is no we failed to detect that before anyway
    // wonder how that should possibly happen???
    HostLocator addr(inHost);
    if (addr.isPortSpecified())
    {
        inHost = addr.getHost();
        inPort = addr.getPortString();
    }

    return getTargetCIMOM(inHost, inPort, inNameSpace);
}


void CIMManagedClient::setPegasusDefaultPort()
{
    _pegasusDefaultPort = String("5988");
}

String CIMManagedClient::_getHostwithPort(
    const String& host,
    const String& port)
{
    String hostwithport=String(host);
    hostwithport.append(":");
    hostwithport.append(port);
    return hostwithport;
}

PEGASUS_NAMESPACE_END
