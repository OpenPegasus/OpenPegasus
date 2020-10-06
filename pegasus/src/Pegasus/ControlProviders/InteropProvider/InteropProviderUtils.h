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

#ifndef InteropProviderUtils_h
#define InteropProviderUtils_h

#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/String.h>

PEGASUS_NAMESPACE_BEGIN

// Enum for class selection for instance operations.
enum TARGET_CLASS
{
    NOCLASS,
    PG_NAMESPACE,
    PG_OBJECTMANAGER,
    PG_CIMXMLCOMMUNICATIONMECHANISM,
    PG_NAMESPACEINMANAGER,
    PG_COMMMECHANISMFORMANAGER,
    PG_REGISTEREDPROFILE,
    PG_REGISTEREDSUBPROFILE,
    PG_REFERENCEDPROFILE,
    PG_ELEMENTCONFORMSTOPROFILE,
    PG_SUBPROFILEREQUIRESPROFILE,
    PG_SOFTWAREIDENTITY,
    PG_ELEMENTSOFTWAREIDENTITY,
    PG_INSTALLEDSOFTWAREIDENTITY,
    PG_COMPUTERSYSTEM,
    PG_HOSTEDOBJECTMANAGER,
    PG_HOSTEDACCESSPOINT,
    PG_ELEMENTCONFORMSTOPROFILE_RP_RP,
    CIM_NAMESPACE,
    PG_PROVIDERPROFILECAPABILITIES,
    PG_PROVIDERREFERENCEDPROFILES
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    ,PG_ELEMENTCAPABILITIES,
    PG_HOSTEDINDICATIONSERVICE,
    PG_SERVICEAFFECTSELEMENT,
    CIM_INDICATIONSERVICE
#endif
};

/***************************************************************
 *                                                             *
 *               Provider Utility Functions                    *
 *                                                             *
 ***************************************************************/

//
// Helper function that constructs an the InstanceId property out of its
// constituent pieces.
//
String buildProfileInstanceId(
    const String & organization,
    const String & name,
    const String & version);

//
// function that creates an object path given a class definition, an
// instance of that class, the host name, and the namespace.
//
CIMObjectPath buildInstancePath(
    const CIMClass & cimClass,
    const String & hostName,
    const CIMNamespaceName & nameSpace,
    const CIMInstance & instance);

//
// Template function that retrieves the value of a property. The template
// type determines the type that should be contained within the CIMValue object
// of the property (specified by the propName parameter) in the supplied
// instance. This is used particularly for Required properties, so if the
// property is not found or is NULL, an exception will be thrown.
//
template <class RetClass>
RetClass getRequiredValue(const CIMInstance & instance,
                          const CIMName & propName)
{
    RetClass retVal;
    Uint32 index = instance.findProperty(propName);
    if(index == PEG_NOT_FOUND)
    {
        throw CIMOperationFailedException("Instance " +
            instance.getPath().toString() +
            " missing expected property " + propName.getString());
    }
    const CIMValue & tmpVal = instance.getProperty(index).getValue();
    if(tmpVal.isNull())
    {
        throw CIMOperationFailedException("Instance " +
            instance.getPath().toString() +
            " has unexpected NULL value for property " + propName.getString());
    }

    tmpVal.get(retVal);

    return retVal;
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
bool namespaceSupported(const CIMObjectPath & path);
*/

//
// Normalize the instance by setting the complete path for the instance and
// executing the instance filter to set the qualifiers, classorigin, and
// property list in accordance with the input.  Note that this can only remove
// characteristics, except for the path completion, so that it expects
// instances with qualifiers, class origin, and a complete set of properties
// already present in the instance.
//
void normalizeInstance(CIMInstance& instance, const CIMObjectPath& path,
                       Boolean includeQualifiers, Boolean includeClassOrigin,
                       const CIMPropertyList& propertyList);

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
    const CIMName & propertyName, const String & defaultValue);

//
// Overload of getPropertyValue for boolean type
//
Boolean getPropertyValue(const CIMInstance & instance,
    const CIMName & propertyName, const Boolean defaultValue);

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
    const String& port);

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
    const String & value);

//
// Same as above, overloaded to check key properties in CIMObjectPath objects
// against a string value.
//
Boolean validateRequiredProperty(
    const CIMObjectPath & objectPath,
    const CIMName & propertyName,
    const String & value);

//
// Verify that this is one of the legal classnames for instance operations and
// return an indicator as to which one it is.
// @param - Classname
// @return - Enum value indicating type
// @Exceptions - throws CIMNotSupportedException if invalid class.
//
TARGET_CLASS translateClassInput(const CIMName& className);

//
// Same as method above, but used specifically for association classes.
//
TARGET_CLASS translateAssocClassInput(const CIMName & className);

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
    const CIMValue & value);

//
// Sets the correct values to the common keys defined for all of the classes.
// This is SystemCreationClassName and SystemName. Note that if the properties
// do not exist, we simply ignore them.
//
void setCommonKeys(CIMInstance& instance);

//
// Retrieves the key binding given by the keyName parameter from the supplied
// object path.
//
String getKeyValue(const CIMObjectPath& instanceName, const CIMName& keyName);

//
// Retrieves the key binding given by the keyName parameter from the supplied
// instance.
//
String getKeyValue(const CIMInstance& instance, const CIMName& keyName);

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
    const CIMClass & classDef);

//
// Same as above, but converts an integral value into a string first so that
// it can be found when searching the Values qualifier (or some similar
// qualifier).
//
String translateValue(Uint16 value, const CIMName & propName,
    const CIMName & sourceQualifier, const CIMName & targetQualifier,
    const CIMClass & classDef);

//
// helper function for building a reference ObjectPath for an instance
// of CIM_Dependency.
//
CIMObjectPath buildDependencyReference(
    const String & hostName,
    const String & instanceId,
    const CIMName & instanceClass);

//
// helper function for building an instance of CIM_Dependency given
// the antecedent and dependent references and the concrete subclass for which
// the instance will be created.
//
CIMInstance buildDependencyInstanceFromPaths(
    const CIMObjectPath & antecedent,
    const CIMObjectPath & dependent,
    const CIMClass & dependencyClass);

//
// Given an instance of PG_ProviderProfileCapabilities, this method retrieves
// the values necessary for constructing instances of PG_RegisteredProfile,
// PG_RegisteredSubProfile, and all of their associations.
// NOTE: This function is implemented in RegisteredProfile.cpp but declared
//       here so that it can be accessed by ElementConformsToProfile.cpp and
//       Software.cpp.
//
String extractProfileInfo(
    const CIMInstance & profileCapabilities,
    const CIMClass & capabilitiesClass,
    const CIMClass & profileClass,
    String & name,
    String & version,
    Uint16 & organization,
    String & organizationName,
    Array<String> & subprofileNames,
    Array<String> & subprofileVersions,
    Array<Uint16> & subprofileOrganizations,
    Array<String> & subprofileOrganizationNames,
    Array<String> & subProfileProviderModuleNames,
    Array<String> & subProfileProviderNames,
    bool noSubProfileInfo = true);

PEGASUS_NAMESPACE_END

#endif // InteropProviderUtils_h
