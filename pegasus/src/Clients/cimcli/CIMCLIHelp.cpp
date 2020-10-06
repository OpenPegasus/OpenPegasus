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
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/General/MofWriter.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/PegasusVersion.h>

#include "CIMCLIClient.h"
#include "ObjectBuilder.h"
#include "CIMCLIOutput.h"
#include "CIMCLICommon.h"
#include "CIMCLIClient.h"
#include "CIMCLIOperations.h"

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN
/*****************************************************************************
**
**    Usage and Help Functions
**
******************************************************************************/
// Character sequences used in help/usage output.

static const char * usage = "This command executes single CIM Operations.\n";

static const char* optionText = "Valid options for this command are : ";
static const char* commonOptions = "    -count, -d, -delay, -p, -l, -u, -o, -x,\
 -v, --sum, --timeout, -r, --t ";

/*
    This function loads the message from resourcebundle using the key passed
*/


void showExamples()
{

    Operations operations;
    cout <<
        loadMessage(
            "Clients.cimcli.CIMCLIClient.EXAMPLES_STRING",
            "Examples : ")
         << endl;

    while(operations.more())
    {
        OperationExampleEntry example = operations.getExampleEntry();
        cout << loadMessage(example.msgKey, example.Example) << endl;

        operations.next();
    }
}

/* showOperations - Display the list of operations possible based
   on what is in the operations table.
   FUTURE: This should probably be in the Operations.cpp file
*/

void showOperations(const char* pgmName, Uint32 lineLength)
{
    Uint32 indent = 27;
    Operations operations;

    while(operations.more())
    {
        OperationTableEntry thisOperation = operations.next();

        cout << endl;
        String opString;
        opString.appendPrintf("%-5s %-21s ",
            thisOperation.ShortCut,
            thisOperation.OperationName);

        opString.append(thisOperation.UsageText);

        String opStringFormatted = foldString(
            opString,
            indent,
            lineLength);

        cout << loadMessage(
            thisOperation.msgKey,
            opStringFormatted.getCString());

    }
    cout << loadMessage(
        "Clients.cimcli.CIMCLIClient.HELP_SUMMARY",
        " -h for all help, -hc for commands, -ho for options")
        << endl;
}

void showVersion(const char* pgmName, OptionManager& om)
{
    String str = "Version ";
    str.append(PEGASUS_PRODUCT_VERSION);

    MessageLoaderParms parms(
        "Clients.cimcli.CIMCLIClient.VERSION",
        str.getCString(),
        PEGASUS_PRODUCT_VERSION);
    parms.msg_src_path = MSG_PATH;
    cout << MessageLoader::getMessage(parms) << endl;
}

void showOptions(const char* pgmName, OptionManager& om)
{
    String optionsTrailer = loadMessage(
        "Clients.cimcli.CIMCLIClient.OPTIONS_TRAILER",
        "Options vary by command consistent with CIM Operations");
    cout << loadMessage(
        "Clients.cimcli.CIMCLIClient.OPTIONS_HEADER",
        "The options for this command are:\n");

    String usageStr;
    usageStr = loadMessage(
        "Clients.cimcli.CIMCLIClient.OPTIONS_USAGE",
        usage);
    om.printOptionsHelpTxt(usageStr, optionsTrailer);
}

void showUsage()
{
    String usageText =
        "Usage: cimcli <command> <CIMObject> <Options> *<extra parameters>\n"
        "    -hc    for <command> set and <CimObject> for each command\n"
        "    -ho    for <Options> set\n"
        "    -h xx  for <command> and <Example> for <xx> operation \n"
        "    -h     for this summary\n"
        "    --help for full help\n";
    CString str = usageText.getCString();
    cout << loadMessage("Clients.cimcli.CIMCLIClient.MENU.STANDARD",
        (const char*)str);
}


/* showFullHelpMsg - Show all of the various help groups including
   usage, version, options, commands, and examples.
*/
void showFullHelpMsg(
    const char* pgmName,
    OptionManager& om,
    Uint32 lineLength)
{
    showUsage();

    showVersion(pgmName, om);

    showOptions(pgmName, om);

    showOperations(pgmName, lineLength);

    cout << endl;

    showExamples();
}

/*
    Show the usage of a single command
    cmd param is shortcut or operation name for the target command.
*/
Boolean showOperationUsage(const char* cmd, OptionManager& om,
                           Uint32 lineLength)
{
    // indent for subsequent lines for help output
    // This value is an arbitary decision.
    Uint32 indent = 27;

    if (cmd)
    {
        // Find the command or the short cut name
        Operations operations;
        if (operations.find(cmd))
        {
            OperationTableEntry thisOperation = operations.get();

            OperationExampleEntry example = operations.getExampleEntry();
            // format the shortcut and
            // command string into a single output string.
            String opString;
            opString.appendPrintf(
                "%-5s %-21s ",
                thisOperation.ShortCut,
                thisOperation.OperationName);

            opString.append(thisOperation.UsageText);

            // Append formatted usage text to the command information
            String txtFormat = foldString(
                opString,
                indent,
                lineLength);

            // output the command and usage information
            cout << loadMessage(
                thisOperation.msgKey,
                txtFormat.getCString());

            cout << endl;

            // Output the corresponding Example and Options information
            cout << loadMessage("Clients.cimcli.CIMCLIClient.EXAMPLE_STRING",
                        "Example : ")
                << endl;
            cout << loadMessage(example.msgKey, example.Example)
                << endl;
            cout << loadMessage("Clients.cimcli.CIMCLIClient.OPTIONS_STRING",
                        optionText)
                << endl;
            cout << loadMessage( example.optionsKey, example.Options)
                << endl;

            // Output the common Options information
            String commonOpStr;
            commonOpStr.appendPrintf("%s", "Common Options are : \n");

            commonOpStr.append(commonOptions);

            cout << loadMessage("Clients.cimcli.CIMCLIClient."
                        "COMMON_OPTIONS_STRING",
                        commonOpStr.getCString())
                << endl;

            return true;
        }
        else
        {
            cerr << "Command \"" << cmd
                    << "\" not legal cimcli operation name.\n"
                    " Type cimcli -hc to list valid commands."
                 << endl;
            cout << loadMessage(
                "Clients.cimcli.CIMCLIClient.HELP_SUMMARY",
                " -h for all help, -hc for commands, -ho for options")
                << endl;
            return false;
        }
    }
    else
    {
        showUsage();
        return true;
    }
}

PEGASUS_NAMESPACE_END
// END_OF_FILE
