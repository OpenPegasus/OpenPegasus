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

#ifndef Pegasus_PersistentStore_h
#define Pegasus_PersistentStore_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMQualifierDecl.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/Pair.h>

#include <Pegasus/Repository/AutoStreamer.h>
#include <Pegasus/Repository/PersistentStoreData.h>

PEGASUS_NAMESPACE_BEGIN

class PersistentStore
{
public:
    static PersistentStore* createPersistentStore(
        const String& repositoryPath,
        ObjectStreamer* streamer,
        Boolean compressMode);

    virtual ~PersistentStore() { }

    virtual Boolean storeCompleteClassDefinitions() = 0;

    virtual Array<NamespaceDefinition> enumerateNameSpaces() = 0;
    virtual void createNameSpace(
        const CIMNamespaceName& nameSpace,
        Boolean shareable,
        Boolean updatesAllowed,
        const String& parent,
        const String& remoteInfo) = 0;
    virtual void modifyNameSpace(
        const CIMNamespaceName& nameSpace,
        Boolean shareable,
        Boolean updatesAllowed) = 0;
    virtual void modifyNameSpaceName(
        const CIMNamespaceName& nameSpace,
        const CIMNamespaceName& newNameSpaceName) = 0;
    virtual void deleteNameSpace(const CIMNamespaceName& nameSpace) = 0;
    virtual Boolean isNameSpaceEmpty(const CIMNamespaceName& nameSpace) = 0;

    virtual Array<CIMQualifierDecl> enumerateQualifiers(
        const CIMNamespaceName& nameSpace) = 0;
    /**
        Gets a qualifier declaration for a specified qualifier name in a
        specified namespace.  Returns an uninitialized object if the qualifier
        is not found.
    */
    virtual CIMQualifierDecl getQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName) = 0;
    virtual void setQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMQualifierDecl& qualifierDecl) = 0;
    virtual void deleteQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName) = 0;

    virtual Array<Pair<String, String> > enumerateClassNames(
        const CIMNamespaceName& nameSpace) = 0;
    virtual CIMClass getClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const CIMName& superClassName) = 0;
    /**
        Creates a class definition.  If the class is an association class,
        the class association entries are also added.
    */
    virtual void createClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& newClass,
        const Array<ClassAssociation>& classAssocEntries) = 0;
    /**
        Modifies a class definition.  If the class is an association class,
        the class association entries are also updated to the specified set.
    */
    virtual void modifyClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& modifiedClass,
        const CIMName& oldSuperClassName,
        Boolean isAssociation,
        const Array<ClassAssociation>& classAssocEntries) = 0;
    /**
        Deletes a class definition.  If the class is an association class,
        the class association entries are also deleted.  It is expected to
        have already been verified that no instances of this class exist.  A
        list of dependent namespace names is provided to allow appropriate
        clean-up of instance files, if necessary.
    */
    virtual void deleteClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const CIMName& superClassName,
        Boolean isAssociation,
        const Array<CIMNamespaceName>& dependentNameSpaceNames) = 0;

    virtual Array<CIMObjectPath> enumerateInstanceNamesForClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className) = 0;
    virtual Array<CIMInstance> enumerateInstancesForClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className) = 0;
    virtual CIMInstance getInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName) = 0;
    /**
        Creates an instance definition.  If it is an association instance,
        the instance association entries are also added.
    */
    virtual void createInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMInstance& cimInstance,
        const Array<InstanceAssociation>& instAssocEntries) = 0;
    virtual void modifyInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMInstance& cimInstance) = 0;
    /**
        Deletes an instance definition.  If it is an association instance,
        the instance association entries are also deleted.
    */
    virtual void deleteInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName) = 0;
    virtual Boolean instanceExists(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName) = 0;

    virtual void getClassAssociatorNames(
        const CIMNamespaceName& nameSpace,
        const Array<CIMName>& classList,
        const Array<CIMName>& assocClassList,
        const Array<CIMName>& resultClassList,
        const String& role,
        const String& resultRole,
        Array<String>& associatorNames) = 0;
    virtual void getClassReferenceNames(
        const CIMNamespaceName& nameSpace,
        const Array<CIMName>& classList,
        const Array<CIMName>& resultClassList,
        const String& role,
        Array<String>& referenceNames) = 0;

    virtual void getInstanceAssociatorNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const Array<CIMName>& assocClassList,
        const Array<CIMName>& resultClassList,
        const String& role,
        const String& resultRole,
        Array<String>& associatorNames) = 0;
    virtual void getInstanceReferenceNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const Array<CIMName>& resultClassList,
        const String& role,
        Array<String>& referenceNames) = 0;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_PersistentStore_h */
