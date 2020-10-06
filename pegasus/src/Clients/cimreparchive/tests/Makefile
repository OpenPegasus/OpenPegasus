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
ROOT = ../../../..

DIR = Clients/cimreparchive/tests

include $(ROOT)/mak/config.mak
include $(ROOT)/mak/test.mak

RESULTFILE =  $(TMP_DIR)/result
IGNOREFILE =  $(TMP_DIR)/ignore
MASTERRESULTFILE = $(ROOT)/src/$(DIR)/result.master

tests:

clean:
	@$(RM) $(RESULTFILE)

poststarttests:
	@$(RM) $(RESULTFILE)
	@ # Test error cases
	@$(MAKE) -i -s poststarttests_ignoreerror
	@ # Test success case (prints an environment-specific message)
	@$(ECHO) "Success" >> $(RESULTFILE)
	@cimreparchive rep.tar > $(IGNOREFILE)
	@$(RM) $(IGNOREFILE)
	@$(RM) rep.tar
	@ # Check the results
	@$(STRIPCRS)
	$(COMPARERESULTS)
	@$(ECHO) +++ Test passed +++
	@$(RM) $(RESULTFILE)

poststarttests_ignoreerror:
	@$(ECHO) "Display usage - error" >> $(RESULTFILE)
	@cimreparchive >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display usage - error" >> $(RESULTFILE)
	@cimreparchive -a >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@cimreparchive a b >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@cimreparchive --help a >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@cimreparchive --bogus >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Duplicate --help option" >> $(RESULTFILE)
	@cimreparchive --help --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Duplicate --version option" >> $(RESULTFILE)
	@cimreparchive --version --version >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Show help" >> $(RESULTFILE)
	@cimreparchive --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Show version" >> $(RESULTFILE)
	@cimreparchive --version >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Invalid archive file" >> $(RESULTFILE)
	# The message printed by this test case is platform-specific
	@cimreparchive a/b/c.tar > $(IGNOREFILE) $(REDIRECTERROR)
	@$(RM) $(IGNOREFILE)

depend:

general:

strip-license:

prepend-license:
