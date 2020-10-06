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
//%////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//  Namespace Provider
//
//      This provider answers to the "false" class __namespace.  This is the
//      deprecated version of manipulation in the DMTF WBEM model.  The function
//      is defined in the CIM Operations over HTTP docuement.  However, while
//      the function exists, no class was ever defined in the CIM model for
//      __nemaspace.  Therefore each implementation is free to provide its own
//      class definition.
///////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <cctype>
#include <iostream>

#include "NamespaceProvider.h"
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMStatusCode.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Config/ConfigManager.h>

#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Common/ResponseHandler.h>
#include <Pegasus/Common/MessageLoader.h> //l10n

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/**
 * Specification for CIM Operations over HTTP
 *
 * Version 1.0
 *
 * 2.5. Namespace Manipulation
 * There are no intrinsic methods defined specifically for the
 * purpose of manipulating CIM Namespaces. However, the modelling
 * of a CIM Namespace using the class __Namespace, together with
 * the requirement that the root Namespace MUST be supported by
 * all CIM Servers, implies that all Namespace operations can be
 * supported.
 *
 * For example:
 *
 * Enumeration of all child Namespaces of a particular Namespace
 * is realized by calling the intrinsic method
 * EnumerateInstanceNames against the parent Namespace,
 * specifying a value for the ClassName parameter of __Namespace.
 *
 * Creation of a child Namespace is realized by calling the
 * intrinsic method CreateInstance against the parent Namespace,
 * specifying a value for the NewInstance parameter which defines
 * a valid instance of the class __Namespace and whose Name
 * property is the desired name of the new Namespace.
 *
*/

/**
    The constant representing the __namespace class name
*/
static const CIMName NAMESPACE_CLASSNAME = CIMNameCast("__Namespace");
static const CIMName NAMESPACE_PROPERTYNAME = CIMNameCast("Name");
static const CIMNamespaceName ROOTNS  = CIMNamespaceName ("root");

static Boolean _isChild(
    const CIMNamespaceName& parentNamespaceName,
    const CIMNamespaceName& namespaceName)
{
    String parent = parentNamespaceName.getString();
    String child = namespaceName.getString();

    return (child.size() > parent.size()) &&
        String::equalNoCase(child.subString(0, parent.size()), parent) &&
        (child[parent.size()] == '/');
}

static void _getKeyValue (
    const CIMInstance& namespaceInstance,
    CIMNamespaceName& childNamespaceName,
    Boolean& isRelativeName)
{
    //Validate key property

    Uint32 pos;
    CIMValue propertyValue;

    // [Key, MaxLen (256), Description (
    //       "A string that uniquely identifies the Namespace "
    //       "within the ObjectManager.") ]
    // string Name;

    pos = namespaceInstance.findProperty(NAMESPACE_PROPERTYNAME);
    if (pos == PEG_NOT_FOUND)
    {
       throw CIMPropertyNotFoundException
           (NAMESPACE_PROPERTYNAME.getString());
    }

    propertyValue = namespaceInstance.getProperty(pos).getValue();
    if (propertyValue.getType() != CIMTYPE_STRING)
    {
        throw CIMInvalidParameterException(MessageLoaderParms(
            "ControlProviders.NamespaceProvider.NamespaceProvider."
                "INVALID_TYPE_FOR_PROPERTY",
            "Invalid type for property: $0",
            NAMESPACE_PROPERTYNAME.getString()));
    }

    String cnsName;
    propertyValue.get(cnsName);
    if (cnsName == String::EMPTY)
    {
       childNamespaceName = CIMNamespaceName();
    }
    else
    {
       childNamespaceName = CIMNamespaceName(cnsName);
    }

    isRelativeName = !(childNamespaceName.isNull());
}

static void _getKeyValue (
    const CIMObjectPath&  instanceName,
    CIMNamespaceName& childNamespaceName,
    Boolean& isRelativeName)
{

    Array<CIMKeyBinding> kbArray = instanceName.getKeyBindings();
    if ((kbArray.size() == 1) &&
            (kbArray[0].getName() == NAMESPACE_PROPERTYNAME))
    {
       String childNamespaceString = kbArray[0].getValue();

       if (childNamespaceString != String::EMPTY)
       {
           childNamespaceName = childNamespaceString;
       }

       isRelativeName = !(childNamespaceName.isNull());
    }
    else
    {
        //l10n
        //throw CIMInvalidParameterException("Invalid key property:  ");
        throw CIMInvalidParameterException(MessageLoaderParms(
            "ControlProviders.NamespaceProvider.NamespaceProvider."
                "INVALID_KEY_PROPERTY",
            "Invalid key property:  "));
    }
}

static void _generateFullNamespaceName(
    Array<CIMNamespaceName>& namespaceNames,
    CIMNamespaceName& parentNamespaceName,
    CIMNamespaceName& childNamespaceName,
    Boolean isRelativeName,
    CIMNamespaceName& fullNamespaceName)
{
   // If isRelativeName is true, then the parentNamespace
   // MUST exist
   //
   if (isRelativeName)
   {
        if (!Contains(namespaceNames, parentNamespaceName))
        {
            throw CIMObjectNotFoundException(MessageLoaderParms(
                "ControlProviders.NamespaceProvider.NamespaceProvider."
                    "PARENT_NAMESPACE_DOES_NOT_EXIST",
                "Parent namespace does not exist: $0",
                parentNamespaceName.getString()));
        }
        // Create full namespace name by prepending parentNamespaceName
        fullNamespaceName = CIMNamespaceName (parentNamespaceName.getString()
            + "/" + childNamespaceName.getString());
    }
    else
    {
      fullNamespaceName = parentNamespaceName;
    }
}

NamespaceProvider::NamespaceProvider(CIMRepository* repository)
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER,"NamespaceProvider::NamespaceProvider");
    _repository = repository;
    PEG_METHOD_EXIT();
}

NamespaceProvider::~NamespaceProvider()
{
    PEG_METHOD_ENTER(TRC_USER_MANAGER,"NamespaceProvider::~NamespaceProvider");

    PEG_METHOD_EXIT();
}

void NamespaceProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance& modifiedIns,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler & handler)
{
    throw PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, "");
}

void NamespaceProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance& myInstance,
    ObjectPathResponseHandler & handler)
    {
        PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
                         "NamespaceProvider::createInstance()");

        CIMNamespaceName childNamespaceName;
            CIMNamespaceName newNamespaceName;
        Boolean isRelativeName;

       // Verify that the className = __namespace
       if (!myInstance.getClassName().equal(NAMESPACE_CLASSNAME))
       {
            PEG_METHOD_EXIT();
            throw CIMNotSupportedException(MessageLoaderParms(
                "ControlProviders.NamespaceProvider.NamespaceProvider."
                    "NOT_SUPPORTED_BY_NAMESPACEPROVIDER",
                "$0 not supported by Namespace Provider",
                myInstance.getClassName().getString()));

       }

       //ATTN-DME-P3-20020522: ADD AUTHORIZATION CHECK TO __NAMESPACE PROVIDER
       String userName;
       try
       {
           IdentityContainer container = context.get(IdentityContainer::NAME);
           userName = container.getUserName();
       }
       catch (...)
       {
           userName = String::EMPTY;
       }

       _getKeyValue(myInstance, childNamespaceName, isRelativeName);
        CIMNamespaceName parentNamespaceName = instanceReference.getNameSpace();

        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
               "childNamespaceName = %s, isRelativeName = %s, "
               "parentNamespaceName = %s",
               (const char*)childNamespaceName.getString().getCString(),
               (isRelativeName?"true":"false"),
               (const char*)parentNamespaceName.getString().getCString()));

       // begin processing the request
       handler.processing();

       Array<CIMNamespaceName> namespaceNames;
       namespaceNames = _repository->enumerateNameSpaces();

       _generateFullNamespaceName(
           namespaceNames, parentNamespaceName,
           childNamespaceName, isRelativeName,
           newNamespaceName);

       _repository->createNameSpace(newNamespaceName);

       PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
           "Namespace = %s successfully created.",
           (const char*)newNamespaceName.getString().getCString()));

       // return key (i.e., CIMObjectPath) for newly created namespace

       Array<CIMKeyBinding> keyBindings;
       keyBindings.append(CIMKeyBinding(NAMESPACE_PROPERTYNAME,
                 isRelativeName?childNamespaceName.getString():
                                parentNamespaceName.getString(),
                                     CIMKeyBinding::STRING));
       CIMObjectPath newInstanceReference (String::EMPTY, parentNamespaceName,
                                     NAMESPACE_CLASSNAME, keyBindings);
       handler.deliver(newInstanceReference);

       // complete processing the request
       handler.complete();

       PEG_METHOD_EXIT();
       return;
   }

void NamespaceProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceName,
    ResponseHandler & handler)
    {
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER, "NamespaceProvider::deleteInstance");

        CIMNamespaceName childNamespaceName;
        CIMNamespaceName deleteNamespaceName;
        Boolean isRelativeName;

       // Verify that the className = __namespace
       if (!instanceName.getClassName().equal(NAMESPACE_CLASSNAME))
       {
            PEG_METHOD_EXIT();
            throw CIMNotSupportedException(MessageLoaderParms(
                "ControlProviders.NamespaceProvider.NamespaceProvider."
                    "NOT_SUPPORTED_BY_NAMESPACEPROVIDER",
                "$0 not supported by Namespace Provider",
                instanceName.getClassName().getString()));
       }

       //ATTN-DME-P3-20020522: ADD AUTHORIZATION CHECK TO __NAMESPACE PROVIDER
       String userName;
       try
       {
           IdentityContainer container = context.get(IdentityContainer::NAME);
           userName = container.getUserName();
       }
       catch (...)
       {
           userName = String::EMPTY;
       }

       _getKeyValue(instanceName, childNamespaceName, isRelativeName);
       CIMNamespaceName parentNamespaceName = instanceName.getNameSpace();

       PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
              "childNamespaceName = %s, isRelativeName = %s, "
              "parentNamespaceName = %s",
              (const char*)childNamespaceName.getString().getCString(),
              (isRelativeName?"true":"false"),
              (const char*)parentNamespaceName.getString().getCString()));

       // begin processing the request
       handler.processing();

       Array<CIMNamespaceName> namespaceNames;
       namespaceNames = _repository->enumerateNameSpaces();

       _generateFullNamespaceName(
           namespaceNames, parentNamespaceName,
           childNamespaceName, isRelativeName,
           deleteNamespaceName);

       if (deleteNamespaceName.equal(ROOTNS))
       {
           throw CIMNotSupportedException(MessageLoaderParms(
               "ControlProviders.NamespaceProvider.NamespaceProvider."
                   "ROOT_NAMESPACE_CANNOT_BE_DELETED",
               "root namespace may be deleted."));
       }

       _repository->deleteNameSpace(deleteNamespaceName);

       PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
           "Namespace = %s successfully deleted.",
           (const char*)deleteNamespaceName.getString().getCString()));

       // complete processing the request
       handler.complete();

       PEG_METHOD_EXIT();
       return ;
    }

void NamespaceProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceName,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & properatyList,
    InstanceResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER, "NamespaceProvider::getInstance");

        CIMNamespaceName childNamespaceName;
        CIMNamespaceName getNamespaceName;
        Boolean isRelativeName;

       // Verify that the className = __namespace
       if (!instanceName.getClassName().equal(NAMESPACE_CLASSNAME))
       {
            PEG_METHOD_EXIT();
            throw CIMNotSupportedException(MessageLoaderParms(
                "ControlProviders.NamespaceProvider.NamespaceProvider."
                    "NOT_SUPPORTED_BY_NAMESPACEPROVIDER",
                "$0 not supported by Namespace Provider",
                instanceName.getClassName().getString()));
       }

       //ATTN-DME-P3-20020522: ADD AUTHORIZATION CHECK TO __NAMESPACE PROVIDER
       String userName;
       try
       {
           IdentityContainer container = context.get(IdentityContainer::NAME);
           userName = container.getUserName();
       }
       catch (...)
       {
           userName = String::EMPTY;
       }

       _getKeyValue(instanceName, childNamespaceName, isRelativeName);
       CIMNamespaceName parentNamespaceName = instanceName.getNameSpace();

       PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
              "childNamespaceName = %s, isRelativeName = %s, "
              "parentNamespaceName = %s",
              (const char*)childNamespaceName.getString().getCString(),
              (isRelativeName?"true":"false"),
              (const char*)parentNamespaceName.getString().getCString()));

       // begin processing the request
       handler.processing();

       Array<CIMNamespaceName> namespaceNames;
       namespaceNames = _repository->enumerateNameSpaces();

       _generateFullNamespaceName(
           namespaceNames, parentNamespaceName,
           childNamespaceName, isRelativeName,
           getNamespaceName);

       if (!Contains(namespaceNames, getNamespaceName))
       {
           throw CIMObjectNotFoundException(MessageLoaderParms(
               "ControlProviders.NamespaceProvider.NamespaceProvider"
                   ".NAMESPACE_DOES_NOT_EXIST",
               "Namespace does not exist: $0",
               getNamespaceName.getString()));
       }

       PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
           "Namespace = %s successfully found.",
           (const char*)getNamespaceName.getString().getCString()));

       //Set name of class
       CIMInstance instance(NAMESPACE_CLASSNAME);

       //
       // construct the instance
       //
       instance.addProperty(CIMProperty(NAMESPACE_PROPERTYNAME,
           isRelativeName?childNamespaceName.getString():
                          parentNamespaceName.getString()));
       //instance.setPath(instanceName);

       handler.deliver(instance);

       // complete processing the request
       handler.complete();

       PEG_METHOD_EXIT();
       return ;
}

void NamespaceProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler & handler)
{
       PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
                        "NamespaceProvider::enumerateInstances()");

        // Verify that ClassName == __Namespace
       if (!ref.getClassName().equal(NAMESPACE_CLASSNAME))
       {
            PEG_METHOD_EXIT();

            throw CIMNotSupportedException(MessageLoaderParms(
                "ControlProviders.NamespaceProvider.NamespaceProvider."
                    "NOT_SUPPORTED_BY_NAMESPACEPROVIDER",
                "$0 not supported by Namespace Provider",
                ref.getClassName().getString()));
       }

       //ATTN-DME-P3-20020522: ADD AUTHORIZATION CHECK TO __NAMESPACE PROVIDER
       String userName;
       try
       {
           IdentityContainer container = context.get(IdentityContainer::NAME);
           userName = container.getUserName();
       }
       catch (...)
       {
          userName = String::EMPTY;
       }

       CIMNamespaceName parentNamespaceName = ref.getNameSpace();

       PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
               "parentNamespaceName = %s",
                (const char*)parentNamespaceName.getString().getCString()));

       // begin processing the request
       handler.processing();

       Array<CIMInstance> instanceArray;

       Array<CIMNamespaceName> namespaceNames =
           _repository->enumerateNameSpaces();

       Boolean enumerateAllNamespaces =
           (parentNamespaceName == PEGASUS_VIRTUAL_TOPLEVEL_NAMESPACE);

       // Build the instances. For now simply build the __Namespace instances
       // Note that for the moment, the only property is name.
       for (Uint32 i = 0; i < namespaceNames.size(); i++)
       {
           if (enumerateAllNamespaces ||
               _isChild(parentNamespaceName, namespaceNames[i]))
           {
               String nsName = enumerateAllNamespaces ?
                   namespaceNames[i].getString() :
                   namespaceNames[i].getString().subString(
                       parentNamespaceName.getString().size() + 1);
               CIMInstance instance(NAMESPACE_CLASSNAME);
               instance.addProperty(CIMProperty(
                   NAMESPACE_PROPERTYNAME, nsName));

               // Set the instance name
               Array<CIMKeyBinding> keyBindings;
               keyBindings.append(CIMKeyBinding(
                   NAMESPACE_PROPERTYNAME, nsName, CIMKeyBinding::STRING));
               CIMObjectPath instanceName(
                   String::EMPTY,
                   parentNamespaceName,
                   NAMESPACE_CLASSNAME,
                   keyBindings);
               instance.setPath(instanceName);

               instanceArray.append(instance);
               PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                   "childNamespace = %s",
                   (const char*)namespaceNames[i].getString().getCString()));
           }
       }

       handler.deliver(instanceArray);

       // complete processing the request
       handler.complete();

       PEG_METHOD_EXIT();
}

void NamespaceProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
    {
        PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
            "NamespaceProvider::enumerateInstanceNames()");

        // Verify that ClassName == __Namespace
        if (!classReference.getClassName().equal(NAMESPACE_CLASSNAME))
        {
            PEG_METHOD_EXIT();

            throw CIMNotSupportedException(MessageLoaderParms(
                "ControlProviders.NamespaceProvider.NamespaceProvider."
                    "NOT_SUPPORTED_BY_NAMESPACEPROVIDER",
                "$0 not supported by Namespace Provider",
                classReference.getClassName().getString()));
        }

        //ATTN-DME-P3-20020522: ADD AUTHORIZATION CHECK TO __NAMESPACE PROVIDER
        String userName;
        try
        {
            IdentityContainer container = context.get(IdentityContainer::NAME);
            userName = container.getUserName();
        }
        catch (...)
        {
           userName = String::EMPTY;
        }

        CIMNamespaceName parentNamespaceName = classReference.getNameSpace();
        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
            "parentNamespaceName = %s",
            (const char*)parentNamespaceName.getString().getCString()));

        Array<CIMObjectPath> instanceRefs;

        Array<CIMNamespaceName> namespaceNames =
            _repository->enumerateNameSpaces();

        Boolean enumerateAllNamespaces =
            (parentNamespaceName == PEGASUS_VIRTUAL_TOPLEVEL_NAMESPACE);

        // Build the instances. Simply build the __Namespace instances
        // Note, the only property is name.
        for (Uint32 i = 0; i < namespaceNames.size(); i++)
        {
            if (enumerateAllNamespaces ||
                _isChild(parentNamespaceName, namespaceNames[i]))
            {
                String nsName = enumerateAllNamespaces ?
                    namespaceNames[i].getString() :
                    namespaceNames[i].getString().subString(
                        parentNamespaceName.getString().size() + 1);
                Array<CIMKeyBinding> keyBindings;
                keyBindings.append(CIMKeyBinding(
                    NAMESPACE_PROPERTYNAME, nsName, CIMKeyBinding::STRING));
                CIMObjectPath ref(
                    String::EMPTY,
                    parentNamespaceName,
                    NAMESPACE_CLASSNAME,
                    keyBindings);
                instanceRefs.append(ref);
                PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                    "childNamespace = %s",
                     (const char*)namespaceNames[i].getString().getCString()));
            }
        }

        handler.deliver(instanceRefs);

        handler.complete();

        PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
