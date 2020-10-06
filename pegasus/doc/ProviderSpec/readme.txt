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
Readme.txt -- doc/providerspec directory

This directory is the source of documentation for the Pegasus Provider interface document.
The documentation is created from files in this directory using the doc++ adn gawk
utility systems.  The make file in this directory services as the core file for
documentation creation.

Note that the documentation is created from the files in this directory and the Pegasus
header files that represent the interfaces.  The classes and methods in those files must
be documented using the doc++ comments and tags.

The resulting HTML documentation is saved in the directory specificed for output in
the makefile, normally a subdirectory to PEGASUS_HOME.

The documentation is defined using the tags provided by Doc++ and HTML.

NOTE: The file PegasusSpec.dxx is the top-level file for this documentation.  It defines
the outline of the document and the list of other documentation and header files to be used
to create the document.

Karl Schopmeyer, September 4 2001
