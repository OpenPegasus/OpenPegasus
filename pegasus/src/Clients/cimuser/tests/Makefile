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

DIR = Clients/cimuser/tests

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

poststarttests_ignoreerror:
	@$(ECHO) "Display usage - error" >> $(RESULTFILE)
	@cimuser >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@cimuser -a -m  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@cimuser -a -r  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Remove user lp" >> $(RESULTFILE)
	@cimuser -r -u lp  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Add user lp" >> $(RESULTFILE)
	@cimuser -a -u lp -w test  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Modify correct password" >> $(RESULTFILE)
	@cimuser -m  -u lp -w test -n test1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Add user lp again - error" >> $(RESULTFILE)
	@cimuser -a -u lp -w test  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Add a user not existing on system - error" >> $(RESULTFILE)
	@cimuser -a -u lp -w test  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display usage - error" >> $(RESULTFILE)
	@cimuser -a  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display usage - error" >> $(RESULTFILE)
	@cimuser -m  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Modify wrong user name - error" >> $(RESULTFILE)
	@cimuser -m  -u test1 -w test -n test1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display usage - error" >> $(RESULTFILE)
	@cimuser -r  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display usage - error" >> $(RESULTFILE)
	@cimuser -r -u  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Remove user lp" >> $(RESULTFILE)
	@cimuser -r -u lp >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Remove user test1 - error"  >> $(RESULTFILE)
	@cimuser -r -u test1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Remove user lp again" >> $(RESULTFILE)
	@cimuser -r -u lp >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Duplicate -a parameter" >> $(RESULTFILE)
	@cimuser -a -u lp -w lp -a -u lp -w lp >> $(RESULTFILE) $(REDIRECTERROR)

depend:
