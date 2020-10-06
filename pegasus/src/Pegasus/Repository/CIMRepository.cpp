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

#include <Pegasus/Common/Config.h>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <Pegasus/Common/InternalException.h>

#include <Pegasus/Common/DeclContext.h>
#include <Pegasus/Common/Resolver.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/ReadWriteSem.h>
#include <Pegasus/Common/SCMOClassCache.h>

#include <Pegasus/Repository/XmlStreamer.h>
#include <Pegasus/Repository/BinaryStreamer.h>
#include <Pegasus/Repository/AutoStreamer.h>

#include "CIMRepository.h"
#include "RepositoryDeclContext.h"
#include "ObjectCache.h"

#include "PersistentStore.h"

#if 0
#undef PEG_METHOD_ENTER
#undef PEG_METHOD_EXIT
#define PEG_METHOD_ENTER(x,y)  cout<<"--- Enter: "<<y<<endl;
#define PEG_METHOD_EXIT()
#endif


//==============================================================================
//
// The class cache caches up PEGASUS_CLASS_CACHE_SIZE fully resolved class
// definitions in memory.  To override the default, define
// PEGASUS_CLASS_CACHE_SIZE in your build environment.  To suppress the cache
// (and not compile it in at all), set PEGASUS_CLASS_CACHE_SIZE to 0.
//
//==============================================================================

#if !defined(PEGASUS_CLASS_CACHE_SIZE)
# define PEGASUS_CLASS_CACHE_SIZE 8
#endif

#if (PEGASUS_CLASS_CACHE_SIZE != 0)
# define PEGASUS_USE_CLASS_CACHE
#endif

#define PEGASUS_QUALIFIER_CACHE_SIZE 80


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

class CIMRepositoryRep
{
public:

    CIMRepositoryRep()
        :
#ifdef PEGASUS_USE_CLASS_CACHE
          _classCache(PEGASUS_CLASS_CACHE_SIZE),
#endif /* PEGASUS_USE_CLASS_CACHE */
          _qualifierCache(PEGASUS_QUALIFIER_CACHE_SIZE)
    {
    }

    /**
        Checks whether an instance with the specified key values exists in the
        class hierarchy of the specified class.

        @param   nameSpace      the namespace of the instance
        @param   instanceName   the name of the instance

        @return  true           if the instance is found
                 false          if the instance cannot be found
     */
    Boolean _checkInstanceAlreadyExists(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName) const;

    // This must be initialized in the constructor using values from the
    // ConfigManager.
    Boolean _isDefaultInstanceProvider;

    AutoPtr<ObjectStreamer> _streamer;

    AutoPtr<PersistentStore> _persistentStore;

    /**
        Indicates whether the class definitions in the persistent store are
        complete (contain propagated elements).
    */
    Boolean _storeCompleteClassDefinitions;

    NameSpaceManager _nameSpaceManager;

    ReadWriteSem _lock;

    RepositoryDeclContext* _context;

    CString _lockFile;

#ifdef PEGASUS_USE_CLASS_CACHE
    ObjectCache<CIMClass> _classCache;
#endif /* PEGASUS_USE_CLASS_CACHE */

    ObjectCache<CIMQualifierDecl> _qualifierCache;
};

static String _getCacheKey(
    const CIMNamespaceName& nameSpace,
    const CIMName& entryName)
{
    String key = nameSpace.getString();
    key.append(':');
    key.append(entryName.getString());
    return key;
}


//
//  The following _xx functions are local to the repository implementation
//
////////////////////////////////////////////////////////////////////////////////
//
//   _containsProperty
//
////////////////////////////////////////////////////////////////////////////////

/** Check to see if the specified property is in the property list
    @param property the specified property
    @param propertyList the property list
    @return true if the property is in the list otherwise false.
*/
static Boolean _containsProperty(
    const CIMProperty& property,
    const CIMPropertyList& propertyList)
{
    //  For each property in the propertly list
    for (Uint32 p=0; p<propertyList.size(); p++)
    {
        if (propertyList[p].equal(property.getName()))
            return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
// removeAllQualifiers - Remove all of the qualifiers from a class
//
////////////////////////////////////////////////////////////////////////////////

/* removes all Qualifiers from a CIMClass.  This function removes all
   of the qualifiers from the class, from all of the properties,
   from the methods, and from the parameters attached to the methods.
   @param cimClass reference to the class from which qualifiers are to
   be removed.
   NOTE: This would be logical to be moved to CIMClass since it may be
   more general than this usage.
*/
static void _removeAllQualifiers(CIMClass& cimClass)
{
    // remove qualifiers of the class
    Uint32 count = 0;
    while ((count = cimClass.getQualifierCount()) > 0)
        cimClass.removeQualifier(count - 1);

    // remove qualifiers from the properties
    for (Uint32 i = 0; i < cimClass.getPropertyCount(); i++)
    {
        CIMProperty p = cimClass.getProperty(i);
        count = 0;
        while ((count = p.getQualifierCount()) > 0)
            p.removeQualifier(count - 1);
    }

    // remove qualifiers from the methods
    for (Uint32 i = 0; i < cimClass.getMethodCount(); i++)
    {
        CIMMethod m = cimClass.getMethod(i);
        for (Uint32 j = 0 ; j < m.getParameterCount(); j++)
        {
            CIMParameter p = m.getParameter(j);
            count = 0;
            while ((count = p.getQualifierCount()) > 0)
                p.removeQualifier(count - 1);
        }
        count = 0;
        while ((count = m.getQualifierCount()) > 0)
            m.removeQualifier(count - 1);
    }
}

/////////////////////////////////////////////////////////////////////////
//
// _stripPropagatedElements
//
/////////////////////////////////////////////////////////////////////////

/* Removes propagated elements from the CIMClass, including properties,
   methods, and qualifiers attached to the class, properties, methods, and
   parameters.
*/
static void _stripPropagatedElements(CIMClass& cimClass)
{
    // Remove the propagated qualifiers from the class.
    // Work backwards because removal may be cheaper. Sint32 covers count=0
    for (Sint32 i = cimClass.getQualifierCount() - 1; i >= 0; i--)
    {
        if (cimClass.getQualifier(i).getPropagated())
        {
            cimClass.removeQualifier(i);
        }
    }

    // Remove the propagated properties.
    for (Sint32 i = cimClass.getPropertyCount() - 1; i >= 0; i--)
    {
        CIMProperty p = cimClass.getProperty(i);
        if (p.getPropagated())
        {
            cimClass.removeProperty(i);
        }
        else
        {
            // Remove the propagated qualifiers from the property.
            for (Sint32 j = p.getQualifierCount() - 1; j >= 0; j--)
            {
                if (p.getQualifier(j).getPropagated())
                {
                    p.removeQualifier(j);
                }
            }
        }
    }

    // Remove the propagated methods.
    for (Sint32 i = cimClass.getMethodCount() - 1; i >= 0; i--)
    {
        CIMMethod m = cimClass.getMethod(i);
        if (m.getPropagated())
        {
            cimClass.removeMethod(i);
        }
        else
        {
            // Remove the propagated qualifiers from the method.
            for (Sint32 j = m.getQualifierCount() - 1; j >= 0; j--)
            {
                if (m.getQualifier(j).getPropagated())
                {
                    m.removeQualifier(j);
                }
            }

            // Remove the propagated qualifiers from the method parameters.
            for (Sint32 j = m.getParameterCount() - 1; j >= 0; j--)
            {
                CIMParameter p = m.getParameter(j);
                for (Sint32 k = p.getQualifierCount() - 1; k >= 0; k--)
                {
                    if (p.getQualifier(k).getPropagated())
                    {
                        p.removeQualifier(k);
                    }
                }
            }
        }
    }
}

/* remove the properties from an instance based on attributes.
    @param Instance from which properties will be removed.
    @param propertyList PropertyList is used in the removal algorithm
    NOTE: This could be logical to move to CIMInstance since the
    usage is more general than just in the repository
*/
static void _removeProperties(
    CIMInstance& cimInstance,
    const CIMPropertyList& propertyList)
{
    if (!propertyList.isNull())
    {
        // Loop through properties to remove those that do not filter through
        // local only attribute and are not in the property list.
        // Work backwards because removal may be cheaper. Sint32 covers count=0
        for (Sint32 i = (cimInstance.getPropertyCount() - 1); i >= 0; i--)
        {
            // Since the propertyList is not NULL, only properties in the list
            // should be included in the instance.
            if (!_containsProperty(cimInstance.getProperty(i), propertyList))
                cimInstance.removeProperty(i);
        }
    }
}

/* remove all Qualifiers from a single CIMInstance. Removes
    all of the qualifiers from the instance and from properties
    within the instance.
    @param instance from which parameters are removed.
    NOTE: This could be logical to be moved to CIMInstance since
    the usage may be more general than just in the repository.
*/
static void _removeAllQualifiers(CIMInstance& cimInstance)
{
    // remove qualifiers from the instance
    Uint32 count = 0;
    while ((count = cimInstance.getQualifierCount()) > 0)
        cimInstance.removeQualifier(count - 1);

    // remove qualifiers from the properties
    for (Uint32 i = 0; i < cimInstance.getPropertyCount(); i++)
    {
        CIMProperty p = cimInstance.getProperty(i);
        count = 0;
        while ((count = p.getQualifierCount()) > 0)
            p.removeQualifier(count - 1);
    }
}

/* removes all ClassOrigin attributes from a single CIMInstance. Removes
    the classOrigin attribute from each property in the Instance.
   @param Instance from which the ClassOrigin Properties will be removed.
   NOTE: Logical to be moved to CIMInstance since it may be more general
   than just the repositoryl
*/
static void _removeClassOrigins(CIMInstance& cimInstance)
{
    PEG_TRACE_CSTRING(TRC_REPOSITORY, Tracer::LEVEL4, "Remove Class Origins");

    Uint32 propertyCount = cimInstance.getPropertyCount();
    for (Uint32 i = 0; i < propertyCount ; i++)
        cimInstance.getProperty(i).setClassOrigin(CIMName());
}

/* Filters the properties, qualifiers, and classorigin out of a single instance.
    Based on the parameters provided for propertyList, includeQualifiers,
    and includeClassOrigin, this function simply filters the properties
    qualifiers, and classOrigins out of a single instance.  This function
    was created to have a single piece of code that processes getinstance
    and enumerateInstances returns.
    @param cimInstance reference to instance to be processed.
    @param propertyList If not null, defines the properties to be included in
        the instance.
    @param includeQualifiers Boolean defining if qualifiers to be returned.
    @param includeClassOrigin Boolean defining if ClassOrigin attribute to
    be removed from properties.
*/
static void _filterInstance(
    CIMInstance& cimInstance,
    const CIMPropertyList& propertyList,
    Boolean includeQualifiers,
    Boolean includeClassOrigin)
{
    // Remove properties based on propertyList
    _removeProperties(cimInstance, propertyList);

    // If includequalifiers false, remove all qualifiers from
    // properties.

    if (!includeQualifiers)
    {
        _removeAllQualifiers(cimInstance);
    }

    // if ClassOrigin Flag false, remove classOrigin info from Instance object
    // by setting the classOrigin to Null.

    if (!includeClassOrigin)
    {
        _removeClassOrigins(cimInstance);
    }
}

static Array<ClassAssociation> _buildClassAssociationEntries(
    const CIMConstClass& assocClass)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "_buildClassAssociationEntries");

    Array<ClassAssociation> classAssocEntries;

    // Get the association's class name:

    CIMName assocClassName = assocClass.getClassName();

    // For each property:

    Uint32 n = assocClass.getPropertyCount();

    for (Uint32 i = 0; i < n; i++)
    {
        CIMConstProperty fromProp = assocClass.getProperty(i);

        if (fromProp.getType() == CIMTYPE_REFERENCE)
        {
            for (Uint32 j = 0; j < n; j++)
            {
                CIMConstProperty toProp = assocClass.getProperty(j);

                if (toProp.getType() == CIMTYPE_REFERENCE &&
                    (!fromProp.getName().equal(toProp.getName())))
                {
                    classAssocEntries.append(ClassAssociation(
                        assocClassName,
                        fromProp.getReferenceClassName(),
                        fromProp.getName(),
                        toProp.getReferenceClassName(),
                        toProp.getName()));
                }
            }
        }
    }

    PEG_METHOD_EXIT();
    return classAssocEntries;
}

/*
    This routine does the following:

        1.  Creates two entries in the association file for each relationship
            formed by this new association instance. A binary association
            (one with two references) ties two instances together. Suppose
            there are two instances: I1 and I2. Then two entries are created:

                I2 -> I1
                I1 -> I2

            For a ternary relationship, six entries will be created. Suppose
            there are three instances: I1, I2, and I3:

                I1 -> I2
                I1 -> I3
                I2 -> I1
                I2 -> I3
                I3 -> I1
                I3 -> I2

            So for an N-ary relationship, there will be N! entries created.

        2.  Verifies that the association instance refers to real objects.
            (note that an association reference may refer to either an instance
            or a class). Throws an exception if one of the references does not
            refer to a valid object.
*/
static Array<InstanceAssociation> _buildInstanceAssociationEntries(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& cimInstance,
    const CIMObjectPath& instanceName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "_buildInstanceAssociationEntries");

    Array<InstanceAssociation> instanceAssocEntries;

    // Get the association's instance name and class name:

    String assocInstanceName = instanceName.toString();
    CIMName assocClassName = instanceName.getClassName();

    // For each property:

    for (Uint32 i = 0, n = cimInstance.getPropertyCount(); i < n; i++)
    {
        CIMConstProperty fromProp = cimInstance.getProperty(i);

        // If a reference property:

        if (fromProp.getType() == CIMTYPE_REFERENCE)
        {
            // For each property:

            for (Uint32 j = 0, m = cimInstance.getPropertyCount(); j < m; j++)
            {
                CIMConstProperty toProp = cimInstance.getProperty(j);

                // If a reference property and not the same property:

                if (toProp.getType() == CIMTYPE_REFERENCE &&
                    (!fromProp.getName().equal (toProp.getName())))
                {
                    CIMObjectPath fromRef;
                    fromProp.getValue().get(fromRef);

                    CIMObjectPath toRef;
                    toProp.getValue().get(toRef);


                    // Fix for bugzilla 667:
                    // Strip off the hostname if it is the same as the
                    // local host
                    if ((fromRef.getHost() != String::EMPTY) &&
                        (System::isLocalHost(fromRef.getHost())))
                    {
                        PEG_TRACE_CSTRING(TRC_REPOSITORY, Tracer::LEVEL4,
                            "Stripping off local hostName from fromRef");
                        fromRef.setHost(String::EMPTY);
                    }

                    // Strip off the namespace when it is the same as the
                    // one this instance is created in.
                    if ((fromRef.getHost() == String::EMPTY) &&
                        (fromRef.getNameSpace() == nameSpace))
                    {
                        PEG_TRACE_CSTRING(TRC_REPOSITORY, Tracer::LEVEL4,
                            "Stripping off local nameSpace from fromRef");
                        fromRef.setNameSpace(CIMNamespaceName());
                    }

                    // Strip off the hostname if it is the same as the
                    // local host
                    if ((toRef.getHost() != String::EMPTY) &&
                        (System::isLocalHost(toRef.getHost())))
                    {
                        PEG_TRACE_CSTRING(TRC_REPOSITORY, Tracer::LEVEL4,
                            "Stripping off local hostName from toRef");
                        toRef.setHost(String::EMPTY);
                    }

                    // Strip off the namespace when it is the same as the
                    // one this instance is created in.
                    if ((toRef.getHost() == String::EMPTY) &&
                        (toRef.getNameSpace() == nameSpace))
                    {
                        PEG_TRACE_CSTRING(TRC_REPOSITORY, Tracer::LEVEL4,
                            "Stripping off local nameSpace from toRef");
                        toRef.setNameSpace(CIMNamespaceName());
                    }

                    instanceAssocEntries.append(InstanceAssociation(
                        assocInstanceName,
                        assocClassName,
                        fromRef.toString(),
                        fromRef.getClassName(),
                        fromProp.getName(),
                        toRef.toString(),
                        toRef.getClassName(),
                        toProp.getName()));
                }
            }
        }
    }

    PEG_METHOD_EXIT();
    return instanceAssocEntries;
}

/**
    Converts an object path to an instance name.  The host name is set to the
    empty string.  The namespace is set to null if it matches the specified
    namespace.  Otherwise, if it is not null, a CIM_ERR_NOT_FOUND exception is
    thrown.

    This function allows the repository to store instance names with
    consistent contents to facilitate equality tests.  (See Bug 1508.)

*/
static CIMObjectPath _stripInstanceName(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName)
{
    CIMObjectPath normalizedInstanceName = instanceName;
    normalizedInstanceName.setHost(String::EMPTY);

    if (instanceName.getNameSpace() == nameSpace)
    {
        normalizedInstanceName.setNameSpace(CIMNamespaceName());
    }
    else if (!instanceName.getNameSpace().isNull())
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_FOUND, instanceName.toString());
    }

    return normalizedInstanceName;
}

////////////////////////////////////////////////////////////////////////////////
//
// CIMRepository
//
//     The following are not implemented:
//
//         CIMRepository::execQuery()
//         CIMRepository::invokeMethod()
//
//     Note that invokeMethod() will not never implemented since it is not
//     meaningful for a repository.
//
//     Note that if declContext is passed to the CIMRepository constructor,
//     the repository object will own it and will delete it when appropriate.
//
////////////////////////////////////////////////////////////////////////////////

CIMRepository::CIMRepository(
    const String& repositoryRoot,
    Uint32 mode,
    RepositoryDeclContext* declContext)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::CIMRepository");

    Boolean binaryMode = mode & CIMRepository::MODE_BIN;

    if (mode == CIMRepository::MODE_DEFAULT)
    {
        binaryMode = ConfigManager::parseBooleanValue(
            ConfigManager::getInstance()->getCurrentValue(
                "enableBinaryRepository"));
    }

    // FUTURE?? -  compressMode = mode & CIMRepository::MODE_COMPRESSED;
    Boolean compressMode = false;

#ifdef PEGASUS_ENABLE_COMPRESSED_REPOSITORY    // PEP214
    compressMode = true;
    char* s = getenv("PEGASUS_ENABLE_COMPRESSED_REPOSITORY");
    if (s && (strcmp(s, "build_non_compressed") == 0))
    {
        compressMode = false;
#ifdef TEST_OUTPUT
        cout << "In Compress mode: build_non_compresed found" << endl;
#endif /* TEST_OUTPUT */
    }
#endif /* PEGASUS_ENABLE_COMPRESSED_REPOSITORY */

#ifdef TEST_OUTPUT
    cout << "repositoryRoot = " << repositoryRoot << endl;
    cout << "CIMRepository: binaryMode="  << binaryMode <<
        ", mode=" << mode << endl;
    cout << "CIMRepository: compressMode= " << compressMode << endl;
#endif /* TEST_OUTPUT */

    _rep = new CIMRepositoryRep();

    if (binaryMode)
    {
        // BUILD BINARY
        _rep->_streamer.reset(
            new AutoStreamer(new BinaryStreamer(), BINREP_MARKER));
        ((AutoStreamer*)_rep->_streamer.get())->addReader(new XmlStreamer(), 0);
    }
    else
    {
        // BUILD XML
        _rep->_streamer.reset(new AutoStreamer(new XmlStreamer(), 0xff));
        ((AutoStreamer*)_rep->_streamer.get())->addReader(
            new BinaryStreamer(), BINREP_MARKER);
        ((AutoStreamer*)_rep->_streamer.get())->addReader(new XmlStreamer(), 0);
    }

    // If declContext is supplied by the caller, don't allocate it.
    // CIMRepository will take ownership and will be responsible for
    // deleting it.
    if (declContext)
        _rep->_context = declContext;
    else
        _rep->_context = new RepositoryDeclContext();
    _rep->_context->setRepository(this);

    _rep->_isDefaultInstanceProvider = ConfigManager::parseBooleanValue(
        ConfigManager::getInstance()->getCurrentValue(
            "repositoryIsDefaultInstanceProvider"));

    _rep->_lockFile = ConfigManager::getInstance()->getHomedPath(
        PEGASUS_REPOSITORY_LOCK_FILE).getCString();

    _rep->_persistentStore.reset(PersistentStore::createPersistentStore(
        repositoryRoot,
        _rep->_streamer.get(),
        compressMode));

    _rep->_storeCompleteClassDefinitions =
        _rep->_persistentStore->storeCompleteClassDefinitions();

    // Initialize the NameSpaceManager

    Array<NamespaceDefinition> nameSpaces =
        _rep->_persistentStore->enumerateNameSpaces();

    Uint32 i = 0;
    while (i < nameSpaces.size())
    {
        if (nameSpaces[i].parentNameSpace.isNull() ||
            _rep->_nameSpaceManager.nameSpaceExists(
                nameSpaces[i].parentNameSpace))
        {
            // Parent namespace exists; go ahead and initialize this namespace
            _rep->_nameSpaceManager.initializeNameSpace(
                nameSpaces[i],
                _rep->_persistentStore->enumerateClassNames(
                    nameSpaces[i].name));
            i++;
        }
        else
        {
            // If the parent namespace appears later in the list, swap the
            // entries and repeat this iteration
            Boolean swapped = false;
            for (Uint32 j = i + 1; j < nameSpaces.size(); j++)
            {
                if (nameSpaces[i].parentNameSpace == nameSpaces[j].name)
                {
                    NamespaceDefinition tmp = nameSpaces[j];
                    nameSpaces[j] = nameSpaces[i];
                    nameSpaces[i] = tmp;
                    swapped = true;
                    break;
                }
            }

            if (!swapped)
            {
                PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "Namespace: %s ignored - parent namespace %s not found",
                    (const char*)nameSpaces[i].name.getString().getCString(),
                    (const char*)nameSpaces[i].parentNameSpace.getString().
                        getCString()));
                nameSpaces.remove(i);
            }
        }
    }

    if (!_rep->_nameSpaceManager.nameSpaceExists("root"))
    {
        // Create a root namespace per ...
        // Specification for CIM Operations over HTTP
        // Version 1.0
        // 2.5 Namespace Manipulation
        //
        // There are no intrinsic methods defined specifically for the
        // purpose of manipulating CIM Namespaces.  However, the
        // modelling of the a CIM Namespace using the class
        // __Namespace, together with the requirement that that
        // root Namespace MUST be supported by all CIM Servers,
        // implies that all Namespace operations can be supported.

        createNameSpace("root");
    }

    PEG_METHOD_EXIT();
}

CIMRepository::~CIMRepository()
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::~CIMRepository");

    delete _rep->_context;

    delete _rep;

    PEG_METHOD_EXIT();
}

String _toString(Boolean x)
{
    return(x ? "true" : "false");
}

CIMClass CIMRepository::getClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::getClass");

    ReadLock lock(_rep->_lock);
    CIMClass cimClass = _getClass(nameSpace,
                                  className,
                                  localOnly,
                                  includeQualifiers,
                                  includeClassOrigin,
                                  propertyList);

    PEG_METHOD_EXIT();
    return cimClass;
}

CIMClass CIMRepository::_getClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    Boolean clone)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::_getClass");

    PEG_TRACE((TRC_REPOSITORY, Tracer::LEVEL4,
           "nameSpace= %s, className= %s, localOnly= %s"
           ", includeQualifiers=  %s, includeClassOrigin= %s",
           (const char*)nameSpace.getString().getCString(),
           (const char*)className.getString().getCString(),
           (localOnly?"true":"false"),
           (includeQualifiers?"true":"false"),
           (includeClassOrigin?"true":"false")));

    CIMClass cimClass;
    // The class cache contains complete class definitions
    Boolean classIncludesPropagatedElements = true;

#ifdef PEGASUS_USE_CLASS_CACHE
    // Check the cache first.  Note that the cache contains complete class
    // definitions including propagated elements.

    String cacheKey = _getCacheKey(nameSpace, className);

    if (!_rep->_classCache.get(cacheKey, cimClass, clone))
    {
        // Not in cache so load from disk:
#endif

        CIMNamespaceName actualNameSpaceName;
        CIMName superClassName;
        _rep->_nameSpaceManager.locateClass(
            nameSpace, className, actualNameSpaceName, superClassName);

        cimClass = _rep->_persistentStore->getClass(
            actualNameSpaceName, className, superClassName);
        classIncludesPropagatedElements = _rep->_storeCompleteClassDefinitions;

        if (!localOnly && !classIncludesPropagatedElements)
        {
            // Propagate the superclass elements to this class.
            Resolver::resolveClass(cimClass, _rep->_context, nameSpace);
            classIncludesPropagatedElements = true;
        }

#ifdef PEGASUS_USE_CLASS_CACHE
        if (classIncludesPropagatedElements)
        {
            // Put in cache:

            _rep->_classCache.put(cacheKey, cimClass, clone);
        }
    }
#endif

#if !defined(PEGASUS_USE_CLASS_CACHE)
    // This flag must be true if caching is disabled, otherwise, an unecessary
    // copy could be created below.
    clone = true;
#endif

    // If clone is true, then cimClass is a clone (not shared with cache).
    // Else, it refers to the same one in the cache and any code below that
    // changes it, will need to clone it first.

    if (localOnly && classIncludesPropagatedElements)
    {
        // We must clone after all since object is modified below.
        if (!clone)
            cimClass = cimClass.clone();

        _stripPropagatedElements(cimClass);
    }

    // Remove properties based on propertyList
    if (!propertyList.isNull())
    {
        // We must clone after all since object is modified below.
        if (!clone)
            cimClass = cimClass.clone();

        // Remove properties that are not in the property list.
        // Work backwards because removal may be cheaper. Sint32 covers count=0
        for (Sint32 i = cimClass.getPropertyCount() - 1; i >= 0; i--)
        {
            if (!_containsProperty(cimClass.getProperty(i), propertyList))
            {
                cimClass.removeProperty(i);
            }
        }
    }

    // If includequalifiers false, remove all qualifiers from
    // properties, methods and parameters.
    if (!includeQualifiers)
    {
        // We must clone after all since object is modified below.
        if (!clone)
            cimClass = cimClass.clone();

        _removeAllQualifiers(cimClass);
    }

    // if ClassOrigin Flag false, remove classOrigin info from class object
    // by setting the property to Null.
    if (!includeClassOrigin)
    {
        // We must clone after all since object is modified below.
        if (!clone)
            cimClass = cimClass.clone();

        PEG_TRACE_CSTRING(TRC_REPOSITORY, Tracer::LEVEL4,
            "Remove Class Origins");

        Uint32 propertyCount = cimClass.getPropertyCount();
        for (Uint32 i = 0; i < propertyCount ; i++)
            cimClass.getProperty(i).setClassOrigin(CIMName());

        Uint32 methodCount =  cimClass.getMethodCount();
        for (Uint32 i=0; i < methodCount ; i++)
            cimClass.getMethod(i).setClassOrigin(CIMName());
    }

    PEG_METHOD_EXIT();
    return cimClass;
}

Boolean CIMRepositoryRep::_checkInstanceAlreadyExists(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName) const
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "CIMRepository::_checkInstanceAlreadyExists");

    //
    // Get the names of all superclasses and subclasses of this class
    //

    Array<CIMName> classNames;
    CIMName className = instanceName.getClassName();
    classNames.append(className);
    _nameSpaceManager.getSubClassNames(nameSpace, className, true, classNames);
    _nameSpaceManager.getSuperClassNames(nameSpace, className, classNames);

    //
    // Search for an instance with the specified key values
    //

    for (Uint32 i = 0; i < classNames.size(); i++)
    {
        CIMObjectPath tmpInstanceName = CIMObjectPath(
            String::EMPTY,
            CIMNamespaceName(),
            classNames[i],
            instanceName.getKeyBindings());

        if (_persistentStore->instanceExists(nameSpace, tmpInstanceName))
        {
            PEG_METHOD_EXIT();
            return true;
        }
    }

    PEG_METHOD_EXIT();
    return false;
}

CIMInstance CIMRepository::getInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::getInstance");

    ReadLock lock(_rep->_lock);

    CIMInstance cimInstance = _getInstance(
        nameSpace,
        instanceName,
        includeQualifiers,
        includeClassOrigin,
        propertyList,
        true);

    PEG_METHOD_EXIT();
    return cimInstance;
}

CIMInstance CIMRepository::_getInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    Boolean resolveInstance)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::_getInstance");

    CIMObjectPath normalizedInstanceName =
        _stripInstanceName(nameSpace, instanceName);

    if (!_rep->_nameSpaceManager.classExists(
        nameSpace, instanceName.getClassName()))
    {
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_INVALID_CLASS, instanceName.getClassName().getString());
    }

    CIMInstance cimInstance =
        _rep->_persistentStore->getInstance(nameSpace, normalizedInstanceName);

    //
    // Resolve the instance (if requested):
    //

    if (resolveInstance && includeQualifiers)
    {
        // Instances are resolved in persistent storage by the
        // createInstance and modifyInstance operations, but qualifiers
        // are not propagated.  The only reason to perform resolution
        // here is if qualifiers are requested in the instance.

        CIMConstClass cimClass;
        Resolver::resolveInstance (
            cimInstance, _rep->_context, nameSpace, cimClass, true);
    }

    _filterInstance(
        cimInstance,
        propertyList,
        includeQualifiers,
        includeClassOrigin);

    PEG_METHOD_EXIT();
    return cimInstance;
}

void CIMRepository::deleteClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,"CIMRepository::deleteClass");

    WriteLock lock(_rep->_lock);
    AutoFileLock fileLock(_rep->_lockFile);

    //
    // Get the class and check to see if it is an association class.
    //

    CIMClass cimClass = _getClass(
        nameSpace, className, false, true, false, CIMPropertyList());
    Boolean isAssociation = cimClass.isAssociation();

    _rep->_nameSpaceManager.checkDeleteClass(nameSpace, className);

    Array<CIMNamespaceName> dependentNameSpaceNames =
        _rep->_nameSpaceManager.getDependentSchemaNameSpaceNames(nameSpace);

    //
    // Ensure no instances of this class exist in the repository.
    //

    for (Uint32 i = 0; i < dependentNameSpaceNames.size(); i++)
    {
        Array<CIMObjectPath> instanceNames =
            _rep->_persistentStore->enumerateInstanceNamesForClass(
                dependentNameSpaceNames[i], className);

        if (instanceNames.size())
        {
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_CLASS_HAS_INSTANCES, className.getString());
        }
    }

#ifdef PEGASUS_USE_CLASS_CACHE

    _rep->_classCache.evict(_getCacheKey(nameSpace, className));

#endif /* PEGASUS_USE_CLASS_CACHE */

    // Remove the class from the SCMOClassCache.
    SCMOClassCache* pSCMOCache = SCMOClassCache::getInstance();
    pSCMOCache->removeSCMOClass(nameSpace,className);

    //
    // Delete the class. The NameSpaceManager::deleteClass() method throws
    // an exception if the class has subclasses.
    //

    CIMName superClassName =
        _rep->_nameSpaceManager.getSuperClassName(nameSpace, className);

    _rep->_nameSpaceManager.deleteClass(nameSpace, className);

    _rep->_persistentStore->deleteClass(
        nameSpace,
        className,
        superClassName,
        isAssociation,
        dependentNameSpaceNames);

    PEG_METHOD_EXIT();
}

void CIMRepository::deleteInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::deleteInstance");

    _rep->_nameSpaceManager.validateClass(
        nameSpace, instanceName.getClassName());

    CIMObjectPath normalizedInstanceName =
        _stripInstanceName(nameSpace, instanceName);

    WriteLock lock(_rep->_lock);
    AutoFileLock fileLock(_rep->_lockFile);

    _rep->_persistentStore->deleteInstance(nameSpace, normalizedInstanceName);

    PEG_METHOD_EXIT();
}

void CIMRepository::createClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::createClass");

    WriteLock lock(_rep->_lock);
    AutoFileLock fileLock(_rep->_lockFile);
    _createClass(nameSpace, newClass);

    PEG_METHOD_EXIT();
}

void CIMRepository::_createClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::_createClass");

    // -- Check whether the class may be created:

    _rep->_nameSpaceManager.checkCreateClass(
        nameSpace, newClass.getClassName(), newClass.getSuperClassName());

    // -- Resolve the class:

    CIMClass cimClass(newClass.clone());
    Resolver::resolveClass(cimClass, _rep->_context, nameSpace);

    // -- If an association class, build association entries:

    Array<ClassAssociation> classAssocEntries;

    if (cimClass.isAssociation())
    {
        classAssocEntries = _buildClassAssociationEntries(cimClass);
    }

    // -- Strip the propagated elements, if required

    if (!_rep->_storeCompleteClassDefinitions)
    {
        _stripPropagatedElements(cimClass);
    }

    // -- Create the class declaration:

    _rep->_persistentStore->createClass(nameSpace, cimClass, classAssocEntries);

    // -- Create namespace manager entry:

    _rep->_nameSpaceManager.createClass(
        nameSpace, cimClass.getClassName(), cimClass.getSuperClassName());

    PEG_METHOD_EXIT();
}

CIMObjectPath CIMRepository::createInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& newInstance)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::createInstance");

    WriteLock lock(_rep->_lock);
    AutoFileLock fileLock(_rep->_lockFile);
    CIMObjectPath instanceName = _createInstance(nameSpace, newInstance);

    PEG_METHOD_EXIT();
    return instanceName;
}

CIMObjectPath CIMRepository::_createInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& newInstance)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::_createInstance");

    //
    // Resolve the instance. Looks up class and fills out properties but
    // not the qualifiers.
    //

    CIMInstance cimInstance(newInstance.clone());
    CIMConstClass cimClass;
    Resolver::resolveInstance (cimInstance, _rep->_context, nameSpace, cimClass,
        false);
    CIMObjectPath instanceName = cimInstance.buildPath(cimClass);

    //
    // Make sure the class has keys (otherwise it will be impossible to
    // create the instance).
    //

    if (!cimClass.hasKeys())
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms("Repository.CIMRepository.CLASS_HAS_NO_KEYS",
                "class has no keys: $0",
                cimClass.getClassName().getString()));
    }

    //
    // Be sure instance does not already exist:
    //

    if (_rep->_checkInstanceAlreadyExists(nameSpace, instanceName))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_ALREADY_EXISTS,
            instanceName.toString());
    }

    //
    // Build association entries if an association instance.
    //

    Array<InstanceAssociation> instAssocEntries;

    if (cimClass.isAssociation())
    {
        instAssocEntries = _buildInstanceAssociationEntries(
            nameSpace, cimInstance, instanceName);
    }

    //
    // Create the instance
    //

    _rep->_persistentStore->createInstance(
        nameSpace, instanceName, cimInstance, instAssocEntries);

    PEG_METHOD_EXIT();
    return instanceName;
}

void CIMRepository::modifyClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& modifiedClass)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::modifyClass");

    WriteLock lock(_rep->_lock);
    AutoFileLock fileLock(_rep->_lockFile);
    _modifyClass(nameSpace, modifiedClass);

    PEG_METHOD_EXIT();
}

void CIMRepository::_modifyClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& modifiedClass)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::_modifyClass");

    //
    // Check to see if it is okay to modify this class:
    //

    CIMName oldSuperClassName;

    _rep->_nameSpaceManager.checkModifyClass(
        nameSpace,
        modifiedClass.getClassName(),
        modifiedClass.getSuperClassName(),
        oldSuperClassName,
        !_rep->_storeCompleteClassDefinitions);

    //
    // Resolve the class:
    //

    CIMClass cimClass(modifiedClass.clone());
    Resolver::resolveClass(cimClass, _rep->_context, nameSpace);

    //
    // ATTN: KS
    // Disallow modification of classes which have instances (that are
    // in the repository). And we have no idea whether the class has
    // instances in other repositories or in providers. We should do
    // an enumerate instance names at a higher level (above the repository).
    //

#ifdef PEGASUS_USE_CLASS_CACHE

    // Modifying this class may invalidate subclass definitions in the cache.
    // Since class modification is relatively rare, we just flush the entire
    // cache rather than specifically evicting subclass definitions.
    _rep->_classCache.clear();

#endif /* PEGASUS_USE_CLASS_CACHE */

    SCMOClassCache* pSCMOCache = SCMOClassCache::getInstance();
    pSCMOCache->clear();


    Boolean isAssociation = cimClass.isAssociation();
    Array<ClassAssociation> classAssocEntries;

    if (isAssociation)
    {
        classAssocEntries = _buildClassAssociationEntries(cimClass);
    }

    // Strip the propagated elements, if required

    if (!_rep->_storeCompleteClassDefinitions)
    {
        _stripPropagatedElements(cimClass);
    }

    _rep->_persistentStore->modifyClass(
        nameSpace,
        cimClass,
        oldSuperClassName,
        isAssociation,
        classAssocEntries);

    PEG_METHOD_EXIT();
}

void CIMRepository::modifyInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& modifiedInstance,
    Boolean includeQualifiers,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::modifyInstance");

    WriteLock lock(_rep->_lock);
    AutoFileLock fileLock(_rep->_lockFile);

    //
    // Do this:
    //

    CIMInstance cimInstance;   // The instance that replaces the original

    if (propertyList.isNull())
    {
        //
        // Replace all the properties in the instance
        //
        if (includeQualifiers)
        {
            //
            // Replace the entire instance with the given instance
            // (this is the default behavior)
            //
            cimInstance = modifiedInstance.clone();
        }
        else
        {
            //
            // Replace all the properties in the instance, but keep the
            // original qualifiers on the instance and on the properties
            //

            cimInstance = _getInstance(
                nameSpace,
                modifiedInstance.getPath (),
                true,
                true,
                CIMPropertyList(),
                false);

            CIMInstance newInstance(
                modifiedInstance.getPath ().getClassName());

            CIMConstInstance givenInstance = modifiedInstance;

            //
            // Copy over the original instance qualifiers
            //

            for (Uint32 i = 0; i < cimInstance.getQualifierCount(); i++)
            {
                newInstance.addQualifier(cimInstance.getQualifier(i));
            }

            //
            // Loop through the properties replacing each property in the
            // original with a new value, but keeping the original qualifiers
            //
            for (Uint32 i=0; i<givenInstance.getPropertyCount(); i++)
            {
                // Copy the given property value (not qualifiers)
                CIMConstProperty givenProperty = givenInstance.getProperty(i);
                CIMProperty newProperty(
                    givenProperty.getName(),
                    givenProperty.getValue(),
                    givenProperty.getArraySize(),
                    givenProperty.getReferenceClassName(),
                    givenProperty.getClassOrigin(),
                    givenProperty.getPropagated());

                // Copy the original property qualifiers
                Uint32 origPos =
                    cimInstance.findProperty(newProperty.getName());
                if (origPos != PEG_NOT_FOUND)
                {
                    CIMProperty origProperty = cimInstance.getProperty(origPos);
                    for (Uint32 j=0; j<origProperty.getQualifierCount(); j++)
                    {
                        newProperty.addQualifier(origProperty.getQualifier(j));
                    }
                }

                // Add the newly constructed property to the new instance
                newInstance.addProperty(newProperty);
            }

            // Use the newly merged instance to replace the original instance
            cimInstance = newInstance;
        }
    }
    else
    {
        //
        // Replace only the properties specified in the given instance
        //

        cimInstance = _getInstance(
            nameSpace,
            modifiedInstance.getPath(),
            true,
            true,
            CIMPropertyList(),
            false);

        CIMConstInstance givenInstance = modifiedInstance;

        // NOTE: Instance qualifiers are not changed when a property list
        // is specified.  Property qualifiers are replaced with the
        // corresponding property values.

        //
        // Loop through the propertyList replacing each property in the original
        //

        for (Uint32 i=0; i<propertyList.size(); i++)
        {
            Uint32 origPropPos = cimInstance.findProperty(propertyList[i]);
            if (origPropPos != PEG_NOT_FOUND)
            {
                // Case: Property set in original
                CIMProperty origProperty =
                    cimInstance.getProperty(origPropPos);

                // Get the given property value
                Uint32 givenPropPos =
                    givenInstance.findProperty(propertyList[i]);
                if (givenPropPos != PEG_NOT_FOUND)
                {
                    // Case: Property set in original and given
                    CIMConstProperty givenProperty =
                        givenInstance.getProperty(givenPropPos);

                    // Copy over the property from the given to the original
                    if (includeQualifiers)
                    {
                        // Case: Total property replacement
                        cimInstance.removeProperty(origPropPos);
                        cimInstance.addProperty(givenProperty.clone());
                    }
                    else
                    {
                        // Case: Replace only the property value (not quals)
                        origProperty.setValue(givenProperty.getValue());
                        cimInstance.removeProperty(origPropPos);
                        cimInstance.addProperty(origProperty);
                    }
                }
                else
                {
                    // Case: Property set in original and not in given
                    // Just remove the property (set to null)
                    cimInstance.removeProperty(origPropPos);
                }
            }
            else
            {
                // Case: Property not set in original

                // Get the given property value
                Uint32 givenPropPos =
                    givenInstance.findProperty(propertyList[i]);
                if (givenPropPos != PEG_NOT_FOUND)
                {
                    // Case: Property set in given and not in original
                    CIMConstProperty givenProperty =
                        givenInstance.getProperty(givenPropPos);

                    // Copy over the property from the given to the original
                    if (includeQualifiers)
                    {
                        // Case: Total property copy
                        cimInstance.addProperty(givenProperty.clone());
                    }
                    else
                    {
                        // Case: Copy only the property value (not qualifiers)
                        CIMProperty newProperty(
                            givenProperty.getName(),
                            givenProperty.getValue(),
                            givenProperty.getArraySize(),
                            givenProperty.getReferenceClassName(),
                            givenProperty.getClassOrigin(),
                            givenProperty.getPropagated());
                        cimInstance.addProperty(newProperty);
                    }
                }
                else
                {
                    // Case: Property not set in original or in given

                    // Nothing to do; just make sure the property name is valid
                    // ATTN: This is not the most efficient solution
                    CIMClass cimClass = getClass(
                        nameSpace, cimInstance.getClassName(), false);
                    if (cimClass.findProperty(propertyList[i]) == PEG_NOT_FOUND)
                    {
                        // ATTN: This exception may be returned by setProperty
                        PEG_METHOD_EXIT();
                        throw PEGASUS_CIM_EXCEPTION(
                            CIM_ERR_NO_SUCH_PROPERTY, "modifyInstance()");
                    }
                }
            }
        }
    }

    CIMObjectPath normalizedInstanceName =
        _stripInstanceName(nameSpace, modifiedInstance.getPath());

    //
    // Resolve the instance (do not propagate qualifiers from class since
    // this will bloat the instance).
    //

    CIMConstClass cimClass;
    Resolver::resolveInstance(
        cimInstance, _rep->_context, nameSpace, cimClass, false);

    //
    // Disallow operation if the instance name was changed:
    //

    if (cimInstance.buildPath(cimClass) != normalizedInstanceName)
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
            MessageLoaderParms(
                "Repository.CIMRepository.ATTEMPT_TO_MODIFY_KEY_PROPERTY",
                "Attempted to modify a key property"));
    }

    _rep->_persistentStore->modifyInstance(
        nameSpace, normalizedInstanceName, cimInstance);

    PEG_METHOD_EXIT();
}

Array<CIMClass> CIMRepository::enumerateClasses(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::enumerateClasses");

    ReadLock lock(_rep->_lock);

    Array<CIMName> classNames;

    _rep->_nameSpaceManager.getSubClassNames(
        nameSpace, className, deepInheritance, classNames);

    Array<CIMClass> result;

    for (Uint32 i = 0; i < classNames.size(); i++)
    {
        result.append(_getClass(nameSpace, classNames[i], localOnly,
            includeQualifiers, includeClassOrigin, CIMPropertyList()));
    }

    PEG_METHOD_EXIT();
    return result;
}

Array<CIMName> CIMRepository::enumerateClassNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::enumerateClassNames");

    ReadLock lock(_rep->_lock);

    Array<CIMName> classNames;

    _rep->_nameSpaceManager.getSubClassNames(
        nameSpace, className, deepInheritance, classNames,true);

    PEG_METHOD_EXIT();
    return classNames;
}

Array<CIMInstance> CIMRepository::enumerateInstancesForSubtree(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "CIMRepository::enumerateInstancesForSubtree");
    // It is not necessary to control access to the ReadWriteSem lock here.
    // This method calls enumerateInstancesForClass, which does its own
    // access control.

    //
    // Get all descendent classes of this class:
    //

    Array<CIMName> classNames;
    classNames.append(className);
    _rep->_nameSpaceManager.getSubClassNames(
        nameSpace, className, true, classNames);

    //
    // Get all instances for this class and all its descendent classes
    //

    Array<CIMInstance> namedInstances;

    for (Uint32 i = 0; i < classNames.size(); i++)
    {
        Array<CIMInstance> localNamedInstances =
            enumerateInstancesForClass(nameSpace, classNames[i],
                includeQualifiers, includeClassOrigin, propertyList);

        // The propertyList, includeQualifiers, and includeClassOrigin
        // filtering is done in enumerateInstancesForClass.
        // ATTN: deepInheritance filtering is not performed.

        namedInstances.appendArray(localNamedInstances);
    }

    PEG_METHOD_EXIT();
    return namedInstances;
}

Array<CIMInstance> CIMRepository::enumerateInstancesForClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "CIMRepository::enumerateInstancesForClass");

    ReadLock lock(_rep->_lock);

    _rep->_nameSpaceManager.validateClass(nameSpace, className);

    //
    // Get all instances for this class
    //

    Array<CIMInstance> namedInstances =
        _rep->_persistentStore->enumerateInstancesForClass(
            nameSpace, className);

    // Do any required filtering of properties, qualifiers, classorigin
    // on the returned instances.
    for (Uint32 i = 0 ; i < namedInstances.size(); i++)
    {
        if (includeQualifiers)
        {
            // Instances are resolved in persistent storage by the
            // createInstance and modifyInstance operations, but qualifiers
            // are not propagated.  The only reason to perform resolution
            // here is if qualifiers are requested in the instance.
            Resolver::resolveInstance(
                namedInstances[i], _rep->_context, nameSpace, true);
        }

        _filterInstance(
            namedInstances[i],
            propertyList,
            includeQualifiers,
            includeClassOrigin);
    }

    PEG_METHOD_EXIT();
    return namedInstances;
}

Array<CIMObjectPath> CIMRepository::enumerateInstanceNamesForSubtree(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "CIMRepository::enumerateInstanceNamesForSubtree");

    // It is not necessary to control access to the ReadWriteSem lock here.
    // This method calls enumerateInstanceNamesForClass, which does its own
    // access control.

    //
    // Get names of descendent classes:
    //

    Array<CIMName> classNames;
    classNames.append(className);
    _rep->_nameSpaceManager.getSubClassNames(
        nameSpace, className, true, classNames);

    //
    // Enumerate instance names for each of the subclasses
    //
    Array<CIMObjectPath> instanceNames;

    for (Uint32 i = 0; i < classNames.size(); i++)
    {
        instanceNames.appendArray(
            enumerateInstanceNamesForClass(nameSpace, classNames[i]));
    }

    PEG_METHOD_EXIT();
    return instanceNames;
}

Array<CIMObjectPath> CIMRepository::enumerateInstanceNamesForClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY,
        "CIMRepository::enumerateInstanceNamesForClass");

    ReadLock lock(_rep->_lock);

    _rep->_nameSpaceManager.validateClass(nameSpace, className);

    Array<CIMObjectPath> instanceNames =
        _rep->_persistentStore->enumerateInstanceNamesForClass(
            nameSpace, className);

    PEG_METHOD_EXIT();
    return instanceNames;
}


Array<CIMObject> CIMRepository::associators(
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
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::associators");

    ReadLock lock(_rep->_lock);

    Array<CIMObjectPath> names = _associatorNames(
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole);

    Array<CIMObject> result;

    for (Uint32 i = 0, n = names.size(); i < n; i++)
    {
        CIMNamespaceName tmpNameSpace = names[i].getNameSpace();

        if (tmpNameSpace.isNull())
            tmpNameSpace = nameSpace;

        //
        //  ATTN-CAKG-P2-20020726:  The following condition does not correctly
        //  distinguish instanceNames from classNames in every case
        //  The instanceName of a singleton instance of a keyless class also
        //  has no key bindings.
        //  This works today because we do not use singleton instances in
        //  the model. See BUG_3302.
        //
        CIMObjectPath tmpRef = names[i];
        tmpRef.setHost(String());
        tmpRef.setNameSpace(CIMNamespaceName());

        if (names[i].getKeyBindings ().size () == 0)
        {
            CIMClass cimClass = _getClass(
                tmpNameSpace,
                tmpRef.getClassName(),
                false,
                includeQualifiers,
                includeClassOrigin,
                propertyList);

            CIMObject cimObject(cimClass);
            cimObject.setPath (names[i]);
            result.append(cimObject);
        }
        else
        {
            CIMInstance cimInstance = _getInstance(
                tmpNameSpace,
                tmpRef,
                includeQualifiers,
                includeClassOrigin,
                propertyList,
                true);

            CIMObject cimObject(cimInstance);
            cimObject.setPath (names[i]);
            result.append(cimObject);
        }
    }

    PEG_METHOD_EXIT();
    return result;
}

Array<CIMObjectPath> CIMRepository::associatorNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::associatorNames");

    ReadLock lock(_rep->_lock);
    Array<CIMObjectPath> result = _associatorNames(
        nameSpace, objectName, assocClass, resultClass, role, resultRole);

    PEG_METHOD_EXIT();
    return result;
}

Array<CIMObjectPath> CIMRepository::_associatorNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::_associatorNames");

    Array<String> associatorNames;

    // The assocClass parameter implies subclasses, so retrieve them
    Array<CIMName> assocClassList;
    if (!assocClass.isNull())
    {
        _rep->_nameSpaceManager.getSubClassNames(
            nameSpace, assocClass, true, assocClassList);
        assocClassList.append(assocClass);
    }

    // The resultClass parameter implies subclasses, so retrieve them
    Array<CIMName> resultClassList;
    if (!resultClass.isNull())
    {
        _rep->_nameSpaceManager.getSubClassNames(
            nameSpace, resultClass, true, resultClassList);
        resultClassList.append(resultClass);
    }

    //
    //  ATTN-CAKG-P2-20020726:  The following condition does not correctly
    //  distinguish instanceNames from classNames in every case
    //  The instanceName of a singleton instance of a keyless class also
    //  has no key bindings
    //  This works today because we do not use singleton instances in
    //  the model. See BUG_3302.
    if (objectName.getKeyBindings ().size () == 0)
    {
        CIMName className = objectName.getClassName();

        Array<CIMName> classList;
        _rep->_nameSpaceManager.getSuperClassNames(
            nameSpace, className, classList);
        classList.append(className);

        Array<CIMNamespaceName> nameSpaceList =
            _rep->_nameSpaceManager.getSchemaNameSpaceNames(nameSpace);

        for (Uint32 i = 0; i < nameSpaceList.size(); i++)
        {
            Array<String> associatorNamesForNameSpace;

            _rep->_persistentStore->getClassAssociatorNames(
                nameSpaceList[i],
                classList,
                assocClassList,
                resultClassList,
                role,
                resultRole,
                associatorNamesForNameSpace);

            associatorNames.appendArray(associatorNamesForNameSpace);
        }
    }
    else
    {
        _rep->_nameSpaceManager.validateClass(
            nameSpace, objectName.getClassName());

        _rep->_persistentStore->getInstanceAssociatorNames(
            nameSpace,
            objectName,
            assocClassList,
            resultClassList,
            role,
            resultRole,
            associatorNames);
    }

    Array<CIMObjectPath> result;

    for (Uint32 i = 0, n = associatorNames.size(); i < n; i++)
    {
        CIMObjectPath r = associatorNames[i];

        if (r.getHost().size() == 0)
            r.setHost(System::getHostName());

        if (r.getNameSpace().isNull())
            r.setNameSpace(nameSpace);

        result.append(r);
    }

    PEG_METHOD_EXIT();
    return result;
}

Array<CIMObject> CIMRepository::references(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::references");

    ReadLock lock(_rep->_lock);

    Array<CIMObjectPath> names = _referenceNames(
        nameSpace,
        objectName,
        resultClass,
        role);

    Array<CIMObject> result;

    for (Uint32 i = 0, n = names.size(); i < n; i++)
    {
        CIMNamespaceName tmpNameSpace = names[i].getNameSpace();

        if (tmpNameSpace.isNull())
            tmpNameSpace = nameSpace;

        // ATTN: getInstance() should this be able to handle instance names
        // with host names and namespaces?

        CIMObjectPath tmpRef = names[i];
        tmpRef.setHost(String());
        tmpRef.setNameSpace(CIMNamespaceName());

        //
        //  ATTN-CAKG-P2-20020726:  The following condition does not correctly
        //  distinguish instanceNames from classNames in every case
        //  The instanceName of a singleton instance of a keyless class also
        //  has no key bindings
        //  This works today because we do not use singleton instances in
        //  the model. See BUG_3302.
        //
        if (objectName.getKeyBindings ().size () == 0)
        {
            CIMClass cimClass = _getClass(
                tmpNameSpace,
                tmpRef.getClassName(),
                false,
                includeQualifiers,
                includeClassOrigin,
                propertyList);

            CIMObject cimObject = CIMObject (cimClass);
            cimObject.setPath (names[i]);
            result.append (cimObject);
        }
        else
        {
            CIMInstance instance = _getInstance(
                tmpNameSpace,
                tmpRef,
                includeQualifiers,
                includeClassOrigin,
                propertyList,
                true);

            CIMObject cimObject = CIMObject (instance);
            cimObject.setPath (names[i]);
            result.append (cimObject);
        }
    }

    PEG_METHOD_EXIT();
    return result;
}

Array<CIMObjectPath> CIMRepository::referenceNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::referenceNames");

    ReadLock lock(_rep->_lock);
    Array<CIMObjectPath> result = _referenceNames(
        nameSpace, objectName, resultClass, role);

    PEG_METHOD_EXIT();
    return result;
}

Array<CIMObjectPath> CIMRepository::_referenceNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::_referenceNames");

    Array<String> tmpReferenceNames;

    // The resultClass parameter implies subclasses, so retrieve them
    Array<CIMName> resultClassList;

    try
    {
        if (!resultClass.isNull())
        {
            _rep->_nameSpaceManager.getSubClassNames(
                nameSpace, resultClass, true, resultClassList);
            resultClassList.append(resultClass);
        }

        //  ATTN-CAKG-P2-20020726:  The following condition does not correctly
        //  distinguish instanceNames from classNames in every case
        //  The instanceName of a singleton instance of a keyless class also
        //  has no key bindings
        //  This works today because we do not use singleton instances in
        //  the model. See BUG_3302.
        //
        if (objectName.getKeyBindings ().size () == 0)
        {
            CIMName className = objectName.getClassName();

            Array<CIMName> classList;
            _rep->_nameSpaceManager.getSuperClassNames(
                nameSpace, className, classList);
            classList.append(className);

            Array<CIMNamespaceName> nameSpaceList =
                _rep->_nameSpaceManager.getSchemaNameSpaceNames(nameSpace);

            for (Uint32 i = 0; i < nameSpaceList.size(); i++)
            {
                Array<String> referenceNamesForNameSpace;

                _rep->_persistentStore->getClassReferenceNames(
                    nameSpaceList[i],
                    classList,
                    resultClassList,
                    role,
                    referenceNamesForNameSpace);

                tmpReferenceNames.appendArray(referenceNamesForNameSpace);
            }
        }
        else
        {
            _rep->_nameSpaceManager.validateClass(
                nameSpace, objectName.getClassName());

            _rep->_persistentStore->getInstanceReferenceNames(
                nameSpace,
                objectName,
                resultClassList,
                role,
                tmpReferenceNames);
        }
    }
    catch (const CIMException& exception)
    {
        if (exception.getCode() == CIM_ERR_INVALID_CLASS)
        {
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_INVALID_PARAMETER, exception.getMessage());
        }
        else
        {
            throw;
        }
    }

    Array<CIMObjectPath> result;

    for (Uint32 i = 0, n = tmpReferenceNames.size(); i < n; i++)
    {
        CIMObjectPath r = tmpReferenceNames[i];

        if (r.getHost().size() == 0)
            r.setHost(System::getHostName());

        if (r.getNameSpace().isNull())
            r.setNameSpace(nameSpace);

        result.append(r);
    }

    PEG_METHOD_EXIT();
    return result;
}

CIMValue CIMRepository::getProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::getProperty");

    ReadLock lock(_rep->_lock);

    //
    // Retrieve the specified instance
    //

    CIMInstance cimInstance = _getInstance(
        nameSpace, instanceName, true, true, CIMPropertyList(), false);

    //
    // Get the requested property from the instance
    //

    Uint32 pos = cimInstance.findProperty(propertyName);

    // ATTN: This breaks if the property is simply null
    if (pos == PEG_NOT_FOUND)
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_NO_SUCH_PROPERTY,
            propertyName.getString());
    }

    CIMProperty prop = cimInstance.getProperty(pos);

    //
    // Return the value:
    //

    PEG_METHOD_EXIT();
    return prop.getValue();
}

void CIMRepository::setProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName,
    const CIMValue& newValue)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::setProperty");

    // It is not necessary to control access to the ReadWriteSem lock here.
    // This method calls modifyInstance, which does its own access control.

    //
    // Create the instance to pass to modifyInstance()
    //

    CIMInstance instance(instanceName.getClassName());
    instance.addProperty(CIMProperty(propertyName, newValue));
    instance.setPath (instanceName);

    //
    // Create the propertyList to pass to modifyInstance()
    //

    Array<CIMName> propertyListArray;
    propertyListArray.append(propertyName);
    CIMPropertyList propertyList(propertyListArray);

    //
    // Modify the instance to set the value of the given property
    //
    modifyInstance(nameSpace, instance, false, propertyList);

    PEG_METHOD_EXIT();
}

CIMQualifierDecl CIMRepository::getQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::getQualifier");

    ReadLock lock(_rep->_lock);
    CIMQualifierDecl qualifierDecl = _getQualifier(nameSpace, qualifierName);

    PEG_METHOD_EXIT();
    return qualifierDecl;
}

CIMQualifierDecl CIMRepository::_getQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::_getQualifier");

    CIMQualifierDecl qualifierDecl;

    String qualifierCacheKey = _getCacheKey(nameSpace, qualifierName);

    // Check the cache first:

    if (!_rep->_qualifierCache.get(qualifierCacheKey, qualifierDecl))
    {
        // Not in cache so load from disk:

        Array<CIMNamespaceName> nameSpaceList =
            _rep->_nameSpaceManager.getSchemaNameSpaceNames(nameSpace);

        for (Uint32 i = 0; i < nameSpaceList.size(); i++)
        {
            qualifierDecl = _rep->_persistentStore->getQualifier(
                nameSpaceList[i], qualifierName);

            if (!qualifierDecl.isUninitialized())
            {
                // Put in cache
                _rep->_qualifierCache.put(qualifierCacheKey, qualifierDecl);

                PEG_METHOD_EXIT();
                return qualifierDecl;
            }
        }

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(
            CIM_ERR_NOT_FOUND, qualifierName.getString());
    }

    PEG_METHOD_EXIT();
    return qualifierDecl;
}

void CIMRepository::setQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMQualifierDecl& qualifierDecl)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::setQualifier");

    WriteLock lock(_rep->_lock);
    AutoFileLock fileLock(_rep->_lockFile);
    _setQualifier(nameSpace, qualifierDecl);

    PEG_METHOD_EXIT();
}

void CIMRepository::_setQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMQualifierDecl& qualifierDecl)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::_setQualifier");

    _rep->_nameSpaceManager.checkNameSpaceUpdateAllowed(
        nameSpace);

    // Exception if namespace does not allow update
    _rep->_persistentStore->setQualifier(nameSpace, qualifierDecl);

    String qualifierCacheKey =
        _getCacheKey(nameSpace, qualifierDecl.getName());

    _rep->_qualifierCache.put(
        qualifierCacheKey, (CIMQualifierDecl&)qualifierDecl);

    PEG_METHOD_EXIT();
}

void CIMRepository::deleteQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::deleteQualifier");

    WriteLock lock(_rep->_lock);
    AutoFileLock fileLock(_rep->_lockFile);

    // Exception exit if not allowed
    _rep->_nameSpaceManager.checkNameSpaceUpdateAllowed(
        nameSpace);

    _rep->_persistentStore->deleteQualifier(nameSpace, qualifierName);

    String qualifierCacheKey = _getCacheKey(nameSpace, qualifierName);
    _rep->_qualifierCache.evict(qualifierCacheKey);

    PEG_METHOD_EXIT();
}

Array<CIMQualifierDecl> CIMRepository::enumerateQualifiers(
    const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::enumerateQualifiers");

    ReadLock lock(_rep->_lock);

    Array<CIMQualifierDecl> qualifiers;

    _rep->_nameSpaceManager.validateNameSpace(nameSpace);

    qualifiers = _rep->_persistentStore->enumerateQualifiers(nameSpace);

    PEG_METHOD_EXIT();
    return qualifiers;
}

void CIMRepository::createNameSpace(
    const CIMNamespaceName& nameSpace,
    const NameSpaceAttributes& attributes)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::createNameSpace");

    WriteLock lock(_rep->_lock);
    AutoFileLock fileLock(_rep->_lockFile);

    Boolean shareable = false;
    Boolean updatesAllowed = true;
    String parentNameSpace;
    String remoteInfo;

    for (NameSpaceAttributes::Iterator i = attributes.start(); i; i++)
    {
        String key = i.key();
        if (String::equalNoCase(key, "shareable"))
        {
            if (String::equalNoCase(i.value(), "true"))
                shareable = true;
        }
        else if (String::equalNoCase(key, "updatesAllowed"))
        {
            if (String::equalNoCase(i.value(), "false"))
                updatesAllowed = false;
        }
        else if (String::equalNoCase(key, "parent"))
        {
            parentNameSpace = i.value();
        }
        else if (String::equalNoCase(key, "remoteInfo"))
        {
            remoteInfo = i.value();
        }
        else
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(
                CIM_ERR_NOT_SUPPORTED,
                nameSpace.getString() + " option not supported: " + key);
        }
    }

    _rep->_nameSpaceManager.createNameSpace(
        nameSpace, shareable, updatesAllowed, parentNameSpace, remoteInfo);

    try
    {
        _rep->_persistentStore->createNameSpace(
            nameSpace, shareable, updatesAllowed, parentNameSpace, remoteInfo);
    }
    catch (...)
    {
        _rep->_nameSpaceManager.deleteNameSpace(nameSpace);
        throw;
    }

    PEG_METHOD_EXIT();
}

void CIMRepository::modifyNameSpace(
    const CIMNamespaceName& nameSpace,
    const NameSpaceAttributes& attributes)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::modifyNameSpace");

    WriteLock lock(_rep->_lock);
    AutoFileLock fileLock(_rep->_lockFile);

    Boolean shareable = false;
    Boolean updatesAllowed = true;

    for (NameSpaceAttributes::Iterator i = attributes.start(); i; i++)
    {
        String key = i.key();
        if (String::equalNoCase(key, "shareable"))
        {
            if (String::equalNoCase(i.value(), "true"))
                shareable = true;
        }
        else if (String::equalNoCase(key, "updatesAllowed"))
        {
            if (String::equalNoCase(i.value(),"false"))
                updatesAllowed = false;
        }
        else
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED,
                nameSpace.getString() + " option not supported: " + key);
        }
    }

    _rep->_nameSpaceManager.validateNameSpace(nameSpace);

    if (!shareable)
    {
        // Check for dependent namespaces

        CIMNamespaceName dependentNameSpaceName;

        if (_rep->_nameSpaceManager.hasDependentNameSpace(
                nameSpace, dependentNameSpaceName))
        {
            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
                "Namespace " + nameSpace.getString() +
                    " has dependent namespace " +
                    dependentNameSpaceName.getString());
        }
    }

    _rep->_persistentStore->modifyNameSpace(
        nameSpace, shareable, updatesAllowed);

    _rep->_nameSpaceManager.modifyNameSpace(
        nameSpace, shareable, updatesAllowed);

    PEG_METHOD_EXIT();
}

void CIMRepository::modifyNameSpaceName(
        const CIMNamespaceName& nameSpace,
        const CIMNamespaceName& newNameSpaceName)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::modifyNameSpaceName");

    WriteLock lock(_rep->_lock);
    AutoFileLock fileLock(_rep->_lockFile);

    _rep->_nameSpaceManager.validateNameSpace(nameSpace);

    _rep->_persistentStore->modifyNameSpaceName(
            nameSpace, newNameSpaceName);

    _rep->_nameSpaceManager.modifyNameSpaceName(
        nameSpace, newNameSpaceName);

    PEG_METHOD_EXIT();
}

Array<CIMNamespaceName> CIMRepository::enumerateNameSpaces() const
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::enumerateNameSpaces");

    ReadLock lock(const_cast<ReadWriteSem&>(_rep->_lock));

    Array<CIMNamespaceName> nameSpaceNames;
    _rep->_nameSpaceManager.getNameSpaceNames(nameSpaceNames);

    PEG_METHOD_EXIT();
    return nameSpaceNames;
}

void CIMRepository::deleteNameSpace(const CIMNamespaceName& nameSpace)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::deleteNameSpace");

    WriteLock lock(_rep->_lock);
    AutoFileLock fileLock(_rep->_lockFile);

    // Check for dependent namespaces

    CIMNamespaceName dependentNameSpaceName;

    if (_rep->_nameSpaceManager.hasDependentNameSpace(
            nameSpace, dependentNameSpaceName))
    {
        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
            "Namespace " + nameSpace.getString() +
                " has dependent namespace " +
                dependentNameSpaceName.getString());
    }

    // Make sure the namespace is empty

    if (!_rep->_persistentStore->isNameSpaceEmpty(nameSpace))
    {
        PEG_METHOD_EXIT();
        throw NonEmptyNameSpace(nameSpace.getString());
    }

    _rep->_persistentStore->deleteNameSpace(nameSpace);

    _rep->_nameSpaceManager.deleteNameSpace(nameSpace);

    PEG_METHOD_EXIT();
}

Boolean CIMRepository::getNameSpaceAttributes(const CIMNamespaceName& nameSpace,
        NameSpaceAttributes& attributes)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::deleteNameSpace");

    ReadLock lock(const_cast<ReadWriteSem&>(_rep->_lock));
    attributes.clear();

    Boolean shareable;
    Boolean updatesAllowed;
    String parent;
    String remoteInfo;

    if (!_rep->_nameSpaceManager.getNameSpaceAttributes(
        nameSpace, shareable, updatesAllowed, parent, remoteInfo))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    attributes.insert("name", nameSpace.getString());

    if (shareable)
        attributes.insert("shareable", "true");
    else
        attributes.insert("shareable", "false");

    if (updatesAllowed)
        attributes.insert("updatesAllowed", "true");
    else
        attributes.insert("updatesAllowed", "false");

    if (parent.size())
        attributes.insert("parent", parent);

    if (remoteInfo.size())
        attributes.insert("remoteInfo", remoteInfo);

    PEG_METHOD_EXIT();
    return true;
}

Boolean CIMRepository::nameSpaceExists(const CIMNamespaceName& nameSpaceName)
{
    ReadLock lock(const_cast<ReadWriteSem&>(_rep->_lock));
    return _rep->_nameSpaceManager.nameSpaceExists(nameSpaceName);
}

Boolean CIMRepository::isRemoteNameSpace(
    const CIMNamespaceName& nameSpaceName,
    String& remoteInfo)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::isRemoteNamespace");
    ReadLock lock(const_cast<ReadWriteSem&>(_rep->_lock));
    PEG_METHOD_EXIT();
    return _rep->_nameSpaceManager.isRemoteNameSpace(
        nameSpaceName, remoteInfo);
}

#ifdef PEGASUS_DEBUG
    void CIMRepository::DisplayCacheStatistics()
    {
#ifdef PEGASUS_USE_CLASS_CACHE
        cout << "Repository Class Cache Statistics:" << endl;
        _rep->_classCache.DisplayCacheStatistics();
#endif
        cout << "Repository Qualifier Cache Statistics:" << endl;
        _rep->_qualifierCache.DisplayCacheStatistics();
    }
#endif

void CIMRepository::getSubClassNames(
    const CIMNamespaceName& nameSpaceName,
    const CIMName& className,
    Boolean deepInheritance,
    Array<CIMName>& subClassNames) const
{
    ReadLock lock(const_cast<ReadWriteSem&>(_rep->_lock));
    _rep->_nameSpaceManager.getSubClassNames(
        nameSpaceName, className, deepInheritance, subClassNames);
}

void CIMRepository::getSuperClassNames(
    const CIMNamespaceName& nameSpaceName,
    const CIMName& className,
    Array<CIMName>& subClassNames) const
{
    ReadLock lock(const_cast<ReadWriteSem&>(_rep->_lock));
    _rep->_nameSpaceManager.getSuperClassNames(
        nameSpaceName, className, subClassNames);
}

Boolean CIMRepository::isDefaultInstanceProvider()
{
    return _rep->_isDefaultInstanceProvider;
}

CIMConstClass CIMRepository::getFullConstClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    PEG_METHOD_ENTER(TRC_REPOSITORY, "CIMRepository::getFullConstClass");

    ReadLock lock(_rep->_lock);
    CIMClass cimClass = _getClass(nameSpace, className, false, true, true,
        CIMPropertyList(), false);

    PEG_METHOD_EXIT();
    return cimClass;
}

PEGASUS_NAMESPACE_END
