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
/* NOCHKSRC */

//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_CIMStatusCode_h
#define Pegasus_CIMStatusCode_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/ContentLanguageList.h>

PEGASUS_NAMESPACE_BEGIN

/**
    The CIMStatusCode enumeration represents the status codes defined in the
    DMTF Specification for CIM Operations over HTTP.
*/
enum CIMStatusCode
{
    /**
        The operation was successful.
    */
    CIM_ERR_SUCCESS = 0,

    /**
        A general error occurred that is not covered by a more specific
        error code.
    */
    CIM_ERR_FAILED = 1,

    /**
        Access to a CIM resource was not available to the client.
    */
    CIM_ERR_ACCESS_DENIED = 2,

    /**
        The target namespace does not exist.
    */
    CIM_ERR_INVALID_NAMESPACE = 3,

    /**
        One or more parameter values passed to the method were invalid.
    */
    CIM_ERR_INVALID_PARAMETER = 4,

    /**
        The specified class does not exist.
    */
    CIM_ERR_INVALID_CLASS = 5,

    /**
        The requested object could not be found.
    */
    CIM_ERR_NOT_FOUND = 6,

    /**
        The requested operation is not supported.
    */
    CIM_ERR_NOT_SUPPORTED = 7,

    /**
        Operation cannot be carried out on this class since it has subclasses.
    */
    CIM_ERR_CLASS_HAS_CHILDREN = 8,

    /**
        Operation cannot be carried out on this class since it has instances.
    */
    CIM_ERR_CLASS_HAS_INSTANCES = 9,

    /**
        Operation cannot be carried out since the specified superClass does
        not exist.
    */
    CIM_ERR_INVALID_SUPERCLASS = 10,

    /**
        Operation cannot be carried out because an object already exists.
    */
    CIM_ERR_ALREADY_EXISTS = 11,

    /**
        The specified property does not exist:
    */
    CIM_ERR_NO_SUCH_PROPERTY = 12,

    /**
        The value supplied is incompatible with the type.
    */
    CIM_ERR_TYPE_MISMATCH = 13,

    /**
        The query language is not recognized or supported.
    */
    CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED = 14,

    /**
        The query is not valid for the specified query language.
    */
    CIM_ERR_INVALID_QUERY = 15,

    /**
        The extrinsic method could not be executed.
    */
    CIM_ERR_METHOD_NOT_AVAILABLE = 16,

    /**
        The specified extrinsic method does not exist.
    */
    CIM_ERR_METHOD_NOT_FOUND = 17
};

PEGASUS_COMMON_LINKAGE const char* cimStatusCodeToString(CIMStatusCode code);

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
/**
    <I><B>Experimental Interface</B></I><BR>
*/
PEGASUS_COMMON_LINKAGE String cimStatusCodeToString(
    CIMStatusCode code,
    const ContentLanguageList &contentLanguages);

/**
    <I><B>Experimental Interface</B></I><BR>
*/
PEGASUS_COMMON_LINKAGE ContentLanguageList cimStatusCodeToString_Thread(
    String & message,
    CIMStatusCode code);
#endif  //    PEGASUS_USE_EXPERIMENTAL_INTERFACES			

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMStatusCode_h */
