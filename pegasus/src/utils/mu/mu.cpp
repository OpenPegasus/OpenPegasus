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

#include <iostream>
#include <cstring>
#include "Config.h"
#include "CompareCmd.h"
#include "CopyCmd.h"
#include "DependCmd.h"
#include "EchoCmd.h"
#include "MkDirHierCmd.h"
#include "MoveCmd.h"
#include "PwdCmd.h"
#include "RmCmd.h"
#include "RmDirHierCmd.h"
#include "TouchCmd.h"
#include "StripCmd.h"
#include "PrependCmd.h"
#include "SleepCmd.h"
#include "SortCmd.h"
#include "SrcListCmd.h"
#include "Files.h"
#include "ReplaceCmd.h"

#ifdef PEGASUS_OS_ZOS
// This is inline code. No external dependency is created.
#include <unistd.h>
#include <Pegasus/General/SetFileDescriptorToEBCDICEncoding.h>

PEGASUS_USING_PEGASUS;
#endif

const char HELP[] =
"\n"
"MU (MakeUtility) Version 1.5.0\n"
"\n"
"Usage: mu command arguments ...\n"
"\n"
"Where command is one of the following:\n"
"    rm, rmdirhier, mkdirhier, echo, touch, pwd, copy, move, compare, depend\n"
"    strip, prepend, sleep, sort, srclist ,replace\n";

int main(int argc, char** argv)
{

#ifdef PEGASUS_OS_ZOS
    // For z/OS set stdout and stderr to EBCDIC
    setEBCDICEncoding(STDOUT_FILENO);
    setEBCDICEncoding(STDERR_FILENO);
#endif

    // Process the help option:

    if ((argc == 2 && strcmp(argv[1], "-help") == 0) || argc < 2)
    {
        cerr << HELP << endl;
        return(1);
    }

    // Build up the argument list (expanding as we go)

    vector<string> args;

    args.push_back(argv[1]);

    for (int i = 2; i < argc; i++)
    {
        vector<string> argsOut;

        if (Glob(argv[i], argsOut))
        {
            args.insert(args.end(), argsOut.begin(), argsOut.end());
        }
        else
        {
            args.push_back(argv[i]);
        }
    }

    // Execute the command:

    int result = 0;

    if (args[0] == "rm")
        result = RmCmd(args);
    else if (args[0] == "rmdirhier")
        result = RmDirHierCmd(args);
    else if (args[0] == "mkdirhier")
        result = MkDirHierCmd(args);
    else if (args[0] == "echo")
        result = EchoCmd(args);
    else if (args[0] == "echo-e")
        result = EchoWithEscapeCmd(args);
    else if (args[0] == "touch")
        result = TouchCmd(args);
    else if (args[0] == "pwd")
        result = PwdCmd(args);
    else if (args[0] == "copy")
        result = CopyCmd(args);
    else if (args[0] == "move")
        result = MoveCmd(args);
    else if (args[0] == "compare")
        result = CompareCmd(args);
    else if (args[0] == "depend")
        result = DependCmd(args);
    else if (args[0] == "strip")
        result = StripCmd(args);
    else if (args[0] == "prepend")
        result = PrependCmd(args);
    else if (args[0] == "sleep")
        result = SleepCmd(args);
    else if (args[0] == "sort")
        result = SortCmd(args);
    else if (args[0] == "srclist")
        result = SrcListCmd(args);
    else if (args[0] == "replace")
        result = ReplaceCmd(args);
    else
    {
        result = 1;
        cerr << args[0] << ": unknown command: \"" << args[0] << "\"" << endl;
    }

    return(result);
}
