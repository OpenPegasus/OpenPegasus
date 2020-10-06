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

//
// implementation of cimmofMessages class

#include "cimmofMessages.h"
#include <cstdio>
#include <iostream> // for debug only
#include <Pegasus/Common/MessageLoader.h>

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

const cimmofMessages::arglist cimmofMessages::EMPTYLIST;

static const char* _cimmofMessages[] =
{
    "OK", //CIM_ERR_SUCCESS
    "$0:$1: $2 before '$3'", //PARSER_SYNTAX_ERROR
    "Error applying parameter $0 to method $1: $2", //APPLY_PARAMETER_ERROR
    "Error creating new CIMParameter object $0: $1", //NEW_PARAMETER_ERROR
    //UNINTIALIZED_PARAMETER_ERROR
    "Internal Error: Uninitialized parameter handle $0 in class $1",
    //METHOD_ALREADY_EXISTS_WARNING
    "Warning: CIMMethod $0 already exists for CIMClass $1",
    "Error applying method $0 to CIMClass $1: $2:", //APPLY_METHOD_ERROR
    "Error creating new CIMMethod $0: $1", //NEW_METHOD_ERROR
    "ADD QUALIFIER: ", //ADD_QUALIFIER
    "[Trace]", //TRACE
    "Error adding qualifier declaration $0: $1", //ADD_QUALIFIER_DECL_ERROR
    "\tOK", //TAB_OK
    "Error creating new Qualifier $0: $1", //NEW_QUALIFIER_ERROR
    "Error adding new Qualifier $0: $1", //ADD_QUALIFIER_ERROR
    //NEW_QUALIFIER_DECLARATION_ERROR
    "Error creating new Qualifier Declaration $0: $1",
    // GET_QUALIFIER_DECL_ERROR
    "Could not find declaration for Qualifier named $0",
    "ADD INSTANCE: ",  //ADD_INSTANCE
    //UNINITIALIZED_PROPERTY_ERROR
    "Internal Error: Uninitialized parameter $1 in class $0",
    //PROPERTY_ALREADY_EXISTS_WARNING
    "Warning:  Property $1 already exists in class $0",
    "Error applying property $1 to class $0: $2", //APPLYING_PROPERTY_ERROR
    "Error creating new Property $0: $1", //NEW_PROPERTY_ERROR
    "Error creating new Class Declaration $0: $1", //NEW_CLASS_ERROR
    "ADD CLASS: ", //ADD_CLASS
    "Warning:  Class $0 already exists in the repository",//CLASS_EXISTS_WARNING
    "Error adding class $0 to the repository: $1", //ADD_CLASS_ERROR
    //SETREPOSITORY_NO_COMPILER_OPTIONS
    "Internal Error: Compiler options not set before setting repository",
    //SETREPOSITORY_BLANK_NAME
    "Internal Error: No repository name was specified to setRepository",
    //NAMESPACE_CREATE_ERROR
    "Error trying to create repository name space $0: $1",
    // REPOSITORY_CREATE_ERROR
    "Error trying to create Repository in path $0: $1",
    // NEW_REFERENCE_ERROR
    "Error trying to create a reference to object $0: $1",
    //FIND_CLASS_OF_INSTANCE_ERROR
    "Error looking for class of current instance while looking up property"
        " $0: $1",
    // FIND_PROPERTY_VALUE_ERROR
    "Error looking up value of property $2 in class $1 (namespace $0): $3",
    "Error cloning property $0: $1",            // CLONING_PROPERTY_ERROR
    // APPLY_INSTANCE_PROPERTY_ERROR
    "Error applying property $0 to an instance of class $1: $2",
    // GET_INSTANCE_PROPERTY_ERROR
    "Error getting property $0 from an instance: $1",
    "Error getting class $0 from namespace $1: $2", //GET_CLASS_ERROR
    "Error getting value of property $0: $1", //GET_PROPERTY_VALUE_ERROR
    "Error creating new instance of class $0: $1", // NEW_INSTANCE_ERROR
    // INSTANCE_PROPERTY_EXISTS_WARNING
    "Warning: property $1 already exists for this instance of class $0",
    // INSTANCE_EXISTS_WARNING
    "Warning: the instance already exists.\nIn this"
        " implementation, that means it cannot be changed.",
    "Error adding an instance: $0", // ADD_INSTANCE_ERROR
    "Error: $0", // GENERAL_ERROR
    "Warning:  Class $0 was not added or updated: $1",     //CLASS_NOT_UPDATED
    "Class has the same version",                          //SAME_VERSION
    // NO_EXPERIMENTAL_UPDATE
    "Experimental update not allowed (set appropriate compiler option)",
    //NO_VERSION_UPDATE
    "Version update not allowed (set appropriate compiler option)",
    //NO_CLASS_UPDATE
    "Class update not allowed (set appropriate compiler option)",
    //INVALID_VERSION_FORMAT
    "Invalid version format in mof class or repository class"
        " (valid format is m.n.u)", //INVALID_VERSION_FORMAT
    "Invalid $0 value: $1",    // INVALID_LITERAL_VALUE
    "Class alias $0 found for class $1. This function not implemented",
    "Class $1 not found for instance alias $0",
    "Warning: Pragma \"$0\" not implemented by the compiler. Ignored",
    "Warning: Pragma locale value \"$0\" not defined by compiler. Ignored",
    ""
};

static const char* _cimmofMessagesKeys [] =
{
    "Compiler.cimmofMessages.CIM_ERR_SUCCESS",
    "Compiler.cimmofMessages.PARSER_SYNTAX_ERROR",
    "Compiler.cimmofMessages.APPLY_PARAMETER_ERROR",
    "Compiler.cimmofMessages.NEW_PARAMETER_ERROR",
    "Compiler.cimmofMessages.UNINITIALIZED_PARAMETER_ERROR",
    "Compiler.cimmofMessages.METHOD_ALREADY_EXISTS_WARNING",
    "Compiler.cimmofMessages.APPLY_METHOD_ERROR",
    "Compiler.cimmofMessages.NEW_METHOD_ERROR",
    "Compiler.cimmofMessages.ADD_QUALIFIER",
    "Compiler.cimmofMessages.TRACE",
    "Compiler.cimmofMessages.ADD_QUALIFIER_DECL_ERROR",
    "Compiler.cimmofMessages.TAB_OK",
    "Compiler.cimmofMessages.NEW_QUALIFIER_ERROR",
    "Compiler.cimmofMessages.ADD_QUALIFIER_ERROR",
    "Compiler.cimmofMessages.NEW_QUALIFIER_DECLARATION_ERROR",
    "Compiler.cimmofMessages.GET_QUALIFIER_DECL_ERROR",
    "Compiler.cimmofMessages.ADD_INSTANCE",
    "Compiler.cimmofMessages.UNINITIALIZED_PROPERTY_ERROR",
    "Compiler.cimmofMessages.PROPERTY_ALREADY_EXISTS_WARNING",
    "Compiler.cimmofMessages.APPLYING_PROPERTY_ERROR",
    "Compiler.cimmofMessages.NEW_PROPERTY_ERROR",
    "Compiler.cimmofMessages.NEW_CLASS_ERROR",
    "Compiler.cimmofMessages.ADD_CLASS",
    "Compiler.cimmofMessages.CLASS_EXISTS_WARNING",
    "Compiler.cimmofMessages.ADD_CLASS_ERROR",
    "Compiler.cimmofMessages.SETREPOSITORY_NO_COMPILER_OPTIONS",
    "Compiler.cimmofMessages.SETREPOSITORY_BLANK_NAME",
    "Compiler.cimmofMessages.NAMESPACE_CREATE_ERROR",
    "Compiler.cimmofMessages.REPOSITORY_CREATE_ERROR",
    "Compiler.cimmofMessages.NEW_REFERENCE_ERROR",
    "Compiler.cimmofMessages.FIND_CLASS_OF_INSTANCE_ERROR",
    "Compiler.cimmofMessages.FIND_PROPERTY_VALUE_ERROR",
    "Compiler.cimmofMessages.CLONING_PROPERTY_ERROR",
    "Compiler.cimmofMessages.APPLY_INSTANCE_PROPERTY_ERROR",
    "Compiler.cimmofMessages.GET_INSTANCE_PROPERTY_ERROR",
    "Compiler.cimmofMessages.GET_CLASS_ERROR",
    "Compiler.cimmofMessages.GET_PROPERTY_VALUE_ERROR",
    "Compiler.cimmofMessages.NEW_INSTANCE_ERROR",
    "Compiler.cimmofMessages.INSTANCE_PROPERTY_EXISTS_WARNING",
    "Compiler.cimmofMessages.INSTANCE_EXISTS_WARNING",
    "Compiler.cimmofMessages.ADD_INSTANCE_ERROR",
    "Compiler.cimmofMessages.GENERAL_ERROR",
    "Compiler.cimmofMessages.CLASS_NOT_UPDATED",
    "Compiler.cimmofMessages.SAME_VERSION",
    "Compiler.cimmofMessages.NO_EXPERIMENTAL_UPDATE",
    "Compiler.cimmofMessages.NO_VERSION_UPDATE",
    "Compiler.cimmofMessages.NO_CLASS_UPDATE",
    "Compiler.cimmofMessages.INVALID_VERSION_FORMAT",
    "Compiler.cimmofMessages.INVALID_LITERAL_VALUE",
    "Compiler.cimmofMessages::CLASS_ALIAS_FOUND",
    "Compiler.cimmofMessages::INSTANCE_ALIAS_CLASS_NOT_FOUND",
    "Compiler.cimmofMessages::PRAGMA_NOT_IMPLEMENTED_WARNING",
    "Compiler.commofMessages::LOCALE_NOT_IMPLEMENTED_WARNING",
    "Compiler.cimmofMessages.END"
};


const char* cimmofMessages::msgCodeToString(MsgCode code)
{
    return _cimmofMessages[(unsigned int)code];
}

void cimmofMessages::getMessage(String &out, MsgCode code, const arglist &args)
{
    Array<String> _args;
    for (unsigned int i = 0; i < 10; i++)
    {
        if(i < args.size())
            _args.append(args[i]);
        else
            _args.append("");
    }

    MessageLoaderParms parms(_cimmofMessagesKeys[(unsigned int)code],
        _cimmofMessages[(unsigned int)code],
        _args[0],_args[1],_args[2],_args[3],_args[4],
        _args[5],_args[6],_args[7],_args[8],_args[9]);

    out = MessageLoader::getMessage(parms);
}

PEGASUS_NAMESPACE_END
