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

#include <Pegasus/Common/MessageLoader.h>
#include "CIMStatusCode.h"

PEGASUS_NAMESPACE_BEGIN

static const char* _cimMessages[] =
{
    "CIM_ERR_SUCCESS",
    "CIM_ERR_FAILED",
    "CIM_ERR_ACCESS_DENIED",
    "CIM_ERR_INVALID_NAMESPACE",
    "CIM_ERR_INVALID_PARAMETER",
    "CIM_ERR_INVALID_CLASS",
    "CIM_ERR_NOT_FOUND",
    "CIM_ERR_NOT_SUPPORTED",
    "CIM_ERR_CLASS_HAS_CHILDREN",
    "CIM_ERR_CLASS_HAS_INSTANCES",
    "CIM_ERR_INVALID_SUPERCLASS",
    "CIM_ERR_ALREADY_EXISTS",
    "CIM_ERR_NO_SUCH_PROPERTY",
    "CIM_ERR_TYPE_MISMATCH",
    "CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED",
    "CIM_ERR_INVALID_QUERY",
    "CIM_ERR_METHOD_NOT_AVAILABLE",
    "CIM_ERR_METHOD_NOT_FOUND",
//EXP_PULL_BEGIN     The DMTF DSP0200 v 1.4 spec added the following
    "empty",
    "empty",
    "CIM_ERR_NAMESPACE_NOT_EMPTY",
    "CIM_ERR_INVALID_ENUMERATION_CONTEXT",
    "CIM_ERR_INVALID_OPERATION_TIMEOUT",
    "CIM_ERR_PULL_HAS_BEEN_ABANDONED",
    "CIM_ERR_PULL_CANNOT_BE_ABANDONED",
    "CIM_ERR_FILTERED_ENUMERATION_NOT_SUPPORTED",
    "CIM_ERR_CONTINUATION_ON_ERROR_NOT_SUPPORTED",
    "CIM_ERR_SERVER_LIMITS_EXCEEDED",
    "CIM_ERR_SERVER_IS_SHUTTING_DOWN"
//EXP_PULL_END
};

// l10n TODO - the first func should go away when all Pegasus is globalized

const char* cimStatusCodeToString(CIMStatusCode code)
{
    if (Uint32(code) < (sizeof(_cimMessages)/sizeof(_cimMessages[0])))
    {
        return _cimMessages[Uint32(code)];
    }

    return "Unrecognized CIM status code";
}


String cimStatusCodeToString(
    CIMStatusCode code,
    const ContentLanguageList& contentLanguages)
{
    if (Uint32(code) < (sizeof(_cimMessages)/sizeof(_cimMessages[0])))
    {
        return _cimMessages[Uint32(code)];
    }

    MessageLoaderParms parms(
        "Common.CIMStatusCode.UNRECOGNIZED_STATUS_CODE",
        "Unrecognized CIM status code \"$0\"", (Uint32)code);

    if (contentLanguages.size() > 0)
    {
        //build AcceptLanguageList from contentLanguages, use in getMessage
        parms.acceptlanguages.clear();
        parms.acceptlanguages.insert(contentLanguages.getLanguageTag(0), 1.0);
    }

    return MessageLoader::getMessage(parms);
}

ContentLanguageList cimStatusCodeToString_Thread(
    String & message,
    CIMStatusCode code)
{
    if (Uint32(code) < (sizeof(_cimMessages)/sizeof(_cimMessages[0])))
    {
        message = _cimMessages[Uint32(code)];
        return ContentLanguageList();
    }

    MessageLoaderParms parms(
        "Common.CIMStatusCode.UNRECOGNIZED_STATUS_CODE",
        "Unrecognized CIM status code \"$0\"", (Uint32)code);

    //parms.useThreadLocale = true;
    message = MessageLoader::getMessage(parms);
    return parms.contentlanguages;
}

PEGASUS_NAMESPACE_END
