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
//

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/CIMStatusCode.h>
#include <Pegasus/Common/Pegasus_inl.h>
#include "CIMCLICommon.h"
#include "CIMCLIClient.h"
#include <cstdarg>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

//Global Values

// Set by input options functions and used to test for expected exit code.
Uint32 expectedExitCode = 0;


// Internal function to generate a string dependent on the exitCode.
// The generated string is a header for output messages (ex. ERROR, etc.)
// The second parameter allows return of the message bundle code for
// internationalized messages.
static String _getMsgHeader(Uint32 exitCode)
{
    // Puts prefix on message
    if (exitCode == CIMCLI_INPUT_ERR)
    {
        return "INPUT ERROR: ";
    }
    else
    {
        if (exitCode != 0)
        {
            return "ERROR: ";
        }
    }
    return String();
}

void cimcliMsg::exit(
    Uint32 exitCode,
    const String& formatString)
{
    cerr << _getMsgHeader(exitCode)
         << formatString << endl;
    cimcliExit(exitCode);
}

/** EXIT functions. All functions with name "exit" output a
 *  message and then Exit cimcli.
 *
 *  Internationalized output message with exit
 *  @param exit Code - See CIMCLIClient for definition of these
 *              codes.
 *  @param msgParams Call to function MessageLoaderParms to
 *                   build internationalized message
*/
void cimcliMsg::exit(
    Uint32 exitCode,
    const MessageLoaderParms& msgParms)
{
    MessageLoaderParms parms = msgParms;
    parms.useProcessLocale = true;
    parms.msg_src_path = MSG_PATH;

    // output header and message
    cout << _getMsgHeader(exitCode)
         << MessageLoader::getMessage(parms) << endl;

    // execute exit from cimcli.  This may involve another
    // message
    cimcliExit(exitCode);
}

/** Optimized one-parameter form of Message output
*/
void cimcliMsg::exit(
    Uint32 exitCode,
    const String& formatString,
    const Formatter::Arg& arg0)
{
    cout << _getMsgHeader(exitCode)
         << Formatter::format(formatString, arg0) << endl;
    cimcliExit(exitCode);
}

/** Optimized two-argument form of Message output
*/
void cimcliMsg::exit(
    Uint32 exitCode,
    const String& formatString,
    const Formatter::Arg& arg0,
    const Formatter::Arg& arg1)
{
    cout << _getMsgHeader(exitCode)
        << Formatter::format(formatString, arg0, arg1) << endl;
    cimcliExit(exitCode);
}

/** Optimized three-argument form of Message output
*/
void cimcliMsg::exit(
    Uint32 exitCode,
    const String& formatString,
    const Formatter::Arg& arg0,
    const Formatter::Arg& arg1,
    const Formatter::Arg& arg2)
{
    cout << _getMsgHeader(exitCode)
         << Formatter::format(formatString, arg0, arg1, arg2) << endl;
    cimcliExit(exitCode);
}

/** Internationalized msg with output Message
*/
void cimcliMsg::msg(
    const MessageLoaderParms& msgParms)
{
    MessageLoaderParms parms = msgParms;
    parms.useProcessLocale = true;
    parms.msg_src_path = MSG_PATH;
    cout << MessageLoader::getMessage(parms) << endl;
}

/** Optimized one-parameter form of Message output
*/
void cimcliMsg::msg(
    const String& formatString,
    const Formatter::Arg& arg0)
{
    cout << Formatter::format(formatString, arg0) << endl;
}

/** Optimized two-argument form of Message output
*/
void cimcliMsg::msg(
    const String& formatString,
    const Formatter::Arg& arg0,
    const Formatter::Arg& arg1)
{
    cout << Formatter::format(formatString, arg0, arg1) << endl;

}

/** Optimized three-argument form of Message output
*/
void cimcliMsg::msg(
    const String& formatString,
    const Formatter::Arg& arg0,
    const Formatter::Arg& arg1,
    const Formatter::Arg& arg2)
{
    cout << Formatter::format(formatString, arg0, arg1, arg2) << endl;
}


/** Internationalized msg with output Message to cerr
*/
void cimcliMsg::errmsg(
    const MessageLoaderParms& msgParms)
{
    MessageLoaderParms parms = msgParms;
    parms.useProcessLocale = true;
    parms.msg_src_path = MSG_PATH;
    cerr << MessageLoader::getMessage(parms) << endl;
}

/** Optimized one-parameter form of Message output
*/
void cimcliMsg::errmsg(
    const String& formatString,
    const Formatter::Arg& arg0)
{
    cout << Formatter::format(formatString, arg0) << endl;
}

/** Optimized two-argument form of Message output
*/
void cimcliMsg::errmsg(
    const String& formatString,
    const Formatter::Arg& arg0,
    const Formatter::Arg& arg1)
{
    cerr << Formatter::format(formatString, arg0, arg1) << endl;
}

/** Optimized three-argument form of Message output
*/
void cimcliMsg::errmsg(
    const String& formatString,
    const Formatter::Arg& arg0,
    const Formatter::Arg& arg1,
    const Formatter::Arg& arg2)
{
    cerr << Formatter::format(formatString, arg0, arg1, arg2) << endl;
}

void _print(Boolean x)
{
    cout << boolToString(x);
}

String _toString(Array<CIMName> array)
{
    String rtn;
    for (Uint32 i = 0 ; i < array.size(); i++)
    {
        rtn.append(" ");

        rtn.append(array[i].getString());
    }
    return rtn;
}

String loadMessage(const char* key, const char* msg)
{
    MessageLoaderParms parms(key, msg);
    parms.msg_src_path = MSG_PATH;
    return MessageLoader::getMessage(parms);
}

// Output a CIMPropertyList to cout
void _print(const CIMPropertyList& pl)
{
    cout << pl.toString();
}

String _toString(const Array<CIMNamespaceName>& nsList)
{
    String rtn;
    for (Uint32 i = 0 ; i < nsList.size() ; i++ )
    {
        if (i != 0)
        {
            rtn.append(", ");
        }
        rtn.append(nsList[i].getString());
    }
    return rtn;
}

void _print(const Array<CIMNamespaceName>& List)
{
    cout << _toString(List);
}

String _toString(const Array<String>& strList)
{
    String rtn;
    for (Uint32 i = 0 ; i < strList.size() ; i++)
    {
        if (i > 0)
        {
            rtn.append(", ");
        }
        rtn.append(strList[i]);
    }
    return rtn;
}

void _print(const Array<String>& strList)
{
    cout << _toString(strList);
}

/* tokenize an input string into an array of Strings,
 * separating the tokens at the separator character
 * provided
 * @param input String
 * @param separator character
 * @param all Boolean if true do multiple tokens.
 * If false, stop after first.
 * @returns Array of separated strings
 * Terminates
 *  after first if all = false.
 */
Array<String> _tokenize(const String& input,
                        const Char16 separator,
                        bool allTokens)
{
    Array<String> tokens;
    if (input.size() != 0)
    {
        Uint32 start = 0;
        Uint32 length = 0;
        Uint32 end = 0;
        if (allTokens)
        {
            while ((end = input.find(start, separator)) != PEG_NOT_FOUND)
            {
                length = end - start;

                tokens.append(input.subString(start, length));
                start += (length + 1);
            }
        }
        else
        {
            if ((length = input.find(start, separator)) != PEG_NOT_FOUND)
            {
                tokens.append(input.subString(start,length));
                start+= (length + 1);
            }
        }
        //Replaced < with <= to consider input param like A="" as valid param.
        //key in this param is 'A'and value is NULL.
        //It also takes care of A= param.
        if(start <= input.size())
        {
            tokens.append(input.subString(start));
        }
    }
    return tokens;
}

/* Build a property list from all of the property names in the input instance.
*/
CIMPropertyList _buildPropertyList(const CIMInstance& inst)
{
    Array<CIMName> tmp;
    for (Uint32 i = 0 ; i < inst.getPropertyCount() ; i++)
    {
        CIMConstProperty instProperty = inst.getProperty(i);
        tmp.append(instProperty.getName());
    }
    CIMPropertyList pl;
    pl.set(tmp);

    return(pl);
}

/*
    Wrapper functions for the conversions in Pegasus Common
    StringConversion.h
*/
Sint64 strToSint(const char* str, CIMType type)
{
    Sint64 s64;
    Boolean success =
        (StringConversion::stringToSint64(
             str, StringConversion::decimalStringToUint64, s64) ||
         StringConversion::stringToSint64(
             str, StringConversion::hexStringToUint64, s64) ||
         StringConversion::stringToSint64(
             str, StringConversion::octalStringToUint64, s64) ||
         StringConversion::stringToSint64(
             str, StringConversion::binaryStringToUint64, s64)) &&
        StringConversion::checkSintBounds(s64, type);
    if (!success)
    {
        cimcliMsg::exit(CIMCLI_INPUT_ERR,
            "Value conversion error. $0s. type $1\n",
               str, cimTypeToString(type));
    }

    return s64;
}

Uint64 strToUint(const char* str, CIMType type)
{
    Uint64 u64;
    Boolean success =
        (StringConversion::decimalStringToUint64(str, u64) ||
         StringConversion::hexStringToUint64(str, u64) ||
         StringConversion::octalStringToUint64(str, u64) ||
         StringConversion::binaryStringToUint64(str, u64)) &&
         StringConversion::checkUintBounds(u64, type);

    if (!success)
    {
        cimcliMsg::exit(CIMCLI_INPUT_ERR,
            "Value conversion error. $0. type $1\n",
               str, cimTypeToString(type));
    }

    return u64;
}

Real64 strToReal(const char * str, CIMType type)
{
    Real64 r64;

    if (!StringConversion::stringToReal64(str, r64))
    {
        cimcliMsg::exit(CIMCLI_INPUT_ERR,
            "Value conversion error. $0. type $1\n",
               str, cimTypeToString(type));
    }
    if (type == CIMTYPE_REAL32)
    {
        if ((r64 > 3.4E+38f) || (r64 < -3.4E38f))
        {
            cimcliMsg::exit(CIMCLI_INPUT_ERR,
                "Value Invalid size. $0. type $1\n",
                   str, cimTypeToString(type));
        }
    }
    return r64;
}

// Local function that tests exitCode against expectedExitCode and
// returns proper result. Used only by cimcliExit and cimcliExitRtn
Uint32 _cimcliExitLocal(Uint32 exitCode)
{
    // This should be the only use of exit in cimcli.
    // all other exits should use cimcliExit
    if (expectedExitCode == exitCode)
    {
        return 0;
    }

    // Do not print a warning message if the expected return code is
    // zero (success) since a more precise exception message was
    // printed already. The warning does not contain additional user-relevant
    // information in this case but can be misleading.
    if ( 0 != expectedExitCode )
    {
        cerr << "WARNING: Expected exit code " << expectedExitCode
             << ". Program delivered exit code (" << exitCode
             << ") " << rtnExitCodeToString(exitCode) << endl;
    }
    return exitCode;
}
Uint32 cimcliExitRtn(Uint32 exitCode)
{
    return _cimcliExitLocal(exitCode);
}
void cimcliExit(Uint32 exitCode)
{
    exit(_cimcliExitLocal(exitCode));
}

void setExpectedExitCode(Uint32 exitCode)
{
    expectedExitCode = exitCode;
}

/***************************************************************************
**
**  String formatting functions.  These are helper functions to allow
**  use of C++ standard printf formatting concepts but that return
**  Pegasus String results rather than char* and handle all memory
**  issues internally.
**
***************************************************************************/

String fillString(Uint32 count,  const char x)
{
    String str;
    for (Uint32 i = 0 ; i < count ; i++)
    {
        str.append(x);
    }
    return str;
}

/* Remap a long string into a multi-line string that can be positioned on a
   line starting at pos and with length defined for each line.
   Each output line consists of fill parameter to pos and max line length
   defined display length parameter.

   The input string is recreated by tokenizing on the space character
   and filled from the left so that the returned string can be output
   as a multiline string starting at pos.
*/
static String _fill(Uint32 count)
{
    String str;
    for (Uint32 i = 0 ; i < count ; i++)
    {
        str.append(' ');
    }
    return str;
}


String foldString(const String& input, Uint32 startPos, Uint32 foldPos)
{
    String strOut;
    String line;
    String word;
    for (Uint32 i = 0; i < input.size(); i++)
    {
        switch (input[i])
        {
            case ' ':
            {
                // if next word overflows line. move it to output String
                            // and start new line
                if ((line.size() + word.size()) >= foldPos)
                {
                    strOut.append(line);
                    strOut.append('\n');
                    line = _fill(startPos);
                }

                    line.append(word);
                    word.clear();
                    word.append(input[i]);
                break;
            }

            case '\n':
            {
                if (line.size() + word.size() >= foldPos)
                {
                    // terminate current line
                    strOut.append(line);
                    strOut.append('\n');
                    line = _fill(startPos);
                }

                line.append(word);
                word.clear();
                line.append('\n');
                strOut.append(line);
                line = _fill(startPos);

                break;
            }

            default:
                word.append(input[i]);
        }
    }

    if (word.size() != 0)
    {
        // if overflow create new line
        if ((line.size() + word.size()) >= foldPos)
        {
            strOut.append(line);
            strOut.append('\n');
            line = _fill(startPos);
        }
        // pick up last word.
        line.append(word);
    }

    // if data in last line, move it.
    if (line.size() != startPos)
    {
        strOut.append(line);
    }
    return strOut;
}

// Return a string definition for each possible exit code
String rtnExitCodeToString(Uint32 exitCode)
{
    String rtn;
    switch (exitCode)
    {
        case CIMCLI_RTN_CODE_OK:
            rtn= "OK";
            break;
        case CIMCLI_INTERNAL_ERR:
            rtn = "Internal error";
            break;
        case CIMCLI_RTN_CODE_PEGASUS_EXCEPTION:
            rtn = "Pegasus Exception";
            break;
        case CIMCLI_RTN_COUNT_TEST_FAILED:
            rtn = "Count test (--count) failed";
            break;
        case CIMCLI_RTN_CODE_ERR_COMPARE_FAILED:
            rtn = "Test Comparison failed";
            break;
        case CIMCLI_CONNECTION_FAILED:
            rtn = "Connection Failed";
            break;
        case CIMCLI_INPUT_ERR:
            rtn = "Input Error";
            break;
        case CIMCLI_RTN_CODE_UNKNOWN_EXCEPTION:
            rtn = "Unknown Exception";
            break;
        case GENERAL_CLI_ERROR_CODE:
            rtn = "General cimcli Error";
            break;
        default:
            if (exitCode < CIMCLI_RTN_CODE_PEGASUS_EXCEPTION)
            {
                rtn = cimStatusCodeToString((CIMStatusCode)exitCode);
            }
            else
                rtn = "Unknown code";
    }
    return rtn;
}

PEGASUS_NAMESPACE_END
// END_OF_FILE
