# Running and Configuring OpenPegasus

## Starting and stopping the server.

The OpenPegasus server is a single executable that requires only
minimum set of environment variables to execute.

The required environment variables required are those that define the runtime
location for the server/client executablesslibraries, location
of SSL keys, trust directories,  crl directories,
and the CIM class repository for the server.

Normally all these are created under a single directory, **PEGASUS_HOME **
which is defined for the build

* **PEGASUS_HOME** - Target directory for compile output; contains object/executable files
for OpenPegasus as part of the build and other
components required for the running OpenPegasus are included in this directory
and its subdirectories as part of the setup.  Thus, binary files, the class repository,
the directories for certificates and certificate revocation lists, any trace
output, are normally placed in subdirectories of **PEGASUS_HOME**. Normally all
of the components of the build and runtime data are built under the
single **PEGASUS_HOME** Directory

The command to start the server is :

    cimserver <runtime options>

    where each runtime option is defined as

       name=value

    name is the name of the option
    value is the value to be set.

    Runtime options are those options defined for the
    for OpenPegasus in the `cimconfig` command CLI.

Thus:

    cimserver traceLevel=2 traceComponents=XMLIO

Starts the server with the cimconfig runtime options set to
trace the execution of the server at trace level 2 and to
trace only the component XMLIO.  This is set as a current
option definition so that the option is defined only for the
current execution of the server.

In addition, the run environment must be able to find the executables
for the server, client executables, etc.  Thus the path to the
binaries for OpenPegasus should be included in the PATH environment variable.

    export PATH=$PEGASUS_HOME/bin:$PATH

Finally the location of the libraries may need to be known. The
environment variable LD_LIBRARY_PATH covers that. For example:

    export LD_LIBRARY_PATH=$PEGASUS_HOME/lib64:

## Configuring the CIMServer

The OpenPegasus CIMServer run configuration is controlled by the
options define in the CLI `cimconfig`, see
<<<<<<< HEAD
[OpenPegasus Runtime Options section](#openPegasus-runtime-options) .
They may be set either as planned (static) options (the value is
retained in the server CIM Repository, is applied at the next sever startup and
is persistent through server start and stop) or current options where
they are set only in the current execution of the server.

`cimconfig` is a command line client that can set or unset any of the
options defined in the CLI **cimconfig**, see
=======
>>>>>>> 5f9c4a0 (Changes status to Released (#100))
[OpenPegasus Runtime Options section](#openPegasus-runtime-options) .
They may be set either as planned (static) options (the value is
retained in the server CIM Repository, is applied at the next sever startup and
is persistent through server start and stop) or current options where
they are set only in the current execution of the server.

<<<<<<< HEAD
=======
`cimconfig` is a command line client that can set or unset any of the
>>>>>>> 5f9c4a0 (Changes status to Released (#100))
OpenPegasus runtime configuration properties, show the current status of each of
the runtime configuration properties.

The options may also be modified through the WEBAdmin facility if it is
enabled.

The syntax of the `cimconfig` command is:

```
cimconfig --help
Usage: cimconfig -g name [ -c ] [ -d ] [ -p ]
                 -s name=value [ -c ] [ -p ] [ -t ]
                 -u name [ -c ] [ -p ] [ -t ]
                 -l [ -c | -p ]
                 -H name | "All"
                 -h
                 --help
                 --version
Options :
    -c         - Use current configuration
                 it is the default optional option for options like -l
    -d         - Use default configuration
    -g         - Get the value of specified configuration property
    -H         - Get help on specified configuration property
                 (or all configuration properties with keyword "All"
    -h, --help - Display this help message
    -l         - Display all the configuration properties
    -p         - Configuration used on next CIM Server start
    -s         - Add or Update configuration property value
    -u         - Reset configuration property to its default value
    -t         - Timeout value in seconds for updating the current or
                 planned value
    --version  - Display CIM Server version number

Usage note: The cimconfig command can be used to update the next planned
    configuration without having the CIM Server running. All other options
    except -h, --help and --version of the cimconfig command require that the
    CIM Server is running.

```

Thus the following command sets the traceLevel to the integer 5 in the
current settings:

    cimconfig -s traceLevel=5 -c

The following command resets the traceLevel to the default value

    cimconfig -u traceLevel -c


### OpenPegasus Runtime Options

The options are documented both in the help for the `cimconfig -H ALL` cli,
in the documentation for OpenPegasus
[docs/BuildAndReleaseOptions.md](docs/BuildAndReleaseOptions.md).

The following is the list of cimconfig run time variables as documented
in the `cimconfig` CLI with the command `cimconfig -H ALL`.
<<<<<<< HEAD

Options defined as static are enabled only when the server is
restarted. Dynamic options may be modified when the server is running.
in the **cimconfig** CLI with the command **cimconfig -H ALL**
=======
>>>>>>> 5f9c4a0 (Changes status to Released (#100))

Options defined as static are enabled only when the server is
restarted. Dynamic options may be modified when the server is running.

```
authorizedUserGroups (Static,Default:)

If set, the value is list of comma-separated user
groups whose members may issue CIM requests. A user not a member of
any of these groups is restricted from issuing CIM requests, except
for privileged users (root user). If not set, any user may issue
CIM requests.

crlStore (Static,Default:crl)

Specifies location of OpenSSL certificate revocation list.
if enabled.

daemon (Static,Default:true)

If 'true' enables forking of the CIM Server to create a background
daemon process.

enableAssociationTraversal (Static,Default:true)

If 'true', CIM Server supports the association traversal operators:
 Associators, AssociatorNames,References, and ReferenceNames.

enableAuditLog (Dynamic,Default:false)

If 'true', audit audit log entries for certain types
of CIM Server activities (i.e. activities that modify server state)
will be written to the system log. Aaudit log entries describe the
who, what, and when of audited activity.

enableAuthentication (Static,Default:false)

If 'true', a Client must be authenticated to access the CIM Server.

enableBinaryRepository (Static,Default:false)

If 'true' activates the binary repository support on the next
restart of cimserver. OpenPegasus supports repositories with a
mixture of binary and XML objects.

enableHttpConnection (Static,Default:true)

If 'true', allows connection to CIM Server using HTTP protocol

enableHttpsConnection (Static,Default:true)

If 'true', allows connection to the CIM Server using HTTPS protocol
(HTTP using Secure Socket Layer encryption)

enableIndicationService (Dynamic,Default:true)
If 'true', the CIM Server will support CIM Indications.

enableNamespaceAuthorization (Static,Default:false)

If 'true', CIM Server restricts access to namespaces based on
configured user authorizations [user authorizations may be
configured using the 'cimauth' command]

enableRemotePrivilegedUserAccess (Static,Default:true)

If 'true', the CIM Server allows access by a privileged user from a
remote system. Many management operations require privileged user
access. Disabling remote access by privileged user could
significantly affect functionality.

enableSubscriptionsForNonprivilegedUsers (Static,Default:true)

If 'true', operations on indication filter, listener destination,
and subscription instances may be performed by non-privileged
users.Otherwise, these operations limited to privileged users.

forceProviderProcesses (Static,Default:true)

If 'true', CIM Server runs Providers in separate processes rather
than loading and calling Provider libraries directly within
CIM Server process.

fullyQualifiedHostName (Static,Default:)

Override local system supplied fully
qualified hostname (with domain)CIM Server uses to build objects
for WBEM Operations that return hostname (ex. associators).
Used where environment needs operations to return operation
responses with hostname other than server name (ex. behind firewalls).
When setting this option 'hostname' should be set also.

hostname (Static,Default:)

Override local system supplied hostname CIM Server uses to
build objects for WBEM operations that return hostname (ex.
associators). Used where environment needs operations to
return operation responses with hostname other than server name
(ex. behind firewalls). When setting this option,
'fullyQualifiedHostName' should be set also.

httpAuthType (Static,Default:Basic)

Type of HTTP authentication. Either 'basic'
indicating basic authentication or 'negotiate' indicating use of
HTTP Negotiate authentication method. 'basic' is the default one.

httpPort (Static,Default:)

Integer defines HTTP port number.
OpenPegasus first attempts to look up port number for HTTP
using getservbyname for the 'wbem-http' service. httpPort.
configuration setting used only when getservbyname lookup fails.
Use of port 5988 recommended by DMTF

httpSessionTimeout (Dynamic,Default:0)

If set to a positive integer, value specifies a HTTP session
cookie expiration time in seconds. If set to 0, cookie
processing in Pegasus is disabled, it will not send nor accept
any cookies.

httpsPort (Static,Default:)

Integer that defines HTTPS port number.
OpenPegasus first attempts to look up port number for
HTTPS using getservbyname for the 'wbem-https' service. httpPorts
configuration setting used only when the getservbyname lookup fails.
Use of port 5989 recommended by DMTF

idleConnectionTimeout (Dynamic,Default:0)

If set to a positive integer, value specifies a minimum timeout
value for idle client connections. If set to zero, idle client
connections do not time out.

indexFile (Static,Default:index.html)
Specifies the name of index file used by pegasus webserver, default
to index.html. This file should be available at the webRoot path

listenAddress (Static,Default:All)

Network interface where the cimserver listens for connections. Default
value  'All' forces listening at all interfaces. A comma seperated
list(without spaces) forces listening at specific interfaces. For
example: 'listenAddress=All' or 'listenAddress=121.12.33.112', or
'listenAddress=121.33.21.26,127.0.0.1,fe80::fe62:9346%eth0'

logLevel (Dynamic,Default:INFORMATION)

Keyword defines the desired level of logging.
Log levels are accumulative(i.e. 'warning' logs 'warning', 'severe',
and 'fatal'.
Possible Values:
    'trace'       Most Detailed. Output all log events
    'information' Log information level. Default
    'warning'     Log warnings and more severe events
    'severe'      Log fatal and severe events
    'fatal'       Log only events that are fatal to server

maxFailedProviderModuleRestarts (Dynamic,Default:3)

If set to a positive integer, this value specifies the number of
times a failed provider module with indications enabled is restarted
automatically before being moved to Degraded state. If set to zero,
failed provider module is not restarted with indications enabled
automatically and will be moved to Degraded state immediately.

maxIndicationDeliveryRetryAttempts (Dynamic,Default:3)

If set to a positive integer, value defines the number of times
indication service will enable the reliableIndication feature
& try to deliver an indication to a particular listener destination.
This does not effect the original delivery attempt. A value of 0
disables reliable indication feature completely, and cimserver will
deliver the indication once.

maxProviderProcesses (Dynamic,Default:0)

Limits number of provider processes (see 'forceProviderProcesses')
that may run concurrently. Value '0' indicates that the number of
 Provider Agent processes unlimited

messageDir (Static,Default:msg)

Name of the directory used for the OpenPegasus translated
message bundles.

mimeTypesFile (Static,Default:www/mimeTypes.txt)

Refers to the file which holds the mime types being served by the
pegasus webserver

minIndicationDeliveryRetryInterval (Dynamic,Default:30)

If set to a positive integer, defines the minimal time interval in
seconds indication service waits before retrying delivery of
indication to a listener destination that previously failed.
Cimserver may take longer due to QoS or other processing.

numberOfTraceFiles (Dynamic,Default:3)

Integer defines the maximum number of trace files
kept when trace files are rolled over upon reaching maximum size
'traceFileSizeKBytes'.  Minimum is 3. Maximum is 20

passwordFilePath (Static,Default:cimserver.passwd)

Path to password file if password file used for
user authentication.

providerDir (Dynamic,Default:lib)

Names of directories that contains provider executables.
If multiple directories they should be comma-separated.

providerManagerDir (Static,Default:lib)

Name of the directory containing ProviderManager plugin libraries.
Should be a dedicated directory to minimize inspection of
non-applicable files.

pullOperationsDefaultTimeout (Dynamic,Default:30)

Defines the default time in seconds of the 'operationTimeout' (the time
between open/pull operations within a single enumeration sequence)
the server uses when the client open... request 'operationTimeout' parameter
is NULL. Minimum value = 1 sec . Maximum value = 30sec

pullOperationsMaxObjectCount (Dynamic,Default:10000)

Defines the system limit on the number of instances or paths that the client
may request in the open... or pull... 'maxObjectCount' argument.
Minimum value = 1. Maximum value = 10000

pullOperationsMaxTimeout (Dynamic,Default:90)

Defines the system maximum limit on the time in seconds between open/pull
operations the client may request within an enumeration sequence with the
'operationTimeout' argument in open... client requests.
Minimum value = 1 sec. Maximum value = 90sec

repositoryDir (Static,Default:repository)

Name of directory to be used for the OpenPegasus repository.

repositoryIsDefaultInstanceProvider (Static,Default:true)

If 'true', Repository functions as Provider (for instance and
association operations) for all classes without an explicitly
registered Provider.

shutdownTimeout (Dynamic,Default:30)

When 'cimserver -s' shutdown command is issued, specifies maximum
time in seconds for CIM Server to complete outstanding CIM
operation requests before shutting down. If specified timeout
period expires, CIM Server shuts down, even with CIM operations
in progress. Minimum value is 2 seconds.

socketWriteTimeout (Dynamic,Default:20)

Integer defines the number of seconds the CIM Server will wait for a
client connection to be ready to receive data. If the CIM Server is
unable to write to a connection in this time, the connection is
closed.

sslBackwardCompatibility (Static,Default:false)

If 'true', OpenPegasus support SSLv3 and TLSv1 protocol.If 'false',
OpenPegasus support only TLS1.2 protocol for openssl 1.0.1+

sslCertificateFilePath (Static,Default:server.pem)

File containing the CIM Server SSL Certificate.

sslCipherSuite (Static,Default:DEFAULT)

String containing OpenSSL cipher specifications to configure
cipher suite the client is permitted to negotiate with the server
during SSL handshake phase. Enclose values in single quotes to
avoid issues with special characters. Default for this parameter is
the list of ciphers in OpenSSL.

sslClientVerificationMode (Static,Default:disabled)

Level of support for certificate-based authentication.
Valid only if 'enableHttpsConnection=true'
Possible Values:
'required': HTTPS client MUST present a trusted certificate to access
    CIM Server. If client fails to send a certificate or sends an
    untrusted certificate, the connection is rejected.
'optional': HTTPS client may, but is not required to, present a
   trusted certificate to access the server. Server requests and
   validates client certificate, but the connection is accepted if no
   certificate or an untrusted certificate is sent. Server will
   then seek to authenticate client via authentication header.
'disabled': CIM Server will not allow HTTPS clients to authenticate
   using a certificate. Basic authentication will be used to
   authenticate HTTPS clients

sslKeyFilePath (Static,Default:file.pem)

File Containing private key for CIM Server SSL Certificate.

sslTrustStore (Static,Default:cimserver_trust)

Specifies location of OpenSSL truststore.
Truststore can be either file or directory. If the truststore is a
directory, all certificates in the directory are trusted.

sslTrustStoreUserName (Static,Default:)

System user name to be associated with all certificate-based authenticated
requests.
No default; for security reasons, system administrator must
explicitly specify this value.  Allows a single user name to be
specified.  This user will be associated with all certificates in
the truststore

traceComponents (Dynamic,Default:)

Defines OpenPegasus server components to be traced.
Define multiple components as comma-separated list. 'ALL' traces
all components. Only OpenPegasus server components on this list
generate entries in the trace output.
Possible Values:
    ALL Xml XmlIO Http Repository Dispatcher OsAbstraction Config
    IndicationHandler Authentication Authorization UserManager Shutdown
    Server IndicationService MessageQueueService ProviderManager
    ObjectResolution WQL CQL FQL Thread CIMExportRequestDispatcher SSL
    ControlProvider CIMOMHandle L10N ExportClient Listener DiscardedData
    ProviderAgent IndicationFormatter StatisticalData CMPIProvider
    IndicationGeneration IndicationReceipt CMPIProviderInterface WsmServer
    RsServer WebServer LogMessages WMIMapperConsumer InternalProvider
    EnumContext

traceFacility (Dynamic,Default:File)

Keyword selects the trace destination.
Possible Values:
    'File'   Trace output to file defined by 'traceFilePath'
    'Log'    Trace output to log file
    'Memory' Trace output to memory

traceFilePath (Dynamic,Default:trace/cimserver.trc)

Specifies location and name of OpenPegasus trace file.
Ignored when 'traceFacility!=File.

traceFileSizeKBytes (Dynamic,Default:1048576)

Integer defines the maximum size of the tracefile in kbytes.
The trace file will be rolled over when size exceeds the specified
size. Minimum is 10,240 kbytes. Maximum is 2,097 mbytes.Ignored if
'traceFacility=memory'. See also 'numberOfTraceFiles'.

traceLevel (Dynamic,Default:0)

Single digit defines level of server tracing enabled.
Possible Values:
    0 Trace off (default)
    1 Severe and log messages
    2 Basic flow trace messages, low data detail
    3 Inter-function logic flow, medium data detail
    4 High data detail
    5 High data detail + Method Enter & Exit

traceMemoryBufferKbytes (Static,Default:10240)

Integer defines size of buffer for in-memory tracing.
Value is in in kbytes. Minimum value is 16 kbytes. Ignored if
'traceFacility!=memory'

webRoot (Static,Default:www)

Points to a location where the static web pages to be served by the
pegasus webserver are stored

```

## Server SSL certificates

See the Admin guide at OpenPegasus/pegasus/doc/Admin_Guide_Release.pdf

## Installing and controlling providers in OpenPegasus

See the Admin guide at OpenPegasus/pegasus/doc/Admin_Guide_Release.pdf

## OpenPegasus WEB Adminstration

Some characteristics of OpenPegasus can be manipulated through a WEB
WEB page managed by OpenPegasus (webadmin).
The existence of the WEB adminstration pags is controlled by the build
environment variable PEGASUS_ENABLE_PROTOCOL_WEB, which is 'true by
default'. To disable it set PEGASUS_ENABLE_PROTOCOL_WEB=false.

There is a readme `OpenPegasus/pegasus/webadmin` that provides more information
on WEBAdmin.

## Gathering and viewing Operation Statistics

OpenPegasus  is capable of gathering statistics on requests executed
including request and response size, number of operations executed, execution
time. This capability is normally disable but may be enabled by setting
a property in the CIM_ObjectManager instance within OpenPegasus.

The cli tool can set this property with the command `cimcli son` or turn
off statistics with `cimcli soff`. The pywbem python tool pywbemcli can
also enable this with its statistics commands.

Note that the statistics gathering is always not persistent, it is
disabled on server startup so it must be reenabled on each restart of
the server.

The file `OpenPegasus\pegasus\readme.statistics` provides more information
on statistics in OpenPegasus.

## OpenPegasus command line utilities

* cimauth - Cli for authorization manaagement. add, modify, remove or list
  CIM user authorizations.
* cimcli - Command line client to execute requests against the WBEM server.
* cimcrl -  Manage the certificate revocation list.
* cimconfig - Manage OpenPegasus run-time parameters.
* cimperf - Display performance information when server statistics are enabled.
* cimtrust - Manage server and indication listener certificates.
* cimmofl - MOF compiler for OpenPegasus. Compiles MOF directly into the
  currently defined server CIM repository. Generally this compiler should only
  be used to build a new CIM repository
* cimmof - MOF compiler for OpenPegasus. Compiles MOF by using the compiler
  as a WBEM client and sending the requests to create/modify/delete CIM
  classes and objects to the WBEM server.
* cimprovider - Provider management. Disable, enable, remove or list
  registered CIM providers or CIM provider modules and module status. For help
  there is an html file with the source code.
* cimsub - Command line to to manage CIM Subscriptions.
* cimuser - Manage users if PEGASUS_PAM_AUTHENTICATION functionality not
  compiled into OpenPegasus. Otherwise nto build


In general these tools have a --help option to get help with parameters and usage.

See the directory `OpenPegasus\pegasus\src\Clients` for list of all of the
client test and usage tools for OpenPegasus.
<<<<<<< HEAD

=======
>>>>>>> 5f9c4a0 (Changes status to Released (#100))
