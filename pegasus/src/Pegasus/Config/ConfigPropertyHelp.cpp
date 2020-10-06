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


///////////////////////////////////////////////////////////////////////////////
//
// This file defines the table of help descriptions.  It is maintained as
// a separate file so that the help info can be modified without touching
// any of the code of Config
//
///////////////////////////////////////////////////////////////////////////////

#include "ConfigPropertyHelp.h"
#include <Pegasus/Common/Constants.h>

PEGASUS_NAMESPACE_BEGIN

// This table defines help descriptions for cimconfig propertis which do
// not have more detailed help in their property owner classes.
// FUTURE: This should be loadable from an external file or otherwise
// part of clients rather than in memory. Big waste of memory.
//
// NOTE: The internationalization message bundle for this set of
// messages SHOULD BE generated with the tool BldMsgBundle which is
// a test program in Config/tests. Please use this tool to generate
// this component of the message bundle for Config if any of the messages
// change rather than manually editing the individual messages in the bundle.
//
// Generally format the messages as they will be displayed with NL characters
// to provide formatting for a terminal output (i.e ~< 60 characters). NL
// characters should be used at any point the user wants to insure that
// there is a new line and spaces used for indenting. Do not put an NL at
// the end of the message.
// Rules for formatting the messages:
// 1. Formatting is done here.  The help display just uses the formatting
//    defined here.  Thus, any indenting of Possible Values, etc. andf EOLs
//    must be defined in this file.  Lines should not exceed 80 characters.
// 2. This text may be translated since it goes into the message bundle.
//    Put any words or phrases that are NOT to be translated into single
//    quotes.  This applies primarily to the possible values but should
//    include words like 'true', etc. if they are to be literally used
//    on the command line or displayed and used literally.
//
struct configPropertyDescription configPropertyDescriptionList[] =
{
    {"traceComponents",
        "Defines OpenPegasus server components to be traced.\n"
        "Define multiple components as comma-separated list. 'ALL' traces\n"
        "all components. Only OpenPegasus server components on this list\n"
        "generate entries in the trace output."},

    {"traceLevel",
        "Single digit defines level of server tracing enabled.\n"
        "Possible Values:\n"
        "    0 Trace off (default)\n"
        "    1 Severe and log messages\n"
        "    2 Basic flow trace messages, low data detail\n"
        "    3 Inter-function logic flow, medium data detail\n"
        "    4 High data detail\n"
        "    5 High data detail + Method Enter & Exit"},

    {"traceFilePath",
        "Specifies location and name of OpenPegasus trace file.\n"
        "Ignored when 'traceFacility!=File."},

    {"traceMemoryBufferKbytes",
        "Integer defines size of buffer for in-memory tracing.\n"
        "Value is in in kbytes. Minimum value is 16 kbytes. Ignored if\n"
        "'traceFacility!=memory'"},

    {"traceFileSizeKBytes",
        "Integer defines the maximum size of the tracefile in kbytes.\n"
        "The trace file will be rolled over when size exceeds the specified\n"
        "size. Minimum is 10,240 kbytes. Maximum is 2,097 mbytes."
        "Ignored if 'traceFacility=memory'. See also 'numberOfTraceFiles'."},

    {"numberOfTraceFiles","Integer defines the maximum number of trace files\n"
        "kept when trace files are rolled over upon reaching maximum size\n"
        "'traceFileSizeKBytes'.  Minimum is 3. Maximum is 20"},

    {"traceFacility", "Keyword selects the trace destination.\n"
        "Possible Values:\n"
        "    'File'   Trace output to file defined by 'traceFilePath'\n"
        "    'Log'    Trace output to log file\n"
        "    'Memory' Trace output to memory"},

    {"hostname", "Override local system supplied hostname CIM Server uses to\n"
        "build objects for WBEM operations that return hostname (ex.\n"
        "associators). Used where environment needs operations to\n"
        "return operation responses with hostname other than server name\n"
        "(ex. behind firewalls). When setting this option,\n"
        "'fullyQualifiedHostName' should be set also."},

    {"fullyQualifiedHostName", "Override local system supplied fully\n"
        "qualified hostname (with domain)CIM Server uses to build objects\n"
        "for WBEM Operations that return hostname (ex. associators).\n"
        "Used where environment needs operations to return operation\n"
        "responses with hostname other than server name (ex. behind firewalls)."
        "\nWhen setting this option 'hostname' should be set also."},

    {"logDir", "Specifies name of directory to be used for OpenPegasus "
            "specific log files.\n"
            "Not supported if PEGASUS_USE_SYSLOGS "
            "defined"},

    {"maxLogFileSizeKBytes",
        "Integer defines maximum size of the logfile in kbytes.\n"
        "If logfile size exceeds maxLogFileSizeKBytes logfile will be pruned.\n"
        "Minimum value is 32 KB. Not supported if PEGASUS_USE_SYSLOGS\n"
        "defined."},

    {"logLevel",
        "Keyword defines the desired level of logging.\n"
        "Log levels are accumulative(i.e. 'warning' logs 'warning', 'severe',\n"
        "and 'fatal'.\n"
        "Possible Values:\n"
        "    'trace'       Most Detailed. Output all log events\n"
        "    'information' Log information level. Default   \n"
        "    'warning'     Log warnings and more severe events \n"
        "    'severe'      Log fatal and severe events  \n"
        "    'fatal'       Log only events that are fatal to server"},

    {"enableHttpConnection",
        "If 'true', allows connection to CIM Server using HTTP protocol"},

    {"enableHttpsConnection",
        "If 'true', allows connection to the CIM Server using HTTPS protocol\n"
        "(HTTP using Secure Socket Layer encryption)"},

    {"httpPort", "Integer defines HTTP port number.\n"
        "OpenPegasus first attempts to look up port number for HTTP\n"
        "using getservbyname for the 'wbem-http' service. httpPort.\n"
        "configuration setting used only when getservbyname lookup fails.\n"
        "Use of port 5988 recommended by DMTF"},

    {"httpsPort", "Integer that defines HTTPS port number.\n"
        "OpenPegasus first attempts to look up port number for\n"
        "HTTPS using getservbyname for the 'wbem-https' service. httpPorts\n"
        "configuration setting used only when the getservbyname lookup fails.\n"
        "Use of port 5989 recommended by DMTF"},

    {"daemon",
        "If 'true' enables forking of the CIM Server to create a background\n"
        "daemon process."},
    /// needs more info
    {"slp",
        "If 'true', OpenPegasus activates an SLP SA and issues DMTF defined\n"
        "SLP advertisements to this SA on startup."},

    {"enableAssociationTraversal",
        "If 'true', CIM Server supports the association traversal operators:\n"
        " Associators, AssociatorNames,References, and ReferenceNames."},

    {"enableIndicationService",
        "If 'true', the CIM Server will support CIM Indications."},

    {"enableAuthentication",
        "If 'true', a Client must be authenticated to access the CIM Server."},

    {"enableNamespaceAuthorization",
        "If 'true', CIM Server restricts access to namespaces based on\n"
        "configured user authorizations [user authorizations may be\n"
        "configured using the 'cimauth' command]"},

    {"httpAuthType",  "Type of HTTP authentication. Either 'basic'\n"
        "indicating basic authentication or 'negotiate' indicating use of\n"
        "HTTP Negotiate authentication method. 'basic' is the default one."},

    {"passwordFilePath",  "Path to password file if password file used for\n"
        "user authentication."},

    {"sslCertificateFilePath",
        "File containing the CIM Server SSL Certificate."},
    //// TBD Clarify if just file name or path also. Both above and following
    {"sslKeyFilePath",
        "File Containing private key for CIM Server SSL Certificate."},

    {"sslTrustStore",
        "Specifies location of OpenSSL truststore.\n"
        "Truststore can be either file or directory. If the truststore is a\n"
        "directory, all certificates in the directory are trusted."},

    {"crlStore",  "Specifies location of OpenSSL certificate revocation list.\n"
         "if enabled."},

    {"sslClientVerificationMode",
        "Level of support for certificate-based authentication.\n"
        "Valid only if 'enableHttpsConnection=true'\n"
        "Possible Values:\n"
        "'required': HTTPS client MUST present a trusted certificate to"
        " access\n"
        "    CIM Server. If client fails to send a certificate or sends an\n"
        "    untrusted certificate, the connection is rejected.\n"
        "'optional': HTTPS client may, but is not required to, present a\n"
        "   trusted certificate to access the server. Server requests and\n"
        "   validates client certificate, but the connection is accepted if"
        " no\n"
        "   certificate or an untrusted certificate is sent. Server will\n"
        "   then seek to authenticate client via authentication header.\n"
        "'disabled': CIM Server will not allow HTTPS clients to authenticate\n"
        "   using a certificate. Basic authentication will be used to\n"
        "   authenticate HTTPS clients"},

    {"sslTrustStoreUserName",
        "System user name to be associated with all certificate-based "
        "authenticated\nrequests.\n"
        "No default; for security reasons, system administrator must\n"
        "explicitly specify this value.  Allows a single user name to be\n"
        "specified.  This user will be associated with all certificates in\n"
        "the truststore"},

    {"kerberosServiceName",  "TBD"},

    {"enableCFZAPPLID",  "TBD"},
    //// Definition Format needs clarification. i.e. multiple ciphers
    {"sslCipherSuite",
        "String containing OpenSSL cipher specifications to configure\n"
        "cipher suite the client is permitted to negotiate with the server\n"
        "during SSL handshake phase. Enclose values in single quotes to\n"
        "avoid issues with special characters. Default if this parameter is\n"
        "is the list of ciphers in OpenSSL."},

    {"repositoryIsDefaultInstanceProvider",
        "If 'true', Repository functions as Provider (for instance and\n"
        "association operations) for all classes without an explicitly\n"
        "registered Provider."},

    {"enableBinaryRepository",
        "If 'true' activates the binary repository support on the next\n"
        "restart of cimserver. OpenPegasus supports repositories with a\n"
        "mixture of binary and XML objects."},

    {"shutdownTimeout",
        "When 'cimserver -s' shutdown command is issued, specifies maximum\n"
        "time in seconds for CIM Server to complete outstanding CIM\n"
        "operation requests before shutting down. If specified timeout\n"
        "period expires, CIM Server shuts down, even with CIM operations\n"
        "in progress. Minimum value is 2 seconds."},

    {"repositoryDir",
        "Name of directory to be used for the OpenPegasus repository."},

    {"providerManagerDir",
        "Name of the directory containing ProviderManager plugin libraries.\n"
        "Should be a dedicated directory to minimize inspection of\n"
        "non-applicable files."},

    {"providerDir",
        "Names of directories that contains provider executables.\n"
        "If multiple directories they should be comma-separated."},

    {"enableRemotePrivilegedUserAccess",
        "If 'true', the CIM Server allows access by a privileged user from a\n"
        "remote system. Many management operations require privileged user\n"
        "access. Disabling remote access by privileged user could\n"
        "significantly affect functionality."},

    {"enableSubscriptionsForNonprivilegedUsers",
        "If 'true', operations on indication filter, listener destination,\n"
        "and subscription instances may be performed by non-privileged\n"
        "users.Otherwise, these operations limited to privileged users."},
    //// This whole description is backwards.  It really defines what
    //// server will accept, not what client may issue
    {"authorizedUserGroups",
        "If set, the value is list of comma-separated user\n"
        "groups whose members may issue CIM requests. A user not a member of\n"
        "any of these groups is restricted from issuing CIM requests, except\n"
        "for privileged users (root user). If not set, any user may issue\n"
        "CIM requests."},

    {"messageDir",
        "Name of the directory used for the OpenPegasus translated\n"
        "message bundles."},

    {"enableNormalization",
        "If 'true', objects returned from instance providers are validated\n"
        "and normalized.\n"
        "Errors encountered during normalization cause\n"
        "CIMException with the status code set to CIM_ERR_FAILED. Provider\n"
        "modules in excludeMOdulesFromNormalizationList excluded from\n"
        "normalization."},

    {"excludeModulesFromNormalization",
        "If not Null, the value is interpreted as comma-separated list of\n"
        "Provider Module names to exclude from validation and normalization."},

    //// What is relation with the provider groups?
    {"forceProviderProcesses",
        "If 'true', CIM Server runs Providers in separate processes rather\n"
        "than loading and calling Provider libraries directly within\n"
        "CIM Server process."},

    {"maxProviderProcesses",
        "Limits number of provider processes (see 'forceProviderProcesses')\n"
        "that may run concurrently. Value '0' indicates that the number of\n"
        " Provider Agent processes unlimited"},

    {"enableAuditLog",  "If 'true', audit audit log entries for certain types\n"
         "of CIM Server activities (i.e. activities that modify server state)\n"
         "will be written to the system log. Aaudit log entries describe the\n"
         "who, what, and when of audited activity."},

    {"socketWriteTimeout",
        "Integer defines the number of seconds the CIM Server will wait for a\n"
        "client connection to be ready to receive data. If the CIM Server is\n"
        "unable to write to a connection in this time, the connection is\n"
        "closed."},

    {"idleConnectionTimeout",
        "If set to a positive integer, value specifies a minimum timeout\n"
        "value for idle client connections. If set to zero, idle client\n"
        "connections do not time out."},

    {"maxFailedProviderModuleRestarts",
        "If set to a positive integer, this value specifies the number of\n"
        "times a failed provider module with indications enabled is restarted\n"
        "automatically before being moved to Degraded state. If set to zero,\n"
        "failed provider module is not restarted with indications enabled\n"
        "automatically and will be moved to Degraded state immediately."},

     {"maxIndicationDeliveryRetryAttempts",
        "If set to a positive integer, value defines the number of times\n"
        "indication service will enable the reliableIndication feature \n"
        "& try to deliver an indication to a particular listener destination.\n"
        "This does not effect the original delivery attempt. A value of 0\n"
        "disables reliable indication feature completely, and cimserver will\n"
        "deliver the indication once."},

    {"minIndicationDeliveryRetryInterval",
        "If set to a positive integer, defines the minimal time interval in\n"
        "seconds indication service waits before retrying delivery of\n"
        "indication to a listener destination that previously failed.\n"
        "Cimserver may take longer due to QoS or other processing."},

    {"slpProviderStartupTimeout",
        "Timeout value in milliseconds used to specify how long the\n"
        "registration with an SLP SA may take. Registration will be retried\n"
        "three times. This value only needs to be increased in case\n"
        "the loading of a set of providers whose implementation\n"
        "of a registered profile takes very long."},

    {"listenAddress",
        "Network interface where the cimserver listens for connections."
        " Default\n"
        "value  'All' forces listening at all interfaces. A comma seperated\n"
        "list(without spaces) forces listening at specific interfaces. For\n"
        "example: 'listenAddress=All' or 'listenAddress=121.12.33.112', or\n"
        "'listenAddress=121.33.21.26,127.0.0.1,fe80::fe62:9346%eth0'"},

   {"sslBackwardCompatibility",
        "If 'true', OpenPegasus support SSLv3 and TLSv1 protocol.If 'false',\n"
        "OpenPegasus support only TLS1.2 protocol for openssl 1.0.1+"},

    {"webRoot",
        "Points to a location where the static web pages to be served by the\n"
        "pegasus webserver are stored"},

    {"indexFile",
        "Specifies the name of index file used by pegasus webserver, default \n"
        "to index.html. This file should be available at the webRoot path"},

    {"mimeTypesFile",
        "Refers to the file which holds the mime types being served by the \n"
        "pegasus webserver "},

    {"mapToLocalName",  "If 'true', all remote client names are translated to\n"
            "local ones. This applies only to clients using Negotiate\n"
            "authentication method."},

    {"httpSessionTimeout",
            "If set to a positive integer, value specifies a HTTP session\n"
            "cookie expiration time in seconds. If set to 0, cookie\n"
            "processing in Pegasus is disabled, it will not send nor accept\n"
            "any cookies."},

    {"pullOperationsMaxObjectCount",
        "Defines the system limit on the number of instances or paths that "
        "the client\n"
        "may request in the open... or pull... 'maxObjectCount' "
        "argument.\n"
        "Minimum value = 1. Maximum value = "
        PEGASUS_PULL_OPERATION_MAX_OBJECT_COUNT_STRING },

    {"pullOperationsMaxTimeout",
        "Defines the system maximum limit on the time in seconds between "
        "open/pull\n"
        "operations the client may request within an enumeration "
        "sequence with the\n\'operationTimeout\' argument in open... client "
        "requests.\n"
        "Minimum value = 1 sec. Maximum value = "
        PEGASUS_PULL_OPERATION_MAX_TIMEOUT_SEC_STRING "sec"},

    {"pullOperationsDefaultTimeout",
        "Defines the default time in seconds of the \'operationTimeout\' "
        "(the time\nbetween open/pull operations within a single "
        "enumeration sequence)\nthe server uses when the client open... "
        "request \'operationTimeout\' parameter\nis NULL. Minimum value = 1 "
        "sec . Maximum value = "
        PEGASUS_DEFAULT_PULL_OPERATION_TIMEOUT_SEC_STRING "sec"}
};

const Uint32 configPropertyDescriptionListSize =
    sizeof(configPropertyDescriptionList) /
        sizeof(configPropertyDescriptionList[0]);

PEGASUS_NAMESPACE_END


