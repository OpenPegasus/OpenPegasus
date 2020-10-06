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

#include "QueryException.h"

PEGASUS_NAMESPACE_BEGIN

const char QueryException::MSG[] = "Query error: $0";
const char QueryException::KEY[] = "QueryCommon.QueryException.QUERY_EXCEPTION";

const char QueryLanguageInvalidException::MSG[] =
    "Query invalid language error: $0";
const char QueryLanguageInvalidException::KEY[] =
    "QueryCommon.QueryException.QUERY_LANGUAGE_INVALID_EXCEPTION";

const char QueryParseException::MSG[] = "Query parse error: $0";
const char QueryParseException::KEY[] =
    "QueryCommon.QueryException.QUERY_PARSE_EXCEPTION";

const char QueryValidationException::MSG[] = "Query validation error: $0";
const char QueryValidationException::KEY[] =
    "QueryCommon.QueryException.QUERY_VALIDATION_EXCEPTION";

const char QueryRuntimeException::MSG[] = "Query runtime error: $0";
const char QueryRuntimeException::KEY[] =
    "QueryCommon.QueryException.QUERY_RUNTIME_EXCEPTION";

const char QueryRuntimePropertyException::MSG[] = "Query property error: $0";
const char QueryRuntimePropertyException::KEY[] =
    "QueryCommon.QueryException.QUERY_RUNTIME_PROPERTY_EXCEPTION";

const char QueryMissingPropertyException::MSG[] =
    "Query missing property error: $0";
const char QueryMissingPropertyException::KEY[] =
    "QueryCommon.QueryException.QUERY_MISSINGPROPERTY_EXCEPTION";

const char CQLChainedIdParseException::MSG[] =
    "CQL chained identifier parse error: $0";
const char CQLChainedIdParseException::KEY[] =
    "QueryCommon.QueryException.CQL_CHAINED_ID_EXCEPTION";

const char CQLIdentifierParseException::MSG[] =
    "CQL identifier parse error: $0";
const char CQLIdentifierParseException::KEY[] =
    "QueryCommon.QueryException.CQL_IDENTIFIER_EXCEPTION";

const char CQLRuntimeException::MSG[] = "CQL runtime error: $0";
const char CQLRuntimeException::KEY[] =
    "QueryCommon.QueryException.CQL_RUNTIME_EXCEPTION";

const char CQLSyntaxErrorException::MSG[] =
    "CQL syntax error: $0 around token $1 in position $2 while processing "
        "rule $3";
const char CQLSyntaxErrorException::KEY[] =
    "QueryCommon.QueryException.CQL_SYNTAX_ERROR_EXCEPTION";

PEGASUS_NAMESPACE_END
