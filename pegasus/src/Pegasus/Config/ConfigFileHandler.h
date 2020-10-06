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


#ifndef Pegasus_ConfigFileHandler_h
#define Pegasus_ConfigFileHandler_h

#include <cctype>
#include <Pegasus/Config/Linkage.h>
#include <Pegasus/Config/ConfigFile.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/AutoPtr.h>


PEGASUS_NAMESPACE_BEGIN


////////////////////////////////////////////////////////////////////////////////
//  ConfigFileHandler Class
////////////////////////////////////////////////////////////////////////////////

struct ConfigTable;

/**
  This class provides methods to read/write config properties
  from the config file.
*/
class PEGASUS_CONFIG_LINKAGE ConfigFileHandler
{
public:

    /**
    Default constructor.

    @param        currentFile current config file name.
    @param        plannedFile planned config file name.
    @exception  NoSuchFile  if file cannot be opened.
    @exception  FileNotReadable  if file is not readable.
    */
    ConfigFileHandler (
        const String& currentFile = PEGASUS_CURRENT_CONFIG_FILE_PATH,
        const String& plannedFile = PEGASUS_PLANNED_CONFIG_FILE_PATH,
        const Boolean offLine = false);


    /** Destructor. */
    ~ConfigFileHandler ();


    /**
    Overwrites config properties in the current config file with the
    the config properties from the planned config file.

    The content of the current config file will be copied in to a
    backup (.bak) file before copying planned file contents over the
    current file.

    @exception  CannotRenameFile  if failed to create the backup file.
    @exception  CannotOpenFile  if failed to set permissions on the config file.
    */
    void copyPlannedFileOverCurrentFile();


    /**
    Load the config properties from the config files.

    @exception ConfigFileSyntaxError if file contains a syntax error.
    */
    void loadAllConfigProperties ();


    /**
    Load the config properties from the current config file.

    @exception ConfigFileSyntaxError if file contains a syntax error.
    */
    void loadCurrentConfigProperties ();


    /**
    Load the config properties from the planned config file.

    @exception ConfigFileSyntaxError if file contains a syntax error.
    */
    void loadPlannedConfigProperties ();


    /**
    Update the specified property name and value in the current
    config file.

    @param  name   name of the property to be updated.
    @param  value  value of the property to be updated.
    @param  userName  User requesting the update
    @param timeoutSeconds Timeout in seconds to complete the update.
    @param  unset  specifies whether the property should be updated or unset.
    @return true   if the property updated successfully, else false.
    */
    Boolean updateCurrentValue(
        const CIMName& name,
        const String& value,
        const String& userName,
        Uint32 timeoutSeconds,
        Boolean unset);


    /**
    Update the specified property name and value in the planned
    config file.

    @param  name   name of the property to be updated.
    @param  value  value of the property to be updated.
    @param  unset  specifies whether the property should be updated or unset.
    @return true   if the property updated successfully, else false.
    */
    Boolean updatePlannedValue(
        const CIMName& name,
        const String& value,
        Boolean unset);


    /**
    Get the current property value for the specified property name.

    @param  name   name of the property.
    @param  value  value of the property (output parameter).
    @return true if the property is found and returned, false otherwise.
    */
    Boolean getCurrentValue (const CIMName& name, String& value) const;


    /**
    Get the planned property value for the specified property name.

    @param  name   name of the property.
    @param  value  value of the property (output parameter).
    @return true if the property is found and returned, false otherwise.
    */
    Boolean getPlannedValue (const CIMName& name, String& value) const;


    /**
    Get all current property names.

    @param  propertyNames   string array to hold the property names.
    */
    void getAllCurrentPropertyNames (Array<CIMName>& propertyNames);


    /**
    Get all current property names and values.

    @param  propertyNames   string array to hold the property names.
    @param  propertyValues  string array to hold the property values.
    */
    void getAllCurrentProperties (
        Array<CIMName>& propertyNames,
        Array<String>& propertyValues);


    /**
    Get all planned config property names.

    @param  propertyNames   string array to hold the property names.
    */
    void getAllPlannedPropertyNames (Array<CIMName>& propertyNames);


    /**
    Get all planned config property names and values.

    @param  propertyNames   string array to hold the property names.
    @param  propertyValues  string array to hold the property values.
    */
    void getAllPlannedProperties(
        Array<CIMName>& propertyNames,
        Array<String>& propertyValues);

private:

    /**
    Tables to store the current and planned config properties.
    */
    ConfigTable* _currentConfig;

    ConfigTable* _plannedConfig;

    /**
    File handlers for the current and planned config files.
    */
    AutoPtr<ConfigFile>  _currentConfFile; //PEP101

    AutoPtr<ConfigFile>  _plannedConfFile; //PEP101

    /**
    Flag indicating whether the current config files exists or not.
    */
    Boolean      _currentFileExist;

    /**
    Flag indicating whether the planned config files exists or not.
    */
    Boolean      _plannedFileExist;

    /**
    Flag indicating whether config file is accessed off line or not.
    */
    Boolean      _offLine;

};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ConfigFileHandler_h */
