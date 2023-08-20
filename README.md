# OpenPegasus CIM/WBEM Implementation

This repository contains the OpenPegasus source code starting with OpenPegasus
version 2.14.1.

This github project was cloned from the OpenPegasus work Ope (see:
[https://OpenPegasus.org](https://OpenPegasus.org). The goal of this github
repository is to continue any required development/fixes to OpenPegasus and
also to make OpenPegasus available through github (the OpenPegasus repository
at https://OpenPegasus.org is maintained in CVS.)

The initial release of OpenPegasus in this github project was OpenPegasus
version 2.14.2 based on the OpenPegasus 2.14.1 release available from
the OpenPegasus web site with changes documented in the the changes document.

The OpenPegasus project at OpenPegaus.org is dormant and changes
to the code base in OpenPegasus CVS may never be made again since the 2.14.1 release.

## OpenPegasus Overview

OpenPegasus is an implementation using largely C++ code of all of the significant
of the CIM/WBEM components defined by the DMTF including:

* CIM/WBEM server and object manager.
* Interface for WBEM providers (C++ and CMPI interface)
* WBEM server indication listener to receive async indications from a WBEM server
* WBEM Server CIM repository in which CIM classes and instances are stored and
  persisted.
* WBEM Client infrastructure that provides a C++ Interface for writing client
  applications for CIM/WBEM.
* Some of the common DMTF Schemas, and profiles specifically tuned to be built
  into OpenPegasus
* A complete build environment in which OpenPegasus can be compiled and tested.
* Unit and systems for all components of OpenPegasus.
* OpenPegasus project and product documentation.
* A number of CLI clients provide for managing OpenPegasus including
  * Controlling runtime variables (cimconfig)
  * Managing the SSL cyrptographic keys (cimtrust and cimcrl)
  * Managing indication subscriptions (cimsub)
* A WEB interface for viewing the status of a running OpenPegasus server and
  managing runtime configuration.
* A command WBEM client (cimcli) for inspecting and executing WBEM requests
  on a running WBEM server.  This client includes some commands specific to
  OpenPegasus.

## OpenPegasus Availability:

The source code is available from this github repository project: OpenPegasus
at:

    https://github.com/OpenPegasus/OpenPegasus.git

The github repository defines `OpenPegasus` as the root directory and
this new `OpenPegasus` directory includes the original `pegasus` directory
(source code, support code, documentation, CIM Schemas, etc.) which was
the root directory in the CVS (ie. OpenPegasus version 2.14.1 and before)) and a
new `OpenPegasus\docs` directory where  new documentation in markdown and
rst is contained.

## Limitations:

There are several limitations to the OpenPegasus code base in github:

1. Releases since 2.14.1 and in  this github repository have not been tested
against the full set of platforms for which OpenPegasus was designed.
Previously releases were tested by the individual OpenPegasus teams on their
own platforms (IBM AIX, HPUX, etc.). The current development team no longer has
access to these platforms. As of 2.14.2 tests are run against the Linux and
Windows platforms.

2. We do not expect to make significant new changes to OpenPegasus but to
keep it up to date with changing compiler and platform technologies,
fix documented issues and to assure that the released code does run on
the platforms available to us.

## Changes made for the github platform

The only significant change made for the github platform was to create a new
top level directory (OpenPegasus) in which the original CVS top level
directory (pegasus) and a new docs directory reside.

## OpenPegasus versions in github repository

See [docs/changes.md](docs/changes.md) and the release documents in the
`OpenPegasus/docs` directory for information on releases maintained in this
github repository.

## Building, Testing, and Running OpenPegasus

### Setting OpenPegasus Build Configuration
OpenPegasus configuration is based entirely on environment variables to define the
location of the source code, the build output directory,  the compile characteristics,
and the OpenPegasus configuration options In general all OpenPegasus configuration
environment variables start with 'PEGASUS_'.

The OpenPegasus build is configurable through a set environment variables.
These variables define OpenPegasus characteristics such as:

- Build type; debug, production
- Definition of the compile hardware target, OS, and compiler
- Location of the source code
- Build output directory
- Compile characteristics for the defined compilers.
- The OpenPegasus functionality and test functionality to be built.

In general, all OpenPegasus environment variables start with 'PEGASUS_'.

The following three environment variables are absolutely required to compile OpenPegaus:

1. **PEGASUS_PLATFORM** Defines the platform for which OpenPegasus is being compiled.
   value defines the Hardware, OS, and compiler for the build (ex. LINUX_X86_64_GNU).
   The allowed configurations is defined by make files in OpenPegaus/pegasus/mak
   (ex. platform_LINUX_X86_64_GNU.mak) and the
   list can be viewed with the make file OpenPegasus/pegasus/mak/
2. **PEGASUS_ROOT** - Defines the `pegasus` directory the top of the pegasus source
   tree. This directory contains the top level build make file (`Makefile`) for OpenPegasus and the
   subdirectories that define the OpenPegasus components (src, docs, schemas, etc.)
   below the src directory the Open Pegasus source components are defined
   (ex. Pegasus, Clients, etc.).
3. **PEGASUS_HOME** - Target directory for compile output.  If this folder does
   not exist, it will be created as part of the make.  This folder is typically
   called build_output and placed at the same level as the `pegasus` folder so
   as to separate build products from the source code. For example:

    # a setup defined in the OpenPegasus directory
    export ROOT=$PWD
    export PEGASUS_ROOT=$ROOT/pegasus
    export PEGASUS_HOME=$ROOT/home
    export PEGASUS_PLATFORM=LINUX_X86_64_GNU

A number of other environment variables may be defined to control compilation of the
various options/features that are used in the OpenPegasus build.  These environment
variables define the various build options such as:

1. including SSL,
2. compiling in debug mode
3. enabling/disabling function options such as the provider user context
4. the audit logger
5. the CIM repository format
6. the query languages enabled.

These options are documented in the OpenPegasus document
[docs/BuildAndReleaseOptions.md](docs/BuildAndReleaseOptions.md). That document is
the most complete reference to all of the OpenPegasus build and test
options and their environment variables. All compile and test options
for OpenPegasus are defined in this file as of OpenPegasus 2.14.3.

### OpenPegasus Build and Test

The make file (`Makefile`) in the `pegasus` directory is the basis for building
and testing OpenPegasus.

It contains specific targets for compiling OpenPegasus and executing the test suite;
which is part of the OpenPegasus source code.

The simplest OpenPegasus build and test is `make world` which does:

* Compiles and links of OpenPegasus and the test tools using the current environment
  variables
* Builds a run environment including creating a class repository, cryptographic
  certificates and keys, builds and installs test providers,
* runs the full test suite based on the build variables.

Other make targets allow cleaning up the build residue, rebuilding the repository,
running tests, etc.  See `Make usage` for help on the make targets.

For more information on building OpenPegasus see
[docs/BuildingPegasusReadme.md](docs/BuildingOpenPegasusReadme.md)

### Running OpenPegasus

The OpenPegasus server is initiated from the command line ( `$< cimserver` ) and includes
the capability to have runtime configuration controlled through runtime parameters.  The runtime
variables are in turn settable and viewable from the OpenPegasus command line utility `cimconfig`.
and provide control over features including:

* Enabling server/indication listener network connections and authorization
  and setting connection parameters
* Defining server logging parameters and log destination
* Locating directories for specific runtime features
* Setting SSL characteristics
* Setting privilege mode access to the server
* Setting the modes for provider access
* Defining the CIM Repository storage mode
* Enabling trace of the server and setting trace parameters
* Execution mode (daemon or in place)

The document [docs/BuildAndReleaseOptions.md](docs/docs/BuildAndReleaseOptions.md)
defines these runtime configuration parameters. The can be set with the
OpenPegasus command line tool `cimconfig`.

To run OpenPegasus the path must be updated to include the path the
the executables as defined in
[docs/RunningPegasusReadme.md](docs/RunningOpenPegasusReadme.md)

For more information on configuring the runtime parameters and environment and
running the OpenPegasus server see:
[docs/RunningPegasusReadme.md](docs/RunningOpenPegasusReadme.md)

## OpenPegasus versions in this repository

See [docs/changes.md](docs/changes.md)

## OpenPegasus Documentation

OpenPegasus documentation is spread over a number of locations and
formats,  and includes both design documentation in the form of PEPS, general design
guidance, and specific implementation documentation and a number of specific
notes on particular subjects.

1. OpenPegasus PEPS (Enhancement proposals) - These are proposal documents
   written during the development of OpenPegasus by the OpenPegasus development
   team.  These documents cover all aspects of the implementation of
   OpenPegasus including architecture, design of specific components, APIs,
   release documentation, and other implementation and usage information. They
   are located at:
   https://collaboration.opengroup.org/pegasus/documents.php?action=list&dcat=
2. OpenPegasus bugs - Originally bug  documentation was maintained in the OpenGroup
   OpenPegasus web at https://http://bugzilla.openpegasus.org/. Bugs are no longer being
   added to that repository.
3. Bugs since OpenPegasus version 2.14.1 and the creation of the Github repository for OpenPegasus
   at https://github.com/OpenPegasus/OpenPegasus/issues
4. Specific OpenPegasus PEPS and other documentation that are of general use are distributed
   with the OpenPegasus source code in the directory (pegasus/doc/OpenPegaus/pegasus/doc).
5. Subsequent to creating the github repository an additional documentation directory
   has been created (OpenPegasus/docs) that includes documents created subsequent to
   the creation of the github project.
5. readme... text documents and other documents maintained in the
   `OpenPegasus/pegasus` directory. These were largely documents on
   particular subjects that it was felt might be important to the developers
   and users of OpenPegasus.

## Other CIM/WBEM Projects:

1. pywbem/pywbemtools (see github/pywbem project) is a Python based implementation of the
CIM/WBEM client and CIM/WBEM indication listener. This project is being maintained by
some of the same time maintaining the OpenPegasus github project and uses OpenPegasus as
one of the test vehicles for code developed for pywbem and pywbemtools.

2. CIMPLE: a CIM/WBEM provider writing environment that significantly simplifies writing
providers for OpenPegasus.  While originally developed as a separate project isnow
being moved to the OpenPegasus project github project as (OpenPegasus/CIMPLE).

## Other OpenPegasus Projects

3. OpenPegasus container creation tools.  This is a set of scripts and make files that
allow building Docker images for OpenPegasus.  They are being defined in the github
project OpenPegasus/OpenPegasusDocker: https://github.com/OpenPegasus/OpenPegasusDocker

## Other CIM/WBEM Documentation

1. DMTF CIM/WBEM specifications - https://www.dmtf.org/standards/published_documents

2. DMTF WBEM specifications - https://www.dmtf.org/standards/wbem

3. SNIA Storage Management Initiative Specification - https://www.snia.org/forums/smi/tech_programs/smis_home

## Other OpenPegasus Documentation

1. OpenPegasus Web site - https://collaboration.opengroup.org/pegasus/

2. OpenPegasus Presentations - https://collaboration.opengroup.org/pegasus/documents.php?action=page&offset=100&limit=100&archived=N&grouped=N&sdir=DESC&sort=gdc_updated&gdc_category=0&gdc_docage=0&allver=N&gdc_status=0&oldcat=0&dcat=&oldarch=N

3. OpenPegasus APIs - http://cvs.opengroup.org/pegasus-doc/

## OpenPegasus license
   OpenPegasus has been and will be maintained under the license defined as
   follows which is the MIT license with Copyright definitions:

    OpenPegasus is subject to one or more of the following copyrights:

        Copyright (c) 2000, 2004 BMC Software.
        Copyright (c) 2003, 2008 EMC Corporation.
        Copyright (c) 2000-2012 Hewlett-Packard Development Company, L.P.
        Copyright (c) 2000-2012 IBM Corp.
        Copyright (c) 2005-2012 Inova Development Inc.
        Copyright (c) 2000, 2003 Michael Day.
        Copyright (c) 2008 Novell.
        Copyright (c) 2006, 2007 Symantec Corporation.
        Copyright (c) 2000-2012 The Open Group.
        Copyright (c) 2000, 2002 Tivoli Systems.
        Copyright (c) 2004, 2005 VERITAS Software Corporation.

    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## OpenPegasus Availability:

The source code is available from the github repository project OpenPegasus
at:

    https://github.com/OpenPegasus/OpenPegasus.git

The github repository defines `OpenPegasus` as the root directory and
this new `OpenPegasus` directory includes the original `pegasus` directory
(source code, support code, documentation, CIM Schemas, etc.) which was
the root directory in the CVS (ie. OpenPegasus version 2.14.1 and before)) and a
new `OpenPegasus\docs` directory where  new documentation in markdown and
rst is contained.

## Limitations:

There are several limitations to the OpenPegasus code base in github:

1. Releases since 2.14.1 and in  this github repository have not been tested
against the full set of platforms for which OpenPegasus was designed.
Previously releases were tested by the individual OpenPegasus teams on their
own platforms (IBM AIX, HPUX, etc.). The current development team no longer has
access to these platforms. As of 2.14.2 tests are run against the Linux and
Windows platforms.

2. We do not expect to make significant new changes to OpenPegasus but to
keep it up to date with changing compiler and platform technologies,
fix documented issues and to assure that the released code does run on
the platforms available to us.

## Changes made for the github platform

The only significant change made for the github platform was to create a new
top level directory (OpenPegasus) in which the original CVS top level
directory (pegasus) and a new docs directory reside.

## OpenPegasus versions in github repository

See [docs/changes.md](docs/changes.md) and the release documents in the
`OpenPegasus/docs` directory for information on releases maintained in this
github repository.

## Building, Testing, and Running OpenPegasus

### Setting OpenPegasus Build Configuration
OpenPegasus configuration is based entirely on environment variables to define the
location of the source code, the build output directory,  the compile characteristics,
and the OpenPegasus configuration options In general all OpenPegasus configuration
environment variables start with 'PEGASUS_'.

The OpenPegasus build is configurable through a set environment variables.
These variables define OpenPegasus characteristics such as:

- Build type; debug, production
- Definition of the compile hardware target, OS, and compiler
- Location of the source code
- Build output directory
- Compile characteristics for the defined compilers.
- The OpenPegasus functionality and test functionality to be built.

In general, all OpenPegasus environment variables start with 'PEGASUS_'.

The following three environment variables are absolutely required to compile OpenPegaus:

1. **PEGASUS_PLATFORM** Defines the platform for which OpenPegasus is being compiled.
   value defines the Hardware, OS, and compiler for the build (ex. LINUX_X86_64_GNU).
   The allowed configurations is defined by make files in OpenPegaus/pegasus/mak
   (ex. platform_LINUX_X86_64_GNU.mak) and the
   list can be viewed with the make file OpenPegasus/pegasus/mak/
2. **PEGASUS_ROOT** - Defines the `pegasus` directory the top of the pegasus source
   tree. This directory contains the top level build make file (`Makefile`) for OpenPegasus and the
   subdirectories that define the OpenPegasus components (src, docs, schemas, etc.)
   below the src directory the Open Pegasus source components are defined
   (ex. Pegasus, Clients, etc.).
3. **PEGASUS_HOME** - Target directory for compile output.  If this folder does
   not exist, it will be created as part of the make.  This folder is typically
   called build_output and placed at the same level as the `pegasus` folder so
   as to separate build products from the source code. For example:

    # a setup defined in the OpenPegasus directory
    export ROOT=$PWD
    export PEGASUS_ROOT=$ROOT/pegasus
    export PEGASUS_HOME=$ROOT/home
    export PEGASUS_PLATFORM=LINUX_X86_64_GNU

A number of other environment variables may be defined to control compilation of the
various options/features that are used in the OpenPegasus build.  These environment
variables define the various build options such as:

1. including SSL,
2. compiling in debug mode
3. enabling/disabling function options such as the provider user context
4. the audit logger
5. the CIM repository format
6. the query languages enabled.

These options are documented in the OpenPegasus document
[docs/BuildAndReleaseOptions.md](docs/BuildAndReleaseOptions.md). That document is
the most complete reference to all of the OpenPegasus build and test
options and their environment variables. All compile and test options
for OpenPegasus are defined in this file as of OpenPegasus 2.14.3.

### OpenPegasus Build and Test

The make file (`Makefile`) in the `pegasus` directory is the basis for building
and testing OpenPegasus.

It contains specific targets for compiling OpenPegasus and executing the test suite;
which is part of the OpenPegasus source code.

The simplest OpenPegasus build and test is `make world` which does:

* Compiles and links of OpenPegasus and the test tools using the current environment
  variables
* Builds a run environment including creating a class repository, cryptographic
  certificates and keys, builds and installs test providers,
* runs the full test suite based on the build variables.

Other make targets allow cleaning up the build residue, rebuilding the repository,
running tests, etc.  See `Make usage` for help on the make targets.

For more information on building OpenPegasus see
[docs/BuildingPegasusReadme.md](docs/BuildingOpenPegasusReadme.md)

### Running OpenPegasus

The OpenPegasus server is initiated from the command line ( `$< cimserver` ) and includes
the capability to have runtime configuration controlled through runtime parameters.  The runtime
variables are in turn settable and viewable from the OpenPegasus command line utility `cimconfig`.
and provide control over features including:

* Enabling server/indication listener network connections and authorization
  and setting connection parameters
* Defining server logging parameters and log destination
* Locating directories for specific runtime features
* Setting SSL characteristics
* Setting privilege mode access to the server
* Setting the modes for provider access
* Defining the CIM Repository storage mode
* Enabling trace of the server and setting trace parameters
* Execution mode (daemon or in place)

The document [docs/BuildAndReleaseOptions.md](docs/docs/BuildAndReleaseOptions.md)
defines these runtime configuration parameters. The can be set with the
OpenPegasus command line tool `cimconfig`.

To run OpenPegasus the path must be updated to include the path the
the executables as defined in
[docs/RunningPegasusReadme.md](docs/RunningOpenPegasusReadme.md)

For more information on configuring the runtime parameters and environment and
running the OpenPegasus server see:
[docs/RunningPegasusReadme.md](docs/RunningOpenPegasusReadme.md)

## OpenPegasus versions in this repository

See [docs/changes.md](docs/changes.md)

## OpenPegasus Documentation

OpenPegasus documentation is spread over a number of locations and
formats,  and includes both design documentation in the form of PEPS, general design
guidance, and specific implementation documentation and a number of specific
notes on particular subjects.

1. OpenPegasus PEPS (Enhancement proposals) - These are proposal documents
   written during the development of OpenPegasus by the OpenPegasus development
   team.  These documents cover all aspects of the implementation of
   OpenPegasus including architecture, design of specific components, APIs,
   release documentation, and other implementation and usage information. They
   are located at:
   https://collaboration.opengroup.org/pegasus/documents.php?action=list&dcat=
2. OpenPegasus bugs - Originally bug  documentation was maintained in the OpenGroup
   OpenPegasus web at https://http://bugzilla.openpegasus.org/. Bugs are no longer being
   added to that repository.
3. Bugs since OpenPegasus version 2.14.1 and the creation of the Github repository for OpenPegasus
   at https://github.com/OpenPegasus/OpenPegasus/issues
4. Specific OpenPegasus PEPS and other documentation that are of general use are distributed
   with the OpenPegasus source code in the directory (pegasus/doc/OpenPegaus/pegasus/doc).
5. Subsequent to creating the github repository an additional documentation directory
   has been created (OpenPegasus/docs) that includes documents created subsequent to
   the creation of the github project.
5. readme... text documents and other documents maintained in the
   `OpenPegasus/pegasus` directory. These were largely documents on
   particular subjects that it was felt might be important to the developers
   and users of OpenPegasus.

## Other CIM/WBEM Projects:

1. pywbem/pywbemtools (see github/pywbem project) is a Python based implementation of the
CIM/WBEM client and CIM/WBEM indication listener. This project is being maintained by
some of the same time maintaining the OpenPegasus github project and uses OpenPegasus as
one of the test vehicles for code developed for pywbem and pywbemtools.

2. CIMPLE: a CIM/WBEM provider writing environment that significantly simplifies writing
providers for OpenPegasus.  While originally developed as a separate project isnow
being moved to the OpenPegasus project github project as (OpenPegasus/CIMPLE).

## Other OpenPegasus Projects

3. OpenPegasus container creation tools.  This is a set of scripts and make files that
allow building Docker images for OpenPegasus.  They are being defined in the github
project OpenPegasus/OpenPegasusDocker: https://github.com/OpenPegasus/OpenPegasusDocker

## Other CIM/WBEM Documentation

1. DMTF CIM/WBEM specifications - https://www.dmtf.org/standards/published_documents

2. DMTF WBEM specifications - https://www.dmtf.org/standards/wbem

3. SNIA Storage Management Initiative Specification - https://www.snia.org/forums/smi/tech_programs/smis_home

## Other OpenPegasus Documentation

1. OpenPegasus Web site - https://collaboration.opengroup.org/pegasus/

2. OpenPegasus Presentations - https://collaboration.opengroup.org/pegasus/documents.php?action=page&offset=100&limit=100&archived=N&grouped=N&sdir=DESC&sort=gdc_updated&gdc_category=0&gdc_docage=0&allver=N&gdc_status=0&oldcat=0&dcat=&oldarch=N

3. OpenPegasus APIs - http://cvs.opengroup.org/pegasus-doc/

## OpenPegasus license
   OpenPegasus has been and will be maintained under the license defined as
   follows which is the MIT license with Copyright definitions:

    OpenPegasus is subject to one or more of the following copyrights:

        Copyright (c) 2000, 2004 BMC Software.
        Copyright (c) 2003, 2008 EMC Corporation.
        Copyright (c) 2000-2012 Hewlett-Packard Development Company, L.P.
        Copyright (c) 2000-2012 IBM Corp.
        Copyright (c) 2005-2012 Inova Development Inc.
        Copyright (c) 2000, 2003 Michael Day.
        Copyright (c) 2008 Novell.
        Copyright (c) 2006, 2007 Symantec Corporation.
        Copyright (c) 2000-2012 The Open Group.
        Copyright (c) 2000, 2002 Tivoli Systems.
        Copyright (c) 2004, 2005 VERITAS Software Corporation.

    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Note that this github repository is OpenPegasus root directory is
OpenPegasus and that directory includes the **pegasus** directory
(source code, support code, documentation, CIM Schemas, etc.) which was
the root directory in CVS) and a new docs directory where over time new
documentation will be acccumulated.

## Limitations:

There are several limitations to the OpenPegasus code base in github:

1. It is not tested against the full set of platforms for which OpenPegasus
was designed and for which the releases up to 2.14 were tested.  This
is because the original test strategy was for each of the OpenPegasus
teams to test OpenPegasus on their own platforms (IBM AIX, HPUX, etc.).
The current developers of the github repository do not have access to
these platforms so we cannot run these tests.  As of 2.14.2 tests are
run against Linux and Windows platforms. 

2. We do not expect to make significant new changes to OpenPegasus but to
keep it up to date with changing compiler and platform technologies,
fix documented issues and to assure that the released code does run on
the platforms available to us.

## Changes made for the github platform

The only significant change made for the github platform was to create a new
top level directory (OpenPegasus) in which the original CVS top level
directory (pegasus) and a new docs directory reside.

## OpenPegasus Versions in Github repository

See [docs/changes.md](docs/changes.md) and the release documents in the
OpenPegasus/docs directory for information on releases maintained in this
github repository. 

## Building, Testing, and Running OpenPegasus

### Setting OpenPegasus Build Configuration
OpenPegasus configuration is based entirely on environment variables to define the
location of the source code, the build output directory,  the compile characteristics,
and the OpenPegasus configuration options In general all OpenPegasus configuration
environment variables start with 'PEGASUS_'.

There are three environment variables that are absolutely required to compile OpenPegaus

1. **PEGASUS_PLATFORM** Defines the platform for which OpenPegasus is being compiled.
2. **PEGASUS_ROOT** - Defines the 'pegasus' directory the top of the OpenPegasus source
   tree.  This directory contains the basic build Makefile for OpenPegasus and the
   subdirectories define the OpenPegasus components (ex. Pegasus, Clients, etc.)
3. **PEGASUS_HOME** - Target directory for compile output.  

A number of other environment variables may be defined to control compilation of the
various options/features that are used in the OpenPegasus build.  These environment
variables define the various build options such as:

1. including SSL,
2. compiling in debug mode
3. enabling/disabling function options such as the provider user context
4. the audit logger
5. the CIM repository format
6. the query languages enabled.

These options are documented in the OpenPegasus document
[docs/BuildAndReleaseOptions.md](docs/BuildAndReleaseOptions.md). That document is
the most complete reference to all of the OpenPegasus build and test
options and their environment variables. All compile and test options
for OpenPegasus are defined in this file as of OpenPegasus 2.14.3.

### OpenPegasus Build and Test

The Makefile in the pegasus directory is the basis for building and testing OpenPegasus.

It contains specific targets for compiling OpenPegasus and executing the test suite;
which is part of the OpenPegasus source code.

The simplest make is **make world** which does a complete build of OpenPegasus and the
test tools, builds a run environment and runs the full test suite.

For more information on building OpenPegasus see
[docs/BuildingPegasusReadme.md](docs/BuildingOpenPegasusReadme.md)

### Running OpenPegasus
The OpenPegasus server is initiated from the command line ( **$< cimserver** ) and includes
the capability to have runtime configuration controlled through runtime variables.  The runtime
variables are in turn settable from the OpenPegasus command line utility **cimconfig**.
and provide control over features including:

* Enabling network connections and setting connection parameters
* Defining logging parameters
* Locating directories for specific features
* Setting SSL characteristics
* Enabling trace of the server and setting trace parameters
* Execution mode (daemon or in place)

For more information on running OpenPegasus see
[docs/RunningPegasusReadme.md](docs/RunningOpenPegasusReadme.md)

## OpenPegasus versions in this repository

See [docs/changes.]

## OpenPegasus Documentation

OpenPegasus documentation is spread over a number of locations and
formats, both design documentation in the form of PEPS, general design
guidance, and specific implementation documentation.

1. OpenPegasus PEPS (Enhancement proposals) - These are proposal documents written during the
   development of OpenPegasus by the
   OpenPegasus development team.  These documents cover all aspects of
   the implementation of OpenPegasus including architecture, design of
   specific components, APIs, release documentation, and other
   implementation and usage information. They are located at
   https://collaboration.opengroup.org/pegasus/documents.php?action=list&dcat=
2. OpenPegasus bugs - Originally bug  documentation was maintained in the OpenGroup
   OpenPegasus web at https://http://bugzilla.openpegasus.org/. Bugs are no longer being
   added to that repository.
3. Bugs since OpenPegasus version 2.14.3 and the creation of the Github repository for OpenPegasus
   at https://github.com/OpenPegasus/OpenPegasus/issues
4. Specific OpenPegasus PEPS and other documentation that are of general use are distributed
   with the OpenPegasus source code in the directory (pegasus/doc/OpenPegaus/pegasus/doc).
5. Subsequent to creating the github repository an additional documentation directory
   has been created (OpenPegasus/docs) that includes documents created subsequent to
   the creation of the github project.
5. Readme... text documents and other documents maintained in the
OpenPegasus/pegasus directory. These were largely documents on
particular subjects that it was felt were important to the developers
and users of OpenPegasus.

## Other CIM/WBEM Projects:

1. pywbem/pywbemtools (see github/pywbem project) is a Python based implementation of the
CIM/WBEM client and CIM/WBEM indication listener. This project is being maintained by
some of the same time maintaining the OpenPegasus github project and uses OpenPegasus as
one of the test vehicles for code developed for pywbem and pywbemtools.

2. CIMPLE: a CIM/WBEM provider writing environment that significantly simplifies writing
providers for OpenPegasus.  While originally developed as a separate project isnow
being moved to the OpenPegasus project github project as (OpenPegasus/CIMPLE).

3. OpenPegasus container ceation tools.  This is a set of scripts and make files that
allow building Docker images for OpenPegasus.

## Other CIM/WBEM Documentation

1. DMTF CIM/WBEM specifications - https://www.dmtf.org/standards/published_documents

2. DMTF WBEM specifications - https://www.dmtf.org/standards/wbem

3. SNIA Storage Management Initiative Specification - https://www.snia.org/forums/smi/tech_programs/smis_home

## Other OpenPegasus Documentation

1. OpenPegasus Web site - https://collaboration.opengroup.org/pegasus/

2. OpenPegasus Presentations - https://collaboration.opengroup.org/pegasus/documents.php?action=page&offset=100&limit=100&archived=N&grouped=N&sdir=DESC&sort=gdc_updated&gdc_category=0&gdc_docage=0&allver=N&gdc_status=0&oldcat=0&dcat=&oldarch=N

3. OpenPegasus APIs - http://cvs.opengroup.org/pegasus-doc/

