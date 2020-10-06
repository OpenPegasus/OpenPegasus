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

// define asprintf used to implement ultostr on Linux
#if defined(PEGASUS_LINUX_GENERIC_GNU)
#define _GNU_SOURCE
#include <features.h>
#include <stdio.h>
#endif

#include <iostream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <Pegasus/getoopt/getoopt.h>
#include <Clients/cliutils/CommandException.h>
#include "CIMTrustCommand.h"

PEGASUS_NAMESPACE_BEGIN

//l10n
/**
 * The CLI message resource name
 */
static const char MSG_PATH []                  = "pegasus/pegasusCLI";

/**
    The command name.
 */
const char   CIMTrustCommand::COMMAND_NAME []      = "cimtrust";

/**
    Label for the usage string for this command.
 */
const char   CIMTrustCommand::_USAGE []            = "Usage: ";

/**
    The option character used to specify add certificate.
*/
const char   CIMTrustCommand::_OPTION_ADD          = 'a';

/**
    The option character used to specify remove certificate.
*/
const char   CIMTrustCommand::_OPTION_REMOVE       = 'r';

/**
    The option character used to specify list certificates.
*/
const char   CIMTrustCommand::_OPTION_LIST         = 'l';

/**
    The option character used to specify the certificate file path.
 */
const char   CIMTrustCommand::_OPTION_CERTFILE     = 'f';

/**
    The option character used to specify the certificate user name.
 */
const char   CIMTrustCommand::_OPTION_CERTUSER     = 'U';

/**
    The option character used to specify the issuer name.
 */
const char   CIMTrustCommand::_OPTION_ISSUER   = 'i';

/**
    The option character used to specify the serial number.
 */
const char   CIMTrustCommand::_OPTION_SERIALNUMBER = 'n';

/**
    The option character used to specify the subject.
 */
const char   CIMTrustCommand::_OPTION_SUBJECT = 'S';

/**
    The option character used to specify the certificate type.
 */
const char   CIMTrustCommand::_OPTION_TYPE= 'T';

/**
    The option character used to specify the authority type.
 */
const String   CIMTrustCommand::_ARG_TYPE_AUTHORITY = "a";

/**
    The option character used to specify the authority issued end-entity type.
 */
const String   CIMTrustCommand::_ARG_TYPE_AUTHORITY_END_ENTITY = "e";

/**
    The option character used to specify the self-signed identity type.
 */
const String   CIMTrustCommand::_ARG_TYPE_SELF_SIGNED_IDENTITY= "s";

/**
    Constant to represent unknown type certificate.
 */
const String   TYPE_UNKNOWN = "Unknown";

/**
    Constant to represent authority type certificate.
 */
const String   TYPE_AUTHORITY_STR = "Authority";

/**
    Constant to represent authority issued end-entity type certificate.
 */
const String   TYPE_AUTHORITY_END_ENTITY_STR = "Authority issued end-entity";

/**
    Constant to represent self-signed identity type certificate.
 */
const String   TYPE_SELF_SIGNED_IDENTITY_STR = "Self-signed identity";

/**
    This constant signifies that an operation option has not
    been recorded
*/
const Uint32 CIMTrustCommand::_OPERATION_TYPE_UNINITIALIZED = 0;

/**
    The constant representing an add operation
*/
const Uint32 CIMTrustCommand::_OPERATION_TYPE_ADD = 1;

/**
    The constant representing a remove operation
*/
const Uint32 CIMTrustCommand::_OPERATION_TYPE_REMOVE = 2;

/**
    The constant representing a list operation
*/
const Uint32 CIMTrustCommand::_OPERATION_TYPE_LIST = 4;

/**
    The constant representing a help operation
*/
const Uint32 CIMTrustCommand::_OPERATION_TYPE_HELP = 5;

/**
    The constant representing a version display operation
*/
const Uint32 CIMTrustCommand::_OPERATION_TYPE_VERSION = 6;


/**
    The name of the Method that implements add certificate to the
    trust store.
*/
static const CIMName ADD_CERTIFICATE_METHOD = CIMName ("addCertificate");

/**
    This constant represents the name of the Issuer name
    property in the schema
*/
static const CIMName PROPERTY_NAME_ISSUER = CIMName ("IssuerName");

/**
    This constant represents the name of the Serial number
    property in the schema
*/
static const CIMName PROPERTY_NAME_SERIALNUMBER = CIMName ("SerialNumber");

/**
    This constant represents the name of the Subject name type
    property in the schema
*/
static const CIMName PROPERTY_NAME_SUBJECTNAME  = CIMName ("SubjectName");

/**
    This constant represents the name of the RegisteredUserName
    property in the schema
*/
static const CIMName PROPERTY_NAME_REGISTERED_USER_NAME
                                            = CIMName ("RegisteredUserName");

/**
    This constant represents the name of the type
    property in the schema
*/
static const CIMName PROPERTY_NAME_TYPE  = CIMName ("CertificateType");

/**
    This constant represents the name of the notBefore type
    property in the schema
*/
static const CIMName PROPERTY_NAME_NOTBEFORE    = CIMName ("NotBefore");

/**
    This constant represents the name of the notAfter type
    property in the schema
*/
static const CIMName PROPERTY_NAME_NOTAFTER     = CIMName ("NotAfter");

/**
    This constant represents the name of the lastUpdate type
    property in the schema
*/
static const CIMName PROPERTY_NAME_LASTUPDATE   = CIMName ("LastUpdate");

/**
    This constant represents the name of the nextUpdate type
    property in the schema
*/
static const CIMName PROPERTY_NAME_NEXTUPDATE   = CIMName ("NextUpdate");

/**
    The input parameter name for certificate content.
*/
static const char   CERT_CONTENTS []    = "certificateContents";

/**
    The input parameter name for registered certificate user.
*/
static const char   CERT_USERNAME []    = "userName";

/**
    The input parameter name for certificate type.
*/
static const char   CERT_TYPE []    = "certificateType";

/**
    The constants representing the display messages.
*/
static const char   LONG_HELP []  = "help";

static const char   LONG_VERSION []  = "version";

static const char REQUIRED_ARGS_MISSING [] =
                "Required arguments missing.";

static const char REQUIRED_ARGS_MISSING_KEY [] =
                "Clients.cimtrust.CIMTrustCommand.REQUIRED_ARGS_MISSING";

static const char CANNOT_CONNECT_CIMSERVER_NOT_RUNNING [] =
                "Cannot connect to CIM Server; CIM Server may not be running.";

static const char CANNOT_CONNECT_CIMSERVER_NOT_RUNNING_KEY [] =
                "Clients.cimtrust."
                    "CIMTrustCommand.CANNOT_CONNECT_CIMSERVER_NOT_RUNNING";

static const char CONNECTION_TIMEOUT [] =
                "Connection timed out.";

static const char CONNECTION_TIMEOUT_KEY [] =
                "Clients.cimtrust.CIMTrustCommand.CONNECTION_TIMEOUT";

static const char ADD_CERT_FAILURE []    =
                "Failed to add certificate.";

static const char ADD_CERT_FAILURE_KEY []    =
                "Clients.cimtrust.CIMTrustCommand.ADD_CERT_FAILURE";

static const char ADD_CERTIFICATE_SUCCESS [] =
                "Certificate added successfully.";

static const char ADD_CERTIFICATE_SUCCESS_KEY [] =
                "Clients.cimtrust.CIMTrustCommand.ADD_CERT_SUCCESS";

static const char REMOVE_CERTIFICATE_SUCCESS [] =
                "Certificate removed successfully.";

static const char REMOVE_CERTIFICATE_SUCCESS_KEY [] =
                "Clients.cimtrust.CIMTrustCommand.REMOVE_CERT_SUCCESS";

static const char CERT_ALREADY_EXISTS []        =
                "Specified certificate already exists.";

static const char CERT_ALREADY_EXISTS_KEY []        =
                "Clients.cimtrust.CIMTrustCommand.CERT_ALREADY_EXISTS";

static const char CERT_NOT_FOUND [] =
                "Specified certificate does not exist.";

static const char CERT_NOT_FOUND_KEY [] =
                "Clients.cimtrust.CIMTrustCommand.CERT_NOT_FOUND";

static const char CERT_SCHEMA_NOT_LOADED []  =
                "Please restore the internal repository on the CIM Server.";

static const char CERT_SCHEMA_NOT_LOADED_KEY []  =
                "Clients.cimtrust.CIMTrustCommand.CERT_SCHEMA_NOT_LOADED";

static const char REMOVE_CERT_FAILURE [] =
                "Failed to remove certificate.";

static const char REMOVE_CERT_FAILURE_KEY []  =
                "Clients.cimtrust.CIMTrustCommand.REMOVE_CERT_FAILURE";

static const char LIST_CERT_FAILURE [] =
                "Failed to list certificate.";

static const char LIST_CERT_FAILURE_KEY [] =
                "Clients.cimtrust.CIMTrustCommand.LIST_CERT_FAILURE";

static const char CERT_WITHOUT_ASSOCIATED_USER_KEY [] =
    "Clients.cimtrust.CIMTrustCommand.CERT_WITHOUT_ASSOCIATED_USER";

static const char CERT_WITHOUT_ASSOCIATED_USER [] =
    "NOTE: No user name will be associated with the certificate"
    " in the truststore.";

static const char INVALID_SYSTEM_USER [] =
                "Specified user is not a valid system user.";

static const char INVALID_SYSTEM_USER_KEY [] =
                "Clients.cimtrust.CIMTrustCommand.INVALID_SYSTEM_USER";

// Return codes

static const Uint32 RC_CONNECTION_FAILED = 2;

static const Uint32 RC_CONNECTION_TIMEOUT = 3;

static const Uint32 RC_CERTIFICATE_ALREADY_EXISTS = 4;

static const Uint32 RC_CERTIFICATE_DOES_NOT_EXIST = 5;

static const Uint32 RC_INVALID_SYSTEM_USER = 6;

/**

    Constructs a CIMTrustCommand and initializes instance variables.

 */
CIMTrustCommand::CIMTrustCommand ()
{
    //
    // Initialize the instance variables.
    //
    _operationType       = _OPERATION_TYPE_UNINITIALIZED;

    _certFileSet         = false;
    _certUserSet         = false;
    _issuerSet           = false;
    _serialNumberSet     = false;
    _subjectSet          = false;
    _typeSet             = false;

    _timeout             = DEFAULT_TIMEOUT_MILLISECONDS;

    //
    // build Usage string.
    //
    _usage = String (_USAGE);
    _usage.append (COMMAND_NAME);

    //
    // Add option 1
    //
    _usage.append (" -").append (_OPTION_ADD);
    _usage.append (" [ -").append (_OPTION_CERTUSER).append (" certuser")
         .append (" ]");
    _usage.append (" -").append (_OPTION_CERTFILE).append (" certfile");
    _usage.append (" -").append (_OPTION_TYPE).append (" ( ")
        .append(_ARG_TYPE_AUTHORITY);
    _usage.append (" | ").append (_ARG_TYPE_AUTHORITY_END_ENTITY);
    _usage.append (" | ").append (_ARG_TYPE_SELF_SIGNED_IDENTITY).append(" ) ");
    _usage.append ("\n");


    //
    // Remove option 1
    //
    //_usage.append ("                   -").append (_OPTION_REMOVE);
    _usage.append("       ").append (COMMAND_NAME);
    _usage.append (" -").append (_OPTION_REMOVE);
    _usage.append (" -").append (_OPTION_ISSUER).append (" issuer");
    _usage.append (" ( -").append (_OPTION_SERIALNUMBER).
        append (" serialnumber");
    _usage.append (" | -").append (_OPTION_SUBJECT).append (" subject )");
    _usage.append ("\n");

    //
    // List option 1
    //
    //_usage.append ("                   -").append (_OPTION_LIST);
    _usage.append("       ").append (COMMAND_NAME);
    _usage.append (" -").append (_OPTION_LIST);
    _usage.append (" [ -").append (_OPTION_ISSUER).append (" issuer");
    _usage.append (" [ -").append (_OPTION_SERIALNUMBER)
         .append (" serialnumber");
    _usage.append (" | -").append (_OPTION_SUBJECT)
         .append (" subject")
         .append (" ] ]");
    _usage.append ("\n");

    //
    // Version option
    //
    _usage.append("       ").append (COMMAND_NAME);
    _usage.append (" --").append (LONG_VERSION)
         .append("\n");

    //
    // Help option
    //
    _usage.append("       ").append (COMMAND_NAME);
    _usage.append (" --").append (LONG_HELP)
         .append("\n");

    //
    // Options description
    //
    _usage.append("Options : \n");
    _usage.append("    -a              ");
    _usage.append("- Adds the specified certificate to the trust store\n");

    _usage.append("    -r              ");
    _usage.append("- Removes the specified certificate from the trust store\n");

    _usage.append("    -l              ");
    _usage.append("- Displays the certificates in the trust store\n");

    _usage.append("    -f certfile     ");
    _usage.append("- Specifies the PEM format file containing an X509\n"
        "                      certificate\n");

    _usage.append("    -U certuser     ");
    _usage.append("- Specifies the user name to be associated with the\n"
        "                      certificate\n");

    _usage.append("    -T (a|e|s)      ");
    _usage.append("- Specifies the certificate type as follows:\n"
        "                      -a (authority): root/intermediate authority\n"
        "                         certificates. This type added to trusted\n"
        "                         certificate store. certuser optional. If\n"
        "                         no certUser specified certificate may not \n"
        "                         be used to authenticate user.\n"
        "                      -e (authority issued end-entity): Certificates\n"
        "                         of this type are not added to trusted\n"
        "                         certificate store. certuser is required.\n"
        "                      -s (Self-signed identity certificate): Added\n"
        "                         to trusted certificate store. certuser is\n"
        "                         required.\n");

    _usage.append("    -i issuer       ");
    _usage.append("- Specifies the certificate issuer name\n");

    _usage.append("    -n serialnumber ");
    _usage.append("- Specifies the certificate serial number\n");

    _usage.append("    -S subject      ");
    _usage.append("- Specifies the certificate subject\n");

    _usage.append("    --help          - Display this help message\n");
    _usage.append("    --version       - Display CIM Server version number\n");

    _usage.append("\nUsage note: The cimtrust command requires that ");
    _usage.append("the CIM Server is running.\n");

//l10n localize usage
#ifdef PEGASUS_HAS_ICU

    MessageLoaderParms menuparms(
        "Clients.cimtrust.CIMTrustCommand.MENU.STANDARD", _usage);
    menuparms.msg_src_path = MSG_PATH;
    _usage = MessageLoader::getMessage(menuparms);

#endif

    setUsage (_usage);
}

/**
    Connect to cimserver.

    @param  client             the handle to CIMClient object

    @param  outPrintWriter     the ostream to which output should be
                               written
 */
void CIMTrustCommand::_connectToServer( CIMClient& client,
                                    ostream& outPrintWriter )
{
    client.connectLocal();
}


/**
    Convert CIMDateTime to user-readable string of the format
    month day-of-month, year  hour:minute:second (value-hrs-GMT-offset)

    @param  cimDateTimeStr  CIM formated DateTime String

    @return  String user-readable date time string.
 */
String CIMTrustCommand::_formatCIMDateTime(const String& cimDateTimeStr)
{
   Uint32 year = 0;
   Uint32 month = 0;
   Uint32 day = 0;
   Uint32 hour = 0;
   Uint32 minute = 0;
   Uint32 second = 0;
   Uint32 microsecond = 0;
   Sint32 timezone = 0;

   sscanf(cimDateTimeStr.getCString(), "%04u%02u%02u%02u%02u%02u.%06u%04d",
          &year, &month, &day, &hour, &minute, &second,
          &microsecond, &timezone);

   char monthString[5];

   switch (month)
   {
      case 1 :
          sprintf(monthString, "Jan");
          break;
      case 2 :
          sprintf(monthString, "Feb");
          break;
      case 3 :
          sprintf(monthString, "Mar");
          break;
      case 4 :
          sprintf(monthString, "Apr");
          break;
      case 5 :
          sprintf(monthString, "May");
          break;
      case 6 :
          sprintf(monthString, "Jun");
          break;
      case 7 :
          sprintf(monthString, "Jul");
          break;
      case 8 :
          sprintf(monthString, "Aug");
          break;
      case 9 :
          sprintf(monthString, "Sep");
          break;
      case 10 :
          sprintf(monthString, "Oct");
          break;
      case 11 :
          sprintf(monthString, "Nov");
          break;
      case 12 :
          sprintf(monthString, "Dec");
          break;

      // covered all known cases, if get to default, just
      // return the input string as received.
      default :
          return (cimDateTimeStr);
   }

   char dateTimeStr[80];
   sprintf(dateTimeStr, "%s %u, %u  %u:%02u:%02u (%+03d%02u)",
           monthString, day, year, hour, minute, second,
           timezone/60, timezone%60);

   return String(dateTimeStr);
}


/**
    Read certificate content.

    @param  certFilePath     the certificate file path

    @return  Buffer containing the certificate content
 */
Buffer CIMTrustCommand::_readCertificateContent(const String &certFilePath)
{
    Buffer content;

    //
    //  Check that cert file exists
    //
    if (!FileSystem::exists(certFilePath))
    {
        throw NoSuchFile(certFilePath);
    }

    //
    //  Check that cert file is readable
    //
    if (!FileSystem::canRead(certFilePath))
    {
        throw FileNotReadable(certFilePath);
    }

    //
    //  Load file content to memory
    //
    FileSystem::loadFileToMemory(content, certFilePath);

    return content;
}


/**
   Add a new certificate to the trust store

   @param  client           the handle to CIMClient object

   @param  outPrintWriter   the ostream to which output should be written
 */
void CIMTrustCommand::_addCertificate (
    CIMClient&   client,
    ostream&     outPrintWriter)
{
    Buffer              content;
    Array<CIMKeyBinding>   kbArray;
    CIMKeyBinding          kb;
    Array<CIMParamValue>   inParams;
    Array<CIMParamValue>   outParams;
    Uint16                 certificateType;

    content = _readCertificateContent(_certFile);

    String contentStr = String(content.getData());

    if (_type == _ARG_TYPE_AUTHORITY)
    {
        certificateType = _CERTIFICATE_TYPE_AUTHORITY;
    }
    else if ( _type == _ARG_TYPE_AUTHORITY_END_ENTITY )
    {
        certificateType = _CERTIFICATE_TYPE_AUTHORITY_END_ENTITY;
    }
    else
    {
        PEGASUS_ASSERT(_type == _ARG_TYPE_SELF_SIGNED_IDENTITY);
        certificateType = _CERTIFICATE_TYPE_SELF_SIGNED_IDENTITY;
    }

    //
    // Build the input params
    //
    inParams.append ( CIMParamValue ( CERT_CONTENTS,
                                      CIMValue (contentStr)));
    inParams.append ( CIMParamValue ( CERT_USERNAME,
                                      CIMValue (_certUser)));
    inParams.append ( CIMParamValue ( CERT_TYPE,
                                      CIMValue (certificateType)));

    CIMObjectPath reference(
        String::EMPTY, PEGASUS_NAMESPACENAME_CERTIFICATE,
        PEGASUS_CLASSNAME_CERTIFICATE, kbArray);

    //
    // If an associated username has not been specified, display an
    // informational message.
    //
    if ( !_certUserSet )
    {
          outPrintWriter << localizeMessage(MSG_PATH,
                            CERT_WITHOUT_ASSOCIATED_USER_KEY,
                            CERT_WITHOUT_ASSOCIATED_USER) << endl;
    }

    //
    // Call the invokeMethod with the input parameters
    //

    client.invokeMethod (
        PEGASUS_NAMESPACENAME_CERTIFICATE,
        reference,
        ADD_CERTIFICATE_METHOD,
        inParams,
        outParams );

    outPrintWriter << localizeMessage(MSG_PATH, ADD_CERTIFICATE_SUCCESS_KEY,
        ADD_CERTIFICATE_SUCCESS) << endl;
}

/**
   Remove an existing certificate from the trust store

   @param  client           the handle to CIMClient object

   @param  outPrintWriter   the ostream to which output should be written
 */
void CIMTrustCommand::_removeCertificate (
    CIMClient&   client,
    ostream&     outPrintWriter)
{
    Array<CIMKeyBinding> kbArray;
    CIMKeyBinding        kb;

    //
    // Build the input params
    //
    kb.setName(PROPERTY_NAME_ISSUER);
    kb.setValue(_issuer);
    kb.setType(CIMKeyBinding::STRING);

    kbArray.append(kb);

    if (_serialNumberSet)
    {
        kb.setName(PROPERTY_NAME_SERIALNUMBER);
        kb.setValue(_serialNumber);
        kb.setType(CIMKeyBinding::STRING);

        kbArray.append(kb);
    }
    else
    {
        //
        // Pass the subject name
        //
        kb.setName(PROPERTY_NAME_SUBJECTNAME);
        kb.setValue(_subject);
        kb.setType(CIMKeyBinding::STRING);

        kbArray.append(kb);
    }

    CIMObjectPath reference(
        String::EMPTY, PEGASUS_NAMESPACENAME_CERTIFICATE,
        PEGASUS_CLASSNAME_CERTIFICATE, kbArray);

    client.deleteInstance(
        PEGASUS_NAMESPACENAME_CERTIFICATE,
        reference);

    outPrintWriter << localizeMessage(MSG_PATH, REMOVE_CERTIFICATE_SUCCESS_KEY,
        REMOVE_CERTIFICATE_SUCCESS) << endl;
}

/**
   List certificates in the trust store

   @param  client           the handle to CIMClient object

   @param  outPrintWriter   the ostream to which output should be written
 */
void CIMTrustCommand::_listCertificates (
    CIMClient&   client,
    ostream&     outPrintWriter)
{
    Array<CIMInstance> certificateNamedInstances;

    //
    // get all the instances of class PG_SSLCertificate
    //
    certificateNamedInstances =
        client.enumerateInstances(
            PEGASUS_NAMESPACENAME_CERTIFICATE,
            PEGASUS_CLASSNAME_CERTIFICATE);

    //
    // copy all the certificate contents
    //
    Uint32 numberInstances = certificateNamedInstances.size();
    Boolean issuerFound = false;

    for (Uint32 i = 0; i < numberInstances; i++)
    {
        CIMInstance& certificateInstance =
            certificateNamedInstances[i];

        String issuer;
        String serialNumber;
        String subjectName;
        String registeredUserName;
        Uint16 type;
        String typeStr;
        CIMDateTime notBefore;
        String notBeforeStr;
        CIMDateTime notAfter;
        String notAfterStr;
        CIMProperty prop;

        //
        // Check if issuer name and serial number are specified
        // and they match
        //
        Uint32 pos = certificateInstance.findProperty(PROPERTY_NAME_ISSUER);
        if (pos != PEG_NOT_FOUND)
        {
            prop = certificateInstance.getProperty(pos);
            prop.getValue().get(issuer);
        }

        pos = certificateInstance.findProperty(PROPERTY_NAME_SERIALNUMBER);
        if (pos != PEG_NOT_FOUND)
        {
            prop = certificateInstance.getProperty(pos);
            prop.getValue().get(serialNumber);
        }

        pos = certificateInstance.findProperty(PROPERTY_NAME_SUBJECTNAME);
        if (pos != PEG_NOT_FOUND)
        {
            prop = certificateInstance.getProperty(pos);
            prop.getValue().get(subjectName);
        }

        if (_issuerSet)
        {
            if (String::equal(_issuer, issuer))
            {
                if (_serialNumberSet &&
                    !String::equal(_serialNumber, serialNumber))
                {
                    continue;
                }
                else if (_subjectSet &&
                    !String::equal(_subject, subjectName))
                {
                    continue;
                }
                else
                {
                    issuerFound = true;
                }
            }
            else
            {
                continue;
            }
        }

        //
        // Get the remaining properties and display them.
        //
        pos = certificateInstance.findProperty(
                        PROPERTY_NAME_REGISTERED_USER_NAME);
        if (pos != PEG_NOT_FOUND)
        {
            prop = certificateInstance.getProperty(pos);
            prop.getValue().get(registeredUserName);
        }

        pos = certificateInstance.findProperty(PROPERTY_NAME_NOTBEFORE);
        if (pos != PEG_NOT_FOUND)
        {
            prop = certificateInstance.getProperty(pos);
            prop.getValue().get(notBefore);
            notBeforeStr = _formatCIMDateTime(notBefore.toString());
        }

        pos = certificateInstance.findProperty(PROPERTY_NAME_NOTAFTER);
        if (pos != PEG_NOT_FOUND)
        {
            prop = certificateInstance.getProperty(pos);
            prop.getValue().get(notAfter);
            notAfterStr = _formatCIMDateTime(notAfter.toString());
        }

        pos = certificateInstance.findProperty(PROPERTY_NAME_TYPE);
        if (pos != PEG_NOT_FOUND)
        {
            prop = certificateInstance.getProperty(pos);
            prop.getValue().get(type);

            if (type == _CERTIFICATE_TYPE_AUTHORITY)
            {
                typeStr = TYPE_AUTHORITY_STR;
            }
            else if (type == _CERTIFICATE_TYPE_AUTHORITY_END_ENTITY)
            {
                typeStr = TYPE_AUTHORITY_END_ENTITY_STR;
            }
            else if (type == _CERTIFICATE_TYPE_SELF_SIGNED_IDENTITY)
            {
                typeStr = TYPE_SELF_SIGNED_IDENTITY_STR;
            }
            else if (type == _CERTIFICATE_TYPE_UNKNOWN)
            {
                typeStr = TYPE_UNKNOWN;
            }
        }

        //
        // Display the certificate content
        //
        outPrintWriter << "Issuer: " << issuer << endl;
        outPrintWriter << "Serial Number: " << serialNumber << endl;
        outPrintWriter << "Subject: " << subjectName << endl;
        outPrintWriter << "Registered User Name: "
                       << registeredUserName << endl;
        outPrintWriter << "Type: "
                       << typeStr << endl;
        outPrintWriter << "Validity:" << endl;
        outPrintWriter << "    NotBefore: " << notBeforeStr << endl;
        outPrintWriter << "    NotAfter : " << notAfterStr << endl ;
        outPrintWriter <<
            "---------------------------------------------"<< endl;
    }

    if (_issuerSet && !issuerFound)
    {
         CIMException ce(CIM_ERR_NOT_FOUND);
         throw ce;
    }
}

/**
    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.

    @param   argc  the number of command line arguments

    @param   argv  the string vector of command line arguments

    @exception  CommandFormatException  if an error is encountered in parsing
                                        the command line
 */
void CIMTrustCommand::setCommand (Uint32 argc, char* argv [])
{
    Uint32         i               = 0;
    Uint32         c               = 0;
    String         timeoutStr;
    String         serialNumberStr;
    String         optString;
    getoopt        getOpts;

    //
    //  Construct optString
    //
    optString.append (_OPTION_ADD);
    optString.append (getoopt::NOARG);
    optString.append (_OPTION_CERTFILE);
    optString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append (_OPTION_CERTUSER);
    optString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append (_OPTION_TYPE);
    optString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);

    optString.append (_OPTION_REMOVE);
    optString.append (getoopt::NOARG);
    optString.append (_OPTION_ISSUER);
    optString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append (_OPTION_SERIALNUMBER);
    optString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append (_OPTION_SUBJECT);
    optString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);

    optString.append (_OPTION_LIST);
    optString.append (getoopt::NOARG);
    optString.append (_OPTION_ISSUER);
    optString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append (_OPTION_SERIALNUMBER);
    optString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    optString.append (_OPTION_SUBJECT);
    optString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);

    //
    //  Initialize and parse getOpts
    //
    getOpts = getoopt ();
    getOpts.addFlagspec (optString);

    //
    // Add long flag options for 'help' and 'version'
    //
    getOpts.addLongFlagspec (LONG_HELP, getoopt::NOARG);
    getOpts.addLongFlagspec (LONG_VERSION, getoopt::NOARG);

    getOpts.parse (argc, argv);

    if (getOpts.hasErrors ())
    {
        throw CommandFormatException(getOpts.getErrorStrings()[0]);
    }
    _operationType = _OPERATION_TYPE_UNINITIALIZED;

    //
    //  Get options and arguments from the command line
    //
    for (i =  getOpts.first (); i <  getOpts.last (); i++)
    {
        if (getOpts [i].getType () == Optarg::LONGFLAG)
        {
            if (getOpts [i].getopt () == LONG_HELP)
            {
                if (_operationType != _OPERATION_TYPE_UNINITIALIZED)
                {
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(String(LONG_HELP));
                }
                _operationType = _OPERATION_TYPE_HELP;
            }
            else if (getOpts [i].getopt () == LONG_VERSION)
            {
                if (_operationType != _OPERATION_TYPE_UNINITIALIZED)
                {
                    //
                    // More than one operation option was found
                    //
                    throw UnexpectedOptionException(String(LONG_VERSION));
                }

               _operationType = _OPERATION_TYPE_VERSION;
            }
        }
        else if (getOpts [i].getType () == Optarg::REGULAR)
        {
            //
            //  The cimtrust command has no non-option argument options
            //
            throw UnexpectedArgumentException(getOpts[i].Value());
        }
        else /* getOpts [i].getType () == FLAG */
        {
            c = getOpts [i].getopt () [0];

            switch (c)
            {
                case _OPTION_ADD:
                {
                    if (_operationType != _OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(_OPTION_ADD);
                    }

                    if (getOpts.isSet (_OPTION_ADD) > 1)
                    {
                        //
                        // More than one add option was found
                        //
                        throw DuplicateOptionException(_OPTION_ADD);
                    }

                    _operationType = _OPERATION_TYPE_ADD;
                    break;
                }

                case _OPTION_REMOVE:
                {
                    if (_operationType != _OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(_OPTION_REMOVE);
                    }

                    if (getOpts.isSet (_OPTION_REMOVE) > 1)
                    {
                        //
                        // More than one remove option was found
                        //
                        throw DuplicateOptionException(_OPTION_REMOVE);
                    }

                    _operationType = _OPERATION_TYPE_REMOVE;
                    break;
                }

                case _OPTION_LIST:
                {
                    if (_operationType != _OPERATION_TYPE_UNINITIALIZED)
                    {
                        //
                        // More than one operation option was found
                        //
                        throw UnexpectedOptionException(_OPTION_LIST);
                    }

                    if (getOpts.isSet (_OPTION_LIST) > 1)
                    {
                        //
                        // More than one list option was found
                        //
                        throw DuplicateOptionException(_OPTION_LIST);
                    }

                    _operationType = _OPERATION_TYPE_LIST;
                    break;
                }

                case _OPTION_CERTUSER:
                {
                    if (getOpts.isSet (_OPTION_CERTUSER) > 1)
                    {
                        //
                        // More than one certificate user option was found
                        //
                        throw DuplicateOptionException(_OPTION_CERTUSER);
                    }

                    _certUser = getOpts [i].Value ();
                    _certUserSet = true;
                    break;
                }

                case _OPTION_CERTFILE:
                {
                    if (getOpts.isSet (_OPTION_CERTFILE) > 1)
                    {
                        //
                        // More than one cert file option was found
                        //
                        throw DuplicateOptionException(_OPTION_CERTFILE);
                    }

                    _certFile = getOpts [i].Value ();
                    _certFileSet = true;
                    break;
                }

                case _OPTION_ISSUER:
                {
                    if (getOpts.isSet (_OPTION_ISSUER) > 1)
                    {
                        //
                        // More than one issuer name option was found
                        //
                        throw DuplicateOptionException(_OPTION_ISSUER);
                    }

                    _issuer = getOpts [i].Value ();
                    _issuerSet = true;
                    break;
                }

                case _OPTION_TYPE:
                {
                    if (getOpts.isSet (_OPTION_TYPE) > 1)
                    {
                        //
                        // More than one type option was found
                        //
                        throw DuplicateOptionException(_OPTION_TYPE);
                    }

                    _type = getOpts [i].Value ();
                    _typeSet = true;
                    break;
                }

                case _OPTION_SERIALNUMBER:
                {
                    if (getOpts.isSet (_OPTION_SERIALNUMBER) > 1)
                    {
                        //
                        // More than one serial number option was found
                        //
                        throw DuplicateOptionException(_OPTION_SERIALNUMBER);
                    }
                    if (_subjectSet)
                    {
                        //
                        // Both the subject and serial number may not be
                        // specified.
                        //
                        throw UnexpectedOptionException(_OPTION_SERIALNUMBER);
                    }

                    _serialNumber = getOpts [i].Value ();
                    _serialNumberSet = true;
                    break;
                }

                case _OPTION_SUBJECT:
                {
                    if (getOpts.isSet (_OPTION_SUBJECT) > 1)
                    {
                        //
                        // More than one subject option was found
                        //
                        throw DuplicateOptionException(_OPTION_SUBJECT);
                    }
                    if (_serialNumberSet)
                    {
                        // Both the subject and serial number may not be
                        // specified.
                        throw UnexpectedOptionException(_OPTION_SUBJECT);
                    }

                    _subject = getOpts [i].Value ();
                    _subjectSet = true;
                    break;
                }
                default:
                {
                    //
                    //  This path should not be hit
                    //
                    break;
                }
            }
        }
    }

    if ( _operationType == _OPERATION_TYPE_UNINITIALIZED )
    {
        //
        // No operation type was specified
        // Show the usage
        //
        throw CommandFormatException(localizeMessage(
            MSG_PATH, REQUIRED_ARGS_MISSING_KEY, REQUIRED_ARGS_MISSING));
    }

    if ( _operationType == _OPERATION_TYPE_ADD )
    {
        //
        // For -a option, the next required option is -f,
        // make sure it is set.
        //
        if ( !_certFileSet )
        {
            //
            // A required option is missing
            //
            throw MissingOptionException(_OPTION_CERTFILE);
        }

        if ( !_typeSet )
        {
            //
            // A required option is missing
            //
            throw MissingOptionException(_OPTION_TYPE);
        }

        if (!(_type == _ARG_TYPE_AUTHORITY ||
              _type == _ARG_TYPE_AUTHORITY_END_ENTITY ||
              _type == _ARG_TYPE_SELF_SIGNED_IDENTITY ))
        {
            throw InvalidOptionArgumentException(_type, _OPTION_TYPE);
        }
        if ((_type == _ARG_TYPE_AUTHORITY_END_ENTITY ||
             _type == _ARG_TYPE_SELF_SIGNED_IDENTITY) &&
            !_certUserSet)
        {
            //
            // A required option is missing
            //
            throw MissingOptionException(_OPTION_CERTUSER);
        }

    }

    if ( _operationType == _OPERATION_TYPE_REMOVE ||
         _operationType == _OPERATION_TYPE_LIST )
    {
        if ( _operationType == _OPERATION_TYPE_REMOVE )
        {
            if (_certFileSet)
            {
                throw UnexpectedOptionException(_OPTION_CERTFILE);
            }
            if (_certUserSet)
            {
                throw UnexpectedOptionException(_OPTION_CERTUSER);
            }

            //
            // For -r option, the required option is -i,
            // make sure it is set.
            //
            if ( !_issuerSet )
            {
                //
                // A required option is missing
                //
                throw MissingOptionException(_OPTION_ISSUER);
            }
            else if ( !_serialNumberSet && !_subjectSet )
            {
                //
                // A required option is missing
                //
                throw CommandFormatException(localizeMessage(
                    MSG_PATH,
                    REQUIRED_ARGS_MISSING_KEY,
                    REQUIRED_ARGS_MISSING));
            }
        }
        else if ( _operationType == _OPERATION_TYPE_LIST )
        {
            //
            // For -l option, there is no required option.
            //

            //
            //  Serial number specified without issuer name
            //
            if ( (_serialNumberSet || _subjectSet) && !_issuerSet )
            {
                throw MissingOptionException(_OPTION_ISSUER);
            }

        }

    }
}


/**

    Executes the command and writes the results to the PrintWriters.

    @param   outPrintWriter     the ostream to which output should be
                                written
    @param   errPrintWriter     the ostream to which error output should be
                                written

    @return  0                  if the command is successful
             1                  if an error occurs in executing the command

 */
Uint32 CIMTrustCommand::execute (
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    if ( _operationType == _OPERATION_TYPE_UNINITIALIZED )
    {
        //
        // The command was not initialized
        //
        return 1;
    }

    //
    // Check if a specified certificate user is valid
    //
    if (_certUserSet)
    {
        if (!System::isSystemUser(_certUser.getCString()))
        {
            outPrintWriter << localizeMessage(MSG_PATH, INVALID_SYSTEM_USER_KEY,
                INVALID_SYSTEM_USER) << endl;
            return ( RC_INVALID_SYSTEM_USER );

        }
    }

    //
    // Options HELP and VERSION
    // CIMServer need not be running for these options to work
    //
    else if (_operationType == _OPERATION_TYPE_HELP)
    {
        errPrintWriter << _usage << endl;
        return (RC_SUCCESS);
    }
    else if(_operationType == _OPERATION_TYPE_VERSION)
    {
        errPrintWriter << "Version " << PEGASUS_PRODUCT_VERSION << endl;
        return (RC_SUCCESS);
    }

    CIMClient client;

    client.setTimeout( _timeout );
    client.setRequestDefaultLanguages(); //l10n

    try
    {
        _connectToServer(client, outPrintWriter);
    }
    catch (CannotConnectException&)
    {
        outPrintWriter << localizeMessage(MSG_PATH,
            CANNOT_CONNECT_CIMSERVER_NOT_RUNNING_KEY,
            CANNOT_CONNECT_CIMSERVER_NOT_RUNNING)
            << endl;
        return RC_CONNECTION_FAILED;
    }
    catch (Exception& e)
    {
        outPrintWriter << e.getMessage() << endl;
        return RC_ERROR;
    }

    //
    // Perform the requested operation
    //
    switch ( _operationType )
    {
        case _OPERATION_TYPE_ADD:
            try
            {
                _addCertificate( client, outPrintWriter );
            }
            catch (ConnectionTimeoutException&)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                    CONNECTION_TIMEOUT_KEY,
                    CONNECTION_TIMEOUT) << endl;
                return RC_CONNECTION_TIMEOUT;
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();

                if (code == CIM_ERR_FAILED || code == CIM_ERR_NOT_SUPPORTED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                            ADD_CERT_FAILURE_KEY, ADD_CERT_FAILURE) << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_ALREADY_EXISTS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        ADD_CERT_FAILURE_KEY, ADD_CERT_FAILURE) << endl;

                    outPrintWriter << localizeMessage(MSG_PATH,
                        CERT_ALREADY_EXISTS_KEY,
                        CERT_ALREADY_EXISTS) << endl;
                    errPrintWriter << e.getMessage()  << endl;
                    return RC_CERTIFICATE_ALREADY_EXISTS;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        ADD_CERT_FAILURE_KEY, ADD_CERT_FAILURE) << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        CERT_SCHEMA_NOT_LOADED_KEY,
                        CERT_SCHEMA_NOT_LOADED) << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }

                return RC_ERROR;
            }
            catch (Exception& e)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                    ADD_CERT_FAILURE_KEY,
                    ADD_CERT_FAILURE) << endl << e.getMessage() << endl;
                return RC_ERROR;
            }
            break;

        case _OPERATION_TYPE_REMOVE:
            try
            {
                _removeCertificate ( client, outPrintWriter );
            }
            catch (ConnectionTimeoutException&)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                    CONNECTION_TIMEOUT_KEY,
                    CONNECTION_TIMEOUT);
                return RC_CONNECTION_TIMEOUT;
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();

                if (code == CIM_ERR_FAILED || code == CIM_ERR_NOT_SUPPORTED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                            REMOVE_CERT_FAILURE_KEY,
                            REMOVE_CERT_FAILURE) << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_NOT_FOUND)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                            REMOVE_CERT_FAILURE_KEY,
                            REMOVE_CERT_FAILURE) << endl;

                    outPrintWriter << localizeMessage(MSG_PATH,
                            CERT_NOT_FOUND_KEY, CERT_NOT_FOUND) << endl;

                    errPrintWriter << e.getMessage()  << endl;
                    return RC_CERTIFICATE_DOES_NOT_EXIST;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                            REMOVE_CERT_FAILURE_KEY,
                            REMOVE_CERT_FAILURE) << endl;

                    outPrintWriter << localizeMessage(MSG_PATH,
                        CERT_SCHEMA_NOT_LOADED_KEY,
                            CERT_SCHEMA_NOT_LOADED) << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }
                return RC_ERROR;
            }
            catch (Exception& e)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                        REMOVE_CERT_FAILURE_KEY,
                        REMOVE_CERT_FAILURE) << endl
                        << e.getMessage() << endl;

                return RC_ERROR;
            }
            break;

        case _OPERATION_TYPE_LIST:
            try
            {
                _listCertificates ( client, outPrintWriter );
            }
            catch (ConnectionTimeoutException&)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                    CONNECTION_TIMEOUT_KEY,
                    CONNECTION_TIMEOUT);
                return RC_CONNECTION_TIMEOUT;
            }
            catch (CIMException& e)
            {
                CIMStatusCode code = e.getCode();

                if (code == CIM_ERR_NOT_FOUND)
                {
                     return RC_CERTIFICATE_DOES_NOT_EXIST;
                }
                if (code == CIM_ERR_FAILED || code == CIM_ERR_NOT_SUPPORTED)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                         LIST_CERT_FAILURE_KEY, LIST_CERT_FAILURE) << endl;
                    errPrintWriter << e.getMessage() << endl;
                }
                else if (code == CIM_ERR_INVALID_CLASS)
                {
                    outPrintWriter << localizeMessage(MSG_PATH,
                        LIST_CERT_FAILURE_KEY, LIST_CERT_FAILURE) << endl;
                    outPrintWriter << localizeMessage(MSG_PATH,
                        CERT_SCHEMA_NOT_LOADED_KEY,
                        CERT_SCHEMA_NOT_LOADED) << endl;
                }
                else
                {
                    errPrintWriter << e.getMessage() << endl;
                }

                return RC_ERROR;
            }
            catch (Exception& e)
            {
                outPrintWriter << localizeMessage(MSG_PATH,
                    LIST_CERT_FAILURE_KEY,
                    LIST_CERT_FAILURE) << endl << e.getMessage() << endl;
                return RC_ERROR;
            }
            break;

        default:
            //
            //  This path should not be hit
            //
            break;
    }

    return RC_SUCCESS;
}

PEGASUS_NAMESPACE_END

// exclude main from the Pegasus Namespace


PEGASUS_USING_PEGASUS;

PEGASUS_USING_STD;

///////////////////////////////////////////////////////////////////////////
/**

    Parses the command line, and executes the command.

    @param   argc  the number of command line arguments
    @param   argv  the string vector of command line arguments

    @return  0                  if the command is successful
             1                  if an error occurs in executing the command

 */
///////////////////////////////////////////////////////////////////////////

int main (int argc, char* argv [])
{
    //l10n set message loading to process locale
    MessageLoader::_useProcessLocale = true;

    CIMTrustCommand    command = CIMTrustCommand();
    int            retCode;

    try
    {
        command.setCommand (argc, argv);
    }
    catch (CommandFormatException& cfe)
    {
        cerr << CIMTrustCommand::COMMAND_NAME << ": " << cfe.getMessage ()
             << endl;
        cerr << "Use '--help' to obtain command syntax." << endl;
        exit (Command::RC_ERROR);
    }

    retCode = command.execute (cout, cerr);

    exit (retCode);
    return 0;
}
