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
//%////////////////////////////////////////////////////////////////////////////

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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <cctype>
#include <iostream>

#include "InteropProvider.h"
#include "InteropProviderUtils.h"
#include "InteropConstants.h"

#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/ArrayIterator.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

/*****************************************************************************
 *
 * The following are constants representing property names for the classes
 * managed by the Interop Provider. Where multiple classes have properties of
 * the same name, there will be a common CIMName object defined, and a macro
 * defined that points to the common CIMName object, but whose macro name
 * reflects the class in which the property is used.
 *
 *****************************************************************************/

//
// Constructor for the InteropProvider control provider
//
InteropProvider::InteropProvider(
    CIMRepository * rep,
    ProviderRegistrationManager *provRegManager):
        repository(rep),
        providerRegistrationManager(provRegManager),
        hostName(System::getHostName()),
        providerInitialized(false),
        updateProfileCache(0),
        profileIds(Array<String>()),
        conformingElements(Array<CIMNameArray>()),
        elementNamespaces(Array<CIMNamespaceArray>())
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,"InteropProvider::InteropProvider");

    ConfigManager *configManager = ConfigManager::getInstance();
#ifdef PEGASUS_ENABLE_SLP
    enableSLP = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("slp"));
#else
    enableSLP = false;
#endif

    httpPort = configManager->getCurrentValue("httpPort");
    if (httpPort.size() == 0)
    {
        Uint32 portNumberHttp = System::lookupPort(
            WBEM_HTTP_SERVICE_NAME, WBEM_DEFAULT_HTTP_PORT);
        char buffer[32];
        Uint32 n;
        const char *output = Uint32ToString(buffer, portNumberHttp, n);
        httpPort.assign(output, n);
    }

    httpsPort = configManager->getCurrentValue("httpsPort");
    if (httpsPort.size() == 0)
    {
        Uint32 portNumberHttps = System::lookupPort(
            WBEM_HTTPS_SERVICE_NAME, WBEM_DEFAULT_HTTPS_PORT);
        char buffer[32];
        Uint32 n;
        const char *output = Uint32ToString(buffer, portNumberHttps, n);
        httpsPort.assign(output, n);
    }


#ifndef PEGASUS_DISABLE_PERFINST
    try
    {
        initProvider();
    }
    catch(const Exception &)
    {
        // Provider initialization may fail if the repository is not
        // populated
    }
#endif

    PEG_METHOD_EXIT();
}

//
// Local version of getInstance to be used by other functions in the the
// provider. Returns a single instance. Note that it always returns an
// instance. If none was found, it is uninialitized.
//
CIMInstance InteropProvider::localGetInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceName,
    const CIMPropertyList & propertyList)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER, "InteropProvider::localGetInstance");

    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "%s getInstance. instanceName= %s , PropertyList= %s",
        thisProvider,
        (const char *)instanceName.toString().getCString(),
        (const char *)propertyList.toString().getCString()));

    // Test if we're looking for something outside of our namespace. This will
    // happen during associators calls from PG_RegisteredProfile instances
    // through the PG_ElementConformsToProfile association
    CIMNamespaceName opNamespace = instanceName.getNameSpace();
    CIMName opClass = instanceName.getClassName();
    if((opNamespace != PEGASUS_NAMESPACENAME_INTEROP &&
        opClass != PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE)
        // Get CIM_IndicationService instance from IndicationService.
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
        || opClass == PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE
#endif
        )
    {
        AutoMutex mut(interopMut);
        CIMInstance gotInstance = cimomHandle.getInstance(
                                         context,
                                         opNamespace,
                                         instanceName,
                                         false,
                                         false,
                                         false,
                                         propertyList);
        PEG_METHOD_EXIT();
        return gotInstance;
    }

    TARGET_CLASS classEnum  = translateClassInput(opClass);
    CIMInstance retInstance;
    switch(classEnum)
    {
        case PG_SOFTWAREIDENTITY:
        {
            retInstance = getSoftwareIdentityInstance(instanceName);
            normalizeInstance(
                retInstance, instanceName, false, false, propertyList);
        }
        break;
        case PG_NAMESPACE:
        {
            retInstance = getNameSpaceInstance(instanceName);
            normalizeInstance(
                retInstance, instanceName, false, false, propertyList);
        }
        break;
        // ATTN: Implement getIntstance for all other classes. Currently
        // this method calls localEnumerateInstances() to select instance
        // which is too expensive.
        default:
        {
            // Create reference from host, namespace, class components of
            // instance name
            CIMObjectPath ref;
            ref.setHost(instanceName.getHost());
            ref.setClassName(opClass);
            ref.setNameSpace(opNamespace);

            // Enumerate instances for this class. Returns all instances
            // Note that this returns paths setup and instances already
            // filtered per the input criteria.
            Array<CIMInstance> instances =  localEnumerateInstances(
                context,
                ref,
                propertyList);
            ConstArrayIterator<CIMInstance> instancesIter(instances);

            // deliver a single instance if found.
            bool found = false;
            for(Uint32 i = 0; i < instancesIter.size(); i++)
            {
                CIMObjectPath currentInstRef = instancesIter[i].getPath();
                currentInstRef.setHost(instanceName.getHost());
                currentInstRef.setNameSpace(instanceName.getNameSpace());
                if(instanceName == currentInstRef)
                {
                    retInstance = instancesIter[i];
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                PEG_METHOD_EXIT();
                throw CIMObjectNotFoundException(instanceName.toString());
            }
        }
    }

    PEG_METHOD_EXIT();
    return retInstance;
}

Array<CIMInstance> InteropProvider::getReferencedInstances(
    const Array<CIMInstance> &refs,
    const String &targetRole,
    const OperationContext & context,
    const CIMPropertyList & propertyList)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::getReferencedObjects");

    Array<CIMInstance> referencedInstances;
    Array<CIMInstance> classInstances;
    CIMName prevClassName;

    ConstArrayIterator<CIMInstance> refsIter(refs);
    for(Uint32 i = 0; i < refsIter.size(); i++)
    {
        CIMInstance thisRef = refsIter[i];
        CIMObjectPath thisTarget = getRequiredValue<CIMObjectPath>(
            thisRef,
            targetRole);

        // Test if we're looking for something outside of our namespace. This
        // will happen during associators calls from PG_RegisteredProfile
        // instances through the PG_ElementConformsToProfile association
        CIMNamespaceName opNamespace = thisTarget.getNameSpace();
        CIMName opClass = thisTarget.getClassName();

        if((opNamespace != PEGASUS_NAMESPACENAME_INTEROP &&
            opClass != PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE)
            // Get CIM_IndicationService instance from IndicationService.
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
            || opClass == PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE
#endif
            )
        {
            AutoMutex mut(interopMut);
            CIMInstance gotInstance = cimomHandle.getInstance(
                context,
                opNamespace,
                thisTarget,
                false,
                false,
                false,
                propertyList);
            referencedInstances.append(gotInstance);
            continue;
        }

        TARGET_CLASS classEnum  = translateClassInput(opClass);
        CIMInstance retInstance;
        switch(classEnum)
        {
            case PG_SOFTWAREIDENTITY:
            {
                CIMInstance retInstance =
                    getSoftwareIdentityInstance(thisTarget);
                normalizeInstance(
                    retInstance, thisTarget, false, false, propertyList);
                retInstance.setPath(thisTarget);
                referencedInstances.append(retInstance);
            }
            break;
            case PG_NAMESPACE:
            {
                CIMInstance retInstance = getNameSpaceInstance(thisTarget);
                normalizeInstance(
                    retInstance, thisTarget, false, false, propertyList);
                retInstance.setPath(thisTarget);
                referencedInstances.append(retInstance);
            }
            break;
            default:
            {
                if( opClass != prevClassName )
                {
                    CIMObjectPath ref;
                    ref.setHost(thisTarget.getHost());
                    ref.setClassName(thisTarget.getClassName());
                    ref.setNameSpace(thisTarget.getNameSpace());
                    classInstances = localEnumerateInstances(
                        context,
                        ref,
                        propertyList);
                    ArrayIterator<CIMInstance> instsIter(classInstances);
                    for(Uint32 n = 0; n < instsIter.size(); n++)
                    {
                        CIMObjectPath tmpInst = instsIter[n].getPath();
                        tmpInst.setHost(thisTarget.getHost());
                        tmpInst.setNameSpace(thisTarget.getNameSpace());
                        instsIter[n].setPath(tmpInst);
                    }
                    prevClassName = opClass;
                }
                ConstArrayIterator<CIMInstance> instsConstIter(classInstances);
                for(Uint32 j = 0; j < instsConstIter.size(); j++)
                {
                    if(thisTarget == instsConstIter[j].getPath())
                    {
                        referencedInstances.append(instsConstIter[j]);
                        break;
                    }
                }
            }
            break;
        }
    }
    PEG_METHOD_EXIT();
    return referencedInstances;
}

//
// Local version of enumerateInstances to be used by other functions in the
// provider. Note that this delivers instances as a group rather than one
// at a time. This design point may need to be revisited if this provider
// is used in environments such that returning segmented responses would have
// significant performance advantages. For now, that doesn't seem to be the
// case.
//
Array<CIMInstance> InteropProvider::localEnumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & ref,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::localEnumerateInstances()");

    const CIMName & className = ref.getClassName();
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "%s enumerateInstances. referenc= %s , PropertyList= %s",
        thisProvider,
        (const char *)className.getString().getCString(),
        (const char *)propertyList.toString().getCString()));

    // Verify that ClassName is correct and get its enum value
    TARGET_CLASS classEnum = translateClassInput(className);

    Array<CIMInstance> instances;
    switch(classEnum)
    {
        case PG_OBJECTMANAGER:
        {
            instances.append(getObjectManagerInstance());
            break;
        }
        case PG_CIMXMLCOMMUNICATIONMECHANISM:
        {
            instances = enumCIMXMLCommunicationMechanismInstances();
            break;
        }
        case PG_NAMESPACEINMANAGER:
        {
            instances = enumNamespaceInManagerInstances();
            break;
        }
        case PG_COMMMECHANISMFORMANAGER:
        {
            instances = enumCommMechanismForManagerInstances();
            break;
        }
        case PG_NAMESPACE:
        {
            instances = enumNamespaceInstances();
            break;
        }
        case PG_REGISTEREDPROFILE:
        {
            instances = enumRegisteredProfileInstances();
            break;
        }
        case PG_REGISTEREDSUBPROFILE:
        {
            instances = enumRegisteredSubProfileInstances();
            break;
        }
        case PG_REFERENCEDPROFILE:
        {
            instances = enumReferencedProfileInstances();
            break;
        }
        case PG_ELEMENTCONFORMSTOPROFILE:
        {
            instances = enumElementConformsToProfileInstances(context,
                ref.getNameSpace());
            break;
        }
        case PG_ELEMENTCONFORMSTOPROFILE_RP_RP:
        {
            instances = enumElementConformsToProfileRPRPInstances(
                context,
                ref.getNameSpace());
            break;
        }
        case PG_SUBPROFILEREQUIRESPROFILE:
        {
            instances = enumSubProfileRequiresProfileInstances();
            break;
        }
        case PG_SOFTWAREIDENTITY:
        {
            instances = enumSoftwareIdentityInstances();
            break;
        }
        case PG_ELEMENTSOFTWAREIDENTITY:
        {
            instances = enumElementSoftwareIdentityInstances();
            break;
        }
        case PG_INSTALLEDSOFTWAREIDENTITY:
        {
            instances = enumInstalledSoftwareIdentityInstances(context);
            break;
        }
        case PG_COMPUTERSYSTEM:
        {
            instances.append(getComputerSystemInstance(context));
            break;
        }
        case PG_HOSTEDOBJECTMANAGER:
        {
            instances.append(getHostedObjectManagerInstance(context));
            break;
        }
        case PG_HOSTEDACCESSPOINT:
        {
            instances = enumHostedAccessPointInstances(context);
            break;
        }
        //We don't support enumerate CIM_Namespace instances. PG_Namespace is
        //supported.
        case CIM_NAMESPACE:
        {
            break;
        }
        case PG_PROVIDERPROFILECAPABILITIES:
        {
            instances = enumProviderProfileCapabilityInstances(false);
            break;
        }

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
        case PG_ELEMENTCAPABILITIES:
        {
            instances = enumElementCapabilityInstances(context);
            break;
        }
        case PG_HOSTEDINDICATIONSERVICE:
        {
            instances = enumHostedIndicationServiceInstances(context);
            break;
        }
        case PG_SERVICEAFFECTSELEMENT:
        {
            instances = enumServiceAffectsElementInstances(context);
            break;
        }
        case CIM_INDICATIONSERVICE:
        {
            instances = enumIndicationServiceInstances(context);
            break;
        }
#endif
        default:
            PEG_METHOD_EXIT();
            throw CIMNotSupportedException(className.getString() +
              " not supported by Interop Provider enumerate");
    }

    // Filter and deliver the resulting instances
    for (Uint32 i = 0 ; i < instances.size() ; i++)
    {
        normalizeInstance(instances[i], ref, false,
            false, propertyList);
    }

    PEG_METHOD_EXIT();
    return instances;
}

//
// Class that determines whether or not the origin class in an association
// operation is valid for the given association class, and also determines
// the origin and target "roles". These values generally correspond to the
// role and resultRole parameter of an associators/associatorNames operation.
//
bool InteropProvider::validAssocClassForObject(
    const OperationContext & context,
    const CIMName & assocClass,
    const CIMObjectPath & objectName,
    const CIMNamespaceName & opNamespace,
    String & originProperty,
    String & targetProperty)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::validAssocClassForObject()");
    TARGET_CLASS assocClassEnum = translateClassInput(assocClass);
    TARGET_CLASS originClassEnum = NOCLASS;
    CIMName originClass = objectName.getClassName();
    // If the association class is PG_ElementConformsToProfile, we'll have to
    // do some special processing in case the origin instance for the operation
    // is managed by another provider.
    if(assocClassEnum == PG_ELEMENTCONFORMSTOPROFILE)
    {
        // Test if the origin is an element managed by another provider
        // that has implemented a registered profile.
        if(opNamespace != PEGASUS_NAMESPACENAME_INTEROP ||
            (originClass != PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE &&
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
             originClass != PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE &&
#endif
             originClass != PEGASUS_CLASSNAME_PG_OBJECTMANAGER ))
        {
            //
            // Search the cached conformingElements list for the originClass,
            // returning false if it is not found
            //
            bool found = false;

            PEGASUS_ASSERT(conformingElements.size() ==
                elementNamespaces.size());
            for(Uint32 i = 0, n = conformingElements.size(); i < n; ++i)
            {
                CIMNameArray & elementList = conformingElements[i];
                CIMNamespaceArray & namespaceList = elementNamespaces[i];
                PEGASUS_ASSERT(elementList.size() == namespaceList.size());
                for(Uint32 j = 0, m = elementList.size(); j < m; ++j)
                {
                    CIMName & curElement = elementList[j];
                    if((curElement == originClass ||
                      curElement.getString().find(PEGASUS_DYNAMIC) == 0) &&
                      opNamespace == namespaceList[j])
                    {
                        found = true;
                        break;
                    }
                }
                if(found)
                    break;
            }

            if(!found)
            {
                PEG_METHOD_EXIT();
                return false;
            }
        }
    }
    else
    {
        // Otherwise, just get the enum value representing the origin class
        // for this operation
        originClassEnum = translateClassInput(originClass);
    }

    CIMName expectedTargetRole;
    CIMName expectedOriginRole;

    Array<CIMName> propNames;
    String profileName;
    CIMPropertyList propertyList;
    CIMInstance tmpInstance;
    Uint32 index;
    propNames.clear();

    //
    // Set the target and origin role values. Note that if these values are
    // not set following the switch block, that implies that the origin class
    // is not valid for the supplied association class.
    //
    switch(assocClassEnum)
    {
      case PG_NAMESPACEINMANAGER:
          if(originClassEnum == PG_OBJECTMANAGER)
          {
              expectedTargetRole = PROPERTY_DEPENDENT;
              expectedOriginRole = PROPERTY_ANTECEDENT;
          }
          else if(originClassEnum == PG_NAMESPACE)
          {
              expectedTargetRole = PROPERTY_ANTECEDENT;
              expectedOriginRole = PROPERTY_DEPENDENT;
          }
          break;
      case PG_COMMMECHANISMFORMANAGER:
          if(originClassEnum == PG_OBJECTMANAGER)
          {
              expectedTargetRole = PROPERTY_DEPENDENT;
              expectedOriginRole = PROPERTY_ANTECEDENT;
          }
          else if(originClassEnum == PG_CIMXMLCOMMUNICATIONMECHANISM)
          {
              expectedTargetRole = PROPERTY_ANTECEDENT;
              expectedOriginRole = PROPERTY_DEPENDENT;
          }
          break;
      case PG_ELEMENTCONFORMSTOPROFILE:
          if(originClass.equal(PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE))
          {
              expectedTargetRole =
                  ELEMENTCONFORMSTOPROFILE_PROPERTY_MANAGEDELEMENT;
              expectedOriginRole =
                  ELEMENTCONFORMSTOPROFILE_PROPERTY_CONFORMANTSTANDARD;
          }
          else
          {
              expectedTargetRole =
                  ELEMENTCONFORMSTOPROFILE_PROPERTY_CONFORMANTSTANDARD;
              expectedOriginRole =
                  ELEMENTCONFORMSTOPROFILE_PROPERTY_MANAGEDELEMENT;
          }
          break;
      case PG_ELEMENTCONFORMSTOPROFILE_RP_RP:
          propNames.append(CIMName("RegisteredName"));
          propertyList = CIMPropertyList(propNames);
          try
          {
              tmpInstance = localGetInstance(
                  context,
                  objectName,
                  propertyList);
          }
          catch (CIMException &e)
          {
              PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL2,
                  "CIMException while getting instance of Registered Profile "
                      ": %s",
                  (const char*)e.getMessage().getCString()));
          }
          if (!tmpInstance.isUninitialized())
          {
              index = tmpInstance.findProperty("RegisteredName");
              if (index != PEG_NOT_FOUND)
              {
                  const CIMValue &tmpVal =
                      tmpInstance.getProperty(index).getValue();
                  if (!tmpVal.isNull())
                  {
                      tmpVal.get(profileName);
                  }
              }
          }
          if (String::compareNoCase(profileName, String("SMI-S")) == 0)
          {
              expectedTargetRole =
                  ELEMENTCONFORMSTOPROFILE_PROPERTY_MANAGEDELEMENT;
              expectedOriginRole =
                  ELEMENTCONFORMSTOPROFILE_PROPERTY_CONFORMANTSTANDARD;
          }
          else
          {
              expectedTargetRole =
                  ELEMENTCONFORMSTOPROFILE_PROPERTY_CONFORMANTSTANDARD;
              expectedOriginRole =
                  ELEMENTCONFORMSTOPROFILE_PROPERTY_MANAGEDELEMENT;
          }
          break;
      case PG_SUBPROFILEREQUIRESPROFILE:
          if(originClassEnum == PG_REGISTEREDPROFILE)
          {
              expectedTargetRole = PROPERTY_DEPENDENT;
              expectedOriginRole = PROPERTY_ANTECEDENT;
          }
          else if(originClassEnum == PG_REGISTEREDSUBPROFILE)
          {
              expectedTargetRole = PROPERTY_ANTECEDENT;
              expectedOriginRole = PROPERTY_DEPENDENT;
          }
          break;
      case PG_REFERENCEDPROFILE:
          if (originClassEnum == PG_REGISTEREDSUBPROFILE)
          {
              expectedTargetRole = PROPERTY_ANTECEDENT;
              expectedOriginRole = PROPERTY_DEPENDENT;
          }
          else if (originClassEnum == PG_REGISTEREDPROFILE)
          {
              if ((targetProperty.size() != 0) &&
                  (originProperty.size() != 0) &&
                  String::equalNoCase(targetProperty, originProperty))
              {
                  return false;
              }
              if (targetProperty.size() != 0)
              {
                  if (!(String::equalNoCase(targetProperty, "Antecedent") ||
                      String::equalNoCase(targetProperty, "Dependent") ))
                  {
                      return false;
                  }
              }
              if (originProperty.size() != 0)
              {
                  if (!(String::equalNoCase(originProperty, "Antecedent") ||
                      String::equalNoCase(originProperty, "Dependent") ))
                  {
                      return false;
                  }
              }
              if (String::equalNoCase(originProperty, "Antecedent") &&
                  targetProperty.size() == 0)
              {
                  targetProperty = String("Dependent");
              }
              if (String::equalNoCase(originProperty, "Dependent") &&
                  targetProperty.size() == 0)
              {
                  targetProperty = String("Antecedent");
              }
              if (String::equalNoCase(targetProperty, "Antecedent") &&
                  originProperty.size() == 0)
              {
                  originProperty = String("Dependent");
              }
              if (String::equalNoCase(targetProperty, "Dependent") &&
                  originProperty.size() == 0)
              {
                  originProperty = String("Antecedent");
              }
              return true;
          }
          break;
      case PG_ELEMENTSOFTWAREIDENTITY:
          if(originClassEnum == PG_SOFTWAREIDENTITY)
          {
              expectedTargetRole = PROPERTY_DEPENDENT;
              expectedOriginRole = PROPERTY_ANTECEDENT;
          }
          else if(originClassEnum == PG_REGISTEREDPROFILE ||
              originClassEnum == PG_REGISTEREDSUBPROFILE)
          {
              expectedTargetRole = PROPERTY_ANTECEDENT;
              expectedOriginRole = PROPERTY_DEPENDENT;
          }
          break;
      case PG_INSTALLEDSOFTWAREIDENTITY:
          if(originClassEnum == PG_SOFTWAREIDENTITY)
          {
              expectedTargetRole = INSTALLEDSOFTWAREIDENTITY_PROPERTY_SYSTEM;
              expectedOriginRole =
                  INSTALLEDSOFTWAREIDENTITY_PROPERTY_INSTALLEDSOFTWARE;
          }
          else if(originClassEnum == PG_COMPUTERSYSTEM)
          {
              expectedTargetRole =
                  INSTALLEDSOFTWAREIDENTITY_PROPERTY_INSTALLEDSOFTWARE;
              expectedOriginRole = INSTALLEDSOFTWAREIDENTITY_PROPERTY_SYSTEM;
          }
          break;
      case PG_HOSTEDACCESSPOINT:
          if(originClassEnum == PG_COMPUTERSYSTEM)
          {
              expectedTargetRole = PROPERTY_DEPENDENT;
              expectedOriginRole = PROPERTY_ANTECEDENT;
          }
          else if(originClassEnum == PG_CIMXMLCOMMUNICATIONMECHANISM)
          {
              expectedTargetRole = PROPERTY_ANTECEDENT;
              expectedOriginRole = PROPERTY_DEPENDENT;
          }
      case PG_HOSTEDOBJECTMANAGER:
          if(originClassEnum == PG_COMPUTERSYSTEM)
          {
              expectedTargetRole = PROPERTY_DEPENDENT;
              expectedOriginRole = PROPERTY_ANTECEDENT;
          }
          else if(originClassEnum == PG_OBJECTMANAGER)
          {
              expectedTargetRole = PROPERTY_ANTECEDENT;
              expectedOriginRole = PROPERTY_DEPENDENT;
          }
          break;
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
      case PG_HOSTEDINDICATIONSERVICE:
          if(originClassEnum == PG_COMPUTERSYSTEM)
          {
              expectedTargetRole = PROPERTY_DEPENDENT;
              expectedOriginRole = PROPERTY_ANTECEDENT;
          }
          else if (originClassEnum == CIM_INDICATIONSERVICE)
          {
              expectedTargetRole = PROPERTY_ANTECEDENT;
              expectedOriginRole = PROPERTY_DEPENDENT;
          }
          break;
#endif
      default:
          break;
    }

    //
    // The rest of this method checks to see if target role and origin roles
    // were found for the association and origin class combination and, if
    // found, checks against the input target and origin roles if provided.
    // Failure for any of these tests points to an invalid association
    // traversal request.
    //
    if(expectedTargetRole.isNull() ||
        expectedOriginRole.isNull())
    {
        PEG_METHOD_EXIT();
        return false;
    }

    if(targetProperty.size() == 0)
    {
        targetProperty = expectedTargetRole.getString();
    }
    else if(!expectedTargetRole.equal(targetProperty))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    if(originProperty.size() == 0)
    {
        originProperty = expectedOriginRole.getString();
    }
    else if(!expectedOriginRole.equal(originProperty))
    {
        PEG_METHOD_EXIT();
        return false;
    }

    PEG_METHOD_EXIT();
    return true;
}

//
// Local version of the references operation. It validates the input
// parameters, setting the origin and target property values if not set
// already, and then performs an enumeration on the association class. It then
// filters the results of that enumeration to see if one of the reference
// properties matches the objectName parameter passed into the method. If so,
// then it is added to the array of association instances to be returned.
//
Array<CIMInstance> InteropProvider::localReferences(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & assocClass,
    String & originProperty,
    String & targetProperty,
    const CIMPropertyList & propertyList,
    const CIMName & targetClass)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::localReferences()");

    Array<CIMInstance> instances;
    CIMName originClass = objectName.getClassName();

    Array<CIMName> targetSubclasses;
    CIMNamespaceName lastTargetNamespace;
    CIMNamespaceName originNamespace(objectName.getNameSpace());

    // Check that the association traversal request is valid
    if (validAssocClassForObject(
        context,
        assocClass,
        objectName,
        originNamespace,
        originProperty,
        targetProperty))
    {
        // retrieve all of the association class instances
        Array<CIMInstance> localInstances = localEnumerateInstances(context,
            CIMObjectPath(hostName, originNamespace,
                assocClass));
        // Filter the association class instances based on the origin instance
        // and other input parameters.
        for(Uint32 i = 0, n = localInstances.size(); i < n; ++i)
        {
            CIMInstance & currentInstance = localInstances[i];
            CIMObjectPath originPath = getRequiredValue<CIMObjectPath>(
                currentInstance, originProperty);
            originPath.setNameSpace(objectName.getNameSpace());
            originPath.setHost(objectName.getHost());
            // Only include instances where the origin instance is present in
            // the association.
            if(originPath.identical(objectName))
            {
                if(!targetClass.isNull())
                {
                    // Have to check if the target reference is of the
                    // targetClass type. We first must determine all the
                    // possible subclasses of the targetClass in the target
                    // namespace.
                    CIMObjectPath targetPath = getRequiredValue<CIMObjectPath>(
                        currentInstance, targetProperty);

                    CIMNamespaceName targetNamespace(
                        targetPath.getNameSpace());
                    if(targetNamespace.isNull())
                    {
                        targetNamespace = originNamespace;
                        targetPath.setNameSpace(targetNamespace);
                    }
                    if(targetNamespace != lastTargetNamespace)
                    {
                        try
                        {
                            targetSubclasses = repository->enumerateClassNames(
                                targetNamespace, targetClass, true);
                        }
                        catch(...)
                        {
                            // If an exception was thrown during enumeration,
                            // then the base class didn't exist in the
                            // namespace, so the target instance retrieved
                            // must not match the targetClass parameter.
                            continue;
                        }
                        targetSubclasses.append(targetClass);
                        lastTargetNamespace = targetNamespace;
                    }

                    // Try to find the targetPath's class in the search space
                    CIMName targetPathClass = targetPath.getClassName();
                    for(Uint32 j = 0, m = targetSubclasses.size(); j < m; ++j)
                    {
                        if(targetPathClass == targetSubclasses[j])
                        {
                            instances.append(currentInstance);
                            break;
                        }
                    }
                }
                else
                {
                    instances.append(currentInstance);
                }
            }
        }
    }

    PEG_METHOD_EXIT();
    return instances;
}


//
// Builds an instance of the class named className. Gets Class defintion and
// fills in the correct properties from the class.  This requires a repository
// getClass request for each instance built. The skeleton is built by
// creating the instance and copying qualifiers and properties from
// the class. Finally the instance is cloned to separate it from the
// original objects.
// NOTE: This is very inefficient for anything larger than a few instances.
// We should separate the get from the createSkeleton.
// @param className CIMName of the class for which the instance is to be built
// @return CIMInstance of this class with properties complete.
// @exception passes on any exceptions received from the repository request.
//
CIMInstance InteropProvider::buildInstanceSkeleton(
      const CIMNamespaceName & nameSpace,
      const CIMName& className,
      Boolean includeQualifiers,
      CIMClass& returnedClass)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::buildInstanceSkeleton()");
    // get class with lo = false, qualifier = true classorig = true
    returnedClass = repository->getClass(nameSpace,
        className, false, true, true);
    CIMInstance skeleton = returnedClass.buildInstance(
        includeQualifiers, true, CIMPropertyList());

    PEG_METHOD_EXIT();
    return skeleton;
}


CIMInstance InteropProvider::buildDependencyInstance(
    const String & antecedentId,
    const CIMName & antecedentClass,
    const String & dependentId,
    const CIMName & dependentClass,
    const CIMClass & dependencyClass)
{
    Array<CIMKeyBinding> dependentKeys;

    dependentKeys.append(CIMKeyBinding(
        COMMON_PROPERTY_INSTANCEID,
        dependentId,CIMKeyBinding::STRING));

    return buildDependencyInstanceFromPaths(
        buildDependencyReference(hostName, antecedentId, antecedentClass),
        buildDependencyReference(hostName, dependentId, dependentClass),
        dependencyClass);
}

void InteropProvider::initProvider()
{
    if(providerInitialized)
        return;
    // Placed METHOD_ENTER trace statement after checking whether the
    // provider is initialized because this method will be called for every
    // operation through the InteropProvider, and this method is only
    // interesting the first time it is successfully run.
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::initProvider()");

    AutoMutex lock(interopMut);
    if(!providerInitialized)
    {
        //
        // Initialize the object manager instance for the CIM Server, and
        // retrieve the object manager's name property. This is retrieved once
        // and stored for use in constructing other instances requiring its
        // value.
        //
        CIMInstance objectManager = getObjectManagerInstance();
        objectManager.getProperty(objectManager.findProperty(
            OM_PROPERTY_NAME)).getValue().get(objectManagerName);

        //
        // Determine whether the CIMOM should be gathering statistical data
        // based on the GatherStatisticalData property in the object manager.
        //
        Uint32 gatherDataIndex = objectManager.findProperty(
            OM_PROPERTY_GATHERSTATISTICALDATA);
        if(gatherDataIndex != PEG_NOT_FOUND)
        {
            CIMConstProperty gatherDataProp =
                objectManager.getProperty(gatherDataIndex);
            if (gatherDataProp.getType() == CIMTYPE_BOOLEAN)
            {
                CIMValue gatherDataVal  = gatherDataProp.getValue();
                if (!gatherDataVal.isNull())
                {
                    Boolean gatherData;
                    gatherDataVal.get(gatherData);
                    if (gatherData == true)
                    {
                        StatisticalData* sd = StatisticalData::current();
                        sd->setCopyGSD(true);
                    }
                }
            }
        }

        // Cache this class definition for use later.
        profileCapabilitiesClass = repository->getClass(
            PEGASUS_NAMESPACENAME_INTEROP,
            PEGASUS_CLASSNAME_PG_PROVIDERPROFILECAPABILITIES,
            false,
            true,
            false);

        providerClassifications.append(Uint16(5)); // "Instrumentation"

        // initialize namespaces.
        initializeNamespaces();

        // Now cache the Registration info used for ElementConformsToProfile
        cacheProfileRegistrationInfo();

        providerInitialized = true;
    }
    PEG_METHOD_EXIT();
}

//
// Initialize the namespaces so that all namespaces with the
// CIM_ElementConformsToProfile class also have the
// PG_ElementConformsToProfile class. Needed in order to implement
// the cross-namespace ElementConformsToProfile association in both
// directions.
//
void InteropProvider::initializeNamespaces()
{
    Array<CIMNamespaceName> namespaceNames =  repository->enumerateNameSpaces();
    // get the PG_ElementConformstoProfile class without the qualifiers
    // and then add just the required ASSOCIATION qualifier, so that
    // resolveclass doesn't fail for the test/EmbeddedInstance/Dynamic
    // namespace, which uses the CIM25 schema that doesn't include any
    // of the new qualifiers added to this class in later versions of
    // the CIMSchema.
    CIMClass conformsClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE,
        true,
        false);
    conformsClass.addQualifier(
        CIMQualifier(CIMName("ASSOCIATION"), CIMValue(true)));
    CIMClass profileClass = repository->getClass(
        PEGASUS_NAMESPACENAME_INTEROP,
        PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE,
        true,
        false);
    for(Uint32 i = 0, n = namespaceNames.size(); i < n; ++i)
    {
        // Check if the PG_ElementConformsToProfile class is present
        CIMNamespaceName & currentNamespace = namespaceNames[i];
        CIMClass tmpCimClass;
        CIMClass tmpPgClass;
        CIMClass tmpPgProfileClass;
        try
        {
            // Look for these classes in the same try-block since the
            // second depends on the first
            tmpCimClass = repository->getClass(
                currentNamespace,
                PEGASUS_CLASSNAME_CIM_ELEMENTCONFORMSTOPROFILE);
            tmpPgClass = repository->getClass(
                currentNamespace,
                PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE);
        }
        catch(const Exception &)
        {
        }
        try
        {
            tmpPgProfileClass = repository->getClass(
                currentNamespace,
                PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE);
        }
        catch(const Exception &)
        {
            // Note: if any of the above three classes aren't found,
            // an exception will be thrown, which we can ignore since it's
            // an expected case
            // TBD: Log trace message?
        }

        // If the CIM_ElementConformsToProfile class is present, but
        // the PG_ElementConformsToProfile or PG_RegisteredProfile
        // class is not, then add it to that namespace.
        //
        // Note that we don't have to check for the
        // CIM_RegisteredProfile class because if the
        // CIM_ElementConformsToProfile class is present, the
        // CIM_RegisteredProfile class must also be present.
        if(!tmpCimClass.isUninitialized())
        {
            if(tmpPgClass.isUninitialized())
            {
                CIMClass newclass = conformsClass.clone();
                CIMObjectPath newPath = conformsClass.getPath();
                newPath.setNameSpace(currentNamespace);
                newclass.setPath(newPath);
                repository->createClass(
                    currentNamespace,
                    newclass);
            }
            if(tmpPgProfileClass.isUninitialized())
            {
                CIMClass newclass = profileClass.clone();
                CIMObjectPath newPath = profileClass.getPath();
                newPath.setNameSpace(currentNamespace);
                newclass.setPath(newPath);
                repository->createClass(
                    currentNamespace,
                    newclass);
            }
        }
    }
}

PEGASUS_NAMESPACE_END
// END OF FILE
