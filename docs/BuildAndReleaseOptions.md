* * *

**Recommended OpenPegasus Build and Configuration Options for Selected Platforms**

* * *

**Abstract:** This document defines a set of recommended options for building, testing and running OpenPegasus on a selected set of platforms. In this version of the document, platform-specific configuration information have been documented for the platforms described in the "Documented Platforms" section of this document.

**Note 1:** The recommendations defined in this document are not intended to document the feature set included in any vendor's OpenPegasus-based product. Rather, the purpose of this document is to provide input to vendors when making product decisions.  

This document is mechanical translation of the document OpenPegasus/pegasus/BuildAndReleaseOptions.html and as such
there may be issues in the transformation to the markdown format that we did not catch
* * *

Definition of the Problem
-------------------------

OpenPegasus supports a large number of build and runtime options. Determining which options to use can be challenging. The purpose of this document is to simplify the build, testing and administration of OpenPegasus by providing a recommended, tested set of options.

Documented Platforms
--------------------

Support for the OpenPegasus build and runtime options described in this document varies by platform.   In this version of the document, platform-specific configuration information has been included for the platforms described in the following table.  For all other platforms, please consult the OpenPegasus Platform Advocate for additional information.

Platform

Status

HPUX\_IA64\_ACC

 

HPUX\_PARISC\_ACC

 

LINUX\_IA64\_GNU

 

LINUX\_IX86\_GNU

 

LINUX\_PPC64\_GNU

 

LINUX\_PPC\_GNU

 

LINUX\_PPC\_E500\_GNU

 

LINUX\_SH4\_GNU

 

LINUX\_XSCALE\_GNU

 

LINUX\_ZSERIES64\_GNU

 

LINUX\_ZSERIES\_GNU

 

VMS\_ALPHA\_DECCXX

 

VMS\_IA64\_DECCXX

 

Proposed Solution
-----------------

### Security Considerations

1.  To avoid introducing security vulnerabilities, vendors must never ship providers incompatible with the security assumptions used in their deployment. For example, providers designed for a single-user deployment that don't perform authorization must not be shipped with a CIM Server that expects the providers to perform authorization. Note that this authorization burden is considerably lighter if the provider is registered as 'run as requestor' and if the CIM Server enables that feature. See [PEP#223 - Security Coding Guidelines](http://www.openpegasus.org/pp/uploads/40/7197/SecurityGuidePEP.htm) for additional details.
2.  Vendors should ensure they configure the CIM Server consistently with the security requirements of their deployment.

### Terminology Notes

1.  In this document, the term "Not Set" is used to indicate that the a value for the environment variable has not been defined. "Set" is used to indicate that the value must be defined, but the actual value of the variable is not checked.

### General Notes

1.  Not all functionality described in this document has achieved "Green"status, please refer to the [OpenPegasus Project Feature Status](http://www.openpegasus.org/page.tpl?CALLER=index.tpl&ggid=799) page for information on the maturity level of OpenPegasus features.
2.  To encourage testing of new functionality, recommendations for the Development Build may include features that are not yet recommended for Release Builds.

### Environment Variable Settings for Building OpenPegasus

#### Definitions

##### ICU\_INSTALL

**Description:** When set, points to the directory containing the [ICU(International Components for Unicode)](%20http:/oss.software.ibm.com/icu) installation. The 'include' subdirectory is used to locate ICU header files at build time. The 'lib' subdirectory is used to locate ICU libraries at link time.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No.  However, if PEGASUS\_HAS\_ICU is set and ICU\_INSTALL is not defined, the ICU libraries and header files are expected to be installed in directories (e.g.,/usr/lib, /usr/include)  that are searched by default.  
**Considerations:** This environment variable is only used if PEGASUS\_HAS\_MESSAGES is set. Refer to PEGASUS\_HAS\_MESSAGES for additional details.

##### ICU\_ROOT\_BUNDLE\_LANG

**Description:** If set, specifies the language to be used to generate the root resource bundle. The default value for ICU\_ROOT\_BUNDLE\_LANG is \_en.  
**Default Value:** \_en  
**Recommended Value (Development Build):** No Recommendation  
**Recommended Value (Release Build):** No Recommendation  
**Required:** No  
**Considerations:** This environment variable is only used if PEGASUS\_HAS\_MESSAGES is set. Refer to PEGASUS\_HAS\_MESSAGES for additional details.  
If set, resource bundles for the designated language must exist for the following files:

*   pegasus/src/Pegasus/msg/Server/pegasusServer\_en.txt
*   pegasus/src/Pegasus/msg/Listener/pegasusListener\_en.txt

##### OPENSSL\_BIN

**Description:** Specifies the location of the OpenSSL _binary_ directory.  
**Default Value:** Not Set  
**Recommended Value (Development Build):**  No Specific Recommendation  
**Recommended Value (Release Build):**  No Specific Recommendation  
**Required:** No. By default, the openssl binary is expected to be installed in a directory that is searched by default (e.g., /usr/bin). For platforms where this isn't true, the OPENSSL\_BIN environment variable may be used to define an alternative path. This environment variable is only used if PEGASUS\_HAS\_SSL is set.

##### OPENSSL\_HOME

**Description:** Specifies the location of the OpenSSL SDK directory. This directory must contain the OpenSSL _include_ directory, $(OPENSSL\_HOME)/include, and the OpenSSL _library_ directory, $(OPENSSL\_HOME)/lib.  
**Default Value:** Not Set  
**Recommended Value (Development Build):**  No Specific Recommendation  
**Recommended Value (Release Build):**  No Specific Recommendation  
**Required:** Yes for Windows, if PEGASUS\_HAS\_SSL is set. No for all other platforms. For AIX, the default value for OPENSSL\_HOME is /usr/linux.  For all other platforms, the OpenSSL include files and libraries are expected to be installed in directories that are searched by default (e.g., /usr/bin, /usr/lib(64), and /usr/include). For platforms where the default value is not correct, the OPENSSL\_HOME environment variable may be used to define an alternative path (i.e., $(OPENSSL\_HOME)/lib and $(OPENSSL\_HOME)/include). This environment variable is only used if PEGASUS\_HAS\_SSL is set.

##### PEGASUS\_ALLOW\_ABSOLUTEPATH\_IN\_PROVIDERMODULE

**Description:** If set to true, a version of OpenPegasus will be built that allows an absolute path to be specified as the value of the Location property of PG\_ProviderModule class.  Otherwise it does not allow the absolute path to be specified.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No

**Considerations:** To avoid introducing security vulnerabilities, vendors should analyze their deployment environments to understand the security implications of allowing providers to be loaded from a dynamically changing set of directories.  

##### PEGASUS\_CCOVER

**Description:** This flag is currently only supported on HP-UX. It is enabled on the Nightly Build & Test system, HP-UX\_11iv3\_Integrity\_C-Cover.  This system is used to generate the nightly BullseyeCoverage code coverage metrics.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** This flag is only supported on HP-UX to enable CCOVER for the NB&T system.  

##### PEGASUS\_CIM\_SCHEMA

**Description:** This variable is used internally by the OpenPegasus development team when testing OpenPegasus in the integrated build/test environment. It defines the version of the CIM Schema (which must exist in the $(PEGASUS\_ROOT)/Schemas directory) that will be used to build the class repository for testing the pegasus environment ("make repository", etc.).  
**Default Value:** CIM241 (Note that this may change as newer versions of the CIM Schema are used as the default.  
**Recommended Value (Development Build):**  No Specific Recommendation  
**Recommended Value (Release Build):**  No Specific Recommendation  
**Required:** No  
**Considerations:** Additional code changes may be required when upgrading or downgrading the CIM Schema version from the default.  The default CIM Schema is defined in the file $(PEGASUS\_ROOT)/mak/config.mak.  
  

##### PEGASUS\_CLASS\_CACHE\_SIZE

**Description:** This environment variable gives the size of the class cache used by CIM repository. This variable defines the size of the class cache (i.e., the maximum number of classes that may be included). If this variable is set to 0, class caching is disabled.  
**Default Value:** 8  
**Recommended Value (Development Build):** 8  
**Recommended Value (Release Build):** 8  
**Required:** No

##### PEGASUS\_DEBUG

**Description:** Builds a debug version of OpenPegasus. Concurrently, this flag controls a) enabling compiler specific debug flags and b) the inclusion of debug-specific functionality.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No

##### PEGASUS\_DEFAULT\_USERCTXT\_REQUESTOR

**Description:**  If set to true, the _Requestor_ user context is used for providers that do not specify a UserContext value in their registration. Otherwise, the _Privileged_ user context value is used by default.  
**Default Value:** false  
**Recommended Value (Development Build):** false  
**Recommended Value (Release Build):** false  
**Required:** No **Considerations:** Use of the _Requestor_ default has improved provider security characteristics. However, some providers may not operate correctly when run in the _Requestor_ user context and setting this value to true may cause those providers to fail.  Providers that can run in the _Requestor_ user context are encouraged to explicitly set the UserContext value in their registration rather than rely on the default setting default by this variable. Please refer to SecurityGuidelinesForDevelopers.html. PEP 223 for a discussion of the rationale/advantages of running providers in the _Requestor_ user context. **Note:** This build variable is not meaningful if the Provider User Context feature is disabled with the PEGASUS\_DISABLE\_PROV\_USERCTXT setting. 

##### PEGASUS\_DEFAULT\_ENABLE\_OOP

**Description:** When this variable is set to false, by default, Providers are run in the cimserver process (forceProviderProcesses=false). If this value is set true, Providers are run out-of-process by default.  
**Default Value:** true for HP-UX and Linux, false for other platforms  
**Recommended Value (Development Build):** true (HP-UX, Linux)  
**Recommended Value (Release Build):** true (HP-UX, Linux)  
**Required:** No  
**Considerations:** This environment variable can be used to change the "hard-coded" default setting for the forceProviderProcesses configuration value.  Refer to the definition of **forceProviderProcesses** for additional information.

##### PEGASUS\_DEST\_LIB\_DIR

**Description:** Specifies the location of OpenPegasus shared libraries.  
**Default Value:** $(PEGASUS\_HOME)/lib  
**Recommended Value (Development Build):**  $(PEGASUS\_HOME)/lib  
**Recommended Value (Release Build):**  No Specific Recommendation  
**Required:** No  
**Considerations:** This variable is NOT is used to define the location of the Provider library directories.  Please refer to the providerDir configuration option for details on defining the directories for the Provider libraries.

##### PEGASUS\_DISABLE\_DEPRECATED\_INTERFACES

**Description:** Removes deprecated symbol definitions from OpenPegasus runtime libraries.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** If this option is set, the resulting OpenPegasus libraries will not be binary compatible with clients and providers built using interface definitions from prior releases. This option may be used to slightly reduce binary footprint in an environment where compatibility is not required.  

##### PEGASUS\_DISABLE\_INSTANCE\_QUALIFIERS

**Description:** In the CIM Infrastructure Specification, version 2.3, the DMTF clarified that instance-level qualifiers are not allowed.  In Version 1.2 of the Specification CIM Operations over HTTP, use of the IncludeQualifiers parameter has been DEPRECATED.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** Existing CIM Clients may depend on qualifiers being returned in the response. In this case, the PEGASUS\_DISABLE\_INSTANCE\_QUALIFIERS should not be set.  If this flag is set, the value of the IncludeQualifiers parameter passed on the GetInstance and EnumerateInstance operations is ignored and qualifiers are not returned.  Disabling the return of qualifiers as part of a CIM Instance will improve performance.  

##### PEGASUS\_DISABLE\_LOCAL\_DOMAIN\_SOCKET

**Description:** Disables support for local (same-system) connections over a Unix domain socket. If this option is NOT set, the CIM Server is built to allow connections to be established using a domain socket rather than a TCP port.  
**Default Value:** Not Set   
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** (1) In high-threat environments, a customer may want to disable all ports or reduce the number of exposed network ports. Supporting a local connection mechanism using Unix domain socket allows the CIM Server to continue to receive and process requests from local CIM Clients. (2) Enabling this option may result in lose of functionality when sslClientVerificationMode = required. (3)The "LOCAL\_DOMAIN\_SOCKET" functionality has not been implemented for  Windows or OpenVMS.  Therefore, by default, this option is "Set" for Windows and OpenVMS platforms.

##### PEGASUS\_DISABLE\_PERFINST

**Description:** Builds a version of OpenPegasus that disables support for gathering performance data.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Set  
**Required:** No  
**Considerations:** The CIM\_ObjectManager.GatherStatisticalData property is used to control statistic gathering.  Once enabled, statistical data can be viewed by retrieving instances of the CIM\_StatisticalData class. Support for gathering performance data requires PEGASUS\_ENABLE\_INTEROP\_PROVIDER to be enabled. If PEGASUS\_ENABLE\_INTEROP\_PROVIDER is explicitly disabled (i.e., set to false),  then PEGASUS\_DISABLE\_PERFINST must be set to true.  If PEGASUS\_DISABLE\_PERFINST is not defined or set to false and PEGASUS\_ENABLE\_INTEROP\_PROVIDER is not set, PEGASUS\_ENABLE\_INTEROP\_PROVIDER will be automatically enabled.

##### PEGASUS\_DISABLE\_PRIVILEGED\_TESTS

**Description:** This variable is used only in the development environment to restrict the level of testing performed by default. If set, tests that require the CIM Server to run in a privileged mode will not be run.  Setting this variable allows developers, without privileged access, to successfully run a subset of the OpenPegasus test suit.  This variable does not affect CIM Server functionality.  It merely controls the level of testing.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Used  
**Required:** No  
**Considerations:** Enabling this variable will significantly reduce the testing of security related features (e.g., authentication and authorization) and can affect testing in other areas.

##### PEGASUS\_DISABLE\_PROV\_USERCTXT

**Description:** Builds a version of OpenPegasus that disables supports for the Provider User Context feature.  This feature allows a Provider to choose the user context in which it is invoked, including the ability to run in the context of the user requesting an operation.  
**Default Value:** Set for Windows and OpenVMS, Not Set for all other platforms  
**Recommended Value (Development Build):** Set for Windows and OpenVMS, Not Set for all other platforms  
**Recommended Value (Release Build):** Set for Windows and OpenVMS, Not Set for all other platforms  
**Required:** No  
**Considerations:** The Provider User Context feature may be disabled by compiling with the PEGASUS\_DISABLE\_PROV\_USERCTXT flag defined. In this case, the Provider Registration Manager rejects provider registration requests that specify a UserContext property value. The user context in which providers run is then unchanged by this enhancement. Some platforms, such as OS/400 and z/OS may define the PEGASUS\_DISABLE\_PROV\_USERCTXT as part of the platform configuration, since these platforms already set the provider user context on a per-thread basis.  When the Provider User Context feature is enabled, support for each of the User Context types may be disabled individually. Provider registration fails when an unsupported UserContext value is specified. Please refer to SecurityGuidelinesForDevelopers.html / PEP 223 for a discussion of the rationale/advantages of user context providers to many platforms.  

##### PEGASUS\_EXTRA\_PROVIDER\_LIB\_DIR

**Description:** Specifies the location of the extra provider libraries which are part of the release builds.  
**Default Value:** Not Set  
**Recommended Value (Development Build):**  No Specific Recommendation  
**Recommended Value (Release Build):**  No Specific Recommendation  
**Required:**  No.  
**Considerations:**  Since providerDir config property is fixed and hidden in release builds, this build time option provides facilty to include the extra provider directories without any security problems. This build time option is useful only when PEGASUS\_OVERRIDE\_DEFAULT\_RELEASE\_DIRS is set.

##### PEGASUS\_EMANATE\_INCLUDE\_DIR

**Description:** Specifies the location of the EMANATE _include_ directory.  
**Default Value:** Not Set  
**Recommended Value (Development Build):**  No Specific Recommendation  
**Recommended Value (Release Build):**  No Specific Recommendation  
**Required:** No. By default, the EMANATE include files are expected to be installed in directories that are searched by default (e.g., /usr/include). For platforms where this isn't true, the PEGASUS\_EMANATE\_INCLUDE\_DIR environment variable may be used to define an alternative path. This environment variable is only used if PEGASUS\_USE\_EMANATE is set.  
**Considerations:** Refer to the description of PEGASUS\_USE\_EMANATE for additional details.  

##### PEGASUS\_EMANATE\_LIB\_DIR

**Description:** Specifies the location of the EMANATE _lib_ directory.  
**Default Value:** Not Set  
**Recommended Value (Development Build):**  No Specific Recommendation  
**Recommended Value (Release Build):**  No Specific Recommendation  
**Required:** No. By default, the EMANATE libraries are expected to be installed in directories that are searched by default (e.g., /usr/lib, /usr/lib64). For platforms where this isn't true, the PEGASUS\_EMANATE\_LIB\_DIR environment variable may be used to define an alternative path. This environment variable is only used if PEGASUS\_USE\_EMANATE is set.  
**Considerations:** Refer to the description of PEGASUS\_USE\_EMANATE for additional details.  

##### PEGASUS\_ENABLE\_AUDIT\_LOGGER

**Description:** When this variable is set to false, support for audit logging will NOT be built into OpenPegasus. If this variable is set to true, or not set, support for audit logging will be included in the build. All other values are invalid.  
**Default Value:** true  
**Recommended Value (Development Build):** true  
**Recommended Value (Release Build):** true  
**Required:** No  

##### PEGASUS\_ENABLE\_CMPI\_PROVIDER\_MANAGER

**Description:** If set to true, a version of OpenPegasus that supports CMPI providers and their dependent components is built.  If this variable is not set or set to false, support for CMPI providers will not be included.  
**Default Value:** true for Linux and OpenVMS, false for other platforms  
**Recommended Value (Development Build):** true (Linux Platform Only)  
**Recommended Value (Release Build):** true (Linux Platform Only)  
**Required:** No

##### PEGASUS\_ENABLE\_COMPRESSED\_REPOSITORY

**Description:** If set the Repository Compression logic is built and enabled and compressed and non compressed repositories are supported. If not set then compressed repositories are not supported.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No

**Considerations:** See pegasus/readme.compressed\_repository

##### PEGASUS\_ENABLE\_CQL

**Description:** When this variable is set to false, support for Indication Subscription filters that have CQL as the language is disabled. It does not remove CQL from the build.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** false  
**Required:** No  
**Considerations:** The DMTF CIM Query Language Specification is Preliminary and subject to backward incompatible changes.

##### PEGASUS\_ENABLE\_DMTF\_INDICATION\_PROFILE\_SUPPORT

**Description:** When this variable is set to true, support the DMTF Indications Profile (DSP1054) is included in OpenPegasus.  If this variable is not set or set to false, support for the DMTF Indications Profile will not be included.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** The implementation of this feature is based on a preliminary version of the DMTF Indications Profile, DSP1053, and subject to change.  If this variable is enabled, two DMTF experimental class definitions, CIM\_IndicationService and CIM\_IndicationServiceCapabilities, will be added to the root/PG\_InterOp namespace. Experimental class definitions are subject to change.  PEGASUS\_ENABLE\_DMTF\_INDICATION\_PROFILE\_SUPPORT requires PEGASUS\_ENABLE\_INTEROP\_PROVIDER to be enabled. If PEGASUS\_ENABLE\_INTEROP\_PROVIDER is not set, setting PEGASUS\_ENABLE\_DMTF\_INDICATION\_PROFILE\_SUPPORT to true, will also enable PEGASUS\_ENABLE\_INTEROP\_PROVIDER. If PEGASUS\_ENABLE\_INTEROP\_PROVIDER is explicitly disabled (i.e., set to false), a build error will be generated.

##### PEGASUS\_ENABLE\_EXECQUERY

**Description:** When this environment variable is set, processing of ExecQuery operations is enabled. When not set, ExecQuery operation requests get a NotSupported response. (Note: The PEGASUS\_ENABLE\_EXECQUERY environment variable controls the definition of the PEGASUS\_DISABLE\_EXECQUERY compile macro.)  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No

##### PEGASUS\_ENABLE\_EMAIL\_HANDLER

**Description:**  When this environment variable is set, an E-Mail Indication Handler is built as part of the OpenPegasus build.  The E-Mail Indication Handler can be used to delivered CIM Indications to a designated e-mail address.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** If PEGASUS\_ENABLE\_EMAIL\_HANDLER is not set when OpenPegasus is built, a request to create an Email Indication Handler instance will be rejected will the error CIM\_ERR\_NOT\_SUPPORTED.

##### PEGASUS\_ENABLE\_GCOV

**Description:** GCOV, [http://gcc.gnu.org/onlinedocs/gcc/index.html#toc\_Gcov](http://gcc.gnu.org/onlinedocs/gcc/index.html#toc_Gcov), is a code coverage utility.  If PEGASUS\_ENABLE\_GCOV is set,  GCC will instrument the code to support analysis by GCOV.  
**Default Value:** Not Set  
**Recommended Value (Development Build):**  Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:**  This flag is currently only supported with GCC on Linux.

Note: LCOV, [http://ltp.sourceforge.net/coverage/lcov.php](http://ltp.sourceforge.net/coverage/lcov.php), is a graphical front-end extension for GCOV. It collects GCOV data for multiple source files and creates HTML pages with coverage information. You can use the following steps to use LCOV to analyze OpenPegasus GCOV data.

1.  Set PEGASUS\_ENABLE\_GCOV = true.
2.  Build and test OpenPegasus (e.g., _make -f pegasus/TestMakefile  build alltests_).
3.  For each source directory,
    *   copy the .gcda and .gcno files from the object directory to the source directory (e.g., _cp build/obj/Pegasus/Common/\*.gcda src/Pegasus/Common/_);
    *   execute the **lcov** command (e.g., _lcov --directory src/Pegasus/Common/ --capture --output-file result.info_) to build the result.info file; and finally,
    *   use the **genhtml** command to generate the .html output (e.g., _genhtml result.info_).  The top .html file is named index.html.

##### PEGASUS\_ENABLE\_INDICATION\_COUNT

**Description:** If true, the CIM Server will be built to count the number of CIM indications generated by providers and matched with subscriptions. If false or not set, indication statistic support will not be included. **Default Value:** true  
**Recommended Value (Development Build):** true  
**Recommended Value (Release Build):** true  
**Required:** No  
**Considerations:** The number of CIM indications generated by providers and matched with subscriptions is stored in hash tables which causes memory usage to increase by 4 bytes for each active subscription and by about 400 bytes for each active indication provider.  

##### PEGASUS\_ENABLE\_PROTOCOL\_BINARY

**Description:** If true, OpenPegasus uses a faster binary protocol between local clients and the CIM server. If false, OpenPegasus uses the conventional XML-based protocol instead. See PEP#340 for more details.  
**Default Value:** false  
**Recommended Value (Development Build):** false  
**Recommended Value (Release Build):** false  
**Required:** No  
**Considerations:** This option improves performance by using a faster binary protocol between local clients and the CIM server.  

##### PEGASUS\_ENABLE\_INTEROP\_PROVIDER

**Description:** If true, the OpenPegasus Interop Provider will be build. If false or not set, the OpenPegasus Interop Provider will not be built.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** false  
**Required:** No  
**Considerations:**  A number of features controlled by OpenPegasus build options are dependent on the Interop Provider.  Please refer to PEGASUS\_ENABLE\_SLP, PEGASUS\_DISABLE\_PERFINST, and PEGASUS\_ENABLE\_DMTF\_INDICATION\_PROFILE\_SUPPORT for additional details.  

##### PEGASUS\_INTEROP\_NAMESPACE

**Description:** This variable is used for configuring the Interop namespace name. This option helps to establish a consistent Interop Namespace as mentioned in DMTF specification.(DSP1033)  
**Default Value:** root/PG\_InterOp  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** The accepted values for this environment variable are "root/PG\_InterOp" or "interop" or "root/interop". For any other value, the default value will be used.With the usage of this build option, we have to start using pegasus with fresh repository.

##### PEGASUS\_ENABLE\_IPV6

**Description:** This variable can be used to specify whether IPv6 support should be included in OpenPegasus.  
**Default Value:** true  
**Recommended Value (Development Build):** true  
**Recommended Value (Release Build):** true  
**Required:** No  
**Considerations:** When this variable is set to 'false', support for IPv6 socket APIs and structures is disabled, and the CIM Server will not accept connections to or from IPv6 network interfaces. When this variable is set to 'true', the newer IPv6 socket APIs and structures will be included when OpenPegasus is built, so they must be supported on the build platform. However, if IPv6 is not supported on the platform on which an IPv6-enabled version of OpenPegasus is run, then the IPv6 feature will be disabled at run-time and only IPv4 connections will be supported.

##### PEGASUS\_ENABLE\_JMPI\_PROVIDER\_MANAGER

**Description:** If true, a version of OpenPegasus that supports JMPI providers and their dependent components is built. If false or not set, support for JMPI Providers is not included in OpenPegasus.  All other values are invalid.  
**Default Value:** false  
**Recommended Value (Development Build):** false  
**Recommended Value (Release Build):** false  
**Required:** No

##### PEGASUS\_ENABLE\_OBJECT\_NORMALIZATION

**Description:** If set, builds in support so that objects returned from provider instance operations can be validated. The enableNormalization must also be set to 'true' (default)  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No     
**Considerations:** If PEGASUS\_ENABLE\_OBJECT\_NORMALIZATION is set, two new configuration options are defined: enableNormalization and excludeModulesFromNormalization.

##### PEGASUS\_ENABLE\_PRIVILEGE\_SEPARATION

**Description:** If true, a version of OpenPegasus that enables the Privilege Separation feature is built. This feature allows the CIM Server to run in a non-privileged user context and use a small executor process to perform privileged operations. If false or not set, the Privilege Separate feature will not be included.  
**Default Value:** false  
**Recommended Value (Development Build):** true (HP-UX, Linux)  
**Recommended Value (Release Build):** true (HP-UX, Linux)  
**Required:** No  
**Considerations:**  The Privilege Separation feature is only supported on HP-UX and Linux platforms. The 'cimsrvr' user must be defined on the system. If the umask setting in the environment is not 0, PEGASUS\_TMP must be set to a directory that is writable by the 'cimsrvr' user for the tests to run successfully.

##### PEGASUS\_ENABLE\_PROTOCOL\_WSMAN

**Description:** If true, a version of OpenPegasus is built that accepts and processes WS-Management requests. This feature is initially limited to WS-Transfer and WS-Enumeration operations. If false or not set, WS-Management support is not built or included in any way.  
**Default Value:** false  
**Recommended Value (Development Build):** true  
**Recommended Value (Release Build):** false  
**Required:** No  
**Considerations:**  The WS-Management standards are not mature, and interoperability problems may arise. The WS-Management functionality may change in incompatible ways in subsequent releases. Also note the known limitations specified in PEP 311.  

##### PEGASUS\_ENABLE\_PROTOCOL\_WEB

**Description:** Enable pegasus to act as webserver capable of serving webpages by comsuming the JSON output from CIM-RS.  
**Default Value:** true  
**Recommended Value (Development Build):** true  
**Recommended Value (Release Build):** true  
**Required:** No  
**Considerations:**  When authentication is disabled, Anybody can modify the pegasus cimserver. Please enable authentication when using webadmin to prevent unauthorized access.  

##### PEGASUS\_ENABLE\_REMOTE\_CMPI

**Description:** If set, a version of OpenPegasus that support Remote CMPI providers and their dependent components is built.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No

##### PEGASUS\_ENABLE\_SESSION\_COOKIES

**Description:** If set to 'true', the OpenPegasus CIM server will use HTTP cookies for session management (RFC 6265).  
After a successful client authentication the client is given a cookie. The client is then not asked for re-authentication as long as it provides the same cookie in all subsequent requests and until the session expires. Session expiry is configurable using httpSessionTimeout configuration option.  
Nothing changes for clients that do not support HTTP cookies (RFC 6465) - their requests are authenticated as usual, either using Basic or Negotiate authentication mechanisms.  
Purpose of this option is to speed up request processing - both Basic and Negotiate authentication can take some time, which will form large fraction of system load on busy servers. **Default Value:** true (on supported platforms, see below)  
**Recommended Value (Development Build):** true  
**Recommended Value (Release Build):** true  
**Required:** No

**Considerations:**  This option can be set to true only when the underlying platform providers cryptographically strong random numbers (to generate strong session ID). Currently these platforms are zOS (using native API) and any platform with OpenSSL. In other words, PEGASUS\_ENABLE\_SESSION\_COOKIES can be enabled only when PEGASUS\_HAS\_SSL is set or on zOS.  
  
Note that while this option is enabled by default (i.e. the code is compiled), it is still turned off in default runtime configuration. System administrators must explicitly set httpSessionTimeout configuration option to nozero value to enable this feature.

##### PEGASUS\_ENABLE\_SLP

**Description:** This variable controls of the inclusion of SLP functionality in the OpenPegasus build. When this environment variable is set to 'true', SLP functionality will be included as part of the OpenPegasus build.  If this variable is set to 'false' or not set, the SLP functionality will not be included.  All other values are considered invalid and will result in a build error (e.g., "PEGASUS\_ENABLE\_SLP ($(PEGASUS\_ENABLE\_SLP)) invalid, must be true or false").  
**Default Value:** 'true' for Windows; 'false' for all other Platforms  
**Recommended Value (Development Build):** 'true' for Windows; 'false' for all other Platforms  
**Recommended Value (Release Build):**   
**Required:** No  
**Considerations:** PEGASUS\_ENABLE\_SLP requires PEGASUS\_ENABLE\_INTEROP\_PROVIDER to be enabled. If PEGASUS\_ENABLE\_INTEROP\_PROVIDER is not set, setting PEGASUS\_ENABLE\_SLP to true, will also enable PEGASUS\_ENABLE\_INTEROP\_PROVIDER. If PEGASUS\_ENABLE\_INTEROP\_PROVIDER is explicitly disabled (i.e., set to false), a build error will be generated.  

##### PEGASUS\_ENABLE\_SORTED\_DIFF

**Description:** This controls if the DIFFSORT function is used rather than a simple DIFF of the test results files to the static results file. Set to 'true' enables the sorted diffs of results to static results files. Otherwise results in regular diffs of results to static results files. See OpenPegasus bug 2283 for background information concerning this config variable.  
**Default Value:** true  
**Recommended Value (Development Build):** true  
**Recommended Value (Release Build):** true  
**Required:** No

##### PEGASUS\_ENABLE\_SYSTEM\_LOG\_HANDER

**Description:** When this environment variable is set, a SysLog Indication Handler is built as part of the OpenPegasus build.  The SysLog Indication Handler can be used to delivered CIM Indications to the system log file.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** If PEGASUS\_ENABLE\_SYSTEM\_LOG\_HANDLER is not set when OpenPegasus is built, a request to create a SysLog Indication Handler instance will be rejected with the error CIM\_ERR\_NOT\_SUPPORTED.  

##### PEGASUS\_ENABLE\_USERGROUP\_AUTHORIZATION

**Description:** Builds a version of OpenPegasus that allows an administrator to restrict access to CIM operations to members of a designated set of groups.  Refer to the **authorizedUserGroups**configuration option for additional details.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Set  
**Required:** No  

##### PEGASUS\_EXTRA\_C\_FLAGS

**Description:**  This environment variable allows a developer to specify an additional set of flags to be included on  the C compile command line.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** No Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No

##### PEGASUS\_EXTRA\_CXX\_FLAGS

**Description:**  This environment variable allows a developer to specify an additional set of flags to be included on the C++ compile command line.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No

##### PEGASUS\_EXTRA\_LIBRARY\_LINK\_FLAGS

**Description:**  This environment variable allows a developer to specify an additional set of flags to include on the link command line used to build a library.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No

##### PEGASUS\_EXTRA\_LINK\_FLAGS

**Description:**  This environment variable allows a developer to specify an additional set of flags to be included on the link command line.  These flags will be included on both library and program executable link command lines.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No **Considerations:** Refer to PEGASUS\_EXTRA\_LIBRARY\_LINK\_FLAGS and PEGASUS\_EXTRA\_PROGRAM\_FLAGS to add flags specific to either a library or program link command line.  

##### PEGASUS\_EXTRA\_PROGRAM\_LINK\_FLAGS

**Description:**  This environment variable allows a developer to specify an additional set of flags to include on the link command line used to build an executable program.  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No

##### PEGASUS\_HAS\_MESSAGES

**Description:** When set (to anything) during the build, OpenPegasus compiles with localization support. The ICU (International Components for Unicode) variables, ICU\_ROOT and ICU\_INSTALL, indicate that the localization support is based on ICU. Only ICU is supported at this time.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** If the PEGASUS\_HAS\_MESSAGES is not set, OpenPegasus is built without localization support. This means that all messages sent by the CIM Server and the CLIs are in English. ICU is an open source project at [http://oss.software.ibm.com/icu](http://oss.software.ibm.com/icu). Only English translations are included with the OpenPegasus distribution. The OpenPegasus distribution does not provide translated messages. But, enabling for ICU would allow vendor to provide the translations. Refer to the OpenPegasus Release README for additional information regarding the use of ICU. Although, in OpenPegasus 2.3.2, experience with localization support has been limited to a select set of platforms, wider platform adoption of this technology is planned for 2.5.  

##### PEGASUS\_HAS\_SSL

**Description:** If set, a version of OpenPegasus that supports SSL (i.e., https) is built.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Set  
**Recommended Value (Release Build):** Set  
**Required:** No  
**Considerations:** Support for SSL in OpenPegasus is dependent on the OpenSSL software developed by the [OpenSSL Project (http://www.openssl.org/)](http://www.openssl.org/). If the PEGASUS\_HAS\_SSL variable is set, the variable OPENSSL\_HOME must also be defined. The OPENSSL\_HOME variable is used, by the OpenPegasus build, to determine the location of the OpenSSL include files, libraries and binaries.

##### PEGASUS\_HOME

**Description:** Specifies the location of the OpenPegasus working directory. The OpenPegasus build will use this directory as the default location for files generated during the build (e.g., binaries, libraries, objects).  
**Default Value:** None  
**Recommended Value (Development Build):**  No Specific Recommendation  
**Recommended Value (Release Build):**  No Specific Recommendation  
**Required:** Yes  
**Considerations:** The error "PEGASUS\_HOME environment variable undefined" is returned if the PEGASUS\_HOME environment variable is not set. This variable is also used during runtime. Refer to the section titled "Environment Variable Settings for Running OpenPegasus 2.3.2 on Linux" for additional details.  

##### PEGASUS\_MAX\_THREADS\_PER\_SVC\_QUEUE

**Description:** Controls the maximum number of threads allowed per message service queue. It is allowed to range between 1 and MAX\_THREADS\_PER\_SVC\_QUEUE\_LIMIT (currently 5000) as set in pegasus/src/Pegasus/Common/MessageQueueService.cpp. If set to 0 (zero) the max threads per service queue is then set to MAX\_THREADS\_PER\_SVC\_QUEUE\_LIMIT. If set larger than the MAX\_THREADS\_PER\_SVC\_QUEUE\_LIMIT it is set equal to MAX\_THREADS\_PER\_SVC\_QUEUE\_LIMIT. There are no other limits on the total number of threads that can exist within the system at this time. When the server starts there on the order of 10 to 20 message service queues created dependent upon build options.   
**Default Value:** 5  
**Recommended Value (Development Build):** 5  
**Recommended Value (Release Build):** 5  
**Required:** No  
**Considerations:** This flag affects consumption of system resources. Not setting, or inappropriately setting this value, may cause the cimserver to hang or crash.  

##### PEGASUS\_ROOT

**Description:** Specifies the location of the directory that corresponds to "pegasus" source directory defined in the OpenPegasus CVS source tree. This environment variable is used by the OpenPegasus build to locate the required build and source files (e.g., $(PEGASUS\_ROOT)/mak and $(PEGASUS\_ROOT)/src).  
**Default Value:** None  
**Recommended Value (Development Build):**  No Specific Recommendation  
**Recommended Value (Release Build):**  No Specific Recommendation  
**Required:** Yes  
**Considerations:** The error "PEGASUS\_ROOT environment variable undefined" is returned if the PEGASUS\_ROOT environment variable is not set.

##### PEGASUS\_NET\_SNMP\_INCLUDE\_DIR

**Description:** Specifies the location of the NET-SNMP _include_ directory.  
**Default Value:** Not Set  
**Recommended Value (Development Build):**  No Specific Recommendation  
**Recommended Value (Release Build):**  No Specific Recommendation  
**Required:** No. By default, the NET-SNMP include files are expected to be installed in directories that are searched by default (e.g., /usr/include). For platforms where this isn't true, the PEGASUS\_NET\_SNMP\_INCLUDE\_DIR environment variable may be used to define an alternative path. This environment variable is only used if PEGASUS\_USE\_NET\_SNMP is set.  

##### PEGASUS\_NET\_SNMP\_LIB\_DIR

**Description:** Specifies the location of the NET-SNMP _lib_ directory.  
**Default Value:** Not Set  
**Recommended Value (Development Build):**  No Specific Recommendation  
**Recommended Value (Release Build):**  No Specific Recommendation  
**Required:** No. By default, the NET-SNMP library is expected to be installed in a directory that is searched by default (e.g., /usr/lib, /usr/lib64). For platforms where this isn't true, the PEGASUS\_NET\_SNMP\_LIB\_DIR environment variable may be used to define an alternative path. This environment variable is only used if PEGASUS\_USE\_NET\_SNMP is set.

##### PEGASUS\_NOASSERTS

**Description:** Defines the NDEBUG compilation macro, which causes the preprocessor to remove PEGASUS\_ASSERT() and disables assertion on PEGASUS\_FCT\_EXECUTE\_AND\_ASSERT statements.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Set  
**Required:** No  

##### PEGASUS\_OPENSLP\_HOME

**Description:** Specifies the location of the OpenSLP SDK directory. This directory must contain the OpenSLP _include_ directory, $(PEGASUS\_OPENSLP\_HOME)/include, and the OpenSLP _library_ directory, $(PEGASUS\_OPENSLP\_HOME)/lib.  
**Default Value:** Not Set  
**Recommended Value (Development Build):**  No Specific Recommendation  
**Recommended Value (Release Build):**  No Specific Recommendation  
**Required:** No. If not set, no additional include or library search paths will be included. **Considerations:** This value is not used if PEGASUS\_USE\_OPENSLP is not defined.  

##### PEGASUS\_PAM\_AUTHENTICATION

**Description:** Enables support for PAM-(Pluggable Authentication Modules) based authentication.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Set  
**Recommended Value (Release Build):** Set  
**Required:** No  
**Considerations:** Support for PAM in OpenPegasus is dependent on platform support for PAM.  

##### PEGASUS\_PAM\_SESSION\_SECURITY

**Description:** Enables support for PAM-(Pluggable Authentication Modules) based authentication and authorization.  
When choosing this PAM method the PAM session started through pam\_start is kept open for the complete CIM request. The PAM item PAM\_RHOST will be set to the connection IP address. This allows PAM modules to restrict number of concurrent connections from a single host.  
If PEGASUS\_PAM\_SESSION\_SECURITY is set, you cannot enable the Privilege Separation feature (PEGASUS\_ENABLE\_PRIVILEGE\_SEPARATION) or the authentication-only PAM mechanism (PEGASUS\_PAM\_AUTHENTICATION).  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** Support for PAM in OpenPegasus is dependent on platform support for PAM.  

##### PEGASUS\_NEGOTIATE\_AUTHENTICATION

**Description:** If set to true enables the HTTP Negotiate authentication method, as described by RFC 4559, both to Pegasus server and client. Enabling this option also adds 1) a new option to the runtime configuration variable "httpAuthType" and 2) a new runtime variable "mapToLocalName".  
**Default Value:** false  
**Recommended Value (Development Build):** false  
**Recommended Value (Release Build):** false  
**Required:** No  
**Considerations:** HTTP Negotiate support requires the GSSAPI development headers and libraries to build OpenPegasus, such as libgssapi\_krb5 from MIT Kerberos5 implementation.  

##### PEGASUS\_PLATFORM

**Description:** Describes the target platform for the build. The list of supported values for this variable is defined in pegasus/mak/config.mak.  Please refer to that file for a complete list of valid PEGASUS\_PLATFORM values.  This document includes platform-specific configuration data for only a subset of the supported platforms.  The following table describes the platforms documented in this version of the document.  
**Default Value:** None

Platform

Recommended Value  
Development Build

Recommended Value  
Release Build

Linux IA32

LINUX\_IX86\_GNU

LINUX\_IX86\_GNU

Linux IA64

LINUX\_IA64\_GNU

LINUX\_IA64\_GNU

Linux X86\_64

LINUX\_X86\_64\_GNU

LINUX\_X86\_64\_GNU

HP-UX PA-RISC

HPUX\_PARISC\_ACC

HPUX\_PARISC\_ACC

HP-UX IPF

HPUX\_IA64\_ACC

HPUX\_IA64\_ACC

OpenVMS Alpha

VMS\_ALPHA\_DECCXX

VMS\_ALPHA\_DECCXX

OpenVMS IA64

VMS\_IA64\_DECCXX

VMS\_IA64\_DECCXX

  
  
**Required:** Yes  
**Considerations:** The error "PEGASUS\_PLATFORM environment variable undefined." is returned if the PEGASUS\_PLATFORM environment variable is not set.

##### PEGASUS\_REMOVE\_METHODTRACE

**Description:** If true, the CIM Server is compiled without method enter and exit trace statements.  Trace Level 5 will continue to be accepted as a valid trace level but, without the method enter and exit trace statements, it will be equivalent to Trace Level 4. If PEGASUS\_REMOVE\_METHODTRACE is false or not set, method enter and exit trace statements are included.  All other values are considered invalid and will result in a build error.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set (The Considerations section gives guidance on when a different decision would be made.)  
**Recommended Value (Release Build):** Not Set (The Considerations section gives guidance on when a different decision would be made.)  
**Required:** No  
**Considerations:** This option completely removes the method enter/exit trace statements from the generated object code. It defines the PEG\_METHOD\_ENTER and PEG\_METHOD\_EXIT macros to null statements. This helps to keep static code size small but reduces the serviceability of the CIM server.  

##### PEGASUS\_REPOSITORY\_MODE

**Description:** This variable defines the default mode used to create repositories that are constructed as part of the automated build tests.  It does not affect the runtime environment. Valid values include: XML (causes the repository to be built in XML mode); BIN (causes the repository to be built in binary mode). Use cimconfig to modify the runtime environment.  
**Default Value:** XML  
**Recommended Value (Development Build):** XML  
**Recommended Value (Release Build):** XML  
**Required:** No

##### PEGASUS\_REPOSITORY\_STORE\_COMPLETE\_CLASSES

**Description:** This variable defines the default contents of a class definition in the repository store when a new repository is created. Specifically, when set to true, propagated elements are included in a class definition in the persistent store. The contents of a class definition returned from the repository interface is not affected by this option.  
**Default Value:** false  
**Recommended Value (Development Build):** false  
**Recommended Value (Release Build):** false  
**Required:** No

**Considerations:** A repository which stores complete class definitions does not allow modifications to non-leaf classes or changes to a superclass designation. Complete classes use more storage space, often twice as much as local (incomplete) classes. Class retrieval may be more expensive when local classes are stored. No specific mechanism is available to change this setting in an existing repository.  

##### PEGASUS\_SLP\_REG\_TIMEOUT

**Description:** Most SLP implementations allow a lifetime value to be specified in the registration sent to the SLP SA or DA. The PEGASUS\_SLP\_REG\_TIMEOUT build variable allows a lifetime (ie. timeout) value to be specified on the SLP registration, with a 're-registration' being sent to SLP at the same specified timeout interval. The purpose of this lifetime or timeout value is to avoid the accumulation of stale service information in the SA or DA.  
**Default Value:** Undefined  
**Recommended Value (Development Build):** Undefined  
**Recommended Value (Release Build):** Undefined  
**Required:** No

**Example:** PEGASUS\_SLP\_REG\_TIME=1440 (ie. 24 hours or 1 day)  
**Considerations:** (1) The OpenPegasus internal SLP implementation does not support timeout registrations. This build option is useful, for example, if PEGASUS\_USE\_OPENSLP is defined. (2) Some SLP implementations may impose a maximum value for the lifetime parameter. For example, OpenSLP limits this value to SLP\_LIFETIME\_MAXIMUM (or 0xFFFF), and if this maximum value is passed to OpenSLP, then the timeout feature is disabled. Refer to the documentation of your SLP implementation for more information. (3) OpenPegasus will not do any validity checking on the specified value, either to determine if it is a valid positive integer or if it exceeds some maximum value.  

##### PEGASUS\_SNIA\_EXTENSIONS

**Description:**  This variable is used to enable a set of workarounds that support the use of OpenPegasus in the SNIA Test Environment.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** The functionality enabled with this variable is experimental and subject to change.

##### PEGASUS\_SKIP\_MOST\_TEST\_DIRS

**Description:** If set to true, this variable can be used to reduce the time required to build OpenPegasus by significantly reducing the number of tests that are built.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** Setting this variable to true affects the behavior of all recursive make targets (e.g., clean, depend, all, unittests, alltests, and slptests). Use of this variable with test-related targets (e.g., unittests, alltests and slptests) can produce unexpected results. To avoid creating an inconsistent PEGASUS\_HOME directory, the PEGASUS\_HOME directory (i.e., the OpenPegasus build directory) should be removed prior to enabling or disabling this variable. This variable should not be used to characterize the set of files to be packaged in a release build. Please refer to the Runtime Packaging Definition PEP for the release for a recommended list of files to be included in a release.

##### PEGASUS\_TEST\_ENABLE\_DEBUG\_TRACE

**Description:**  This variable is only used for testing. If set to true, Discarded data trace will be enabled by the install\_run target of the src/Server/Makefile before running the OpenPegasus TestMakefile test suite.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:**  This variable is useful in certain test environments to explicitly enable tracing to aid in better troubleshooting and detection of test failures (E.g. Test systems are experiencing test failures due to connection timeouts, lost indications etc).  

##### PEGASUS\_TEST\_ISGROUP\_GROUP/PEGASUS\_TEST\_ISGROUP\_USER\_SUCCESS/PEGASUS\_TEST\_ISGROUP\_USER\_FAILURE

**Description:** These variables are only used for testing. These variables allow the developer to define a valid group name, PEGASUS\_TEST\_ISGROUP\_GROUP, a valid user name that is in the group, PEGASUS\_TEST\_ISGROUP\_USER\_SUCCESS, and an a valid user name that is not in the group, PEGASUS\_TEST\_ISGROUP\_USER\_FAILURE. These variables are only used for testing and NOT part of the release.  
**Default Value(Windows):** Administrator/Administrators/Guest  
**Default Value(Non-Windows):** root/root/guest  
**Recommended Value (Development Build):**  No Recommendation  
**Recommended Value (Release Build):** Not Used  
**Required:** No

##### PEGASUS\_TEST\_IPV6

**Description:** This variable is only used for testing.  If set to true, the IPv6-specific tests will be run as part of the "alltests" OpenPegasus TestMakefile test suite. If this variable is not set or set to false,  the IPv6-specific tests will not be run.  All other values are invalid.  
**Default Value:**  If PEGASUS\_TEST\_IPV6 is not defined, PEGASUS\_TEST\_IPV6 is set to the value of PEGASUS\_ENABLE\_IPV6.  
**Recommended Value (Development Build):** Set to the value of PEGASUS\_ENABLE\_IPV6  
**Recommended Value (Release Build):** Not Used  
**Required:** No  
**Considerations:** This variable is useful in certain test environments to explicitly disable running the IPv6 tests (e.g., when running an IPv6-enabled version of OpenPegasus on an an IPv4 system).  

##### PEGASUS\_TEST\_SDK

**Description:** This variable is only used for testing.  It is used to control testing of the SDK sample providers and clients. If set, the setup, building and testing of the SDK samples is enabled as part of the OpenPegasus automated build and test.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Set  
**Recommended Value (Release Build):** Not Used  
**Required:** No

##### PEGASUS\_TEST\_USER\_DEFINED

**Description:** This variable is only used for testing. If this variable is set, the tests will assume that the PEGASUS\_TEST\_USER\_ID and PEGASUS\_TEST\_USER\_PASS variables defines a valid user/password pair and can be used for certificate-based authentication testing.  These tests also assumes that "invalidUser" is not a valid user.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Set  
**Recommended Value (Release Build):** Not Used  
**Required:** No

##### PEGASUS\_TEST\_USER\_ID/PEGASUS\_TEST\_USER\_PASS

**Description:** These variables are only used for testing. These variables allow the developer to define a valid, non-privileged  user name and password on the test system that can be used as part of the OpenPegasus automated test scripts to perform authentication and authorization testing.  These variables are only used for testing and NOT part of the release.  
**Default Value:** guest/guest  
**Recommended Value (Development Build):**  No Recommendation  
**Recommended Value (Release Build):** Not Used  
**Required:** No  

##### PEGASUS\_TEST\_VALGRIND\_LOG\_DIR

**Description:**  This variable is currently only supported on Linux. It may be set to run regression tests under the [Valgrind](http://www.valgrind.org/) memory checking tool. It must specify a directory for writing Valgrind log files. The file name format for a particular log is PEGASUS\_TEST\_VALGRIND\_LOG\_DIR/PROGRAM\_NAME.pid.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** This flag is only supported on Linux to enable running tests under Valgrind.  

##### PEGASUS\_TMP

**Description:** This variable is only used in a test environment. It can be used to define the directory that OpenPegasus test programs should use to store temporary files.  
**Default Value:** "."  
**Recommended Value (Development Build):**  No Specific Recommendation  
**Recommended Value (Release Build):** Not Used  
**Required:** No  

##### PEGASUS\_USE\_DEBUG\_BUILD\_OPTIONS

**Description:** The PEGASUS\_DEBUG flag is used to control a variety of settings, including compile options, assertion enablement, method definitions for testing, and diagnostic output.  The PEGASUS\_USE\_DEBUG\_BUILD\_OPTIONS variable can be used to enable just the compiler specific debug options (e.g., -g in g++) .  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** This variable is automatically set if PEGASUS\_DEBUG is set.  

##### PEGASUS\_USE\_EMANATE

**Description:** EMANATE, [http://www.snmp.com/products/emanate.html](http://www.snmp.com/products/emanate.html), is an SNMP Research International product that supports SNMPv1, SNMPv2c, and SNMPv3.  If the PEGASUS\_USE\_EMANATE variable is defined, the OpenPegasus SNMP Handler will use the EMANATE libraries to send traps to the SNMP Management application.  The EMANATE libraries are NOT included as part of OpenPegasus.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No

**Considerations:** Either the PEGASUS\_USE\_NET\_SNMP or PEGASUS\_USE\_EMANATE variable must be set for the SNMP Handler to be supported.  

##### PEGASUS\_USE\_NET\_SNMP

**Description:** NET-SNMP, [http://net-snmp.sourceforge.net](http://net-snmp.sourceforge.net),  is an OSS implementation of a suite of applications used to implement [SNMP v1](http://www.ietf.org/rfc/rfc1157.txt), [SNMP v2c](http://www.ietf.org/rfc/rfc1901.txt) and [SNMP v3](http://www.ietf.org/rfc/rfc2571.txt) using both IPv4 and IPv6.   If the PEGASUS\_USE\_NET\_SNMP variable is defined, the OpenPegasus SNMP Handler will use the NET-SNMP libraries to send traps to the SNMP Management application.  The NET-SNMP libraries are NOT included as part of OpenPegasus.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** true for Linux, false for all other platforms  
**Required:** No

**Considerations:** Either the PEGASUS\_USE\_NET\_SNMP or PEGASUS\_USE\_EMANATE variable must be set for the SNMP Handler to be supported.  

##### PEGASUS\_ENABLE\_NET\_SNMPV3

**Description:** If this variable is defined, then OpenPegasus SNMP Handler will build the SNMPV3 features. This should be set to false if NET-SNMP 5.2 and lower libraries are used.  
**Default Value:** true  
**Recommended Value (Development Build):** true  
**Recommended Value (Release Build):** true  
**Required:** No

**Considerations:** PEGASUS\_USE\_NET\_SNMP variable must be set.  

##### PEGASUS\_USE\_OPENSLP

**Description:** OpenSLP, [http://openslp.org](http://www.openslp.org), is an open-source implementation of Service Location Protocol. If the PEGASUS\_USE\_OPENSLP variable is defined, OpenPegasus will be built to use the OpenSLP implementation of SLP instead of the internal OpenPegasus implementation of SLP. The OpenSLP libaries are NOT included as part of OpenPegasus. **Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** PEGASUS\_ENABLE\_SLP must be defined if this variable is set.

##### PEGASUS\_USE\_PAM\_STANDALONE\_PROC

**Description:** Moves the processing of PAM requests from the CIM Server process to a separate process managed by the CIM Server.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Set  
**Recommended Value (Release Build):** Set  
**Required:** No  
**Considerations:** If PAM Authentication is enabled, the PAM API is used, during the processing of each request, to authenticate the client. This level of use makes the CIM Server extremely sensitive to memory leaks in the PAM library. In addition, certain PAM modules are not thread-safe. If your platform is experiencing problems with PAM Authentication, use of option may provide a work-around. PEGASUS\_USE\_PAM\_STANDALONE\_PROC requires PEGASUS\_PAM\_AUTHENTICATION to be set.  

##### PEGASUS\_USE\_RELEASE\_CONFIG\_OPTIONS

**Description:** If set, OpenPegasus is built using the "Release Build" configuration options. By default, OpenPegasus is built using the "Development Build" configuration options.  
**Default Value:** false  
**Recommended Value (Development Build):** false  
**Recommended Value (Release Build):** true  
**Required:** No  
**Considerations:** (1)The PEGASUS\_USE\_RELEASE\_CONFIG\_OPTIONS variable allows a platform vendor to easily toggle between two sets of configurations options, a set of options tuned for the development/debug ("Development Build") environment and a set of options tuned for the production environment ("Release Build"). Refer the section titled "Configuration Properties" for additional detail.  (2) For each configuration variable,  the "Recommended Value (Release Build)" value defines the recommended settings to use for a "Release Build".  Please refer to the description of each variable for additional information.

##### PEGASUS\_USE\_RELEASE\_DIRS

**Description:** If set, OpenPegasus is built using the "Release Build" directory definitions. By default, OpenPegasus is built using the "Development Build" directory definitions.  
**Default Value:** Not Set  
**Recommended Value (Development Build):** Not Set  
**Recommended Value (Release Build):** Set  
**Required:** No  
**Considerations:**  The PEGASUS\_USE\_RELEASE\_DIRS variable allows a platform vendor to easily toggle between two sets of directory definitions, a set tuned for the development/debug ("Development Build") environment and a set tuned for the production environment ("Release Build"). Refer the section titled "Configuration Properties" for additional detail.  

##### PEGASUS\_FLAVOR

**Description:**  This variable is used for configuring Multi CIMOM on same OS by eliminating single point for storage of PID's and configuration files. Flavor is used in structuring the release dirs,configuration file names, log identity and PAM config file. If set, OpenPegasus is built using the flavor value. By default OpenPegasus is built using "tog" as the flavor. RPMs are named using the flavor value.  
**Default Value:** tog  
**Recommended Value (Development Build):** tog  
**Recommended Value (Release Build):** tog  
**Required:** No  
**Considerations:**  To build RPM using flavor, RPM spec file needs to be regenerated after setting the flavor value. This can be done using the create\_OpenPegasusRPMSpecFile target in the Makefile.Release. Platforms have to define their own release dir structure using the flavor for release builds. An error will be reported if both PEGASUS\_FLAVOR and PEGASUS\_USE\_RELEASE\_DIRS are set and PEGASUS\_OVERRIDE\_DEFAULT\_RELEASE\_DIRS is not set. RPM build on linux defines PEGASUS\_OVERRIDE\_DEFAULT\_RELEASE\_DIRS by default.  

##### PEGASUS\_PLATFORM\_FOR\_32BIT\_PROVIDER\_SUPPORT

**Description:**  Builds the Pegasus with 32-bit provider support for the specified platform. If this option is set, the value MUST be one of the 32-bit supported platforms by Pegasus.  
**Default Value:** Not set  
**Recommended Value (Development Build):** Not set  
**Recommended Value (Release Build):** Not set  
**Required:** No  
**Considerations:**  This option is used to run the 32-bit providers under 64-bit CIMserver. To register the 32-bit providers, register the provider module by setting the Bitness property value to "2". 32-bit Providers are always loaded out of process irrespective of forceProviderProcesses config option.  

##### PEGASUS\_USE\_SQLITE\_REPOSITORY

**Description:** If true, new repository stores are created using a SQLite database. Existing file-based stores will continue to be recognized and operate seamlessly.  
**Default Value:** false  
**Recommended Value (Development Build):** true  
**Recommended Value (Release Build):** true  
**Required:** No  
**Considerations:** If PEGASUS\_USE\_SQLITE\_REPOSITORY is set to true and SQLite files are not installed in default search locations, SQLITE\_HOME must also be configured. PEGASUS\_REPOSITORY\_STORE\_COMPLETE\_CLASSES may not be set to true when PEGASUS\_USE\_SQLITE\_REPOSITORY is set to true.

##### PEGASUS\_USE\_SYSLOGS

**Description:** If set, OpenPegasus will be built to send log messages to the system logger (syslog). Otherwise, log messages will be sent to OpenPegasus specific log files.  
**Recommended Value (Development Build):** Set  
**Recommended Value (Release Build):** Set  
**Required:** No  
**Considerations:** This variable is currently not implemented as an environment variable. To use the option -DPEGASUS\_USE\_SYSLOGS must be explicitly included in the appropriate platform makefile (pegasus/mak/platform\_\*.mak).

##### PEGASUS\_WINDOWS\_SDK\_HOME

**Description:** This variable should be set to point to the Microsoft Platform SDK on Windows if using a compiler version < 1300 (i.e. VC6). If using VC7 or VC8, this flag does not need to be set, as the necessary libraries are already included.  
**Recommended Value (Development Build):**  No Specific Recommendation  
**Recommended Value (Release Build):**  No Specific Recommendation  
**Required:** Yes, if building on Windows with a compiler version < 1300.  

##### SQLITE\_HOME

**Description:** Specifies the location of the SQLite SDK directory. This directory must contain sqlite3.h in the $(SQLITE\_HOME)/include directory, and libsqlite3 in the $(SQLITE\_HOME)/lib directory.  
**Default Value:** Not Set  
**Recommended Value (Development Build):**  No Specific Recommendation  
**Recommended Value (Release Build):**  No Specific Recommendation  
**Required:** Yes, if PEGASUS\_USE\_SQLITE\_REPOSITORY is set to true and the SQLite files are not installed in locations that are searched by default.  
**Considerations:** This environment variable is only used if PEGASUS\_USE\_SQLITE\_REPOSITORY is set to true.

##### PEGASUS\_INITIAL\_THREADSTACK\_SIZE

**Description:** This environment variable sets the initial size of the stack on new threads. Value is specified in number of bytes.  
**Default Value:** 262144  
**Recommended Value (Development Build):** 262144  
**Recommended Value (Release Build):** 262144  
**Required:** No

#### Examples Building a Linux IA32 (#IA64) Development/Debug Version

export PEGASUS\_ROOT=/home/pegasusbld/pegasus  
export PEGASUS\_HOME=/home/pegasusbld/pegasus/build  
export PEGASUS\_PLATFORM=LINUX\_IX86\_GNU  
#export PEGASUS\_PLATFORM=LINUX\_IA64\_GNU  
export PEGASUS\_PAM\_AUTHENTICATION=true  
export PEGASUS\_USE\_PAM\_STANDALONE\_PROC=true  
export PEGASUS\_HAS\_SSL=yes  
export PEGASUS\_DEBUG=TRUE  
export ENABLE\_CMPI\_PROVIDER\_MANAGER=true  
export PEGASUS\_USE\_SYSLOGS=true  
export PATH=/home/pegasusbld:/home/pegasusbld/pegasus/build/bin:/usr/local/bin:$PATH  

#### Building a Linux IA32 (#IA64) Production Release Version

export PEGASUS\_ROOT=/home/pegasusbld/pegasus  
export PEGASUS\_HOME=/home/pegasusbld/pegasus/build  
export PEGASUS\_PLATFORM=LINUX\_IX86\_GNU  
#export PEGASUS\_PLATFORM=LINUX\_IA64\_GNU  
export PEGASUS\_ENABLE\_CQL=false  
export PEGASUS\_PAM\_AUTHENTICATION=true  
export PEGASUS\_USE\_PAM\_STANDALONE\_PROC=true  
export PEGASUS\_HAS\_SSL=yes  
export PEGASUS\_USE\_RELEASE\_CONFIG\_OPTIONS=true  
export PEGASUS\_USE\_RELEASE\_DIRS=true  
export PEGASUS\_NOASSERTS=true  
export ENABLE\_CMPI\_PROVIDER\_MANAGER=true  
export PEGASUS\_ENABLE\_USERGROUP\_AUTHORIZATION=true  
export PEGASUS\_USE\_SYSLOGS=true  
export PATH=/home/pegasusbld:/home/pegasusbld/pegasus/build/bin:/usr/local/bin:$PATH  

### Environment Variable Settings for Running OpenPegasus

#### Definitions

##### PEGASUS\_HOME

**Description:** There are multiple options for configuring the location of OpenPegasus runtime files (e.g., configuration files, libraries, repository, executables). If no other option is specified, OpenPegasus will attempt to use the value PEGASUS\_HOME.  
**Default Value:** "."  
**Recommended Value (Development Build):**  The same value defined at build time.  
**Recommended Value (Release Build):** Not Set  
**Required:** No  
**Considerations:** Although this variable can be useful in a development environment use of this environment variable is not recommended in a production environment. Instead, the use of configuration properties to explicitly set the location of runtime files and directories is recommended.

##### PEGASUS\_MSG\_HOME

**Description:** This value is not used by the CIM Server.  The CIM Server uses the messageDir configuration parameter to locate the directory where the ICU resource bundles. However, CIM Client applications must rely on an alternative mechanism to locate this directory.  
**Default Value:** if $PEGASUS\_HOME is set then "$PEGASUS\_HOME/msg" else "."  
**Recommended Value (Development Build):** $PEGASUS\_HOME/msg  
**Recommended Value (Release Build):** Not Set  
**Required:** No  

### Configuration Properties

#### Notation

This section describes the notation used to define the configuration properties.

##### Default Value

The value of _Default Value_ is the OpenPegasus default setting for this configuration option. This value is used if a platform-specific setting is not defined.

##### Recommended Default Value

The PEGASUS\_USE\_RELEASE\_CONFIG\_OPTIONS variable allows a platform vendor to easily toggle between two sets of configurations options, a set of options tuned for the development/debug ("Development Build") environment and a set of options tuned for the production environment ("Release Build"). The value of _Recommended Value (Development Build)_ is the recommended default value for use in a development/ debug environment. The value of _Recommended Value (Release Build)_ is the recommended default value for use in a production environment.

##### Recommend To Be Fixed/Hidden

Configuration options can be defined as fixed or hidden. A "fix" configuration option is set at build time and cannot be changed without rebuilding. "Fixed" configuration options are not displayed using the cimconfig command. This feature can be used by vendors to limit OpenPegasus functionality and configurability. A "hidden" configuration option is an option that is configurable (i.e., "not fixed"), but not externally advertised using the cimconfig command. The "hidden" feature can be used to define "internal use only" configuration options.

##### Dynamic?

If yes, the value of the configuration option can be changed without stopping and restarting the CIM Server.

##### Directory Specifications

Note that the variables included in the following table (e.g., $LOGDIR) are for documentation purposes only and have been defined to simplify the description of the configuration options.  In particular, these variables DO NOT correspond to environment variables implemented in the OpenPegasus code.  Default values for the following variables are platform and vendor dependent and not documented in this document.

 

$LOGDIR

$PROVIDERDIRS

$PROVIDERMANAGERDIRS

$REPOSITORYDIR

$CERTIFICATEDIR 

$LOCALAUTHDIR

$TRACEDIR

$CONFIGDIR

$PIDFILE

$RANDOMDIR

$SOCKETDIR

$MESSAGEDIR

$WEBCONFDIR

#### Definitions

##### authorizedUserGroups

**Description:** If the **authorizedUserGroups** property is set, the value is interpreted as a list of comma-separated user groups whose members may issue CIM requests. A user who is not a member of any of these groups is restricted from issuing CIM requests, with the exception of privileged users (root user). If the **authorizedUserGroups** property is not set, any user may issue CIM requests.  
**Default Value:** blank  
**Recommended Default Value (Development Build):** blank  
**Recommended Default Value (Release Build):** blank  
**Recommend To Be Fixed/Hidden (Development Build):**  No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Example:** #cimconfig -s authorizedUserGroups=users,systemusers -p  
**Considerations:** This feature is available only when the OpenPegasus source is compiled with the flag PEGASUS\_ENABLE\_USERGROUP\_AUTHORIZATION set.

##### daemon

**Description:** This option enables/disables forking of the code to create a background daemon process.  
**Default Value:** true  
**Recommended Default Value (Development Build):** true  
**Recommended Default Value (Release Build):** true  
**Recommend To Be Fixed/Hidden (Development Build):**  No/No  
**Recommend To Be Fixed/Hidden (Release Build):**  No/Yes  
**Dynamic?:** No

   
  

Platform

Source Configuration File

Linux

Pegasus/Config/DefaultPropertyTableLinux.h

HP-UX

Pegasus/Config/DefaultPropertyTableHpux.h

OpenVMS

Pegasus/Config/DefaultPropertyTableVms.h

##### enableAssociationTraversal

**Description:** If true, the CIM Server will support the four association traversal operators: Associators, AssociatorNames,References, and ReferenceNames.  
**Default Value:** true  
**Recommended Default Value (Development Build):** true  
**Recommended Default Value (Release Build):** true  
**Recommend To Be Fixed/Hidden (Development Build):**  No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:** There is still an outstanding Provider Registration issue that restricts the degree to which separate Providers can register for Associations.  
  

Platform

Source Configuration File

Linux

Pegasus/Config/DefaultPropertyTableLinux.h

HP-UX

Pegasus/Config/DefaultPropertyTableHpux.h

OpenVMS

Pegasus/Config/DefaultPropertyTableVms.h

##### enableAuditLog

**Description:** Used to enable or disable audit logging at run time. If true, audit logging will be enabled.  
**Recommended Default Value (Development Build):** false  
**Recommended Default Value (Release Build):** false  
**Recommend To Be Fixed/Hidden (Development Build):**  No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** Yes  
**Source Configuration File:**  Pegasus/Config/DefaultPropertyTable.h  

##### enableAuthentication

**Description:** If true, a Client must be authenticated to access the CIM Server.  
**Recommended Default Value (Development Build):** false  
**Recommended Default Value (Release Build):** true  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** No  
**Source Configuration File:**  Pegasus/Config/SecurityPropertyOwner.cpp  

##### enableBinaryRepository

**Description:** Setting this to true will activate the binary repository support on the next restart of cimserver.  
**Recommended Default Value (Development Build):** false  
**Recommended Default Value (Release Build):** false  
**Recommend To Be Fixed/Hidden (Development Build):** Yes  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** No  
**Considerations:** Repositories containing a mixture of binary and XML objects are supported.  
**Source Configuration File:**  Pegasus/Config/RepositoryPropertyOwner.cpp

##### enableHttpConnection

**Description:** If true, allows connections to the CIM Server using the HTTP protocol  
**Default Value:** true  
**Recommended Default Value (Development Build):** true  
**Recommended Default Value (Release Build):** false  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:**  Enabling HTTP will allow clients to connect with security properties different than those associated with encrypted SSL traffic, and its configured SSL authentication or other security behavior. This option should only be enabled in environments where sending the HTTP Request and HTTP Response as clear text messages does not introduce a security risk.  Note that if authentication is enabled (enableAuthentication), user names and passwords will be included in the text of the HTTP message.  
  

Platform

Source Configuration File

Linux

Pegasus/Config/DefaultPropertyTableLinux.h

HP-UX

Pegasus/Config/DefaultPropertyTableHpux.h

OpenVMS

Pegasus/Config/DefaultPropertyTableVms.h

##### enableHttpsConnection

**Description:** If true, allows connections to the CIM Server using the HTTPS protocol (HTTP using Secure Socket Layer encryption)  
**Default Value:** false  
**Recommended Default Value (Development Build):** true  
**Recommended Default Value (Release Build):** true  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:** For this option to work, the environment variable PEGASUS\_HAS\_SSL must have been set when CIM Server was built.  
  

Platform

Source Configuration File

Linux

Pegasus/Config/DefaultPropertyTableLinux.h

HP-UX

Pegasus/Config/DefaultPropertyTableHpux.h

OpenVMS

Pegasus/Config/DefaultPropertyTableVms.h

##### enableIndicationService

**Description:** If true, the CIM Server will support CIM Indications.  
**Default Value:** true  
**Recommended Default Value (Development Build):** true  
**Recommended Default Value (Release Build):** true  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** Yes/No  
**Considerations:** When enableIndicationService is set to false dynamically, delete subscription requests are sent to all indication providers and indication related operations are not supported until indication service is re-enabled. Dynamic configuration facility is available only when Pegasus is compiled with PEGASUS\_ENABLE\_DMTF\_INDICATION\_PROFILE\_SUPPORT=true. Clients who wants to utilize the IndicationService services MUST check for CIM\_IndicationService EnabledState and HealthState properties to know the current indication service state. For example, User can enumerate instances of CIM\_IndicationService and there is exactly one instance of CIM\_IndicationService returned. (Ex. using cimcli command 'cimcli ei -n root/PG\_InterOp CIM\_IndicationService'). When requested state change(Enabled/Disabled) cannot be made because of timeout or some other exception IndicationService HealthState is set to "Degraded/Warning". For example, if user requested for indication service disable (by setting enableIndicationService=false), disable failed because of timeout or some other exception. Now service health state is set to "Degraded/Warning" and service EnabledState will remain enabled. User can now identify the faulty provider causing the timeout or any other problem and can take appropriate action. User can again request for service disable to recover from degraded state. Similar sequence applies while enabling the indication service. Only difference is that service EnabledState is set to Enabled if exception or timeout happens. There is no automatic recovery from degraded state is available at present.  
  

Platform

Source Configuration File

Linux

Pegasus/Config/DefaultPropertyTableLinux.h

HP-UX

Pegasus/Config/DefaultPropertyTableHpux.h

OpenVMS

Pegasus/Config/DefaultPropertyTableVms.h

##### enableNamespaceAuthorization

**Description:** If true, the CIM Server restricts access to namespaces based on configured user authorizations \[user authorizations may be configured using the cimauth command\]  
**Default Value:** false  
**Recommended Default Value (Development Build):** false  
**Recommended Default Value (Release Build):** false  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:**  This option offers limited functionality and, in most environments, expensive to administer. It's use is not recommended.  
**Source Configuration File:**  Pegasus/Config/SecurityPropertyOwner.cpp

##### enableNormalization

**Description:** If true, objects returned from instance providers are validated and normalized.  
**Default Value:** true  
**Recommended Default Value (Development Build):** true  
**Recommended Default Value (Release Build):** false  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** Yes  
**Considerations:** 

*   Any errors encountered during normalization result in a CIMException with the status code set to CIM\_ERR\_FAILED and a meaningful, context specific error description (messages will be localized). Providers that catch the exception have an opportunity to handle the error. Otherwise, the exception is passed to the Provider Manager and becomes the response for that Provider.
*   Provider Modules that are listed on the excludeModulesFromNormalization configuration option are excluded from normalization.
*   Since the concept of normalization is relatively new to OpenPegasus and it establishes and enforces rules for objects returned by providers, it should only apply to providers written against the SDK in which it was introduced (or later). Therefore only Provider Modules that have the following InterfaceType and InterfaceVersion combination will be subject to normalization:
    *   'C++Default' / 2.5.0 or greater
    *   'CMPI' / 2.0.0 or greater
    *   'JMPI' / 1.0.0 or greater

**Source Configuration File:**  Pegasus/Config/NormalizationPropertyOwner.cpp  

##### enableRemotePrivilegedUserAccess

**Description:** If true, the CIM Server allows access by a privileged user from a remote system  
**Default Value:** true  
**Recommended Default Value (Development Build):** true  
**Recommended Default Value (Release Build):** true  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:** Many management operations require privileged user access. Disabling remote access by privileged user could significantly affect functionality.  
**Source Configuration File:**  Pegasus/Config/SecurityPropertyOwner.cpp

##### enableSubscriptionsForNonprivilegedUsers

**Description:** If true, operations (create instance, modify instance, delete instance, get instance, enumerate instances, enumerate instance names) on indication filter, listener destination, and subscription instances may be performed by non-privileged users. Otherwise, these operations may only be performed on these instances by privileged users.  
**Default Value:** true  
**Recommended Default Value (Development Build):** true  
**Recommended Default Value (Release Build):** false  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:** This option has meaning only if enableIndicationService=true.  
**Source Configuration File:**  Pegasus/Config/SecurityPropertyOwner.cpp

##### excludeModulesFromNormalization

**Description:** If the excludeModulesfromNormalization property is set, the value is interpreted as a list of comma-separated Provider Module names (as specified in PG\_ProviderModule.Name) to exclude from validation and normalization.  
**Default Value:** ""  
**Recommended Default Value (Development Build):** ""  
**Recommended Default Value (Release Build):**  ""  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:** All objects managed by all providers will be excluded from validation and normalization if its module name appears in this list.  
**Source Configuration File:**  Pegasus/Config/NormalizationPropertyOwner.cpp

##### forceProviderProcesses

**Description:** If true, the CIM Server runs Providers in separate processes rather than loading and calling Provider libraries directly within the CIM Server process.  
**Default Value:** Dependent on the value of PEGASUS\_DEFAULT\_ENABLE\_OOP  
**Recommended Default Value (Development Build):**true (HP-UX, Linux)  
**Recommended Default Value (Release Build):** true (HP-UX, Linux)  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:** In a dynamic, potentially multi-vendor, environment, the ability to run Providers in separate processes can significantly increase the stability, maintainability and supportability of the overall solution.  Running Providers in separate process spaces: (1) helps isolate components (i.e., cimserver and Providers) from a catastrophic failure (segmentation faults, memory leaks) of a single Provider and (2) reduces the risk of accidental or malicious sharing of sensitive data among Providers.  This benefit comes at a cost.  Moving Providers out of the cimserver process increases communication costs and can significantly increase response time.  Actual results will vary, but it is common for response times to increase by a factor of 2.  
**Note 1:** When forceProviderProcesses is true, the CIM Server will load each Provider Module (i.e., library) into a separate process. Providers in different Provider Modules cannot depend on a communication mechanism that requires them to run in the same process space.  
**Note 2:** Although the catastrophic failure of an Out-Of-Process (OOP) Provider Module will not result in a failure of the CIM Server process, if the failed OOP Provider Module included Indication Providers, Indications may be lost.  If logging is enabled, the CIM Server will log a message informing administrators of what happened and what corrective actions can be taken.  
  

Platform

Source Configuration File

Linux

Pegasus/Config/DefaultPropertyTableLinux.h

HP-UX

Pegasus/Config/DefaultPropertyTableHpux.h

OpenVMS

Pegasus/Config/DefaultPropertyTableVms.h

##### home

**Description:** If set, this configuration option defines the runtime default value for PEGASUS\_HOME.  
**Default Value:** "./"  
**Recommended Default Value (Development Build)**"./"  
**Recommended Default Value (Release Build)""**  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** No

   
  

Platform

Source Configuration File

Linux

Pegasus/Config/DefaultPropertyTableLinux.h

HP-UX

Pegasus/Config/DefaultPropertyTableHpux.h

OpenVMS

Pegasus/Config/DefaultPropertyTableVms.h

##### hostname

**Description:**  Can be used to override the local system supplied hostname without domain the CIM Server is using to build objects. If not set, querying this configuration option will report the system supplied hostname and behavior is not changed.  
Setting this value to a valid hostname on CIM Server startup or as planned value will force the server to assume the configured name as the local hosts name. Setting this allows the administrator to set the name that operations such as associator, reference etc. return with object paths.  
In common setups it is not recommended to explicitly set this value.  
**Usage scenarios:**  The following scenarios in which changing the default value makes sense are known today:  
1\. The case where what the CIM Server shall return to clients for host name is NOT the name of the current host but some administrator defined name (ex. system behind a firewall or some redirector).  
2\. Embedded systems that have a system set hostname which is not valid and needs to be overridden by CIM administrator without changing the actual IP configuration.  
**Recommended Default Value (Development Build):**  blank  
**Recommended Default Value (Release Build):**  blank  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:** When setting this configuration option the option fullyQualifiedHostName should be adjusted too.  
Providers returning the systems hostname in an instance property might return a different value then the CIM server is setting in the object path. hostnames explicitly set into the object paths by providers will not be overridden by the CIM Server to allow cross-namespace associations.  
It is necessary to upgrade existing repositories using the repupgrade utility when upgrading from an OpenPegasus version less than 2.12.  
**Source Configuration File:**  Pegasus/Config/DefaultPropertyOwner.cpp

##### fullyQualifiedHostName

**Description:**  Can be used to override the local system supplied fully qualified hostname (with domain) the CIM Server is using to build objects. If not set, querying this configuration option will report the system supplied fully qualified hostname and behavior is not changed.  
Setting this value to a valid hostname on CIM Server startup or as planned value will force the server to assume the configured name as the local hosts name with domain. Setting this allows the administrator to set the name that indication operations and the CIM\_ObjectManager assume for SystemName returned as instance and key properties.  
In common setups it is not recommended to explicitly set this value.  
**Usage scenarios:**  The following scenarios in which changing the default value makes sense are known today:  
1\. The case where what the CIM Server shall return to clients for SystemName is NOT the name of the current host but some administrator defined name (ex. system behind a firewall or some redirector).  
2\. Embedded systems that have a system set hostname which is not valid and needs to be overridden by CIM administrator without changing the actual IP configuration.  
**Recommended Default Value (Development Build):**  blank  
**Recommended Default Value (Release Build):**  blank  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:** When setting this configuration option the option fullyQualifiedHostName should be adjusted too.  
Providers returning the systems hostname in an instance property might return a different value then the CIM server is setting in SystemName.  
It is necessary to upgrade existing repositories using the repupgrade utility when upgrading from an OpenPegasus version less than 2.12.  
**Source Configuration File:**  Pegasus/Config/DefaultPropertyOwner.cpp

##### httpAuthType

**Description:** Type of HTTP authentication. Allowed values are 'basic', indicating basic authentication or 'negotiate' indicating use of HTPP Negotiate authentication method. This variable is effective only when 'enableAuthentication' variable is set to 'true'.  
**Default Value:** basic  
**Recommended Default Value (Development Build):** basic  
**Recommended Default Value (Release Build):** basic  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No  
**Dynamic?:** No  
**Considerations:** Value 'negotiate' is available only when Pegasus is compiled with PEGASUS\_NEGOTIATE\_AUTHENTICATION=true.  
**Source Configuration File:**  Pegasus/Config/SecurityPropertyOwner.cpp

##### httpPort

**Description:** OpenPegasus first attempts to look up the port number for HTTP using getservbyname for the 'wbem-http' service. The httpPort configuration setting is used only when the getservbyname lookup fails.  
**Default Value:** blank  
**Recommended Default Value (Development Build):** blank  
**Recommended Default Value (Release Build):** 5988  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** No  
**Considerations:** (1)The use of 5988 for WBEM HTTP is recommended by the DMTF. This port has been registered with [IANA](http://www.iana.org/assignments/port-numbers). In a production environment, the recommendation is to use /etc/services to configure the value of this port. (2) With the release of OpenPegasus 2.5, the default values for httpPort was changed from 5988 to "". This change was made to allow the OpenPegasus to differentiate between a default setting of 5988 and a customer-defined setting of 5988.  With this change,  if the port number is explicitly set, the configured port will be used regardless of the settings in /etc/services. If no port number is specified, the server will continue to use the value in /etc/services.  
  

Platform

Source Configuration File

Linux

Pegasus/Config/DefaultPropertyTableLinux.h

HP-UX

Pegasus/Config/DefaultPropertyTableHpux.h

OpenVMS

Pegasus/Config/DefaultPropertyTableVms.h

##### httpsPort

**Description:**  OpenPegasus first attempts to look up the port number for HTTPS using getservbyname for the 'wbem-https' service. The httpsPort configuration setting is used only when the getservbyname lookup fails.  
**Default Value:** blank  
**Recommended Default Value (Development Build):** blank  
**Recommended Default Value (Release Build):** 5989  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** No  
**Considerations:** (1)The use of 5989 for WBEM HTTPS is recommended by the DMTF. This port has been registered with [IANA](http://www.iana.org/assignments/port-numbers). In a production environment, the recommendation is to use /etc/services to configure the value of this port.(2) With the release of OpenPegasus 2.5, the default values for httpPort was changed from 5989 to "". This change was made to allow the OpenPegasus to differentiate between a default setting of 5989 and a customer-defined setting of 5989.  With this change,  if the port number is explicitly set, the configured port will be used regardless of the settings in /etc/services. If no port number is specified, the server will continue to use the value in /etc/services.  
  

Platform

Source Configuration File

Linux

Pegasus/Config/DefaultPropertyTableLinux.h

HP-UX

Pegasus/Config/DefaultPropertyTableHpux.h

OpenVMS

Pegasus/Config/DefaultPropertyTableVms.h

##### httpSessionTimeout

**Description:** If set to a positive integer, this value specifies a HTTP session lifetime in seconds.  
After initial authentication using standard HTTP mechanisms (HTTP Basic or Negotiate authentication), the server generates a cookie and sends it back to the client, as described in RFC 6265. The client can then use the cookie in subsequent requests to skip the usual HTTP authentication. The cookie is valid only for period of time specified by this configuration option.  
If set to zero, session management is turned off and CIM server will not issue cookies.  
This option is available only when Pegasus was compiled with PEGASUS\_ENABLE\_SESSION\_COOKIES set to 'true'.  
**Recommended Default Value (Development Build):** 0  
**Recommended Default Value (Release Build):** 0  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** Yes  
**Considerations:**  The session timeout should be set to relatively small number (e.g. 30) to ensure the window, when the a potentially stolen cookie can be re-used, is quite small. This means every client will be re-authenticated the usual way (HTTP Basic or Negotiate) at least every 30 seconds.  

##### idleConnectionTimeout

**Description:** If set to a positive integer, this value specifies a minimum timeout value for idle client connections. If set to zero, idle client connections do not time out.  
**Recommended Default Value (Development Build):** 0  
**Recommended Default Value (Release Build):** 0  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** Yes  
**Considerations:** A client connection is considered idle when it is not in the process of sending a request and the CIM Server is not processing a request from that connection. An idle connection timeout allows reclamation of the system resources allocated for idle CIM Server connections. Note that the CIM Server may allow an idle connection to persist longer than the configured timeout value based on server activity. Some client applications may not behave correctly if a connection timeout is introduced. Client compatibility should be considered when configuring an idle connection timeout value. The idle connection time is computed using the system clock. Thus, resetting the system clock may cause unexpected timeout behavior.This config property can be changed dynamically. The changes will apply to all new and existing connections.  

##### maxFailedProviderModuleRestarts

**Description:** If set to a positive integer, this value specifies the number of times the failed provider module with indications enabled are restarted automatically before being moved to Degraded state. If set to zero, failed provider module is not restarted with indications enabled automatically and will be moved to Degraded state immediately.  
**Recommended Default Value (Development Build):** 3  
**Recommended Default Value (Release Build):** 3  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** Yes  
**Considerations:** This option controls the automatic re-enabling of the failed provider module with indications enabled. If the provider module with indications enabled fails very frequently, it affects the CIMServer normal operations because CIMServer would be busy with reloading the provider module every time it fails. This option would be helpful if the long running indication provider crashes and the management application want to receive the indications from the provider while provider fix is being delivered. In case of provider module grouping, if one of the indication provider crashes, it affects all indication providers in the provider modules of the group. Note that this is the facility provided by the CIMServer for temporary recovery of the provider and the ultimate goal SHALL be fixing the faulty provider.

##### maxIndicationDeliveryRetryAttempts

**Description:**  If set to a positive integer, value defines the number of times indication service will enable the reliableIndication feature and try to deliver an indication to a particular listener destination. This does not effect the original delivery attempt. A value of disables reliable indication feature completely, and cimserver will deliver the indication once.This is in accord with DTMF Specification 1054, Indication Profile, version 1.2.1.  
**Recommended Default Value (Development Build):** 3  
**Recommended Default Value (Release Build):** 3  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:**  This value is used to set the CIM\_IndicationService.DeliveryRetryAttempts property. See CIM\_IndicationService.DeliveryRetryAttempts property for more details. This option is supported only when PEGASUS\_ENABLE\_DMTF\_INDICATION\_PROFILE\_SUPPORT is defined.  
**Source Configuration File:**  Pegasus/Config/IndicationServicePropertyOwner.cpp  

##### minIndicationDeliveryRetryInterval

**Description:**  If set to a positive integer, this value defines the minimal time interval in seconds for the indication service to wait before retrying to deliver an indication to a listener destination that previously failed. Cimserver may take longer due to QoS or other processing.  
**Recommended Default Value (Development Build):** 30  
**Recommended Default Value (Release Build):** 30  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:**  This value is used to initialize the property CIM\_IndicationService.DeliveryRetryInterval. See CIM\_IndicationService.DeliveryRetryInterval property for more details. This option is supported only when PEGASUS\_ENABLE\_DMTF\_INDICATION\_PROFILE\_SUPPORT is defined.  
**Source Configuration File:**  Pegasus/Config/IndicationServicePropertyOwner.cpp  

##### listenAddress

**Description:**  Network interface where the cimserver should listen to for connections.For example listenAddress=All or listenAddress=121.12.33.112 or listenAddress=121.33.21.26,127.0.0.1, fe80::fe62:9346%eth0  
**Recommended Default Value (Development Build):** All  
**Recommended Default Value (Release Build):** No Specific suggestion  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:** Default value of "All" will make cimserver to listen on all the network interfaces.  
**Source Configuration File:**  Pegasus/Config/DefaultPropertyOwner.cpp  

##### logdir

**Description:** Specifies the name of the directory to be used for the OpenPegasus specific log files.  
**Recommended Default Value (Development Build):** logs  
**Recommended Default Value (Release Build):** $LOGDIR  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** Yes  
**Considerations:** This option is not supported if PEGASUS\_USE\_SYSLOGS is defined.  
**Source Configuration File:**  Pegasus/Config/LogPropertyOwner.cpp  

##### logLevel

**Description:** Defines the desired level of logging. Valid values include: TRACE, INFORMATION, WARNING, SEVERE, FATAL.  
**Default Value:** "INFORMATION"  
**Recommended Default Value (Development Build):**  "INFORMATION"  
**Recommended Default Value (Release Build):** "SEVERE"  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** Yes  
**Considerations:**   
**Source Configuration File:**  Pegsus/Config/LogPropertyOwner.cpp

##### mapToLocalName

**Description:**  If 'true', all remote client names are translated to local ones using GSSAPI. This applies only to clients using Negotiate authentication method, i.e. when "httpAuthType" runtime variable is set to "negotiate".  
**Default Value:** false  
**Recommended Default Value (Development Build):**  false  
**Recommended Default Value (Release Build):** false  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:** This option is available only when Pegasus is compiled with PEGASUS\_NEGOTIATE\_AUTHENTICATION=true.  
**Source Configuration File:**  Pegasus/Config/SecurityPropertyOwner.cpp

##### maxLogFileSizeKBytes

**Description:** Specifies the maximum size of the logfile in Kilo Bytes. If the logfile size exceeds maxLogFileSizeKBytes logfile will be pruned.  
**Default Value:** 32768  
**Recommended Default Value (Development Build):**  32768  
**Recommended Default Value (Release Build):** 32768  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** Yes  
**Considerations:** This option is not supported if PEGASUS\_USE\_SYSLOGS is defined. Minimum accepted value is 32 KB.   
**Source Configuration File:**  Pegsus/Config/LogPropertyOwner.cpp

##### maxProviderProcesses

**Description:** Limits the number of provider processes (see 'forceProviderProcesses) that may run concurrently. A 'maxProviderProcesses' value of '0' indicates that the number of Provider Agent processes is unlimited  
**Default Value:** 0  
**Recommended Default Value (Development Build):** 0  
**Recommended Default Value (Release Build):** 0  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/Yes  
**Dynamic?:** Yes  
**Considerations:** Consider there are five provider processes already running and the maxProviderProcesses is dynamically changed to four. In this case though new provider processes will not be created but existing five will continue running  
  

Platform

Source Configuration File

Linux

Pegasus/Config/DefaultPropertyTableLinux.h

HP-UX

Pegasus/Config/DefaultPropertyTableHpux.h

OpenVMS

Pegasus/Config/DefaultPropertyTableVms.h

##### messageDir

**Description:** Specifies the name of the directory to be used for the OpenPegasus translated messages.  
**Default Value:** msg  
**Recommended Default Value (Development Build):** msg  
**Recommended Default Value (Release Build):** $MESSAGEDIR/msg  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** No  
**Considerations:** Only used when message localization is enabled.  See PEGASUS\_HAS\_MESSAGES for details.  
**Source Configuration File:**  FileSystemPropertyOwner.cpp  

##### providerDir

**Description:** Specifies the names of the directories that contains Provider executables.  
**Recommended Default Value (Development Build):** lib  
**Recommended Default Value (Release Build):**  $PROVIDERDIRS  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** Yes  
**Considerations:** The CIM Server runs as a privileged user. And, since Providers run in the same process space as the CIM Server, Providers also have privileged access to the system. Allowing a registered Provider’s executable to be replaced with malicious code would result in a critical security defect, giving a malicious user privileged access to the system. Administrators are responsible for ensuring that only trusted Providers are loaded into providerDir. In order to simplify the management task associated with securing Provider executable, the value of providerDir can be fixed at build time (i.e., set to one or more fixed, well-known locations).  
**Source Configuration File:**  Pegasus/Config/FileSystemPropertyOwner.cpp  

##### providerManagerDir

**Description:** Specifies the name of the directory that contains the ProviderManager plugin libraries.  
**Recommended Default Value (Development Build):** lib  
**Recommended Default Value (Release Build):**  $PROVIDERMANAGERDIRS  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** No  
**Considerations:** Should be a dedicated directory to minimize inspection of non-applicable files. Additionally, since all calls to the providers go through the provider managers, the same care should be taken with regards to security for the provider manager directory, as for provider directories.  
**Source Configuration File:**  Pegasus/Config/FileSystemPropertyOwner.cpp  

##### repositoryDir

**Description:** Specifies the name of the directory to be used for the OpenPegasus repository.  
**Recommended Default Value (Development Build):** repository  
**Recommended Default Value (Release Build):**  $REPOSITORYDIR/repository  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** No  
**Considerations:**   
**Source Configuration File:**  Pegasus/Config/FileSystemPropertyOwner.cpp

  

##### **repositoryIsDefaultInstanceProvider**

**Description:** If true, the Repository functions as the Instance Provider for all classes for which there is not an explicitly registered Provider. This flag is also used to control the behavior of the repository when processing association operators.  
**Recommended Default Value (Development Build):** true  
**Recommended Default Value (Release Build):** false  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/Yes  
**Dynamic?:**  No  
**Considerations:**  This flag can be used to configure the CIM Server to allow the Repository to be used by CIM Clients and CIM Providers as a dynamic data store. The setting of this flag will significantly affect CIM Server behavior as viewed by the CIM Client.  
The following issues should be considered when setting repositoryIsDefaultInstanceProvider=true:

*   **Authorization.**  The repository has no concept of which users (or components) are authorized to perform which operations.
*   **Appearance of instrumentation.**  A client application cannot determine whether instrumentation exists for a given CIM class. For example,  imagine a client issues a CreateInstance operation on a Disk class for    which no provider is registered.  The instance will be stored in the repository, and the client gets a "success" response. Does that mean a disk device has been created on the server? Even worse, when another client issues an EnumerateInstances operation, the spurious Disk instance is returned.  With repositoryIsDefaultInstanceProvider=false, the CreateInstance operation above would have returned a NOT\_SUPPORTED error.
*   **Potential for inconsistent data.**  Using the repository as a data cache increases the probability that a client will see stale or incorrect data. A provider can better control the correlation between the data returned and the current state of the underlying managed resource (whether or not the provider caches the data).

  
The following issues should be considered when setting repositoryIsDefaultInstanceProvider=false:

*   **Provider Availability.** Certain Providers have been developed to use the Repository as a dynamic data store. These Providers are currently not supported on system where this flag is disabled. E.g., the Common Diagnostics Model (CDM) Provider (http://developer.intel.com/design/servers/cdm/) relies on use of the Repository to store instances of CIM\_DiagnosticSetting.  If this flag set to false, this Provider will not function correctly. Note that this feature is supported by other WBEM implementation.  Therefore, disabling this feature can increase the cost of migrating existing Providers to OpenPegasus.

**Source Configuration File:**  Pegasus/Config/RepositoryPropertyOwner.cpp  

##### **shutdownTimeout**

**Description:** When a cimserver -s shutdown command is issued, specifies the maximum time in seconds for the CIM Server to complete outstanding CIM operation requests before shutting down; if the specified timeout period expires, the CIM Server will shut down, even if there are still CIM operations in progress. Minimum value is 2 seconds.  
**Recommended Default Value (Development Build):** 10  
**Recommended Default Value (Release Build):** 10  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):**No/No  
**Dynamic?:** Yes  
**Considerations:**   
**Source Configuration File:**  Pegasus/Config/ShutdownPropertyOwner.cpp  

##### **slp**

**Description:** When set to true, OpenPegasus activates an SLP SA and issues DMTF defined SLP advertisements to this SA on startup.  
**Default Value:** false  
**Recommended Default Value (Development Build):** false  
**Recommended Default Value (Release Build):** false  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** No

   
  

Platform

Source Configuration File

Linux

Pegasus/Config/DefaultPropertyTableLinux.h

HP-UX

Pegasus/Config/DefaultPropertyTableHpux.h

OpenVMS

Pegasus/Config/DefaultPropertyTableVms.h

##### **slpProviderStartupTimeout**

**Description:** Timeout value in milli seconds used to specify how long the registration with an SLP SA may take. Registration will be retried three times. This value only needs to be increased in case that the loading of a set of providers who reporting their implementation of a registered profile takes very long.  
**Default Value:** 300000  
**Recommended Default Value (Development Build):** 300000  
**Recommended Default Value (Release Build):** 300000  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No

 

Platform

Source Configuration File

Linux

Pegasus/Config/DefaultPropertyTable.h

HP-UX

Pegasus/Config/DefaultPropertyTable.h

OpenVMS

Pegasus/Config/DefaultPropertyTable.h

##### **socketWriteTimeout**

**Description:** If the CIM Server receives an EWOULDBLOCK/EAGAIN error on a non-blocking write, socketWriteTimeout defines the number of seconds the CIM Server will wait for the socket to get ready and resume writing data.  
**Recommended Default Value (Development Build):**  20 (seconds)  
**Recommended Default Value (Release Build):**  20 (seconds)  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** Yes  
**Considerations:** Any changes made to this config property dynamically will apply to new CIM Server connections only keeping old already established connections unchanged.  
**Source Configuration File:**  Pegasus/Config/DefaultPropertyTable.h

##### **sslCertificateFilePath**

**Description:** Contains the CIM Server SSL Certificate.  
**Recommended Default Value (Development Build):**  server.pem  
**Recommended Default Value (Release Build):**  $CERTIFICATEDIR/server.pem"  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** No  
**Considerations:** For a more detailed look at SSL options and their ramifications, please see the SSL guidelines.  
**Source Configuration File:**  Pegasus/Config/SecurityPropertyOwner.cpp

##### **sslCipherSuite**

**Description:** String containing OpenSSL cipher specifications to configure the cipher suite the client is permitted to negotiate with the server during the SSL handshake phase.  
**Recommended Default Value (Development Build):**  DEFAULT (the default cipher list of OpenSSL)  
**Recommended Default Value (Release Build):**  DEFAULT (the default cipher list of OpenSSL)  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:** The value should be mentioned between single quotes since it can contain special characters like .+, !, -.. For more details look at SSL ciphers.  
**Source Configuration File:**  Pegasus/Config/SecurityPropertyOwner.cpp

##### **sslBackwardCompatibility**

**Description:**  If set to true, OpenPegasus support SSLv3 and TLSv1 protocol.If set to false OpenPegasus support only TLS1.2 protocol for openssl 1.0.1+  
**Recommended Default Value (Development Build):**  false  
**Recommended Default Value (Release Build):**  false  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:**  For a more detailed look at SSL options and their ramifications, please see the SSL guidelines.  
**Source Configuration File:**  Pegasus/Config/SecurityPropertyOwner.cpp

##### **sslClientVerificationMode**

**Description:** Describes the desired level of support for certificate-based authentication.

*   “**required**” – The server requires certificate-based client authentication.  A client MUST present a trusted certificate in order to access the CIM Server.  If the client fails to send a certificate or sends an untrusted certificate, the connection will be rejected.
*   “**optional**” – The server supports, but does not require, certificate-based client authentication. The server will request and attempt to validate a client certificate, however the connection will be accepted even if no certificate is sent or an untrusted certificate is sent.  The server will then seek to authenticate the client via an authentication header.
*   “**disabled**” – The server does not support certificate-based client authentication.

**Recommended Default Value (Development Build):**  disabled  
**Recommended Default Value (Release Build):**  disabled  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:** (1) This property is only used if **enableHttpsConnection** is **"true"**. (2) If the platform does not support PEGASUS\_LOCAL\_DOMAIN\_SOCKET, OpenPegasus attempts to use either HTTPS or HTTP to establish a local connection (connectLocal).  For platforms that do not support PEGASUS\_LOCAL\_DOMAIN\_SOCKET, Clients which use connectLocal, such as cimconfig -l -c, cimprovider -l -s and cimserver -s, will not work if  the "sslClientVerificationMode" variable is set to "required" and HTTP is disabled.  For these platforms, the recommended course of action is to change the property value to "optional."  
**Source Configuration File:**  Pegasus/Config/SecurityPropertyOwner.cpp

##### **sslKeyFilePath**

**Description:** Contains the private key for the CIM Server SSL Certificate.  
**Recommended Default Value (Development Build):**  file.pem  
**Recommended Default Value (Release Build):**  $CERTIFICATEDIR/file.pem  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** No  
**Considerations:** File should be readable only by the user the cimserver is running as and/or privileged users. The file should be writeable only by a privileged user.  
**Source Configuration File:**  Pegasus/Config/SecurityPropertyOwner.cpp  

##### **sslTrustStore**

**Description:** Specifies the location of the OpenSSL truststore. Consistent with the OpenSSL implementation, a truststore can be either a file or directory. If the truststore is a directory, all the certificates within the directory are considered trusted.  
**Recommended Default Value (Development Build):**  client.pem  
**Recommended Default Value (Release Build):**  $CERTIFICATEDIR/client.pem  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** No  
**Source Configuration File:**  Pegsus/Config/SecurityPropertyOwner.cpp

##### **sslTrustStoreUserName**

**Description:** Specifies the system user name to be associated with all certificate-based authenticated requests.  
**Recommended Default Value (Development Build):** NONE  
**Recommended Default Value (Release Build):** NONE  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Considerations:** This property has no default; for security reasons, the system administrator must explicitly specify this value.  This feature allows a single user name to be specified.  This user will be associated with all certificates in the truststore.  In the future, OpenPegasus will allow a system administrator to associate a distinct user name with each certificate.  
**Source Configuration File:**  Pegsus/Config/SecurityPropertyOwner.cpp  

##### **traceComponents**

**Description:** Defines the components to be traced.  
**Default Value:** ""  
**Recommended Default Value (Development Build):** ""  
**Recommended Default Value (Release Build):** ""  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/Yes  
**Dynamic?:** Yes  
**Source Configuration File:**  Pegasus/Config/TracePropertyOwner.cpp  

##### **traceFacility**

**Description:** Defines the desired facility for trace messages. Valid values include: "File" , "Log" and "Memory" .  
**Default Value:** File  
**Recommended Default Value (Development Build):** File  
**Recommended Default Value (Release Build):** File  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/Yes  
**Dynamic?:** Yes  
**Case Sensitive?:** No  
**Considerations:** If you specify "File", trace messagesare written into the file specified by the configuration option traceFilePath. If you specify "Log", trace messages are written to the OpenPegasus Log. If you specify Memory, the trace messages are collected in a wrap-around memory buffer. If this memory is part of a dump the trace messages can be found by the eye-catcher "PEGASUSMEMTRACE" at the top of the memory buffer. The trace is in clear text and the last written message has the suffix "EOTRACE".  
**Source Configuration File:**  Pegasus/Config/TracePropertyOwner.cpp  

##### **traceFilePath**

**Description:** Specifies the location of the OpenPegasus trace file.  
**Default Value:** cimserver.trc  
**Recommended Default Value (Development Build):** cimserver.trc  
**Recommended Default Value (Release Build):**  $TRACEDIR/cimserver.trc  
**Recommend To Be Fixed/Hidden (Development Build):**No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes  
**Dynamic?:** Yes  
**Source Configuration File:**  Pegasus/Config/TracePropertyOwner.cpp  

##### **traceLevel**

**Description:** Defines the desired level of tracing. Valid values include: 0, 1, 2, 3, 4, 5.  
**Default Value:** 0  
**Recommended Default Value (Development Build):** 0  
**Recommended Default Value (Release Build):** 0<  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/Yes  
**Dynamic?:** Yes  
**Source Configuration File:**  Pegasus/Config/TracePropertyOwner.cpp  

##### **traceMemoryBufferKbytes**

**Description:** Defines the size of the buffer for in memory tracing in kbytes ( 210 bytes ). Smallest value is 16.  
**Default Value:** 10240  
**Recommended Default Value (Development Build):** 10240  
**Recommended Default Value (Release Build):** 10240  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/No  
**Dynamic?:** No  
**Case Sensitive?:** No  
**Considerations:** Is ignored if configuration property traceFacility is not set to Memory.  
**Source Configuration File:**  Pegasus/Config/TracePropertyOwner.cpp  

##### **traceFileSizeKBytes**

**Description:** Defines the size of the tracefile in Kilo bytes. The trace file will be rolled once its size exceeds the specified size.  
**Default Value:** 1048576  
**Recommended Default Value (Development Build):** 1048576  
**Recommended Default Value (Release Build):** 1048576  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/Yes  
**Dynamic?:** Yes  
**Considerations:**Minimum accepted value is 10240 KB,Maximum accepted value is 2097152 KB.    
**Source Configuration File:**  Pegsus/Config/TracePropertyOwner.cpp

##### **numberOfTraceFiles**

**Description:** Specifies the number of the tracefiles for rolling.  
**Default Value:** 3  
**Recommended Default Value (Development Build):** 3  
**Recommended Default Value (Release Build):** 3  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** No/Yes  
**Dynamic?:** Yes  
**Considerations:**Minimum accepted value is 3,Maximum accepted value is 20.    
**Source Configuration File:**  Pegsus/Config/TracePropertyOwner.cpp

##### **webRoot**

**Description:**  Points to a location where the static web pages to be served by the pegasus webserver are stored.  
**Default Value:** www  
**Recommended Default Value (Development Build):** www  
**Recommended Default Value (Release Build):** $WEBCONFDIR/www  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes/Yes  
**Dynamic?:** No  
**Considerations:** When not fixed and hidden, it can be modified to point to a location having spurious pages. Please consider fixing this value.    
**Source Configuration File:**  Pegsus/Config/DefaultPropertyOwner.cpp

##### **indexFile**

**Description:**  Specifies the name of index file used by pegasus webserver, default to index.html. This file should be available at the webRoot path.  
**Default Value:** index.html  
**Recommended Default Value (Development Build):** index.html  
**Recommended Default Value (Release Build):** index.html  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes/Yes  
**Dynamic?:** No  
**Considerations:**   Hide it and fixed in release build to prevent using any file as index file  
**Source Configuration File:**  Pegsus/Config/DefaultPropertyOwner.cpp

##### **mimeTypesFile**

**Description:**  Refers to the file which holds the mime types being served by the pegasus webserver.  
**Default Value:** www/mimeTypes.txt  
**Recommended Default Value (Development Build):** www/mimeTypes.txt  
**Recommended Default Value (Release Build):** $WEBCONFDIR/www/mimeTypes.txt  
**Recommend To Be Fixed/Hidden (Development Build):** No/No  
**Recommend To Be Fixed/Hidden (Release Build):** Yes/Yes  
**Dynamic?:** No  
**Considerations:** Fix and make it read only when to prevent modification in deployment  
**Source Configuration File:**  Pegsus/Config/DefaultPropertyOwner.cpp

### **Configuration Constants**

#### **Definitions**

##### **static char CURRENT\_CONFIG\_FILE \[\] =**

**Description:** Default file name for the current configuration file.  
**Recommended Default Value (Development Build):** cimserver\_current.conf  
**Recommended Default Value (Release Build):** $CONFIGDIR/cimserver\_current.conf  
**Dynamic?:** No   
  

Platform

Source Configuration File

Linux

Pegasus/Config/ConfigFileDirLinux.h

HP-UX

ConfigFileDirHpux.h

OpenVMS

Pegasus/Config/ConfigFileDirVms.h

##### **static char PLANNED\_CONFIG\_FILE \[\] =**

**Description:** Default file name for the planned configuration file.  
**Recommended Default Value (Development Build):**  cimserver\_planned.conf  
**Recommended Default Value (Release Build):**  $CONFIGDIR/cimserver\_planned.conf  
**Dynamic?:** No  
  

Platform

Source Configuration File

Linux

Pegasus/Config/ConfigFileDirLinux.h

HP-UX

Pegasus/Config/ConfigFileDirHpux.h

OpenVMS

Pegasus/Config/ConfigFileDirVms.h

##### **static char CIMSERVER\_START\_FILE \[\] =**

**Description:** This is for the default file name for the cimserver startup file containing the PID.  
**Default Value:** /tmp/cimserver\_start.conf  
**Recommended Default Value (Development Build):**  /tmp/cimserver\_start.conf  
**Recommended Default Value (Release Build):**  $PIDFILE  
**Dynamic?:** No  
  

Platform

Source Configuration File

Linux

Pegasus/Config/ConfigFileDirLinux.h

HP-UX

Pegasus/Config/ConfigFileDirHpux.h

OpenVMS

Pegasus/Config/ConfigFileDirVms.h

### **Configuration Macros**

#### **Definitions**

##### **PEGASUS\_SSLCLIENT\_CERTIFICATEFILE**

****Description:**   
**Recommended Default Value (Development Build):** client.pem  
**Recommended Default Value (Release Build):**  $CERTIFICATEDIR/client.pem  
**Dynamic?:** No  
**Source Configuration File:**  Pegasus/Common/Constants.h  
**

##### **PEGASUS\_SSLCLIENT\_RANDOMFILE**

**Description:** On platforms that do not support /dev/random or /dev/urandom, OpenSSL will use this file to seed the PRNG (Pseudo-Random Number Generator). This PEGASUS\_SSLCLIENT\_RANDOMFILE defines the default CIM Client random file.  
**Recommended Default Value (Development Build):**ssl.rnd  
**Recommended Default Value (Release Build):** $RANDOMDIR/ssl.rnd  
**Dynamic?:** No  
**Considerations:** Use of this variable requires  PEGASUS\_HAS\_SSL to be set.  CIM Client use of a shared random file is not recommended.  On systems where /dev/random or /dev/urandom are not available, CIM Clients are encouraged to generate a unique random file which contains sufficient entropy.  
**Source Configuration File:**  Pegasus/Common/Constants.h  

##### **PEGASUS\_SSLSERVER\_RANDOMFILE**

**Description:** On platforms that do not support /dev/random or /dev/urandom, OpenSSL will use this file to seed the PRNG (Pseudo-Random Number Generator). This PEGASUS\_SSLSERVER\_RANDOMFILE defines the CIM Server random file.  
**Recommended Default Value (Development Build):**  cimserver.rnd  
**Recommended Default Value (Release Build):**  $RANDOMDIR/cimserver.rnd  
**Dynamic?:** No  
**Considerations:** 1)Use of this variable requires  PEGASUS\_HAS\_SSL to be set.(2) On systems where /dev/random or /dev/urandom are not available, CIM Server vendors need to be sure that the cimserver.rnd file contains sufficient entropy and is uniquely generated for each system.  
**Source Configuration File:**  Pegasus/Common/Constants.h

##### ****PEGASUS\_LOCAL\_AUTH\_DIR****

**Description:** For local connections (i.e., connectLocal), OpenPegasus uses a file-based authentication mechanism.  
**Recommended Default Value (Development Build):** /tmp  
**Recommended Default Value (Release Build):**  $LOCALAUTHDIR/localauth  
**Dynamic?:** No  
**Source Configuration File:**  Pegasus/Common/Constants.h  

##### **PEGASUS\_LOCAL\_DOMAIN\_SOCKET\_PATH**

**Description:**   
**Recommended Default Value (Development Build):**  /tmp/cimxml.socket  
**Recommended Default Value (Release Build):**  $SOCKETDIR/cimxml.socket  
**Dynamic?:** No  
**Source Configuration File:**  Pegasus/Common/Constants.h  

* * *

**Licensed to The Open Group (TOG) under one or more contributor license agreements. Refer to the OpenPegasusNOTICE.txt file distributed with this work for additional information regarding copyright ownership. Each contributor licenses this file to you under the OpenPegasus Open Source License; you may not use this file except in compliance with the License.**

**Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:**

**The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.**

**THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.**

* * *
