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
# wbemexec tests for ManagedSystem IP provider

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
  f01_ci_BIPTLEp \
  f02_di_BIPTLEp \
  f04_mi_BIPTLEp \
  f05_giShortKeys_BIPTLEp \
  f06_giExtraKeys_BIPTLEp \
  f07_giDupKeys_BIPTLEp \
  f08_giBadSCCN_BIPTLEp \
  f09_giBadInst_BIPTLEp \
  f10_giShortRef_BIPTLEp \
  f11_giExtraRef_BIPTLEp \
  f12_giDupRef_BIPTLEp \
  f01_ci_IPPEp \
  f02_di_IPPEp \
  f04_mi_IPPEp \
  f05_giShortKeys_IPPEp \
  f06_giExtraKeys_IPPEp \
  f07_giDupKeys_IPPEp \
  f08_giBadSCCN_IPPEp \
  f09_giBadInst_IPPEp \
  f01_ci_IPR \
  f02_di_IPR \
  f04_mi_IPR \
  f05_giShortKeys_IPR \
  f06_giExtraKeys_IPR \
  f07_giDupKeys_IPR \
  f08_giBadSCCN_IPR \
  f09_giBadInst_IPR
XMLCOVSCRIPTS = \
  c01_ei_BIPTLEp \
  c01_ei_IPPEp \
  c01_ei_IPR
else
ifeq ($(OS_TYPE),windows)
XMLSCRIPTS = \
  f01_ci_BIPTLEp \
  f02_di_BIPTLEp \
  f04_mi_BIPTLEp \
  f05_giShortKeys_BIPTLEp \
  f06_giExtraKeys_BIPTLEp \
  f07_giDupKeys_BIPTLEp \
  f08_giBadSCCN_BIPTLEp \
  f09_giBadInst_BIPTLEp \
  f10_giShortRef_BIPTLEp \
  f11_giExtraRef_BIPTLEp \
  f12_giDupRef_BIPTLEp \
  f01_ci_IPPEp \
  f02_di_IPPEp \
  f04_mi_IPPEp \
  f05_giShortKeys_IPPEp \
  f06_giExtraKeys_IPPEp \
  f07_giDupKeys_IPPEp \
  f08_giBadSCCN_IPPEp \
  f09_giBadInst_IPPEp \
  f01_ci_IPR \
  f02_di_IPR \
  f04_mi_IPR \
  f05_giShortKeys_IPR \
  f06_giExtraKeys_IPR \
  f07_giDupKeys_IPR \
  f08_giBadSCCN_IPR \
  f09_giBadInst_IPR
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

messages:

poststarttests: $(XMLRESPONSES) $(XMLCOVRESPONSES)

depend:

general:

strip-license:

prepend-license:

