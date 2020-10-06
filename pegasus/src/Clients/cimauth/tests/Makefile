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

DIR = Clients/cimauth/tests

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
	@cimauth >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display usage - error" >> $(RESULTFILE)
	@cimauth -a  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display usage - error" >> $(RESULTFILE)
	@cimauth -m  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@cimauth -r  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@cimauth -a -m  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Display Usage - error" >> $(RESULTFILE)
	@cimauth -a -r  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Duplicate -a parameter" >> $(RESULTFILE)
	@cimauth -a -u www -n root/cimv2 -a -u xp  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Add auth for user not existing on system - error" >> $(RESULTFILE)
	@cimauth -a -u xp -n test -R  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Add CIM user www" >> $(RESULTFILE)
	@cimuser -a -u www -w www  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Add auth for user www on root/cimxyz2 - error" >> $(RESULTFILE)
	@cimauth -a -u www -n root/cimxyz2 -R  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Add auth for user www on root/cimv2" >> $(RESULTFILE)
	@cimauth -a -u www -n root/cimv2 -R  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Modify auth for user www on root/cimv2" >> $(RESULTFILE)
	@cimauth -m -u www -n root/cimv2 -R -W  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Add auth again for user www on root/cimv2 - error" >> $(RESULTFILE)
	@cimauth -a -u www -n root/cimv2 -R  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Modify wrong user name - error" >> $(RESULTFILE)
	@cimauth -m -u xuztest1 -n root/cimv2 -R >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Remove user xp - error" >> $(RESULTFILE)
	@cimauth -r -u xp -n root/cimv2 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Remove auth for user www on root/cimv2" >> $(RESULTFILE)
	@cimauth -r -u www -n root/cimv2  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Remove all auth for user www" >> $(RESULTFILE)
	@cimauth -r -u www  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Remove CIM user www" >> $(RESULTFILE)
	@cimuser -r -u www  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) "Test all Long options" >> $(RESULTFILE)
	@cimuser --help >> $(RESULTFILE) $(REDIRECTERROR)
	@cimuser --version >> $(RESULTFILE) $(REDIRECTERROR)


depend:
