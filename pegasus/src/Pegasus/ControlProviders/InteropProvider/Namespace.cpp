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

///////////////////////////////////////////////////////////////////////////////
//  Interop Provider - This provider services those classes from the
//  DMTF Interop schema in an implementation compliant with the SMI-S v1.1
//  Server Profile
//
//  Please see PG_ServerProfile20.mof in the directory
//  $(PEGASUS_ROOT)/Schemas/Pegasus/InterOp/VER20 for retails regarding the
//  classes supported by this control provider.
//
//  Interop forces all creates to the PEGASUS_NAMESPACENAME_INTEROP
//  namespace. There is a test on each operation that returns
//  the Invalid Class CIMDError
//  This is a control provider and as such uses the Tracer functions
//  for data and function traces.  Since we do not expect high volume
//  use we added a number of traces to help diagnostics.
///////////////////////////////////////////////////////////////////////////////

#include "InteropProvider.h"
#include "InteropProviderUtils.h"
#include "InteropConstants.h"
#include <Pegasus/Common/ArrayIterator.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

//
// Property names for CIM_Namespace Class
//
#define CIM_NAMESPACE_PROPERTY_NAME  COMMON_PROPERTY_NAME
#define CIM_NAMESPACE_PROPERTY_CREATIONCLASSNAME \
    COMMON_PROPERTY_CREATIONCLASSNAME
#define CIM_NAMESPACE_PROPERTY_SYSTEMCREATIONCLASSNAME \
    COMMON_PROPERTY_SYSTEMCREATIONCLASSNAME
#define CIM_NAMESPACE_PROPERTY_SYSTEMNAME COMMON_PROPERTY_SYSTEMNAME
static const CIMName CIM_NAMESPACE_PROPERTY_OBJECTMANAGERCREATIONCLASSNAME(
    "ObjectManagerCreationClassName");
static const CIMName CIM_NAMESPACE_PROPERTY_OBJECTMANAGERNAME(
    "ObjectManagerName");
static const CIMName CIM_NAMESPACE_PROPERTY_CLASSINFO(
    "ClassInfo");
static const CIMName CIM_NAMESPACE_PROPERTY_DESCRIPTIONOFCLASSINFO(
    "DescriptionOfClassInfo");
static const CIMName CIM_NAMESPACE_PROPERTY_CLASSTYPE("ClassType");

// Additional Property names for PG_Namespace Class

static const CIMName PG_NAMESPACE_PROPERTY_SCHEMAUPDATESALLOWED(
    "SchemaUpdatesAllowed");
static const CIMName PG_NAMESPACE_PROPERTY_ISSHAREABLE(
    "IsShareable");
static const CIMName PG_NAMESPACE_PROPERTY_PARENTNAMESPACE(
    "ParentNamespace");
#define PG_NAMESPACE_PROPERTY_NAME COMMON_PROPERTY_NAME

//
// Get the instances of CIM_Namespace. Gets all instances of the namespace from
// the repository namespace management functions. Builds instances that
// match all of the request attributes.
//
Array<CIMInstance> InteropProvider::enumNamespaceInstances()
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::enumNamespaceInstances()");

    Array<CIMNamespaceName> namespaceNames = repository->enumerateNameSpaces();
    Array<CIMInstance> instanceArray;

    // Build instances of PG namespace since that is the leaf class
    for (Uint32 i = 0, n = namespaceNames.size(); i < n; i++)
    {
       instanceArray.append(
           buildNamespaceInstance(namespaceNames[i].getString()));
    }

    PEG_METHOD_EXIT();
    return instanceArray;
}

//
// Returns an array of all of the NamespaceInManager association instances
// for this CIMOM. One instance will be produced for every namespace present
// in the CIMOM/repository.
//
Array<CIMInstance> InteropProvider::enumNamespaceInManagerInstances()
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::enumNamespaceInManagerInstances()");

    Array<CIMInstance> namespaceInstances = enumNamespaceInstances();

    CIMObjectPath objectManagerPath = getObjectManagerInstance().getPath();

    Array<CIMInstance> assocInstances;
    CIMClass targetClass;

    CIMInstance instanceskel = buildInstanceSkeleton(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_NAMESPACEINMANAGER, true, targetClass);
    // Build and instance for each namespace instance.
    for (Uint32 i = 0 ; i < namespaceInstances.size() ; i++)
    {
        CIMInstance instance = instanceskel.clone();
        setPropertyValue(instance, PROPERTY_ANTECEDENT, objectManagerPath);
        setPropertyValue(instance, PROPERTY_DEPENDENT,
            namespaceInstances[i].getPath());

        CIMObjectPath objPath = instance.buildPath(targetClass);
        objPath.setHost(hostName);
        objPath.setNameSpace(PEGASUS_NAMESPACENAME_INTEROP);
        instance.setPath(objPath);
        assocInstances.append(instance);
    }
    PEG_METHOD_EXIT();
    return assocInstances;
}


//
// Generates one instance of the PG_Namespace class for the specified
// namespace.
//
CIMInstance InteropProvider::buildNamespaceInstance(
    const String & nameSpace)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::buildNamespaceInstance()");

    CIMClass targetClass;
    CIMInstance instance = buildInstanceSkeleton(
        PEGASUS_NAMESPACENAME_INTEROP, PEGASUS_CLASSNAME_PGNAMESPACE,
        true, targetClass);

    setCommonKeys(instance);

    // ObjectManagerCreationClassName
    setPropertyValue(instance,
        CIM_NAMESPACE_PROPERTY_OBJECTMANAGERCREATIONCLASSNAME,
        PEGASUS_CLASSNAME_PG_OBJECTMANAGER.getString());

    // ObjectManagerName
    setPropertyValue(instance, CIM_NAMESPACE_PROPERTY_OBJECTMANAGERNAME,
        objectManagerName);

    // CreationClassName
    setPropertyValue(instance, CIM_NAMESPACE_PROPERTY_CREATIONCLASSNAME,
        PEGASUS_CLASSNAME_PGNAMESPACE.getString());
    // Name
    // This is the namespace name itself
    setPropertyValue(instance, CIM_NAMESPACE_PROPERTY_NAME,
        nameSpace);

    // ClassInfo
    // Set the classinfo to unknown and the description to namespace.
    setPropertyValue(instance, CIM_NAMESPACE_PROPERTY_CLASSINFO, Uint16(0));

    // DescriptionOfClassInfo
    setPropertyValue(instance, CIM_NAMESPACE_PROPERTY_DESCRIPTIONOFCLASSINFO,
        String("namespace"));

    // ClassType
    setPropertyValue(instance, CIM_NAMESPACE_PROPERTY_CLASSTYPE,
        Uint16(2));

    //
    //  Everything above was commmon to CIM Namespace.  The following are
    //  PG_Namespace Properties
    //
    CIMRepository::NameSpaceAttributes attributes;
    repository->getNameSpaceAttributes(nameSpace, attributes);
    String parent;
    String name;
    Boolean shareable = false;
    Boolean updatesAllowed = true;
    for (CIMRepository::NameSpaceAttributes::Iterator i = attributes.start();
        i; i++)
    {
        String key=i.key();
        String value = i.value();
        if(String::equalNoCase(key,"shareable"))
        {
            if (String::equalNoCase(value,"true"))
                shareable=true;
        }
        else if(String::equalNoCase(key,"updatesAllowed"))
        {
            if (String::equalNoCase(value,"false"))
                updatesAllowed=false;
        }
        // Test to be sure we are returning proper namespace name
        else if (String::equalNoCase(key,"name"))
        {
            if (!String::equalNoCase(value, nameSpace))
            {
                PEG_METHOD_EXIT();
                // This is poor exception since it reflects internal error. Do
                // error log
                throw CIMNotSupportedException(
                    "Namespace attribute rtnd error for key " + key +
                    "expected " + nameSpace + value + " in " +
                    String(thisProvider));
            }

            name = value;
        }
        else if (String::equalNoCase(key,"parent"))
        {
            parent=value;
        }
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
        else if (String::equalNoCase(key,"remoteInfo"))
        {
            //ATTN: remoteInfo property is not part of PG_Namespace class,
            // add the property to PG_Namespace instance once avilable.
        }
#endif
        else
        {
            PEG_METHOD_EXIT();
            // Poor error definition since it reflects internal error. do error
            // log
            throw PEGASUS_CIM_EXCEPTION (CIM_ERR_NOT_SUPPORTED, nameSpace +
                " namespace attribute " + key + " option not supported in" +
                String(thisProvider));
        }
    }

    // SchemaUpdatesAllowed
    setPropertyValue(instance, PG_NAMESPACE_PROPERTY_SCHEMAUPDATESALLOWED,
        updatesAllowed);

    // IsShareable
    setPropertyValue(instance, PG_NAMESPACE_PROPERTY_ISSHAREABLE, shareable);

    // ParentNamespace
    setPropertyValue(instance, PG_NAMESPACE_PROPERTY_PARENTNAMESPACE, parent);
    setPropertyValue(instance, PG_NAMESPACE_PROPERTY_NAME, name);

    CIMObjectPath objPath = instance.buildPath(targetClass);
    objPath.setHost(hostName);
    objPath.setNameSpace(PEGASUS_NAMESPACENAME_INTEROP);
    instance.setPath(objPath);
    PEG_METHOD_EXIT();
    return instance;
}

CIMInstance InteropProvider::getNameSpaceInstance(
    const CIMObjectPath & ref)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "getNameSpaceInstance()");
    Array<CIMKeyBinding> keyBindings = ref.getKeyBindings();
    ConstArrayIterator<CIMKeyBinding> keyIter(keyBindings);
    String name;

    for (Uint32 i = 0; i < keyIter.size(); i++)
    {
        if (keyIter[i].getName().equal(CIM_NAMESPACE_PROPERTY_NAME))
        {
            name = keyIter[i].getValue();
            break;
        }
    }

    if(repository->nameSpaceExists(name))
    {
        CIMInstance newInst = buildNamespaceInstance(name);
        if( newInst.getPath() != ref )
        {
            throw CIMObjectNotFoundException(ref.toString());
        }
        PEG_METHOD_EXIT();
        return newInst;
    }

    PEG_METHOD_EXIT();
    throw CIMObjectNotFoundException(ref.toString());
}

//
// Function that takes an instance of the CIM_Namespace class, checks whether
// the key properties are present,
//
String buildNamespacePath(
    CIMObjectPath & namespacePath,
    const CIMInstance& instance,
    const String & objectManagerName)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "buildNamespacePath()");

    unsigned int propIndex = PEG_NOT_FOUND;
    CIMName propertyName;

    if(instance.findProperty(CIM_NAMESPACE_PROPERTY_SYSTEMCREATIONCLASSNAME)
        == PEG_NOT_FOUND)
    {
        propertyName = CIM_NAMESPACE_PROPERTY_SYSTEMCREATIONCLASSNAME;
    }
    else if(instance.findProperty(CIM_NAMESPACE_PROPERTY_SYSTEMNAME)
        == PEG_NOT_FOUND)
    {
        propertyName = CIM_NAMESPACE_PROPERTY_SYSTEMNAME;
    }
    else if(instance.findProperty(
        CIM_NAMESPACE_PROPERTY_OBJECTMANAGERCREATIONCLASSNAME)
        == PEG_NOT_FOUND)
    {
        propertyName = CIM_NAMESPACE_PROPERTY_OBJECTMANAGERCREATIONCLASSNAME;
    }
    else if(instance.findProperty(
        CIM_NAMESPACE_PROPERTY_OBJECTMANAGERNAME)
        == PEG_NOT_FOUND)
    {
        propertyName = CIM_NAMESPACE_PROPERTY_OBJECTMANAGERNAME;
    }
    else if(instance.findProperty(
        CIM_NAMESPACE_PROPERTY_CREATIONCLASSNAME)
        == PEG_NOT_FOUND)
    {
        propertyName = CIM_NAMESPACE_PROPERTY_CREATIONCLASSNAME;
    }
    else if((propIndex = instance.findProperty(CIM_NAMESPACE_PROPERTY_NAME))
        == PEG_NOT_FOUND)
    {
        propertyName = CIM_NAMESPACE_PROPERTY_NAME;
    }

    if(propIndex == PEG_NOT_FOUND)
    {
        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL1,
            "Invalid CIM_Namespace Key Property %s",
            (const char*)propertyName.getString().getCString()));
        PEG_METHOD_EXIT();
        throw CIMInvalidParameterException(
            "Invalid CIM_Namespace key property: " + propertyName.getString());
    }

    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "CIM_Namespace Keys Valid");

    Array<CIMKeyBinding> keyBindings;

    String newNamespaceName;
    instance.getProperty(propIndex).getValue().get(newNamespaceName);
    keyBindings.append(CIMKeyBinding(
        CIM_NAMESPACE_PROPERTY_NAME,
        newNamespaceName, CIMKeyBinding::STRING));

    keyBindings.append(CIMKeyBinding(
        CIM_NAMESPACE_PROPERTY_CREATIONCLASSNAME,
        PEGASUS_CLASSNAME_PGNAMESPACE.getString(), CIMKeyBinding::STRING));

    keyBindings.append(CIMKeyBinding(
        CIM_NAMESPACE_PROPERTY_SYSTEMCREATIONCLASSNAME,
        System::getSystemCreationClassName(), CIMKeyBinding::STRING));

    String hostName = System::getFullyQualifiedHostName();
    keyBindings.append(CIMKeyBinding(
        CIM_NAMESPACE_PROPERTY_SYSTEMNAME,
        hostName, CIMKeyBinding::STRING));

    keyBindings.append(CIMKeyBinding(
        CIM_NAMESPACE_PROPERTY_OBJECTMANAGERCREATIONCLASSNAME,
        PEGASUS_CLASSNAME_PG_OBJECTMANAGER.getString(),
        CIMKeyBinding::STRING));

    keyBindings.append(CIMKeyBinding(
        CIM_NAMESPACE_PROPERTY_OBJECTMANAGERNAME,
        objectManagerName, CIMKeyBinding::STRING));

    namespacePath = CIMObjectPath(hostName, PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PGNAMESPACE, keyBindings);
    return newNamespaceName;
}

//
// Validates the keys for the ObjectPath representing an instance of
// PG_Namespace. Having this function should be more efficient than creating an
// instance of PG_Namespace and then comparing the object paths.
//
CIMNamespaceName validateNamespaceKeys(
    const CIMObjectPath& objectPath,
    const String & objectManagerName)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "validateNamespaceKeys()");

    CIMName propertyName;
    if(!validateRequiredProperty(
        objectPath,
        CIM_NAMESPACE_PROPERTY_SYSTEMCREATIONCLASSNAME,
        PEGASUS_CLASSNAME_PG_COMPUTERSYSTEM.getString()))
    {
        propertyName = CIM_NAMESPACE_PROPERTY_SYSTEMCREATIONCLASSNAME;
    }
    else if(!validateRequiredProperty(
        objectPath,
        CIM_NAMESPACE_PROPERTY_SYSTEMNAME,
        System::getFullyQualifiedHostName()))
    {
        propertyName = CIM_NAMESPACE_PROPERTY_SYSTEMNAME;
    }
    else if(!validateRequiredProperty(
        objectPath,
        CIM_NAMESPACE_PROPERTY_OBJECTMANAGERCREATIONCLASSNAME,
        PEGASUS_CLASSNAME_PG_OBJECTMANAGER.getString()))
    {
        propertyName = CIM_NAMESPACE_PROPERTY_OBJECTMANAGERCREATIONCLASSNAME;
    }
    else if(!validateRequiredProperty(
        objectPath,
        CIM_NAMESPACE_PROPERTY_OBJECTMANAGERNAME,
        objectManagerName))
    {
        propertyName = CIM_NAMESPACE_PROPERTY_OBJECTMANAGERNAME;
    }
    else if(!validateRequiredProperty(
        objectPath,
        CIM_NAMESPACE_PROPERTY_CREATIONCLASSNAME,
        PEGASUS_CLASSNAME_PGNAMESPACE.getString()))
    {
        propertyName = CIM_NAMESPACE_PROPERTY_CREATIONCLASSNAME;
    }
    else if(!validateRequiredProperty(objectPath,
        CIM_NAMESPACE_PROPERTY_NAME,
        String::EMPTY))
    {
        propertyName = CIM_NAMESPACE_PROPERTY_NAME;
    }

    if(!propertyName.isNull())
    {
        PEG_METHOD_EXIT();
        throw CIMInvalidParameterException(
            "Invalid key property: " + propertyName.getString());
    }

    PEG_METHOD_EXIT();
    return CIMNamespaceName(
        getKeyValue(objectPath, CIM_NAMESPACE_PROPERTY_NAME));
}

void InteropProvider::deleteNamespace(
    const CIMObjectPath & instanceName)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::deleteNamespace()");
    // Validate requred keys and retrieve namespace name. An exception
    // will be thrown if the object path is not valid.
    CIMNamespaceName deleteNamespaceName =
        validateNamespaceKeys(instanceName, objectManagerName);

    if (deleteNamespaceName.equal(PEGASUS_NAMESPACENAME_ROOT))
    {
        PEG_METHOD_EXIT();
        throw CIMNotSupportedException("root namespace cannot be deleted.");
    }

    repository->deleteNameSpace(deleteNamespaceName);

    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "Namespace = %s successfully deleted.",
        (const char*)deleteNamespaceName.getString().getCString()));

    PEG_METHOD_EXIT();
}

CIMObjectPath InteropProvider::createNamespace(
    const CIMInstance & namespaceInstance)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::createNamespace()");
    // The buildNamespacePath method performs some validation on the
    // namespace instance, creates a valid object path that can be used
    // by the client to access the instance later, and retrieves the
    // namespace to be created.
    CIMObjectPath newInstanceReference;
    CIMNamespaceName newNamespaceName = buildNamespacePath(
        newInstanceReference, namespaceInstance, objectManagerName);

    // Create the new namespace

    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "Namespace = %s to be created.",
        (const char*)newNamespaceName.getString().getCString()));

    CIMRepository::NameSpaceAttributes attributes;

    // Set shareable attribute to "false" if property is not present
    if (getPropertyValue(namespaceInstance,
        PG_NAMESPACE_PROPERTY_ISSHAREABLE, false))
    {
        attributes.insert("shareable", "true");
    }
    else
    {
        attributes.insert("shareable", "false");
    }

    // Set updatesAllowed attribute to "false" if property is not present
    if (getPropertyValue(namespaceInstance,
        PG_NAMESPACE_PROPERTY_SCHEMAUPDATESALLOWED, false))
    {
        attributes.insert("updatesAllowed", "true");
    }
    else
    {
        attributes.insert("updatesAllowed", "false");
    }

    // Set the parent attribute if the property is present, but don't set
    // it at all otherwise.
    String parent = getPropertyValue(namespaceInstance,
        PG_NAMESPACE_PROPERTY_PARENTNAMESPACE, String::EMPTY);
    if (parent != String::EMPTY)
        attributes.insert("parent",parent);

    //
    // Create the namespace with the retrieved attributes.
    //
    repository->createNameSpace(newNamespaceName, attributes);

    PEG_TRACE((
        TRC_CONTROLPROVIDER,
        Tracer::LEVEL4,
        "Namespace %s: Parent: %s"
            "  successfully created.",
        (const char*) newNamespaceName.getString().getCString(),
        (const char*) parent.getCString()));

    return newInstanceReference;
}

PEGASUS_NAMESPACE_END

// END OF FILE
