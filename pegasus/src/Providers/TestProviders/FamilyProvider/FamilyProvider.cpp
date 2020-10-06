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

/*
    This provider implements the TST_Family mof in the ../load directory.  It
    implements the dynamic classes.  It was created to test the capabilities
    of associations with classes and subclasses and association classes and
    subclasses.
    It implements the instance and associator functions for the classes
    TST_PersonDynamic
    TST_LineageDynamic
    TST_LabeledLineageDynamic

    The provider works by creating instances during intitialization and then
    responding to all of the instance and association commands for these
    functions.
    Note that it also handles create Instance and modify Instance.

    This was designed as a sample provider for the association functions and a
    test of some of the complexities of building association providers.

    The Faimily comes from the fact that the TST_Family mof is built around
    the concepts of a family with people, lineage between people, etc.

    Includes all instance operations and association operations on these
    classes and properly handles propertyLists. This includes the modify, create
    and delete functions for all of the classes but since a number of tests
    are based on the predefined counts, using these functions today in the
    test environment could break existing tests.

    TODO:
    1. Eliminate the _cimboject arrays (_instanceNames, etc.) they were
    a carryover from the original intance provider and are not necessary since
    all of the required information is in the namedinstance.

    2. Clone the cimobject path on responses. Right now it is not cloned.

    3. Implement the association subclasses

    4. Modify the intialize so that it uses the buildInstance from the class
    library and therefore gets the qualifiers, etc. so that the
    includequalifiers and classorigin functions work.

    5. clean up displays, diagnostics so more readable.

    6. review issues of requiring the key property on creates.

    7. Need to complete the references with host name and namespace - DONE.

    8. Use only a single instance list representing the built instances
    to make the code simpler and more dynamic. This will make the provider much
    more general and maintainable.
*/

#include <Pegasus/Common/Tracer.h>

#include "FamilyProvider.h"

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

//***************************************************
// Debug Tools
//***************************************************

#define CDEBUG(X)

#if 0
#define CDEBUG(X) PEGASUS_STD(cout) << "=== FamilyProvider " << \
    X << PEGASUS_STD(endl)
#endif

//*********************************************
//   Constants
//********************************************
// Class Names used by this Provider

const CIMName personDynamicClassName = "TST_PersonDynamic";
const CIMName personDynamicSubclassClassName = "TST_PersonDynamicSubClass";
const CIMName lineageDynamicAssocClassName = "TST_LineageDynamic";
const CIMName labeledLineageDynamicAssocClassName = "TST_LabeledLineageDynamic";

// The namespace in which we expect to reside.
// We need to think out some of the issues with this
String nameSpace = "test/TestProvider";

// Name of this provider
String FamilyProviderName = "FamilyProvider";

// Defines to serve as the ENUM for class selection for
// operations.

enum targetClass{
     TST_PERSONDYNAMIC = 1,
     TST_LINEAGEDYNAMIC = 2,
     TST_LABELEDLINEAGEDYNAMIC = 3,
     TST_PERSONDYNAMICSUBCLASS = 4
};

//**************************************************
//          Support Functions
//**************************************************

String _showBool(Boolean x)
{
    return(x? "true" : "false");
}
String _showPathArray(Array<CIMObjectPath>& p)
{
    String rtn;
    for (Uint32 i = 0 ; i < p.size() ; i++)
    {
        if (i > 0)
            rtn.append(" ");
        rtn.append(p[i].toString());
    }
    return(rtn);
}

static targetClass _verifyValidClassInput(const CIMName& className)
{
    if (className.equal(personDynamicClassName))
        return TST_PERSONDYNAMIC;

    if (className.equal(personDynamicSubclassClassName))
        return TST_PERSONDYNAMICSUBCLASS;
    if (className.equal(lineageDynamicAssocClassName))
        return TST_LINEAGEDYNAMIC;

    if (className.equal(labeledLineageDynamicAssocClassName))
        return TST_LABELEDLINEAGEDYNAMIC;

    throw CIMNotSupportedException
            (className.getString() + " not supported by " + FamilyProviderName);
}

static targetClass _verifyValidAssocClassInput(const CIMName& className)
{
    if (className.equal(lineageDynamicAssocClassName))
        return TST_LINEAGEDYNAMIC;

    if (className.equal(labeledLineageDynamicAssocClassName))
        return TST_LABELEDLINEAGEDYNAMIC;
    throw CIMNotSupportedException
        (className.getString() + " not supported by Family Provider");
}

/** returns a local (no namespace or host component) version of the
    input CIMObjectPath.
    @param p CIMObjectPath input
    @returns CIMObjectPath object with only className and keybinding
    components
*/
CIMObjectPath _makeRefLocal(const CIMObjectPath& path)
{
    CIMObjectPath rtn(path);
    rtn.setHost(String());
    rtn.setNameSpace(CIMNamespaceName());
    /* return (CIMObjectPath(String(),
        CIMNamespaceName(),
        path.getClassName(),
        path.getKeyBindings()));*/
    return(rtn);
}

String _toStringPropertyList(const CIMPropertyList& pl)
{
    String tmp;
    for (Uint32 i = 0; i < pl.size() ; i++)
    {
        if (i > 0)
            tmp.append(", ");
        tmp.append(pl[i].getString());
    }
    return(tmp);
}

String _showPropertyList(const CIMPropertyList& pl)
{
    if (pl.isNull())
        return("NULL");

    String tmp;

    tmp.append((pl.size() == 0) ? "Empty" : _toStringPropertyList(pl));
    return(tmp);
}

/** determine of the the input property is in the list.
    @param property list
    @param name of property to compare
*/
Boolean _returnThisProperty(const CIMPropertyList& pl, const CIMName& pn)
{
    if (pl.isNull())
        return(true);
    if (pl.size() == 0)
        return(false);

    for (Uint32 i = 0; i < pl.size() ; i++)
    {
        // if name found in propertyList return true
        if (pn.equal(pl[i].getString()))
            return(true);
    }
    return(false);
}
/*
void FamilyProvider::_setCompleteObjectPath(CIMInstance & instance)
{
    CIMObjectPath p = instance.getPath();
    p.setHost(_hostname);
    p.setNameSpace(nameSpace);
    instance.setPath(p);
}
*/
/* set the hostname and namespace fields into the cimobjectpath
   of the defined instance
*/

void setCompleteInstancePath(CIMInstance& instance,
                           const CIMObjectPath& inputPath)
{
    CIMObjectPath p = instance.getPath();

    p.setHost(inputPath.getHost());
    p.setNameSpace(inputPath.getNameSpace());
    CDEBUG("setCompleteInstancePath inputPath" << inputPath.toString() <<
        "path= " << p.toString());
    instance.setPath(p);
}

/* Clone the path and set the host and namespace names from
   the input path into it.
   @param path CIMObjectPath for this object
   @param inputPath CIMObjectPath input with operation. Expected
   to contain a hostname and namespace name
   @return cloned path with host and namespace set from
   inputpath
*/
CIMObjectPath setCompleteObjectPath(const CIMObjectPath& path,
                                const CIMObjectPath& inputPath)
{
    CIMObjectPath outpath(path);
    outpath.setHost(inputPath.getHost());
    outpath.setNameSpace(inputPath.getNameSpace());
    CDEBUG("setCompleteObjectPath path= " << outpath.toString());
    return(outpath);
}

/** clone the input instance.
    @return cloned instance with object path
    complete.
*/
CIMInstance _clone(const CIMInstance& instance,
    const CIMObjectPath& inputPath)
{
    // Copy of instance.
    CIMInstance rtnInstance = instance.clone();

    setCompleteInstancePath(rtnInstance, inputPath);
    CDEBUG("Instance path= " << rtnInstance.getPath().toString());
    return(rtnInstance);
}

/** _filterInstancesToTargetPaths - Filters one associaton and returns
    references that represent the result of filtering on resultclass and role.
    Any reference that matches the resultclass and role and not the target is
    returned
    @param assocInstance - The association instance being processed.
    @param targetObjectPath - The original target. This is required since this
    is the one reference we don't want.
    @resultClass - The resultClass we want to filter on
    @resultRole  - The result role we want to filter on
    @return - returns the CIMObjectPaths that represent the other side of the
    association that pass the resultClass and resultRole filters.
 */
Array<CIMObjectPath> _filterAssocInstanceToTargetPaths(
    const CIMInstance & assocInstance,
    const CIMObjectPath & targetObjectPath,
    const CIMName resultClass,
    const String resultRole)
{
    Array<CIMObjectPath> returnPaths;
    CDEBUG("_filterAssocInst. path= " << targetObjectPath.toString()
        << " resultClass= "
        << (const char*)resultClass.getString().getCString()
        << " result role= " << resultRole);
    // get all reference properties except for target.
    for (Uint32 i = 0 ; i < assocInstance.getPropertyCount() ; i++)
    {
        CIMConstProperty p = assocInstance.getProperty(i);

        if (p.getType() == CIMTYPE_REFERENCE)
        {
            CIMValue v = p.getValue();
            CIMObjectPath path;
            v.get(path);

            if (!targetObjectPath.identical(path))
            {
                if (resultClass.isNull() || resultClass == path.getClassName())
                {
                    if ((resultRole.size() == 0) ||
                        (CIMName(resultRole) == p.getName()))
                    {
                        returnPaths.append(path);
                    }
                }
            }
        }
    }
    CDEBUG("_filterAssoc PathsReturned. Count = " << returnPaths.size()
        << " " << _showPathArray(returnPaths));
    return( returnPaths );
}

/** Test for valid CIMReferences from an association instance. If there is a
    role property, gets all but the role property.
    @param target - The target path for the association. Localization assumed.
    @param instance - The association class instance we are searching for
    references
    @param role The role we require. If there is no role filter, the value is
        an empty string.
    @return - returns Boolean true if target is found in a reference that is
    the same role
 */
Boolean _isInstanceValidReference(
    const CIMObjectPath& target,
    CIMInstance& instance,
    const String& role)
{
    // Test if role parameter is valid property.
    Uint32 pos;
    if (role.size() != 0)
    {
        // Test if property with this role exists.
        if ((pos = instance.findProperty(role)) == PEG_NOT_FOUND)
            throw CIMException(CIM_ERR_INVALID_PARAMETER);

         // Check to be sure this is a reference property
         // This test may not be necessary. Combine it into the loop.
         if (instance.getProperty(pos).getType() != CIMTYPE_REFERENCE)
             throw CIMException(CIM_ERR_INVALID_PARAMETER);
    }

    //Now search instance for all reference properties
    for (Uint32 j = 0; j < instance.getPropertyCount() ; j++)
    {
        const CIMProperty p = instance.getProperty(j);
        if (p.getType() == CIMTYPE_REFERENCE)
        {
            // If there is no role or the role is the same as this property name
            CIMValue v = p.getValue();
            CIMObjectPath path;
            v.get(path);

            // if no role or role == this role and target = this path, rtn true.
            if ((role.size() == 0) || (CIMName(role) == p.getName()))
            {
                if (target.identical(path))
                    return(true);
            }
        }
    }
    return( false );
}

Boolean _isValidObjectClass(CIMObjectPath)
{
    return true;
}

/** Filters the input list of instances (which contain path info)
    using theclass and keybinding components from the input
    objectName, the role.
    TBD - filter inputlist against role (if not null) and result
    class (if not null) to determine association instances that
    have this role representing this objectName
    @param targetAssociationInstanceList Array<CIMInstance> to be
    filtered.
    @param objectName CIMobjectPath representing the target path input.
    @param resultClass CIMName representing the association class to
    be kept.  Ignored if empty. Note that if this refered to as association
    class in associators calls.
    @param role String representing the reference role to be kept.
    @return Array<CIMInstance> with the instances from the
    targetAssociaitonInstanceList that pass the filters.
*/
Array<CIMInstance> _filterReferenceNames(
    const Array<CIMInstance>& targetAssociationInstanceList,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role)
{
    CDEBUG("_filterReferenceNames. objName= " << objectName.toString()
        << " resultClass= "
        << (const char*)resultClass.getString().getCString()
        << " role= " << role);
    // Build a host and namespace independent reference to the target objectname
    CIMObjectPath targetReference = CIMObjectPath(
                            String(),
                            CIMNamespaceName(),
                            objectName.getClassName(),
                            objectName.getKeyBindings());

    Array<CIMInstance> foundList;

    for (Uint32 i = 0 ; i < targetAssociationInstanceList.size() ; i++)
    {
        CIMInstance instance = targetAssociationInstanceList[i];
        if (resultClass.isNull() || resultClass.equal(instance.getClassName()))
        {
            // if this association instance has this role in targetReference,
            // true
            if (_isInstanceValidReference(targetReference, instance, role))
                {
                    foundList.append(instance);
                }
        }
    }
    CDEBUG("_filterReferenceNames return. Count= " << foundList.size());
    return( foundList );
}

/*  Build an instance of the class from the input properties and set the path.
    Note that path is set without host and namespace and these are added at
    the last minute after prep for response.
    @param thisClass CIMClass that this instance is created from
    @param name String Data for name property
    ...
    @return CIMInstance built from the input.
*/
CIMInstance _buildPersonDynamicInstance(
    const CIMClass& thisClass,
    const String& name,
    const String& secondProperty,
    const Uint32 counterProperty)
{
    CIMInstance instance(personDynamicClassName);
    instance.addProperty(CIMProperty("Name", name));   // key
    instance.addProperty(CIMProperty("secondProperty", secondProperty));
    instance.addProperty(CIMProperty("instanceCounter", counterProperty));
    CIMObjectPath p = instance.buildPath(thisClass);
    instance.setPath(p);
    return(instance);
}


CIMInstance _buildPersonDynamicSubClassInstance(
    const CIMClass& thisClass,
    const String& name,
    const String& secondProperty,
    const Uint32 counterProperty,
    const String& thirdProperty)
{
    CIMInstance instance(personDynamicSubclassClassName);
    instance.addProperty(CIMProperty("Name", name));   // key
    instance.addProperty(CIMProperty("secondProperty", secondProperty));
    instance.addProperty(CIMProperty("instanceCounter", counterProperty));
    instance.addProperty(CIMProperty("thirdProperty", thirdProperty));

    CIMObjectPath p = instance.buildPath(thisClass);
    instance.setPath(p);
    return(instance);
}
CIMInstance _buildInstanceLineageDynamic(
    const CIMClass& thisClass,
    const CIMObjectPath& parent,
    const CIMObjectPath& child)
{
    // Note that this is a nasty assumption. May be a different namespace. Need
    // to build dynamically
    // This namespacename is used to set the namespace in the references.
    // We should set it dynamically.

    //String nameSpace = "test/TestProvider";

    CIMName thisClassReference = personDynamicClassName;
    CIMName assocClassName = "TST_LineageDynamic";
    CIMInstance instance(assocClassName);
    //parent.setHost(_hostname);
    //parent.setNameSpace(nameSpace);
    instance.addProperty(CIMProperty("parent", parent,0,thisClassReference));
    instance.addProperty(CIMProperty("child", child, 0, thisClassReference));
    CIMObjectPath p = instance.buildPath(thisClass);
    instance.setPath(p);
    return(instance);
}

CIMInstance _buildInstanceLabeledLineageDynamic(
    const CIMClass& thisClass,
    const CIMObjectPath& parent,
    const CIMObjectPath& child,
    const String& label)
{
    // Note that this is a nasty assumption. May be a different namespace. Need
    // to build dynamically
    // This namespacename is used to set the namespace in the references.
    // We should set it dynamically.

    CIMName thisClassReference = personDynamicClassName;
    CIMName assocClassName = "TST_LabeledLineageDynamic";
    CIMInstance instance(assocClassName);
    instance.addProperty(CIMProperty("parent", parent,0,thisClassReference));
    instance.addProperty(CIMProperty("child", child, 0, thisClassReference));
    instance.addProperty(CIMProperty("label", label));
    CIMObjectPath p = instance.buildPath(thisClass);
    instance.setPath(p);
    return(instance);
}

/**************************** FamilyProvider class Implementation*********/
FamilyProvider::FamilyProvider()
{
}

FamilyProvider::~FamilyProvider()
{
}

/* get the defined class from the repository.
    @param className CIMName name of the class to get
    @return CIMClass with the class or unitialized if
    there was an error in the getClass
*/
CIMClass FamilyProvider::_getClass(const CIMName& className)
{
    CIMClass c;

    try
    {
        c = _cimomHandle.getClass(
            OperationContext(),
            nameSpace,
            CIMName(className),
            false,
            true,
            true,
            CIMPropertyList());
    }
    catch (CIMException& e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "FamilyProvider GetClass operation failed: Class %s. Msg %s",
            (const char*) className.getString().getCString(),
            (const char*) e.getMessage().getCString()));
    }
    return c;
}

void FamilyProvider::initialize(CIMOMHandle & cimom)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "FamilyProvider::initialize");

    _initError = false;
    _cimomHandle = cimom;
    {
        /*
        // Create the classes so we have something to build from.
        */

        CIMClass class1 = _getClass(personDynamicClassName);
        if (class1.isUninitialized())
        {
            _initError = true;
        }
        _personDynamicClass = class1;

        // Get the subclass from the repository.
        CIMClass class2 = _getClass(personDynamicSubclassClassName);
        if (class1.isUninitialized())
        {
            _initError = true;
        }
        _personDynamicSubclass = class2;

        // Get the association class
        CIMClass a1 = _getClass(lineageDynamicAssocClassName);
        if (a1.isUninitialized())
            _initError = true;

        _assocClass = a1;

        // Get the second association class.
        CIMClass a2 = _getClass(lineageDynamicAssocClassName);
        _assocLabeledClass = a2;

        if (a2.isUninitialized())

            _initError = true;

    }

    // Do not try to initialize instances if class initialization failed.
    if (_initError)
    {
        return;
    }

    // build the instances of personDynamic
    Uint32 Father = _instances.size();
    _instances.append(_buildPersonDynamicInstance(_personDynamicClass,
        String("Father"), String("one"), 1));
    Uint32 Mother = _instances.size();
    _instances.append(_buildPersonDynamicInstance(_personDynamicClass,
        String("Mother"), String("two"), 2));
    Uint32 Son1 = _instances.size();
    _instances.append(_buildPersonDynamicInstance(_personDynamicClass,
        String("Son1"), String( "three"), 3));
    Uint32 Son2 = _instances.size();
    _instances.append(_buildPersonDynamicInstance(_personDynamicClass,
        String("Son2"), String( "four"), 4));
    Uint32 Daughter1 = _instances.size();
    _instances.append(_buildPersonDynamicInstance(_personDynamicClass,
        String("Daughter1"), String( "five"), 5));

    for (Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        // Create Instance Names
        _instanceNames.append(_instances[i].buildPath(_personDynamicClass));
    }
    _instancesSubclass.append(_buildPersonDynamicSubClassInstance(
        _personDynamicSubclass,
        String("AnotherKid"), String("six"), 6, "SubclassInstance1"));

    //
    // Now make the instances for the associations
    //
    {
        CIMName thisClassReference = "TST_PersonDynamic";
        CIMName assocClassName = "TST_LineageDynamic";

        _instancesLineageDynamic.append(_buildInstanceLineageDynamic(
            _assocClass, _instanceNames[Father],_instanceNames[Son1]));
        _instancesLineageDynamic.append(_buildInstanceLineageDynamic(
            _assocClass, _instanceNames[Father],_instanceNames[Son2]));
        _instancesLineageDynamic.append(_buildInstanceLineageDynamic(
            _assocClass, _instanceNames[Father],_instanceNames[Daughter1]));
        _instancesLineageDynamic.append(_buildInstanceLineageDynamic(
            _assocClass, _instanceNames[Mother],_instanceNames[Son1]));
        _instancesLineageDynamic.append(_buildInstanceLineageDynamic(
            _assocClass, _instanceNames[Mother],_instanceNames[Son2]));
        _instancesLineageDynamic.append(_buildInstanceLineageDynamic(
            _assocClass, _instanceNames[Mother],_instanceNames[Daughter1]));

        for (Uint32 i = 0, n = _instancesLineageDynamic.size(); i < n; i++)
        {
            // Create Instance Names
            _instanceNamesLineageDynamic.append(
                _instancesLineageDynamic[i].buildPath(assocClassName));
        }
    }

    {
        CIMName thisClassReference = "TST_PersonDynamic";
        CIMName assocLabeledClassName = "TST_LabeledLineageDynamic";

        _instancesLabeledLineageDynamic.append(
            _buildInstanceLabeledLineageDynamic(
                _assocLabeledClass, _instanceNames[Father],
                _instanceNames[Son1], String("one")));
        _instancesLabeledLineageDynamic.append(
            _buildInstanceLabeledLineageDynamic(
                _assocLabeledClass, _instanceNames[Father],
                _instanceNames[Son2], String("two")));
        _instancesLabeledLineageDynamic.append(
            _buildInstanceLabeledLineageDynamic(
                _assocLabeledClass, _instanceNames[Father],
                _instanceNames[Daughter1], String("three")));
        _instancesLabeledLineageDynamic.append(
            _buildInstanceLabeledLineageDynamic(
                _assocLabeledClass, _instanceNames[Mother],
                _instanceNames[Son1], String("four")));
        _instancesLabeledLineageDynamic.append(
            _buildInstanceLabeledLineageDynamic(
                _assocLabeledClass, _instanceNames[Mother],
                _instanceNames[Son2], String("five")));
        _instancesLabeledLineageDynamic.append(
            _buildInstanceLabeledLineageDynamic(
                _assocLabeledClass, _instanceNames[Mother],
                _instanceNames[Daughter1], String("six")));

        for (Uint32 i=0, n = _instancesLabeledLineageDynamic.size(); i < n; i++)
        {
            // Create Instance Names
            _instanceNamesLabeledLineageDynamic.append(
                _instancesLabeledLineageDynamic[i].buildPath(
                    assocLabeledClassName));
        }
        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
            "Initialize %s. Count= %i instances",
            "TST_LabeledLineageDynamic",
            _instancesLabeledLineageDynamic.size()));
    }

    PEG_METHOD_EXIT();
}

void FamilyProvider::terminate()
{
    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4, "Terminate");
    delete this;
}

/* local get instance, gets from instanceArray for
   the array provided.  Gets the instance defined by
   localReference from the array defined by
   instanceArray and _filters it(clone + information
   filter) before delivering it.
   @param instanceArray Array<CIMInstance> to search
   for this instance.
   @localReference CIMObjectPath with the localized
   reference information (class and keybindings.
   @exception Returns CIM_ERR_NOT_FOUND if the instance
   cannot be found.
*/
void FamilyProvider::_getInstance(
    const Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = _makeRefLocal(instanceReference);

    for (Uint32 i = 0, n = instanceArray.size(); i < n; i++)
    {
        if (localReference == instanceArray[i].getPath())
        {
            // deliver filtered clone of requested instance
            handler.deliver(_clone(instanceArray[i],
                localReference));
            return;
        }
    }
    throw CIMException(CIM_ERR_NOT_FOUND);
}

/* getInstance call from another service. See InstanceProvider.h
    for detailed definition of paramaters.
    Based on the input instance reference, it gets the instance
    from the appropriate list of created instances.
    NOTE FOR FUTURE: We could just as easily have worked from a
    single list of instances and searched.  In that case, the
    enumerate would be the same search as the get execpt that one
    would be on class and the other on instance.  Would have reduced
    the level of testing for types in these two functions.
*/
void FamilyProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "getInstance. Class= %s",
        (const char *)instanceReference.toString().getCString()));

    // begin processing the request
    handler.processing();
    targetClass myClassEnum =
        _verifyValidClassInput(instanceReference.getClassName());
    switch (myClassEnum)
    {
        case TST_PERSONDYNAMIC:
            _getInstance(_instances, context, instanceReference,
                includeQualifiers, includeClassOrigin, propertyList, handler);
            break;
        case TST_PERSONDYNAMICSUBCLASS:
            _getInstance(_instancesSubclass, context, instanceReference,
                includeQualifiers, includeClassOrigin, propertyList, handler);
            break;
        case TST_LINEAGEDYNAMIC:
            _getInstance(_instancesLineageDynamic, context, instanceReference,
                includeQualifiers, includeClassOrigin, propertyList, handler);
            break;
        case TST_LABELEDLINEAGEDYNAMIC:
            _getInstance(
                _instancesLabeledLineageDynamic, context, instanceReference,
                includeQualifiers, includeClassOrigin, propertyList, handler);
            break;
        default:
            // ATTN: Should really be general error since the verify should
            // catch this.
            throw CIMException(CIM_ERR_NOT_FOUND);
    }

    // complete processing the request
    handler.complete();
}

/** internal enumerateInstances that delivers instances for
    the defined instanceArray.  It takes all of the input
    properties plus the instance array reference.
*/
void FamilyProvider::_enumerateInstances(
    const Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    for (Uint32 i = 0, n = instanceArray.size(); i < n; i++)
    {
        handler.deliver(_clone(instanceArray[i],
            classReference));
    }
}

/* enumerateInstances call from another service. See InstanceProvider.h
    for detailed definition of paramaters.
*/
void FamilyProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "enumerateInstances. Class= %s, includeQualifiers= %s, \
        includeClassOrigin= %s, PropertyList= %s",
        (const char *) classReference.toString().getCString(),
        (const char *) _showBool(includeQualifiers).getCString(),
        (const char *) _showBool(includeClassOrigin).getCString(),
        (const char *) _showPropertyList(propertyList).getCString()));

    CIMNamespaceName nameSpace = classReference.getNameSpace();

    // begin processing the request
    handler.processing();
    targetClass myClassEnum =
        _verifyValidClassInput(classReference.getClassName());
    switch (myClassEnum)
    {
        case TST_PERSONDYNAMIC:
            _enumerateInstances( _instances, context, classReference,
                includeQualifiers, includeClassOrigin, propertyList, handler);
            break;
        case TST_PERSONDYNAMICSUBCLASS:
            _enumerateInstances( _instancesSubclass, context, classReference,
                includeQualifiers, includeClassOrigin, propertyList, handler);
            break;
        case TST_LINEAGEDYNAMIC:
            _enumerateInstances(
                _instancesLineageDynamic, context, classReference,
                includeQualifiers, includeClassOrigin, propertyList, handler);
            break;
        case TST_LABELEDLINEAGEDYNAMIC:
            _enumerateInstances(
                _instancesLabeledLineageDynamic, context, classReference,
                includeQualifiers, includeClassOrigin, propertyList, handler);
            break;
        default:
            // ATTN: Should really be general error since the verify should
            // catch this.
            throw CIMException(CIM_ERR_NOT_FOUND);
    }

    // complete processing the request
    handler.complete();
}

void FamilyProvider::_enumerateInstanceNames(
    const Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    for (Uint32 i = 0, n = instanceArray.size(); i < n; i++)
    {
        // Set the host and namespace into these.
        handler.deliver(setCompleteObjectPath(instanceArray[i].getPath(),
                    classReference));
    }
}

void FamilyProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "enumerateInstanceNames. Class= %s",
        (const char *) classReference.toString().getCString()));

    // begin processing the request

    handler.processing();
    targetClass MyClassEnum =
        _verifyValidClassInput(classReference.getClassName());

    switch (MyClassEnum)
    {
        case TST_PERSONDYNAMIC:
            _enumerateInstanceNames(
                _instances, context, classReference, handler);
            break;
        case TST_PERSONDYNAMICSUBCLASS:
            _enumerateInstanceNames(
                _instancesSubclass, context, classReference, handler);
            break;
        case TST_LINEAGEDYNAMIC:
            _enumerateInstanceNames(
                _instancesLineageDynamic, context, classReference, handler);
            break;
        case TST_LABELEDLINEAGEDYNAMIC:
            _enumerateInstanceNames(
                _instancesLabeledLineageDynamic, context, classReference,
                handler);
            break;
        default:
            // ATTN: Should really be general error since the verify should
            // catch this.
            throw CIMException(CIM_ERR_NOT_FOUND);
    }
    // complete processing the request
    handler.complete();
}

void FamilyProvider::_modifyInstance(
    Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & localReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{

    for (Uint32 i = 0, n = instanceArray.size(); i < n; i++)
    {
        // ATTN: We will need to localize the input here.
        if (localReference == instanceArray[i].getPath())
        {
            // overwrite existing instance
            // ATTN:too simplistic.  Does not take into accout
            // include qualifers, property list.
            // But we are not really using this right now as
            // part of tests so leave for the future.
            instanceArray[i] = instanceObject;
            break;
        }
    }
    throw CIMException(CIM_ERR_NOT_FOUND);
}

void FamilyProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    CDEBUG("modifyInstance  hostname: " << instanceReference.getHost() );

    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "modifyInstance. Class= %s",
        (const char *) instanceReference.toString().getCString()));

    CIMObjectPath localReference =_makeRefLocal(instanceReference);

    // begin processing the request
    handler.processing();
    targetClass myClassEnum =
        _verifyValidClassInput(instanceReference.getClassName());
    if (myClassEnum == TST_PERSONDYNAMIC)
    {
        _modifyInstance(_instances, context, localReference,
            instanceObject, includeQualifiers, propertyList,
            handler);
    }
    if (myClassEnum == TST_PERSONDYNAMICSUBCLASS)
    {
        _modifyInstance(_instancesSubclass, context, localReference,
            instanceObject, includeQualifiers, propertyList,
            handler);
    }
    if (myClassEnum == TST_LINEAGEDYNAMIC)
    {
        _modifyInstance(_instancesLineageDynamic, context, localReference,
            instanceObject, includeQualifiers, propertyList,
            handler);
    }
    if (myClassEnum ==  TST_LABELEDLINEAGEDYNAMIC)
    {
        _modifyInstance(
            _instancesLabeledLineageDynamic, context, localReference,
            instanceObject, includeQualifiers, propertyList,
            handler);
    }
    // complete processing the request
    handler.complete();
}

void FamilyProvider::_createInstance(
    Array<CIMInstance> & instanceArray,
    Array<CIMObjectPath> & pathArray,
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{

    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = _makeRefLocal(instanceReference);
    CIMInstance saveInstance = instanceObject.clone();
    for (Uint32 i = 0, n = instanceArray.size(); i < n; i++)
    {
        if (localReference == instanceArray[i].buildPath(_personDynamicClass))
            throw CIMObjectAlreadyExistsException(localReference.toString());
    }
    // add the new instance to the array
    // Note that we make it local.  This is a decicision of this provider
    // to allow such instances to be part of any namespace when
    // created

    CIMObjectPath p(saveInstance.getPath());
    p.setHost(String());
    p.setNameSpace(CIMNamespaceName());
    saveInstance.setPath(p);
    instanceArray.append(saveInstance);
    pathArray.append(localReference);

    // deliver the new instance
    handler.deliver(instanceReference);
}

void FamilyProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "createInstance. Class= %s",
        (const char *) instanceReference.toString().getCString()));

    // ATTN: Add test here to be sure that the instancereference and the
    // keys in the object are the same.

    handler.processing();
    targetClass myClassEnum =
        _verifyValidClassInput(instanceReference.getClassName());
    switch (myClassEnum)
    {
        case TST_PERSONDYNAMIC:
            _createInstance(_instances, _instanceNames, context,
                instanceReference, instanceObject, handler);
            break;
        case TST_PERSONDYNAMICSUBCLASS:
            _createInstance(_instancesSubclass, _instanceNames, context,
                instanceReference, instanceObject, handler);
            break;
        case TST_LINEAGEDYNAMIC:
            _createInstance(
                _instancesLabeledLineageDynamic,
                _instanceNamesLabeledLineageDynamic, context,
                instanceReference, instanceObject, handler);
            break;
        case TST_LABELEDLINEAGEDYNAMIC:
            _createInstance(
                _instancesLabeledLineageDynamic,
                _instanceNamesLabeledLineageDynamic, context,
                instanceReference, instanceObject, handler);
            break;
        default:
            // ATTN: Should really be general error since the verify should
            // catch this.
            throw CIMException(CIM_ERR_NOT_FOUND);
    }
    // complete processing the request
    handler.complete();
}

void FamilyProvider::_deleteInstance(
    Array<CIMInstance> & instanceArray,
    Array<CIMObjectPath> & pathArray,
    const OperationContext & context,
    const CIMObjectPath & localReference,
    ResponseHandler & handler)
{
    for (Uint32 i = 0, n = instanceArray.size(); i < n; i++)
    {
        if (localReference == instanceArray[i].getPath())
        {
            // remove instance from the array
            instanceArray.remove(i);
            pathArray.remove(i);
            return;
        }
    }
    throw CIMException(CIM_ERR_NOT_FOUND);
    return;             // should never get here
}

void FamilyProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler)
{
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "deleteInstance. Class= %s",
        (const char *) instanceReference.toString().getCString()));

    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = _makeRefLocal(instanceReference);

    // begin processing the request

    CIMName myClassName = instanceReference.getClassName();

    handler.processing();
    targetClass myClassEnum = _verifyValidClassInput(
        instanceReference.getClassName());
    switch (myClassEnum)
    {
        case TST_PERSONDYNAMIC:
            _deleteInstance(_instances, _instanceSubclassNames, context,
                localReference, handler);
            break;
        case TST_PERSONDYNAMICSUBCLASS:
            _deleteInstance(_instancesSubclass, _instanceSubclassNames, context,
                localReference, handler);
            break;
        case TST_LINEAGEDYNAMIC:
            _deleteInstance(_instancesLineageDynamic,
                _instanceNamesLineageDynamic, context,
                localReference, handler);
            break;
        case TST_LABELEDLINEAGEDYNAMIC:
            _deleteInstance(_instancesLabeledLineageDynamic,
                _instanceNamesLabeledLineageDynamic, context,
                localReference, handler);
            break;
        default:
            PEGASUS_TEST_ASSERT(false);
    }
    handler.complete();
}

void FamilyProvider::_associators(
    Array<CIMInstance> & instanceArray,
    Array<CIMInstance> & resultInstanceArray,
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    ObjectResponseHandler & handler)
{
    CIMObjectPath localObjectName = _makeRefLocal(objectName);
    // Filter out the required objectpaths from the association list.
    Array<CIMInstance> assocInstances;
    assocInstances = _filterReferenceNames(instanceArray,
                                               localObjectName,
                                               associationClass,
                                               role);

    for (Uint32 i = 0 ; i < assocInstances.size() ; i++)
    {
        Array<CIMObjectPath> resultPaths;
        resultPaths = _filterAssocInstanceToTargetPaths(assocInstances[i],
                                    localObjectName,
                                    resultClass,
                                    resultRole);

        // Loop to process all resultPaths
        for (Uint32 j = 0 ; j < resultPaths.size() ; j++)
        {
            // instance index corresponds to reference index
            // For each resultInstance, if matches result path, Deliver.
            for (Uint32 k = 0, n = resultInstanceArray.size(); k < n; k++)
            {
                CIMObjectPath newPath = resultInstanceArray[k].getPath();

                if (resultPaths[j].identical(newPath))
                {

                    handler.deliver(_clone(resultInstanceArray[k],
                        objectName));
                }
            }
        }
    }
}

void FamilyProvider::associators(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    ObjectResponseHandler & handler)
{
    CDEBUG("associators  objectPath: " << objectName.toString());
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "associators. object= %s, assocClass= %s, resultClass= %s, "
            "role= %s, resultRole= %s, IncludeQualifiers= %s, "
            "ClassOrig= %s, propertyList= %s",
        (const char *) objectName.toString().getCString(),
        (const char *) associationClass.getString().getCString(),
        (const char *) resultClass.getString().getCString(),
        (const char *) role.getCString(),
        (const char *) resultRole.getCString(),
        (const char *) _showBool(includeQualifiers).getCString(),
        (const char *) _showBool(includeClassOrigin).getCString(),
        (const char *) _showPropertyList(propertyList).getCString()));

    // begin processing the request
    // Get the namespace and host names to create the CIMObjectPath

    // ATTN: We may have an issue where with namespace to assure that this
    // works in any namespace.
    // Need to get the input namespace and confirm it is legit.

    handler.processing();
    targetClass myClassEnum  = _verifyValidAssocClassInput(associationClass);
    switch (myClassEnum)
    {
        case TST_LINEAGEDYNAMIC:
            _associators (_instancesLineageDynamic, _instances, context,
                objectName, associationClass, resultClass, role, resultRole,
                includeQualifiers, includeClassOrigin, propertyList,
                handler);
            break;

        case TST_LABELEDLINEAGEDYNAMIC:
            _associators (_instancesLabeledLineageDynamic, _instances, context,
                objectName, associationClass, resultClass, role, resultRole,
                includeQualifiers, includeClassOrigin, propertyList,
                handler);
            break;

        default:
            // ATTN: Should really be general error since the verify should
            // catch this.
            throw CIMException(CIM_ERR_NOT_FOUND);
    }
    // complete processing the request
    handler.complete();
}

void FamilyProvider::_associatorNames(
    Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & localObjectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    ObjectPathResponseHandler & handler)
{

    CDEBUG("_AssociationNames. objectName= "
           << localObjectName.toString()
           << " assocClass= "
           << (const char*)associationClass.getString().getCString()
           << " resultClass= "
           << (const char*)resultClass.getString().getCString()
           << " role= " << role);
    // Filter out the required objectpaths from the association list.
    Array<CIMInstance> assocInstances;

    assocInstances = _filterReferenceNames(instanceArray,
                                               localObjectName,
                                               associationClass,
                                               role);

    for (Uint32 i = 0 ; i < assocInstances.size() ; i++)
    {
        Array<CIMObjectPath> resultPaths;
        resultPaths = _filterAssocInstanceToTargetPaths(
            assocInstances[i], localObjectName, resultClass, resultRole);

        for (Uint32 j = 0 ; j < resultPaths.size() ; j++)
        {
            handler.deliver(
                setCompleteObjectPath(resultPaths[j],localObjectName));
        }
    }
}

void FamilyProvider::associatorNames(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    ObjectPathResponseHandler & handler)
{
    CDEBUG("asociatorNames hostname: " << objectName.getHost());

    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "associatorNames. object= %s, assocClass= %s, "
            "resultClass= %s, role= %s, resultRole=%s",
        (const char*)objectName.toString().getCString(),
        (const char*)associationClass.getString().getCString(),
        (const char*)resultClass.getString().getCString(),
        (const char*)role.getCString(),
        (const char*)resultRole.getCString()));

    // Get the namespace and host names to create the CIMObjectPath

    //CIMNamespaceName nameSpace = objectName.getNameSpace();

    CIMObjectPath localObjectName = _makeRefLocal(objectName);

    handler.processing();
    Array<CIMInstance> assocInstances;

    targetClass myClassEnum  = _verifyValidAssocClassInput(associationClass);
    switch (myClassEnum)
    {
        case TST_LINEAGEDYNAMIC:
            _associatorNames(_instancesLineageDynamic, context, localObjectName,
                associationClass, resultClass, role, resultRole, handler);
            break;

        case TST_LABELEDLINEAGEDYNAMIC:
            _associatorNames(
                _instancesLabeledLineageDynamic, context, localObjectName,
                associationClass, resultClass, role, resultRole, handler);
            break;

        default:
            PEGASUS_TEST_ASSERT(false);
            throw CIMException(CIM_ERR_NOT_FOUND);
    }
    // complete processing the request
    handler.complete();
}

void FamilyProvider::_references(
    Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    ObjectResponseHandler & handler)
{
    CIMObjectPath localObjectName = _makeRefLocal(objectName);

    // Filter out the required objectpaths from the association list.
    Array<CIMInstance> returnInstances = _filterReferenceNames(instanceArray,
                                               localObjectName,
                                               resultClass,
                                               role);

    for (Uint32 i = 0 ; i < returnInstances.size() ; i++)
    {

        handler.deliver(_clone(returnInstances[i],
            objectName));
    }
}

void FamilyProvider::references(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    ObjectResponseHandler & handler)
{
    CDEBUG("references path: " << objectName.toString());

    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "references. object= %s, resultClass= %s, role= %s, "
            "IncludeQualifiers= %s ClassOrig= %s, propertyList= %s",
        (const char*)objectName.toString().getCString(),
        (const char *)resultClass.getString().getCString(),
        (const char *)role.getCString(),
        (const char*)_showBool(includeQualifiers).getCString(),
        (const char *) _showBool(includeClassOrigin).getCString(),
        (const char*)_showPropertyList( propertyList).getCString()));

    //CIMNamespaceName nameSpace = objectName.getNameSpace();


    handler.processing();
    targetClass myClassEnum  = _verifyValidAssocClassInput(resultClass);
    switch (myClassEnum)
    {
        case TST_LINEAGEDYNAMIC:
            _references (_instancesLineageDynamic, context,
                objectName, resultClass, role,
                includeQualifiers, includeClassOrigin, propertyList,
                handler);
            break;

        case TST_LABELEDLINEAGEDYNAMIC:
            _references (_instancesLabeledLineageDynamic, context,
                objectName, resultClass, role,
                includeQualifiers, includeClassOrigin, propertyList,
                handler);
            break;

        default:
            // ATTN: Should really be general error since the verify should
            // catch this.
            throw CIMException(CIM_ERR_NOT_FOUND);
    }

    // complete processing the request
    handler.complete();
}

void FamilyProvider::_referenceNames(
    Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    ObjectPathResponseHandler & handler)
{

    Array<CIMInstance> returnInstances;

    CIMObjectPath localObjectName = CIMObjectPath(
        String(),
        CIMNamespaceName(),
        objectName.getClassName(),
        objectName.getKeyBindings());

    // Filter out the required objectpaths from the association list.
    returnInstances = _filterReferenceNames(instanceArray,
                                               localObjectName,
                                               resultClass,
                                               role);

    for (Uint32 i = 0 ; i < returnInstances.size() ; i++)
    {
        handler.deliver(setCompleteObjectPath(
            returnInstances[i].getPath(),localObjectName));
    }
}

// Return all references(association instance names) in which the give
// object is involved.
void FamilyProvider::referenceNames(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    ObjectPathResponseHandler & handler)
{
    CDEBUG("referenceNames  hostname: " << objectName.getHost());

    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "referenceNames. object= %s, resultClass= %s, role= %s",
        (const char*)objectName.toString().getCString(),
        (const char *)resultClass.getString().getCString(),
        (const char*)role.getCString()));

    CDEBUG("ReferenceNames Operation. objectName= " << objectName.toString()
           << " resultClass= "
           << (const char*)resultClass.getString().getCString()
           << " role= " << role);

    targetClass myClassEnum  = _verifyValidAssocClassInput(resultClass);

    handler.processing();
    switch (myClassEnum)
    {
        case TST_LINEAGEDYNAMIC:
            _referenceNames (_instancesLineageDynamic, context,
                objectName, resultClass, role, handler);
            break;

        case TST_LABELEDLINEAGEDYNAMIC:
            _referenceNames (_instancesLabeledLineageDynamic, context,
                objectName, resultClass, role, handler);
            break;

        default:
            // ATTN: Should really be general error since the verify should
            // catch this.
            throw CIMException(CIM_ERR_NOT_FOUND);
    }
    // complete processing the request
    handler.complete();
}
