# Building and Testing OpenPegasus

##Configuring OpenPegasus for build

### Overview

OpenPegasus must be built from the source code and a build/test configuration based
on Environment variables.

OpenPegasus build configuration is based entirely on environment variables to define the
location of the source code, the build output directory, and the compile configuration
options In general all OpenPegasus configuration environment variables start with
'PEGASUS_'.

There are three environment variables required to define the OpenPegasus compile
environment:

1. **PEGASUS_PLATFORM** Defines the platform for which OpenPegasus is being compiled.
   This variable is of the form <OS><hardware><Compiler>. Thus
       `export PEGASUS_PLATFORM=LINUX_X86_64_GNU`
   sets up for OpenPegasus build on:
   * Linux,
   * an X86_64 hardware platform
   * the GNU compiler
   The possible build configurations are controlled by specific make files in the
   `pegasus/make` directory.  Thus the build configuration `LINUX_X86_64_GNU` is defined
   in the makefile `OpenPegasus/pegasus/mak/platform_LINUX_IX86_GNU.mak`.
2. **PEGASUS_ROOT** - Defines the 'pegasus' directory which is the top of the OpenPegasus source
   tree.  This directory contains the basic build Makefile for OpenPegasus.
3. **PEGASUS_HOME** - Target directory for compile output.  The object/executable files
   for OpenPegasus are put into this directory as part of the build and other
   components required for the running OpenPegasus are included in this directory
   and its subdirectories as part of the setup.  Thus, binary files, the class repository,
   the directories for certificates and certificate revocation lists, any trace
   output, are normally placed in subdirectories of **PEGASUS_HOME**

A number of other environment variables may be defined to control compiling of the
various options that are used in the OpenPegasus build.  There are many options including:

* Use of SSL
* Build basic options like debug build
* Enable specific options such as enabling OOP (Out-of-process providers),
  provider user context, audit logger, CIM Repository format. Use of query languages
  (i.e. CQL), disabling some request types such as ExecQuery, Enabling the interop
  provider and setting the interop namespace name.
* Definig specific components of the test environment.

These options are documented in
[OpenPegasus/docs/BuildAndReleaseOptions.md](docs/BuildAndReleaseOptions.md)

### Setting up the build configuration variables

The configuration options must be defined before building OpenPegasus.  There are serveral
options for setting them.

1. There is a configuration script (**configure** , **configure.bat**) that can be used
   to define groups of environment variables based on general OpenPegasus
   characteristics.  This configure tool aids in setting up the required
   configuration options defined above (in particular it defines **PEGASUS_HOME**,
   and the **PEGASUS_PLATFORM**) variable based on the hardware, OS, and compilers to be used.

   The **configure** tool sets the configuration variables into a file
   **options.mak** and creates new top level makefile **GNUmakefile**
   which exports the definitions in **options.mak** and then calls the
   OpenPegasus **pegasus/Makefile**. This eliminates the step of
   exporting the OpenPegasus configure options since they are created
   each time make is executed.

   **NOTE:** **configure** is considered experimental but provides a
   good starting point for building the configuration for a specific
   OpenPegasus since it defines the required variables and logical
   choices for the other configuration environment variables and tests
   the environment for external requirements.

3. Create the environment variables by hand and export them to the environment before
   starting the OpenPegasus build.

4. Use pre-existing files in the OpenPegasus/root directory to set up particular
   environments for each platform type. These were developed by the original
   OpenPegasus developers and are named with the suffix **.status**. Thus, for example
   the proposed configuration file for linux is **env_var_Linux.status**.

## Building OpenPegasus

Once the configuration variables that will control the compile (and possibly the
setup and test of the build) are defined the build is executed by executing the
makefile.  There are a number of options in the Makefile itself for how OpenPegasus is
built, installed and tested including the capability to:

* Execute the complete make ( ex. ``` >$ make world```). The world make will
  1. Compile and link all of the OpenPegasus source including the Server,
     clients, test programs, and test providers
  2. Configure a test environment with a test CIM repository, certificates,
     and test provider registration in PEGASUS_HOME. This will include
     multiple namespaces including an interop namespace.
  3. Test the complete OpenPegasus server, clients, etc. Using the tests 
     defined in the OpenPegasus code. 
* Execute creation of components such as the repository and tests and
  execution of unit and system level tests (ex. make repository, make testrepository).  
* Remove the repository, and other build components (make clobber, etc.)

## OpenPegasus Makefile build structure

OpenPegaus uses a hierarchy of make files starting with Makefile in the
pegasus directory and utility make files in  the pegasus pegasus/mak.  There
is a Makefile for each directory in the OpenPegasus source code
structure (**Makefile**) defining the make for compile, test, and
cleanup of the OpenPegasus components in that directory.  The top level
makefile recurses through all the directories in OpenPegasus
executing these individual makefiles.

Thus, for example, with the environment variables set simply executing
the command

   $ > make world

executes the complete compile and test of OpenPegasus using the
currently defined CIM MOF repository (environment variable
**PEGASUS_CIM_SCHEMA**), the providers defined for the test environment
and the MOF defined to install and use those  test providers.

NOTE: the build process issues a significant number of warning messages
primarily tied to class-memaccess, deprecated-copy, and deprecated-declarations
depending on the version of the compiler used.

## Testing OpenPegasus

OpenPegasus includes tests for all of the server,  public APIs,
client infrastructure, client applications
components of the environment, both for unit tests and for end-end
tests.  The test components for test of each of the OpenPegasus
components is in a subdirectory name tests for the component. Thus the
tests for The OpenPegasus server common component
'pegasus/src/Pegasus/Common' is in the directory
(pegasus/src/Pegasus/Common/tests)

In addition to the unit tests, OpenPegasus includes a number of test
providers using both of the OpenPegasus provider interfaces (C++ and
CMPI) that are built and installed in the OpenPegasus run environment
when tests are to be executed (ex. Make world).   These providers also
are a .examples of OpenPegasus providers for both of these interfaces.
They are located in the directory pegasus/src/providers.

The test suite is based on a particular DMTF CIM Schema
version ( DMTF CIM Schema Version 2.41.0) which is the default schema built into the
OpenPegasus namespaces (Except for the interop and internal
namespaces).  Use of other versions of the DMTF CIM Schema may cause
issued for these tests becasue of changes to the model.

Since the Interop Internal and other test namespaces create specific
providers they are dependent on specific definitions of classes and
qualifiers.
