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


#ifndef _CIMMOFMESSAGES_H_
#define _CIMMOFMESSAGES_H_

//
// cimmofMessages:  an interface that hides the underlying
// internationalization implementation for the CIM MOF compiler.
// Any strings that the compiler needs come out of this interface.
// When we settle on an MSB internationalization method, the
// strings will be translated for the locale.

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/ArrayInternal.h>

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

class cimmofMessages
{
    public:
        enum MsgCode
        {
            CIM_ERR_SUCCESS = 0,
            PARSER_SYNTAX_ERROR,
            APPLY_PARAMETER_ERROR,
            NEW_PARAMETER_ERROR,
            UNINITIALIZED_PARAMETER_ERROR,
            METHOD_ALREADY_EXISTS_WARNING,
            APPLY_METHOD_ERROR,
            NEW_METHOD_ERROR,
            ADD_QUALIFIER,
            TRACE,
            ADD_QUALIFIER_DECL_ERROR,
            TAB_OK,
            NEW_QUALIFIER_ERROR,
            ADD_QUALIFIER_ERROR,
            NEW_QUALIFIER_DECLARATION_ERROR,
            GET_QUALIFIER_DECL_ERROR,
            ADD_INSTANCE,
            UNINITIALIZED_PROPERTY_ERROR,
            PROPERTY_ALREADY_EXISTS_WARNING,
            APPLYING_PROPERTY_ERROR,
            NEW_PROPERTY_ERROR,
            NEW_CLASS_ERROR,
            ADD_CLASS,
            CLASS_EXISTS_WARNING,
            ADD_CLASS_ERROR,
            SETREPOSITORY_NO_COMPILER_OPTIONS,
            SETREPOSITORY_BLANK_NAME,
            NAMESPACE_CREATE_ERROR,
            REPOSITORY_CREATE_ERROR,
            NEW_REFERENCE_ERROR,
            FIND_CLASS_OF_INSTANCE_ERROR,
            FIND_PROPERTY_VALUE_ERROR,
            CLONING_PROPERTY_ERROR,
            APPLY_INSTANCE_PROPERTY_ERROR,
            GET_INSTANCE_PROPERTY_ERROR,
            GET_CLASS_ERROR,
            GET_PROPERTY_VALUE_ERROR,
            NEW_INSTANCE_ERROR,
            INSTANCE_PROPERTY_EXISTS_WARNING,
            INSTANCE_EXISTS_WARNING,
            ADD_INSTANCE_ERROR,
            GENERAL_ERROR,
            CLASS_NOT_UPDATED,
            SAME_VERSION,
            NO_EXPERIMENTAL_UPDATE,
            NO_VERSION_UPDATE,
            NO_CLASS_UPDATE,
            INVALID_VERSION_FORMAT,
            INVALID_LITERAL_VALUE,
            CLASS_ALIAS_FOUND,
            INSTANCE_ALIAS_CLASS_NOT_FOUND,
            PRAGMA_NOT_IMPLEMENTED_WARNING,
            LOCALE_NOT_IMPLEMENTED_WARNING,
            END
        };
        typedef Array<String> arglist;
        static const arglist EMPTYLIST;

        static const char* msgCodeToString(MsgCode code);

        static void getMessage(String &out, MsgCode code,
            const arglist &al =
            cimmofMessages::EMPTYLIST);
};

PEGASUS_NAMESPACE_END

#endif
