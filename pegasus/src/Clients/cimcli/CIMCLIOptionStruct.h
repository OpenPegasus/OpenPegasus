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

#ifndef _CLI_CLIOPTIONSSTRUCT_H
#define _CLI_CLIOPTIONSSTRUCT_H

#include <Pegasus/Common/Config.h>
#include <Clients/cimcli/Linkage.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/General/Stopwatch.h>
#include <Pegasus/Client/CIMClient.h>
#include <Clients/cimcli/CIMCLIClient.h>

PEGASUS_NAMESPACE_BEGIN

/******************************************************************************
**
**          Data structure definitions for the Output Type
**          Defines a data type, enums for the various output types
**          and a table to map
**          The type strings are defined in CIMCLIOptionStruct.cpp
**
******************************************************************************/

// List of possible output types.  Type Strings are defined in
// CIMCLIOptionStruct.cpp and also there is a constanct string in
// CIMCLIOptions.cpp that defines the strings for help.
enum OutputType {
    OUTPUT_TYPE_ILLEGAL = 0,
    OUTPUT_XML   = 1,
    OUTPUT_MOF   = 2,
    OUTPUT_TEXT  = 3,
    OUTPUT_TABLE = 4
};

// Structure defining output type enum / string
struct  OutputTypeStruct
{
    OutputType OutputTypeValue;
    const char* OutputName;

    // Methods

    // Return Output type corresponding to input String or
    // OUTPUT_TYPE_ILLEGAL if not found.  This is a noCase
    // compare
    static OutputType getOutputType(String& OutputTypeName);
    // Return String containing all valid output types for display.
    // They are separated by comma/space
    static String listOutputTypes();

    static String getTypeStr(OutputType x);
};



/******************************************************************************
**
**          Data structure definitions for the input Options
**          Defines a data struct, and a typedef for the collection of
**          parameters that represent the result of cimcli input processing
**          The OptionStruct structure and functions manage the command
**          line input parameters and options and communicate this information
**          between the parsing mechanisms and the operation action functions.
**          One instance of this object is created and maintained for each
**          operation processed
**
******************************************************************************/

// The input options used by the individual commands. Note that
// Use of these varies by command.

struct  OptionStruct
{
    // Structure Methods - constructor
    // Sets up defaults for all variables in the structure
    OptionStruct();

    // TargetObject Processing Methods
    String getTargetObjectNameStr();
    CIMName getTargetObjectNameClassName();
    CIMObjectPath getTargetObjectName();

    // true if is class only (no keyBindings)
    Boolean targetObjectNameClassOnly();
    //
    // Variables used by cimcli operations functions
    //
    CIMClient client;           // CIMClient object for communciation
    String location;            // Host parameter
    String user;                // User name input parameter
    String password;            // password input parameter
#ifdef PEGASUS_HAS_SSL          // SSL input parameters
    Boolean ssl;
    String clientCert;          // client Certificate path
    String clientKey;           // client Key path
    String clientTruststore;    // Path name for a client truststore
#endif

    // The following variables represent the input Object Name parameter,
    // normally the second argument to the input command.
    String nameSpace;             // Input namespace
                                  //
    CIMName className;            // inputObjectName mapped to class name
    CIMObjectPath targetObjectName; // target Name for this request with
                                  // format validated by conversion
    String inputObjectName;       // Name to display as input target object on
                                  // error. Allows display whether class or
                                  // object input or error.

    String qualifierName;         // used if qualifier action function.

    String cimCmd;                // Command name

    // output format pamameter information
    String outputTypeParamStr;
    OutputType outputType;

    // CIM operation parameter input parameters
    Boolean deepInheritance;
    Boolean localOnly;
    Boolean includeQualifiersRequested;       // -iq parameter supplied
    Boolean notIncludeQualifiersRequested;    // -niq parameter supplied
    Boolean includeQualifiers;                // parameter used for call
    Boolean includeClassOrigin;
    String  newValue;

    CIMQualifierDecl qualifierDeclaration;

    String propertyName;
    CIMPropertyList propertyList;

    // Associaton/reference specific input parameters
    CIMName assocClass;
    CIMName resultClass;
    String role;
    String resultRole;

    String queryLanguage;
    String query;

    // InvokeMethod specific input parameters
    CIMName methodName;
    Array<CIMParamValue> inParams;

    // General parameters that modify output
    Boolean verboseTest;        // Verbose output during processing
    Boolean summary;            // Do summary information
    Uint32 delay;               // Delay between connect and command
    Uint32 trace;               // Trace cimcli
    Boolean debug;              // Execute cimcli in debug mode
    Boolean sort;               // Sort output of multi-object responses

    Uint32 expectedCount;       // Number of expected items if
                                // count test is to be executed.
    Boolean executeCountTest;   // Execute the count test if true
                                // Separate  from expectedCount
                                // because ANY value can exist
                                // for the count itself including zero.

    Uint32 repeat;              // execute the operation repeat times
    Boolean time;               // report time for operation

    Uint32 termCondition;       // Termination code put here by action function

    Uint32 connectionTimeout;    // Connection timeout in seconds
    Array<String> valueParams;   // additional param list. depends on command.
    Boolean interactive;         // use interactive mode.
    Boolean setRtnHostNames;    // Define whether we modify host names on
                                      // returned paths and references
    String rtnHostSubstituteName;   // Host name to set if we
                                 // do modify

    // Internal work variables
    Stopwatch elapsedTime;
    double saveElapsedTime;

//EXP_PULL_BEGIN  -- Add parameters for pull operations
    Uint32Arg pullOperationTimeout;
    Uint32 maxObjectCount;     // maxObjects to receive for pull operations
    Boolean continueOnError;      // ContinueOnError Flag for open... operations
    Uint32 maxObjectsToReceive;
    Uint32 pullDelay;
//EXP_PULL_END
};

typedef struct OptionStruct Options;

PEGASUS_NAMESPACE_END

#endif  // _CLI_CLIOPTIONSSTRUCT_H
