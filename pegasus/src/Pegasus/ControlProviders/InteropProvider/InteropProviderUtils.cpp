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
/* TODO LIST:

    1. UUID generation should become a system function since it will be used
       by many providers, etc. as part of id generation.

    2. Review the key parameters on create, etc. to be sort out which are
       required from user and which we can supply.  I think we are asking too
       much of the user right now.
*/

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <cctype>
#include <iostream>

#include "InteropProvider.h"
#include "InteropProviderUtils.h"
#include "InteropConstants.h"

#include <stdlib.h>

//The following includes are needed for gethostbyname and AF_INET6
#if defined(PEGASUS_OS_TYPE_WINDOWS)
#include <objbase.h>
#include <winsock2.h>
#else
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

/***************************************************************
 *                                                             *
 *               Provider Utility Functions                    *
 *                                                             *
 ***************************************************************/


//
// function that creates an object path given a class definition, an
// instance of that class, the host name, and the namespace.
//
CIMObjectPath buildInstancePath(
    const CIMClass & cimClass,
    const String & hostName,
    const CIMNamespaceName & nameSpace,
    const CIMInstance & instance)
{
    CIMObjectPath objPath = instance.buildPath(cimClass);
    objPath.setHost(hostName);
    objPath.setNameSpace(nameSpace);
    return objPath;
}

//
// Determines if the namespace is allowable for this operation.
// This provider is designed to accept either all namespaces or
// limit itself to just one for operations.  In all cases, it
// will provide the required answers and use the correct namespace
// for any persistent information.  However, it may be configured
// to either accept input operations from any namespace or simply
// from one (normally the interop namespace).
// @ objectReference for the operation.  This must include the
// namespace and class name for the operation.
// @return Returns normally if the namespace test is passed. Otherwise
// it generates a CIMException (CIM_ERR_NOT_SUPPORTED)
// @exception CIMException(CIM_ERR_NOT_SUPPORTED)
//
// NOTE:This function is commented out because the routing tables will always
// do the right thing and that's the only way requests get here. If this
// changes, then this function should be reinstated along with the various
// locations where calls to the function are also commented out.
//
/*
bool namespaceSupported(const CIMObjectPath & path)
{
    // To allow use of all namespaces, uncomment the following line
    // return;
    if(path.getNameSpace().getString() == PEGASUS_NAMESPACENAME_INTEROP)
      return true;
    //// If this is ever reinstalled please review which Exception should
    //// be used. Not clear that NotSupported is correct
    throw CIMNotSupportedException(path.getClassName().getString() +
      " in namespace " + path.getNameSpace().getString());

    return false;
}
*/

// Normalize the instance by setting the complete path for the instance.
void normalizeInstance(CIMInstance& instance, const CIMObjectPath& path,
                       Boolean includeQualifiers, Boolean includeClassOrigin,
                       const CIMPropertyList& propertyList)
{
    CIMObjectPath p = instance.getPath();
    p.setHost(path.getHost());
    p.setNameSpace(path.getNameSpace());

    instance.setPath(p);
}

//
// Get one string property from an instance. Note that these functions simply
// return the default value if the property cannot be found or is of the wrong
// type.
// @param instance CIMInstance from which we get property value
// @param propertyName String name of the property containing the value
// @param default String optional parameter that is substituted if the property
// does not exist or is NULL.
// @return String value found or defaultValue.
//
String getPropertyValue(const CIMInstance & instance,
    const CIMName & propertyName, const String & defaultValue)
{
    String output = defaultValue;
    Uint32 pos = instance.findProperty(propertyName);
    if(pos != PEG_NOT_FOUND)
    {
        CIMConstProperty p1 = instance.getProperty(pos);
        if(p1.getType() == CIMTYPE_STRING)
        {
            CIMValue v1  = p1.getValue();

            if(!v1.isNull())
                v1.get(output);
        }
        else
        {
            throw CIMInvalidParameterException(
                "Incorrect Property Type for Property " +
                propertyName.getString());
        }
    }

    return output;
}

//
// Overload of getPropertyValue for boolean type
//
Boolean getPropertyValue(const CIMInstance & instance,
    const CIMName & propertyName, const Boolean defaultValue)
{
    Boolean output = defaultValue;
    Uint32 pos = instance.findProperty(propertyName);
    if(pos != PEG_NOT_FOUND)
    {
        CIMConstProperty p1 = instance.getProperty(pos);
        if (p1.getType() == CIMTYPE_BOOLEAN)
        {
            CIMValue v1  = p1.getValue();

            if (!v1.isNull())
                v1.get(output);
        }
        else
        {
            throw CIMInvalidParameterException(
                "Incorrect Property Type for Property " +
                propertyName.getString());
        }
    }
    return output;
}

//
// Get Host IP address from host name. If the host name is not provided,
// uses internal function. If everything fails, gets the definition normally
// used for localhost (127.0.0.1).
//
// @param hostName String with the name of the host. Allows String:EMPTY and
//     in that case, gets it directly from system.
// @param namespaceType - Uint32 representing the access protocol for this
//     request.  This is exactly the definition in the
//     PG_CIMXMLCommunicationMechanism mof for the property
//     namespaceAccessProtocol.
// @param port String defining the port to be used.
//
// @return String with the IP address to be used. This must be the complete
//     address sufficient to access the IP address. Therefore, it includes the
//     port number.
//
String getHostAddress(
    const String& hostName,
    Uint32 namespaceType,
    const String& port)
{
  String ipAddress;
  int af;
  if(hostName == String::EMPTY)
      System::getHostIP(System::getHostName(), &af, ipAddress);
  else
      System::getHostIP(hostName, &af, ipAddress);

  if(ipAddress == String::EMPTY)
  {
      // set default address if everything else failed
      ipAddress = String("127.0.0.1");
  }

  // Question: Is there a case where we leave off the port number?
  // Code to get the property service_location_tcp ( which is equivalent to
  // "IP address:5988")
#ifdef PEGASUS_ENABLE_IPV6
  if (af == AF_INET6)
  {
      ipAddress = "[" + ipAddress + "]";
  }
#endif
  ipAddress.append(":");
  ipAddress.append(port);

  return ipAddress;
}

//
// Validate that the property exists, is string type and optionally the value
// itself. NOTE: This function processes only String properties.
//
// @param Instance to search for property.
// @param CIMName containing property Name
// @param String containing value. If not String::EMPTY, compare to value
//     in the property
// @return True if passes all tests
//
Boolean validateRequiredProperty(
    const CIMInstance & instance,
    const CIMName & propertyName,
    const String & value)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
            "InteropProvider::_validateRequiredProperty()");
    Uint32 pos = instance.findProperty(propertyName);
    if(pos == PEG_NOT_FOUND)
    {
        PEG_METHOD_EXIT();
        return false;
    }
    //
    //  Get the property
    //
    CIMConstProperty theProperty = instance.getProperty(pos);
    const CIMValue theValue = theProperty.getValue();
    //
    // Required property must have a non-null value
    //
    if(theValue.getType() != CIMTYPE_STRING || theValue.isNull())
    {
        PEG_METHOD_EXIT();
        return false;
    }

    String valueField;
    theValue.get(valueField);
    if(value == String::EMPTY || valueField == value)
    {
        PEG_METHOD_EXIT();
        return true;
    }
    PEG_METHOD_EXIT();
    return false;
}

//
// Same as above method, overloaded for Uint16 values
//
/*
Boolean validateRequiredProperty(
    const CIMInstance & instance,
    const CIMName & propertyName,
    const Uint16 & value)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::_validateRequiredProperty()");

    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4, "Validate "
        + propertyName.getString());

    Uint32 pos;
    if ((pos = instance.findProperty (propertyName)) == PEG_NOT_FOUND)
    {
        PEG_METHOD_EXIT();
        return false;
    }
    //
    //  Get the property
    //
    CIMConstProperty theProperty = instance.getProperty(pos);
    CIMValue theValue = theProperty.getValue();
    //
    //  ATTN:Required property must have a non-null value
    //
    if ((theValue.getType() != CIMTYPE_UINT16)
        || (theValue.isNull()) )
    {
        PEG_METHOD_EXIT();
        return false;
    }
    PEG_METHOD_EXIT();
    return true;
}*/

//
// Same as above, overloaded to check key properties in CIMObjectPath objects
// against a string value.
//
Boolean validateRequiredProperty(
    const CIMObjectPath & objectPath,
    const CIMName & propertyName,
    const String & value)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
            "InteropProvider::validateRequiredProperty()");
    Array<CIMKeyBinding> kbArray = objectPath.getKeyBindings();
    Boolean retVal = false;
    // find the correct key binding
    for(Uint32 i = 0; i < kbArray.size(); i++)
    {
        if(kbArray[i].getName() == propertyName)
        {
            retVal = (value == String::EMPTY ||
                value == kbArray[i].getValue());
            break;
        }
    }
    PEG_METHOD_EXIT();
    return retVal;
}

//
// Verify that this is one of the legal classnames for instance operations and
// return an indicator as to which one it is.
// @param - Classname
// @return - Enum value indicating type
// @Exceptions - throws CIMOperationFailedException if invalid class.
//
TARGET_CLASS translateClassInput(const CIMName& className)
{
    if(className.equal(PEGASUS_CLASSNAME_PG_OBJECTMANAGER))
        return PG_OBJECTMANAGER;

    else if(className.equal(PEGASUS_CLASSNAME_PG_CIMXMLCOMMUNICATIONMECHANISM))
        return PG_CIMXMLCOMMUNICATIONMECHANISM;

    else if(className.equal(PEGASUS_CLASSNAME_PG_NAMESPACEINMANAGER))
        return PG_NAMESPACEINMANAGER;

    else if(className.equal(PEGASUS_CLASSNAME_PG_COMMMECHANISMFORMANAGER))
        return PG_COMMMECHANISMFORMANAGER;

    else if(className.equal(PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE))
        return PG_REGISTEREDPROFILE;

    else if(className.equal(PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE))
        return PG_REGISTEREDSUBPROFILE;

    else if(className.equal(PEGASUS_CLASSNAME_PG_REFERENCEDPROFILE))
        return PG_REFERENCEDPROFILE;

    else if(className.equal(PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE))
        return PG_ELEMENTCONFORMSTOPROFILE;

    else if(className.equal(
        PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE_RP_RP))
            return PG_ELEMENTCONFORMSTOPROFILE_RP_RP;

    else if(className.equal(PEGASUS_CLASSNAME_PG_SUBPROFILEREQUIRESPROFILE))
        return PG_SUBPROFILEREQUIRESPROFILE;

    else if(className.equal(PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY))
        return PG_SOFTWAREIDENTITY;

    else if(className.equal(PEGASUS_CLASSNAME_PG_ELEMENTSOFTWAREIDENTITY))
        return PG_ELEMENTSOFTWAREIDENTITY;

    else if(className.equal(PEGASUS_CLASSNAME_PG_INSTALLEDSOFTWAREIDENTITY))
        return PG_INSTALLEDSOFTWAREIDENTITY;

    else if(className.equal(PEGASUS_CLASSNAME_PG_COMPUTERSYSTEM))
        return PG_COMPUTERSYSTEM;

    else if(className.equal(PEGASUS_CLASSNAME_PG_HOSTEDOBJECTMANAGER))
        return PG_HOSTEDOBJECTMANAGER;

    else if(className.equal(PEGASUS_CLASSNAME_PG_HOSTEDACCESSPOINT))
        return PG_HOSTEDACCESSPOINT;

    else if(className.equal(PEGASUS_CLASSNAME_CIMNAMESPACE))
        return CIM_NAMESPACE;

    else if(className.equal(PEGASUS_CLASSNAME_PG_PROVIDERPROFILECAPABILITIES))
        return PG_PROVIDERPROFILECAPABILITIES;

    else if(className.equal(PEGASUS_CLASSNAME_PG_PROVIDERREFERENCEDPROFILES))
        return PG_PROVIDERREFERENCEDPROFILES;

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    else if(className.equal(PEGASUS_CLASSNAME_PG_ELEMENTCAPABILITIES))
        return PG_ELEMENTCAPABILITIES;

    else if(className.equal(PEGASUS_CLASSNAME_PG_HOSTEDINDICATIONSERVICE))
        return PG_HOSTEDINDICATIONSERVICE;

    else if(className.equal(PEGASUS_CLASSNAME_PG_SERVICEAFFECTSELEMENT))
        return PG_SERVICEAFFECTSELEMENT;

    else if(className.equal(PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE))
        return CIM_INDICATIONSERVICE;
#endif

    // Last entry, reverse test and throw exception if not PG_Namespace
    // Note: Changed to PG_Namespace for CIM 2.4
    else if(!className.equal(PEGASUS_CLASSNAME_PGNAMESPACE))
    {
        throw CIMOperationFailedException
            (className.getString() +
               " Class not supported by Interop Provider");
    }

    return PG_NAMESPACE;
}

//
// Same as method above, but used specifically for association classes.
//
TARGET_CLASS translateAssocClassInput(const CIMName & className)
{
    if(className.equal(PEGASUS_CLASSNAME_PG_NAMESPACEINMANAGER))
        return PG_NAMESPACEINMANAGER;

    else if(className.equal(PEGASUS_CLASSNAME_PG_COMMMECHANISMFORMANAGER))
        return PG_COMMMECHANISMFORMANAGER;

    else if(className.equal(PEGASUS_CLASSNAME_PG_REFERENCEDPROFILE))
        return PG_REFERENCEDPROFILE;

    else if(className.equal(PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE))
        return PG_ELEMENTCONFORMSTOPROFILE;

    else if(className.equal(
        PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE_RP_RP))
            return PG_ELEMENTCONFORMSTOPROFILE_RP_RP;

    else if(className.equal(PEGASUS_CLASSNAME_PG_ELEMENTSOFTWAREIDENTITY))
        return PG_ELEMENTSOFTWAREIDENTITY;

    // Last entry, reverse test and throw exception if not
    // PG_SubProfileRequiresProfile
    else if(!className.equal(PEGASUS_CLASSNAME_PG_SUBPROFILEREQUIRESPROFILE))
    {
        throw CIMOperationFailedException(className.getString() +
          " Class not supported by association operations in the "
              "Interop Provider");
    }

    return PG_SUBPROFILEREQUIRESPROFILE;
}

//
// Set the value of a property defined by property name in the instance
// provided. If the property cannot be found, it simply returns.
//
// @param instance CIMInstance in which to set property value
// @param propertyName CIMName of property in which value will be set.
// @param value CIMValue value to set into property
//
// @return true if property value was set, false if the property was not found
//
void setPropertyValue(CIMInstance& instance, const CIMName& propertyName,
    const CIMValue & value)
{
    //return bool? would be pos != PEG_NOT_FOUND
    unsigned int pos = instance.findProperty(propertyName);
    if(pos != PEG_NOT_FOUND)
    {
        instance.getProperty(pos).setValue(value);
    }
}

//
// Sets the correct values to the common keys defined for all of the classes.
// This is SystemCreationClassName and SystemName. Note that if the properties
// do not exist, we simply ignore them.
//
void setCommonKeys(CIMInstance& instance)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::setCommonKeys()");

    setPropertyValue(instance, COMMON_PROPERTY_SYSTEMCREATIONCLASSNAME,
        PEGASUS_CLASSNAME_PG_COMPUTERSYSTEM.getString());

    // Add property SystemName
    setPropertyValue(instance, COMMON_PROPERTY_SYSTEMNAME,
        System::getFullyQualifiedHostName());
    PEG_METHOD_EXIT();
}


//
// Retrieves the key binding given by the keyName parameter from the supplied
// object path.
//
String getKeyValue(const CIMObjectPath& instanceName, const CIMName& keyName)
{
    Array<CIMKeyBinding> kbArray = instanceName.getKeyBindings();

    // find the correct key binding
    for(Uint32 i = 0, n = kbArray.size(); i < n; ++i)
    {
        if (kbArray[i].getName() == keyName)
            return kbArray[i].getValue();
    }

    throw CIMInvalidParameterException("Key property not found: " +
        keyName.getString());
}

//
// Retrieves the key binding given by the keyName parameter from the supplied
// instance.
//
String getKeyValue(const CIMInstance& instance, const CIMName& keyName)
{
    Uint32 pos;
    CIMValue propertyValue;

    pos = instance.findProperty(keyName);
    if(pos == PEG_NOT_FOUND)
        throw CIMPropertyNotFoundException(keyName.getString());

    propertyValue = instance.getProperty(pos).getValue();
    if(propertyValue.getType() != CIMTYPE_STRING)
    {
        throw CIMInvalidParameterException("Invalid type for property: "
            + keyName.getString());
    }

    String name;
    propertyValue.get(name);
    return name;
}

//
// The following method is used to translate a string based on the
// Value/ValueMap qualifiers of a property. Note that the method is written
// in such a way that the translation could be done in either direction
// (from Value value to ValueMap value or vice versa) or with another pair
// of qualifiers with a relationship similar to the Value/ValueMap pair.
//
String translateValue(
    const String & value,
    const CIMName & propName,
    const CIMName & sourceQualifier,
    const CIMName & targetQualifier,
    const CIMClass & classDef)
{
    String mappedValue;
    Uint32 index = classDef.findProperty(propName);
    if(index != PEG_NOT_FOUND)
    {
        CIMConstProperty prop = classDef.getProperty(index);
        index = prop.findQualifier(sourceQualifier);
        if(index != PEG_NOT_FOUND)
        {
            Array<String> sourceQualValues;
            prop.getQualifier(index).getValue().get(sourceQualValues);
            for(Uint32 i = 0, n = sourceQualValues.size(); i < n; ++i)
            {
                // If we have a match in the Source qualifier, then get the
                // related string from the Target qualifier
                if(sourceQualValues[i] == value)
                {
                    index = prop.findQualifier(targetQualifier);
                    if(index != PEG_NOT_FOUND)
                    {
                        Array<String> targetQualValues;
                        prop.getQualifier(index).getValue().get(
                            targetQualValues);
                        mappedValue = targetQualValues[i];
                    }
                    break;
                }
            }
        }
    }

    return mappedValue;
}

//
// Same as above, but converts an integral value into a string first so that
// it can be found when searching the Values qualifier (or some similar
// qualifier).
//
String translateValue(Uint16 value, const CIMName & propName,
    const CIMName & sourceQualifier, const CIMName & targetQualifier,
    const CIMClass & classDef)
{
    return translateValue(CIMValue(value).toString(), propName,
      sourceQualifier, targetQualifier, classDef);
}

//
// helper function for building a reference ObjectPath for an instance
// of CIM_Dependency.
//
CIMObjectPath buildDependencyReference(
    const String & hostName,
    const String & instanceId,
    const CIMName & instanceClass)
{
    Array<CIMKeyBinding> instanceKeys;
    instanceKeys.append(CIMKeyBinding(
        COMMON_PROPERTY_INSTANCEID,
        instanceId,CIMKeyBinding::STRING));

    return CIMObjectPath(hostName,
        PEGASUS_NAMESPACENAME_INTEROP,
        instanceClass,
        instanceKeys);
}

//
// helper function for building an instance of CIM_Dependency given
// the antecedent and dependent references and the concrete subclass for which
// the instance will be created.
//
CIMInstance buildDependencyInstanceFromPaths(
    const CIMObjectPath & antecedent,
    const CIMObjectPath & dependent,
    const CIMClass & dependencyClass)
{
    CIMInstance dependencyInst = dependencyClass.buildInstance(false, false,
            CIMPropertyList());
    setPropertyValue(dependencyInst, PROPERTY_ANTECEDENT,
        CIMValue(antecedent));
    setPropertyValue(dependencyInst, PROPERTY_DEPENDENT,
        CIMValue(dependent));
    dependencyInst.setPath(dependencyInst.buildPath(dependencyClass));
    return dependencyInst;
}

//
// Helper function that constructs an the InstanceId property out of its
// constituent pieces.
//
String buildProfileInstanceId(const String & organization,
                                     const String & name,
                                     const String & version)
{
    return organization + "+" + name + "+" + version;
}

PEGASUS_NAMESPACE_END

// END_OF_FILE
