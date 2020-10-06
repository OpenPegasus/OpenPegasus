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

#ifndef Pegasus_NameSpaceManager_h
#define Pegasus_NameSpaceManager_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Pair.h>
#include <Pegasus/Repository/InheritanceTree.h>
#include <Pegasus/Repository/PersistentStoreData.h>
#include <Pegasus/Repository/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

struct NameSpaceManagerRep;
class NameSpace;

/** The NameSpaceManager class manages a collection of NameSpace objects.

    The shared schema support is based on these tenets:

    1.  A primary namespace is a namespace with no parent namespace.
    2.  A secondary namespace is a namespace with a parent namespace.
    3.  A secondary namespace derives all the schema from its parent namespace.
    4.  A primary namespace is not read-only.
    5.  A read-only namespace contains no schema of its own, but it may
        contain instances.
    6.  The parent of a secondary read-write namespace must be a primary
        namespace.
*/
class PEGASUS_REPOSITORY_LINKAGE NameSpaceManager
{
public:

    /** Constructor.
    */
    NameSpaceManager();

    /** Destructor.
    */
    ~NameSpaceManager();

    /** Initializes the namespace definition in the NameSpaceManager.  If the
        namespace has a parent namespace, the the caller MUST ensure that the
        parent namespace is already initialized.
        @param nameSpace The namespace definition to initialize.
        @param classList An Array of class names and superclass names that are
            defined in the namespace.
    */
    void initializeNameSpace(
        const NamespaceDefinition& nameSpace,
        const Array<Pair<String, String> >& classList);

    /** Indicates whether the specified namespace exists.
        @param nameSpaceName name of namespace.
        @return true if namespace exists; false otherwise.
    */
    Boolean nameSpaceExists(const CIMNamespaceName& nameSpaceName) const;

    /** Creates the given namespace.
        @param nameSpaceName name of namespace to be created.
    */
    void createNameSpace(
        const CIMNamespaceName& nameSpaceName,
        Boolean shareable,
        Boolean updatesAllowed,
        const String& parent,
        const String& remoteInfo = String::EMPTY);

    void modifyNameSpace(
        const CIMNamespaceName& nameSpaceName,
        Boolean shareable,
        Boolean updatesAllowed);

    void modifyNameSpaceName(
        const CIMNamespaceName& nameSpaceName,
        const CIMNamespaceName& newNameSpaceName);
    
    /** Deletes the given namespace.
        @param nameSpaceName name of namespace to be deleted.
        @exception CIMException(CIM_ERR_INVALID_NAMESPACE)
        @exception NonEmptyNameSpace
        @exception FailedToRemoveDirectory
    */
    void deleteNameSpace(const CIMNamespaceName& nameSpaceName);

    Boolean isRemoteNameSpace(
        const CIMNamespaceName& nameSpaceName,
        String& remoteInfo);

    /** Gets array of all namespace names.
        @param nameSpaceNames filled with names of all namespaces.
    */
    void getNameSpaceNames(Array<CIMNamespaceName>& nameSpaceNames) const;

    Boolean getNameSpaceAttributes(
        const CIMNamespaceName& nameSpace,
        Boolean& shareable,
        Boolean& updatesAllowed,
        String& parent,
        String& remoteInfo);

    void validateNameSpace(
        const CIMNamespaceName& nameSpaceName) const;

    /** Gets a list of names of namespaces that are directly dependent on the
        specified namespace.  The specified namespace is also included in the
        list.  The list contains all derived read-write namespaces (and
        perhaps some read-only ones as well), but not necessarily all
        derived read-only namespaces.  This makes it insufficient to find all
        instances of a given class in derived namespaces, as it is currently
        being used.
        @param nameSpaceName name of the origin namespace.
        @return An Array of namespace names that depend on the origin namespace.
    */
    Array<CIMNamespaceName> getDependentSchemaNameSpaceNames(
        const CIMNamespaceName& nameSpaceName) const;

    /** Determines whether a specified namespace has one or more dependent
        namespaces.  If so, the name of one of the dependents is returned.
        @param nameSpaceName Name of the namespace to check for dependents.
        @param nameSpaceName (Output) Name of a dependent namespace, if found.
        @return A Boolean indicating whether the namespace has a dependent
            namespace.
    */
    Boolean hasDependentNameSpace(
        const CIMNamespaceName& nameSpaceName,
        CIMNamespaceName& dependentNameSpaceName) const;

    /** Lists the names of namespaces whose schema is accessible from the
        specified namespace.  The list contains all R/W parent namespaces
        of the specified namespace, as well as the specified namespace if it
        is R/W.  Since a R/W namespace may depend only on a primary namespace,
        the maximum length of the returned list is 2.
        @param nameSpaceName name of the origin namespace.
        @return An Array of namespace names whose schema is accessible from
            the specified namespace.
    */
    Array<CIMNamespaceName> getSchemaNameSpaceNames(
        const CIMNamespaceName& nameSpaceName) const;

    /**
        Validates that the specified class exists in the specified namespace
        (or one of its parent namespaces).  It is intended for use on instance
        operations.
        @param nameSpaceName The name of the namespace to check for the class.
        @param className The name of the class for which to validate existence.
        @exception CIMException Error code CIM_ERR_INVALID_CLASS if the class
            does not exist.
    */
    void validateClass(
        const CIMNamespaceName& nameSpaceName,
        const CIMName& className) const;

    CIMName getSuperClassName(
        const CIMNamespaceName& nameSpaceName,
        const CIMName& className) const;

    void locateClass(
        const CIMNamespaceName& nameSpaceName,
        const CIMName& className,
        CIMNamespaceName& actualNameSpaceName,
        CIMName& superClassName) const;

    /** Check whether the specified class may be deleted
        @param nameSpaceName Namespace in which the class exists.
        @param className Name of class to be deleted.
        @exception CIMException If the class may not be deleted
    */
    void checkDeleteClass(
        const CIMNamespaceName& nameSpaceName,
        const CIMName& className) const;

    /** Check whether update to namespace allowed.
        @param nameSpaceName Namespace in which the qualifier exists.
        @exception CIMException If the updates not allowed
    */
    void checkNameSpaceUpdateAllowed(
        const CIMNamespaceName& nameSpaceName) const;

    /** Deletes the class file for the given class.
        @param nameSpaceName name of namespace.
        @param className name of class.
        @exception CIMException(CIM_ERR_INVALID_NAMESPACE)
        @exception CIMException(CIM_ERR_INVALID_CLASS)
        @exception CIMException(CIM_ERR_CLASS_HAS_CHILDREN)
    */
    void deleteClass(
        const CIMNamespaceName& nameSpaceName,
        const CIMName& className) const;

    /** Print out the namespaces. */
    void print(PEGASUS_STD(ostream)& os) const;

    /** Checks whether it is okay to create a new class.
        @param nameSpaceName namespace to contain class.
        @param className name of class
        @param superClassName name of superClassName
    */
    void checkCreateClass(
        const CIMNamespaceName& nameSpaceName,
        const CIMName& className,
        const CIMName& superClassName);

    /** Creates an entry for a new class.
        @param nameSpaceName namespace to contain class.
        @param className name of class
        @param superClassName name of superClassName
    */
    void createClass(
        const CIMNamespaceName& nameSpaceName,
        const CIMName& className,
        const CIMName& superClassName);

    /** Checks whether it is okay to modify this class.
        @param nameSpaceName namespace.
        @param className name of class being modified.
        @param superClassName superclass of class being modified.
        @param oldSuperClassName Output name of existing superclass of class
            being modified.
        @exception CIMException(CIM_ERR_INVALID_CLASS)
        @exception CIMException(CIM_ERR_FAILED) if there is an attempt
            to change the superclass of this class.
        @exception CIMException(CIM_ERR_CLASS_HAS_CHILDREN) if class
            has any children.
    */
    void checkModifyClass(
        const CIMNamespaceName& nameSpaceName,
        const CIMName& className,
        const CIMName& superClassName,
        CIMName& oldSuperClassName,
        Boolean allowNonLeafModification);

    /** Get subclass names of the given class in the given namespace.
        @param nameSpaceName
        @param className - class whose subclass names will be gotten. If
            className is empty, all classnames are returned.
        @param deepInheritance - if true all descendent classes of class
            are returned. If className is empty, only root classes are returned.
        @param subClassNames - output argument to hold subclass names.
        @exception CIMException(CIM_ERR_INVALID_CLASS)
    */
    void getSubClassNames(
        const CIMNamespaceName& nameSpaceName,
        const CIMName& className,
        Boolean deepInheritance,
        Array<CIMName>& subClassNames,
        Boolean enm=false) const;

    /** Get the names of all superclasses (direct and indirect) of this
        class.
    */
    void getSuperClassNames(
        const CIMNamespaceName& nameSpaceName,
        const CIMName& className,
        Array<CIMName>& subClassNames) const;

    Boolean classExists(
        NameSpace* nameSpace,
        const CIMName& className,
        Boolean throwExcp=false) const;

    Boolean classExists(
        const CIMNamespaceName& nameSpaceName,
        const CIMName& className) const;

private:

    NameSpace* _getNameSpace(const CIMNamespaceName& ns) const;
    NameSpace* _lookupNameSpace(const String& ns);

    NameSpaceManagerRep* _rep;
};

/** This exception is thrown if one attempts to remove a namespace that still
    contains classes, instances, or qualifier.
*/
class PEGASUS_REPOSITORY_LINKAGE NonEmptyNameSpace : public Exception
{
public:
    NonEmptyNameSpace(const String& nameSpaceName)
    : Exception(MessageLoaderParms(
          "Repository.NameSpaceManager.ATTEMPT_DELETE_NONEMPTY_NAMESPACE",
          "Attempt to delete a non-empty namespace: $0", nameSpaceName))
    {
    }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_NameSpaceManager_h */
