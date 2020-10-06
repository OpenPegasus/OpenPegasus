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

DIR = Clients/cimprovider/tests

include $(ROOT)/mak/config.mak
include $(ROOT)/mak/test.mak

RESULTFILE =  $(TMP_DIR)/result
MASTERRESULTFILE = $(ROOT)/src/$(DIR)/result.master

tests:

clean:
	@$(RM) $(RESULTFILE)

poststarttests:
	@$(RM) $(RESULTFILE)
	@$(MAKE) -i -s poststarttests_ignoreerror
	@$(STRIPCRS)
	$(COMPARERESULTS)
	@$(ECHO) +++ Test passed +++
	@$(RM) $(RESULTFILE)

poststarttests_ignoreerror:
	@$(ECHO) "Display usage - error" >> $(RESULTFILE)
	@cimprovider >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display usage - error" >> $(RESULTFILE)
	@cimprovider -d  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@cimprovider -d -m  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@cimprovider -e  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@cimprovider -e -m  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@cimprovider -r >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@cimprovider -r -m >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Duplicate -d parameter" >> $(RESULTFILE)
	@cimprovider -d -m test -d -m test2  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Duplicate -e parameter" >> $(RESULTFILE)
	@cimprovider -e -m test -e -m test2  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Duplicate -r parameter" >> $(RESULTFILE)
	@cimprovider -r -m test -r -m test2  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Duplicate -l parameter" >> $(RESULTFILE)
	@cimprovider -l -l >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Duplicate -s parameter" >> $(RESULTFILE)
	@cimprovider -l -s -s >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Unexpected option" >> $(RESULTFILE)
	@cimprovider -d -l -m test >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Unexpected option" >> $(RESULTFILE)
	@cimprovider -e -l -m test >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Unexpected option" >> $(RESULTFILE)
	@cimprovider -r -l -m test >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Unexpected option" >> $(RESULTFILE)
	@cimprovider -l -r >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Unexpected option" >> $(RESULTFILE)
	@cimprovider -d -p test>> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Unexpected option" >> $(RESULTFILE)
	@cimprovider -l -s -m test>> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "argument was unexpected" >> $(RESULTFILE)
	@cimprovider -l test>> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "argument was unexpected" >> $(RESULTFILE)
	@cimprovider -l -s test>> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Disable provider module" >> $(RESULTFILE)
	@cimprovider -d -m ComputerSystemModule>> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	@$(ECHO) "Disable the provider module again" >> $(RESULTFILE)
	@cimprovider -d -m ComputerSystemModule>> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	@$(ECHO) "Enable provider module" >> $(RESULTFILE)
	@cimprovider -e -m ComputerSystemModule>> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	@$(ECHO) "Enable the provider module again" >> $(RESULTFILE)
	@cimprovider -e -m ComputerSystemModule>> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

depend:

general:

strip-license:

prepend-license:
