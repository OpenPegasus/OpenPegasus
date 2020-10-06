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
# wbemexec tests for ManagedSystem NIS provider

ROOT = ../../../../..
include $(ROOT)/mak/config.mak
include $(ROOT)/mak/test.mak

HOSTNAME =
PORT = -p 5988
HTTPMETHOD = -m M-POST
HTTPVERSION = -v 1.1
USER = -u $(PEGASUS_TEST_USER_ID)
PASSWORD = -w $(PEGASUS_TEST_USER_PASS)

XMLREQUESTS = $(foreach i, $(XMLSCRIPTS), $i.xml)
XMLRESPONSES = $(XMLREQUESTS:.xml=.rsp)

WBEMEXECOPTIONS = $(HOSTNAME) $(PORT) $(HTTPMETHOD) $(HTTPVERSION) $(USER) $(PASSWORD)

%.rsp: %.xml
	@ wbemexec $(WBEMEXECOPTIONS) $*.xml > $(TMP_DIR)/$*.rsp || cd .
	@ $(DIFF) $*rspgood.xml $(TMP_DIR)/$*.rsp
	@ $(RM) $(TMP_DIR)/$*.rsp
	@ $(ECHO) +++ $* passed successfully +++

XMLCOVREQUESTS = $(foreach i, $(XMLCOVSCRIPTS), $i.xml)
XMLCOVRESPONSES = $(XMLCOVREQUESTS:.xml=.cov)

%.cov: %.xml
	@ wbemexec $(WBEMEXECOPTIONS) $*.xml > $(TMP_DIR)/$*.cov || cd .
	@ $(RM) $(TMP_DIR)/$*.cov
	@ $(ECHO) +++ $* code coverage exercise complete +++

ifeq ($(OS), HPUX)
XMLSCRIPTS = \
	PG_NIS_Fail1_CreateInstance \
	PG_NIS_Fail2_DeleteInstance \
	PG_NIS_Fail3_ModifyInstance \
	PG_NIS_Fail4_GetInstance_ShortKeys \
	PG_NIS_Fail5_GetInstance_ExtraKeys \
	PG_NIS_Fail6_GetInstance_DupKeys
XMLCOVSCRIPTS =

else
ifeq ($(OS_TYPE),windows)
XMLSCRIPTS =
XMLCOVSCRIPTS =
else
ifeq ($(PEGASUS_PLATFORM), LINUX_IA64_GNU)
XMLSCRIPTS =
XMLCOVSCRIPTS =
else
XMLSCRIPTS =
XMLCOVSCRIPTS =
endif
endif
endif

clean:
	$(RM) $(XMLRESPONSES)


tests:

poststarttests: $(XMLRESPONSES) $(XMLCOVRESPONSES)

depend:

messages:

general:

strip-license:

prepend-license:

