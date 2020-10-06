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
ROOT = ../../..

DIR = Clients/cimcli
include $(ROOT)/mak/config.mak

CIMCLI = @cimcli

LIBRARIES = \
    pegclient \
    peggeneral \
    pegcommon \
    pegcliutils

LOCAL_DEFINES = -DPEGASUS_CLI_INTERNAL -DPEGASUS_INTERNALONLY

EXTRA_INCLUDES = $(SYS_INCLUDES)

PROGRAM = cimcli

SOURCES = CIMCLICommand.cpp \
	  CIMCLIClient.cpp  \
	  ObjectBuilder.cpp \
	  CIMCLIOutput.cpp \
	  CIMCLIHelp.cpp \
	  CIMCLIOptions.cpp \
	  CIMCLICommon.cpp \
	  CIMCLIOptionStruct.cpp \
	  CIMCLIOperations.cpp

include $(ROOT)/mak/program.mak

tests:

poststarttests:

## generate the message bundle text from the cimcli source files.
## it is the developers responsibility to generate this file each time
## the message bundle definition in the source changes and putting the
## output into the correct message bundle.
buildMsgBundle:
	awk -f buildMsgStrings.awk $(SOURCES)

## When the message bundle is correct (ile buildMsgBundle produces no errors)
## this target installs the new bundle into into the CLI msg bundle in msg
## directory.
## Creates new message bundle by merging the created local bundle and the
## existing CLI message bundle and puts it into the msg/CLI directory
## also copies the original msg bundle file here so it can be compared
## with the new bundle file for any issues.
## NOTE: The keywords CIMCLIAUTOMEDMESSAGEBUNDLE_END and 
##     CIMCLIAUTOMEDMESSAGEBUNDLE_BEGIN must exist in the message bundle for
##     this to work. They are set into the local bundle by buildMsgBundle
##     and should be part of the msg/CLI bundle.
##     TODO check the bundle for these keywords.  
installMsgBundle:
	cp $(ROOT)/src/Pegasus/msg/CLI/pegasusCLI_en.txt pegasusCLI_en.txtOld
	sed '1,/CIMCLIAUTOMEDMESSAGEBUNDLE_END/d' $(ROOT)/src/Pegasus/msg/CLI/pegasusCLI_en.txt > tail.txt
	sed '/CIMCLIAUTOMEDMESSAGEBUNDLE_BEGIN/,99999d' $(ROOT)/src/Pegasus/msg/CLI/pegasusCLI_en.txt > head.txt
	cat  head.txt cimcliCLI_en.txt tail.txt >pegasusCLI_en.txt
	cp pegasusCLI_en.txt $(PEGASUS_ROOT)/src/Pegasus/msg/CLI/pegasusCLI_en.txt
	rm tail.txt head.txt cimcliCLI_en.txt
	echo Compare local files pegasusCLI_en.txt and pegasusCLI_en.txtOld
	echo New file installed in msg/CLI directory. test with make messages.


#############################################################################
##Setup for manual tests.

PROVNSOPT = -n $(PROVIDERNS)
EMBED_CLASS_DEF = -n $(PROVIDERNS) Test_CLITestEmbeddedClass
ASSOC_CLASS_DEF = -n $(PROVIDERNS) Test_CLITestProviderLinkClass

