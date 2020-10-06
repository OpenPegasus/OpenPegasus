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


#ifndef Pegasus_CIMConfigCommand_h
#define Pegasus_CIMConfigCommand_h

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Clients/cliutils/Command.h>
#include <Clients/cliutils/CommandException.h>
#include <iostream>

PEGASUS_NAMESPACE_BEGIN

/**
This is a configuration CLI used to manage the configuration of the
CIM Server.  This command supports operations to get, set, unset,
and list properties.  The APIs in the ConfigFile class are used to
access the CIM Server configuration file.

@author Nag Boranna, Hewlett-Packard Company
*/

class CIMConfigCommand : public Command
{

public:

    /**
        Constructs a CIMConfigCommand and initializes instance variables.
    */
    CIMConfigCommand ();

    /**
    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.

    @param   args  the string array containing the command line arguments
    @param   argc  the int containing the arguments count

    @throws  CommandFormatException  if an error is encountered in parsing
                                     the command line
    */
    void setCommand (Uint32 argc, char* argv []);

    /**
    Executes the command and writes the results to the output streams.

    @param ostream    The stream to which command output is written.
    @param ostream    The stream to which command errors are written.
    @return  0      if the command is successful
             1      if an error occurs in executing the command
    */
    Uint32 execute (PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter);


private:

    /**
    Get property values for the specified property from the CIM Server.

    @param ostream        The stream to which command output is written.
    @param ostream        The stream to which command errors are written.
    @param propertyName The name of the property to get.
    @param propertyValues A string array to store the values.

    @exception CIMException  if failed to get the properties from CIMOM.
    */
    void _getPropertiesFromCIMServer
    (
        PEGASUS_STD(ostream)&    outPrintWriter,
        PEGASUS_STD(ostream)&    errPrintWriter,
        const CIMName&           propertyName,
        Array <String>&          propertyValues
    );

    /**
    Send an updated property value to the CIM Server.

    @param ostream          The stream to which command output is written.
    @param ostream          The stream to which command errors are written.
    @param propertyName   The name of the property to update.
    @param propertyValue  The new value of the property.
    @param isUnsetOperation Specifies whether the property should be updated
                            or unset.

    @exception CIMException  if failed to get the properties from CIMOM.
    */
    void _updatePropertyInCIMServer
    (
        PEGASUS_STD(ostream)&    outPrintWriter,
        PEGASUS_STD(ostream)&    errPrintWriter,
        const CIMName&           propertyName,
        const String&            propertyValue,
        Boolean                  isUnsetOperation
    );

    /**
    Get a list of all property names and value from the CIM Server.

    @param propertyNames   The string array to store property names.
    @param propertyValues  The string array to store property values.
    */
    void _listAllPropertiesInCIMServer
    (
        ostream&    outPrintWriter,
        ostream&    errPrintWriter,
        Array <CIMName>&  propertyNames,
        Array <String>&   propertyValues
    );

    /* Get the best possible list of property names, from the server
        from the server if possible else from the config files
    */
    Boolean _getConfigPropertyNames(
        Array <CIMName>&  propertyNames, Boolean connected);

    /**
        Configuration File handler
    */
    AutoPtr<ConfigFileHandler> _configFileHandler;

    /**
    The CIM Client reference
    */
    AutoPtr<CIMClient> _client;

    /**
    The host name.
    */
    String        _hostName;

    /**
    The name of a config property.
    */
    CIMName       _propertyName;

    /**
    The value of a config property.
    */
    String        _propertyValue;

    /**
    The flag to indicate whether the default value is set or not.
    */
    Boolean       _defaultValueSet;

    /**
    The flag to indicate whether the current value is set or not.
    */
    Boolean       _currentValueSet;

    /**
    The flag to indicate whether the planned value is set or not.
    */
    Boolean       _plannedValueSet;

    /**
    The timeout value for set/unset operations.
    */
    Uint32        _timeoutSeconds;

#ifdef PEGASUS_OS_PASE
    /**
    The flag to indicate whether to disable any output.
    */
    Boolean      _defaultQuietSet;
#endif

    /**
    The type of operation specified on the command line.
    */
    Uint32        _operationType;

    String usage;

};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMConfigCommand_h */
