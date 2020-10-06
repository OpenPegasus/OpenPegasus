#//%LICENSE////////////////////////////////////////////////////////////////
#//
#// Licensed to The Open Group (TOG) under one or more contributor license
#// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
#// this work for additional information regarding copyright ownership.
#// Each contributor licenses this file to you under the OpenPegasus Open
#// Source License; you may not use this file except in compliance with the
#// License.
#//
#// Permission is hereby granted, free of charge, to any person obtaining a
#// copy of this software and associated documentation files (the "Software"),
#// to deal in the Software without restriction, including without limitation
#// the rights to use, copy, modify, merge, publish, distribute, sublicense,
#// and/or sell copies of the Software, and to permit persons to whom the
#// Software is furnished to do so, subject to the following conditions:
#//
#// The above copyright notice and this permission notice shall be included
#// in all copies or substantial portions of the Software.
#//
#// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
#// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
#// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
#// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#//
#//////////////////////////////////////////////////////////////////////////

# Make functions to manage the copywrite license on Pegasus files. This is normally
# used to perform the regular file license text changes that are required by Pegasus.
#
# Includes targets to replace license text each year.
#
# The functions in this make file MAY NOT be portable to other Operating systems
# than Linux since its usage is limited and based on the bash shell syntax.
# Usage. See each target.
# Note that right now this make file can be used in conjuction with the script
# licensechange to change the majority of licenes in a single operation.

MAKEFILELIST="*Makefile Makefile.* *.mak BuildMakefile Makefile* Makefile*.*

licenseupdate:
	@ - bash $(ROOT)/mak/licensechange

# create the license for make files corresponding to the license for .cpp.
# Prepends # to each line of the license.
create-make-license:
	sed 's_//_#//_' $(PEGASUS_ROOT)/doc/license.txt >$(PEGASUS_ROOT)/doc/license.mak.txt

# create the license for lexfiles files corresponding to the license for .cpp.
# Surrounds the existing license with /& */ comments because Lex cannot handle the
# // comments at the beginning of the .l file
# This script prepends the first line with /* and postpends the last line with */
create-lex-license:
	cp $(PEGASUS_ROOT)/doc/license.txt $(PEGASUS_ROOT)/doc/tmp1 | sed -e '1 s_//_/*//_' -e '$$ s_/*$$_&*/_' >$(PEGASUS_ROOT)/doc/license.lex.txt

# Strip the license text from each file in the current directory
# removes the license text (all lines starting with the line containing
# //% and ending with the line  with "//="
strip-license:
	mu strip //% //= *.h *.cpp
	mu strip \#//% \#//= $(MAKEFILELIST)

# Prepend the license file to each file in the current directory
# used on any file with comments that begin with //
prepend-license:
	mu prepend $(ROOT)/doc/license.txt *.h *.cpp
	mu prepend $(ROOT)/doc/license.mak.txt $(MAKEFILELIST)

# Update the license for a single file. Can be used on any file
# that requires comments beginning with //.  This will either replace
# existing license or install new license text
fix-code-license:
	mu strip //% //= $(FILENAME)
	mu prepend $(PEGASUS_ROOT)/doc/license.txt $(FILENAME)

# Update the license text for a single file using the makefile license
# Can be used on any file that requires comments beginning with #.
# Will either replace or install new license text.
fix-make-license:
	mu strip \#//% \#//= $(FILENAME)
	mu prepend $(PEGASUS_ROOT)/doc/license.mak.txt $(FILENAME)

# Update the license text for a single file using the makefile license
# Can be used on any file that requires comments beginning with #.
# Will either replace or install new license text.
fix-lex-license:
	mu strip \#//% \#//= $(FILENAME)
	mu prepend $(PEGASUS_ROOT)/doc/license.lex.txt $(FILENAME)

# Show files with old license
# Update with year of previous license (i.e. 2005)
show-old-licenses:
	grep -r \/%2005\/

# Show Files with NO license
nolicense:
	grep -r -L %2006 .| grep -v CVS |grep -v ".log" | grep -v ".master" | grep -v ".cert" | grep -v ".xml" | grep -v ".key" | grep -v Unsupported | grep -v InterfaceArchive | grep -v Schema
