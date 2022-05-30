# Building OpenPegasus

##Configuring OpenPegasus for build

### Overview

OpenPegasus  can be built from the source code in this repository and a build/test configuration based
on environment variables.

OpenPegasus is configured to be built with the gcc compiler, the CLANG compiler,
or the Microsoft compiler (windows). Note: The current release has been tested
only with the gcc compiler versions 8 and 9.

The following three environment variables required to build OpenPegasus:

1. **PEGASUS_PLATFORM** - Defines the platform for which OpenPegasus is being compiled.
   This variable name has three components defining the hardware, OS, and compiler
   build prerequisits. The values allowed are contained in a string that defines the
   hardware platform, OS, and compiler to be used.
2. **PEGASUS_ROOT** - Defines the 'pegasus' directory the top of the OpenPegasus source
   tree.  This directory contains the basic build Makefile for OpenPegasus
3. **PEGASUS_HOME** - Target directory for compile output.  The object/executable files
   for OpenPegasus are put into this directory as part of the build and other
   components required for the running OpenPegasus are included in this directory
   and its subdirectories as part of the setup.  Thus, binary files, the class repository,
   the directories for certificates and certificate revocation lists, any trace
   output, are normally placed in subdirectories of **PEGASUS_HOME**

OpenPegasus configuration is based entirely on environment variables to define the
location of the source code, the build output directory, and the compile configuration
options In general all OpenPegasus configuration environment variables start with
'PEGASUS_'.

Typically these could be defined as the following if OpenPegasus was cloned into
a linux user home directory:

``
export PEGASUS_ROOT="~/OpenPegasus/pegasus"
export PEGASUS_PLATFORM="LINUX_X86_64_GNU"
export PEGASUS_HOME="~/OpenPegasus/home"

```
Where a x86_64 hardware linux, and the gcc compiler are the basis for building
OpenPegasus.  The PEGASUS_HOME directory can be anywhere and was placed beneath
the OpenPegasus in this example for simplicity.

A number of other environment variables may be defined to control compiling of the
various options that are used in the OpenPegasus build.  There are many options including:

* Use of SSL
* Build basic options like debug build
* Enable specific otpions such as enabling the OOP (Out-of-process providers),
  provider user context, audit logger, CIM Repository format. Use of query languages
  ()i.e. CQL), disableing some request types such as ExecQuery, Enabling the interop
  provider and setting the interop namespace name.
* Defining specific components of the test environment.

These options are documented in the OpenPegasus document file BuildAndReleaseOptions.html
located in the OpenPegaus/pegasus/doc directory of the source code.  All compile and
test options for OpenPegasus are defined in this file as of OpenPegasus 2.14.3

### Setting up the configuration variables

The configuration options must be set before building OpenPegasus.  There are serveral
options for setting them.

1. There is a confiburation script (**configure** or **configure.bat**) that can be used
   to define groups of environment variables based on general OpenPegasus required
   general characteristics.  This configure aids in setting upt the required
   configuration options defined above (in particular it defines **PEGASUS_HOME**,
   and the **PEGASUS_PLATFORM**) variable based on the hardware, OS, and compilers to be used.

   This works with another OpenPegasus build mechanism, a
   **GNUmakefile** that is created by **configure** and automatically
   exports the configuration file **options.mak** created by
   **configure** and calls the OpenPegasus **pegasus/Makefile** to
   which eliminates the step of exporting the OpenPegasus configure
   options since they are created each time make is executed.

   **NOTE:** The **configure** script is considered experimental but
   provides a good starting point for building the configuration for a
   specific OpenPegasus since it defines the required variables and
   logical choices for the other configuration environment variables
   and tests the environment for external requirements.

3. Create the environment variables by hand or in a script and export
   them to the environment before starting the OpenPegasus build. the
   file setup.sh in OpenPegasus is an example of a hand build script.

4. Use pre-existing files in the OpenPegasus/root directory to set
   up particular environments for each platform type. These were
   developed by the original OpenPegasus developers and are named with
   the suffix **.status**. Thus, for example the proposed configuration
   file for linux is **env_var_Linux.status**.

## Building OpenPegasus

Once the configuration variables that will control the compile (and possibly the
setup and test of the build) are defined the build is executed by executing the
makefile <target>.  There are a number of targets in the Makefile to control how
OpenPegasus is
built, installed and tested or the code cleaned up including the capabilty to:

* execute the complete make ( ex. **make world**)
* Execute creation of componets such as the repository and tests and execution of unit
  and system level tests.  NOTE: The test typically take between one and two hours.
* remove the repository, and other build components (make clobber, etc.)

The list of <targets> can be found by executing:

    make usage

from the OpenPegasus/pegasus directory 

## OpenPegasus Makefile structure

OpenPegaus uses a hiearchy of make files starting with Makefile in the
pegasus directory and using utility make files in the
OpenPegasus/pegasus/mak directory.  There is a Makefile for each
directory in the OpenPegasus source code structure (**Makefile**)
defining the make for compile, test, and cleanup of the OpenPegasus
components in that directory.  The top level makefile simply recurses
through all the directoriess in OpenPegasus executing these individual
makefiles.

Thus, for example, with the environment variables set simply executing the command

   $ > make world

executes the complete compile and test of OpenPegasus using the currently defined
CIM MOF repository (**PEGASUS_CIM_SCHEMA**), the providers defined for the test
environment and the MOF defined to install and use those  test providers. There
are a number of test command line executables built, a number of test providers
installed and a class repository built when the **world** target is executed.

## Testing OpenPegasus OpenPegasus includes tests for all of the
Server, Client, IndicationListener etc. components of its capabilities,
with unit tests and end-end tests.  The test components for test of
each of the OpenPegasus components is contained in a subdirectory name
**tests** for the component. Thus the tests for The OpenPegasus server
common component (pegasus/src/Pegasus/Common) is in the directory
(pegauss/src/Pegasus/Common/tests)
In addition to the unit tests, OpenPegasus includes a number of test providers using
both of the OpenPegasus provider interfaces (C++ and CMPI) that are built and installed
in the OpenPegasus run environment when tests are to be executed.   These providers
also are a examples of OpenPegasus providers for both of these interfaces.  They
are located in the directory pegasus/src/providers.

The command:

    $> make world
    
executes the tests also.

OpenPegasus was designed to operate as root permissions so a number of options
may have to be disabled to test in within a user environment.
