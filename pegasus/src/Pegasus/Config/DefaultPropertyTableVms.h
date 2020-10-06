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

#ifndef Pegasus_DefaultPropertyTableVms_h
#define Pegasus_DefaultPropertyTableVms_h

    {"httpPort", "", IS_STATIC, IS_VISIBLE},
    {"httpsPort", "", IS_STATIC, IS_VISIBLE},
#ifdef PEGASUS_USE_RELEASE_CONFIG_OPTIONS
    {"enableHttpConnection", "false", IS_STATIC, IS_VISIBLE},
    {"enableHttpsConnection", "true", IS_STATIC, IS_VISIBLE},
#else
    {"enableHttpConnection", "true", IS_STATIC, IS_VISIBLE},
    {"enableHttpsConnection", "false", IS_STATIC, IS_VISIBLE},
#endif
    {"daemon", "false", IS_STATIC, IS_VISIBLE},
#ifdef PEGASUS_ENABLE_SLP
    {"slp", "false", IS_STATIC, IS_VISIBLE},
#endif
    {"enableAssociationTraversal", "true", IS_STATIC, IS_VISIBLE},
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
    {"enableIndicationService", "true", IS_DYNAMIC, IS_VISIBLE},
#else
    {"enableIndicationService", "true", IS_STATIC, IS_VISIBLE},
#endif
#ifdef PEGASUS_DEFAULT_ENABLE_OOP
    {"forceProviderProcesses", "true", IS_STATIC, IS_VISIBLE},
#else
    {"forceProviderProcesses", "false", IS_STATIC, IS_VISIBLE},
#endif
#ifdef PEGASUS_ENABLE_AUDIT_LOGGER
    {"enableAuditLog", "false", IS_DYNAMIC, IS_VISIBLE},
#endif
    {"maxProviderProcesses", "0", IS_DYNAMIC, IS_VISIBLE}

#endif /* Pegasus_DefaultPropertyTableVms_h */
