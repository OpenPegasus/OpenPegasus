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

#ifdef PEGASUS_ZOS_SECURITY
    {"enableAuthentication",                     "true"},
#else
    {"enableAuthentication",                     "false"},
#endif
    {"httpAuthType",                             "Basic"},
#if defined(PEGASUS_USE_SYSLOGS)
    {"logdir",                                        ""},
#endif
    {"passwordFilePath",              "cimserver.passwd"},
    {"sslCertificateFilePath",              "server.pem"},
    {"sslKeyFilePath",                        "file.pem"},
#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
    {"crlStore",                                   "crl"},
#endif
    {"sslClientVerificationMode",             "disabled"},
    {"sslTrustStoreUserName",                         ""},
    {"enableNamespaceAuthorization",             "false"},
    {"enableSubscriptionsForNonprivilegedUsers",  "true"},
#ifdef PEGASUS_ENABLE_USERGROUP_AUTHORIZATION
    {"authorizedUserGroups",                          ""},
#endif
    {"providerManagerDir",                         "lib"},
#ifdef PEGASUS_USE_RELEASE_CONFIG_OPTIONS
    {"enableAssociationTraversal",                "true"},
    {"enableBinaryRepository",                    "true"},
#endif
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
    {"webRoot",                          "/var/wbem/www"},
    {"indexFile",                           "index.html"},
    {"mimeTypesFile",       "/var/wbem/www/mimeType.txt"},
#endif
    {"sslTrustStore",                  "cimserver_trust"}

