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

#include "AssociationTestProvider.h"
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/General/MofWriter.h>


PEGASUS_NAMESPACE_BEGIN

AssociationTestProvider::AssociationTestProvider()
{
}

AssociationTestProvider::~AssociationTestProvider()
{
}

CIMInstance _createInstanceFromClass(
    CIMClass& classObject,
    Boolean includeQualifiers = false,
    Boolean includeClassOrigin = false)
{
    CDEBUG("_createInstanceFromClass Function ");
    CIMInstance newInstance(classObject.getClassName());
    if (includeQualifiers)
    {
        for (Uint32 i = 0 ; i < classObject.getQualifierCount(); i++ )
        {
            CIMQualifier c;
            c = classObject.getQualifier(i);
            newInstance.addQualifier(c);
            // Need to sort out the value.
        }
    }
    for (Uint32 i = 0 ; i < classObject.getPropertyCount() ; i++)
    {
        CIMProperty p;
        p = classObject.getProperty(i);
        // Remove any qualifiers if includeQualifiers false
        if (!includeQualifiers)
        {
            for (Uint32 j = 0 ; j < p.getQualifierCount() ; j ++)
            {
                p.removeQualifier(j);
            }
        }
        // Setting the value for the property to default
        newInstance.addProperty(p);
    }

    XmlWriter::printClassElement(classObject);
    XmlWriter::printInstanceElement(newInstance);
    return newInstance;
}

void _addQualifiersToInstance(
    CIMClass& classObject,
    CIMInstance& instance)
{
    CDEBUG("_addQualifiersToInstance");
    for (Uint32 i = 0 ; i < classObject.getQualifierCount(); i++ )
    {
        CIMQualifier c;
        c = classObject.getQualifier(i);
        instance.addQualifier(c);
        // Need to sort out the value issues.
    }
    for (Uint32 i = 0 ; i < classObject.getPropertyCount() ; i++)
    {
        CIMProperty p;
        p = classObject.getProperty(i);
        // Remove any qualifiers if includeQualifiers false
        for (Uint32 j = 0 ; j < p.getQualifierCount() ; j ++)
        {
            p.addQualifier(p.getQualifier(j));
        }
    }
    XmlWriter::printClassElement(classObject);
    XmlWriter::printInstanceElement(instance);
}

CIMInstance _makeInstance(
    CIMClass& classObject,
    Boolean includeQualifiers,
    String& name)
{
    CIMInstance instance;
    instance = _createInstanceFromClass(classObject, includeQualifiers);
    // put the new name in the name property.
    Uint32 pos;
    if ((pos = instance.findProperty(name)) != PEG_NOT_FOUND)
    {
        // set the name filed
        CIMProperty p;
        p = instance.getProperty(pos);
        PEGASUS_TEST_ASSERT(p.getType() == CIMTYPE_STRING);
        CIMValue v;
        v = p.getValue();
        PEGASUS_TEST_ASSERT(v.getType() ==  CIMTYPE_STRING);
        //... set the new value in the CIMValue
        v.set(name);
        p.setValue(v);

    }
    else     // property not found
    {
        CDEBUG("_makeInstance cannot find property");
    }
    return instance;
}

/** Get the class defined by the className from the CIMOM
 *  Note that there is an exception return if the class cannot
 *  be found
 */
CIMClass _getClassFromCIMOM(
    CIMOMHandle& cimom,
    CIMNamespaceName& nameSpace,
    CIMName& className,
    Boolean includeQualifiers = false)
{
    CIMClass classObject;
    try
    {
        classObject = cimom.getClass(
            OperationContext(),
            nameSpace,
            className,
            false,
            includeQualifiers,
            false,
            CIMPropertyList());
    }
    catch (CIMException& e)
    {
        CDEBUG("Exception hit " << e.getMessage());
        throw CIMException(CIM_ERR_NOT_FOUND);
    }
    return classObject;
}

void AssociationTestProvider::initialize(CIMOMHandle& cimom)
{

    CDEBUG ("initialize");
    _cimom = cimom;
    /* Initialize all classes and associations to a protected set of tables.
        Classes and associatons are as defined in the AssociationTest MOF
    */
    CIMNamespaceName nameSpace("root/testassoc");

    String instanceID = "INSTANCEA";
    _classTableItem cti;
    cti.className = "TST_ClassA";
    CIMInstance thisInstance;
    cti.thisClass = _getClassFromCIMOM(_cimom, nameSpace, cti.className);
    thisInstance = _makeInstance(cti.thisClass, false, instanceID);
    cti.instances.append(thisInstance);
    _classTable.append(cti);
    XmlWriter::printClassElement(cti.thisClass);
    XmlWriter::printInstanceElement(thisInstance);

}

void AssociationTestProvider::terminate()
{
    // ATTN: additional cleanup needed
    CDEBUG("Terminate AssociationTestProvider ");
    delete this;
}

void AssociationTestProvider::getInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    throw CIMNotSupportedException("AssociationTestProvider::getInstance");

    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    // begin processing the request
    handler.processing();

    // instance index corresponds to reference index
    for (Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        if (localReference == _instances[i].buildPath(_referencedClass))
        // if (localReference == _instanceNames[i])
        {
            // deliver requested instance
            handler.deliver(_instances[i]);
            break;
        }
    }

    // complete processing the request
    handler.complete();
}

void AssociationTestProvider::enumerateInstances(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    CDEBUG("enumerateInstances of "  << classReference.toString());
    handler.processing();

    // Find the class corresponding to this instance
    CIMName reqClassName = classReference.getClassName();
    for (Uint32 i = 0; i < _classTable.size() ; i++)
    {
        if (reqClassName == _classTable[i].className)
        {
            CDEBUG("Class found " << reqClassName);
            for (Uint32 j = 0; j < _classTable[i].instances.size(); j++)
            {
                handler.deliver(_classTable[i].instances[j]);
            }
            handler.complete();
            return;
        }
    }
    throw CIMException(CIM_ERR_NOT_FOUND);
}

void AssociationTestProvider::enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath& classReference,
    ObjectPathResponseHandler& handler)
{
    CDEBUG("Enumerate InstanceNames of " << classReference.toString());
    // begin processing the request
    handler.processing();
    //handler.deliver(_instances[i].buildPath(cimClass));
    //
    CIMName reqClassName = classReference.getClassName();
    for (Uint32 i = 0; i < _classTable.size() ; i++)
    {
        if (reqClassName == _classTable[i].className)
        {
            CIMClass cimClass = _classTable[i].thisClass;
            CDEBUG("Class found " << reqClassName);
            for (Uint32 j = 0; j < _classTable[i].instances.size(); j++)
            {
                handler.deliver(
                    _classTable[i].instances[j].buildPath(cimClass));
            }
            handler.complete();
            return;
        }
    }
    throw CIMException(CIM_ERR_NOT_FOUND);
}

void AssociationTestProvider::modifyInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler& handler)
{
    throw CIMNotSupportedException("AssociationTestProvider::modifyInstance");

    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    // begin processing the request
    handler.processing();

    // instance index corresponds to reference index
    for (Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        //if (localReference == _instanceNames[i])
        if (localReference == _instances[i].buildPath(_referencedClass))
        {
            // overwrite existing instance
            _instances[i] = instanceObject;

            break;
        }
    }

    // complete processing the request
    handler.complete();
}

void AssociationTestProvider::createInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    const CIMInstance& instanceObject,
    ObjectPathResponseHandler& handler)
{
    throw CIMNotSupportedException("AssociationTestProvider::createInstance");

    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    // instance index corresponds to reference index
    for (Uint32 i = 0, n = _instanceNames.size(); i < n; i++)
    {
        if (localReference == _instanceNames[i])
        {
            throw CIMObjectAlreadyExistsException(localReference.toString());
        }
    }

    // begin processing the request
    handler.processing();

    // add the new instance to the array
    _instances.append(instanceObject);
    _instanceNames.append(instanceReference);

    // deliver the new instance
    handler.deliver(_instanceNames[_instanceNames.size() - 1]);

    // complete processing the request
    handler.complete();
}

void AssociationTestProvider::deleteInstance(
    const OperationContext& context,
    const CIMObjectPath& instanceReference,
    ResponseHandler& handler)
{
    // convert a potential fully qualified reference into a local reference
    // (class name and keys only).
    CIMObjectPath localReference = CIMObjectPath(
        String(),
        String(),
        instanceReference.getClassName(),
        instanceReference.getKeyBindings());

    // begin processing the request
    handler.processing();
    CIMName myClass = instanceReference.getClassName();
    if (myClass == CIMName("tst_persondynamic"))
    {
        for (Uint32 i = 0, n = _instances.size(); i < n; i++)
        {
            // save the instance locally
            CIMInstance cimInstance(_instances[i]);

            // remove instance from the array
            _instances.remove(i);
            _instanceNames.remove(i);

            // exit loop
            break;
        }
    }
    if (myClass == CIMName("tst_lineagedynamic"))
    {
        for (Uint32 i = 0, n = _instancesLineageDynamic.size(); i < n; i++)
        {
            // save the instance locally
            CIMInstance cimInstance(_instancesLineageDynamic[i]);

            // remove instance from the array
            _instancesLineageDynamic.remove(i);
            _instanceNamesLineageDynamic.remove(i);

            // exit loop
            break;
        }

    }
    if (myClass ==  CIMName("tst_labeledlineagedynamic"))
    {
        for (Uint32 i = 0, n = _instancesLabeledLineageDynamic.size();
             i < n; i++)
        {
            // save the instance locally
            CIMInstance cimInstance(_instancesLineageDynamic[i]);

            // remove instance from the array
            _instancesLabeledLineageDynamic.remove(i);
            _instanceNamesLabeledLineageDynamic.remove(i);

            // exit loop
            break;
        }

    }
    handler.complete();
    /*
    // instance index corresponds to reference index
    for (Uint32 i = 0, n = _instances.size(); i < n; i++)
    {
        if (localReference == _instanceNames[i])
        {
            // save the instance locally
            CIMInstance cimInstance(_instances[i]);

            // remove instance from the array
            _instances.remove(i);
            _instanceNames.remove(i);

            // exit loop
            break;
        }
    }
    **** Delete this whole thing*/
    // complete processing the request
}

void AssociationTestProvider::associators(
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
    CDEBUG("Associators");
    // begin processing the request
    // Get the namespace and host names to create the CIMObjectPath

    String nameSpace = "SampleProvider";
    String host = System::getHostName();

    handler.processing();
    // Here make the decision between Lineage and LabeledLineage

    // For all of the association objects.
    // This is wrong.  Simply want to deliver something right now.
    for (Uint32 i = 0, n = _instanceNamesLineageDynamic.size(); i < n; i++)
    {
        /*
        The AssocClass input parameter, if not NULL, MUST be a valid CIM
        Association Class name.  It acts as a filter on the returned set of
        Objects by mandating that each returned Object MUST be associated to
        the source Object via an Instance of this Class or one of its
        subclasses.

        The ResultClass input parameter, if not NULL, MUST be a valid CIM Class
        name.  It acts as a filter on the returned set of Objects by mandating
        that each returned Object MUST be either an Instance of this Class (or
        one of its subclasses) or be this Class (or one of its subclasses).

        The Role input parameter, if not NULL, MUST be a valid Property name.
        It acts as a filter on the returned set of Objects by mandating that
        each returned Object MUST be associated to the source Object via an
        Association in which the source Object plays the specified role (i.e.,
        the name of the Property in the Association Class that refers to the
        source Object MUST match the value of this parameter).

        The ResultRole input parameter, if not NULL, MUST be a valid Property
        name.  It acts as a filter on the returned set of Objects by mandating
        that each returned Object MUST be associated to the source Object via
        an Association in which the returned Object plays the specified role
        (i.e., the name of the Property in the Association Class that refers
        to the returned Object MUST match the value of this parameter).

        If the IncludeQualifiers input parameter is true, this specifies that
        all Qualifiers for each Object (including Qualifiers on the Object and
        on any returned Properties) MUST be included as <QUALIFIER> elements
        in the response.  If false no <QUALIFIER> elements are present in each
        returned Object.

        If the IncludeClassOrigin input parameter is true, this specifies that
        the CLASSORIGIN attribute MUST be present on all appropriate elements
        in each returned Object.  If false, no CLASSORIGIN attributes are
        present in each returned Object.
        */

        // Filter out by resultClass and role.

        // Note that here we test to determine if the returned object name
        // equals resultClass or any of its subclasses

        // NOTE: This code is just a hack today to return objects so we can
        // test the internal paths.  In fact, it just returns the associations,
        // not the corresponding objects from the association.
        // ATTN: Fix this KS.

        CIMInstance instance = _instancesLineageDynamic[i];

        // Filter out associations that do not match the association class
        if (instance.getClassName().equal(associationClass))
        {
            CDEBUG("FamilyProvider AssociatorsResult Class = " <<
                resultClass.getString()<< " Role = " << role);
            if (resultClass.isNull() ||
                instance.getClassName().equal(resultClass))
            {
                // Incomplete.  Need to add the other filters.
                CIMObjectPath objectPathInput = objectName;
                if (objectPathInput.getHost().size() == 0)
                    objectPathInput.setHost(host);

                if (objectPathInput.getNameSpace().isNull())
                    objectPathInput.setNameSpace(nameSpace);
                CIMObject cimObject(instance);
                cimObject.setPath (objectPathInput);

                // set path from CIMObjectPath containing the path.
                handler.deliver(cimObject);
            }
        }
    }

    // complete processing the request
    handler.complete();
}

void AssociationTestProvider::associatorNames(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& associationClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    ObjectPathResponseHandler& handler)
{
    // Get the namespace and host names to create the CIMObjectPath
    String nameSpace = "SampleProvider";
    String host = System::getHostName();
    // ATTN: Just a hack to get objects back. Note that today it returns the
    // association class, not the corresponding
    CDEBUG("Result Class = " << resultClass.getString() << " Role = " << role);
    for (Uint32 i = 0, n = _instanceNamesLineageDynamic.size(); i < n; i++)
    {
        // Filter out by resultClass and role.
        // The ResultClass input parameter, if not NULL, MUST be a valid CIM
        // Class name.  It acts as a filter on the returned set of Object
        // Names by mandating that each returned Object Name MUST identify an
        // Instance of this Class (or one of its subclasses), or this Class
        // (or one of its subclasses).

        // The Role input parameter, if not NULL, MUST be a valid Property
        // name. It acts as a filter on the returned set of Object Names by
        // mandating that each returned Object Name MUST identify an Object
        // that refers to the target Instance via a Property whose name
        // matches the value of this parameter.

        // Note that here we test to determine if the returned object name
        // equals resultClass or any of its subclasses

        CIMObjectPath r = _instanceNamesLineageDynamic[i];

        if (resultClass.isNull() || r.getClassName().equal(resultClass))
        {
            CDEBUG("Sending AssociatorNameResponse");
            if (r.getHost().size() == 0)
                r.setHost(host);

            if (r.getNameSpace().isNull())
                r.setNameSpace(nameSpace);
        }
        handler.deliver(r);
    }

    // complete processing the request
    handler.complete();
}

void AssociationTestProvider::references(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    ObjectResponseHandler& handler)
{
    CDEBUG("references");
    // begin processing the request
    // Get the namespace and host names to create the CIMObjectPath
    String nameSpace = "SampleProvider";
    String host = System::getHostName();

    handler.processing();
    // Here make the decision between Lineage and LabeledLineage
    CIMName objectClassName = objectName.getClassName();

    // For all of the corresponding association objects.
    // NOTE: This is just a hack today to return objects so we can get paths
    // running
    for (Uint32 i = 0, n = _instanceNamesLineageDynamic.size(); i < n; i++)
    {
        // Filter out by resultClass and role.
        // The ResultClass input parameter, if not NULL, MUST be a valid CIM
        // Class name.  It acts as a filter on the returned set of Object
        // Names by mandating that each returned Object Name MUST identify an
        // Instance of this Class (or one of its subclasses), or this Class
        // (or one of its subclasses).

        // The Role input parameter, if not NULL, MUST be a valid Property
        // name. It acts as a filter on the returned set of Object Names by
        // mandating that each returned Object Name MUST identify an Object
        // that refers to the target Instance via a Property whose name
        // matches the value of this parameter.

        // Note that here we test to determine if the returned object name
        // equals resultClass or any of its subclasses

        CIMInstance instance = _instancesLineageDynamic[i];

        CDEBUG("References Result Class = " << resultClass.getString() <<
            " Role = " << role);
        if (resultClass.isNull() || instance.getClassName().equal(resultClass))
        {
            // Incomplete.  Need to add the other filters.
            CIMObjectPath objectPathInput = objectName;
            if (objectPathInput.getHost().size() == 0)
                objectPathInput.setHost(host);

            if (objectPathInput.getNameSpace().isNull())
                objectPathInput.setNameSpace(nameSpace);
            CIMObject cimObject(instance);
            cimObject.setPath (objectPathInput);
            handler.deliver(cimObject);
        }
    }

    // complete processing the request
    handler.complete();
}


// Return all references (association instance names) in which the given
// object is involved.

void AssociationTestProvider::referenceNames(
    const OperationContext& context,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    ObjectPathResponseHandler& handler)
{
    CDEBUG("ReferenceNames Operation");

    // Get the namespace and host names to create the CIMObjectPath
    String nameSpace = objectName.getNameSpace();
    String host = System::getHostName();

    CIMName myClass = objectName.getClassName();
    CIMClass cimClass;
    try
    {
        cimClass = _cimom.getClass(
            OperationContext(),
            objectName.getNameSpace(),
            objectName.getClassName(),
            false,
            false,
            false,
            CIMPropertyList());
    }
    catch (CIMException& e)
    {
        CDEBUG("Exception hit " << e.getMessage());
        throw CIMException(CIM_ERR_NOT_FOUND);
    }

    // For all of the association objects.
    CDEBUG("resultClass = " << ((resultClass.isNull())? "NULL" : resultClass));
    for (Uint32 i = 0, n = _instancesLineageDynamic.size(); i < n; i++)
    {
        // Filter out by resultClass and role.
        // The ResultClass input parameter, if not NULL, MUST be a valid CIM
        // Class name.  It acts as a filter on the returned set of Object
        // Names by mandating that each returned Object Name MUST identify an
        // Instance of this Class (or one of its subclasses), or this Class
        // (or one of its subclasses).

        // The Role input parameter, if not NULL, MUST be a valid Property
        // name. It acts as a filter on the returned set of Object Names by
        // mandating that each returned Object Name MUST identify an Object
        // that refers to the target Instance via a Property whose name
        // matches the value of this parameter.

        // Note that here we test to determine if the returned object name
        // equals resultClass or any of its subclasses

        //CIMObjectPath objectPath = _instanceNamesLineageDynamic[i];

        // ATTN: Need to dynamically get the assocClass and also put a try
        // block around the following code.
        CIMObjectPath objectPath =
            _instancesLineageDynamic[i].buildPath(_assocClass);
        if (resultClass.isNull() ||
            objectPath.getClassName().equal(resultClass))
        {
            if (objectPath.getHost().size() == 0)
                objectPath.setHost(host);

            if (objectPath.getNameSpace().isNull())
                objectPath.setNameSpace(nameSpace);
            handler.deliver(objectPath);
        }
    }

    // complete processing the request
    handler.complete();
}

PEGASUS_NAMESPACE_END
