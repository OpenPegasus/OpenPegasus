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

#ifndef Pegasus_WBEMSLPTemplate_h
#define Pegasus_WBEMSLPTemplate_h

/** DEFINES for well-known attribute names and delimiters.
    Notice that there is a section for adding custom attributes and
    a place to add items from the WBEM SLP Template.  Only "literals"
    should be added to this header
*/

// The following are literals for SLP wire format handling
#define PEG_SLP_ATTR_BEGIN                '('
#define PEG_SLP_ATTR_END                  ')'
#define PEG_SLP_ATTR_DELIMITER            '='
#define PEG_SLP_ATTR_SEPARATOR            ','

// The following are literals for PEGASUS custom attributes
#define PEG_CUSTOM_ATTR_HOST              "host"
#define PEG_CUSTOM_ATTR_PORT              "port"

// The following are literals in the WBEM SLP Template
//#define PEG_WBEM_SLP_URL_DELIMITER        ':'
#define PEG_WBEM_SLP_TYPE                 "wbem"
#define PEG_WBEM_SLP_SERVICE_ID           "service-id"

// DEFAULTS
#define PEG_WBEM_SLP_SERVICE_ID_DEFAULT   "cimserver"

#endif /* Pegasus_WBEMSLPTemplate_h */
