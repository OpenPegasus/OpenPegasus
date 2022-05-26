# OpenPegasus CIM/WBEM Implementation

This repository contains the OpenPegasus source code starting with OpenPegasus 14.1.

This project was cloned from the OpenPegasus work at OpenGroup
(see https://OpenPegasus.org). The goal of this github repository is to continue
any required development/fixes to OpenPegasus and also to make OpenPegasus available
through github (the OpenPegasus repositoryat https://OpenPegaus.org is maintained
in CVS.)

The intial release of OpenPegasus in this github project is OpenPegasus
version 2.14.2 based on the OpenPegasus 2.14.1 release available from
the OpenPegasus web site.

The inital release of this github project corrected a number of issues with
OpenPegasus 2.14.1 including:

1. Compile with current gcc compilers on Linux systems

2. Modify OpenPegasus to be compatible with OpenSSL 1.1.x and the new API.
   It also supports older versions of OpenSSL.

3. Support DMTF experimental schemas and includes DMTF schema 2.41.0 Experimental.

4. Development, continuous integration, and release moved to github.

The OpenPegasus project at OpenPegaus.org is apparently dormant and changes
to the code base in OpenPegasus CVS  may never be made again since the 2.14.1 release.

## OpenPegasus Overview

OpenPegasus is an implementation using largely C++ code of all of the significant
of the CIM/WBEM components define by the DMTF including:

* CIM/WBEM server and object manager.
* Interface for WBEM providers (C++ and CMPI interface)
* WBEM server indication listener to recive async indications from a WBEM server
* WBEM Server CIM repository in which CIM classes and instances are stored
* WBEM Client infrastructure that provides a C++ Interface for writing client
  applications for CIM/WBEM.
* A number of DMTF Schemas specifically tuned to be built into OpenPegasus
* A complete build environment in which OpenPegasus can be compiled and tested.
* A test suite for all of the components of OpenPegasus for unit and system
  tests.
* Documentation for OpenPegasus.

## OpenPegasus license
   OpenPegasus has been and will be maintained under the MIT license.

## OpenPegasus Availability:

The source code is available from the github repository project OpenPegasus
at:

    https://github.com/OpenPegasus/OpenPegasus.git

Note that the clone of OpenPegasus root directory is OpenPegasus and that
directory includes the pegasus directory (source and support code) and a docs
directory where over time most of the documentation will be acccumulated.

A tarball and zip file will be made available.

## Limitations:

There are a number of limitations to the code base in gitub:

1. It does not test against the full set of platfroms for which OpenPegasus
was designed and for which the releases up to 2.14 were tested.  This is because
the orginal test scheme was for each of the OpenPegasus teams to test OpenPegasus
on their own platforms (IBM AIX, HPUX, etc.). The current developers do not have
access to these platforms so we cannot run these tests.  Today tests are run
against Linux and Windows platforms.

2. We do not expect to make significant new changes to OpenPegasus but to
keep it up to date with changing compiler and platform technologies and to assure
that the released code does run on the platforms available to us.

## Changes made for the github platform

The only real change made for the github platform was to create a new top level
directory (OpenPegasus) in which the original CVS top level directory (pegasus) and a new
docs directory reside.

## Building and Testing OpenPegasus

OpenPegasus configuration is based entirely on environment variables to define the
location of the source code, the build output directory,  the compile characteristics,
and the OpenPegasus configuration options In general all OpenPegasus configuration e
nvironment variables start with 'PEGASUS_'.

There are three environment variable that are absolutely required to compile OpenPegaus

1. **PEGASUS_PLATFORM** Defines the platform for which OpenPegasus is being compiled.
2. **PEGASUS_ROOT** - Defines the 'pegasus' directory the top of the OpenPegasus source
   tree.  This directory contains the basic build Makefile for OpenPegasus and the
   subdirectories define the OpenPegasus components (ex. Pegasus, Clients, etc.)
3. **PEGASUS_HOME** - Target directory for compile output.  

A number of other environment variables may be defined to control compiling of the
various options that are used in the OpenPegasus build.  These environment
variables define the various build options such as including SSL, compiling in
debug mode, enabling/disabling function options such as the provider user context,
the autid logger, the CIM repository format, the query languages enabled.

These options are documented in the OpenPegasus document file './pegaus/doc/'BuildAndReleaseOptions.html''
located in the OpenPegaus/pegasus/doc directory of the source code.  All compile and
test options for OpenPegasus are defined in this file as of OpenPegasus 2.14.3.

For more information on building OpenPegasus see the document
'./docs/BuildingOpenPegasusReadme.md'

## OpenPegasus Documentation

Open Pegasus documentation is located in a number of locations, both design documentation in
the form of PEPS, general design guidance, and specific implementation documentation.

1. OpenPegasus PEPS (Enhancement proposals) - This are proposaldocuments written during the
   development of OpenPegasus and either approved or rejected by the OpenPegasus
   development team.  This documents cover all aspects of the implementation of OpenPegasus including
   architecture, APIs, and implementation. They are located at
   https://collaboration.opengroup.org/pegasus/documents.php?action=list&dcat=
2. OpenPegasus bugs.  Originally bug  documentation was maintained in the OpenGroup
   OpenPegasus web at https://http://bugzilla.openpegasus.org/. Bugs are no longer being
   added to this bug repository
3. Bugs since OepnPegasu 2.14.3 and the creation of the Github respository for OpenPegasus
   at https://github.com/OpenPegasus/OpenPegasus/issues
4. Specific OpenPegasus PEPS and other documentation that is of general use is distributed
   with the OpenPegasus source code in the directory OpenPegaus/pegasus/doc.
5. Readme and other documents maintained in the OpenPegasus/pegasud directory. These were
   largely documents on particular subjects that it was felt were important to the
   developers and users of OpenPegasus.

## Other CIM/WBEM Projects:

1. pywbem/pywbemtools (see github/pywbem project) is a Python based implementation of the
CIM/WBEM client and CIM/WBEM indication listener. This project is being maintained by
some of the same time maintaining the OpenPegasus github project and uses OpenPegasus as
one of the test vehicles for code developed for pywbem and pywbemtools
2. CIMPLE: a CIM/WBEM provider writing environment that significantly simplifies writing
providers for OpenPegasus.  While originally developed as a separate project is is now
being moved to the OpenPegasus project github project as (OpenPegasus/CIMPLE)

   









   


   



