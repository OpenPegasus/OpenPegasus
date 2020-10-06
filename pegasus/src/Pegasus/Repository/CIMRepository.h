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

#ifndef Pegasus_Repository_h
#define Pegasus_Repository_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMQualifierDecl.h>
#include <Pegasus/Common/ReadWriteSem.h>

#include <Pegasus/Config/ConfigManager.h>

#include <Pegasus/Repository/Linkage.h>
#include <Pegasus/Repository/NameSpaceManager.h>
#include <Pegasus/Repository/ObjectStreamer.h>

PEGASUS_NAMESPACE_BEGIN

class RepositoryDeclContext;
class compilerDeclContext;

/** This class provides a simple implementation of a CIM repository.
    Concurrent access is controlled by an internal lock.
*/
class PEGASUS_REPOSITORY_LINKAGE CIMRepository
{
public:

    enum CIMRepositoryMode
    {
        MODE_DEFAULT = 0,
        MODE_XML = 1,
        MODE_BIN = 2,
        MODE_COMPRESSED = 4
    };

    /// Constructor
    CIMRepository(
        const String& repositoryRoot,
        Uint32 mode = CIMRepository::MODE_DEFAULT,
        RepositoryDeclContext* declContext = 0);

    /// Descructor
    ~CIMRepository();

    /// getClass
    CIMClass getClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean localOnly = true,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList());

    // getFullConstClass
    CIMConstClass getFullConstClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className);

    /// getInstance
    CIMInstance getInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList());

    /// deleteClass
    void deleteClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className);

    /// deleteInstance
    void deleteInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName);

    /// createClass
    void createClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& newClass);

    /// createInstance
    CIMObjectPath createInstance(
        const CIMNamespaceName& nameSpace,
        const CIMInstance& newInstance);

    /// modifyClass
    void modifyClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& modifiedClass);

    /// modifyInstance
    void modifyInstance(
        const CIMNamespaceName& nameSpace,
        const CIMInstance& modifiedInstance,
        Boolean includeQualifiers = true,
        const CIMPropertyList& propertyList = CIMPropertyList());

    /// enumerateClasses
    Array<CIMClass> enumerateClasses(
        const CIMNamespaceName& nameSpace,
        const CIMName& className = CIMName(),
        Boolean deepInheritance = false,
        Boolean localOnly = true,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = false);

    /// enumerateClassNames
    Array<CIMName> enumerateClassNames(
        const CIMNamespaceName& nameSpace,
        const CIMName& className = CIMName(),
        Boolean deepInheritance = false);

    /**
        Enumerates the instances of the specified class and its subclasses.
        This method mimics the client behavior for the EnumerateInstances
        operation, but of course it can only return the instances that reside
        in the repository.  This method does not perform deepInheritance
        filtering.

        This method is useful mainly for testing purposes, and should not be
        relied upon for complete results in a CIM Server environment.
    */
    Array<CIMInstance> enumerateInstancesForSubtree(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList());

    /**
        Enumerates the instances of just the specified class.
        This method mimics the provider behavior for the EnumerateInstances
        operation.
    */
    Array<CIMInstance> enumerateInstancesForClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList());


    /**
        Enumerates the names of the instances of the specified class and its
        subclasses.  This method mimics the client behavior for the
        EnumerateInstanceNames operation, but of course it can only return
        the names of the instances that reside in the repository.

        This method is useful mainly for testing purposes, and should not be
        relied upon for complete results in a CIM Server environment.

        @param nameSpace The namespace in which className resides.
        @param className The name the class for which to retrieve the instance
            names.
        @return An Array of CIMObjectPath objects containing the names of the
            instances of the specified class in the specified namespace.
    */
    Array<CIMObjectPath> enumerateInstanceNamesForSubtree(
        const CIMNamespaceName& nameSpace,
        const CIMName& className);

    /**
        Enumerates the names of the instances of just the specified class.
        This method mimics the provider behavior for the EnumerateInstanceNames
        operation.

        @param nameSpace The namespace in which className resides.
        @param className The name the class for which to retrieve the instance
            names.
        @return An Array of CIMObjectPath objects containing the names of the
            instances of the specified class in the specified namespace.
    */
    Array<CIMObjectPath> enumerateInstanceNamesForClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className);

    /**
        Get the associated(reference) classes or instances for the
        input ObjectName filtered by the assocClass,resultClass,
        role and result role parameters. This is analogous to the
        operation defined in the DMTF spec DSP200.
        @param nameSpace CIMNamespaceName for the operation
        @param objectName CIMObjectPath for the operation.  If this includes
        ONLY a class in the object with no keys the return is CIMClasses. Else
        it is CIMInstances. See bug 3302
        @param assocClass CIMName with name of association class for which this
        is to be filtered or Null if no filtering
        @param resultClass CIMName with name of associated class for which
        response is to be filtered or Null of no filtering.
        @param role String defining role parameter from association class to
        objectName
        @param resultRole String defining role between association and
        associated classes.
        @param includeQualifiers Boolean to force inclusion of Qualifiers if
        true and if this is a class request.
        @param includeClassOrigin Boolean to force inclusion of ClassOrigin
        information if true
        @param propertyList CIMPropertyList (optional). if Null,
        return all properties. If empty but not Null, return no
        properties. Else return only properties in the list.
        @return Array<CIMObject> containing either the classes or
                instances requested.
        @exception CIMException Error code CIM_ERR_INVALID_CLASS if the class
            does not exist.
        @exception - Throws NoSuchDirectory if the Namespace
                       does not exist.
    */
    Array<CIMObject> associators(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass = CIMName(),
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList());

    /**
        Get the associated class or instance object paths for the
        input ObjectName filtered by the assocClass,resultClass,
        role and result role parameters. This is analogous to the
        operation defined in the DMTF spec DSP200.
        @param nameSpace CIMNamespaceName for the operation
        @param objectName CIMObjectPath for the operation.  If this includes
        ONLY a class in the object with no keys the return is CIMClasses. Else
        it is CIMInstances. See bug 3302
        @param assocClass CIMName with name of association class for which this
        is to be filtered or Null if no filtering
        @param resultClass CIMName with name of associated class for which
        response is to be filtered or Null of no filtering.
        @param role String defining role parameter from association class to
        objectName
        @param resultRole String defining role between association and
        associated classes.
        @return Array<CIMObjectPath> containing  the path of either
                classes or instances requested.
        @exception CIMException Error code CIM_ERR_INVALID_CLASS if the class
            does not exist.
        @exception - Throws NoSuchDirectory if the Namespace
                       does not exist.        
    */
    Array<CIMObjectPath> associatorNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass = CIMName(),
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY);

    /**
        Get the association classes or instances for the input
        ObjectName filtered by the resultClass and role parameters.
        This is analogous to the operation defined in the DMTF spec
        DSP200.
        @param nameSpace CIMNamespaceName for the operation
        @param objectName CIMObjectPath for the operation.  If this includes
        ONLY a class in the object with no keys the return is CIMClasses. Else
        it is CIMInstances. See bug 3302
        @param role String defining role parameter from association class to
        objectName
        @param resultRole String defining role between association and
        associated classes.
        @param includeQualifiers Boolean to force inclusion of Qualifiers if
        true and if this is a class request.
        @param includeClassOrigin Boolean to force inclusion of ClassOrigin
        information if true
        @param propertyList CIMPropertyList (optional). if Null,
        return all properties. If empty but not Null, return no
        properties. Else return only properties in the list.
        @return Array<CIMObject> containing either the classes or
                instances requested.
        @exception CIMException Error code CIM_ERR_INVALID_CLASS if the class
            does not exist.
        @exception - Throws NoSuchDirectory if the Namespace does
            not exist. 
    */
    Array<CIMObject> references(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList());

    /**
        Get the association class or instance object paths for the
        input ObjectName filtered by the assocClass,resultClass,
        role and result role parameters. This is analogous to the
        operation defined in the DMTF spec DSP200.
        @param nameSpace CIMNamespaceName for the operation
        @param objectName CIMObjectPath for the operation.  If this includes
        ONLY a class in the object with no keys the return is CIMClasses. Else
        it is CIMInstances. See bug 3302
        @param role String defining role parameter from association class to
        objectName
        @param resultRole String defining role between association and
        associated classes.
        @return Array<CIMObjectPath> containing  the path of either
                classes or instances requested.
        @exception CIMException Error code CIM_ERR_INVALID_CLASS if the class
            does not exist.
        @exception - Throws NoSuchDirectory if the Namespace
                       does not exist.        
    */
    Array<CIMObjectPath> referenceNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY);

    /// getProperty
    CIMValue getProperty(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& propertyName);

    /// setProperty
    void setProperty(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& propertyName,
        const CIMValue& newValue = CIMValue());

    /// getQualifier
    CIMQualifierDecl getQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName);

    /// setQualifier
    void setQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMQualifierDecl& qualifierDecl);

    /// deleteQualifier
    void deleteQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName);

    /// enumerateQualifiers
    Array<CIMQualifierDecl> enumerateQualifiers(
        const CIMNamespaceName& nameSpace);

    typedef HashTable <String, String, EqualNoCaseFunc, HashLowerCaseFunc>
        NameSpaceAttributes;

    void createNameSpace(const CIMNamespaceName& nameSpace,
        const NameSpaceAttributes& attributes = NameSpaceAttributes());

    void modifyNameSpace(const CIMNamespaceName& nameSpace,
        const NameSpaceAttributes& attributes = NameSpaceAttributes());

    void modifyNameSpaceName(const CIMNamespaceName& nameSpace,
        const CIMNamespaceName& newNameSpaceName);

    Array<CIMNamespaceName> enumerateNameSpaces() const;

    /** Deletes a namespace in the repository.
        The deleteNameSpace method will only delete a namespace if there are
        no classed defined in the namespace.  Today this is a Pegasus
        characteristics and not defined as part of the DMTF standards.
        @param String with the name of the namespace
        @exception - Throws NoSuchDirectory if the Namespace does not exist.
    */
    void deleteNameSpace(const CIMNamespaceName& nameSpace);

    Boolean getNameSpaceAttributes(
        const CIMNamespaceName& nameSpace,
        NameSpaceAttributes& attributes);

    Boolean nameSpaceExists(const CIMNamespaceName& nameSpaceName);

    ////////////////////////////////////////////////////////////////////////////

    /** Indicates whether instance operations that do not have a provider
        registered should be served by this repository.
    */
    Boolean isDefaultInstanceProvider();

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
        Array<CIMName>& subClassNames) const;

    /** Get the names of all superclasses (direct and indirect) of this
        class.
    */
    void getSuperClassNames(
        const CIMNamespaceName& nameSpaceName,
        const CIMName& className,
        Array<CIMName>& subClassNames) const;

    Boolean isRemoteNameSpace(
        const CIMNamespaceName& nameSpaceName,
        String& remoteInfo);

#ifdef PEGASUS_DEBUG
    void DisplayCacheStatistics();
#endif

protected:

    // Internal getClass implementation that does not do access control
    // If readOnlyClass is true, then the caller ensures that the returned
    // class, will never be modified, which allows returning a reference to
    // the one that is in the cache.
    CIMClass _getClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean localOnly,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        Boolean clone = true);

    /// Internal getInstance implementation that does not do access control
    CIMInstance _getInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        Boolean resolveInstance);

    /// Internal createClass implementation that does not do access control
    void _createClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& newClass);

    /// Internal createInstance implementation that does not do access control
    CIMObjectPath _createInstance(
        const CIMNamespaceName& nameSpace,
        const CIMInstance& newInstance);

    /// Internal modifyClass implementation that does not do access control
    void _modifyClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& modifiedClass);

    /// Internal associatorNames implementation that does not do access control
    Array<CIMObjectPath> _associatorNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole);

    /// Internal referenceNames implementation that does not do access control
    Array<CIMObjectPath> _referenceNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role);

    /// Internal getQualifier implementation that does not do access control
    CIMQualifierDecl _getQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName);

    /// Internal setQualifier implementation that does not do access control
    void _setQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMQualifierDecl& qualifierDecl);

private:

    class CIMRepositoryRep* _rep;
    friend class compilerDeclContext;
    friend class RepositoryDeclContext;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Repository_h */
