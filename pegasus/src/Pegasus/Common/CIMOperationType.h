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

#ifndef Pegasus_CIMOperationType_h
#define Pegasus_CIMOperationType_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    The CIMOperationType enumeration represents the CIM operation types
    defined in the DMTF Specification for CIM Operations over HTTP.
*/
enum CIMOperationType
{
    CIMOPTYPE_INVOKE_METHOD = 0,
    CIMOPTYPE_GET_CLASS = 1,
    CIMOPTYPE_GET_INSTANCE = 2,
    CIMOPTYPE_DELETE_CLASS = 3,
    CIMOPTYPE_DELETE_INSTANCE = 4,
    CIMOPTYPE_CREATE_CLASS = 5,
    CIMOPTYPE_CREATE_INSTANCE = 6,
    CIMOPTYPE_MODIFY_CLASS = 7,
    CIMOPTYPE_MODIFY_INSTANCE = 8,
    CIMOPTYPE_ENUMERATE_CLASSES = 9,
    CIMOPTYPE_ENUMERATE_CLASS_NAMES = 10,
    CIMOPTYPE_ENUMERATE_INSTANCES = 11,
    CIMOPTYPE_ENUMERATE_INSTANCE_NAMES = 12,
    CIMOPTYPE_EXEC_QUERY = 13,
    CIMOPTYPE_ASSOCIATORS = 14,
    CIMOPTYPE_ASSOCIATOR_NAMES = 15,
    CIMOPTYPE_REFERENCES = 16,
    CIMOPTYPE_REFERENCE_NAMES = 17,
    CIMOPTYPE_GET_PROPERTY = 18,
    CIMOPTYPE_SET_PROPERTY = 19,
    CIMOPTYPE_GET_QUALIFIER = 20,
    CIMOPTYPE_SET_QUALIFIER = 21,
    CIMOPTYPE_DELETE_QUALIFIER = 22,
    CIMOPTYPE_ENUMERATE_QUALIFIERS = 23,
// EXP_PULL_BEGIN
    CIMOPTYPE_OPEN_ENUMERATE_INSTANCES = 24,
    CIMOPTYPE_OPEN_ENUMERATE_INSTANCE_PATHS = 25,
    CIMOPTYPE_OPEN_ASSOCIATOR_INSTANCES = 26,
    CIMOPTYPE_OPEN_ASSOCIATOR_INSTANCE_PATHS = 27,
    CIMOPTYPE_OPEN_REFERENCE_INSTANCES = 28,
    CIMOPTYPE_OPEN_REFERENCE_INSTANCE_PATHS = 29,
    CIMOPTYPE_OPEN_QUERY_INSTANCES = 30,
    CIMOPTYPE_PULL_INSTANCES_WITH_PATH = 31,
    CIMOPTYPE_PULL_INSTANCE_PATHS = 32,
    CIMOPTYPE_PULL_INSTANCES = 33,
    CIMOPTYPE_CLOSE_ENUMERATION = 34,
    CIMOPTYPE_ENUMERATION_COUNT = 35
//EXP_PULL_END
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMOperationType_h */
