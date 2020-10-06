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
##
##  Note that in addition to the normal targets defined for building and
##  executing poststarttests, this Makefile includes a target
##      createSSLFilesforCVS-SystemSpecific
##  To recreate the cert.pem ssl.cnf, and file.pem files that are in this
##  directory and are committed to CVS. Normally this would be executed only
##  if there is a problem with this self-signed certificate which is used
##  only in local testing.
##
ROOT = ../..
DIR = Server
include $(ROOT)/mak/config.mak

LIBRARIES = \
    pegservice \
    pegclient \
    pegserver \
    pegprm \
    NamespaceProvider \
    pegindicationservice \
    peghandlerservice \
    ConfigSettingProvider \
    DefaultProviderManager \
    ProviderRegistrationProvider \
    pegauthentication \
    pegqueryexpression

ifndef PEGASUS_PAM_AUTHENTICATION
    LIBRARIES += peguser \
		 UserAuthProvider
endif

ifeq ($(PEGASUS_ENABLE_CQL),true)
    LIBRARIES += \
       pegcql
endif

LIBRARIES += \
    pegwql \
    pegquerycommon

ifeq ($(PEGASUS_ENABLE_CQL),true)
    LIBRARIES += \
       CIMQueryCapabilitiesProvider
endif

LIBRARIES += \
    DefaultProviderManager \
    pegprovidermanager

ifndef PEGASUS_DISABLE_PERFINST
    LIBRARIES += \
       CIMOMStatDataProvider
endif

ifeq ($(PEGASUS_ENABLE_INTEROP_PROVIDER),true)
    LIBRARIES += \
       InteropProvider
endif

ifdef PEGASUS_HAS_SSL
LIBRARIES += \
    CertificateProvider
endif

ifeq ($(PEGASUS_ENABLE_SLP),true)
    LIBRARIES += pegslp
endif

ifeq ($(PEGASUS_ENABLE_PROTOCOL_WSMAN),true)
    LIBRARIES += pegwsmserver
endif

ifeq ($(PEGASUS_ENABLE_PROTOCOL_WEB),true)
    LIBRARIES += pegwebserver
endif

LIBRARIES += \
    pegprovidermanager \
    pegpmservice \
    pegpmrouter \
    pegprovider \
    pegexportserver \
    pegrepository \
    pegconfig \
    peghandler \
    peggeneral \
    pegcommon \
    cimrsserver

ifeq ($(PEGASUS_ENABLE_COMPRESSED_REPOSITORY),true)
    LIBRARIES += z
endif

ifeq ($(PEGASUS_PLATFORM),PASE_ISERIES_IBMCXX)
  LIBRARIES += ILEWrapperUtils
endif

EXTRA_INCLUDES = $(SYS_INCLUDES)

EXTRA_LINK_FLAGS += $(JVM_LINK_FLAGS)

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY

ifeq ($(PEGASUS_ENABLE_PRIVILEGE_SEPARATION),true)
  PROGRAM = cimservermain
else
  PROGRAM = cimserver
endif

SOURCES = cimserver.cpp

#Large Program Support
#make program able to use maximum of 2GB memory
ifeq ($(PEGASUS_PLATFORM),PASE_ISERIES_IBMCXX)
  EXTRA_LINK_FLAGS += -bmaxdata:0x80000000
endif

include $(ROOT)/mak/program.mak

ifeq ($(OS_TYPE),windows)
  SYS_LIBS = ws2_32.lib advapi32.lib	
endif

tests:

poststarttests:

#####################################################################
##
## The following set of definitions and targets generate the certificate,
## key and ssl configuration files that are in this directory and are used
## for local nightly, stress, etc. testing.  This is # a manual operation.
## It may be invoked at any time but generally when
## the cert expires (about each 10 years). There is no automatic tool to
## test for expired cert and generate new cert.
##

include $(ROOT)/mak/commands.mak

PEGASUS_CREATE_SSLCNF_OPTIONS = \
   PEGASUS_SSLCNF_COUNTRY_CODE="UK" \
   PEGASUS_SSLCNF_STATE="Berkshire" \
   PEGASUS_SSLCNF_LOCALITY="Reading" \
   PEGASUS_SSLCNF_ORGANIZATION="The Open Group" \
   PEGASUS_SSLCNF_ORGANIZATION_UNIT="The OpenPegasus Project"

ifdef PEGASUS_SSLCNF_FULLY_QUALIFIED_DSN
PEGASUS_CREATE_SSLCNF_OPTIONS+= \
   PEGASUS_SSLCNF_FULLY_QUALIFIED_DSN=$(PEGASUS_SSLCNF_FULLY_QUALIFIED_DSN)
endif

ifndef PEGASUS_SSLCERT_CNFFILE
   PEGASUS_SSLCERT_CNFFILE=$(PEGASUS_HOME)/ssl.cnf
endif

ifndef PEGASUS_SSLCERT_KEYFILE
   PEGASUS_SSLCERT_KEYFILE=$(PEGASUS_HOME)/file.pem
endif

ifndef PEGASUS_SSLCERT_CERTFILE
   PEGASUS_SSLCERT_CERTFILE=$(PEGASUS_HOME)/cert.pem
endif

PEGASUS_CREATE_SSLCERT_OPTIONS = \
   PEGASUS_SSLCERT_DAYS=3650 \
   PEGASUS_SSLCERT_CNFFILE=$(PEGASUS_SSLCERT_CNFFILE) \
   PEGASUS_SSLCERT_KEYFILE=$(PEGASUS_SSLCERT_KEYFILE)\
   PEGASUS_SSLCERT_CERTFILE=$(PEGASUS_SSLCERT_CERTFILE)

ifdef PEGASUS_SSL_RANDOMFILE
PEGASUS_CREATE_SSL_CERT_OPTIONS += \
   PEGASUS_SSLCERT_RANDOMFILE=$(PEGASUS_HOME)/cimserver.rnd
endif


##
## This target created the ssl file based on predefined information and
## then creates the certificate
##
_createSSLFiles:
	@$(MAKE) -f $(ROOT)/src/Server/Makefile createSSLCnfFile \
            $(PEGASUS_CREATE_SSLCNF_OPTIONS) \
            PEGASUS_SSLCERT_CNFFILE=$(PEGASUS_SSLCERT_CNFFILE)
	@$(MAKE) -f $(ROOT)/src/Server/Makefile createSSLCertificate \
            $(PEGASUS_CREATE_SSLCERT_OPTIONS)

##
##  Create a new ssl.cnf, key file, and cert file based on the definitions
##  defined below.  This target generates the files generally used in local
##  testing
##
createSSLFilesforCVS-SystemSpecific:
	@$(MAKE) -f $(ROOT)/src/Server/Makefile _createSSLFiles \
            PEGASUS_CREATE_PEGASUS_INTERNAL_CERT=yes \
            PEGASUS_SSLCERT_CNFFILE=$(ROOT)/src/Server/ssl.cnf \
            PEGASUS_SSLCERT_KEYFILE=$(ROOT)/src/Server/file.pem \
            PEGASUS_SSLCERT_CERTFILE=$(ROOT)/src/Server/cert.pem \
            PEGASUS_SSLCNF_FULLY_QUALIFIED_DSN="\"PEGASUS TEST CERTIFICATE-DO NOT USE\""

##
## Simply displays the existing certificate
##
displayCERTFILE:
	@$(ECHO) "*** $(PEGASUS_SSLCERT_CERTFILE) ***"
	@$(MAKE) -f $(ROOT)/src/Server/Makefile displayCertificate \
            PEGASUS_SSLCERT_FILE=$(PEGASUS_SSLCERT_CERTFILE)

clean: cleanupSSLFiles

install_run_clean: cleanupSSLFiles

cleanupSSLFiles:
	@$(RM) $(PEGASUS_HOME)/cert.pem
	@$(RM) $(PEGASUS_HOME)/file.pem
	@$(RM) $(PEGASUS_HOME)/server.pem
	@$(RM) $(PEGASUS_HOME)/client.pem
	@$(RM) $(PEGASUS_HOME)/cimserver.rnd
	@$(RM) $(PEGASUS_HOME)/ssl.rnd
	@$(RM) $(PEGASUS_HOME)/ssl.cnf
	@$(RM) $(PEGASUS_HOME)/client_cert.pem
	@$(RM) $(PEGASUS_HOME)/client_file.pem
	@$(RMDIRHIER) $(PEGASUS_HOME)/cimserver_trust
	@$(RMDIRHIER) $(PEGASUS_HOME)/indication_trust
	@$(RMDIRHIER) $(PEGASUS_HOME)/crl
	@$(RM) $(PEGASUS_HOME)/setupserverdev

createSSLTrustDirectories:
	@$(MKDIRHIER) $(PEGASUS_HOME)/cimserver_trust
	@$(MKDIRHIER) $(PEGASUS_HOME)/indication_trust
	@$(MKDIRHIER) $(PEGASUS_HOME)/crl

removeSSLTrustDirectories:
	@$(RMDIRHIER) $(PEGASUS_HOME)/cimserver_trust
	@$(RMDIRHIER) $(PEGASUS_HOME)/indication_trust
	@$(RMDIRHIER) $(PEGASUS_HOME)/crl

stageServerSSLFiles-SystemSpecific:
	@$(MAKE) -f $(ROOT)/src/Server/Makefile cleanupSSLFiles
	@$(COPY) ssl.rnd $(PEGASUS_HOME)
	@$(COPY) cimserver.rnd $(PEGASUS_HOME)
	@$(MAKE) -f $(ROOT)/src/Server/Makefile _createSSLFiles \
            $(PEGASUS_CREATE_SSLCNF_OPTIONS) \
            PEGASUS_SSLCERT_CNFFILE=$(PEGASUS_SSLCERT_CNFFILE)
	@$(COPY) $(PEGASUS_SSLCERT_CERTFILE) $(PEGASUS_HOME)/server.pem
	@$(COPY) $(PEGASUS_SSLCERT_CERTFILE) $(PEGASUS_HOME)/client.pem

stageServerSSLFiles-SystemIndependent:
	@$(MAKE) -f $(ROOT)/src/Server/Makefile cleanupSSLFiles
	@$(COPY) cert.pem $(PEGASUS_HOME)
	@$(COPY) file.pem $(PEGASUS_HOME)
	@$(COPY) cert.pem $(PEGASUS_HOME)/server.pem
	@$(COPY) cert.pem $(PEGASUS_HOME)/client.pem
	@$(COPY) cimserver.rnd $(PEGASUS_HOME)
	@$(COPY) ssl.rnd $(PEGASUS_HOME)
	@$(COPY) ssl.cnf $(PEGASUS_HOME)

stageClientSSLFiles-SystemIndependent:
	@$(COPY) cert.pem $(PEGASUS_HOME)/client_cert.pem
	@$(COPY) file.pem $(PEGASUS_HOME)/client_file.pem
	@$(MAKE) -f $(ROOT)/src/Server/Makefile createSSLTrustDirectories

stageCIMServerConfigurationFiles:
	@$(RM) $(PEGASUS_HOME)/cimserver_planned.conf
	@$(COPY) cimserver_planned.conf $(PEGASUS_HOME)

install_run: $(PEGASUS_HOME)/setupserverdev

##
## Note: the $(PEGASUS_HOME)/trace dir must be writable by all users for
## the tracing facility to work.

$(PEGASUS_HOME)/setupserverdev: cert.pem file.pem cimserver.rnd ssl.rnd ssl.cnf
	@$(MAKE) -f $(ROOT)/src/Server/Makefile stageServerSSLFiles-SystemIndependent
	@$(MAKE) -f $(ROOT)/src/Server/Makefile stageClientSSLFiles-SystemIndependent
	@$(MAKE) -f $(ROOT)/src/Server/Makefile stageCIMServerConfigurationFiles
	$(MKDIRHIER) $(PEGASUS_HOME)/trace
ifeq ($(OS_TYPE),unix)
ifdef PEGASUS_TEST_VALGRIND_LOG_DIR
	-$(MKDIRHIER) $(PEGASUS_TEST_VALGRIND_LOG_DIR)
endif
# Allow the repository to be created as the cimserver user
	$(CHMOD) 777 $(PEGASUS_HOME)
# Allow trace files to be created by cimserver and cimprovagt process owners
	$(CHMOD) 777 $(PEGASUS_HOME)/trace
endif
ifeq ($(OS_TYPE),windows)
	-@$(PEGASUS_HOME)/bin/cimserver -install
endif
	@$(TOUCH) $(PEGASUS_HOME)/setupserverdev
ifeq ($(PEGASUS_TEST_ENABLE_DEBUG_TRACE),true)
        # Enable trace to capture Discarded data trace information to help
        # troubleshoot and detect test failures.
	cimconfig -s traceComponents=DiscardedData -p
	cimconfig -s traceLevel=4 -p
endif

uninstall:
ifeq ($(OS_TYPE),windows)
	$(PEGASUS_HOME)/bin/cimserver -remove
endif

run: install_run
ifeq ($(OS_TYPE),windows)
	cimserver -start
else
	cimserver
endif

