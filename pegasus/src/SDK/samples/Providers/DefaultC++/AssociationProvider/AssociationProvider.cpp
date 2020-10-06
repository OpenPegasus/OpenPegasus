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
// This is a sample association provider.   It implements the instance and
// association methods for the following classes:
//
//    Sample_Teacher
//    Sample_Student
//    Sample_TeacherStudent   (association class)
//    Sample_AdvisorStudent   (association class)
//
// Class definitions are defined in the AssociationProvider.mof file and the
// provider registration file is AssociationProviderR.mof.
//
// Instance methods supported are:  getInstance, enumerateInstances,
//                                  enumerateInstanceNames
//
// Association methods supported are: associators, associatorNames,
//                                    references, referenceNames
//
// Instances for the supported classes are created during provider
// initialization.
//

#include "AssociationProvider.h"

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

//********************************************************
//  Constants
//********************************************************

// Namespace name
const CIMNamespaceName NAMESPACE = CIMNamespaceName("SDKExamples/DefaultCXX");

// Class names
const CIMName SAMPLE_TEACHER = "Sample_Teacher";
const CIMName SAMPLE_STUDENT = "Sample_Student";
const CIMName SAMPLE_TEACHERSTUDENT = "Sample_TeacherStudent";
const CIMName SAMPLE_ADVISORSTUDENT = "Sample_AdvisorStudent";

AssociationProvider::AssociationProvider()
{
}

AssociationProvider::~AssociationProvider()
{
}

void AssociationProvider::initialize(CIMOMHandle& cimom)
{
    // create default instances
    _createDefaultInstances();
}

void AssociationProvider::terminate()
{
    delete this;
}

void AssociationProvider::getInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler& handler)
{
    // validate namespace
    const CIMNamespaceName& nameSpace = instanceReference.getNameSpace();
    if (!nameSpace.equal(NAMESPACE))
    {
        throw CIMNotSupportedException(
            nameSpace.getString() + " not supported.");
    }

    CIMName className = instanceReference.getClassName();

    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    //
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        CIMNamespaceName(),
        className,
        instanceReference.getKeyBindings());

    // begin processing the request
    handler.processing();

    if (className == SAMPLE_TEACHER)
    {
        _getInstance(_teacherInstances, localReference, handler);
    }
    else if (className == SAMPLE_STUDENT)
    {
        _getInstance(_studentInstances, localReference, handler);
    }
    else if (className == SAMPLE_TEACHERSTUDENT)
    {
        _getInstance(_TSassociationInstances, localReference, handler);
    }
    else if (className == SAMPLE_ADVISORSTUDENT)
    {
        _getInstance(_ASassociationInstances, localReference, handler);
    }
    else
    {
        throw CIMNotSupportedException(
            className.getString() + " is not supported");
    }

    // complete processing the request
    handler.complete();
}

void AssociationProvider::enumerateInstances(
        const OperationContext& context,
        const CIMObjectPath& classReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler& handler)
{
    // validate namespace
    const CIMNamespaceName& nameSpace = classReference.getNameSpace();
    if (!nameSpace.equal(NAMESPACE))
    {
        throw CIMNotSupportedException(
            nameSpace.getString() + " not supported.");
    }

    CIMName className = classReference.getClassName();

    // begin processing the request
    handler.processing();

    if (className == SAMPLE_TEACHER)
    {
        _enumerateInstances(_teacherInstances, handler);
    }
    else if (className == SAMPLE_STUDENT)
    {
        _enumerateInstances(_studentInstances, handler);
    }
    else if (className == SAMPLE_TEACHERSTUDENT)
    {
        _enumerateInstances(_TSassociationInstances, handler);
    }
    else if (className == SAMPLE_ADVISORSTUDENT)
    {
        _enumerateInstances(_ASassociationInstances, handler);
    }
    else
    {
        throw CIMNotSupportedException(
            className.getString() + " is not supported");
    }

    // complete processing the request
    handler.complete();
}

void AssociationProvider::enumerateInstanceNames(
        const OperationContext& context,
        const CIMObjectPath& classReference,
        ObjectPathResponseHandler& handler)
{
    // validate namespace
    const CIMNamespaceName& nameSpace = classReference.getNameSpace();
    if (!nameSpace.equal(NAMESPACE))
    {
        throw CIMNotSupportedException(
            nameSpace.getString() + " not supported.");
    }

    CIMName className = classReference.getClassName();

    // begin processing the request
    handler.processing();

    if (className == SAMPLE_TEACHER)
    {
        _enumerateInstanceNames(_teacherInstances, handler);
    }
    else if (className == SAMPLE_STUDENT)
    {
        _enumerateInstanceNames(_studentInstances, handler);
    }
    else if (className == SAMPLE_TEACHERSTUDENT)
    {
        _enumerateInstanceNames(_TSassociationInstances, handler);
    }
    else if (className == SAMPLE_ADVISORSTUDENT)
    {
        _enumerateInstanceNames(_ASassociationInstances, handler);
    }
    else
    {
        throw CIMNotSupportedException(
            className.getString() + " is not supported");
    }

    // complete processing the request
    handler.complete();
}

void AssociationProvider::modifyInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        const CIMInstance& instanceObject,
        const Boolean includeQualifiers,
        const CIMPropertyList& propertyList,
        ResponseHandler& handler)
{
    throw CIMNotSupportedException("AssociationProvider::modifyInstance");
}

void AssociationProvider::createInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    ObjectPathResponseHandler& handler)
{
    throw CIMNotSupportedException("AssociationProvider::createInstance");
}

void AssociationProvider::deleteInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        ResponseHandler& handler)
{
    throw CIMNotSupportedException("AssociationProvider::deleteInstance");
}

void AssociationProvider::associators(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& associationClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        ObjectResponseHandler& handler)
{
    // validate namespace
    const CIMNamespaceName& nameSpace = objectName.getNameSpace();
    if (!nameSpace.equal(NAMESPACE))
    {
        throw CIMNotSupportedException(
            nameSpace.getString() + " not supported.");
    }

    // Build a host and namespace independent object path
    CIMObjectPath localObjectPath = CIMObjectPath(
        String(),
        CIMNamespaceName(),
        objectName.getClassName(),
        objectName.getKeyBindings());

    // begin processing the request
    handler.processing();

    if (associationClass == SAMPLE_TEACHERSTUDENT)
    {
        _associators(_TSassociationInstances, localObjectPath, role,
            resultClass, resultRole, handler);
    }
    else if (associationClass == SAMPLE_ADVISORSTUDENT)
    {
        _associators(_ASassociationInstances, localObjectPath, role,
            resultClass, resultRole, handler);
    }
    else
    {
        throw CIMNotSupportedException(
            associationClass.getString() + " is not supported");
    }

    // complete processing the request
    handler.complete();
}

void AssociationProvider::associatorNames(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& associationClass,
        const CIMName& resultClass,
        const String& role,
        const String& resultRole,
        ObjectPathResponseHandler& handler)
{
    // validate namespace
    const CIMNamespaceName& nameSpace = objectName.getNameSpace();
    if (!nameSpace.equal(NAMESPACE))
    {
        throw CIMNotSupportedException(
            nameSpace.getString() + " not supported.");
    }

    // Build a host and namespace independent object path
    CIMObjectPath localObjectPath = CIMObjectPath(
        String(),
        CIMNamespaceName(),
        objectName.getClassName(),
        objectName.getKeyBindings());

    // begin processing the request
    handler.processing();

    if (associationClass == SAMPLE_TEACHERSTUDENT)
    {
        _associatorNames(_TSassociationInstances, localObjectPath, role,
            resultClass, resultRole, handler);
    }
    else if (associationClass == SAMPLE_ADVISORSTUDENT)
    {
        _associatorNames(_ASassociationInstances, localObjectPath, role,
            resultClass, resultRole, handler);
    }
    else
    {
        throw CIMNotSupportedException(
            associationClass.getString() + " is not supported");
    }

    // complete processing the request
    handler.complete();
}

void AssociationProvider::references(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        ObjectResponseHandler& handler)
{
    // validate namespace
    const CIMNamespaceName& nameSpace = objectName.getNameSpace();
    if (!nameSpace.equal(NAMESPACE))
    {
        throw CIMNotSupportedException(
            nameSpace.getString() + " not supported.");
    }

    // Build a host and namespace independent object path
    CIMObjectPath localObjectPath = CIMObjectPath(
            String(),
            CIMNamespaceName(),
            objectName.getClassName(),
            objectName.getKeyBindings());

    // begin processing the request
    handler.processing();

    // Filter the instances from the list of association instances against
    // the specified role filter
    //
    Array<CIMInstance> resultInstances;
    if (resultClass == SAMPLE_TEACHERSTUDENT)
    {
        resultInstances =
            _filterAssociationInstancesByRole(_TSassociationInstances,
                localObjectPath, role);
    }
    else if (resultClass == SAMPLE_ADVISORSTUDENT)
    {
        resultInstances =
            _filterAssociationInstancesByRole(_ASassociationInstances,
                localObjectPath, role);
    }
    else
    {
        throw CIMNotSupportedException(
            resultClass.getString() + " is not supported");
    }

    // return the instances
    for (Uint32 i = 0, n = resultInstances.size(); i < n; i++)
    {
        handler.deliver(resultInstances[i]);
    }

    // complete processing the request
    handler.complete();
}

void AssociationProvider::referenceNames(
        const OperationContext& context,
        const CIMObjectPath& objectName,
        const CIMName& resultClass,
        const String& role,
        ObjectPathResponseHandler& handler)
{
    // validate namespace
    const CIMNamespaceName& nameSpace = objectName.getNameSpace();
    if (!nameSpace.equal(NAMESPACE))
    {
        throw CIMNotSupportedException(
            nameSpace.getString() + " not supported.");
    }

    // Build a host and namespace independent object path
    CIMObjectPath localObjectPath = CIMObjectPath(
        String(),
        CIMNamespaceName(),
        objectName.getClassName(),
        objectName.getKeyBindings());

    // begin processing the request
    handler.processing();

    // Filter the instances from the list of association instances against
    // the specified role filter
    //
    Array<CIMInstance> resultInstances;
    if (resultClass == SAMPLE_TEACHERSTUDENT)
    {
        resultInstances =
            _filterAssociationInstancesByRole(_TSassociationInstances,
                localObjectPath, role);
    }
    else if (resultClass == SAMPLE_ADVISORSTUDENT)
    {
        resultInstances =
            _filterAssociationInstancesByRole(_ASassociationInstances,
                localObjectPath, role);
    }
    else
    {
        throw CIMNotSupportedException(
            resultClass.getString() + " is not supported");
    }

    // return the instance names
    for (Uint32 i = 0, n = resultInstances.size(); i < n; i++)
    {
        CIMObjectPath objectPath = resultInstances[i].getPath();
        handler.deliver(objectPath);
    }

    // complete processing the request
    handler.complete();
}

///////////////////////////////////////////////////////////////////////////////
//  Private methods
///////////////////////////////////////////////////////////////////////////////

void AssociationProvider::_getInstance(
    const Array<CIMInstance>& instances,
    const CIMObjectPath& localReference,
    InstanceResponseHandler& handler)
{
    // instance index corresponds to reference index
    for (Uint32 i = 0, n = instances.size(); i < n; i++)
    {
        if (localReference == instances[i].getPath())
        {
            // deliver requested instance
            handler.deliver(instances[i]);
            return;
        }
    }
    throw CIMException(CIM_ERR_NOT_FOUND);
}

void AssociationProvider::_enumerateInstances(
    const Array<CIMInstance>& instances,
    InstanceResponseHandler& handler)
{
    // instance index corresponds to reference index
    for (Uint32 i = 0, n = instances.size(); i < n; i++)
    {
        // deliver instance
        handler.deliver(instances[i]);
    }
}

void AssociationProvider::_enumerateInstanceNames(
    const Array<CIMInstance>& instances,
    ObjectPathResponseHandler& handler)
{
    for (Uint32 i = 0, n = instances.size(); i < n; i++)
    {
        // deliver instance name
        handler.deliver(instances[i].getPath());
    }
}

void AssociationProvider::_associators(
    const Array<CIMInstance>& associationInstances,
    const CIMObjectPath& localReference,
    const String& role,
    const CIMName& resultClass,
    const String& resultRole,
    ObjectResponseHandler& handler)
{
    // Filter the instances from the list of association instances against
    // the specified role filter
    //
    Array<CIMInstance> assocInstances;
    assocInstances= _filterAssociationInstancesByRole(associationInstances,
        localReference, role);

    // Now filter the result association instances against the specified
    // resultClass and resultRole filters
    //
    for (Uint32 i = 0, m = assocInstances.size(); i < m; i++)
    {
        Array<CIMObjectPath> resultPaths;
        resultPaths = _filterAssociationInstances(assocInstances[i],
            localReference, resultClass, resultRole);

        for (Uint32 j = 0, n = resultPaths.size(); j < n; j++)
        {
            CIMName className = resultPaths[j].getClassName();
            if (className == SAMPLE_TEACHER)
            {
                // find instance that corresponds to the reference
                for (Uint32 k = 0, s = _teacherInstances.size(); k < s; k++)
                {
                    CIMObjectPath path = _teacherInstances[k].getPath();

                    // Build a host and namespace independent object path
                    CIMObjectPath localPath = CIMObjectPath(
                        String(),
                        CIMNamespaceName(),
                        path.getClassName(),
                        path.getKeyBindings());

                    if (resultPaths[j].identical(localPath))
                    {
                        // deliver the instance
                        handler.deliver(_teacherInstances[k]);
                    }
                }
            }
            else if (className == SAMPLE_STUDENT)
            {
                // find instance that corresponds to the reference
                for (Uint32 k = 0, s = _studentInstances.size(); k < s; k++)
                {
                    CIMObjectPath path = _studentInstances[k].getPath();

                    // Build a host and namespace independent object path
                    CIMObjectPath localPath = CIMObjectPath(
                        String(),
                        CIMNamespaceName(),
                        path.getClassName(),
                        path.getKeyBindings());

                    if (resultPaths[j].identical(localPath))
                    {
                        // deliver instance
                        handler.deliver(_studentInstances[k]);
                    }
                }
            }
        }
    }
}

void AssociationProvider::_associatorNames(
    const Array<CIMInstance>& associationInstances,
    const CIMObjectPath& localReference,
    const String& role,
    const CIMName& resultClass,
    const String& resultRole,
    ObjectPathResponseHandler& handler)
{
    // Filter the instances from the list of association instances against
    // the specified role filter
    //
    Array<CIMInstance> assocInstances;
    assocInstances= _filterAssociationInstancesByRole(associationInstances,
        localReference, role);

    // Now filter the result association instances against the specified
    // resultClass and resultRole filters
    //
    for (Uint32 i = 0, n = assocInstances.size(); i < n; i++)
    {
        Array<CIMObjectPath> resultPaths;
        resultPaths = _filterAssociationInstances(assocInstances[i],
            localReference, resultClass, resultRole);

        for (Uint32 i = 0, n = resultPaths.size(); i < n; i++)
        {
            handler.deliver(resultPaths[i]);
        }
    }
}

/**
 ***************************************************************************
   _filterAssociationInstancesByRole is used to filter the list of association
   instances against the specified role filter.  It returns a list of
   association instances that pass the filter test.

    @param associationInstance   - The target association instances
    @param targetObjectPath      - The target ObjectPath
    @param role                  - The role filter.  If there is no role, this
                                   is String::EMPTY

    @return   the set of association instances that pass the filter test.
 ***************************************************************************
*/
Array<CIMInstance> AssociationProvider::_filterAssociationInstancesByRole(
    const Array<CIMInstance>& associationInstances,
    const CIMObjectPath& targetObjectPath,
    const String& role)
{
    Array<CIMInstance> returnInstances;

    // Filter the instances from the list of association instances against
    // the specified role filter
    //
    for (Uint32 i = 0, n = associationInstances.size(); i < n; i++)
    {
        CIMInstance instance = associationInstances[i];

        // Search the association instance for all reference properties
        for (Uint32 i = 0, n = instance.getPropertyCount(); i < n; i++)
        {
            const CIMProperty p = instance.getProperty(i);
            if (p.getType() == CIMTYPE_REFERENCE)
            {
                CIMValue v = p.getValue();
                CIMObjectPath path;
                v.get(path);

                if ((role == String::EMPTY) ||
                    (p.getName() == CIMName(role)))
                {
                    if (targetObjectPath.identical(path))
                    {
                        returnInstances.append(instance);
                    }
                }
            }
        }
    }
    return returnInstances;
}

/**
 ***************************************************************************
   _filterAssociationInstances is used to filter the set of possible return
   instances against the filters (resultClass and resultRole) provided with
   the associators and associatorNames operations.  It returns the ObjectPaths
   of the set of objects that pass the filter tests.

    @param assocInstance     - The target association class instance
    @param sourceObjectPath  - The source ObjectPath
    @param resultClass       - The result class. If there is no resultClass,
                               this is String::EMPTY.
    @param resultRole        - The result role. If there is no role, this is
                               String::EMPTY

    @return   the ObjectPaths of the set of association instances that pass
              the filter tests.
 ***************************************************************************
*/
Array<CIMObjectPath> AssociationProvider::_filterAssociationInstances(
    CIMInstance& assocInstance,
    const CIMObjectPath& sourceObjectPath,
    CIMName resultClass,
    String resultRole)
{
    Array<CIMObjectPath> returnPaths;

    // get all Reference properties
    for (Uint32 i = 0, n = assocInstance.getPropertyCount(); i < n; i++)
    {
        CIMProperty p = assocInstance.getProperty(i);

        if (p.getType() == CIMTYPE_REFERENCE)
        {
            CIMValue v = p.getValue();
            CIMObjectPath path;
            v.get(path);

            if (!sourceObjectPath.identical(path))
            {
                if (resultClass.isNull() || resultClass == path.getClassName())
                {
                    if (resultRole == String::EMPTY ||
                        (p.getName() == CIMName(resultRole)))
                    {
                        returnPaths.append(path);
                    }
                }
            }
        }
    }
    return returnPaths;
}

/**
 ***************************************************************************
    _createDefaultInstances creates the dynamic instances for this provider.
 ***************************************************************************
*/
void AssociationProvider::_createDefaultInstances()
{
    //
    // create 4 instances of the Sample_Teacher class
    //
    _teacherInstances.append(_createInstance(SAMPLE_TEACHER, "Teacher1", 1));
    _teacherInstances.append(_createInstance(SAMPLE_TEACHER, "Teacher2", 2));
    _teacherInstances.append(_createInstance(SAMPLE_TEACHER, "Teacher3", 3));
    _teacherInstances.append(_createInstance(SAMPLE_TEACHER, "Teacher4", 4));

    //
    // create 3 instances of the Sample_Student class
    //
    _studentInstances.append(_createInstance(SAMPLE_STUDENT, "Student1", 1));
    _studentInstances.append(_createInstance(SAMPLE_STUDENT, "Student2", 2));
    _studentInstances.append(_createInstance(SAMPLE_STUDENT, "Student3", 3));

    //
    // create the instances for the Sample_TeacherStudent association class
    //    (Teacher1, Student1)
    //    (Teacher1, Student2)
    //    (Teacher1, Student3)
    //    (Teacher2, Student1)
    //    (Teacher2, Student2)
    //    (Teacher3, Student2)
    //    (Teacher3, Student3)
    //    (Teacher4, Student1)
    //
    _TSassociationInstances.append(_createTSAssociationInstance(
        _teacherInstances[0].getPath(), _studentInstances[0].getPath()));
    _TSassociationInstances.append(_createTSAssociationInstance(
        _teacherInstances[0].getPath(), _studentInstances[1].getPath()));
    _TSassociationInstances.append(_createTSAssociationInstance(
        _teacherInstances[0].getPath(), _studentInstances[2].getPath()));
    _TSassociationInstances.append(_createTSAssociationInstance(
        _teacherInstances[1].getPath(), _studentInstances[0].getPath()));
    _TSassociationInstances.append(_createTSAssociationInstance(
        _teacherInstances[1].getPath(), _studentInstances[1].getPath()));
    _TSassociationInstances.append(_createTSAssociationInstance(
        _teacherInstances[2].getPath(), _studentInstances[1].getPath()));
    _TSassociationInstances.append(_createTSAssociationInstance(
        _teacherInstances[2].getPath(), _studentInstances[2].getPath()));
    _TSassociationInstances.append(_createTSAssociationInstance(
        _teacherInstances[3].getPath(), _studentInstances[0].getPath()));

    //
    // create the instances for the Sample_AdvisorStudent association class
    //    (Teacher1, Student1)
    //    (Teacher1, Student2)
    //    (Teacher2, Student3)
    //
    _ASassociationInstances.append(_createASAssociationInstance(
        _teacherInstances[0].getPath(), _studentInstances[0].getPath()));
    _ASassociationInstances.append(_createASAssociationInstance(
        _teacherInstances[0].getPath(), _studentInstances[1].getPath()));
    _ASassociationInstances.append(_createASAssociationInstance(
        _teacherInstances[1].getPath(), _studentInstances[2].getPath()));

    return;
}

CIMInstance AssociationProvider::_createInstance(
    const CIMName& className, const String& name, Uint8 id)
{
    CIMInstance instance(className);
    instance.addProperty(CIMProperty("Name", name));
    instance.addProperty(CIMProperty("Identifier", id));

    // Build CIMObjectPath from keybindings
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding(CIMName("Name"), name,
                                     CIMKeyBinding::STRING));
    CIMObjectPath path("", CIMNamespaceName(), className, keyBindings);
    instance.setPath(path);

    return instance;
}

CIMInstance AssociationProvider::_createTSAssociationInstance(
    CIMObjectPath ref1, CIMObjectPath ref2)
{
    CIMInstance assocInst(SAMPLE_TEACHERSTUDENT);
    assocInst.addProperty(CIMProperty("Teaches", ref1, 0, SAMPLE_TEACHER));
    assocInst.addProperty(CIMProperty("TaughtBy", ref2, 0, SAMPLE_STUDENT));

    // Build CIMObjectPath from keybindings
    Array <CIMKeyBinding> keyBindings;
    CIMKeyBinding binding1(
        CIMName("Teaches"), ref1.toString(), CIMKeyBinding::REFERENCE);
    CIMKeyBinding binding2(
        CIMName("TaughtBy"), ref2.toString(), CIMKeyBinding::REFERENCE);
    keyBindings.append (binding1);
    keyBindings.append (binding2);

    CIMObjectPath path(
        "", CIMNamespaceName(), SAMPLE_TEACHERSTUDENT, keyBindings);

    assocInst.setPath(path);

    return assocInst;
}

CIMInstance AssociationProvider::_createASAssociationInstance(
    CIMObjectPath ref1,
    CIMObjectPath ref2)
{
    CIMInstance assocInst(SAMPLE_ADVISORSTUDENT);
    assocInst.addProperty(CIMProperty("Advises", ref1, 0, SAMPLE_TEACHER));
    assocInst.addProperty(CIMProperty("AdvisedBy", ref2, 0, SAMPLE_STUDENT));

    // Build CIMObjectPath from keybindings
    Array <CIMKeyBinding> keyBindings;
    CIMKeyBinding binding1(
        CIMName("Advises"), ref1.toString(), CIMKeyBinding::REFERENCE);
    CIMKeyBinding binding2(
        CIMName("AdvisedBy"), ref2.toString(), CIMKeyBinding::REFERENCE);
    keyBindings.append(binding1);
    keyBindings.append(binding2);

    CIMObjectPath path(
        "", CIMNamespaceName(), SAMPLE_ADVISORSTUDENT, keyBindings);

    assocInst.setPath(path);

    return assocInst;
}
