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

#ifndef Pegasus_CIMCRLCommand_h
#define Pegasus_CIMCRLCommand_h

#include <Pegasus/Common/String.h>
#include <Pegasus/Client/CIMClient.h>
#include <Clients/cliutils/Command.h>
#include <Clients/cliutils/CommandException.h>

PEGASUS_NAMESPACE_BEGIN

/**

    The CIMCRLCommand command provides a command line interface to manage X509
    CRLs in a PEM format CRL store.  The command supports add, remove and
    list operations on the target CRL store. The CIMCRLCommand command
    requires the CIM Server to be running on the local host.

    @author  Hewlett-Packard Company

 */
class CIMCRLCommand : public Command
{
public:

    enum { DEFAULT_TIMEOUT_MILLISECONDS = 200000 };

    /**
        Constructs a CIMCRLCommand and initializes instance variables.
     */
    CIMCRLCommand ();

    /**
        Parses the command line, validates the options, and sets instance
        variables based on the option arguments.

        @param  argc   the number of command line arguments
        @param  argv   the string vector of command line arguments

        @exception  CommandFormatException  if an error is encountered in
                                            parsing the command line
     */
    void setCommand (Uint32 argc, char* argv []);

    /**
        Executes the command and writes the results to the PrintWriters.

        @param  outPrintWriter    the ostream to which output should be
                                  written
        @param  errPrintWriter    the ostream to which error output should be
                                  written

        @return  0                if the command is successful
                 1                if an error occurs in executing the command
     */
    Uint32 execute (
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter);

    /**
        The command name.
     */
    static const char   COMMAND_NAME [];

private:

    /**
        Convert CIMDateTime to user-readable string of the format
        month day-of-month, year  hour:minute:second (value-hrs-GMT-offset)

        @param  cimDateTimeStr  CIM formated DateTime String

        @return  String user-readable date time string.
     */
    String _formatCIMDateTime(const String& cimDateTimeStr);


    /**
        Read CRL content.

        @param  crlFilePath    the CRL file path

        @return  Buffer containing the certificate content

        @exception  Exception   if an error is encountered
                                in reading the certificate file
     */
    Buffer _readCRLContent(const String &crlFilePath);

    /**
       Add a CRL to the CRL store

       @param  client           the handle to CIMClient object

       @param  outPrintWriter   the ostream to which output should be written
     */
    void _addCRL (
        CIMClient&               client,
        PEGASUS_STD(ostream)&    outPrintWriter);

    /**
       Remove an existing CRL from the CRL store

       @param  client           the handle to CIMClient object

       @param  outPrintWriter   the ostream to which output should be written
     */
    void _removeCRL (
        CIMClient&               client,
        PEGASUS_STD(ostream)&    outPrintWriter);

    /**
       List CRL's in the CRL store

       @param  client           the handle to CIMClient object

       @param  outPrintWriter   the ostream to which output should be written
     */
    void _listCRL (
        CIMClient&               client,
        PEGASUS_STD(ostream)&    outPrintWriter);

    /**
        The crl file to be added to the CRL store.
     */
    String _crlFile;

    /**
        Indicates that the crl file is set.
     */
    Boolean _crlFileSet;

    /**
        The issuer name of the CRL to be removed or listed
        from the CRL store.
     */
    String _issuer;

    /**
        Indicates that the issuer name is set.
     */
    Boolean _issuerSet;

    /**
        The timeout value to be used in milliseconds.
        The default timeout value is DEFAULT_TIMEOUT_MILLISECONDS.
     */
    Uint32 _timeout;

    /**
        The type of operation specified on the command line.
     */
    Uint32 _operationType;

    /**
        The usage string.
     */
    String _usage;

    /**
        The option character used to specify the add option.
     */
    static const char   _OPTION_ADD;

    /**
        The option character used to specify the remove option.
     */
    static const char   _OPTION_REMOVE;

    /**
        The option character used to specify the list option.
     */
    static const char   _OPTION_LIST;

    /**
        The option character used to specify the issuer name.
     */
    static const char   _OPTION_ISSUERNAME;

    /**
        The option character used to specify the crl file.
     */
    static const char   _OPTION_CRLFILE;

    /**
        Label for the usage string for this command.
     */
    static const char   _USAGE [];

    /**
        This constant signifies that an operation option has not been recorded
     */
    static const Uint32 _OPERATION_TYPE_UNINITIALIZED;

    /**
        The constant representing an add operation
     */
    static const Uint32 _OPERATION_TYPE_ADD;

    /**
        The constant representing a remove operation
     */
    static const Uint32 _OPERATION_TYPE_REMOVE;

    /**
        The constant representing a list operation
     */
    static const Uint32 _OPERATION_TYPE_LIST;

    /**
        The constant representing a help operation
     */
    static const Uint32 _OPERATION_TYPE_HELP;

    /**
        The constant representing a version display operation
     */
    static const Uint32 _OPERATION_TYPE_VERSION;

};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMCRLCommand_h */
