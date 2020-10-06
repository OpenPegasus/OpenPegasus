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

/**
 * This file redefines the CIMRepository.h interfaces to be a
 * read-only memory-resident  class repository and an in-memory instance
 * repository. It implements all functions of the
 * CIMRepository.h interface. All  class functions that would
 * modify the repository return exceptions since the repository is read-only.
 * Functions that return data (ex. getClass, enumerateClasses, etc.) return
 * the same form of data as the other repository implementations.
 *
 * NOTE: This repository does NOT implement a cache because it
 * appeared that it would be of little gain since the movement
 * of data from the in-memory repository is already fast.
 * Therefore there are some parameters of the API (Ex clone)
 * that are not implemented.
 *
 * The class repository is actually a .CPP/.h file that is
 * produced by the MOF compiler from input mof files. This file
 * includes the definition of all namespaces, qualifiers, and
 * classes defined as part of the mof input.
 *
 * The instance repository is an in-memory repository that
 * includes functions to load the instance repository on startup
 * and checkpoint the instance repository each time a change is
 * made
 *
 * See PEP 307 for more information on the implementation and
 * usage of the memory-resident repository.
 */

#include "MRR.h"
#include <cstdarg>
#include <Pegasus/Common/Resolver.h>
#include <Pegasus/Common/Once.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Pair.h>
#include "CIMRepository.h"
#include "RepositoryDeclContext.h"
#include "MRRSerialization.h"
#include "MRRTypes.h"

PEGASUS_NAMESPACE_BEGIN

typedef Pair<CIMNamespaceName, CIMInstance> NamespaceInstancePair;

#define PEGASUS_ARRAY_T NamespaceInstancePair
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

class CIMRepositoryRep
{
public:

    Uint32 _findInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName);

    void _processSaveCallback();

    void _processLoadCallback();

    Array<NamespaceInstancePair> _rep;
};

typedef const MRRClass* ConstMRRClassPtr;
#define PEGASUS_ARRAY_T ConstMRRClassPtr
# include <Pegasus/Common/ArrayInter.h>
# include <Pegasus/Common/ArrayImpl.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T NamespaceInstancePair
# include <Pegasus/Common/ArrayImpl.h>
#undef PEGASUS_ARRAY_T

//==============================================================================
//
// Local definitions:
//
//==============================================================================

static size_t const _MAX_NAMESPACE_TABLE_SIZE = 64;
static const MRRNameSpace* _nameSpaceTable[_MAX_NAMESPACE_TABLE_SIZE];
static size_t _nameSpaceTableSize = 0;

class ThrowContext
{
public:

    PEGASUS_FORMAT(3, 4)
    ThrowContext(CIMStatusCode code_, const char* format, ...) : code(code_)
    {
        char buffer[1024];
        va_list ap;
        va_start(ap, format);
        vsprintf(buffer, format, ap);
        va_end(ap);
        msg = buffer;
    }
    CIMStatusCode code;
    String msg;
};

#define Throw(ARGS) \
    do \
    { \
        ThrowContext c ARGS; \
        throw CIMException(c.code, c.msg); \
    } \
    while (0)

class Str
{
public:
    Str(const String& s) : _cstr(s.getCString()) { }
    Str(const CIMName& n) : _cstr(n.getString().getCString()) { }
    Str(const CIMNamespaceName& n) : _cstr(n.getString().getCString()) { }
    Str(const Exception& e) : _cstr(e.getMessage().getCString()) { }
    Str(const CIMDateTime& x) : _cstr(x.toString().getCString()) { }
    Str(const CIMObjectPath& x) : _cstr(x.toString().getCString()) { }
    const char* operator*() const { return (const char*)_cstr; }
    operator const char*() const { return (const char*)_cstr; }
private:
    CString _cstr;
};

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
        {
            return true;
        }
    }
    return false;
}

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
    {
        cimClass.removeQualifier(count - 1);
    }

    // remove qualifiers from the properties
    for (Uint32 i = 0; i < cimClass.getPropertyCount(); i++)
    {
        CIMProperty p = cimClass.getProperty(i);
        count = 0;
        while ((count = p.getQualifierCount()) > 0)
        {
            p.removeQualifier(count - 1);
        }
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
            {
                p.removeQualifier(count - 1);
            }
        }
        count = 0;
        while ((count = m.getQualifierCount()) > 0)
        {
            m.removeQualifier(count - 1);
        }
    }
}

/////////////////////////////////////////////////////////////////////////
//
// _removePropagatedQualifiers - Removes all qualifiers from the class
// that are marked propagated
//
/////////////////////////////////////////////////////////////////////////

/* removes propagatedQualifiers from the defined CIMClass.
   This function removes the qualifiers from the class,
   from each of the properties, from the methods and
   the parameters if the qualifiers are marked propagated.
   NOTE: This could be logical to be moved to CIMClass since it may be
   more general than the usage here.
*/
static void _removePropagatedQualifiers(CIMClass& cimClass)
{
    Uint32 count = cimClass.getQualifierCount();
    // Remove nonlocal qualifiers from Class
    for (Sint32 i = (count - 1); i >= 0; i--)
    {
        CIMQualifier q = cimClass.getQualifier(i);
        if (q.getPropagated())
        {
            cimClass.removeQualifier(i);
        }
    }

    // remove nonlocal qualifiers from the properties
    for (Uint32 i = 0; i < cimClass.getPropertyCount(); i++)
    {
        CIMProperty p = cimClass.getProperty(i);
        // loop to search qualifiers for nonlocal parameters
        count = p.getQualifierCount();
        for (Sint32 j = (count - 1); j >= 0; j--)
        {
            CIMQualifier q = p.getQualifier(j);
            if (q.getPropagated())
            {
                p.removeQualifier(j);
            }
        }
    }

    // remove nonlocal qualifiers from the methods and parameters
    for (Uint32 i = 0; i < cimClass.getMethodCount(); i++)
    {
        CIMMethod m = cimClass.getMethod(i);
        // Remove  nonlocal qualifiers from all parameters
        for (Uint32 j = 0 ; j < m.getParameterCount(); j++)
        {
            CIMParameter p = m.getParameter(j);
            count = p.getQualifierCount();
            for (Sint32 k = (count - 1); k >= 0; k--)
            {
                CIMQualifier q = p.getQualifier(k);
                if (q.getPropagated())
                {
                    p.removeQualifier(k);
                }
            }
        }

        // remove nonlocal qualifiers from the method
        count = m.getQualifierCount();
        for (Sint32 j = (count - 1); j >= 0; j--)
        {
            CIMQualifier q = m.getQualifier(j);
            if (q.getPropagated())
            {
                m.removeQualifier(j);
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
            {
                cimInstance.removeProperty(i);
            }
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
    {
        cimInstance.removeQualifier(count - 1);
    }

    // remove qualifiers from the properties
    for (Uint32 i = 0; i < cimInstance.getPropertyCount(); i++)
    {
        CIMProperty p = cimInstance.getProperty(i);
        count = 0;
        while ((count = p.getQualifierCount()) > 0)
        {
            p.removeQualifier(count - 1);
        }
    }
}

/* removes all ClassOrigin attributes from a single CIMInstance. Removes
    the classOrigin attribute from each property in the Instance.
   @param Instance from which the ClassOrigin Properties will be removed.
   NOTE: Logical to be moved to CIMInstance since it may be more general
   than just the repositoryl
*/
void _removeClassOrigins(CIMInstance& cimInstance)
{
    PEG_TRACE_CSTRING(TRC_REPOSITORY, Tracer::LEVEL4, "Remove Class Origins");

    Uint32 propertyCount = cimInstance.getPropertyCount();
    for (Uint32 i = 0; i < propertyCount ; i++)
    {
        cimInstance.getProperty(i).setClassOrigin(CIMName());
    }
}

/* Filters the properties, qualifiers, and classorigin out of a single instance.
    Based on the parameters provided for propertyList, includeQualifiers,
    and includeClassOrigin, this function simply filters the properties
    qualifiers, and classOrigins out of a single instance.  This function
    was created to have a single piece of code that processes getinstance
    and enumerateInstances returns.
    @param cimInstance reference to instance to be processed.
    @param includeQualifiers Boolean defining if qualifiers to be returned.
    @param includeClassOrigin Boolean defining if ClassOrigin attribute to
        be removed from properties.
    @param propertyList If not null, defines the properties to be included in
        the instance.
*/
static void _filterInstance(
    CIMInstance& cimInstance,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
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

static void _filterClass(
    CIMClass& cimClass,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    // Remove properties based on propertylist and localOnly flag (Bug 565)
    Boolean propertyListNull = propertyList.isNull();

    // if localOnly OR there is a property list, process properties
    if ((!propertyListNull) || localOnly)
    {
        // Loop through properties to remove those that do not filter through
        // local only attribute and are not in the property list.
        Uint32 count = cimClass.getPropertyCount();
        // Work backwards because removal may be cheaper. Sint32 covers count=0
        for (Sint32 i = (count - 1); i >= 0; i--)
        {
            CIMProperty p = cimClass.getProperty(i);
            // if localOnly==true, ignore properties defined in super class
            if (localOnly && (p.getPropagated()))
            {
                cimClass.removeProperty(i);
                continue;
            }

            // propertyList NULL means all properties.  PropertyList
            // empty, none.
            // Test for removal if propertyList not NULL. The empty list option
            // is covered by fact that property is not in the list.
            if (!propertyListNull)
                if (!_containsProperty(p, propertyList))
                    cimClass.removeProperty(i);
        }
    }

    // remove methods based on localOnly flag
    if (localOnly)
    {
        Uint32 count = cimClass.getMethodCount();
        // Work backwards because removal may be cheaper.
        for (Sint32 i = (count - 1); i >= 0; i--)
        {
            CIMMethod m = cimClass.getMethod(i);

            // if localOnly==true, ignore properties defined in super class
            if (localOnly && (m.getPropagated()))
                cimClass.removeMethod(i);
        }

    }
    // If includequalifiers false, remove all qualifiers from
    // properties, methods and parameters.
    if (!includeQualifiers)
    {
        _removeAllQualifiers(cimClass);
    }
    else
    {
        // if includequalifiers and localOnly, remove nonLocal qualifiers
        if (localOnly)
        {
            _removePropagatedQualifiers(cimClass);
        }

    }


    // if ClassOrigin Flag false, remove classOrigin info from class object
    // by setting the property to Null.
    if (!includeClassOrigin)
    {
        PEG_TRACE_CSTRING(TRC_REPOSITORY, Tracer::LEVEL4,
            "Remove Class Origins");

        Uint32 propertyCount = cimClass.getPropertyCount();
        for (Uint32 i = 0; i < propertyCount ; i++)
        {
            cimClass.getProperty(i).setClassOrigin(CIMName());
        }

        Uint32 methodCount =  cimClass.getMethodCount();
        for (Uint32 i=0; i < methodCount ; i++)
        {
            cimClass.getMethod(i).setClassOrigin(CIMName());
        }
    }
}


static bool _contains(const CIMPropertyList& propertyList, const CIMName& name)
{
    for (Uint32 i = 0; i < propertyList.size(); i++)
    {
        if (propertyList[i] == name)
        {
            return true;
        }
    }

    return false;
}

static void _applyModifiedInstance(
    const MRRClass* sc,
    const CIMInstance& modifiedInstance_,
    const CIMPropertyList& propertyList,
    CIMInstance& resultInstance)
{
    CIMInstance& modifiedInstance = *((CIMInstance*)&modifiedInstance_);

    for (Uint32 i = 0; i < modifiedInstance.getPropertyCount(); i++)
    {
        CIMProperty cp = modifiedInstance.getProperty(i);
        Uint32 pos = resultInstance.findProperty(cp.getName());

        if (propertyList.isNull() || _contains(propertyList, cp.getName()))
        {
            // Reject attempts to add properties not in class:

            const MRRFeature* sf = FindFeature(sc,
                *Str(cp.getName()), MRR_FLAG_PROPERTY|MRR_FLAG_REFERENCE);

            if (!sf)
            {
                Throw((CIM_ERR_NOT_FOUND,
                    "modifyInstance() failed: unknown property: %s",
                    *Str(cp.getName())));
            }

            // Reject attempts to modify key properties:

            if (sf->flags & MRR_FLAG_KEY)
            {
                Throw((CIM_ERR_FAILED,
                    "modifyInstance() failed to modify key property: %s",
                    *Str(cp.getName())));
            }

            // Add or replace property in result instance:

            if (pos != PEG_NOT_FOUND)
            {
                resultInstance.removeProperty(pos);
            }

            resultInstance.addProperty(cp);
        }
    }
}

static void _print(const CIMInstance& ci)
{
    CIMObject co(ci);

    std::cout << co.toString() << std::endl;
}

static Once _once = PEGASUS_ONCE_INITIALIZER;
static const char* _hostName = 0;

static void _initHostName()
{
    String hn = System::getHostName();
    _hostName = strdup(*Str(hn));
}

static inline const char* _getHostName()
{
    once(&_once, _initHostName);
    return _hostName;
}

static bool _eqi(const char* s1, const char* s2)
{
    return System::strcasecmp(s1, s2) == 0;
}

static const MRRNameSpace* _findNameSpace(const char* name)
{
    for (size_t i = 0; i < _nameSpaceTableSize; i++)
    {
        const MRRNameSpace* ns = _nameSpaceTable[i];

        if (_eqi(ns->name, name))
        {
            return ns;
        }
    }

    // Not found!
    return 0;
}

static bool _isSubClass(const MRRClass* super, const MRRClass* sub)
{
    if (!super)
    {
        return true;
    }

    for (MRRClass* p = sub->super; p; p = p->super)
    {
        if (p == super)
        {
            return true;
        }
    }

    return false;
}

static inline bool _isDirectSubClass(
    const MRRClass* super,
    const MRRClass* sub)
{
    return sub->super == super;
}

static char** _makePropertyList(const CIMPropertyList& propertyList)
{
    if (propertyList.isNull())
    {
        return 0;
    }

    size_t size = propertyList.size();
    char** pl = (char**)malloc(sizeof(char*) * (size + 1));

    for (size_t i = 0; i < size; i++)
    {
        pl[i] = strdup(*Str(propertyList[i]));
    }

    pl[size] = 0;

    return pl;
}

static void _freePropertyList(char** pl)
{
    if (!pl)
    {
        return;
    }

    for (size_t i = 0; pl[i]; i++)
    {
        free(pl[i]);
    }

    free(pl);
}

static void _printPropertyList(const char* const* pl)
{
    if (!pl)
    {
        return;
    }

    for (size_t i = 0; pl[i]; i++)
    {
        printf("pl[%s]\n", pl[i]);
    }
}

static bool _contains(const Array<const MRRClass*>& x, const MRRClass* sc)
{
    Uint32 n = x.size();
    const MRRClass* const* p = x.getData();

    while (n--)
    {
        if (*p++ == sc)
        {
            return true;
        }
    }

    return false;
}

static void _associators(
    const MRRNameSpace* ns,
    const CIMName& className,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    Array<const MRRClass*>& result)
{
    // Lookup source class:

    const MRRClass* sc = FindClass(ns, *Str(className));

    if (!sc)
    {
        Throw((CIM_ERR_NOT_FOUND, "unknown class: %s", *Str(className)));
    }


    // Lookup result class (if any).

    const MRRClass* rmc = 0;

    if (!resultClass.isNull())
    {
        rmc = FindClass(ns, *Str(resultClass));

        if (!rmc)
        {
            Throw((CIM_ERR_NOT_FOUND, "unknown class: %s", *Str(resultClass)));
        }
    }

    // Convert these to UTF8 now to avoid doing so in loop below.

    Str ac(assocClass);
    Str r(role);
    Str rr(resultRole);

    // Process association classes:

    for (size_t i = 0; ns->classes[i]; i++)
    {
        MRRClass* amc = ns->classes[i];

        // Skip non-association classes:

        if (!(amc->flags & MRR_FLAG_ASSOCIATION))
        {
            continue;
        }

        // Filter by assocClass parameter:

        if (!assocClass.isNull() && !_eqi(ac, amc->name))
        {
            continue;
        }

        // Process reference properties:

        MRRFeatureInfo features[MRR_MAX_FEATURES];
        size_t size = 0;
        MergeFeatures(amc, false, MRR_FLAG_REFERENCE, features, size);

        for (size_t j = 0; j < size; j++)
        {
            const MRRFeature* sf = features[j].sf;

            // Skip non references:

            if (!(sf->flags & MRR_FLAG_REFERENCE))
            {
                continue;
            }

            const MRRReference* sr = (const MRRReference*)sf;

            // Filter by role parameter.

            if (role.size() && !_eqi(r, sf->name))
            {
                continue;
            }

            // Filter by source class:

            if (!IsA(sr->ref, sc))
            {
                continue;
            }

            // Process result reference:

            for (size_t k = 0; k < size; k++)
            {
                const MRRFeature* rmf = features[k].sf;

                // Skip the feature under consideration:

                if (rmf == sf)
                {
                    continue;
                }

                // Skip non references:

                if (!(rmf->flags & MRR_FLAG_REFERENCE))
                {
                    continue;
                }

                const MRRReference* rmr = (const MRRReference*)rmf;

                // Filter by resultRole parameter.

                if (resultRole.size() && !_eqi(rr, rmf->name))
                {
                    continue;
                }

                // Skip references not of the result class kind:

                if (rmc && !IsA(rmr->ref, rmc))
                {
                    continue;
                }

                // ATTN: should we include entire class hierarchy under
                // result class?

                // If reached, then save this one.

                if (!_contains(result, rmr->ref))
                {
                    result.append(rmr->ref);
                }
            }
        }
    }
}

static void _references(
    const MRRNameSpace* ns,
    const CIMName& className,
    const CIMName& resultClass,
    const String& role,
    Array<const MRRClass*>& result)
{
    // Lookup source class:

    const MRRClass* sc = FindClass(ns, *Str(className));

    if (!sc)
    {
        Throw((CIM_ERR_NOT_FOUND, "unknown class: %s", *Str(className)));
    }

    // Lookup result class (if any).

    const MRRClass* rmc = 0;

    if (!resultClass.isNull())
    {
        rmc = FindClass(ns, *Str(resultClass));

        if (!rmc)
        {
            Throw((CIM_ERR_NOT_FOUND, "unknown class: %s", *Str(resultClass)));
        }
    }

    // Convert these to UTF8 now to avoid doing so in loop below.

    Str r(role);

    // Process association classes:

    for (size_t i = 0; ns->classes[i]; i++)
    {
        MRRClass* amc = ns->classes[i];

        // Skip non-association classes:

        if (!(amc->flags & MRR_FLAG_ASSOCIATION))
        {
            continue;
        }

        // Filter by result class:

        if (rmc && !IsA(rmc, amc))
        {
            continue;
        }

        // Process reference properties:

        MRRFeatureInfo features[MRR_MAX_FEATURES];
        size_t size = 0;
        MergeFeatures(amc, false, MRR_FLAG_REFERENCE, features, size);

        for (size_t j = 0; j < size; j++)
        {
            const MRRFeature* sf = features[j].sf;

            // Skip non references:

            if (!(sf->flags & MRR_FLAG_REFERENCE))
            {
                continue;
            }

            const MRRReference* sr = (const MRRReference*)sf;

            // Filter by role parameter.

            if (role.size() && !_eqi(r, sf->name))
            {
                continue;
            }

            // Filter by source class:

            if (!IsA(sr->ref, sc))
            {
                continue;
            }

            // Add this one to the output:

            if (!_contains(result, amc))
            {
                result.append((MRRClass*)amc);
            }
        }
    }
}

static const MRRClass* _findMRRClass(
    const char* nameSpace,
    const char* className)
{
    // Lookup namespace:

    const MRRNameSpace* ns = _findNameSpace(nameSpace);

    if (!ns)
    {
        return 0;
    }

    return FindClass(ns, className);
}

static Array<CIMName> _enumerateClassNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance)
{
    // Lookup namespace:

    const MRRNameSpace* ns = _findNameSpace(*Str(nameSpace));

    if (!ns)
    {
        Throw((CIM_ERR_INVALID_NAMESPACE, "%s", *Str(nameSpace)));
    }

    // Lookup class:

    const MRRClass* super = 0;

    if (!className.isNull())
    {
        super = FindClass(ns, *Str(className));

        if (!super)
        {
            Throw((CIM_ERR_NOT_FOUND, "unknown class: %s", *Str(className)));
        }
    }

    // Iterate all classes looking for matches:

    Array<CIMName> result;

    for (size_t i = 0; ns->classes[i]; i++)
    {
        MRRClass* sc = ns->classes[i];

        if (deepInheritance)
        {
            if (_isSubClass(super, sc))
            {
                result.append(sc->name);
            }
        }
        else
        {
            if (_isDirectSubClass(super, sc))
            {
                result.append(sc->name);
            }
        }
    }

    return result;
}

static void _getSubClassNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Array<CIMName>& subClassNames)
{
    subClassNames = _enumerateClassNames(
        nameSpace, className, deepInheritance);
}

static Array<CIMObject> _associatorClasses(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    // Lookup namespace:

    const MRRNameSpace* ns = _findNameSpace(*Str(nameSpace));

    if (!ns)
    {
        Throw((CIM_ERR_INVALID_NAMESPACE, "%s", *Str(nameSpace)));
    }

    // Get associator schema-classes:

    Array<const MRRClass*> mcs;
    _associators(ns, className, assocClass, resultClass, role, resultRole, mcs);

    // Convert schema-classes to classes.

    Array<CIMObject> result;

    char** pl = _makePropertyList(propertyList);

    for (Uint32 i = 0; i < mcs.size(); i++)
    {
        const MRRClass* sc = mcs[i];
        CIMClass cc;

        if (MakeClass(_getHostName(), ns, sc, false, includeQualifiers,
            includeClassOrigin, pl, cc) != 0)
        {
            _freePropertyList(pl);
            Throw((CIM_ERR_FAILED, "conversion failed: %s", sc->name));
        }

        result.append(cc);
    }

    _freePropertyList(pl);
    return result;
}

static Array<CIMObjectPath> _associatorClassPaths(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole)
{
    // Lookup namespace:

    const MRRNameSpace* ns = _findNameSpace(*Str(nameSpace));

    if (!ns)
    {
        Throw((CIM_ERR_INVALID_NAMESPACE, "%s", *Str(nameSpace)));
    }

    // Get associator schema-classes:

    Array<const MRRClass*> mcs;
    _associators(ns, className, assocClass, resultClass, role, resultRole, mcs);

    // Convert schema-classes to object names:

    Array<CIMObjectPath> result;

    for (Uint32 i = 0; i < mcs.size(); i++)
    {
        result.append(CIMObjectPath(_getHostName(), nameSpace, mcs[i]->name));
    }

    return result;
}

static Array<CIMObject> _referenceClasses(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const CIMName& resultClass,
    const String& role,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    // Lookup namespace:

    const MRRNameSpace* ns = _findNameSpace(*Str(nameSpace));

    if (!ns)
    {
        Throw((CIM_ERR_INVALID_NAMESPACE, "%s", *Str(nameSpace)));
    }

    // Get reference schema-classes:

    Array<const MRRClass*> mcs;
    _references(ns, className, resultClass, role, mcs);

    // Convert schema-classes to classes.

    Array<CIMObject> result;

    char** pl = _makePropertyList(propertyList);

    for (Uint32 i = 0; i < mcs.size(); i++)
    {
        const MRRClass* sc = mcs[i];
        CIMClass cc;

        if (MakeClass(_getHostName(), ns, sc, false, includeQualifiers,
            includeClassOrigin, pl, cc) != 0)
        {
            _freePropertyList(pl);
            Throw((CIM_ERR_FAILED, "conversion failed: %s", sc->name));
        }

        result.append(cc);
    }

    _freePropertyList(pl);
    return result;
}

static Array<CIMObjectPath> _referenceClassPaths(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const CIMName& resultClass,
    const String& role)
{
    // Lookup namespace:

    const MRRNameSpace* ns = _findNameSpace(*Str(nameSpace));

    if (!ns)
    {
        Throw((CIM_ERR_INVALID_NAMESPACE, "%s", *Str(nameSpace)));
    }

    // Get reference schema-classes:

    Array<const MRRClass*> mcs;
    _references(ns, className, resultClass, role, mcs);

    // Convert schema-classes to object paths.

    Array<CIMObjectPath> result;

    for (Uint32 i = 0; i < mcs.size(); i++)
    {
        result.append(CIMObjectPath(_getHostName(), nameSpace, mcs[i]->name));
    }

    return result;
}

static CIMQualifierDecl _getQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    // Lookup namespace:

    const MRRNameSpace* ns = _findNameSpace(*Str(nameSpace));

    if (!ns)
    {
        Throw((CIM_ERR_INVALID_NAMESPACE, "%s", *Str(nameSpace)));
    }

    // Lookup qualifier:

    const MRRQualifierDecl* mqd = FindQualifierDecl(ns, *Str(qualifierName));

    if (!mqd)
    {
        Throw((CIM_ERR_NOT_FOUND,
            "unknown qualifier: %s", *Str(qualifierName)));
    }

    // Make the qualifier declaration:

    CIMQualifierDecl cqd;

    if (MakeQualifierDecl(ns, mqd, cqd) != 0)
    {
        Throw((CIM_ERR_FAILED, "conversion failed: %s", mqd->name));
    }

    return cqd;
}

static Array<CIMQualifierDecl> _enumerateQualifiers(
    const CIMNamespaceName& nameSpace)
{
    // Lookup namespace:

    const MRRNameSpace* ns = _findNameSpace(*Str(nameSpace));

    if (!ns)
    {
        Throw((CIM_ERR_INVALID_NAMESPACE, "%s", *Str(nameSpace)));
    }

    // Build the array of qualifier declarations:

    Array<CIMQualifierDecl> result;

    for (size_t i = 0; ns->qualifiers[i]; i++)
    {
        const MRRQualifierDecl* mqd = ns->qualifiers[i];
        CIMQualifierDecl cqd;

        if (MakeQualifierDecl(ns, mqd, cqd) != 0)
        {
            Throw((CIM_ERR_FAILED, "conversion failed: %s", mqd->name));
        }

        result.append(cqd);
    }

    return result;

}

static Array<CIMNamespaceName> _enumerateNameSpaces()
{
    Array<CIMNamespaceName> result;

    for (size_t i = 0; i < _nameSpaceTableSize; i++)
    {
        const MRRNameSpace* ns = _nameSpaceTable[i];
        result.append(ns->name);
    }

    return result;
}

static void _getSuperClassNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Array<CIMName>& superClassNames)
{
    superClassNames.clear();

    // Lookup namespace:

    const MRRNameSpace* ns = _findNameSpace(*Str(nameSpace));

    if (!ns)
    {
        Throw((CIM_ERR_INVALID_NAMESPACE, "%s", *Str(nameSpace)));
    }

    // Lookup class:

    const MRRClass* sc = FindClass(ns, *Str(className));

    if (!sc)
        Throw((CIM_ERR_NOT_FOUND, "unknown class: %s", *Str(className)));

    // Append superclass names:

    for (const MRRClass* p = sc->super; p; p = p->super)
    {
        superClassNames.append(p->name);
    }
}

//==============================================================================
//
// class CIMRepository:
//
//==============================================================================

static void (*_saveCallback)(const Buffer& buffer, void* data);
static void* _saveData;

static void (*_loadCallback)(Buffer& buffer, void* data);
static void* _loadData;

static void (*_initializeCallback)(CIMRepository* rep, void* data);
static void* _initializeData;

CIMRepository::CIMRepository(
    const String& repositoryRoot,
    Uint32 mode,
    RepositoryDeclContext* declContext)
{
    /* ATTN: declContext is not used here! */

    _rep = new CIMRepositoryRep;

    // Load users data if any:
    _rep->_processLoadCallback();

    // Call initialize callback if any.

    if (_initializeCallback)
        (*_initializeCallback)(this, _initializeData);
}

CIMRepository::~CIMRepository()
{
    delete _rep;
}

CIMClass CIMRepository::getClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    // Lookup namespace:

    const MRRNameSpace* ns = _findNameSpace(*Str(nameSpace));

    if (!ns)
    {
        Throw((CIM_ERR_INVALID_NAMESPACE, "%s", *Str(nameSpace)));
    }

    // Lookup class:

    const MRRClass* sc = FindClass(ns, *Str(className));

    if (!sc)
    {
        Throw((CIM_ERR_NOT_FOUND, "unknown class: %s", *Str(className)));
    }

    // Build property list:

    char** pl = _makePropertyList(propertyList);

    // Make class:

    CIMClass cc;

    if (MakeClass(_getHostName(), ns, sc, localOnly, includeQualifiers,
        includeClassOrigin, pl, cc) != 0)
    {
        _freePropertyList(pl);
        Throw((CIM_ERR_FAILED, "conversion failed: %s", sc->name));
    }

    _freePropertyList(pl);
    return cc;
}

CIMInstance CIMRepository::getInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    Uint32 pos = _rep->_findInstance(nameSpace, instanceName);

    if (pos == PEG_NOT_FOUND)
    {
        Throw((CIM_ERR_NOT_FOUND, "%s", *Str(instanceName)));
    }

    CIMInstance cimInstance = _rep->_rep[pos].second.clone();

    _filterInstance(
        cimInstance,
        includeQualifiers,
        includeClassOrigin,
        propertyList);

    return cimInstance;
}

void CIMRepository::deleteClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    Throw((CIM_ERR_NOT_SUPPORTED, "deleteClass()"));
}

void CIMRepository::deleteInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName)
{
    Uint32 pos = _rep->_findInstance(nameSpace, instanceName);

    if (pos == PEG_NOT_FOUND)
    {
        Throw((CIM_ERR_NOT_FOUND, "%s", *Str(instanceName)));
    }

    _rep->_rep.remove(pos);
    _rep->_processSaveCallback();
}

void CIMRepository::createClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass)
{
    Throw((CIM_ERR_NOT_SUPPORTED, "createClass()"));
}

CIMObjectPath CIMRepository::createInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& newInstance)
{
    // Resolve the instance first:

    CIMInstance ci(newInstance.clone());
    CIMConstClass cc;
    RepositoryDeclContext context;
    context.setRepository(this);
    Resolver::resolveInstance(ci, &context, nameSpace, cc, false);
    CIMObjectPath cop = ci.buildPath(cc);

    ci.setPath(cop);

    // Reject if an instance with this name already exists:

    if (_rep->_findInstance(nameSpace, cop) != PEG_NOT_FOUND)
    {
        Throw((CIM_ERR_ALREADY_EXISTS, "%s", *Str(cop)));
    }

    // Add instance to array:

    _rep->_rep.append(NamespaceInstancePair(nameSpace, ci));
    _rep->_processSaveCallback();

    return cop;
}

void CIMRepository::modifyClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& modifiedClass)
{
    Throw((CIM_ERR_NOT_SUPPORTED, "modifyClass()"));
}

void CIMRepository::modifyInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& modifiedInstance,
    Boolean includeQualifiers,
    const CIMPropertyList& propertyList)
{
    const CIMObjectPath& cop = modifiedInstance.getPath();
    CIMName className = cop.getClassName();

    // Get the schema-class for this instance.

    const MRRClass* sc = _findMRRClass(*Str(nameSpace), *Str(className));

    if (!sc)
    {
        Throw((CIM_ERR_FAILED,
            "modifyInstance() failed: unknown class: %s:%s",
            *Str(nameSpace), *Str(className)));
    }

    // Get original instance to be modified:

    Uint32 pos = _rep->_findInstance(nameSpace, cop);

    if (pos == PEG_NOT_FOUND)
    {
        Throw((CIM_ERR_NOT_FOUND,
            "modifyInstance() failed: unknown instance: %s",
            *Str(cop.toString())));
    }

    CIMInstance resultInstance = _rep->_rep[pos].second.clone();

    // Apply features of modifiedInstance to result instance.

    _applyModifiedInstance(sc, modifiedInstance, propertyList, resultInstance);

    // Resolve the instance.

    CIMConstClass cc;
    RepositoryDeclContext context;
    context.setRepository(this);
    Resolver::resolveInstance(resultInstance, &context, nameSpace, cc, false);

    // Replace original instance.

    _rep->_rep[pos].second = resultInstance;
    _rep->_processSaveCallback();
}

Array<CIMClass> CIMRepository::enumerateClasses(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin)
{
    // Lookup namespace:

    const MRRNameSpace* ns = _findNameSpace(*Str(nameSpace));

    if (!ns)
    {
        Throw((CIM_ERR_INVALID_NAMESPACE, "%s", *Str(nameSpace)));
    }

    // Lookup class:

    const MRRClass* super = 0;

    if (!className.isNull())
    {
        super = FindClass(ns, *Str(className));

        if (!super)
        {
            Throw((CIM_ERR_NOT_FOUND, "unknown class: %s", *Str(className)));
        }
    }

    // Iterate all classes looking for matches:

    Array<CIMClass> result;

    for (size_t i = 0; ns->classes[i]; i++)
    {
        MRRClass* sc = ns->classes[i];

        bool flag = false;

        if (deepInheritance)
        {
            if (_isSubClass(super, sc))
                flag = true;
        }
        else
        {
            if (_isDirectSubClass(super, sc))
                flag = true;
        }

        if (flag)
        {
            CIMClass cc;

            if (MakeClass(_getHostName(), ns, sc, localOnly, includeQualifiers,
                includeClassOrigin, 0, cc) != 0)
            {
                Throw((CIM_ERR_FAILED, "conversion failed: %s", sc->name));
            }

            result.append(cc);
        }
    }

    return result;
}

Array<CIMName> CIMRepository::enumerateClassNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance)
{
    return _enumerateClassNames(nameSpace, className, deepInheritance);
}

Array<CIMInstance> CIMRepository::enumerateInstancesForSubtree(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    // Form array of classnames for this class and descendent classes:

    Array<CIMName> classNames;
    classNames.append(className);
    _getSubClassNames(nameSpace, className, true, classNames);

    // Get all instances for this class and all descendent classes

    Array<CIMInstance> result;

    for (Uint32 i = 0; i < classNames.size(); i++)
    {
        Array<CIMInstance> instances = enumerateInstancesForClass(
            nameSpace, classNames[i], includeQualifiers,
            includeClassOrigin, propertyList);

        for (Uint32 i = 0 ; i < instances.size(); i++)
        {
            _filterInstance(
                instances[i],
                includeQualifiers,
                includeClassOrigin,
                propertyList);
        }

        result.appendArray(instances);
    }

    return result;
}

Array<CIMInstance> CIMRepository::enumerateInstancesForClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    Array<CIMInstance> result;

    for (Uint32 i = 0; i < _rep->_rep.size(); i++)
    {
        if (_rep->_rep[i].first != nameSpace)
            continue;

        CIMInstance& ci = _rep->_rep[i].second;

        if (ci.getPath().getClassName() == className)
        {
            CIMInstance tmp = ci.clone();

            _filterInstance(
                tmp,
                includeQualifiers,
                includeClassOrigin,
                propertyList);

            result.append(tmp);
        }
    }

    return result;
}

Array<CIMObjectPath> CIMRepository::enumerateInstanceNamesForSubtree(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    // Form array of classnames for this class and descendent classes:

    Array<CIMName> classNames;
    classNames.append(className);
    _getSubClassNames(nameSpace, className, true, classNames);

    // Get all instances for this class and all descendent classes

    Array<CIMObjectPath> result;

    for (Uint32 i = 0; i < classNames.size(); i++)
    {
        Array<CIMObjectPath> paths = enumerateInstanceNamesForClass(
            nameSpace, classNames[i]);

        result.appendArray(paths);
    }

    return result;
}

Array<CIMObjectPath> CIMRepository::enumerateInstanceNamesForClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    Array<CIMObjectPath> result;

    for (Uint32 i = 0; i < _rep->_rep.size(); i++)
    {
        if (_rep->_rep[i].first != nameSpace)
        {
            continue;
        }

        CIMInstance& ci = _rep->_rep[i].second;

        if (ci.getPath().getClassName() == className)
        {
            result.append(ci.getPath());
        }
    }

    return result;
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
    if (objectName.getKeyBindings().size() == 0)
    {
        return _associatorClasses(
            nameSpace,
            objectName.getClassName(),
            assocClass,
            resultClass,
            role,
            resultRole,
            includeQualifiers,
            includeClassOrigin,
            propertyList);
    }
    else
    {
        Throw((CIM_ERR_NOT_SUPPORTED, "associators()"));
        return Array<CIMObject>();
    }
}

Array<CIMObjectPath> CIMRepository::associatorNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole)
{
    if (objectName.getKeyBindings().size() == 0)
    {
        return _associatorClassPaths(
            nameSpace,
            objectName.getClassName(),
            assocClass,
            resultClass,
            role,
            resultRole);
    }
    else
    {
        Throw((CIM_ERR_NOT_SUPPORTED, "associatorNames()"));
        return Array<CIMObjectPath>();
    }
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
    if (objectName.getKeyBindings().size() == 0)
    {
        return _referenceClasses(
            nameSpace,
            objectName.getClassName(),
            resultClass,
            role,
            includeQualifiers,
            includeClassOrigin,
            propertyList);
    }
    else
    {
        Throw((CIM_ERR_NOT_SUPPORTED, "references()"));
        return Array<CIMObject>();
    }
}

Array<CIMObjectPath> CIMRepository::referenceNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role)
{
    if (objectName.getKeyBindings().size() == 0)
    {
        return _referenceClassPaths(
            nameSpace,
            objectName.getClassName(),
            resultClass,
            role);
    }
    else
    {
        Throw((CIM_ERR_NOT_SUPPORTED, "referenceNames()"));
        return Array<CIMObjectPath>();
    }
}

CIMValue CIMRepository::getProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName)
{
    CIMInstance ci = getInstance(
        nameSpace, instanceName, true, true, CIMPropertyList());

    Uint32 pos = ci.findProperty(propertyName);

    if (pos == PEG_NOT_FOUND)
    {
        Throw((CIM_ERR_NO_SUCH_PROPERTY, "%s", *Str(propertyName)));
    }

    return ci.getProperty(pos).getValue();
}

void CIMRepository::setProperty(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName,
    const CIMValue& newValue)
{
    CIMInstance ci(instanceName.getClassName());
    ci.addProperty(CIMProperty(propertyName, newValue));
    ci.setPath(instanceName);

    Array<CIMName> tmp;
    tmp.append(propertyName);
    CIMPropertyList properties(tmp);

    modifyInstance(nameSpace, ci, false, properties);
}

CIMQualifierDecl CIMRepository::getQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    return _getQualifier(nameSpace, qualifierName);
}

void CIMRepository::setQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMQualifierDecl& qualifierDecl)
{
    Throw((CIM_ERR_NOT_SUPPORTED, "setQualifier()"));
}

void CIMRepository::deleteQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    Throw((CIM_ERR_NOT_SUPPORTED, "deleteQualifier()"));
}

Array<CIMQualifierDecl> CIMRepository::enumerateQualifiers(
    const CIMNamespaceName& nameSpace)
{
    return _enumerateQualifiers(nameSpace);
}

void CIMRepository::createNameSpace(
    const CIMNamespaceName& nameSpace,
    const NameSpaceAttributes& attributes)
{
    Throw((CIM_ERR_NOT_SUPPORTED, "createNameSpace()"));
}

void CIMRepository::modifyNameSpace(
    const CIMNamespaceName& nameSpace,
    const NameSpaceAttributes& attributes)
{
    Throw((CIM_ERR_NOT_SUPPORTED, "modifyNameSpace()"));
}

Array<CIMNamespaceName> CIMRepository::enumerateNameSpaces() const
{
    return _enumerateNameSpaces();
}

void CIMRepository::deleteNameSpace(
    const CIMNamespaceName& nameSpace)
{
    Throw((CIM_ERR_NOT_SUPPORTED, "deleteNameSpace()"));
}

Boolean CIMRepository::getNameSpaceAttributes(
    const CIMNamespaceName& nameSpace,
    NameSpaceAttributes& attributes)
{
    attributes.clear();
    return false;
}

Boolean CIMRepository::isDefaultInstanceProvider()
{
    return true;
}

void CIMRepository::getSubClassNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Array<CIMName>& subClassNames) const
{
    _getSubClassNames(nameSpace, className, deepInheritance, subClassNames);
}

void CIMRepository::getSuperClassNames(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Array<CIMName>& superClassNames) const
{
    _getSuperClassNames(nameSpace, className, superClassNames);
}

Boolean CIMRepository::isRemoteNameSpace(
    const CIMNamespaceName& nameSpace,
    String& remoteInfo)
{
    return false;
}

#ifdef PEGASUS_DEBUG
void CIMRepository::DisplayCacheStatistics()
{
}
#endif

Uint32 CIMRepositoryRep::_findInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName)
{
    for (Uint32 i = 0; i < _rep.size(); i++)
    {
        if (_rep[i].first == nameSpace &&
            _rep[i].second.getPath() == instanceName)
        {
            return i;
        }
    }

    return PEG_NOT_FOUND;
}

void MRRInstallSaveCallback(
    void (*callback)(const Buffer& buffer, void* data),
    void * data)
{
    _saveCallback = callback;
    _saveData = data;
}

void MRRInstallLoadCallback(
    void (*callback)(Buffer& buffer, void* data),
    void * data)
{
    _loadCallback = callback;
    _loadData = data;
}

void CIMRepositoryRep::_processSaveCallback()
{
    if (!_saveCallback)
        return;

    Buffer out;

    for (Uint32 i = 0; i < _rep.size(); i++)
    {
        MRRSerializeNameSpace(out, _rep[i].first);
        MRRSerializeInstance(out, _rep[i].second);
    }

    (*_saveCallback)(out, _saveData);
}

void CIMRepositoryRep::_processLoadCallback()
{
    if (!_loadCallback)
        return;

    Buffer in;
    (*_loadCallback)(in, _loadData);
    size_t pos = 0;

    while (pos != in.size())
    {
        CIMNamespaceName nameSpace;

        if (MRRDeserializeNameSpace(in, pos, nameSpace) != 0)
        {
            return;
        }

        CIMInstance cimInstance;

        if (MRRDeserializeInstance(in, pos, cimInstance) != 0)
        {
            return;
        }

        _rep.append(NamespaceInstancePair(nameSpace, cimInstance));
    }
}

Boolean MRRAddNameSpace(const MRRNameSpace* nameSpace)
{
    if (!nameSpace)
    {
        return false;
    }

    if (_nameSpaceTableSize == _MAX_NAMESPACE_TABLE_SIZE)
    {
        return false;
    }

    if (_findNameSpace(nameSpace->name))
    {
        return false;
    }

    _nameSpaceTable[_nameSpaceTableSize++] = nameSpace;

    return true;
}

void MRRInstallInitializeCallback(
    void (*callback)(CIMRepository* repository, void * data),
    void *data)
{
    _initializeCallback = callback;
    _initializeData = data;
}

//==============================================================================
//
// Non-locking forms of repository methods.
//
//==============================================================================

CIMClass CIMRepository::_getClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    Boolean clone)                 // Not used by MRR.
{
    return getClass(nameSpace, className, localOnly, includeQualifiers,
        includeClassOrigin, propertyList);
}

CIMInstance CIMRepository::_getInstance(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& instanceName,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    Boolean resolveInstance)
{
    return getInstance(nameSpace, instanceName, includeQualifiers,
        includeClassOrigin, propertyList);
}

void CIMRepository::_createClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass)
{
    createClass(nameSpace, newClass);
}

CIMObjectPath CIMRepository::_createInstance(
    const CIMNamespaceName& nameSpace,
    const CIMInstance& newInstance)
{
    return createInstance(nameSpace, newInstance);
}

void CIMRepository::_modifyClass(
    const CIMNamespaceName& nameSpace,
    const CIMClass& modifiedClass)
{
    modifyClass(nameSpace, modifiedClass);
}

Array<CIMObjectPath> CIMRepository::_associatorNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole)
{
    return associatorNames(nameSpace, objectName, assocClass, resultClass,
        role, resultRole);
}

Array<CIMObjectPath> CIMRepository::_referenceNames(
    const CIMNamespaceName& nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role)
{
    return referenceNames(nameSpace, objectName, resultClass, role);
}

CIMQualifierDecl CIMRepository::_getQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMName& qualifierName)
{
    return getQualifier(nameSpace, qualifierName);
}

void CIMRepository::_setQualifier(
    const CIMNamespaceName& nameSpace,
    const CIMQualifierDecl& qualifierDecl)
{
    return setQualifier(nameSpace, qualifierDecl);
}

PEGASUS_NAMESPACE_END
