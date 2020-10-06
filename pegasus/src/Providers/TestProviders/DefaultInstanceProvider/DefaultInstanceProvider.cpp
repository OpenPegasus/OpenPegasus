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

//
// This is a generic instance provider for any CIM class.  It serves as
// a testing tool to allow client developers to test their client
// applications without having to have an instance provider available.
//
// Client developers that want to make use of this provider must register
// this provider for their CIM classes.
//
// This provider uses the existing repository interface to store and
// retrieve instances in a repository.  A separate repository is created
// to store the instances.  The location of this new repository can be
// specified in the environment variable PEGASUS_TEST_REPOSITORY.  If
// the environment variable is not set, the default location for the
// new repository will be /tmp.
//
// The first time this provider is called to handle a request for a
// particular CIM class, it creates the new repository if it does not
// already exist.  It then uses the repository interface to copy the
// necessary objects (such as the namespace, the CIM class, superclasses
// of the CIM class, and all the qualifiers) from the original repository
// to the new repository.  After the new repository is created, all
// subsequent requests on the same CIM class will be processed using
// the repository interface to access objects stored in the new repository.
//

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/CIMQualifierDecl.h>
#include <Pegasus/Client/CIMClient.h>

#include "DefaultInstanceProvider.h"

PEGASUS_NAMESPACE_BEGIN

DefaultInstanceProvider::DefaultInstanceProvider()
{
}

DefaultInstanceProvider::~DefaultInstanceProvider()
{
}

void DefaultInstanceProvider::initialize(CIMOMHandle& cimom)
{
    // get the test repository path
    const char* testRepositoryPath = getenv("PEGASUS_TEST_REPOSITORY");

    String repositoryDir;
    if (!testRepositoryPath)
    {
        // environment variable not set
        repositoryDir = "/tmp";
    }
    else
    {
        repositoryDir = testRepositoryPath;
    }
    repositoryDir.append("/repository");

    // create the repository object
    _repository = new CIMRepository(repositoryDir);
}

void DefaultInstanceProvider::terminate()
{
    delete this;
}

/***************************************************************************
   _copyClass
***************************************************************************/

void DefaultInstanceProvider::_copyClass(
    const String& nameSpace,
    const String& className)
{
    CIMClient client;
    try
    {
        // connect to the CIM server as a client
        client.connectLocal();
    }
    catch (Exception& ex)
    {
        const String msg = "Connect failed. " + ex.getMessage();
        throw CIMOperationFailedException( msg );
    }

    // get the class
    CIMClass cimClass;
    Boolean localOnly = false;
    Boolean includeQualifiers = true;
    Boolean includeClassOrigin = false;

    try
    {
        cimClass = client.getClass(
            nameSpace,
            CIMName(className),
            localOnly,
            includeQualifiers,
            includeClassOrigin);
    }
    catch (Exception& ex)
    {
        const String msg = "Get Class failed. " + ex.getMessage();
        throw CIMOperationFailedException( msg );
    }

    // copy the super classes
    Array<CIMClass> superClasses;
    _copySuperClasses(client, nameSpace, cimClass, superClasses);

    // disconnect from the server
    client.disconnect();

    // now we can copy the base class
    try
    {
        _repository->createClass(nameSpace, cimClass);
    }
    catch (Exception& ex)
    {
        const String msg = "Create Class failed. " + ex.getMessage();
        throw CIMOperationFailedException(msg);
    }
}

/***************************************************************************
   _copySuperClasses
***************************************************************************/

void DefaultInstanceProvider::_copySuperClasses(
    CIMClient& client,
    const String& nameSpace,
    const CIMClass& cimClass,
    Array<CIMClass>& superClasses)
{
    // get the super class name
    CIMName superClassName = cimClass.getSuperClassName();

    if (superClassName.isNull())
    {
        Uint32 numSuperClasses = superClasses.size();
        if (numSuperClasses == 0)
        {
            // No super class, just return
            return;
        }

        // copy the super classes
        for (Uint32 i = numSuperClasses; i > 0; i--)
        {
            // check to see if class already exists
            //
            CIMClass superClass;
            try
            {
                superClass = _repository->getClass(
                    nameSpace, superClasses[i-1].getClassName());
            }
            catch (Exception&)
            {
                //
                // Super class does not exist, create the super class
                //
                try
                {
                    _repository->createClass(nameSpace, superClasses[i-1]);
                }
                catch (Exception& ex)
                {
                    const String msg = "Create superClass failed. " +
                        ex.getMessage();
                    throw CIMOperationFailedException(msg);
                }
            }
        }
        return;
    }

    // get the super class
    CIMClass superClass;
    Boolean localOnly = false;
    Boolean includeQualifiers = true;
    Boolean includeClassOrigin = false;

    try
    {
        superClass = client.getClass(
            nameSpace,
            superClassName,
            localOnly,
            includeQualifiers,
            includeClassOrigin);

        // add superclass to array
        superClasses.append(superClass);
    }
    catch (Exception& ex)
    {
        const String msg = "Get Super Class failed. " + ex.getMessage();
        throw CIMOperationFailedException(msg);
    }

    // recursive call.  copy superclasses of this class
    _copySuperClasses(client, nameSpace, superClass, superClasses);
}

/***************************************************************************
/  _nameSpaceExists
/
/  Returns true if the namespace already exists.  Returns false otherwise.
***************************************************************************/

Boolean DefaultInstanceProvider::_nameSpaceExists(
    const CIMNamespaceName& nameSpace) const
{
    //
    //  Get list of namespaces in the test repository
    //
    Array <CIMNamespaceName> nameSpaceNames;

    nameSpaceNames = _repository->enumerateNameSpaces();

    //
    //  check for the existence of the specified namespace
    //
    for (Uint8 i = 0; i < nameSpaceNames.size (); i++)
    {
        if (nameSpaceNames[i] == nameSpace)
        {
            // namespace already exists in repository
            return true;
        }
    }

    // namespace not found
    return false;
}

/***************************************************************************
/  _copyNameSpace
/
/  Copies the specified namespace to the test repository.  The new namespace
/  will contain the same qualifiers as the specified namespace, class and
/  all its super classes.
***************************************************************************/

void DefaultInstanceProvider::_copyNameSpace(
    const String& nameSpace,
    const String& className)
{
    try
    {
        // create the new name space
        _repository->createNameSpace(nameSpace);
    }
    catch (Exception& ex)
    {
        const String msg = "Failed to create namespace: " + ex.getMessage();
        throw CIMOperationFailedException(msg);
    }

    CIMClient client;

    try
    {
        // connect to the CIM server as a client
        client.connectLocal();
    }
    catch (Exception& ex)
    {
        const String msg = "Connect failed. " + ex.getMessage();
        throw CIMOperationFailedException(msg);
    }

    //
    // copy the qualifiers from the specified namespace to the new one
    //
    try
    {
        Array<CIMQualifierDecl> quals = client.enumerateQualifiers(nameSpace);

        int size = quals.size();
        for (int i = 0; i < size; ++i)
        {
            try
            {
                _repository->setQualifier(nameSpace, quals[i]);
            }
            catch (Exception& ex)
            {
                const String msg = "Failed to copy qualifiers. " +
                    ex.getMessage();
                throw CIMOperationFailedException(msg);
            }
        }

        // disconnect from the server
        client.disconnect();
    }
    catch (Exception& ex)
    {
        const String msg = "Copy class failed. " + ex.getMessage();
        throw CIMOperationFailedException(msg);
    }
}

/***************************************************************************
   getInstance
***************************************************************************/

void DefaultInstanceProvider::getInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    CIMNamespaceName nameSpace = instanceReference.getNameSpace();

    // get the class name
    CIMName className = instanceReference.getClassName();

    CIMInstance cimInstance;

    // create the namespace if necessary
    if (!_nameSpaceExists(nameSpace))
    {
        _copyNameSpace(nameSpace.getString(), className.getString());
    }

    // check to see if class already exists
    // if not, copy the class
    //
    try
    {
        CIMClass cimClass = _repository->getClass(nameSpace, className);
    }
    catch (Exception&)
    {
        // class does not exist

        //
        // copy the class
        //
        _copyClass(nameSpace.getString(), className.getString());
    }

    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    try
    {
        cimInstance = _repository->getInstance(
            nameSpace,
            localReference,
            includeQualifiers,
            includeClassOrigin,
            propertyList);
    }
    catch (Exception& ex)
    {
        const String msg = "Get Instance failed. " + ex.getMessage();
        throw CIMOperationFailedException( msg );
    }

    // begin processing the request
    handler.processing();

    // deliver requested instance
    handler.deliver(cimInstance);

    // complete processing the request
    handler.complete();
}

/***************************************************************************
   enumerateInstances
***************************************************************************/

void DefaultInstanceProvider::enumerateInstances(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    CIMNamespaceName nameSpace = classReference.getNameSpace();

    // get the class name
    CIMName className = classReference.getClassName();

    // create the namespace if necessary
    if (!_nameSpaceExists(nameSpace))
    {
        _copyNameSpace(nameSpace.getString(), className.getString());
    }

    // check to see if class already exists
    // if not, copy the class
    //
    try
    {
        CIMClass cimClass = _repository->getClass(nameSpace, className);
    }
    catch (Exception&)
    {
        // class does not exist

        //
        // copy the class
        //
        _copyClass(nameSpace.getString(), className.getString());
    }

    Array<CIMInstance> cimNamedInstances;

    try
    {
        cimNamedInstances = _repository->enumerateInstancesForClass(
            nameSpace,
            className,
            includeQualifiers,
            includeClassOrigin,
            propertyList);
    }
    catch (Exception& ex)
    {
        const String msg = "Enumerate Instances failed. " + ex.getMessage();
        throw CIMOperationFailedException( msg );
    }

    // begin processing the request
    handler.processing();

    for (Uint32 i = 0, n = cimNamedInstances.size(); i < n; i++)
    {
        // deliver instance
        handler.deliver(cimNamedInstances[i]);
    }

    // complete processing the request
    handler.complete();
}

/***************************************************************************
   enumerateInstanceNames
***************************************************************************/

void DefaultInstanceProvider::enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    ObjectPathResponseHandler& handler)
{
    CIMNamespaceName nameSpace = classReference.getNameSpace();
    CIMName className = classReference.getClassName();

    // create the namespace if necessary
    if (!_nameSpaceExists(nameSpace))
    {
        _copyNameSpace(nameSpace.getString(), className.getString());
    }

    // check to see if class already exists
    // if not, copy the class
    //
    try
    {
        CIMClass cimClass = _repository->getClass(nameSpace, className);
    }
    catch (Exception&)
    {
        // class does not exist

        //
        // copy the class
        //
        _copyClass(nameSpace.getString(), className.getString());
    }

    Array<CIMObjectPath> instanceNames;

    try
    {
        instanceNames = _repository->enumerateInstanceNamesForClass(
            nameSpace, className);
    }
    catch (Exception& ex)
    {
        const String msg = "Enumerate InstanceNames failed. " + ex.getMessage();
        throw CIMOperationFailedException(msg);
    }

    // begin processing the request
    handler.processing();

    for (Uint32 i = 0, n = instanceNames.size(); i < n; i++)
    {
        // deliver reference
        handler.deliver(instanceNames[i]);
    }

    // complete processing the request
    handler.complete();
}

/***************************************************************************
   modifyInstance
***************************************************************************/

void DefaultInstanceProvider::modifyInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler& handler)
{
    CIMNamespaceName nameSpace = instanceReference.getNameSpace();

    // get the class name
    CIMName className = instanceReference.getClassName();

    Array<CIMObjectPath> instanceNames;

    // create the namespace if necessary
    if (!_nameSpaceExists(nameSpace))
    {
        _copyNameSpace(nameSpace.getString(), className.getString());
    }

    // check to see if class already exists
    // if not, copy the class
    //
    try
    {
        CIMClass cimClass = _repository->getClass(nameSpace, className);
    }
    catch (Exception&)
    {
        // class does not exist

        //
        // copy the class
        //
        _copyClass(nameSpace.getString(), className.getString());
    }

    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    try
    {
       _repository->modifyInstance(
            nameSpace,
            instanceObject,
            includeQualifiers,
            propertyList);
    }
    catch (Exception& ex)
    {
        const String msg = "Modify Instance failed. " + ex.getMessage();
        throw CIMOperationFailedException( msg );
    }

    // begin processing the request
    handler.processing();

    // complete processing the request
    handler.complete();
}

/***************************************************************************
   createInstance
***************************************************************************/

void DefaultInstanceProvider::createInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    ObjectPathResponseHandler& handler)
{
    CIMNamespaceName nameSpace = instanceReference.getNameSpace();

    // get the class name
    CIMName className = instanceReference.getClassName();

    // create the namespace if necessary
    if (!_nameSpaceExists(nameSpace))
    {
        _copyNameSpace(nameSpace.getString(), className.getString());
    }

    // check to see if class already exists
    // if not, copy the class
    //
    try
    {
        CIMClass cimClass = _repository->getClass(nameSpace, className);
    }
    catch (Exception&)
    {
        // class does not exist

        //
        // copy the class
        //
        _copyClass(nameSpace.getString(), className.getString());
    }

    CIMObjectPath cimRef;

    // begin processing the request
    handler.processing();

    try
    {
        cimRef = _repository->createInstance(nameSpace, instanceObject);
    }
    catch (Exception& ex)
    {
        const String msg = "create Instance failed. " + ex.getMessage();
        throw CIMOperationFailedException( msg );
    }

    // deliver the new instance
    handler.deliver(cimRef);

    // complete processing the request
    handler.complete();
}

/***************************************************************************
   deleteInstance
***************************************************************************/

void DefaultInstanceProvider::deleteInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    ResponseHandler& handler)
{
    CIMNamespaceName nameSpace = instanceReference.getNameSpace();

    // get the class name
    CIMName className = instanceReference.getClassName();

    // create the namespace if necessary
    if (!_nameSpaceExists(nameSpace))
    {
        _copyNameSpace(nameSpace.getString(), className.getString());
    }

    // check to see if class already exists
    // if not, copy the class
    //
    try
    {
        CIMClass cimClass = _repository->getClass(nameSpace, className);
    }
    catch (Exception&)
    {
        // Class does not exist.  Copy the class.
        //
        _copyClass(nameSpace.getString(), className.getString());
    }

    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    // begin processing the request
    handler.processing();

    try
    {
       _repository->deleteInstance(nameSpace, localReference);
    }
    catch (Exception& ex)
    {
        const String msg = "delete Instance failed. " + ex.getMessage();
        throw CIMOperationFailedException( msg );
    }

    // complete processing the request
    handler.complete();
}

PEGASUS_NAMESPACE_END
