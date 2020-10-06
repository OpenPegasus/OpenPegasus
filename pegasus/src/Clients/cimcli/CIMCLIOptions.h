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

#ifndef _CLI_OPTIONS_H
#define _CLI_OPTIONS_H

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/General/OptionManager.h>
#include <Clients/cimcli/Linkage.h>
#include <Pegasus/General/Stopwatch.h>
#include <Pegasus/Common/MessageLoader.h>

#include "CIMCLIClient.h"

PEGASUS_NAMESPACE_BEGIN

void  PEGASUS_CLI_LINKAGE BuildOptionsTable(
    OptionManager& om,
    int& argc,
    char** argv,
    const String& testHome);

/** Test the common Options
    @param om
    @param argv
    @param opts

    @return Boolean true if results good or false if error
    or if evaluated to a help command and we want to terminate
    immediatly
 */
Boolean PEGASUS_CLI_LINKAGE CheckCommonOptionValues(
    OptionManager& om,
    char** argv,
    Options& opts);

PEGASUS_NAMESPACE_END

#endif  /* _CLI_OPTIONS_H */
