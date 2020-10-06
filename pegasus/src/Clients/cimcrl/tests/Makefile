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

DIR = Clients/cimcrl/tests

include $(ROOT)/mak/config.mak
include $(ROOT)/mak/test.mak
include $(ROOT)/mak/commands.mak

RESULTFILE =  $(TMP_DIR)/result
MASTERRESULTFILE = $(ROOT)/src/$(DIR)/result.master
OPENSSL_CNF = openssl.cnf

tests:

poststarttests:

clean:
ifeq ($(PEGASUS_ENABLE_SSL_CRL_VERIFICATION), true)
	@$(RM) $(RESULTFILE)
	@$(RM) $(PEGASUS_HOME)/testcert2.cert
	@$(RM) $(PEGASUS_HOME)/testcert2.key
	@$(RM) $(PEGASUS_HOME)/testcert3.cert
	@$(RM) $(PEGASUS_HOME)/testcert3.key
	@$(RM) $(PEGASUS_HOME)/testcert4.cert
	@$(RM) $(PEGASUS_HOME)/testcert4.key
	@$(RM) $(PEGASUS_HOME)/testcrlca1.cert
	@$(RM) $(PEGASUS_HOME)/testcrlca1.key
	@$(RM) $(PEGASUS_HOME)/testca1.srl
	@$(RM) $(PEGASUS_HOME)/testca1.crl
	@$(RM) $(PEGASUS_HOME)/testca2.crl
	@$(RM) $(PEGASUS_HOME)/testmonth1.crl
	@$(RM) $(PEGASUS_HOME)/testmonth2.crl
	@$(RM) $(PEGASUS_HOME)/testmonth3.crl
	@$(RM) $(PEGASUS_HOME)/testmonth4.crl
	@$(RM) $(PEGASUS_HOME)/testmonth5.crl
	@$(RM) $(PEGASUS_HOME)/testmonth6.crl
	@$(RM) $(PEGASUS_HOME)/testmonth7.crl
	@$(RM) $(PEGASUS_HOME)/testmonth8.crl
	@$(RM) $(PEGASUS_HOME)/testmonth9.crl
	@$(RM) $(PEGASUS_HOME)/testmonth10.crl
	@$(RM) $(PEGASUS_HOME)/testmonth11.crl
	@$(RM) $(PEGASUS_HOME)/testmonth12.crl
	@$(RM) $(PEGASUS_HOME)/invalid.crl
endif
	@$(RM) index.txt
	@$(RM) index.txt.attr
	@$(RM) index.txt.old
	@$(RM) index.txt.attr.old
	@$(RM) .rnd
	@$(RM) $(PEGASUS_HOME)/crltest_setup

#
# Create certificates and CRLs required for running the tests
#
test_setup: $(PEGASUS_HOME)/crltest_setup 

$(PEGASUS_HOME)/crltest_setup: $(OPENSSL_CNF)
	@$(MAKE) -i -s clean
	@$(TOUCH) $(PEGASUS_HOME)/crltest_setup
ifdef PEGASUS_HAS_SSL
ifeq ($(PEGASUS_ENABLE_SSL_CRL_VERIFICATION), true)
        #
        # Copy certs and CRLs to PEGASUS_HOME
        #
	@$(COPY) testcrlca1.cert $(PEGASUS_HOME)
	@$(COPY) testcrlca1.key  $(PEGASUS_HOME)

	@$(COPY) testcert2.cert $(PEGASUS_HOME)
	@$(COPY) testcert2.key  $(PEGASUS_HOME)

	@$(COPY) testcert3.cert $(PEGASUS_HOME)
	@$(COPY) testcert3.key  $(PEGASUS_HOME)

	@$(COPY) testcert4.cert $(PEGASUS_HOME)
	@$(COPY) testcert4.key  $(PEGASUS_HOME)

	@$(COPY) testca1.crl  $(PEGASUS_HOME)
	@$(COPY) testca2.crl  $(PEGASUS_HOME)
	@$(COPY) invalid.crl  $(PEGASUS_HOME)

	@$(COPY) testmonth1.crl $(PEGASUS_HOME)
	@$(COPY) testmonth2.crl  $(PEGASUS_HOME)
	@$(COPY) testmonth3.crl $(PEGASUS_HOME)
	@$(COPY) testmonth4.crl $(PEGASUS_HOME)
	@$(COPY) testmonth5.crl $(PEGASUS_HOME)
	@$(COPY) testmonth6.crl $(PEGASUS_HOME)
	@$(COPY) testmonth7.crl $(PEGASUS_HOME)
	@$(COPY) testmonth8.crl $(PEGASUS_HOME)
	@$(COPY) testmonth9.crl $(PEGASUS_HOME)
	@$(COPY) testmonth10.crl $(PEGASUS_HOME)
	@$(COPY) testmonth11.crl $(PEGASUS_HOME)
	@$(COPY) testmonth12.crl $(PEGASUS_HOME)
endif
endif

SSLCRLTest1: test_setup
ifdef PEGASUS_HAS_SSL
ifeq ($(PEGASUS_ENABLE_SSL_CRL_VERIFICATION), true)
	@$(RM) $(RESULTFILE)
	@$(MAKE) -i -s SSLCRLTest1_ignoreerror
	@$(STRIPCRS)
ifdef PEGASUS_OVERRIDE_SSL_CERT_VERIFICATION_RESULT
	@$(DIFF) result.master.override result
else
	$(COMPARERESULTS)
endif
	@$(ECHO) +++ Test passed +++
	@$(RM) $(RESULTFILE)
endif
endif

SSLCRLTest1_ignoreerror:
ifdef PEGASUS_HAS_SSL
	@$(ECHO) ++++cimcrl tests
ifndef PEGASUS_PAM_AUTHENTICATION
    ifneq ($(OS), zos)
	cimuser -a -u $(CURRENT_USER) -w $(CURRENT_USER)
    endif
endif	

	@$(ECHO) "Missing required options." >> $(RESULTFILE)
	@cimcrl abcd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate add option." >> $(RESULTFILE)
	@cimcrl -a -a >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Combine remove and add options." >> $(RESULTFILE)
	@cimcrl -r -a >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Combine add and remove options." >> $(RESULTFILE)
	@cimcrl -a -r >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Combine add and list options." >> $(RESULTFILE)
	@cimcrl -a -l >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate remove option." >> $(RESULTFILE)
	@cimcrl -r -r >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate list option." >> $(RESULTFILE)
	@cimcrl -l -l >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate issuer with list option." >> $(RESULTFILE)
	@cimcrl -l -i abc -i abc >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing issuer with list option." >> $(RESULTFILE)
	@cimcrl -l -i >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing issuer with remove option." >> $(RESULTFILE)
	@cimcrl -r >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing issuer value with remove option." >> $(RESULTFILE)
	@cimcrl -r -i >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Add option without a crl file. Should error out." >> $(RESULTFILE)
	@cimcrl -a -f >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Add option with a non-existent crl file. Should error out." >> $(RESULTFILE)
	@cimcrl -a -f ./nosuchcrl.pem >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Add option without a crl file option. Should error out." >> $(RESULTFILE)
	@cimcrl -a >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Unexpected option with add operation. Should error out." >> $(RESULTFILE)
	@cimcrl -a -f abc.pem -z >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Unexpected option with list operation. Should error out." >> $(RESULTFILE)
	@cimcrl -l -z >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Unexpected option with remove operation. Should error out." >> $(RESULTFILE)
	@cimcrl -r -z >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate crlfiles specified." >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testca.crl -f $(PEGASUS_HOME)/testca.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "No command line options specified." >> $(RESULTFILE)
	@cimcrl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid command line options specified." >> $(RESULTFILE)
	@cimcrl -no >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Display help." >> $(RESULTFILE)
	@cimcrl --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "List the CRLs, no CRLs displayed." >> $(RESULTFILE)
	@cimcrl -l >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Help and version together. Should error out." >> $(RESULTFILE)
	@cimcrl --help --version >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Version and help together. Should error out." >> $(RESULTFILE)
	@cimcrl --version --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Add an invalid CRL to the truststore. Should error out" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/invalid.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the CA certificate to the truststore" >> $(RESULTFILE)
	@cimtrust -a -U $(CURRENT_USER) -Ta -f $(PEGASUS_HOME)/testcrlca1.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 1st certificate issued by the CA, this should succeed" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert2.cert $(PEGASUS_HOME)/testcert2.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 2nd certificate issued by the CA, this should succeed" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert3.cert $(PEGASUS_HOME)/testcert3.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 3rd certificate issued by the CA, this should succeed" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert4.cert $(PEGASUS_HOME)/testcert4.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Add the CA CRL to the truststore" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testca1.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "List the CRL." >> $(RESULTFILE)
	@cimcrl -l >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 1st certificate, this should fail" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert2.cert $(PEGASUS_HOME)/testcert2.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 2nd certificate, this should succeed" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert3.cert $(PEGASUS_HOME)/testcert3.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 3rd certificate issued by the CA, this should succeed" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert4.cert $(PEGASUS_HOME)/testcert4.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Add the updated CA CRL to the truststore, this should succeed" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testca2.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	 
	@$(ECHO) "List the updated CRL by issuer." >> $(RESULTFILE)
	@cimcrl -l -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	 
	@$(ECHO) "List the by non-existant issuer." >> $(RESULTFILE)
	@cimcrl -l -i abcd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	
	@$(ECHO) "Try to connect with the 1st certificate, this should fail" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert2.cert $(PEGASUS_HOME)/testcert2.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 2nd certificate, this should succeed" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert3.cert $(PEGASUS_HOME)/testcert3.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 3rd certificate, this should fail" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert4.cert $(PEGASUS_HOME)/testcert4.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Remove the CA CRL certificate" >> $(RESULTFILE)
	@cimcrl -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 1st certificate, this should succeed" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert2.cert $(PEGASUS_HOME)/testcert2.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 2nd certificate, this should succeed" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert3.cert $(PEGASUS_HOME)/testcert3.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 3rd certificate, this should succeed" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert4.cert $(PEGASUS_HOME)/testcert4.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	 
	@$(ECHO) "Remove the CA CRL that we just removed, this should get a DNE error" >> $(RESULTFILE)
	@cimcrl -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	@$(STRIPL) "CIM_ERR_NOT_FOUND" ".r0." $(RESULTFILE)

	@$(ECHO) "Remove the CA certificate from the truststore to clean everything up, this should succeed" >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 1st certificate issued by the CA, this should fail" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert2.cert $(PEGASUS_HOME)/testcert2.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 2nd certificate issued by the CA, this should fail" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert3.cert $(PEGASUS_HOME)/testcert3.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 3rd certificate issued by the CA, this should fail" >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testcert4.cert $(PEGASUS_HOME)/testcert4.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)	
	
	@$(ECHO) "Add Jan month CA CRL to the truststore, this should succeed" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testmonth1.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	 
	@$(ECHO) "List the updated CRL by issuer." >> $(RESULTFILE)
	@cimcrl -l -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	 
	@$(ECHO) "Remove CRL by issuer." >> $(RESULTFILE)
	@cimcrl -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	
	@$(ECHO) "Add Feb month CA CRL to the truststore, this should succeed" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testmonth2.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	 
	@$(ECHO) "List the updated CRL by issuer." >> $(RESULTFILE)
	@cimcrl -l -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	 
	@$(ECHO) "Remove CRL by issuer." >> $(RESULTFILE)
	@cimcrl -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	
	@$(ECHO) "Add Mar month CA CRL to the truststore, this should succeed" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testmonth3.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	 
	@$(ECHO) "List the updated CRL by issuer." >> $(RESULTFILE)
	@cimcrl -l -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	 
	@$(ECHO) "Remove CRL by issuer." >> $(RESULTFILE)
	@cimcrl -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	
	@$(ECHO) "Add Apr month CA CRL to the truststore, this should succeed" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testmonth4.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	 
	@$(ECHO) "List the updated CRL by issuer." >> $(RESULTFILE)
	@cimcrl -l -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	 
	@$(ECHO) "Remove CRL by issuer." >> $(RESULTFILE)
	@cimcrl -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	
	@$(ECHO) "Add May month CA CRL to the truststore, this should succeed" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testmonth5.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	 
	@$(ECHO) "List the updated CRL by issuer." >> $(RESULTFILE)
	@cimcrl -l -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	 
	@$(ECHO) "Remove CRL by issuer." >> $(RESULTFILE)
	@cimcrl -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	
	@$(ECHO) "Add June month CA CRL to the truststore, this should succeed" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testmonth6.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	 
	@$(ECHO) "List the updated CRL by issuer." >> $(RESULTFILE)
	@cimcrl -l -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	 
	@$(ECHO) "Remove CRL by issuer." >> $(RESULTFILE)
	@cimcrl -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	
	@$(ECHO) "Add July month CA CRL to the truststore, this should succeed" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testmonth7.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	 
	@$(ECHO) "List the updated CRL by issuer." >> $(RESULTFILE)
	@cimcrl -l -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	 
	@$(ECHO) "Remove CRL by issuer." >> $(RESULTFILE)
	@cimcrl -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	
	@$(ECHO) "Add Aug month CA CRL to the truststore, this should succeed" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testmonth8.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	 
	@$(ECHO) "List the updated CRL by issuer." >> $(RESULTFILE)
	@cimcrl -l -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	 
	@$(ECHO) "Remove CRL by issuer." >> $(RESULTFILE)
	@cimcrl -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	
	@$(ECHO) "Add Sept month CA CRL to the truststore, this should succeed" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testmonth9.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	 
	@$(ECHO) "List the updated CRL by issuer." >> $(RESULTFILE)
	@cimcrl -l -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	 
	@$(ECHO) "Remove CRL by issuer." >> $(RESULTFILE)
	@cimcrl -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	
	@$(ECHO) "Add Oct month CA CRL to the truststore, this should succeed" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testmonth10.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	 
	@$(ECHO) "List the updated CRL by issuer." >> $(RESULTFILE)
	@cimcrl -l -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	 
	@$(ECHO) "Remove CRL by issuer." >> $(RESULTFILE)
	@cimcrl -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	
	@$(ECHO) "Add Nov month CA CRL to the truststore, this should succeed" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testmonth11.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	 
	@$(ECHO) "List the updated CRL by issuer." >> $(RESULTFILE)
	@cimcrl -l -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	 
	@$(ECHO) "Remove CRL by issuer." >> $(RESULTFILE)
	@cimcrl -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	
	@$(ECHO) "Add Dec month CA CRL to the truststore, this should succeed" >> $(RESULTFILE)
	@cimcrl -a -f $(PEGASUS_HOME)/testmonth12.crl >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	 
	@$(ECHO) "List the updated CRL by issuer." >> $(RESULTFILE)
	@cimcrl -l -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       
	 
	@$(ECHO) "Remove CRL by issuer." >> $(RESULTFILE)
	@cimcrl -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)       

ifndef PEGASUS_PAM_AUTHENTICATION
    ifneq ($(OS), zos)
	cimuser -r -u $(CURRENT_USER)
    endif
endif
	@$(ECHO) ++++cimcrl tests completed
endif

depend:

general:

strip-license:

prepend-license:
