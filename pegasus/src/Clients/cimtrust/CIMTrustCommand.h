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
// Author: Nag Boranna, Hewlett-Packard Company (nagaraja.boranna@hp.com)
//
// Modified By: David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//              Vijay Eli, vijayeli@in.ibm.com, fix for #2572
//              Sushma Fernandes,  Hewlett-Packard Company
//                  sushma@hp.com, PEP 259
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_CIMTrustCommand_h
#define Pegasus_CIMTrustCommand_h

#include <Pegasus/Common/String.h>
#include <Pegasus/Client/CIMClient.h>
#include <Clients/cliutils/Command.h>
#include <Clients/cliutils/CommandException.h>

PEGASUS_NAMESPACE_BEGIN

/**

    The CIMTrustCommand command provides a command line interface to manage X509
    certificates in a PEM format truststore. The command supports add, remove
    and list operations on the trust store. The CIMTrustCommand command
    requires the CIM Server to be running on the local host.

    @author  Hewlett-Packard Company

 */
class CIMTrustCommand : public Command
{
public:

    enum { DEFAULT_TIMEOUT_MILLISECONDS = 200000 };

    /**
        Constructs a CIMTrustCommand and initializes instance variables.
     */
    CIMTrustCommand ();

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
        Connect to cimserver.

        @param  client          the handle to CIMClient object

        @param  outPrintWriter  the ostream to which output should be
                                written

        @exception  Exception   if an error is encountered in creating
                                the connection
     */
    void _connectToServer(
        CIMClient& client,
        PEGASUS_STD(ostream)& outPrintWriter );

    /**
        Convert CIMDateTime to user-readable string of the format
        month day-of-month, year  hour:minute:second (value-hrs-GMT-offset)

        @param  cimDateTimeStr  CIM formated DateTime String

        @return  String user-readable date time string.
     */
    String _formatCIMDateTime(const String& cimDateTimeStr);


    /**
        Read certificate content.

        @param  certFilePath    the certificate file path

        @return  Buffer containing the certificate content
     */
    Buffer _readCertificateContent(const String &certFilePath);

    /**
       Add a new certificate to the trust store

       @param  client           the handle to CIMClient object

       @param  outPrintWriter   the ostream to which output should be written
     */
    void _addCertificate (
        CIMClient&               client,
        PEGASUS_STD(ostream)&    outPrintWriter);

    /**
       Remove an existing certificate from the trust store

       @param  client           the handle to CIMClient object

       @param  outPrintWriter   the ostream to which output should be written
     */
    void _removeCertificate (
        CIMClient&               client,
        PEGASUS_STD(ostream)&    outPrintWriter);

    /**
       List certificates in the trust store

       @param  client           the handle to CIMClient object

       @param  outPrintWriter   the ostream to which output should be written

     */
    void _listCertificates (
        CIMClient&               client,
        PEGASUS_STD(ostream)&    outPrintWriter);

    /**
        The certificate file to be added to the trust store.
     */
    String _certFile;

    /**
        Indicates that the certificate file is set.
     */
    Boolean _certFileSet;

    /**
        The user name to be mapped with the certificate being added
        to the trust store.
     */
    String _certUser;

    /**
        Indicates that the mapping certificate user is set.
     */
    Boolean _certUserSet;

    /**
        The issuer name of the certificate to be removed or listed
        from the trust store.
     */
    String _issuer;

    /**
        Indicates that the issuer name is set.
     */
    Boolean _issuerSet;

    /**
        The type of the certificate
     */
    String _type;

    /**
        Indicates that the type is set.
     */
    Boolean _typeSet;

    /**
        The serial number of the certificate to be removed or listed
        from the trust store for a specified issuer.
     */
    String _serialNumber;

    /**
        Indicates that the serial number is set.
     */
    Boolean _serialNumberSet;

    /**
        The subject of the certificate to be removed
        from the trust store for a specified issuer.
     */
    String _subject;

    /**
        Indicates that the subject name is set.
     */
    Boolean _subjectSet;

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
        The option character used to display help info.
     */
    static const char   _OPTION_HELP;

    /**
        The option character used to display version info.
     */
    static const char   _OPTION_VERSION;

    /**
        The option character used to specify the certificate file.
     */
    static const char   _OPTION_CERTFILE;

    /**
        The option character used to specify the certificate user name.
     */
    static const char   _OPTION_CERTUSER;

    /**
        The option character used to specify the issuer name.
     */
    static const char   _OPTION_ISSUER;

    /**
        The option character used to specify the serial number.
     */
    static const char   _OPTION_SERIALNUMBER;

    /**
        The option character used to specify the certificate type.
     */
    static const char _OPTION_TYPE;

    /**
        Specifies the certificate types.
     */
    enum _CERTIFICATE_TYPES
    {
        _CERTIFICATE_TYPE_UNKNOWN = 0,
        _CERTIFICATE_TYPE_AUTHORITY = 2,
        _CERTIFICATE_TYPE_AUTHORITY_END_ENTITY = 3,
        _CERTIFICATE_TYPE_SELF_SIGNED_IDENTITY = 4
    };

    /**
        The character used to specify the authority type.
     */
    static const String _ARG_TYPE_AUTHORITY;

    /**
        The character used to specify the authority
        issued end-entity type.
     */
    static const String _ARG_TYPE_AUTHORITY_END_ENTITY;

    /**
        The character used to specify the self-signed
        identity type.
     */
    static const String _ARG_TYPE_SELF_SIGNED_IDENTITY;

    /**
        The option character used to specify the subject.
     */
    static const char   _OPTION_SUBJECT;

    /**
        Label for the usage string for this command.
     */
    static const char   _USAGE [];

    /**
        This constant signifies that an operation option has not been recorded
     */
    static const Uint32 _OPERATION_TYPE_UNINITIALIZED;

    /**
        The constant representing a add operation
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

#endif /* Pegasus_CIMTrustCommand_h */
