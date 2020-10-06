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

#ifndef Pegasus_FileBasedStore_h
#define Pegasus_FileBasedStore_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Repository/PersistentStore.h>
#include <Pegasus/Repository/PersistentStoreData.h>
#include <Pegasus/Repository/AssocClassTable.h>
#include <Pegasus/Repository/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_REPOSITORY_LINKAGE FileBasedStore : public PersistentStore
{
public:

    static Boolean isExistingRepository(const String& repositoryRoot);

    FileBasedStore(
        const String& repositoryPath,
        ObjectStreamer* streamer,
        Boolean compressMode);

    ~FileBasedStore();

    Boolean storeCompleteClassDefinitions()
    {
        return _storeCompleteClasses;
    }

    Array<NamespaceDefinition> enumerateNameSpaces();
    void createNameSpace(
        const CIMNamespaceName& nameSpace,
        Boolean shareable,
        Boolean updatesAllowed,
        const String& parent,
        const String& remoteInfo);

    void modifyNameSpace(
        const CIMNamespaceName& nameSpace,
        Boolean shareable,
        Boolean updatesAllowed);

    void modifyNameSpaceName(
        const CIMNamespaceName& nameSpace,
        const CIMNamespaceName& newNameSpaceName);

    void deleteNameSpace(const CIMNamespaceName& nameSpace);
    Boolean isNameSpaceEmpty(const CIMNamespaceName& nameSpace);

    Array<CIMQualifierDecl> enumerateQualifiers(
        const CIMNamespaceName& nameSpace);
    /**
        Gets a qualifier declaration for a specified qualifier name in a
        specified namespace.  Returns an uninitialized object if the qualifier
        is not found.
    */
    CIMQualifierDecl getQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName);
    void setQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMQualifierDecl& qualifierDecl);
    void deleteQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName);

    Array<Pair<String, String> > enumerateClassNames(
        const CIMNamespaceName& nameSpace);
    CIMClass getClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const CIMName& superClassName);
    /**
        Creates a class definition.  If the class is an association class,
        the class association entries are also added.
    */
    void createClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& newClass,
        const Array<ClassAssociation>& classAssocEntries);
    /**
        Modifies a class definition.  If the class is an association class,
        the class association entries are also updated to the specified set.
    */
    void modifyClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& modifiedClass,
        const CIMName& oldSuperClassName,
        Boolean isAssociation,
        const Array<ClassAssociation>& classAssocEntries);
    /**
        Deletes a class definition.  If the class is an association class,
        the class association entries are also deleted.  It is expected to
        have already been verified that no instances of this class exist.  A
        list of dependent namespace names is provided to allow appropriate
        clean-up of instance files, if necessary.
    */
    void deleteClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const CIMName& superClassName,
        Boolean isAssociation,
        const Array<CIMNamespaceName>& dependentNameSpaceNames);

    Array<CIMObjectPath> enumerateInstanceNamesForClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className);
    Array<CIMInstance> enumerateInstancesForClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className);
    CIMInstance getInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName);
    /**
        Creates an instance definition.  If it is an association instance,
        the instance association entries are also added.
    */
    void createInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMInstance& cimInstance,
        const Array<InstanceAssociation>& instAssocEntries);
    void modifyInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMInstance& cimInstance);
    /**
        Deletes an instance definition.  If it is an association instance,
        the instance association entries are also deleted.
    */
    void deleteInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName);
    Boolean instanceExists(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName);

    void getClassAssociatorNames(
        const CIMNamespaceName& nameSpace,
        const Array<CIMName>& classList,
        const Array<CIMName>& assocClassList,
        const Array<CIMName>& resultClassList,
        const String& role,
        const String& resultRole,
        Array<String>& associatorNames);
    void getClassReferenceNames(
        const CIMNamespaceName& nameSpace,
        const Array<CIMName>& classList,
        const Array<CIMName>& resultClassList,
        const String& role,
        Array<String>& referenceNames);

    void getInstanceAssociatorNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const Array<CIMName>& assocClassList,
        const Array<CIMName>& resultClassList,
        const String& role,
        const String& resultRole,
        Array<String>& associatorNames);
    void getInstanceReferenceNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const Array<CIMName>& resultClassList,
        const String& role,
        Array<String>& referenceNames);

private:

    void _rollbackIncompleteTransactions();

    /**
        Searches for state file in the "instance" directory of all
        namespaces.
        i)   Removes the rollback files to void a begin operation.
        ii)  Removes the rollback files to complete a commit operation.
        iii) Restores instance index and data files complete a
             rollback operation.
        If no state files are present, this method returns false
    */
    Boolean _completeTransactions();

    /**
        Converts a namespace name into a directory path.  The specified
        namespace name is not required to match the case of the namespace
        name that was originally created.

        @param nameSpace The namespace for which to determine the directory
            path.
        @return A string containing the directory path for the namespace.
     */
    String _getNameSpaceDirPath(const CIMNamespaceName& nameSpace) const;

    /** Returns the path of the qualifier file.

        @param   nameSpace      the namespace of the qualifier
        @param   qualifierName  the name of the qualifier

        @return  a string containing the qualifier file path
     */
    String _getQualifierFilePath(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName) const;

    /** Returns the path of the class file.

        @param   nameSpace  the namespace of the class
        @param   className  the name of the class

        @return  a string containing the class file path
     */
    String _getClassFilePath(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const CIMName& superClassName) const;

    /** Returns the path of the instance index file.

        @param   nameSpace      the namespace of the instance
        @param   className      the name of the class

        @return  a string containing the index file path
     */
    String _getInstanceIndexFilePath(
        const CIMNamespaceName& nameSpace,
        const CIMName& className) const;

    /** Returns the path of the instance file.

        @param   nameSpace      the namespace of the instance
        @param   className      the name of the class

        @return  a string containing the instance file path
     */
    String _getInstanceDataFilePath(
        const CIMNamespaceName& nameSpace,
        const CIMName& className) const;

    /** Returns the path of the class association file.

        @param   nameSpace      the namespace of the associations

        @return  a string containing the class association file path
     */
    String _getAssocClassPath(const CIMNamespaceName& nameSpace) const;

    /** Returns the path of the instance association file.

        @param   nameSpace      the namespace of the associations

        @return  a string containing the instance association file path
     */
    String _getAssocInstPath(const CIMNamespaceName& nameSpace) const;

    Boolean _loadInstance(
        const String& path,
        CIMInstance& object,
        Uint32 index,
        Uint32 size);

    /** loads all the instance objects from disk to memeory.  Returns true
        on success.

        @param   nameSpace      the namespace of the instances to be loaded
        @param   className      the class of the instances to be loaded
        @param   namedInstances an array of CIMInstance objects to which
                                the loaded instances are appended

        @return  true      if successful
                 false     if an error occurs in loading the instances
     */
    Boolean _loadAllInstances(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Array<CIMInstance>& namedInstances);

    void _addClassAssociationEntries(
        const CIMNamespaceName& nameSpace,
        const Array<ClassAssociation>& classAssocEntries);
    void _removeClassAssociationEntries(
        const CIMNamespaceName& nameSpace,
        const CIMName& assocClassName);

    void _addInstanceAssociationEntries(
        const CIMNamespaceName& nameSpace,
        const Array<InstanceAssociation>& instanceAssocEntries);
    void _removeInstanceAssociationEntries(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& assocInstanceName);
    void _SaveObject(const String& path,
        Buffer& objectXml);

    String _repositoryPath;
    ObjectStreamer* _streamer;
#ifdef PEGASUS_ENABLE_COMPRESSED_REPOSITORY
    Boolean _compressMode;
#endif
    Boolean _storeCompleteClasses;

    /**
        Maps namespace names to directory paths
    */
    HashTable<String, String, EqualNoCaseFunc, HashLowerCaseFunc>
        _nameSpacePathTable;

    /**
        This table must be managed dynamically per repository because it
        caches class association data in addition to handling persistent
        storage and lookup.
    */
    AssocClassTable _assocClassTable;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_FileBasedStore_h */
