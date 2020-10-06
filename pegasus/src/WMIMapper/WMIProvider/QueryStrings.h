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
// Author: Barbara Packard (barbara_packard@hp.com)
//
// Modified By:
//
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_WMIQueryStrings_h
#define Pegasus_WMIQueryStrings_h

/** This file contains definitions of string constants used to define queries
    that will be used to process the Associator and Reference requests.
*/

PEGASUS_NAMESPACE_BEGIN

#define qString(x)        String(QUERY_STRING_LIST[x])
#define qChar(x)        QUERY_STRING_LIST[x]

static char const* QUERY_STRING_LIST[] =
{
    "ASSOCIATORS OF {%s}" ,
    " WHERE",
    " ClassDefsOnly",
    " AssocClass = ",
    " RequiredAssocQualifier = ",
    " RequiredQualifier = ",
    " ResultClass = ",
    " ResultRole = ",
    " Role = ",
    " SchemaOnly",
    "REFERENCES OF {%s}",
    "WQL",
    ":",
    ".",
    "=R\"",
    "\\\"",
    "\"",
    "/"
};

// Constants identifying the strings

enum QUERY_STRING_ID
{
    Q_ASSOCIATORS,
    Q_WHERE,
    Q_CLS_DEFS,
    Q_ASSOC_CLS,
    Q_REQ_ASSOC_QUAL,
    Q_REQ_QUAL,
    Q_RESULT_CLASS,
    Q_RESULT_ROLE,
    Q_ROLE,
    Q_SCHEMA,
    Q_REFERENCES,
    Q_WQL,
    Q_COLON,
    Q_PERIOD,
    Q_REF_KEY,
    Q_SLASH_QUOTE,
    Q_QUOTE,
    Q_SLASH

};



PEGASUS_NAMESPACE_END

#endif /* Pegasus_WMIQueryStrings_h */
