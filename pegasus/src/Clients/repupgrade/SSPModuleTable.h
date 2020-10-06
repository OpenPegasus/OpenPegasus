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

#ifndef Pegasus_SSPModuleTable_h
#define Pegasus_SSPModuleTable_h

#include "SSPModule.h"

PEGASUS_NAMESPACE_BEGIN

class SSPModuleTable
{

public:

    /**

    Contains Schema Special Process module info:

    moduleName : Name of the Module

    moduleType : Defines the type of the schema element processed by the module.

                 The choices are :
                       (1) 'c' : Processes a CIMClass type.
                       (2) 'q' : Processes a CIMQualifier type.
                       (3) 'i' : Processes a CIMInstance type.
                       (4) 'a' : Processes all types (CIMClass,CIMInstance &
                                                      CIMQualifier)

    */
    struct SchemaSpecialProcessModuleTable
    {
        const char* moduleName;
        const char*  moduleType;
    };


   //
   // Contains the count of number of modules.
   //
    static const Uint32 NUM_MODULES;
};

//
//    Enables module processing. If there are any modules defined in the
//    module table below, then the following must be uncommented.
//
// #define ENABLE_MODULE_PROCESSING

//
//    List of Special Process Modules.
//
//    A sample module entry "pegsamplesspmodule" has been included below.
//    Sample module is located under SSPModule/SampleSSPModule directory.
//    The sample module shows a sample implementation of the processClass
//    and processInstance methods.
//
//    Steps for adding a module:
//    1. Add an entry for the module in the structure below.
//    2. Uncomment the ENABLE_MODULE_PROCESSING macro defined above.
//
//    NOTE: If no modules are defined the ENABLE_MODULE_PROCESSING macro
//          define must be commented out.
//
//
#ifdef ENABLE_MODULE_PROCESSING
static struct SSPModuleTable::SchemaSpecialProcessModuleTable
    schemaProcessingModules[] =
{
//    {"pegsamplesspmodule",   "a"},
};
#endif

//
// Contains the count of number of modules.
//
#ifdef ENABLE_MODULE_PROCESSING
const Uint32 SSPModuleTable::NUM_MODULES =
          sizeof(schemaProcessingModules) / sizeof(schemaProcessingModules[0]);
#endif

PEGASUS_NAMESPACE_END

#endif /* Pegasus_SSPModuleTable_h */
