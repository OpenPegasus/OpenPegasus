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

#ifndef _CLI_COMMON_H
#define _CLI_COMMON_H

#include <Pegasus/Common/Config.h>
#include <Clients/cimcli/Linkage.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/MessageLoader.h>
#include <cstdarg>
PEGASUS_NAMESPACE_BEGIN


// Diagnostic tool maintained just to trace development issues.
// issues a single line when set into source code.
#define TRACELINE cout << __FILE__ << ":" << __LINE__ << endl;

// Common functions used in cimcli

String loadMessage(const char* key, const char* msg);

/**
 * cliExit - Function to actually execute cimcli exits
 */
void cliExit(Uint32 exitCode, const char* format, ...);

/**
    This class provides a set of static functions to provide:

    1. Standard error message generation with exit from cimcli
    in a form that the messages can be internationalized.  These
    are the Exit(...) functions where the arguements include a
    cimcli exit code and information to format the messages

    2. Stand error message generation in a form that the
    messages can be internationalized. These methods are all
    Msg(...) where the arguments are either internationalized or
    internationlaizable messages.

    All the messages generated use the Pegasus Formatter.h as
    the basis for defining the messages (formatString, one or
    more arguments).

    Both Exit(...) and Msg(...) methods include:

    a. Generation of non-internationalized messages with 1 or
    more arguments for the formatString

    b. Internationalized messages that use MessageLoaderParms in
    place of the direct call to define the messages.

    Once a non-internationalized message is defined in cimcli
       ex: cimcliMsg::Msg("This is my $0 msg. $1", count, name);
    it can be internationlaized by simply enveloping the
    parameters in MessageLoaderParms method
       ex:
       ex: cimcliMsg::Msg(MessageLoaderParms(
            "Clients.cimcli.CIMCLIClient.BLAH_MSG",
            "This is my $0 msg. $1", count, name));
    NOTE: To make it easier in the future to confirm whether
    messages and exits are internationalized, please use the
    form of the message above so a simple scan will determine
    whether internationalized (if cimcliMsg and MsgLoaderParms
    exist on the same source line)
*/
class cimcliMsg
{
public:
    /** Internationalized form of exit with message output
     *
     *
     * @param exitCode Code to set for this exit.
     * @param msgParms MessageLoaderParms definition with key to
     * internationalized message and default message consisting of
     * format string and variable number of parameters.
     */
    static void exit(
        Uint32 exitCode,
        const MessageLoaderParms& msgParms);

    /** Non internationalized exit with message output.
     * @param exitCode Uint32 exit code to set for this exit
     * @param formatString String without parameters that is
     *                     displayed
     */
    static void exit(
        Uint32 exitCode,
        const String& formatString);

    /** One-argument form of exit with message
    */
    static void exit(
        Uint32 exitCode,
        const String& formatString,
        const Formatter::Arg& arg0);

    /** Two-argument form of exit with message
    */
    static void exit(
        Uint32 exitCode,
        const String& formatString,
        const Formatter::Arg& arg0,
        const Formatter::Arg& arg1);

    /** Three-argument form of exit with message
    */
    static void exit(
        Uint32 exitCode,
        const String& formatString,
        const Formatter::Arg& arg0,
        const Formatter::Arg& arg1,
        const Formatter::Arg& arg2);


    /** Internationalized form of message output. The following
     *  methods output messages but do not
     *  terminate cimcli
     */
     /**
     * Generate an Internationlaized output message.
     *
     * @param msgParms MessageLoaderParms definition with key to
     * internationalized message and default message consisting of
     * format string and variable number of parameters.
     */
    static void msg(
        const MessageLoaderParms& msgParms);

    /** Non internationalized message output with a single String
     *  parameter
     * @param formatString String without parameters that is
     *                     displayed
     */
    static void msg(
        const String& formatString);

    /** One-argument form of message that generates a message
     *  from a formatString and a single argument.
     *  @param formatString - formatting string as defined in the
     *  file src/Pegasus/Common/Formatter.h
     *  @param arg0  Input argument for formatterString.See
     *               Formatter.h for more information on possible
     *               arguments.
    */
    static void msg(
        const String& formatString,
        const Formatter::Arg& arg0);

    /** Two-argument form of message that generates a message
     *  from a formatString and a single argument.
     *  @param formatString - formatting string as defined in the
     *  file src/Pegasus/Common/Formatter.h
     *  @param arg0 - First input argument (replaces $0 in
     *              formatString)
     *  @param arg1 - Second input argument (replaces $1 in
     *              formatString)
    */
        static void msg(
        const String& formatString,
        const Formatter::Arg& arg0,
        const Formatter::Arg& arg1);

    /** Three-argument form of message that generates a message
     *  from a formatString and a single argument.
     *  @param formatString - formatting string as defined in the
     *  file src/Pegasus/Common/Formatter.h
     *  @param arg0 - First input argument (replaces $0 in
     *              formatString)
     *  @param arg1 - Second input argument (replaces $1 in
     *              formatString)
     *  @param arg3 - Thirs input argument (replaces $2 in
     *              formatString)
    */
    static void msg(
        const String& formatString,
        const Formatter::Arg& arg0,
        const Formatter::Arg& arg1,
        const Formatter::Arg& arg2);

    /** Internationalized form of message output. The following
     *  methods output messages but do not
     *  terminate cimcli
     */
     /**
     * Generate an Internationlaized output message to cerr
     *
     * @param msgParms MessageLoaderParms definition with key to
     * internationalized message and default message consisting of
     * format string and variable number of parameters.
     */
    static void errmsg(
        const MessageLoaderParms& msgParms);

    /** Non internationalized message output with a single String
     *  parameter
     * @param formatString String without parameters that is
     *                     displayed
     */
    static void errmsg(
        const String& formatString);

    /** One-argument form of message that generates a message
     *  from a formatString and a single argument.
     *  @param formatString - formatting string as defined in the
     *  file src/Pegasus/Common/Formatter.h
     *  @param arg0  Input argument for formatterString.See
     *               Formatter.h for more information on possible
     *               arguments.
    */
    static void errmsg(
        const String& formatString,
        const Formatter::Arg& arg0);

    /** Two-argument form of message that generates a message
     *  from a formatString and a single argument.
     *  @param formatString - formatting string as defined in the
     *  file src/Pegasus/Common/Formatter.h
     *  @param arg0 - First input argument (replaces $0 in
     *              formatString)
     *  @param arg1 - Second input argument (replaces $1 in
     *              formatString)
    */
        static void errmsg(
        const String& formatString,
        const Formatter::Arg& arg0,
        const Formatter::Arg& arg1);

    /** Three-argument form of message that generates a message
     *  from a formatString and a single argument.
     *  @param formatString - formatting string as defined in the
     *  file src/Pegasus/Common/Formatter.h
     *  @param arg0 - First input argument (replaces $0 in
     *              formatString)
     *  @param arg1 - Second input argument (replaces $1 in
     *              formatString)
     *  @param arg3 - Thirs input argument (replaces $2 in
     *              formatString)
    */
    static void errmsg(
        const String& formatString,
        const Formatter::Arg& arg0,
        const Formatter::Arg& arg1,
        const Formatter::Arg& arg2);
private:

};

/****************************************************************************
*
*        Collection of toString functions that are not part of the
*        basic objects, primitive types, or Array functions but are
*        used in CIMCLI.
*
**
*****************************************************************************/
/**
 * Create a string filled with the defined character
*/
String  PEGASUS_CLI_LINKAGE fillString(Uint32 count, const char x = ' ');

/**
 * convert a Boolean to a String ("true" or "false")
 */
String PEGASUS_CLI_LINKAGE _toString(Boolean x);

/**
 * print the string repesentation of a Boolean
 * @param x Bpp;ean to print
 */
void  PEGASUS_CLI_LINKAGE _print(Boolean x);

/**
 * Convert a PropertyList object to displayable form
 * @param pl propertyList to print
 * @return String containing propertyList in displayable form
 */
void  PEGASUS_CLI_LINKAGE _print(const CIMPropertyList& pl);

String _toString(Array<CIMName> array);

/**
    Return a String with display representation of an Array of
    Strings. The form is comma separated on a single line
*/
String  PEGASUS_CLI_LINKAGE _toString(const Array<String>& strList);
/**
    Display a String representation of the array of Strings to cout.
 */
void  PEGASUS_CLI_LINKAGE _print(const Array<String>& strList);

/**
    Generate comma separated list of namespace names
*/
String PEGASUS_CLI_LINKAGE _toString(const Array<CIMNamespaceName>& List);
void PEGASUS_CLI_LINKAGE _print(const Array<CIMNamespaceName>& List);

/**
 * Create an array of strings where each entry is the substring
 * of the input between the separator character.
 *
 * @param input
 * @param separator
 * @param allTokens
 * @return Array<String>
 */
Array<String>  PEGASUS_CLI_LINKAGE _tokenize(
    const String& input,
    const Char16 separator,
    bool allTokens);

/**
   Build a property list from all of the property names in the input instance
   @param inst CIMInstance from which propertylist built
   @return CIMPropertyList will all names from the instance
*/
CIMPropertyList PEGASUS_CLI_LINKAGE _buildPropertyList(
    const CIMInstance& inst);

/**
    Common functions for conversion of char* strings to CIMTypes defined
    by the type variable.  Note that all of these functions execute an exit
    if the conversion fails with the exit code set to CIMCLI_INPUT_ERR.
    They are intended for parsing of input from command line, config files,
    etc. All allow input in binary, octal or decimal formats.
    @param str const char* containing the ASCII definition of the input
    @param type CIMType definition for the conversion process.  Note that
    while the type is used to validate input (i.e. max size) the return is
    always 64 bit values.

    @return 64 bit output of unsigned int, signed int, or Real.  Note that
    the
*/
Sint64 PEGASUS_CLI_LINKAGE strToSint(const char* str, CIMType type);

Uint64 PEGASUS_CLI_LINKAGE strToUint(const char* str, CIMType type);

Real64 PEGASUS_CLI_LINKAGE strToReal(const char * str, CIMType type);

/**
 * Exit cimcli. This function executes an exit of cimcli after
 * testing the exitCode provided against the code defined in the
 * --expExit option.
 * NOTE: In the future, insure that this is only local to common
 * and that all exits are through the cimcliExit class defined
 * below. KS Aug 2011
 *
 * @param exitCode  exit code defined by the application
 * If the exitCode matches the expectedExit code exit with exit
 * code == 0. else exit with exitCode.
 * This allows user to set a certain code that system will
 * convert to good exit from program.  Primarily used for
 * Makefile testing where only zero allows the test to continue.
 *
 * This function does not return to the the caller but does a
 * program exit.
 */
void PEGASUS_CLI_LINKAGE cimcliExit(Uint32 exitCode);

/**
    Same cimcliExit except returns to the user with the return
    code to be used.  This was defined to allow the main to to a
    return on exit rather than exit which helps clean up on
    cimcli exit. (See bug 9758)

    @param exitCode

    @return Uint32 code to be used for the exit.
 */
Uint32 PEGASUS_CLI_LINKAGE cimcliExitRtn(Uint32 exitCode);
/** Set the expected exit code to some value.  This value will
 *  be tested by cimcliExit to determine if expected exit
 *  taken and put out message if not.
 *  @param expectedExitCode value to set.
 */
void setExpectedExitCode(Uint32 expectedExitCode);

/**
   Remap a long string into a multi-line string that can be
   positioned on a line starting at pos and with length defined
   for each line. Each output line consists of fill parameter to
   pos and max line length with characters from the input
   defined by lineLength parameter.

   The input string is recreated by tokenizing on the space character
   and filled from the left so that the returned string can be output
   as a multiline string starting at pos.

   New lines are created when the current line length exceeds
   lineLength or a EOL is identified in the input string.

   NOTE: The first line is started a 0 position, not the indent.
   This is based on assumption that it appends to other data on
   the first line.
   FUTURE: Extend so we start the first line either at 0 or the
   indent position

   @param input const char * - input string.  May contain EOL
                characters
   @param pos Uint32 Left margin for folded string.  Each line
              is filled with spaces up to this position.
   @param lineLength Uint32 max line position where input is
                     folded by adding EOL and spacing of new
                     line to pos
   @return String containing folded string
*/
String PEGASUS_CLI_LINKAGE foldString (const String& input,
    Uint32 pos,
    Uint32 lineLength);

/**
    Return a string description for each possible return code
*/
String rtnExitCodeToString(Uint32 rtnCode);

PEGASUS_NAMESPACE_END

#endif  /* _CLI_COMMON_H */
