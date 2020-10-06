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

## This makefile provides a general container for tests of invalid MOF inpt
## to the compiler.  The tests in this mof should, in general fail with
## some form of compiler error message.
## There may be some tests that will generate a warning message and continue.
## The tests each consist of inputting MOF that includes a syntax or
## symantec error that the compiler should catch

ROOT = ../../../../..
DIR = Pegasus/Compiler/tests/invalidMOF

include $(ROOT)/mak/config.mak
include $(ROOT)/mak/test.mak

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY

REPOSITORYDIR = $(TMP_DIR)
RESULTFILE = $(TMP_DIR)/result
RESULTOUTPUT = $(RESULTFILE) $(REDIRECTERROR)

RESULTMASTERFILE = $(ROOT)/src/$(DIR)/result.master

NAMESPACE = -ntest

## test addition of selected qualifierDecls. Note that this is not a complete
## qualifierDecl test today but largely for fixes in bug 133 and a number
## of errors in parsing the qualifierDecl
tests: clean
	@$(MAKE) -i -s qualiferDeclInvalidTests
	@$(MAKE) -i -s instAliasInvalidTests
	@$(MAKE) -i -s pragmaInvalidTests
	@$(MAKE) -i -s qualifierDefInvalidTests
	@$(MAKE) -i -s invalidClassDefinitionTests

	@ $(DIFF) $(RESULTFILE) $(RESULTMASTERFILE)
	@ $(ECHO) +++ invalidMOF tests passed successfully +++
	@$(MAKE) clean

##  Error tests. One mof file for each syntax error test. 1 - 9 are
##  Flavor error. 10-16 are Scope errors.
qualiferDeclInvalidTests:
	@ $(ECHO) qualiferDeclInvalidTests > $(RESULTFILE)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDecl1.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDecl2.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDecl3.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDecl4.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDecl5.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDecl6.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDecl10.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDecl11.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDecl12.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDecl13.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDecl14.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDecl15.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDecl16.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDecl20.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ $(ECHO) +++ qualifierDeclInvalid error tests executed successfully +++

instAliasInvalidTests:
	@ $(ECHO) instAliasInvalidTests >> $(RESULTFILE)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    InstAliasError.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    InstAliasError2.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    InstAliasError3.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ $(ECHO) +++ instAliasInvalid error tests executed successfully +++

pragmaInvalidTests:
	@ $(ECHO) pragmaInvalidTests >> $(RESULTFILE)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    InvalidPragma1.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    InvalidPragma2.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ $(ECHO) +++ pragmaInvalid error tests executed successfully +++

## errors in the definition of qualifiers.
qualifierDefInvalidTests:
	@ $(ECHO) qualifierDefInvalidTests >> $(RESULTFILE)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badQualifierDef1.mof >> $(RESULTFILE) $(REDIRECTERROR)
	@ $(ECHO) +++ qualifierDefInvalid error tests executed successfully +++

invalidClassDefinitionTests:
	@ $(ECHO) invalidClassDefinitionTests >> $(RESULTFILE)
	@ cimmofl "-W" "-nroot" "-R$(REPOSITORYDIR)" "-M$(REPOSITORY_MODE)" \
	    badClassDefinition1.mof
	@ $(ECHO) +++ invalidClassDefinition error tests executed successfully +++

poststarttests:

clean:
	@$(RM) $(RESULTFILE)
	@$(RMREPOSITORY) $(REPOSITORYDIR)/repository

depend:

general:

poststarttests:
