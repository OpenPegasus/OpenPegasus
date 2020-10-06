@echo off
REM ***********************************************************
REM ***********************************************************
REM Batch file for building the Pegasus-WMI Mapper on Windows
REM
REM NOTE: To run this script, GNU make (gnumake.exe) and
REM Pegasus mu.exe MUST BE in the PATH or in PEGASUS_ROOT
REM
REM Also, this batch file assumes Visual C++ 6.0 and the
REM Microsoft Platform SDK are installed, vcvars32.bat is
REM in the PATH and the MSSdk env var is defined. Other
REM environments (e.g., VisualStudio.NET) may require
REM modifications to this batch file.
REM
REM Environment Variables:
REM   PLATFORM
REM   - Indicates the target platform for the build
REM   - Valid settings are "Win32" and "Win64"
REM   - Default(if not set) = "Win32"
REM   FLAVOR
REM   - Indicates whether to build debug or product/release bits
REM   - Valid settings are "product" and "debug"
REM   - Default(if not set) = "product"
REM   PEGASUS_ROOT
REM   - Root directory of the Pegasus "source" to build from
REM     (e.g., "C:\Pegasus")
REM   - No default -- MUST BE SET prior to running this script
REM     or the script will fail.
REM   PEGASUS_HOME
REM   - Root directory where the built Pegasus binaries, etc.,
REM     are to be "installed" (e.g., "C:\Pegasus")
REM   - No default -- MUST BE SET prior to running this script
REM     or the script will fail.
REM   PEGASUS_HAS_SSL
REM   - If set, indicates Pegasus to be built with SSL support
REM   - If not set, Pegasus is still built with SSL by default
REM     (may be overridden by the command line param, below)
REM   OPENSSL_HOME
REM   - Root directory of the OpenSSL libraries/sdk
REM   - No default -- must be set prior to running this script
REM     if SSL support is specified (see PEGASUS_HAS_SSL, above).
REM
REM Command Line Parameters (used to override environment vars)
REM   [-]Win32      overrides PLATFORM=Win32
REM   [-]Win64      overrides PLATFORM=Win64
REM   [-]d[ebug]    overrides FLAVOR=debug
REM   [-]p[rodcut]  overrides FLAVOR=product
REM   [-]r[elease]  overrides FLAVOR=product
REM   [-]ssl        overrides PEGASUS_HAS_SSL defined
REM   [-]nossl      overrides PEGASUS_HAS_SSL not defined
REM
REM ***********************************************************
REM ***********************************************************
echo Building Pegasus...

REM ***********************************************************
REM Keep any environment changes local
REM ***********************************************************
SETLOCAL

REM ***********************************************************
REM Default variables, if not already set
REM ***********************************************************
if not defined PLATFORM set PLATFORM=Win32
if not defined FLAVOR set FLAVOR=product
if not defined PEGASUS_HAS_SSL set PEGASUS_HAS_SSL=Yes
if not defined PEGASUS_WMIMAPPER set PEGASUS_WMIMAPPER=Yes
if not defined PEGASUS_ROOT goto ERR_NO_PEGASUS_ROOT
if not defined PEGASUS_HOME goto ERR_NO_PEGASUS_HOME

REM ***********************************************************
REM Add %PEGASUS_HOME%\bin to the PATH (needed for tests, etc.)
REM ***********************************************************
set PATH=%PEGASUS_HOME%\bin;%PATH%

REM ***********************************************************
REM Process command line args
REM
REM Valid args are (NOT case-sensitive):
REM [-]Win32      overrides PLATFORM=Win32
REM [-]Win64      overrides PLATFORM=Win64
REM [-]d[ebug]    overrides FLAVOR=debug
REM [-]p[rodcut]  overrides FLAVOR=product
REM [-]r[elease]  overrides FLAVOR=product
REM [-]ssl        overrides PEGASUS_HAS_SSL defined
REM [-]nossl      overrides PEGASUS_HAS_SSL not defined
REM ***********************************************************
:CHECK_COMMAND_LINE_ARG
if "%1" == "" goto DONE_WITH_COMMAND_LINE

if /i "%1"=="-Win32"   goto ARG_PLATFORM_WIN32
if /i "%1"=="Win32"    goto ARG_PLATFORM_WIN32

if /i "%1"=="-Win64"   goto ARG_PLATFORM_WIN64
if /i "%1"=="Win64"    goto ARG_PLATFORM_WIN64

if /i "%1"=="-debug"   goto ARG_FLAVOR_DEBUG
if /i "%1"=="debug"    goto ARG_FLAVOR_DEBUG
if /i "%1"=="-d"       goto ARG_FLAVOR_DEBUG
if /i "%1"=="d"        goto ARG_FLAVOR_DEBUG

if /i "%1"=="-product" goto ARG_FLAVOR_PRODUCT
if /i "%1"=="product"  goto ARG_FLAVOR_PRODUCT
if /i "%1"=="-p"       goto ARG_FLAVOR_PRODUCT
if /i "%1"=="p"        goto ARG_FLAVOR_PRODUCT

if /i "%1"=="-release" goto ARG_FLAVOR_PRODUCT
if /i "%1"=="release"  goto ARG_FLAVOR_PRODUCT
if /i "%1"=="-r"       goto ARG_FLAVOR_PRODUCT
if /i "%1"=="r"        goto ARG_FLAVOR_PRODUCT

if /i "%1"=="-ssl"     goto ARG_SSL
if /i "%1"=="ssl"      goto ARG_SSL

if /i "%1"=="-nossl"   goto ARG_NOSSL
if /i "%1"=="nossl"    goto ARG_NOSSL

REM: If here, have an INVALID ARGUMENT:
echo Invalid argument on the command line: %1. Ignoring...
goto NEXT_ARG

:ARG_PLATFORM_WIN32
set PLATFORM=Win32
goto NEXT_ARG

:ARG_PLATFORM_WIN64
set PLATFORM=Win64
goto NEXT_ARG

:ARG_FLAVOR_DEBUG
set FLAVOR=debug
goto NEXT_ARG

:ARG_FLAVOR_PRODUCT
set FLAVOR=product
goto NEXT_ARG

:ARG_SSL
set PEGASUS_HAS_SSL=Yes
if not defined OPENSSL_HOME goto ERR_NO_OPENSSL_HOME
set PATH=%OPENSSL_HOME%\bin;%PATH%
goto NEXT_ARG

:ARG_NOSSL
set PEGASUS_HAS_SSL=
goto NEXT_ARG

:NEXT_ARG
shift
goto CHECK_COMMAND_LINE_ARG

:DONE_WITH_COMMAND_LINE

REM ***********************************************************
REM Translate the PLATFORM and FLAVOR vars into environment
REM variables used by Pegasus for specifying platform & flavor
REM ***********************************************************
if /i "%FLAVOR%" == "product" set PEGASUS_DEBUG=
if /i "%FLAVOR%" == "debug" set PEGASUS_DEBUG=1

REM If/when Pegasus has a Win64/IPF config, the following can be
REM used for setting the appropriate config, based on the PLATFORM:
REM if "%PLATFORM%" == "Win32" set PEGASUS_PLATFORM=WIN32_IX86_MSVC
REM if "%PLATFORM%" == "Win64" set PEGASUS_PLATFORM=WIN64_IA64_MSVC
REM For now, using WIN32_IX86_MSVC, (even for Win64/IPF builds):
set PEGASUS_PLATFORM=WIN32_IX86_MSVC

REM ***********************************************************
REM This sets up the Visual C++/Platform SDK environment
REM ***********************************************************

REM The MS Platform SDK has the following platform options:
REM     /2000 - target Windows 2000 and IE 5.0
REM     /XP32 (default) - target Windows XP 32
REM     /XP64 - target Windows XP 64
REM     /SRV32 - target Windows Server 2003 32 bit
REM     /SRV64 - target Windows Server 2003 64 bit
REM     /x86_64 - target Windows for the x86_64 bit platform
REM
if /i "%PLATFORM%" == "Win32" set PSDKPLATFORM=/2000
if /i "%PLATFORM%" == "Win64" set PSDKPLATFORM=/SRV64

REM The MS Platform SDK has the following flavor options:
REM     /DEBUG - set the environment to DEBUG
REM     /RETAIL - set the environment to RETAIL
REM
if /i "%FLAVOR%" == "product" set PSDKFLAVOR=/RETAIL
if /i "%FLAVOR%" == "debug" set PSDKFLAVOR=/DEBUG

REM Configure the development environment
call vcvars32.bat
call "%MSSdk%\setenv.bat" %PSDKPLATFORM% %PSDKFLAVOR%

REM ***********************************************************
REM Finally, run the builds
REM ***********************************************************

REM ***********************************************************
REM Makefiles must be run from the directory where they reside
REM ***********************************************************
pushd %PEGASUS_ROOT%

echo.
echo "======================================="
echo "Building the Pegasus core lib's & app's"
echo "======================================="
REM NOTE: Remove "rebuild" from the make command, below,
REM for a simpler build of Pegasus (build new/updated
REM files/dependencies only, no repository & no tests)
make rebuild

REM NOTE: Comment-out the following line to ignore Pegasus
REM build/test errors, and go on to build the Mapper:
if %errorlevel% NEQ 0 goto ERR_BUILD_FAILED

REM ***********************************************************
REM return to the original directory
REM ***********************************************************
popd

REM ***********************************************************
REM Now build the WMI Mapper
REM ***********************************************************
pushd %PEGASUS_ROOT%\src\WMIMapper

echo.
echo "======================================="
echo "Building the Pegasus WMI Mapper        "
echo "======================================="
make mapper
if %errorlevel% NEQ 0 goto ERR_BUILD_FAILED

REM ***********************************************************
REM return to the original directory
REM ***********************************************************
popd

REM ***********************************************************
REM Copy header files to PEGASUS_HOME, to complete the SDK
REM ***********************************************************
REM First, delete any existing include directory:
rmdir /S /Q %PEGASUS_HOME%\include

REM Now, create the include directory structure:
set COMMON_INCL=%PEGASUS_HOME%\include\Pegasus\Common
set CLIENT_INCL=%PEGASUS_HOME%\include\Pegasus\Client
set PROVDR_INCL=%PEGASUS_HOME%\include\Pegasus\Provider

REM This is how to do it if NT extensions are not enabled:
REM mkdir %PEGASUS_HOME%\include
REM mkdir %PEGASUS_HOME%\include\Pegasus
REM mkdir %PEGASUS_HOME%\include\Pegasus\Common
REM mkdir %PEGASUS_HOME%\include\Pegasus\Client
REM mkdir %PEGASUS_HOME%\include\Pegasus\Provider

REM With NT extensions enabled, can just do this:
mkdir %COMMON_INCL%
mkdir %CLIENT_INCL%
mkdir %PROVDR_INCL%

REM Finally, copy the headers:

pushd %PEGASUS_ROOT%\src\Pegasus\Common
copy AcceptLanguageList.h %COMMON_INCL%
copy Array.h %COMMON_INCL%
copy ArrayInter.h %COMMON_INCL%
copy Char16.h %COMMON_INCL%
copy CIMClass.h %COMMON_INCL%
copy CIMDateTime.h %COMMON_INCL%
copy CIMFlavor.h %COMMON_INCL%
copy CIMIndication.h %COMMON_INCL%
copy CIMInstance.h %COMMON_INCL%
copy CIMMethod.h %COMMON_INCL%
copy CIMName.h %COMMON_INCL%
copy CIMObject.h %COMMON_INCL%
copy CIMObjectPath.h %COMMON_INCL%
copy CIMParameter.h %COMMON_INCL%
copy CIMParamValue.h %COMMON_INCL%
copy CIMProperty.h %COMMON_INCL%
copy CIMPropertyList.h %COMMON_INCL%
copy CIMQualifier.h %COMMON_INCL%
copy CIMQualifierDecl.h %COMMON_INCL%
copy CIMScope.h %COMMON_INCL%
copy CIMStatusCode.h %COMMON_INCL%
copy CIMType.h %COMMON_INCL%
copy CIMValue.h %COMMON_INCL%
copy Config.h %COMMON_INCL%
copy ContentLanguageList.h %COMMON_INCL%
copy Exception.h %COMMON_INCL%
copy Formatter.h %COMMON_INCL%
copy LanguageParser.h %COMMON_INCL%
copy LanguageTag.h %COMMON_INCL%
copy Linkage.h %COMMON_INCL%
copy Logger.h %COMMON_INCL%
copy MessageLoader.h %COMMON_INCL%
copy OperationContext.h %COMMON_INCL%
copy Platform_WIN32_IX86_MSVC.h %COMMON_INCL%
copy ResponseHandler.h %COMMON_INCL%
copy SSLContext.h %COMMON_INCL%
copy String.h %COMMON_INCL%
copy System.h %COMMON_INCL%
popd

pushd %PEGASUS_ROOT%\src\Pegasus\Client
copy CIMClient.h %CLIENT_INCL%
copy CIMClientException.h %CLIENT_INCL%
copy Linkage.h %CLIENT_INCL%
popd

pushd %PEGASUS_ROOT%\src\Pegasus\Provider
copy CIMAssociationProvider.h %PROVDR_INCL%
copy CIMIndicationConsumerProvider.h %PROVDR_INCL%
copy CIMIndicationProvider.h %PROVDR_INCL%
copy CIMInstanceProvider.h %PROVDR_INCL%
copy CIMMethodProvider.h %PROVDR_INCL%
copy CIMOMHandle.h %PROVDR_INCL%
copy CIMProvider.h %PROVDR_INCL%
copy CIMQueryProvider.h %PROVDR_INCL%
copy Linkage.h %PROVDR_INCL%
copy ProviderException.h %PROVDR_INCL%
copy ResponseHandler.h %PROVDR_INCL%
popd


goto DONE
REM ***********************************************************
REM Error Message Sub-Routines
REM ***********************************************************
:ERR_NO_PEGASUS_ROOT
echo "Error: The environment variable PEGASUS_ROOT is not set!"
goto DONE
:ERR_NO_PEGASUS_HOME
echo "Error: The environment variable PEGASUS_HOME is not set!"
goto DONE
:ERR_NO_OPENSSL_HOME
echo "Error: The environment variable OPENSSL_HOME is not set!"
goto DONE
:ERR_BUILD_FAILED
echo Error: An error occured during the build.
goto DONE

:DONE
ENDLOCAL

:EOF
