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
/*
//
//%/////////////////////////////////////////////////////////////////////////////
*/

#ifndef Pegasus_Constants_h
#define Pegasus_Constants_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>

/*
**==============================================================================
**
**  C/C++ Common Section
**
**==============================================================================
*/

/*
 * Message Queue Names
 */

#define PEGASUS_QUEUENAME_METADISPATCHER      "pegasus meta dispatcher"

#define PEGASUS_QUEUENAME_HTTPACCEPTOR        "HTTPAcceptor"
#define PEGASUS_QUEUENAME_HTTPCONNECTION      "HTTPConnection"

#define PEGASUS_QUEUENAME_HTTPAUTHDELEGATOR   "HTTPAuthenticatorDelegator"
#define PEGASUS_QUEUENAME_OPREQDECODER        "CIMOpRequestDecoder"
#define PEGASUS_QUEUENAME_OPREQAUTHORIZER     "CIMOpRequestAuthorizer"
#define PEGASUS_QUEUENAME_OPREQDISPATCHER     "CIMOpRequestDispatcher"
#define PEGASUS_QUEUENAME_OPRESPENCODER       "CIMOpResponseEncoder"

#define PEGASUS_QUEUENAME_CONTROLSERVICE      "ControlService"
#define PEGASUS_QUEUENAME_INDICATIONSERVICE   "Server::IndicationService"
#define PEGASUS_QUEUENAME_PROVIDERMANAGER_CPP "Server::ProviderManagerService"
#define PEGASUS_QUEUENAME_INDHANDLERMANAGER   "IndicationHandlerService"

#define PEGASUS_QUEUENAME_CLIENT              "CIMClient"
#define PEGASUS_QUEUENAME_OPREQENCODER        "CIMOperationRequestEncoder"
#define PEGASUS_QUEUENAME_OPRESPDECODER       "CIMOperationResponseDecoder"

#define PEGASUS_QUEUENAME_EXPORTCLIENT        "CIMExportClient"
#define PEGASUS_QUEUENAME_EXPORTREQENCODER    "CIMExportRequestEncoder"
#define PEGASUS_QUEUENAME_EXPORTRESPDECODER   "CIMExportResponseDecoder"

#define PEGASUS_QUEUENAME_EXPORTREQDECODER    "CIMExportRequestDecoder"
#define PEGASUS_QUEUENAME_EXPORTREQDISPATCHER "CIMExportRequestDispatcher"
#define PEGASUS_QUEUENAME_EXPORTRESPENCODER   "CIMExportResponseEncoder"

#define PEGASUS_QUEUENAME_WBEMEXECCLIENT      "WbemExecClient"
#define PEGASUS_QUEUENAME_INTERNALCLIENT       "InternalClient"

#define PEGASUS_QUEUENAME_WSMANEXPORTCLIENT    "WSMANExportClient"
#define PEGASUS_QUEUENAME_WSMANEXPORTREQENCODER  "WSMANExportRequestEncoder"
#define PEGASUS_QUEUENAME_WSMANEXPORTRESPENCODER  "WSMANExportResponseDecoder"

/*
 * ModuleController Module Names
 */

#define PEGASUS_MODULENAME_CONFIGPROVIDER \
    PEGASUS_QUEUENAME_CONTROLSERVICE "::ConfigProvider"
#define PEGASUS_MODULENAME_USERAUTHPROVIDER \
    PEGASUS_QUEUENAME_CONTROLSERVICE "::UserAuthProvider"
#define PEGASUS_MODULENAME_PROVREGPROVIDER \
    PEGASUS_QUEUENAME_CONTROLSERVICE "::ProviderRegistrationProvider"
#define PEGASUS_MODULENAME_SHUTDOWNPROVIDER \
    PEGASUS_QUEUENAME_CONTROLSERVICE "::ShutdownProvider"
#define PEGASUS_MODULENAME_NAMESPACEPROVIDER \
    PEGASUS_QUEUENAME_CONTROLSERVICE "::NamespaceProvider"
#define PEGASUS_MODULENAME_CERTIFICATEPROVIDER \
    PEGASUS_QUEUENAME_CONTROLSERVICE "::CertificateProvider"
#define PEGASUS_MODULENAME_TEMP \
    PEGASUS_QUEUENAME_CONTROLSERVICE "::temp::do not use this name"
#define PEGASUS_MODULENAME_MONITOR \
    PEGASUS_QUEUENAME_CONTROLSERVICE "::Monitor"
#define PEGASUS_MODULENAME_INTEROPPROVIDER \
    PEGASUS_QUEUENAME_CONTROLSERVICE "::InteropProvider"

#ifndef PEGASUS_DISABLE_PERFINST
# define PEGASUS_MODULENAME_CIMOMSTATDATAPROVIDER \
     PEGASUS_QUEUENAME_CONTROLSERVICE "::CIMOMStatDataProvider"
#endif

#ifdef PEGASUS_ENABLE_CQL
# define PEGASUS_MODULENAME_CIMQUERYCAPPROVIDER \
     PEGASUS_QUEUENAME_CONTROLSERVICE "::CIMQueryCapabilitiesProvider"
#endif

/*
 * HTTP Header Tags, Status Codes, and Reason Phrases
 */

#define PEGASUS_HTTPHEADERTAG_ERRORDETAIL "PGErrorDetail"

#define HTTP_STATUSCODE_OK 200
#define HTTP_REASONPHRASE_OK "OK"
#define HTTP_STATUS_OK "200 OK"

#define HTTP_STATUSCODE_PARTIALCONTENT 206
#define HTTP_REASONPHRASE_PARTIALCONTENT "Partial Content"
#define HTTP_PARTIALCONTENT "206 Partial Content"

#define HTTP_STATUSCODE_BADREQUEST 400
#define HTTP_REASONPHRASE_BADREQUEST "Bad Request"
#define HTTP_STATUS_BADREQUEST "400 Bad Request"

#define HTTP_STATUSCODE_UNAUTHORIZED 401
#define HTTP_REASONPHRASE_UNAUTHORIZED "Unauthorized"
#define HTTP_STATUS_UNAUTHORIZED "401 Unauthorized"

#define HTTP_STATUSCODE_FORBIDDEN    403
#define HTTP_REASONPHRASE_FORBIDDEN    "Forbidden"
#define HTTP_STATUS_FORBIDDEN    "403 Forbidden"

#define HTTP_STATUSCODE_NOTFOUND    404
#define HTTP_REASONPHRASE_NOTFOUND  "Not Found"
#define HTTP_STATUS_NOTFOUND        "404 Not Found"

#define HTTP_STATUSCODE_NOTFOUND    404
#define HTTP_REASONPHRASE_NOTFOUND  "Not Found"
#define HTTP_STATUS_NOTFOUND        "404 Not Found"

#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
 #define HTTP_STATUSCODE_METHODNOTALLOWED 405
 #define HTTP_REASONPHRASE_METHODNOTALLOWED "Method Not Allowed"
 #define HTTP_STATUS_METHODNOTALLOWED "405 Method Not Allowed"

 #define HTTP_STATUSCODE_NOTACCEPTABLE 406
 #define HTTP_REASONPHRASE_NOTACCEPTABLE "Not Acceptable"
 #define HTTP_STATUS_NOTACCEPTABLE "406 Not Acceptable"
#endif /* PEGASUS_ENABLE_PROTOCOL_WEB */

#define HTTP_STATUSCODE_REQUEST_TOO_LARGE 413
#define HTTP_REASONPHRASE_REQUEST_TOO_LARGE "Request Entity Too Large"
#define HTTP_STATUS_REQUEST_TOO_LARGE "413 Request Entity Too Large"

#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
 #define HTTP_STATUSCODE_REQUESTURITOOLONG 414
 #define HTTP_REASONPHRASE_REQUESTURITOOLONG "Request URI Too Long"
 #define HTTP_STATUS_REQUESTURITOOLONG "414 Request URI Too Long"
#endif /* PEGASUS_ENABLE_PROTOCOL_WEB */

#define HTTP_STATUSCODE_INTERNALSERVERERROR 500
#define HTTP_REASONPHRASE_INTERNALSERVERERROR "Internal Server Error"
#define HTTP_STATUS_INTERNALSERVERERROR "500 Internal Server Error"

#define HTTP_STATUSCODE_NOTIMPLEMENTED 501
#define HTTP_REASONPHRASE_NOTIMPLEMENTED "Not Implemented"
#define HTTP_STATUS_NOTIMPLEMENTED "501 Not Implemented"

#define HTTP_STATUSCODE_SERVICEUNAVAILABLE 503
#define HTTP_REASONPHRASE_SERVICEUNAVAILABLE "Service Unavailable"
#define HTTP_STATUS_SERVICEUNAVAILABLE "503 Service Unavailable"

#ifdef PEGASUS_ENABLE_PROTOCOL_WEB
 #define HTTP_STATUSCODE_VERSIONNOTSUPPORTED 505
 #define HTTP_REASONPHRASE_VERSIONNOTSUPPORTED "HTTP Version Not Supported"
 #define HTTP_STATUS_VERSIONNOTSUPPORTED "505 HTTP Version Not Supported"
#endif /* PEGASUS_ENABLE_PROTOCOL_WEB */


/*
 * Default port numbers
 */

#define WBEM_DEFAULT_HTTP_PORT 5988
#define WBEM_DEFAULT_HTTPS_PORT 5989

#define SNMP_TRAP_DEFAULT_PORT 162


/*
 * Default timeout values for sockets and clients
 */

#define PEGASUS_DEFAULT_CLIENT_TIMEOUT_MILLISECONDS 20000
#define PEGASUS_DEFAULT_SHUTDOWN_TIMEOUT_SECONDS_STRING "30"
#define PEGASUS_DEFAULT_SOCKETWRITE_TIMEOUT_SECONDS 20
#define PEGASUS_DEFAULT_SOCKETWRITE_TIMEOUT_SECONDS_STRING "20"
#define PEGASUS_SSL_ACCEPT_TIMEOUT_SECONDS 20
#define PEGASUS_PROVIDER_IDLE_TIMEOUT_SECONDS 300

/*
 * Pull Operation constants.  These constants define the
 * limits for each of the defined configuration variables that may
 * be set by the configuration manager as well as compile time
 * constants
*/
//
//  Runtime pull operation config pull configuration parameter limits
//
#define PEGASUS_DEFAULT_PULL_OPERATION_TIMEOUT_SEC 30
#define PEGASUS_DEFAULT_PULL_OPERATION_TIMEOUT_SEC_STRING "30"
#define PEGASUS_PULL_OPERATION_MAX_TIMEOUT_SEC 90
#define PEGASUS_PULL_OPERATION_MAX_TIMEOUT_SEC_STRING "90"
#define PEGASUS_PULL_OPERATION_MAX_OBJECT_COUNT 10000
#define PEGASUS_PULL_OPERATION_MAX_OBJECT_COUNT_STRING "10000"

//
// Constants that are NOT part of runtime configuration
//
// Maximum time server will wait in seconds after receiving a pull before
// returning zero objects response. This should be significantly less that
// the client timeout to assure that some response gets back to client
// before client times out.
#define PEGASUS_PULL_MAX_OPERATION_WAIT_SEC 9
// Number of times dispatcher will send the zero length keep alive
// response (because providers not responding) before it concludes
// there was an error and tries to send msg to provider manager to clean up
#define PEGASUS_MAX_CONSECUTIVE_WAITS_BEFORE_ERR 6

// Number of times dispatcher will send the zero length keep alive
// response (because providers not responding) before it concludes
// there was an error and closes the enumerationContext
// This should be greater than PEGASUS_MAX_CONSECUTIVE_WAITS_BEFORE_ERR
// to allow an attempt at cleanup before the enumeration is failed.
#define PEGASUS_MAX_CONSECUTIVE_WAITS_BEFORE_FAIL 16
/*
 * Wbem service names
 */

#define WBEM_HTTP_SERVICE_NAME "wbem-http"
#define WBEM_HTTPS_SERVICE_NAME "wbem-https"

/*
 * Hard limit for number of HTTP headers, elements in container and keybindings
 */
#define PEGASUS_MAXELEMENTS_NUM 1000
#define PEGASUS_MAXELEMENTS "1000"

/*
 * File system layout
 */

#ifdef PEGASUS_USE_RELEASE_DIRS
#ifdef PEGASUS_OVERRIDE_DEFAULT_RELEASE_DIRS
# include <Pegasus/Common/ProductDirectoryStructure.h>
#else
# if defined(PEGASUS_OS_HPUX)
#  define PEGASUS_CIMSERVER_START_FILE      "/etc/opt/wbem/cimserver_start.conf"
#  define PEGASUS_CIMSERVER_START_LOCK_FILE "/var/opt/wbem/cimserver_start.lock"
#  define PEGASUS_REPOSITORY_DIR            "/var/opt/wbem/repository"
#  define PEGASUS_CURRENT_CONFIG_FILE_PATH  \
    "/var/opt/wbem/cimserver_current.conf"
#  define PEGASUS_PLANNED_CONFIG_FILE_PATH  \
    "/var/opt/wbem/cimserver_planned.conf"
#  define PEGASUS_SSLCLIENT_CERTIFICATEFILE "/etc/opt/hp/sslshare/client.pem"
#  define PEGASUS_SSLCLIENT_RANDOMFILE      "/var/opt/wbem/ssl.rnd"
#  define PEGASUS_SSLSERVER_RANDOMFILE      "/var/opt/wbem/cimserver.rnd"
#  define PEGASUS_LOCAL_AUTH_DIR            "/var/opt/wbem/localauth"
#  define PEGASUS_LOCAL_DOMAIN_SOCKET_PATH  "/var/opt/wbem/socket/cimxml.socket"
#  define PEGASUS_CORE_DIR                  "/var/opt/wbem"
#  define PEGASUS_PAM_STANDALONE_PROC_NAME  "/opt/wbem/lbin/cimservera"
#  define PEGASUS_PROVIDER_AGENT_PROC_NAME  "/opt/wbem/lbin/cimprovagt"
#  define PEGASUS_DEFAULT_MESSAGE_SOURCE    \
    "/opt/wbem/share/locale/ICU_Messages"
# elif defined(PEGASUS_OS_PASE)
#  define PEGASUS_CIMSERVER_START_FILE      \
    "/QOpenSys/var/UME/cimserver_start.conf"
#  define PEGASUS_CIMSERVER_START_LOCK_FILE \
    "/QOpenSys/var/UME/cimserver_start.lock"
#  define PEGASUS_REPOSITORY_DIR            \
    "/QOpenSys/QIBM/UserData/UME/Pegasus/repository"
#  define PEGASUS_CURRENT_CONFIG_FILE_PATH  \
    "/QOpenSys/QIBM/UserData/UME/Pegasus/cimserver_current.conf"
#  define PEGASUS_PLANNED_CONFIG_FILE_PATH  \
    "/QOpenSys/QIBM/UserData/UME/Pegasus/cimserver_planned.conf"
#  define PEGASUS_SSLCLIENT_CERTIFICATEFILE \
    "/QOpenSys/QIBM/UserData/UME/Pegasus/client.pem"
#  define PEGASUS_SSLCLIENT_RANDOMFILE      \
    "/QOpenSys/QIBM/UserData/UME/Pegasus/ssl.rnd"
#  define PEGASUS_SSLSERVER_RANDOMFILE      \
    "/QOpenSys/var/UME/cimserver.rnd"
#  define PEGASUS_LOCAL_AUTH_DIR            "/QOpenSys/var/UME"
#  define PEGASUS_LOCAL_DOMAIN_SOCKET_PATH  "/QOpenSys/var/UME/cimxml.socket"
#  define PEGASUS_PAM_STANDALONE_PROC_NAME  \
    "/QOpenSys/QIBM/ProdData/UME/Pegasus/bin/cimservera"
#  define PEGASUS_PROVIDER_AGENT_PROC_NAME  \
    "/QOpenSys/QIBM/ProdData/UME/Pegasus/bin/cimprovagt"
#  undef PEGASUS_DEFAULT_MESSAGE_SOURCE     /* Not defined */
# elif defined(PEGASUS_OS_AIX)
#  define PEGASUS_CIMSERVER_START_FILE      "/tmp/cimserver_start.conf"
#  define PEGASUS_CIMSERVER_START_LOCK_FILE \
    "/opt/freeware/cimom/pegasus/etc/cimserver_start.lock"
#  define PEGASUS_REPOSITORY_DIR            \
    "/opt/freeware/cimom/pegasus/etc/repository"
#  define PEGASUS_CURRENT_CONFIG_FILE_PATH  \
    "/opt/freeware/cimom/pegasus/etc/cimserver_current.conf"
#  define PEGASUS_PLANNED_CONFIG_FILE_PATH  \
    "/opt/freeware/cimom/pegasus/etc/cimserver_planned.conf"
#  define PEGASUS_SSLCLIENT_CERTIFICATEFILE \
    "/opt/freeware/cimom/pegasus/etc/client.pem"
#  define PEGASUS_SSLCLIENT_RANDOMFILE      \
    "/opt/freeware/cimom/pegasus/etc/ssl.rnd"
#  define PEGASUS_SSLSERVER_RANDOMFILE      \
    "/opt/freeware/cimom/pegasus/etc/cimserver.rnd"
#  define PEGASUS_LOCAL_AUTH_DIR            "/tmp"
#  define PEGASUS_LOCAL_DOMAIN_SOCKET_PATH  \
    "/opt/freeware/cimom/pegasus/etc/cimxml.socket"
#  define PEGASUS_PAM_STANDALONE_PROC_NAME  \
    "/opt/freeware/cimom/pegasus/bin/cimservera"
#  define PEGASUS_PROVIDER_AGENT_PROC_NAME  \
    "/opt/freeware/cimom/pegasus/bin/cimprovagt"
#  define PEGASUS_DEFAULT_MESSAGE_SOURCE    "/opt/freeware/cimom/pegasus/msg"
# elif defined(PEGASUS_OS_LINUX)
#  define PEGASUS_CIMSERVER_START_FILE      "/var/run/tog-pegasus/cimserver.pid"
#  define PEGASUS_CIMSERVER_START_LOCK_FILE \
    "/var/run/tog-pegasus/cimserver_start.lock"
#  define PEGASUS_REPOSITORY_DIR            "/var/opt/tog-pegasus/repository"
#  define PEGASUS_CURRENT_CONFIG_FILE_PATH  \
    "/var/opt/tog-pegasus/cimserver_current.conf"
#  define PEGASUS_PLANNED_CONFIG_FILE_PATH  \
    "/var/opt/tog-pegasus/cimserver_planned.conf"
#  define PEGASUS_SSLCLIENT_CERTIFICATEFILE "/etc/opt/tog-pegasus/client.pem"
#  define PEGASUS_SSLCLIENT_RANDOMFILE      "/var/opt/tog-pegasus/ssl.rnd"
#  define PEGASUS_SSLSERVER_RANDOMFILE      "/var/opt/tog-pegasus/cimserver.rnd"
#  define PEGASUS_LOCAL_AUTH_DIR            \
    "/var/opt/tog-pegasus/cache/localauth"
#  define PEGASUS_LOCAL_DOMAIN_SOCKET_PATH  \
    "/var/run/tog-pegasus/socket/cimxml.socket"
#  define PEGASUS_CORE_DIR                  "/var/opt/tog-pegasus/cache"
#  define PEGASUS_PAM_STANDALONE_PROC_NAME  "/opt/tog-pegasus/sbin/cimservera"
#  define PEGASUS_PROVIDER_AGENT_PROC_NAME  "/opt/tog-pegasus/sbin/cimprovagt"
#  undef PEGASUS_DEFAULT_MESSAGE_SOURCE     /* Not defined */
# elif defined(PEGASUS_OS_VMS)
#  define PEGASUS_CIMSERVER_START_FILE      \
    "/wbem_var/opt/wbem/cimserver_start.conf"
#  define PEGASUS_CIMSERVER_START_LOCK_FILE \
    "/wbem_var/opt/wbem/cimserver_start.lock"
#  define PEGASUS_REPOSITORY_DIR            "/wbem_var/opt/wbem/repository"
#  define PEGASUS_CURRENT_CONFIG_FILE_PATH  \
    "/wbem_var/opt/wbem/cimserver_current.conf"
#  define PEGASUS_PLANNED_CONFIG_FILE_PATH  \
    "/wbem_var/opt/wbem/cimserver_planned.conf"
#  define PEGASUS_SSLCLIENT_CERTIFICATEFILE \
    "/wbem_etc/opt/hp/sslshare/client.pem"
#  define PEGASUS_SSLCLIENT_RANDOMFILE      "/wbem_var/opt/wbem/ssl.rnd"
#  define PEGASUS_SSLSERVER_RANDOMFILE      "/wbem_var/opt/wbem/cimserver.rnd"
#  define PEGASUS_LOCAL_AUTH_DIR            "/wbem_var/opt/wbem/localauth"
#  undef PEGASUS_LOCAL_DOMAIN_SOCKET_PATH   /* Not used */
#  define PEGASUS_PROVIDER_AGENT_PROC_NAME  "/wbem_var/opt/wbem/bin/cimprovagt"
#  undef PEGASUS_DEFAULT_MESSAGE_SOURCE     /* Not defined */
# elif defined(PEGASUS_OS_ZOS)
#  define PEGASUS_CIMSERVER_START_FILE      "/var/wbem/cimserver.pid"
#  define PEGASUS_CIMSERVER_START_LOCK_FILE "/var/wbem/cimserver_start.lock"
#  define PEGASUS_REPOSITORY_DIR            "/var/wbem/repository"
#  define PEGASUS_CURRENT_CONFIG_FILE_PATH  "/etc/wbem/cimserver_current.conf"
#  define PEGASUS_PLANNED_CONFIG_FILE_PATH  "/etc/wbem/cimserver_planned.conf"
#  undef PEGASUS_SSLCLIENT_CERTIFICATEFILE  /* Not used */
#  undef PEGASUS_SSLCLIENT_RANDOMFILE       /* Not used */
#  undef PEGASUS_SSLSERVER_RANDOMFILE       /* Not used */
#  define PEGASUS_LOCAL_AUTH_DIR            "/tmp"
#  define PEGASUS_LOCAL_DOMAIN_SOCKET_PATH  "/var/wbem/cimxml.socket"
#  undef PEGASUS_PAM_STANDALONE_PROC_NAME   /* Not used */
#  define PEGASUS_PROVIDER_AGENT_PROC_NAME  "bin/cimprovagt"
#  define PEGASUS_DEFAULT_MESSAGE_SOURCE    "/usr/lpp/wbem/msg"
# elif defined(PEGASUS_OS_DARWIN)
#  define PEGASUS_CIMSERVER_START_FILE      \
    "/var/cache/pegasus/cimserver_start.conf"
#  define PEGASUS_CIMSERVER_START_LOCK_FILE \
    "/var/cache/pegasus/cimserver_start.lock"
#  define PEGASUS_REPOSITORY_DIR            "repository"
#  define PEGASUS_CURRENT_CONFIG_FILE_PATH  \
    "/etc/pegasus/cimserver_current.conf"
#  define PEGASUS_PLANNED_CONFIG_FILE_PATH  \
    "/etc/pegasus/cimserver_planned.conf"
#  define PEGASUS_SSLCLIENT_CERTIFICATEFILE "client.pem"
#  define PEGASUS_SSLCLIENT_RANDOMFILE      "ssl.rnd"
#  define PEGASUS_SSLSERVER_RANDOMFILE      "cimserver.rnd"
#  define PEGASUS_LOCAL_AUTH_DIR            "/tmp"
#  define PEGASUS_LOCAL_DOMAIN_SOCKET_PATH  "/tmp/cimxml.socket"
#  define PEGASUS_PAM_STANDALONE_PROC_NAME  "bin/cimservera"
#  define PEGASUS_PROVIDER_AGENT_PROC_NAME  "bin/cimprovagt"
#  undef PEGASUS_DEFAULT_MESSAGE_SOURCE     /* Not defined */
# endif
#endif
#else  // Not PEGASUS_USE_RELEASE_DIRS
# if defined(PEGASUS_OS_LINUX)
#  define PEGASUS_CIMSERVER_STARTFILES_DIR "/var/tmp"
# else
#  define PEGASUS_CIMSERVER_STARTFILES_DIR "/tmp"
# endif

#ifdef PEGASUS_FLAVOR
# define PEGASUS_CIMSERVER_START_FILE      \
    PEGASUS_CIMSERVER_STARTFILES_DIR "/cimserver" PEGASUS_FLAVOR "_start.conf"
# define PEGASUS_CIMSERVER_START_LOCK_FILE \
    PEGASUS_CIMSERVER_STARTFILES_DIR "/cimserver" PEGASUS_FLAVOR "_start.lock"
#else
# define PEGASUS_CIMSERVER_START_FILE      \
    PEGASUS_CIMSERVER_STARTFILES_DIR "/cimserver_start.conf"
# define PEGASUS_CIMSERVER_START_LOCK_FILE \
    PEGASUS_CIMSERVER_STARTFILES_DIR "/cimserver_start.lock"
#endif

# define PEGASUS_REPOSITORY_DIR            "repository"
# define PEGASUS_CURRENT_CONFIG_FILE_PATH  "cimserver_current.conf"
# define PEGASUS_PLANNED_CONFIG_FILE_PATH  "cimserver_planned.conf"
# define PEGASUS_SSLCLIENT_CERTIFICATEFILE "client.pem"
# define PEGASUS_SSLCLIENT_RANDOMFILE      "ssl.rnd"
# define PEGASUS_SSLSERVER_RANDOMFILE      "cimserver.rnd"
# define PEGASUS_LOCAL_AUTH_DIR            PEGASUS_CIMSERVER_STARTFILES_DIR

#ifdef PEGASUS_FLAVOR
# define PEGASUS_LOCAL_DOMAIN_SOCKET_PATH  \
    PEGASUS_CIMSERVER_STARTFILES_DIR "/cimxml" PEGASUS_FLAVOR ".socket"
#else
# define PEGASUS_LOCAL_DOMAIN_SOCKET_PATH  \
    PEGASUS_CIMSERVER_STARTFILES_DIR "/cimxml.socket"
#endif

# define PEGASUS_PAM_STANDALONE_PROC_NAME  "bin/cimservera"
# define PEGASUS_PROVIDER_AGENT_PROC_NAME  "bin/cimprovagt"
# undef PEGASUS_DEFAULT_MESSAGE_SOURCE      /* Not defined */
#endif

#define PEGASUS_PROVIDER_AGENT32_PROC_NAME \
    PEGASUS_PROVIDER_AGENT_PROC_NAME "32"

/* Use the PID file as a semaphore for repository access */
/* Use the configuration file instead of the PID file on Windows*/
#ifdef PEGASUS_OS_TYPE_WINDOWS
#define PEGASUS_REPOSITORY_LOCK_FILE PEGASUS_PLANNED_CONFIG_FILE_PATH
#else
#define PEGASUS_REPOSITORY_LOCK_FILE PEGASUS_CIMSERVER_START_FILE
#endif

/*
 * Miscellaneous Constants
 */

/*
  Constant defines the maximum number of providers that will be addressed in
  a single enumerate instance request.  This is strictly a performance
  requirement for different systems.  To disable the whole test, simply set
  this variable to 0
*/
#define MAX_ENUMERATE_BREADTH 30

/* Constant defines the maximum timeout to disable a provider */
#define PROVIDER_DISABLE_TIMEOUT 15

/* Constants defining the operational status of a CIM_ManagedSystemElement */
#define CIM_MSE_OPSTATUS_VALUE_OK 2
#define CIM_MSE_OPSTATUS_VALUE_DEGRADED 3
#define CIM_MSE_OPSTATUS_VALUE_ERROR 6
#define CIM_MSE_OPSTATUS_VALUE_STOPPING 9
#define CIM_MSE_OPSTATUS_VALUE_STOPPED 10

/* Constants defining UserContext values in a PG_ProviderModule */
#define PG_PROVMODULE_USERCTXT_REQUESTOR 2
#define PG_PROVMODULE_USERCTXT_DESIGNATED 3
#define PG_PROVMODULE_USERCTXT_PRIVILEGED 4
#define PG_PROVMODULE_USERCTXT_CIMSERVER 5

/* Constants defining Bitness values in a PG_ProviderModule */
#define PG_PROVMODULE_BITNESS_DEFAULT 1
#define PG_PROVMODULE_BITNESS_32 2
#define PG_PROVMODULE_BITNESS_64 3

/* Constant defining Server truststore type value in a PG_SSLCertificate */
#define PG_SSLCERTIFICATE_TSTYPE_VALUE_SERVER Uint16(2)

#ifdef PEGASUS_DEFAULT_USERCTXT_REQUESTOR
#define PEGASUS_DEFAULT_PROV_USERCTXT PG_PROVMODULE_USERCTXT_REQUESTOR
#else
#define PEGASUS_DEFAULT_PROV_USERCTXT PG_PROVMODULE_USERCTXT_PRIVILEGED
#endif

/* Constants defining the size of the hash table used in the OrderedSet
   implementation. Specific classes have their own hash table size to
   accomodate for amounts of probable members
*/
#define PEGASUS_PROPERTY_ORDEREDSET_HASHSIZE 32
#define PEGASUS_QUALIFIER_ORDEREDSET_HASHSIZE 16
#define PEGASUS_PARAMETER_ORDEREDSET_HASHSIZE 16
#define PEGASUS_METHOD_ORDEREDSET_HASHSIZE 16

/* defines a number value for an OrderedSet when either
   the index to a specific member is unknown, meaning not initialized
   (PEGASUS_ORDEREDSET_INDEX_UNKNOWN)
   of the specific member is not part of the list
   (PEGASUS_ORDEREDSET_INDEX_NOTFOUND)
*/
#define PEGASUS_ORDEREDSET_INDEX_NOTFOUND 0xFFFFFFFF
#define PEGASUS_ORDEREDSET_INDEX_UNKNOWN 0xFFFFFFFE

/*
   Defines the maximum authenticated username length.
*/
#ifdef PEGASUS_OS_PASE
#define PEGASUS_MAX_USER_NAME_LEN 10
#else
#define PEGASUS_MAX_USER_NAME_LEN 256
#endif

/*
   Defines the min/max size of the memory trace buffer
*/
#define PEGASUS_TRC_BUFFER_MAX_SIZE_KB 1024*1024
#define PEGASUS_TRC_BUFFER_MIN_SIZE_KB 16

#ifdef PEGASUS_OS_ZOS
# define PEGASUS_TRC_BUFFER_OOP_SIZE_DEVISOR 1
#else
# define PEGASUS_TRC_BUFFER_OOP_SIZE_DEVISOR 10
#endif

/*
 * Minimum value for maxLogFileSizeKBytes config property.
*/
#if !defined(PEGASUS_USE_SYSLOGS)
#define PEGASUS_MAXLOGFILESIZEKBYTES_CONFIG_PROPERTY_MINIMUM_VALUE 32
#endif

/*
**==============================================================================
**
**  C++ Section
**
**==============================================================================
*/

#if defined(__cplusplus)
# include <Pegasus/Common/CIMName.h>

PEGASUS_NAMESPACE_BEGIN

/**
    Values for the Subscription State property of the Subscription class,
    as defined in the CIM Events MOF
 */
enum SubscriptionState {STATE_UNKNOWN = 0, STATE_OTHER = 1,
     STATE_ENABLED = 2, STATE_ENABLEDDEGRADED = 3,
     STATE_DISABLED = 4};

/**
    Values for the Persistence Type property of the Handler class,
    as defined in the CIM Events MOF
 */
enum PersistenceType {PERSISTENCE_OTHER = 1, PERSISTENCE_PERMANENT = 2,
    PERSISTENCE_TRANSIENT = 3};

/**
    Values for the SNMPVersion property of the PG_IndicationHandlerSNMPMapper
    class, as defined in the PG Events MOF
 */
enum SnmpVersion {SNMPV1_TRAP = 2, SNMPV2C_TRAP = 3, SNMPV2C_INFORM = 4,
     SNMPV3_TRAP = 5, SNMPV3_INFORM = 6};

/**
   Values for the AlertCause property of the PG_ProviderModuleInstAlert
   class, as defined in the PG Events MOF

 */
enum PMInstAlertCause {PM_UNKNOWN = 1, PM_OTHER = 2, PM_CREATED = 3,
    PM_DELETED = 4, PM_ENABLED = 5, PM_DISABLED = 6, PM_DEGRADED = 7,
    PM_FAILED = 8, PM_FAILED_RESTARTED = 9, PM_GROUP_CHANGED = 10,
    PM_PROVIDER_ADDED = 11, PM_PROVIDER_REMOVED = 12,
    PM_ENABLED_CIMSERVER_START = 13, PM_DISABLED_CIMSERVER_STOP = 14};

/* Values for Delivery mode property of CIM_ListenerDestinationWSManagement
    class , as defined in CIM_ListenerDestinationWSManagement.mof */

enum deliveryMode {Push = 2 ,PushWithAck = 3, Events = 4 ,Pull = 5,
    DMTF_Reserved = 6 , Vendor_Reserved = 7 };

//
// CIM Class Names
//

PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_CONFIGSETTING;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_AUTHORIZATION;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_USER;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_CERTIFICATE;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_CRL;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_PROVIDERMODULE;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_PROVIDER;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_CAPABILITIESREGISTRATION;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_CONSUMERCAPABILITIES;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PROVIDERCAPABILITIES;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_INDSUBSCRIPTION;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_FORMATTEDINDSUBSCRIPTION;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_INDHANDLER;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_LSTNRDST;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_INDHANDLER_CIMXML;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_LSTNRDST_CIMXML;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_INDHANDLER_SNMP;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_INDHANDLER_WSMAN;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_LSTNRDST_SYSTEM_LOG;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_LSTNRDST_FILE;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_LSTNRDST_EMAIL;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_INDFILTER;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PROVIDERMODULE_INSTALERT;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_SHUTDOWN;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME___NAMESPACE;

#ifndef PEGASUS_DISABLE_PERFINST
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_CIMOMSTATDATA;
#endif

#ifdef PEGASUS_ENABLE_CQL
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_CIMQUERYCAPABILITIES;
#endif

PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_PROVIDERINDDATA;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_SUBSCRIPTIONINDDATA;

// Definitions for Interop Classes Accessed through Interop Control Provider

PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_CIMNAMESPACE;

PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_PG_OBJECTMANAGER;

#if defined PEGASUS_ENABLE_INTEROP_PROVIDER

PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_OBJECTMANAGER;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_PGNAMESPACE;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_OBJECTMANAGERCOMMUNICATIONMECHANISM;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_CIMXMLCOMMUNICATIONMECHANISM;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_CIMXMLCOMMUNICATIONMECHANISM;
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_PROTOCOLADAPTER;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_NAMESPACEINMANAGER;

//
// Server Profile-related class names
//

PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_COMMMECHANISMFORMANAGER;

PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_NAMESPACEINMANAGER;

PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_REGISTEREDPROFILE;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_REGISTEREDSUBPROFILE;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_REFERENCEDPROFILE;

PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_CIM_ELEMENTCONFORMSTOPROFILE;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_ELEMENTCONFORMSTOPROFILE_RP_RP;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_SUBPROFILEREQUIRESPROFILE;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_SOFTWAREIDENTITY;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_ELEMENTSOFTWAREIDENTITY;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_INSTALLEDSOFTWAREIDENTITY;

PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_COMPUTERSYSTEM;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_HOSTEDOBJECTMANAGER;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_HOSTEDACCESSPOINT;

// Profile Registration classes
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_PROVIDERPROFILECAPABILITIES;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_PROVIDERREFERENCEDPROFILES;

#endif    // PEGASUS_ENABLE_INTEROP_PROVIDER

// slp Class which Controls slp provider. Started by system

#ifdef PEGASUS_ENABLE_SLP

#define PEGASUS_SLP_SERVICE_TYPE   "service:wbem"

PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_CLASSNAME_WBEMSLPTEMPLATE;
#endif

// DMTF Indications profile classes
#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_CIM_INDICATIONSERVICE;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_CIM_INDICATIONSERVICECAPABILITIES;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_ELEMENTCAPABILITIES;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_HOSTEDINDICATIONSERVICE;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_SERVICEAFFECTSELEMENT;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_CLASSNAME_PG_LSTNRDSTQUEUE;
#endif

//
// Property Names Indication Subscriptions
//

PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_PROPERTYNAME_INDSUB_CREATOR;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_PROPERTYNAME_INDSUB_ACCEPTLANGS;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_PROPERTYNAME_INDSUB_CONTENTLANGS;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_PROPERTYNAME_MODULE_USERCONTEXT;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_PROPERTYNAME_MODULE_DESIGNATEDUSER;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_PROPERTYNAME_MODULE_MODULEGROUPNAME;
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_PROPERTYNAME_MODULE_BITNESS;

/**
    The name of the Destination property for CIM XML Indication Handler
    subclass
*/
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_PROPERTYNAME_LSTNRDST_DESTINATION;

/**
    Property names for WSMAN Indication Handler subclass.
*/
// Delivery Mode
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_PROPERTYNAME_WSM_DELIVERY_MODE;

/**
    Property names for File Indication Handler subclass.
*/
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_PROPERTYNAME_LSTNRDST_FILE;

/**
    The name of the CreationTime property for CIM XML Indication Handler
    subclass.
*/

PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_PROPERTYNAME_LSTNRDST_CREATIONTIME;

/**
    The name of the TargetHost property for SNMP Mapper Indication
    Handler subclass
*/
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_PROPERTYNAME_LSTNRDST_TARGETHOST;

/**
    The name of the TextFormat property for Formatted Indication
    Subscription class
*/
    PEGASUS_COMMON_LINKAGE extern const CIMName _PROPERTY_TEXTFORMAT;

/**
    The name of the TextFormatParameters property for Formatted
    Indication Subscription class
*/
    PEGASUS_COMMON_LINKAGE extern const CIMName _PROPERTY_TEXTFORMATPARAMETERS;

/**
    The name of the Name property for PG_ProviderModule class
*/
    PEGASUS_COMMON_LINKAGE extern const CIMName _PROPERTY_PROVIDERMODULE_NAME;

/**
    The name of the operational status property
*/
    PEGASUS_COMMON_LINKAGE extern const CIMName _PROPERTY_OPERATIONALSTATUS;

/**
    The name of the SubscriptionInfo property for Formatted Indication
    Subscription class
*/
    PEGASUS_COMMON_LINKAGE extern const CIMName _PROPERTY_SUBSCRIPTION_INFO;

/**
    The name of the Filter reference property for indication subscription class
 */
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_PROPERTYNAME_FILTER;

/**
    The name of the Handler reference property for indication subscription class
 */
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_PROPERTYNAME_HANDLER;

/**
    The name of the Subscription State property for indication subscription
    class
 */
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_PROPERTYNAME_SUBSCRIPTION_STATE;

/**
    The name of the Query property for indication filter class
 */
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_PROPERTYNAME_QUERY;

/**
    The name of the Query Language property for indication filter class
 */
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_PROPERTYNAME_QUERYLANGUAGE;

/**
    The name of the Name property for indication filter and indications handler
    classes
 */
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_PROPERTYNAME_NAME;

/**
    The name of the SubscriptionRemovalTimeInterval property of
    IndicationService class.
*/
PEGASUS_COMMON_LINKAGE extern const
    CIMName _PROPERTY_SUBSCRIPTIONREMOVALTIMEINTERVAL;

/**
    The name of the Creation Class Name property for indication filter and
    indications handler classes
 */
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_PROPERTYNAME_CREATIONCLASSNAME;

/**
    The name of the Persistence Type property for Indication Handler class
 */
PEGASUS_COMMON_LINKAGE
    extern const CIMName PEGASUS_PROPERTYNAME_PERSISTENCETYPE;

/**
    The name of the SNMP Version property for SNMP Mapper Indication Handler
    subclass
 */
PEGASUS_COMMON_LINKAGE extern const CIMName PEGASUS_PROPERTYNAME_SNMPVERSION;

//
// CIM Namespace Names
//

PEGASUS_COMMON_LINKAGE
    extern const CIMNamespaceName PEGASUS_NAMESPACENAME_INTEROP;
PEGASUS_COMMON_LINKAGE
    extern const CIMNamespaceName PEGASUS_NAMESPACENAME_INTERNAL;
PEGASUS_COMMON_LINKAGE
    extern const CIMNamespaceName PEGASUS_NAMESPACENAME_CIMV2;

PEGASUS_COMMON_LINKAGE
    extern const CIMNamespaceName PEGASUS_NAMESPACENAME_AUTHORIZATION;
PEGASUS_COMMON_LINKAGE
    extern const CIMNamespaceName PEGASUS_NAMESPACENAME_CONFIG;
PEGASUS_COMMON_LINKAGE
    extern const CIMNamespaceName PEGASUS_NAMESPACENAME_PROVIDERREG;
PEGASUS_COMMON_LINKAGE
    extern const CIMNamespaceName PEGASUS_NAMESPACENAME_SHUTDOWN;
PEGASUS_COMMON_LINKAGE
    extern const CIMNamespaceName PEGASUS_NAMESPACENAME_USER ;
PEGASUS_COMMON_LINKAGE
    extern const CIMNamespaceName PEGASUS_NAMESPACENAME_CERTIFICATE;

#ifndef PEGASUS_DISABLE_PERFINST
PEGASUS_COMMON_LINKAGE
    extern const CIMNamespaceName PEGASUS_NAMESPACENAME_CIMOMSTATDATA;
#endif

#ifdef PEGASUS_ENABLE_CQL
PEGASUS_COMMON_LINKAGE
    extern const CIMNamespaceName PEGASUS_NAMESPACENAME_CIMQUERYCAPABILITIES;
#endif

/**
    An EnumerateInstances operation on the __Namespace class returns the
    "child" namespaces of the namespace in which the operation is requested.
    Since there is no universal top-level namespace, one cannot find all
    namespaces by enumerating the __Namespace class.  To circumvent this
    problem, a virtual top-level namespace is introduced.  An enumeration of
    __Namespace in the virtual top-level namespace returns all namespaces.
*/
PEGASUS_COMMON_LINKAGE
    extern const CIMNamespaceName PEGASUS_VIRTUAL_TOPLEVEL_NAMESPACE;

/**
    Values for the Perceived Severity property of the Alert Indication
    class, as defined in the CIM Events MOF
 */
enum PerceivedSeverity {_SEVERITY_UNKNOWN = 0, _SEVERITY_OTHER = 1,
    _SEVERITY_INFORMATION = 2, _SEVERITY_WARNING = 3, _SEVERITY_MINOR = 4,
    _SEVERITY_MAJOR = 5, _SEVERITY_CRITICAL = 6, _SEVERITY_FATAL = 7};

/**
    The name of the MailTo property for Email Handler subclass
*/
PEGASUS_COMMON_LINKAGE extern const CIMName
    PEGASUS_PROPERTYNAME_LSTNRDST_MAILTO;

/**
    The name of the MailSubject property for Email Handler subclass
*/
PEGASUS_COMMON_LINKAGE extern const CIMName
    PEGASUS_PROPERTYNAME_LSTNRDST_MAILSUBJECT;

/**
    The name of the MailCc  property for Email Handler subclass
*/
PEGASUS_COMMON_LINKAGE extern const
    CIMName PEGASUS_PROPERTYNAME_LSTNRDST_MAILCC;

/**
    The qualifier name of embedded objects/instances.
*/
PEGASUS_COMMON_LINKAGE extern const
    CIMName PEGASUS_QUALIFIERNAME_EMBEDDEDOBJECT;

PEGASUS_COMMON_LINKAGE extern const
    CIMName PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE;

PEGASUS_NAMESPACE_END

#endif /* defined(__cplusplus) */

#endif /* Pegasus_Constants_h */
