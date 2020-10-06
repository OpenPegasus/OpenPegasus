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

#include "ConfigPropertyOwner.h"
#include "ConfigPropertyHelp.h"

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

const String STRING_TRUE = "true";
const String STRING_FALSE = "false";

static const char CONFIG_PROP_HELP_NOT_AVAILABLE [] =
    "Help not available for this property";
static const char  CONFIG_PROP_HELP_NOT_AVAILABLE_KEY [] =
    "Config.ConfigPropertyOwner.CONFIG_PROP_HELP_NOT_AVAILABLE";

const char CONFIG_MSG_PATH [] = "pegasus/pegasusServer";

// Creates string of form:
//     Config.DefaultPropertyOwner.<propertyName>.DESCRIPTION
String _createKey(const char* propertyName)
{
    String rtn = "Config.ConfigPropertyHelp.";
    rtn.append("DESCRIPTION_");
    rtn.append(propertyName);
    return rtn;
}

String _descriptionWithPrefix(const char* description)
{
    String rtn = description;
    return rtn;
}

// build up the standard propertyInfo containing info from the property
// defined by the provided ConfigProperty
void ConfigPropertyOwner::buildPropertyInfo(const String& name,
    const struct ConfigProperty * configProperty,
    Array<String>& propertyInfo) const
{
    propertyInfo.clear();
    // build propertyInfo String Array.
    propertyInfo.append(configProperty->propertyName);
    propertyInfo.append(configProperty->defaultValue);
    propertyInfo.append(configProperty->currentValue);
    propertyInfo.append(configProperty->plannedValue);
    if (configProperty->dynamic)
        {
            propertyInfo.append(STRING_TRUE);
        }
        else
        {
            propertyInfo.append(STRING_FALSE);
        }
    if (configProperty->externallyVisible)
        {
            propertyInfo.append(STRING_TRUE);
        }
        else
        {
            propertyInfo.append(STRING_FALSE);
        }

    propertyInfo.append(getPropertyHelp(name));
}

// This function provides the default behaviour for PropertyOwners which do
// not override getPropertyHelp() function. Attempts to get a Description
// text from the configPropertyDescriptionList (see ConfigPropertyHelp.cpp)
// for the string and return the data as String.
//
String ConfigPropertyOwner::getPropertyHelp(const String& name) const
{
    String localPropertyInfo;
    for (Uint32 i = 0; i < configPropertyDescriptionListSize ; i++)
    {
        // Return the complete message in propertyInfo if name found
        if (configPropertyDescriptionList[i].name == name)
        {
            // Create the key and add description with any standard
            // prefix information.
            localPropertyInfo.append(loadMessage(
                (const char *)_createKey(
                    configPropertyDescriptionList[i].name).getCString(),
                (const char *)_descriptionWithPrefix(
                    configPropertyDescriptionList[i].Description).getCString()
                ));
            return localPropertyInfo;
        }
    }

    // Not found, return error.
    localPropertyInfo.append(loadMessage(
        CONFIG_PROP_HELP_NOT_AVAILABLE_KEY,
        CONFIG_PROP_HELP_NOT_AVAILABLE));

    return localPropertyInfo;
}

// Returns empty String since this is the default behavior. May be overridden in
// subclasses to supply specific values for entries such as traceComponents,
// etc.  Property owners that do not require anything but the description
// string for help output should not override this function. Normally
// all help should be in the fixed help file (see getPropertyHelp above)
// and only information that might vary by system version or server compile
// addressed through PropertyHelpSupplement. This would allow a particular
// server configuration to, for example, list the selection options available.
String ConfigPropertyOwner::getPropertyHelpSupplement(const String& name) const
{
    return "";
}

String ConfigPropertyOwner::loadMessage(const char* key, const char* msg)
{
    MessageLoaderParms parms(key, msg);
    parms.msg_src_path = CONFIG_MSG_PATH;
    return MessageLoader::getMessage(parms);
}



PEGASUS_NAMESPACE_END
