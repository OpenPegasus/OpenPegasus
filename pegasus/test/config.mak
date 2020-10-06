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
include $(ROOT)/mak/test.mak

HOSTNAME =
PORT =
HTTPMETHOD = -m M-POST
HTTPVERSION = -v 1.1
USER = -u $(PEGASUS_TEST_USER_ID)
PASSWORD = -w $(PEGASUS_TEST_USER_PASS)
SSL =

XMLREQUESTS = $(foreach i, $(XMLSCRIPTS), $i.xml)
XMLRESPONSES = $(XMLREQUESTS:.xml=.rsp)

XMLREQUESTS_DS = $(foreach i, $(XMLSCRIPTS_DS), $i.xml)
XMLRESPONSES_DS = $(XMLREQUESTS_DS:.xml=.rsp_ds)

XMLREQUESTS_STRIPPED = $(foreach i, $(XMLSCRIPTS_STRIPPED), $i.xml)
XMLRESPONSES_STRIPPED = $(XMLREQUESTS_STRIPPED:.xml=.rsp_stripped)
XMLGOODRESPONSE_STRIPPED = $(XMLREQUESTS_STRIPPED:.xml=rspgood.xml.stripped)

ifeq ($(OS),$(filter $(OS),linux zos))
XMLREQUESTS_STRIPPED_MU = $(foreach i, $(XMLSCRIPTS_STRIPPED_MU), $i.xml)
XMLRESPONSES_STRIPPED_MU = $(XMLREQUESTS_STRIPPED_MU:.xml=.rsp_stripped_diff)
XMLGOODRESPONSE_STRIPPED_MU = $(XMLREQUESTS_STRIPPED_MU:.xml=rspgood.xml.stripped)
else
XMLREQUESTS_STRIPPED_MU = $(foreach i, $(XMLSCRIPTS_STRIPPED_MU), $i.xml)
XMLRESPONSES_STRIPPED_MU = $(XMLREQUESTS_STRIPPED_MU:.xml=.rsp_stripped_mu)
XMLGOODRESPONSE_STRIPPED_MU = $(XMLREQUESTS_STRIPPED_MU:.xml=rspgood.xml.stripped)
endif

XMLREQUESTS_DS_STRIPPED = $(foreach i, $(XMLSCRIPTS_DS_STRIPPED), $i.xml)
XMLRESPONSES_DS_STRIPPED = $(XMLREQUESTS_DS_STRIPPED:.xml=.rsp_ds_stripped)
XMLGOODRESPONSE_DS_STRIPPED = $(XMLREQUESTS_DS_STRIPPED:.xml=rspgood.xml.stripped)

WBEMEXECOPTIONS = $(HOSTNAME) $(PORT) $(HTTPMETHOD) $(HTTPVERSION) $(USER) $(PASSWORD) $(SSL)

%.rsp: %.xml
	@ wbemexec $(WBEMEXECOPTIONS) $*.xml > $(TMP_DIR)/$*.rsp || cd .
	@ $(DIFF) $*rspgood.xml $(TMP_DIR)/$*.rsp
	@ $(RM) $(TMP_DIR)/$*.rsp
	@ $(ECHO) +++ $* passed successfully +++

##	@ $(call DIFFSORT,file_unsorted,file_sorted)

%.rsp_ds: %.xml
	@ wbemexec $(WBEMEXECOPTIONS) $*.xml > $(TMP_DIR)/$*.rsp_ds || cd .
	@ $(call DIFFSORT,$*rspgood.xml,$(TMP_DIR)/$*.rsp_ds)
	@ $(RM) $(TMP_DIR)/$*.rsp_ds
	@ $(ECHO) +++ $* passed successfully +++

%.rsp_stripped: %.xml
	@ wbemexec $(WBEMEXECOPTIONS) $*.xml > $(TMP_DIR)/$*.rsp_stripped || cd .
	@ $(COPY) $*rspgood.xml $(TMP_DIR)/$*rspgood.xml.stripped
	@ $(STRIPL) $(STRIPLINES) $(TMP_DIR)/$*.rsp_stripped
	@ $(STRIPL) $(STRIPLINES) $(TMP_DIR)/$*rspgood.xml.stripped
	@ $(DIFF) $(TMP_DIR)/$*rspgood.xml.stripped $(TMP_DIR)/$*.rsp_stripped
	@ $(RM) $(TMP_DIR)/$*.rsp_stripped
	@ $(RM) $(TMP_DIR)/$*rspgood.xml.stripped
	@ $(ECHO) +++ $* passed successfully +++

# if OS = linux
%.rsp_stripped_diff: %.xml
	@ wbemexec $(WBEMEXECOPTIONS) $*.xml > $(TMP_DIR)/$*.rsp_stripped_mu || cd .
	@ $(COPY) $*rspgood.xml $(TMP_DIR)/$*rspgood.xml.stripped
	@ $(STRIPL) $(STRIPLINES) $(TMP_DIR)/$*.rsp_stripped_mu
	@ $(STRIPL) $(STRIPLINES) $(TMP_DIR)/$*rspgood.xml.stripped
	@ $(DIFF) -I $(DIFFLINE1) -I $(DIFFLINE2) $(TMP_DIR)/$*rspgood.xml.stripped $(TMP_DIR)/$*.rsp_stripped_mu
	@ $(RM) $(TMP_DIR)/$*.rsp_stripped_mu
	@ $(RM) $(TMP_DIR)/$*rspgood.xml.stripped
	@ $(ECHO) +++ $* passed successfully +++

# if OS!=linux
%.rsp_stripped_mu: %.xml
	@ wbemexec $(WBEMEXECOPTIONS) $*.xml > $(TMP_DIR)/$*.rsp_stripped_mu || cd .
	@ $(COPY) $*rspgood.xml $(TMP_DIR)/$*rspgood.xml.stripped
	@ $(STRIPL) $(STRIPLINES) $(TMP_DIR)/$*.rsp_stripped_mu
	@ $(STRIPL) $(STRIPLINES) $(TMP_DIR)/$*rspgood.xml.stripped
	@ $(MU) $(MULINES) $(TMP_DIR)/$*.rsp_stripped_mu
	@ $(MU) $(MULINES) $(TMP_DIR)/$*rspgood.xml.stripped
	@ $(DIFF) $(TMP_DIR)/$*rspgood.xml.stripped $(TMP_DIR)/$*.rsp_stripped_mu
	@ $(RM) $(TMP_DIR)/$*.rsp_stripped_mu
	@ $(RM) $(TMP_DIR)/$*rspgood.xml.stripped
	@ $(ECHO) +++ $* passed successfully +++

%.rsp_ds_stripped: %.xml
	@ wbemexec $(WBEMEXECOPTIONS) $*.xml > $(TMP_DIR)/$*.rsp_ds_stripped || cd .
	@ $(COPY) $*rspgood.xml $(TMP_DIR)/$*rspgood.xml.stripped
	@ $(STRIPL) $(STRIPLINES_DS) $(TMP_DIR)/$*.rsp_ds_stripped
	@ $(STRIPL) $(STRIPLINES_DS) $(TMP_DIR)/$*rspgood.xml.stripped
	@ $(call DIFFSORT,$(TMP_DIR)/$*rspgood.xml.stripped,$(TMP_DIR)/$*.rsp_ds_stripped)
	@ $(RM) $(TMP_DIR)/$*.rsp_ds_stripped
	@ $(RM) $(TMP_DIR)/$*rspgood.xml.stripped
	@ $(ECHO) +++ $* passed successfully +++

