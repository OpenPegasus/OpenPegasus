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

#ifndef _CLI_CLIENTLIB_H
#define _CLI_CLIENTLIB_H

#include <Pegasus/Common/Config.h>
#include <Clients/cimcli/Linkage.h>

#include "CIMCLIOptionStruct.h"


PEGASUS_NAMESPACE_BEGIN
#define CDEBUG(X) PEGASUS_STD(cout) << "cimcli " << X << PEGASUS_STD(endl)
//#define CDEBUG(X)

// Defined here because apparently not all platforms support max and min.
#define LOCAL_MAX(a, b) ((a > b) ? a : b)
#define LOCAL_MIN(a, b) ((a < b) ? a : b)

static const char MSG_PATH [] = "pegasus/pegasusCLI";


////////////////////////////////////////////////////////////////////////////
//
// Return codes used for cimcli exit values

#define CIMCLI_RTN_CODE_OK 0   // successful completion of the command

// All of the codes up to 50 are reserved.  This is the set of error codes
// returned by CIM Exception and cimcli passes any such received exception
// codes back through cimcli completion. Therefore we start the internal
// error codes outside of this range
// NOTE: Please coordinate the rtnExitCodeToString(...) function in
// CIMCLICommon with any changes to the following defines
//
#define CIMCLI_RTN_CODE_PEGASUS_EXCEPTION 50

// codes for failures based on user input or general issues
#define GENERAL_CLI_ERROR_CODE 51
#define CIMCLI_RTN_CODE_UNKNOWN_EXCEPTION 52
#define CIMCLI_INPUT_ERR 53
#define CIMCLI_CONNECTION_FAILED 54

// codes for cimcli test failures.
#define CIMCLI_RTN_CODE_ERR_COMPARE_FAILED 60
#define CIMCLI_RTN_COUNT_TEST_FAILED 61

// CIMCLI internal logic failures (i.e. These should NEVER happen :-)
#define CIMCLI_INTERNAL_ERR 70

/////////////////////////////////////////////////////////////////////////////
// ************* cimcli operations ******************************************
// These are the operation functions executed by cimcli.  Each takes input
// from the opts structure and generates output from a single or set of cim
// operations executed against the server defined in the opts structure.
// These functions correspond directly to the cimcli operation definitions
// (also called commands) that are the first parameter of each cimcli
// command line definition.
//
int PEGASUS_CLI_LINKAGE enumerateClassNames(Options& opts);

int PEGASUS_CLI_LINKAGE enumerateClasses(Options& opts);

int PEGASUS_CLI_LINKAGE deleteClass(Options& opts);

int PEGASUS_CLI_LINKAGE getClass(Options& opts);

int PEGASUS_CLI_LINKAGE deleteInstance(Options& opts);

int PEGASUS_CLI_LINKAGE enumerateInstanceNames(Options& opts);

int PEGASUS_CLI_LINKAGE enumerateAllInstanceNames(Options& opts);

int PEGASUS_CLI_LINKAGE enumerateInstances(Options& opts);

int PEGASUS_CLI_LINKAGE createInstance(Options& opts);

int PEGASUS_CLI_LINKAGE testInstance(Options& opts);

int PEGASUS_CLI_LINKAGE modifyInstance(Options& opts);

int PEGASUS_CLI_LINKAGE getInstance(Options& opts);

int PEGASUS_CLI_LINKAGE getProperty(Options& opts);

int PEGASUS_CLI_LINKAGE setProperty(Options& opts);

int PEGASUS_CLI_LINKAGE setQualifier(Options& opts);

int PEGASUS_CLI_LINKAGE getQualifier(Options& opts);

int PEGASUS_CLI_LINKAGE deleteQualifier(Options& opts);

int PEGASUS_CLI_LINKAGE enumerateQualifiers(Options& opts);

int PEGASUS_CLI_LINKAGE referenceNames(Options& opts);

int PEGASUS_CLI_LINKAGE references(Options& opts);

int PEGASUS_CLI_LINKAGE associators(Options& opts);

int PEGASUS_CLI_LINKAGE associatorNames(Options& opts);

int PEGASUS_CLI_LINKAGE invokeMethod(Options& opts);

int PEGASUS_CLI_LINKAGE execQuery(Options& opts);

int PEGASUS_CLI_LINKAGE enumerateNamespaceNames(Options& opts);

int PEGASUS_CLI_LINKAGE showProfiles(Options& opts);

int PEGASUS_CLI_LINKAGE setObjectManagerStatistics(Options& opts,
                                   Boolean newState,
                                   Boolean& stateAfterMod);

//KS_PULL_BEGIN
int PEGASUS_CLI_LINKAGE pullEnumerateInstances(Options& opts);

int PEGASUS_CLI_LINKAGE pullEnumerateInstancePaths(Options& opts);

int PEGASUS_CLI_LINKAGE pullReferenceInstances(Options& opts);

int PEGASUS_CLI_LINKAGE pullReferenceInstancePaths(Options& opts);

int PEGASUS_CLI_LINKAGE pullAssociatorInstances(Options& opts);

int PEGASUS_CLI_LINKAGE pullAssociatorInstancePaths(Options& opts);

int PEGASUS_CLI_LINKAGE pullQueryInstances(Options& opts);

int PEGASUS_CLI_LINKAGE countInstances(Options& opts);
//KS_PULL_END

int PEGASUS_CLI_LINKAGE countInstances(Options& opts);

int PEGASUS_CLI_LINKAGE classTree(Options& opts);
PEGASUS_NAMESPACE_END

#endif
