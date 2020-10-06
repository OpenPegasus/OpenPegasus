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
    This provider implements the TEST_AggregationOutputProvider mof in the /MOF
    directory. It implements the instance and associator functions for the
    classes
    TEST_PersonDynamic
    TEST_TeachesDynamic

    This provider would also return CIM_NOT_SUPPORTED for the following classes
    TEST_WorksDynamic
    TEST_MarriageDynamic
    The reason for returning CIM_ERR_NOT_SUPPORTED exception would be to test
    the aggregation of Provider results with one or more CIM_ERR_NOT_SUPPORTED
    Exception.  This provider would return CIM_ERR_NOT_FOUND Exception for the
    TEST_FamilyDynamic class. The reason for returning CIM_ERR_NOT_FOUND
    exception would be to test the aggregation of Provider results with one or
    more CIM_ERR_NOT_FOUND exception.
*/

#include <Pegasus/Common/Tracer.h>
#include "TestAggregationOutputProvider.h"

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

#define CDEBUG(X)

const CIMName personClassName = "TEST_Person";
const CIMName personDynamicClassName = "TEST_PersonDynamic";
const CIMName teachesAssocClassName = "TEST_TeachesDynamic";
const CIMName worksAssocClassName = "TEST_WorksDynamic";
const CIMName marriageAssocClassName = "TEST_MarriageDynamic";
const CIMName familyAssocClassName = "TEST_FamilyDynamic";

String nameSpace = "test/TestProvider";

String TestAggregationOutputProviderName = "TestAggregationOutputProvider";

// Defines to serve as the ENUM for class selection for operations.

enum targetClass{
     TEST_PERSON = 1,
     TEST_PERSONDYNAMIC = 2,
     TEST_TEACHESDYNAMIC = 3,
     TEST_WORKSDYNAMIC = 4,
     TEST_MARRIAGEDYNAMIC = 5,
     TEST_FAMILYDYNAMIC = 6
     };

//**************************************************
//          Support Functions
//**************************************************

static targetClass _verifyValidClassInput(const CIMName& className)
{
    if (className.equal(personClassName))
        return TEST_PERSON;
    if(className.equal(personDynamicClassName))
        return TEST_PERSONDYNAMIC;
    if (className.equal(teachesAssocClassName))
        return TEST_TEACHESDYNAMIC;
    if (className.equal(worksAssocClassName))
        return TEST_WORKSDYNAMIC;
    if (className.equal(marriageAssocClassName))
        return TEST_MARRIAGEDYNAMIC;
    if (className.equal(familyAssocClassName))
        return TEST_FAMILYDYNAMIC;

    throw CIMNotSupportedException(className.getString() + " Not supported by "
                                   + TestAggregationOutputProviderName);
}

static targetClass _verifyValidAssocClassInput(const CIMName& className)
{
    if (className.equal(teachesAssocClassName))
        return TEST_TEACHESDYNAMIC;
    if (className.equal(worksAssocClassName))
        return TEST_WORKSDYNAMIC;
    if (className.equal(marriageAssocClassName))
        return TEST_MARRIAGEDYNAMIC;
    if (className.equal(familyAssocClassName))
        return TEST_FAMILYDYNAMIC;

    throw CIMNotSupportedException(
        className.getString() + " not supported by Test Association Provider");
}

/** returns a local (no namespace or host component) version of the input
    CIMObjectPath.
    @param p CIMObjectPath input
    @returns CIMObjectPath object with only className and keybinding components
*/
CIMObjectPath _makeRefLocal(const CIMObjectPath& path)
{
  CIMObjectPath rtn(path);
  rtn.setHost(String());
  rtn.setNameSpace(CIMNamespaceName());
  return(rtn);
}

void _setCompleteObjectPath(CIMInstance & instance)
{
    String host = System::getHostName();
    CIMObjectPath p = instance.getPath();
    p.setHost(host);
    p.setNameSpace(nameSpace);
    instance.setPath(p);
}

/** clone the input instance  
    @return cloned instance.
*/
CIMInstance _clone(const CIMInstance& instance)
{
    CIMInstance rtnInstance = instance.clone();

    _setCompleteObjectPath(rtnInstance);
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
    // get all reference properties except for target.
    for (Uint32 i = 0 ; i < assocInstance.getPropertyCount() ; ++i)
    {
        CIMConstProperty p = assocInstance.getProperty(i);

        if (CIMTYPE_REFERENCE == p.getType())
        {
            CIMValue v = p.getValue();
            CIMObjectPath path;
            v.get(path);

            if (!targetObjectPath.identical(path))
            {
                if (resultClass.isNull() || resultClass == path.getClassName())
                {
                    if (String::EMPTY == resultRole ||
                        p.getName().getString() == resultRole)
                    {
                        returnPaths.append(path);
                    }
                }
            }
        }
    }
    return( returnPaths );
}

/** Test for valid CIMReferences from an association instance. If there is a
    role property, gets all but the role property.
    @param target - The target path for the association. Localization assumed.
    @param instance - The association class instance we are searching for
    references
    @param role - The role we require. If there is no role, this is
    String::EMPTY
    @return - returns Boolean true if target is found in a reference that is
    the same role
 */
Boolean _isInstanceValidReference(
    const CIMObjectPath& target,
    CIMInstance& instance,
    const String& role)
{
    Uint32 pos;
    // Test if role parameter is valid property.
    if (role != String::EMPTY)
    {
        // Test if property with this role exists.
        if (PEG_NOT_FOUND == (pos = instance.findProperty(role)))
            throw CIMException(CIM_ERR_INVALID_PARAMETER);

         // Check to be sure this is a reference property
         if (instance.getProperty(pos).getType() != CIMTYPE_REFERENCE)
             throw CIMException(CIM_ERR_INVALID_PARAMETER);
    }

    //Now search instance for all reference properties
    for (Uint32 j = 0; j < instance.getPropertyCount() ; ++j)
    {
        const CIMProperty p = instance.getProperty(j);
        if (CIMTYPE_REFERENCE == p.getType())
        {
            // If there is no role or the role is the same as this property name
            CIMValue v = p.getValue();
            CIMObjectPath path;
            v.get(path);

            // if no role or role == this role and target = this path, rtn true.
            if ((String::EMPTY == role) || (role == p.getName().getString()))
            {
                Boolean compare =  target.identical(path);
                if (compare)
                {
                    return(true);
                }
            }
        }
    }
    return (false);
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
    CDEBUG("_filterReferenceNames. objName= " << objectName.toString() <<
        " resultClass= " << resultClass << " role= " << role);
    CIMObjectPath targetReference = CIMObjectPath(
        String(),
        CIMNamespaceName(),
        objectName.getClassName(),
        objectName.getKeyBindings());

    Array<CIMInstance> foundList;

    for (Uint32 i = 0 ; i < targetAssociationInstanceList.size() ; ++i)
    {
        CIMInstance instance = targetAssociationInstanceList[i];
        if (resultClass.isNull() || resultClass.equal(instance.getClassName()))
        {
            // if this association instance has this role in targetReference,
            // then true
            if (_isInstanceValidReference(targetReference, instance, role))
            {
                foundList.append(instance);
            }
        }
    }
    CDEBUG("_filterReferenceNames return. Count= " << foundList.size());
    return( foundList );
}

/* build an instance of the class from the input properties and set the path.
   Note that path is set without host and namespace and these are added at the
   last minute after prep for response.
    @param thisClass CIMClass that this instance is created from
    @param name String Data for name property
    ...
    @return CIMInstance built from the input.
*/
CIMInstance _buildPersonInstance(const CIMClass& thisClass, const String& name)
{
    CIMInstance instance(personDynamicClassName);
    instance.addProperty(CIMProperty("Name", name));
    CIMObjectPath p = instance.buildPath(thisClass);
    instance.setPath(p);
    return(instance);
}

CIMInstance _buildInstanceTeaches(const CIMClass& thisClass,
    const CIMObjectPath& teacher, const CIMObjectPath& student)
{
    CIMInstance instance(teachesAssocClassName);
    instance.addProperty(
        CIMProperty("teacher", teacher,0,personDynamicClassName));
    instance.addProperty(
        CIMProperty("student", student, 0, personDynamicClassName));
    CIMObjectPath p = instance.buildPath(thisClass);
    instance.setPath(p);
    return(instance);
}

/*********** TestAggregationOutputProvider class Implementation *********/
TestAggregationOutputProvider::TestAggregationOutputProvider(void)
{
}

TestAggregationOutputProvider::~TestAggregationOutputProvider(void)
{
}

/* get the defined class from the repository.
    @param className CIMName name of the class to get
    @return CIMClass with the class or uninitialized if
    there was an error in the getClass
*/
CIMClass TestAggregationOutputProvider::_getClass(const CIMName& className)
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
            "TestAggregationOutputProvider GetClass operation failed: "
                "Class %s. Msg %s",
            (const char*) className.getString().getCString(),
            (const char*) e.getMessage().getCString()));
    }
    return(c);
}


void TestAggregationOutputProvider::initialize(CIMOMHandle & cimom)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "TestAggregationOutputProvider::initialize");

    _initError = false;
    _cimomHandle = cimom;
    {
        CIMClass personclass = _getClass(personDynamicClassName);
        if (personclass.isUninitialized())
            _initError = true;
        _personClass = personclass;

        // Create the association class
        CIMClass teachesclass = _getClass(teachesAssocClassName);
        if (teachesclass.isUninitialized())
            _initError = true;
        _teachesClass = teachesclass;

        CIMClass worksclass = _getClass(worksAssocClassName);
        if (worksclass.isUninitialized())
            _initError = true;
        _worksClass = worksclass;

        CIMClass marriageclass = _getClass(marriageAssocClassName);
        if (marriageclass.isUninitialized())
            _initError = true;
        _marriageClass = marriageclass;

        CIMClass familyclass = _getClass(familyAssocClassName);
        if (familyclass.isUninitialized())
            _initError = true;
        _familyClass = familyclass;

    }
    // Do not try to initialize instances if class initialization failed.
    if (_initError)
        return;
    try
    {
        // build the instances of TEST_Person
        Uint32 Teacher = _instances.size();
        _instances.append(
            _buildPersonInstance(_personClass, String("Teacher")));
        Uint32 Student = _instances.size();
        _instances.append(
            _buildPersonInstance(_personClass, String("Student")));
        _instances.append(
            _buildPersonInstance(_personClass, String("Employee")));
        _instances.append(
            _buildPersonInstance(_personClass, String("Manager")));
        _instances.append(
            _buildPersonInstance(_personClass, String("Husband")));
        _instances.append(_buildPersonInstance(_personClass, String("Wife")));
        _instances.append(_buildPersonInstance(_personClass, String("Father")));
        _instances.append(_buildPersonInstance(_personClass, String("Child")));

        for(Uint32 i = 0, n = _instances.size(); i < n; ++i)
        {
            _instanceNames.append(_instances[i].buildPath(_personClass));
        }

        // Make the instances for the associations

        CIMInstance ciminstance;
        ciminstance = _buildInstanceTeaches(
            _teachesClass, _instanceNames[Teacher],_instanceNames[Student]);
        _instancesTeaches.append(_buildInstanceTeaches(
            _teachesClass, _instanceNames[Teacher],_instanceNames[Student]));
        for(Uint32 i = 0, n = _instancesTeaches.size(); i < n; ++i)
        {
            _instanceNamesTeaches.append(
                _instancesTeaches[i].buildPath(teachesAssocClassName));
        }
    }
    catch(Exception &e)
    {
        cerr << "Exception occured :  " << e.getMessage() << endl;
    }

    PEG_METHOD_EXIT();
}

void TestAggregationOutputProvider::terminate(void)
{
    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4, "Terminate");
    delete this;
}


/*  local get instance, gets from instanceArray for the array provided.  Gets
    the instance defined by localReference from the array defined by
    instanceArray and _filters it(clone + information filter) before
    delivering it.
    @param instanceArray Array<CIMInstance> to search for this instance.
    @localReference CIMObjectPath with the localized reference information
    (class and keybindings)
    @exception Returns CIM_ERR_NOT_FOUND if the instance cannot be found.
*/
void TestAggregationOutputProvider::_getInstance(
    const Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & localReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    CIMObjectPath localReference1 = _makeRefLocal(localReference);

    for(Uint32 i = 0, n = instanceArray.size(); i < n; ++i)
    {
        if(localReference1 == instanceArray[i].getPath())
        {
            // deliver filtered clone of requested instance
            handler.deliver(_clone(instanceArray[i]));
            return;
        }
    }
    throw CIMException(CIM_ERR_NOT_FOUND);
}

/*  getInstance call from another service.
    Based on the input instance reference, it gets the instance
    from the appropriate list of created instances.
    This would return the instances for the Classes TEST_PersonDynamic
    and TEST_TeachesDynamic.  This would return CIM_ERR_NOT_SUPPORTED for
    TEST_WorksDynamic and TEST_MarriageDynamic. This would return
    CIM_ERR_NOT_FOUND for TEST_FamilyDynamic
*/
void TestAggregationOutputProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    // begin processing the request
    handler.processing();
    targetClass myClassEnum =
        _verifyValidClassInput(instanceReference.getClassName());
    switch (myClassEnum)
    {
        case TEST_PERSONDYNAMIC:
            _getInstance(_instances, context, instanceReference,
                includeQualifiers, includeClassOrigin, propertyList, handler);
            break;
        case TEST_TEACHESDYNAMIC:
            _getInstance(_instancesTeaches, context, instanceReference,
                includeQualifiers, includeClassOrigin, propertyList, handler);
            break;
        case TEST_WORKSDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_MARRIAGEDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_FAMILYDYNAMIC:
            throw CIMException(CIM_ERR_NOT_FOUND);
        default:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }

    handler.complete();
}
/** internal enumerateInstances that delivers instances for
    the defined instanceArray.  It takes all of the input
    properties plus the instance array reference.
*/
void TestAggregationOutputProvider::_enumerateInstances(
    const Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    try
    {
        for(Uint32 i = 0, n = instanceArray.size(); i < n; ++i)
        {
            handler.deliver(_clone(instanceArray[i]));
        }
    }
    catch(Exception & e)
    {
        cerr << "Exception Occured "<<e.getMessage() << endl;
    }
}

/*  enumerateInstances call from another service.
    This would return the instances for the Classes TEST_PersonDynamic
    and TEST_TeachesDynamic.  This would return CIM_ERR_NOT_SUPPORTED for
    TEST_WorksDynamic and TEST_MarriageDynamic. This would return
    CIM_ERR_NOT_FOUND for TEST_FamilyDynamic
*/
void TestAggregationOutputProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    // begin processing the request
    handler.processing();
    targetClass myClassEnum =
        _verifyValidClassInput(classReference.getClassName());
    switch (myClassEnum)
    {
        case TEST_PERSONDYNAMIC:
            _enumerateInstances( _instances, context, classReference,
                includeQualifiers, includeClassOrigin, propertyList, handler);
            break;
        case TEST_TEACHESDYNAMIC:
            _enumerateInstances( _instancesTeaches, context, classReference,
                includeQualifiers, includeClassOrigin, propertyList, handler);
            break;
        case TEST_WORKSDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_MARRIAGEDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_FAMILYDYNAMIC:
            throw CIMException(CIM_ERR_NOT_FOUND);
        default:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }

    // complete processing the request
    handler.complete();
}

/** internal _enumerateInstanceNames that delivers instance names for
    the defined instanceArray.  It takes all of the input
    properties plus the instance array reference.
*/
void TestAggregationOutputProvider::_enumerateInstanceNames(
    const Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    try
    {
        for(Uint32 i = 0, n = instanceArray.size(); i < n; ++i)
        {
            handler.deliver(instanceArray[i].getPath());
        }
    }
    catch(Exception & e)
    {
        cerr << "Exception Occured "<<e.getMessage() << endl;
    }
}

/* enumerateInstanceNames call from another service.
    This would return the instance names for the Classes TEST_PersonDynamic
    and TEST_TeachesDynamic.  This would return CIM_ERR_NOT_SUPPORTED for
    TEST_WorksDynamic and TEST_MarriageDynamic. This would return
    CIM_ERR_NOT_FOUND for TEST_FamilyDynamic
*/
void TestAggregationOutputProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    // begin processing the request
    handler.processing();
    targetClass MyClassEnum =
        _verifyValidClassInput(classReference.getClassName());
    switch (MyClassEnum)
    {
        case TEST_PERSONDYNAMIC:
            _enumerateInstanceNames(
                _instances, context, classReference,handler);
            break;
        case TEST_TEACHESDYNAMIC:
            _enumerateInstanceNames(
                _instancesTeaches, context, classReference,handler);
            break;
        case TEST_WORKSDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_MARRIAGEDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_FAMILYDYNAMIC:
            throw CIMException(CIM_ERR_NOT_FOUND);
        default:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }
    handler.complete();
}

/** internal _associators that delivers associators for
    the defined instanceArray.  It takes all of the input
    properties plus the instance array reference.
*/
void TestAggregationOutputProvider::_associators(
    Array<CIMInstance> & instanceArray,
    Array<CIMInstance> & resultInstanceArray,
    const OperationContext & context,
    const CIMObjectPath & localObjectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    ObjectResponseHandler & handler)
{
    try
    {
        CIMObjectPath localObjectName1 = _makeRefLocal(localObjectName);

        // Filter out the required objectpaths from the association list.
        Array<CIMInstance> assocInstances = _filterReferenceNames(
            instanceArray,
            localObjectName,
            associationClass,
            role);
        for (Uint32 i = 0 ; i < assocInstances.size() ; ++i)
        {
            Array<CIMObjectPath> resultPaths =
                _filterAssocInstanceToTargetPaths(
                    assocInstances[i],
                    localObjectName1,
                    resultClass,
                    resultRole);

            // Loop to process all resultPaths
            for (Uint32 j = 0 ; j < resultPaths.size() ; ++j)
            {
                // instance index corresponds to reference index
                // For each resultInstance, if matches result path, Deliver.
                for(Uint32 k = 0, n = resultInstanceArray.size(); k < n; ++k)
                {
                    CIMObjectPath newPath = resultInstanceArray[k].getPath();

                    if(resultPaths[j].identical(newPath))
                    {
                        handler.deliver(_clone(
                            resultInstanceArray[k]));
                    }
                }
            }
        }
    }
    catch(Exception & e)
    {
        cerr << "Exception Occured "<<e.getMessage() << endl;
    }
}

/*  associators call from another service.
    This would return the associators for the Classes TEST_PersonDynamic
    and TEST_TeachesDynamic.  This would return CIM_ERR_NOT_SUPPORTED for
    TEST_WorksDynamic and TEST_MarriageDynamic. This would return
    CIM_ERR_NOT_FOUND for TEST_FamilyDynamic
*/
void TestAggregationOutputProvider::associators(
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
    // begin processing the request
    handler.processing();
    targetClass myClassEnum  = _verifyValidAssocClassInput(associationClass);
    switch (myClassEnum)
    {
        case TEST_TEACHESDYNAMIC:
            _associators (_instancesTeaches, _instances, context,
                objectName, associationClass, resultClass, role, resultRole,
                includeQualifiers, includeClassOrigin, propertyList,
                handler);
            break;
        case TEST_WORKSDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_MARRIAGEDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_FAMILYDYNAMIC:
            throw CIMException(CIM_ERR_NOT_FOUND);
        default:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }
    // complete processing the request
    handler.complete();
}

/** internal _associatorNames that delivers associator names for
    the defined instanceArray.  It takes all of the input
    properties plus the instance array reference.
*/
void TestAggregationOutputProvider::_associatorNames(
    Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & localObjectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    ObjectPathResponseHandler & handler)
{
    try
    {

        // Filter out the required objectpaths from the association list.
        Array<CIMInstance> assocInstances = _filterReferenceNames(instanceArray,
                                                localObjectName,
                                                associationClass,
                                                role);

        for (Uint32 i = 0 ; i < assocInstances.size() ; ++i)
        {
            Array<CIMObjectPath> resultPaths =
                _filterAssocInstanceToTargetPaths(
                    assocInstances[i],
                    localObjectName,
                    resultClass,
                    resultRole);

            for (Uint32 j = 0 ; j < resultPaths.size() ; ++j)
            {
                CIMObjectPath sendPath = resultPaths[j];
                String host = System::getHostName();
                if (sendPath.getHost().size() == 0)
                    sendPath.setHost(host);

                if (sendPath.getNameSpace().isNull())
                    sendPath.setNameSpace(nameSpace);
                handler.deliver(sendPath);
            }
        }
    }
    catch(Exception & e)
    {
        cerr << "Exception Occured "<<e.getMessage() << endl;
    }
}

/* associatorNames call from another service.
    This would return the associator names for the Classes TEST_PersonDynamic
    and TEST_TeachesDynamic.  This would return CIM_ERR_NOT_SUPPORTED for
    TEST_WorksDynamic and TEST_MarriageDynamic. This would return
    CIM_ERR_NOT_FOUND for TEST_FamilyDynamic
*/
void TestAggregationOutputProvider::associatorNames(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    ObjectPathResponseHandler & handler)
{
    // Get the namespace and host names to create the CIMObjectPath
    CIMObjectPath localobjectName = _makeRefLocal (objectName);
    handler.processing();
    targetClass myClassEnum  = _verifyValidAssocClassInput(associationClass);
    switch (myClassEnum)
    {
        case TEST_TEACHESDYNAMIC:
            _associatorNames(_instancesTeaches, context, localobjectName,
                associationClass, resultClass, role, resultRole, handler);
            break;
        case TEST_WORKSDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_MARRIAGEDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_FAMILYDYNAMIC:
            throw CIMException(CIM_ERR_NOT_FOUND);
        default:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }
    // complete processing the request
    handler.complete();
}

/** internal _references that delivers references for the defined instanceArray.
    It takes all of the input properties plus the instance array reference.
*/
void TestAggregationOutputProvider::_references(
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
    try
    {
        String host = System::getHostName();
        CIMObjectPath localobjectName = _makeRefLocal (objectName);
        // Filter out the required objectpaths from the association list.
        Array<CIMInstance> returnInstances = _filterReferenceNames(
            instanceArray,
            localobjectName,
            resultClass,
            role);

        for (Uint32 i = 0 ; i < returnInstances.size() ; ++i)
        {
            CIMObjectPath objectPath =  returnInstances[i].getPath();
            if (objectPath.getHost().size() == 0)
                objectPath.setHost(host);
            if (objectPath.getNameSpace().isNull())
                objectPath.setNameSpace(nameSpace);

            returnInstances[i].setPath(objectPath);
            CIMInstance inst = _clone(
                returnInstances[i]);
            handler.deliver(inst);
        }
    }
    catch(Exception & e)
    {
        cerr << "Exception Occured "<<e.getMessage() << endl;
    }
}

/*  references call from another service.
    This would return the referencesreferences(association instances) for the
    Classes TEST_PersonDynamic and TEST_TeachesDynamic.
    This would return CIM_ERR_NOT_SUPPORTED for TEST_WorksDynamic and
    TEST_MarriageDynamic.
    This would return CIM_ERR_NOT_FOUND for TEST_FamilyDynamic
*/
void TestAggregationOutputProvider::references(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    ObjectResponseHandler & handler)
{
    targetClass myClassEnum  = _verifyValidAssocClassInput(resultClass);
    handler.processing();
    switch (myClassEnum)
    {
        case TEST_TEACHESDYNAMIC:
            _references(
                _instancesTeaches, context,objectName, resultClass, role,
                 includeQualifiers, includeClassOrigin, propertyList,handler);
            break;
        case TEST_WORKSDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_MARRIAGEDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_FAMILYDYNAMIC:
            throw CIMException(CIM_ERR_NOT_FOUND);
        default:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }
    // complete processing the request
    handler.complete();
}
/** internal _referenceNames that delivers reference names for
    the defined instanceArray.  It takes all of the input
    properties plus the instance array reference.
*/
void TestAggregationOutputProvider::_referenceNames(
    Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    ObjectPathResponseHandler & handler)
{
    try
    {
        String host = System::getHostName();
        Array<CIMInstance> returnInstances =
            _filterReferenceNames(
                instanceArray, objectName,resultClass,role);
        for (Uint32 i = 0 ; i < returnInstances.size() ; ++i)
        {
            CIMObjectPath sendPath =  returnInstances[i].getPath();
            if (sendPath.getHost().size() == 0)
                sendPath.setHost(host);
            if (sendPath.getNameSpace().isNull())
                sendPath.setNameSpace(nameSpace);
            handler.deliver(sendPath);
        }
    }
    catch(Exception & e)
    {
        cerr << "Exception Occured "<<e.getMessage() << endl;
    }
}

/*  referenceNames call from another service.
    This would return the references(association instance names) for the
    Classes TEST_PersonDynamic and TEST_TeachesDynamic.  This would return
    CIM_ERR_NOT_SUPPORTED for TEST_WorksDynamic and TEST_MarriageDynamic.
    This would return CIM_ERR_NOT_FOUND for TEST_FamilyDynamic
*/
void TestAggregationOutputProvider::referenceNames(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    ObjectPathResponseHandler & handler)
{
    targetClass myClassEnum  = _verifyValidAssocClassInput(resultClass);

    handler.processing();
    switch (myClassEnum)
    {
        case TEST_TEACHESDYNAMIC:
            _referenceNames(_instancesTeaches, context,
                objectName, resultClass, role, handler);
            break;
        case TEST_WORKSDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_MARRIAGEDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_FAMILYDYNAMIC:
            throw CIMException(CIM_ERR_NOT_FOUND);
        default:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }
    // complete processing the request
    handler.complete();
}
/** internal _createInstance that creates instances for
    the defined instanceArray.  It takes all of the input
    properties plus the instance array reference.
*/
void TestAggregationOutputProvider::_createInstance(
    Array<CIMInstance> & instanceArray,
    Array<CIMObjectPath> & pathArray,
    const OperationContext & context,
    const CIMObjectPath & localReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    try
    {
        for(Uint32 i = 0, n = instanceArray.size(); i < n; ++i)
        {
            if(localReference == instanceArray[i].buildPath(_personClass))
                throw CIMObjectAlreadyExistsException(
                                    localReference.toString());
        }
        // add the new instance to the array
        instanceArray.append(instanceObject);
        pathArray.append(localReference);

        // deliver the new instance
        handler.deliver(localReference);
    }
    catch(Exception & e)
    {
        cerr << "Exception Occured "<<e.getMessage() << endl;
    }
}

/*  createInstance call from another service.
    This would create the instances for the Classes TEST_PersonDynamic
    and TEST_TeachesDynamic.
    This would return CIM_ERR_NOT_SUPPORTED for TEST_WorksDynamic and
    TEST_MarriageDynamic.
    This would return CIM_ERR_NOT_FOUND for TEST_FamilyDynamic
*/
void TestAggregationOutputProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    handler.processing();
    targetClass myClassEnum =
        _verifyValidClassInput(instanceReference.getClassName());
    switch (myClassEnum)
    {
        case TEST_PERSONDYNAMIC:
            _createInstance(_instances, _instanceNames, context,
                instanceReference, instanceObject, handler);
            break;
        case TEST_TEACHESDYNAMIC:
            _createInstance(_instancesTeaches, _instanceNamesTeaches, context,
                instanceReference, instanceObject, handler);
            break;
        case TEST_WORKSDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_MARRIAGEDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_FAMILYDYNAMIC:
            throw CIMException(CIM_ERR_NOT_FOUND);
        default:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }
    // complete processing the request
    handler.complete();
}
/** internal _modifyInstance that modifies the instances for
    the defined instanceArray.  It takes all of the input
    properties plus the instance array reference.
*/
void TestAggregationOutputProvider::_modifyInstance(
    Array<CIMInstance> & instanceArray,
    const OperationContext & context,
    const CIMObjectPath & localReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    try
    {
        CIMObjectPath localReference1 = _makeRefLocal (localReference);
        for(Uint32 i = 0, n = instanceArray.size(); i < n; ++i)
        {
            if(localReference1 == instanceArray[i].getPath())
            {
                instanceArray[i] = instanceObject;
                break;
            }
        }
    }
    catch(Exception & e)
    {
        cerr << "Exception Occured "<<e.getMessage() << endl;
    }
}
/*  modifyInstance call from another service.
    This would modify the instances for the Classes TEST_PersonDynamic
    and TEST_TeachesDynamic.
    This would return CIM_ERR_NOT_SUPPORTED for TEST_WorksDynamic and
    TEST_MarriageDynamic.
    This would return CIM_ERR_NOT_FOUND for TEST_FamilyDynamic
*/
void TestAggregationOutputProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    // begin processing the request
    handler.processing();
    targetClass myClassEnum =
        _verifyValidClassInput(instanceReference.getClassName());
    if (TEST_PERSONDYNAMIC == myClassEnum )
    {
        _modifyInstance(_instances, context, instanceReference,
            instanceObject, includeQualifiers, propertyList,
            handler);
     }
    if (TEST_TEACHESDYNAMIC == myClassEnum )
    {
        _modifyInstance(_instancesTeaches, context, instanceReference,
            instanceObject, includeQualifiers, propertyList,
            handler);
     }
    if (TEST_WORKSDYNAMIC == myClassEnum)
    {
        throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }
    if (TEST_MARRIAGEDYNAMIC == myClassEnum)
    {
        throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }
    if (TEST_FAMILYDYNAMIC == myClassEnum)
    {
        throw CIMException(CIM_ERR_NOT_FOUND);
    }

    // complete processing the request
    handler.complete();
}
/** internal _deleteInstance that deletes the instances for
    the defined instanceArray.  It takes all of the input
    properties plus the instance array reference.
*/
void TestAggregationOutputProvider::_deleteInstance(
    Array<CIMInstance> & instanceArray,
    Array<CIMObjectPath> & pathArray,
    const OperationContext & context,
    const CIMObjectPath & localReference,
    ResponseHandler & handler)
{
    try
    {
        CIMObjectPath localReference1 = _makeRefLocal (localReference);
        for(Uint32 i = 0, n = instanceArray.size(); i < n; ++i)
        {
            if(localReference1 == instanceArray[i].getPath())
            {
            // remove instance from the array
            instanceArray.remove(i);
            pathArray.remove(i);
            return;
            }
        }
    }
    catch(Exception & e)
    {
        cerr << "Exception Occured "<<e.getMessage() << endl;
    }
    return;
}

/*  deleteInstance call from another service.
    This would delete the instances for the Classes TEST_PersonDynamic
    and TEST_TeachesDynamic.
    This would return CIM_ERR_NOT_SUPPORTED for TEST_WorksDynamic and
    TEST_MarriageDynamic.
    This would return CIM_ERR_NOT_FOUND for TEST_FamilyDynamic
*/
void TestAggregationOutputProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler)
{
    // begin processing the request
    CIMName myClassName = instanceReference.getClassName();

    handler.processing();
    targetClass myClassEnum =
        _verifyValidClassInput(instanceReference.getClassName());
    switch (myClassEnum)
    {
        case TEST_PERSONDYNAMIC:
            _deleteInstance(_instances, _instanceNames, context,
                instanceReference, handler);
            break;
        case TEST_TEACHESDYNAMIC:
            _deleteInstance(_instancesTeaches, _instanceNamesTeaches, context,
                instanceReference, handler);
            break;
        case TEST_WORKSDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_MARRIAGEDYNAMIC:
            throw CIMException(CIM_ERR_NOT_SUPPORTED);
        case TEST_FAMILYDYNAMIC:
            throw CIMException(CIM_ERR_NOT_FOUND);
        default:
            PEGASUS_TEST_ASSERT(false);
    }
    handler.complete();
}
