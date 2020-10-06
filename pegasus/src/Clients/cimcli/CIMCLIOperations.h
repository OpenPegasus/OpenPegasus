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

/*
    Definition of the operations possible with cimcli. An operation is
    defined by a cimcli operation name (the first parameter on the command
    line input) and an operation function in CIMCLIClient.
    The structures below and their corresponding tables in CIMCLIOperations.cpp
    define the names, characteristics, usage, etc. for all operations
    in cimcli.
*/
#ifndef _CLI_OPERATIONS_H
#define _CLI_OPERATIONS_H

#include <Pegasus/Common/Config.h>
#include <Clients/cimcli/Linkage.h>
#include <Pegasus/Common/PegasusAssert.h>

#include <Pegasus/General/Stopwatch.h>

PEGASUS_NAMESPACE_BEGIN

/*
    Definition of the cimcli Operations (Second Parameter on input).
*/

// Enumeration of all possible operations
enum OperationID
{
    ID_EnumerateInstanceNames = 1,
    ID_EnumerateInstances        ,
    ID_GetClass                  ,
    ID_GetInstance               ,
    ID_CreateInstance            ,
    ID_DeleteInstance            ,
    ID_CreateClass               ,
    ID_DeleteClass               ,
    ID_EnumerateClasses          ,
    ID_EnumerateClassNames       ,
    ID_GetProperty               ,
    ID_SetProperty               ,
    ID_GetQualifier              ,
    ID_SetQualifier              ,
    ID_EnumerateQualifiers       ,
    ID_DeleteQualifier           ,
    ID_Associators               ,
    ID_AssociatorNames           ,
    ID_References                ,
    ID_ReferenceNames            ,
    ID_InvokeMethod              ,
    ID_ExecQuery                 ,
    ID_ModifyClass               ,
    ID_ModifyInstance            ,
    ID_EnumerateAllInstanceNames ,
    ID_EnumerateNamespaces       ,
    ID_ShowOptions               ,
    ID_StatisticsOn              ,
    ID_StatisticsOff             ,
    ID_TestInstance              ,
//EXP_PULL_BEGIN
    ID_PullEnumerateInstances    ,
    ID_PullEnumerateInstancePaths,
    ID_PullReferenceInstances    ,
    ID_PullReferenceInstancePaths,
    ID_PullAssociatorInstances   ,
    ID_PullAssociatorInstancePaths,
    ID_PullQueryInstances,
//EXP_PULL_END
    ID_CountInstances            ,
    ID_ClassTree
//
//ID_Profile                   ,
};
/*
    Structure for the Operation Table.  This table is used to define the
    allowed input operations (The initial parameter on the command line)
    The table of commands is defined in CIMCLIOperations.cpp.
*/
struct  OPERATION_TABLE_ENTRY
{
    OperationID ID_Operation;
    const char* OperationName;  // Name for the operation. Used as operation
                                // name (long name) and link to OptionManager
    int minNumArgs;             // minimum number of input args to accept cmd
    const char* ShortCut;       // shortcut operation name (nc for classNames)
    const char* msgKey;         //Message key for globalization
    const char* UsageText;      // Simple usage text
};

/*
    today there is a second table with a single entry corresponding to each
    entry in the OperationTable.  This contains additional text and message
    keys for the operation
    NOTE: This should really be a single table but since this secondary table
    was built instead of expanding the operation table it is left that way.
    Also this makes the operation table a bit easier to understand since the
    table is smaller.
*/
typedef struct OPERATION_TABLE_ENTRY OperationTableEntry;

struct OPERATION_EXAMPLE_ENTRY
{
    const char* msgKey;   //Message key for globalization of example string
    const char* Example;  //Example String
    const char* optionsKey; //Message key for options supported string
    const char* Options;  //Options supported
};

typedef struct OPERATION_EXAMPLE_ENTRY OperationExampleEntry;

/////////////////////////////////////////////////////////////////////////////
/***************** Command / Operation table processing ********************
    Class to process the CommandTable which defines all of the
    cimcli operations.
    This class provides for two types of operations (find followed by
    get) to get a particular command and more() next() to iterate the
    complete table.
    This allows cimcli to process the operations and examples table without
    knowledge other than  the two structs above and the methods in this
    class
*/

class PEGASUS_CLI_LINKAGE Operations
{
public:
    Operations();

    // find the operation defined by  opString.  Return true if found
    // or false if not found.
    Boolean find(const String& opString);

    // return the current position in the table. Normally follows
    // the find command to get the entry found.
    OperationTableEntry get();

    // get the Index for the current entry.  This is required only because
    // there are two tables today that use the index (the command table
    // and the examples table.
    Uint32 getIndex();

    // Determine if there are more entries in the table. Returns true
    // if there are more entries in the table
    Boolean more();

    // get the current entry in the table and increment the internal
    // pointer to the next entry.
    OperationTableEntry next();

    // get the current example table entry
    OperationExampleEntry getExampleEntry();
private:
    Uint32 _index;
};


PEGASUS_NAMESPACE_END

#endif

