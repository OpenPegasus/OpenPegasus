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

#ifndef Pegasus_FixedPropertyTablePase_h
#define Pegasus_FixedPropertyTablePase_h

#ifdef PEGASUS_USE_RELEASE_CONFIG_OPTIONS
    {"enableRemotePrivilegedUserAccess",         "true"},
    {"enableBinaryRepository",                  "false"},
    {"daemon",                                   "false"},
    {"passwordFilePath",                         "cimserver.passwd"},
# ifdef PEGASUS_ENABLE_SLP
    {"slp",                                      "true"},
# endif
#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
    {"webRoot",         "/QOpenSys/QIBM/UserData/UME/Pegasus/www"},
    {"indexFile",       "index.html"},
    {"mimeTypesFile",   "/QOpenSys/QIBM/UserData/UME/Pegasus/www/mimeType.txt"},
#endif
    {"authorizedUserGroups",                          ""},
    {"repositoryDir",         "/QOpenSys/QIBM/UserData/UME/Pegasus/repository"},
    {"providerDir",           "/QOpenSys/QIBM/ProdData/UME/Pegasus/provider:"
                                  "/QOpenSys/usr/lib"},
    {"messageDir",            "/QOpenSys/QIBM/ProdData/UME/Pegasus/msg"},
    {"providerManagerDir",    "/QOpenSys/QIBM/ProdData/UME/Pegasus/lib"},
#endif

#endif /* Pegasus_FixedPropertyTablePase_h */
