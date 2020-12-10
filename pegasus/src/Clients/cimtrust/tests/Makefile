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

DIR = Clients/cimtrust/tests

include $(ROOT)/mak/config.mak
include $(ROOT)/mak/test.mak
include $(ROOT)/mak/commands.mak

RESULTFILE =  $(TMP_DIR)/result

ifeq ($(OS_TYPE),windows)
MASTERRESULTFILE = $(ROOT)/src/$(DIR)/result.master.windows
else
MASTERRESULTFILE = $(ROOT)/src/$(DIR)/result.master
endif

OPENSSL_CNF = openssl.cnf

ifeq ($(OPENSSL_SET_SERIAL_SUPPORTED),true)
    SETSERIAL_0 = -set_serial 0
    SETSERIAL_1 = -set_serial 1
    SETSERIAL_2 = -set_serial 2
    SETSERIAL_3 = -set_serial 3
else
    SETSERIAL_0 =
    SETSERIAL_1 =
    SETSERIAL_2 =
    SETSERIAL_3 =
endif

tests:

poststarttests:

clean:
	@$(RM) $(RESULTFILE)
	@$(RM) $(PEGASUS_HOME)/testdn1.cert
	@$(RM) $(PEGASUS_HOME)/testdn1.key
	@$(RM) $(PEGASUS_HOME)/testdn2.cert
	@$(RM) $(PEGASUS_HOME)/testdn2.key
	@$(RM) $(PEGASUS_HOME)/testdn3.cert
	@$(RM) $(PEGASUS_HOME)/testdn3.key
	@$(RM) $(PEGASUS_HOME)/testdn4.cert
	@$(RM) $(PEGASUS_HOME)/testdn4.key
	@$(RM) $(PEGASUS_HOME)/testexpired1.cert
	@$(RM) $(PEGASUS_HOME)/testexpired1.key
	@$(RM) $(PEGASUS_HOME)/testnotyetvalid1.cert
	@$(RM) $(PEGASUS_HOME)/testnotyetvalid1.key 
	@$(RM) $(PEGASUS_HOME)/testca1.cert
	@$(RM) $(PEGASUS_HOME)/testca1.key
	@$(RM) $(PEGASUS_HOME)/testca1.srl
	@$(RM) $(PEGASUS_HOME)/testca2.cert
	@$(RM) $(PEGASUS_HOME)/testca2.key
	@$(RM) $(PEGASUS_HOME)/testca2.srl
	@$(RM) $(PEGASUS_HOME)/testchca.cert
	@$(RM) $(PEGASUS_HOME)/testchca.key
	@$(RM) $(PEGASUS_HOME)/testchca.srl
	@$(RM) $(PEGASUS_HOME)/testinter.cert
	@$(RM) $(PEGASUS_HOME)/testinter.key
	@$(RM) $(PEGASUS_HOME)/testinter.srl
	@$(RM) $(PEGASUS_HOME)/testleaf.cert
	@$(RM) $(PEGASUS_HOME)/testleaf.key
	@$(RM) $(PEGASUS_HOME)/testdup.cert
	@$(RM) $(PEGASUS_HOME)/testdup.key
	@$(RM) $(PEGASUS_HOME)/testmonth1.cert
	@$(RM) $(PEGASUS_HOME)/testmonth1.key
	@$(RM) $(PEGASUS_HOME)/testmonth2.cert
	@$(RM) $(PEGASUS_HOME)/testmonth2.key
	@$(RM) $(PEGASUS_HOME)/testmonth3.cert
	@$(RM) $(PEGASUS_HOME)/testmonth3.key
	@$(RM) $(PEGASUS_HOME)/testmonth4.cert
	@$(RM) $(PEGASUS_HOME)/testmonth4.key
	@$(RM) $(PEGASUS_HOME)/testmonth5.cert
	@$(RM) $(PEGASUS_HOME)/testmonth5.key
	@$(RM) $(PEGASUS_HOME)/testmonth6.cert
	@$(RM) $(PEGASUS_HOME)/testmonth6.key
	@$(RM) $(PEGASUS_HOME)/testmonth7.cert
	@$(RM) $(PEGASUS_HOME)/testmonth7.key
	@$(RM) $(PEGASUS_HOME)/testmonth8.cert
	@$(RM) $(PEGASUS_HOME)/testmonth8.key
	@$(RM) $(PEGASUS_HOME)/testmonth9.cert
	@$(RM) $(PEGASUS_HOME)/testmonth9.key
	@$(RM) $(PEGASUS_HOME)/testmonth10.cert
	@$(RM) $(PEGASUS_HOME)/testmonth10.key
	@$(RM) $(PEGASUS_HOME)/testmonth11.cert
	@$(RM) $(PEGASUS_HOME)/testmonth11.key
	@$(RM) $(PEGASUS_HOME)/testmonth12.cert
	@$(RM) $(PEGASUS_HOME)/testmonth12.key
	@$(RM) $(PEGASUS_HOME)/testmulti.cert
	@$(RM) $(PEGASUS_HOME)/testmulti.key
	@$(RM) $(PEGASUS_HOME)/testinvalid_x509.cert
	@$(RM) $(PEGASUS_HOME)/testinvalid_x509.key
	@$(RM) index.txt
	@$(RM) index.txt.attr
	@$(RM) index.txt.old
	@$(RM) index.txt.attr.old
	@$(RM) .rnd
	@$(RM) $(PEGASUS_HOME)/ssltest_setup

#
# Create certificates required for running the tests
#
test_setup: $(PEGASUS_HOME)/ssltest_setup 

$(PEGASUS_HOME)/ssltest_setup: $(OPENSSL_CNF) testca1.txt testdn1.txt testdn2.txt testdn3.txt testdn4.txt testexpired1.cert testexpired1.key testnotyetvalid1.cert testnotyetvalid1.key testchca.cert testchca.key testinter.cert testinter.key testleaf.cert testleaf.key
	@$(MAKE) -i -s clean
	@$(TOUCH) $(PEGASUS_HOME)/ssltest_setup
ifdef PEGASUS_HAS_SSL
        #
        # Copy the file containing multiple certificates to the staging area
        #
	@$(COPY) testmulti.cert $(PEGASUS_HOME)/testmulti.cert
        #
        # Copy the expired certificate to the staging area
        #
	@$(COPY) testexpired1.cert $(PEGASUS_HOME)/testexpired1.cert
	@$(COPY) testexpired1.key $(PEGASUS_HOME)/testexpired1.key
        #
        # Create the not yet valid certificate to the staging area
        #   
	@$(COPY) testnotyetvalid1.cert $(PEGASUS_HOME)/testnotyetvalid1.cert
	@$(COPY) testnotyetvalid1.key $(PEGASUS_HOME)/testnotyetvalid1.key
        #
        # Copy the test chain root certificate to the staging area
        #
	@$(COPY) testchca.cert $(PEGASUS_HOME)/testchca.cert
	@$(COPY) testchca.key $(PEGASUS_HOME)/testchca.key
        #
        # Copy the test chain intermediate certificate to the staging area
        #   
	@$(COPY) testinter.cert $(PEGASUS_HOME)/testinter.cert
	@$(COPY) testinter.key $(PEGASUS_HOME)/testinter.key
        #
        # Copy the leaf certificate to the staging area
        #   
	@$(COPY) testleaf.cert $(PEGASUS_HOME)/testleaf.cert
	@$(COPY) testleaf.key $(PEGASUS_HOME)/testleaf.key
        #
        # Copy the invalid X509 certificate to the staging area
        #   
	@$(COPY) testinvalid_x509.cert $(PEGASUS_HOME)/testinvalid_x509.cert
        #
        # Copy the test month certificates to the staging area
        #   
	@$(COPY) testmonth1.cert $(PEGASUS_HOME)/testmonth1.cert
	@$(COPY) testmonth1.key $(PEGASUS_HOME)/testmonth1.key
	@$(COPY) testmonth2.cert $(PEGASUS_HOME)/testmonth2.cert
	@$(COPY) testmonth2.key $(PEGASUS_HOME)/testmonth2.key
	@$(COPY) testmonth3.cert $(PEGASUS_HOME)/testmonth3.cert
	@$(COPY) testmonth3.key $(PEGASUS_HOME)/testmonth3.key
	@$(COPY) testmonth4.cert $(PEGASUS_HOME)/testmonth4.cert
	@$(COPY) testmonth4.key $(PEGASUS_HOME)/testmonth4.key
	@$(COPY) testmonth5.cert $(PEGASUS_HOME)/testmonth5.cert
	@$(COPY) testmonth5.key $(PEGASUS_HOME)/testmonth5.key
	@$(COPY) testmonth6.cert $(PEGASUS_HOME)/testmonth6.cert
	@$(COPY) testmonth6.key $(PEGASUS_HOME)/testmonth6.key
	@$(COPY) testmonth7.cert $(PEGASUS_HOME)/testmonth7.cert
	@$(COPY) testmonth7.key $(PEGASUS_HOME)/testmonth7.key
	@$(COPY) testmonth8.cert $(PEGASUS_HOME)/testmonth8.cert
	@$(COPY) testmonth8.key $(PEGASUS_HOME)/testmonth8.key
	@$(COPY) testmonth9.cert $(PEGASUS_HOME)/testmonth9.cert
	@$(COPY) testmonth9.key $(PEGASUS_HOME)/testmonth9.key
	@$(COPY) testmonth10.cert $(PEGASUS_HOME)/testmonth10.cert
	@$(COPY) testmonth10.key $(PEGASUS_HOME)/testmonth10.key
	@$(COPY) testmonth11.cert $(PEGASUS_HOME)/testmonth11.cert
	@$(COPY) testmonth11.key $(PEGASUS_HOME)/testmonth11.key
	@$(COPY) testmonth12.cert $(PEGASUS_HOME)/testmonth12.cert
	@$(COPY) testmonth12.key $(PEGASUS_HOME)/testmonth12.key
        #
        # Create a self-signed certificate
        #
	@$(OPENSSL_COMMAND) genrsa -out $(PEGASUS_HOME)/testdn1.key 1024
	@$(OPENSSL_COMMAND) req -config $(OPENSSL_CNF) -new -key $(PEGASUS_HOME)/testdn1.key -out $(PEGASUS_HOME)/testdn1.csr < testdn1.txt
	@$(OPENSSL_COMMAND) x509 -in $(PEGASUS_HOME)/testdn1.csr -out $(PEGASUS_HOME)/testdn1.cert -req -signkey $(PEGASUS_HOME)/testdn1.key $(SETSERIAL_0) -days 356
	@$(RM) $(PEGASUS_HOME)/testdn1.csr
        #
        # Create a self-signed CA
        #
	@$(OPENSSL_COMMAND) genrsa -out $(PEGASUS_HOME)/testca1.key 1024
	@$(OPENSSL_COMMAND) req -new -key $(PEGASUS_HOME)/testca1.key -x509 -config $(OPENSSL_CNF) $(SETSERIAL_0) -days 365 -out $(PEGASUS_HOME)/testca1.cert < testca1.txt
        #
        # Create a certificate signed by the CA
        #
	@$(OPENSSL_COMMAND) genrsa -out $(PEGASUS_HOME)/testdn2.key 1024
	@$(OPENSSL_COMMAND) req -config $(OPENSSL_CNF) -new -key $(PEGASUS_HOME)/testdn2.key -out $(PEGASUS_HOME)/testdn2.csr < testdn2.txt
	@$(OPENSSL_COMMAND) x509 -req $(SETSERIAL_1) -days 365 -in $(PEGASUS_HOME)/testdn2.csr -CA $(PEGASUS_HOME)/testca1.cert -CAkey $(PEGASUS_HOME)/testca1.key -CAcreateserial -out $(PEGASUS_HOME)/testdn2.cert
	@$(RM) $(PEGASUS_HOME)/testdn2.csr
        #
        # Create a second test certificate signed by the CA
        #
	@$(OPENSSL_COMMAND) genrsa -out $(PEGASUS_HOME)/testdn3.key 1024
	@$(OPENSSL_COMMAND) req -config $(OPENSSL_CNF) -new -key $(PEGASUS_HOME)/testdn3.key -out $(PEGASUS_HOME)/testdn3.csr < testdn3.txt
	@$(OPENSSL_COMMAND) x509 -req $(SETSERIAL_2) -days 365 -in $(PEGASUS_HOME)/testdn3.csr -CA $(PEGASUS_HOME)/testca1.cert -CAkey $(PEGASUS_HOME)/testca1.key -CAcreateserial -out $(PEGASUS_HOME)/testdn3.cert
	@$(RM) $(PEGASUS_HOME)/testdn3.csr
        #
        # Create a third test certificate signed by the CA
        #
	@$(OPENSSL_COMMAND) genrsa -out $(PEGASUS_HOME)/testdn4.key 1024
	@$(OPENSSL_COMMAND) req -config $(OPENSSL_CNF) -new -key $(PEGASUS_HOME)/testdn4.key -out $(PEGASUS_HOME)/testdn4.csr < testdn4.txt
	@$(OPENSSL_COMMAND) x509 -req $(SETSERIAL_3) -days 365 -in $(PEGASUS_HOME)/testdn4.csr -CA $(PEGASUS_HOME)/testca1.cert -CAkey $(PEGASUS_HOME)/testca1.key -CAcreateserial -out $(PEGASUS_HOME)/testdn4.cert
	@$(RM) $(PEGASUS_HOME)/testdn4.csr
        #
        # Create a self-signed CA2
        #
	@$(OPENSSL_COMMAND) genrsa -out $(PEGASUS_HOME)/testca2.key 1024
	@$(OPENSSL_COMMAND) req -new -key $(PEGASUS_HOME)/testca2.key -x509 -config $(OPENSSL_CNF) $(SETSERIAL_0) -days 365 -out $(PEGASUS_HOME)/testca2.cert < testca2.txt
        #
        # Create a certificate signed by the CA
        #
	@$(OPENSSL_COMMAND) genrsa -out $(PEGASUS_HOME)/testdup.key 1024
	@$(OPENSSL_COMMAND) req -config $(OPENSSL_CNF) -new -key $(PEGASUS_HOME)/testdup.key -out $(PEGASUS_HOME)/testdup.csr < testdn1.txt
	@$(OPENSSL_COMMAND) x509 -req $(SETSERIAL_1) -days 365 -in $(PEGASUS_HOME)/testdup.csr -CA $(PEGASUS_HOME)/testca2.cert -CAkey $(PEGASUS_HOME)/testca2.key -CAcreateserial -out $(PEGASUS_HOME)/testdup.cert
	@$(RM) $(PEGASUS_HOME)/testdup.csr
endif

SSLCertificateTest1: test_setup
ifdef PEGASUS_HAS_SSL
	@$(RM) $(RESULTFILE)
	@$(MAKE) -i -s SSLCertificateTest1_ignoreerror
	@$(STRIPCRS)
ifdef PEGASUS_OVERRIDE_SSL_CERT_VERIFICATION_RESULT
	@$(DIFF) result.master.override result
else
	$(COMPARERESULTS)
endif
	@$(ECHO) +++ Test passed +++
	@$(RM) $(RESULTFILE)
endif

SSLCertificateTest1_ignoreerror:
ifdef PEGASUS_HAS_SSL
	@$(ECHO) ++++cimtrust tests
ifndef PEGASUS_PAM_AUTHENTICATION
    ifneq ($(OS), zos)
	cimuser -a -u $(CURRENT_USER) -w $(CURRENT_USER)
    endif
endif	
	
	@$(ECHO)  +++++++++ Command line testing +++++++++ >> $(RESULTFILE)

	@$(ECHO) "Add the authority certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ta -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testchca.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate list option." >> $(RESULTFILE)
	@cimtrust -l -l  -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "No command line options specified." >> $(RESULTFILE)
	@cimtrust >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid command line options specified." >> $(RESULTFILE)
	@cimtrust -no >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid command line options specified." >> $(RESULTFILE)
	@cimtrust nosuchoption >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Help option." >> $(RESULTFILE)
	@cimtrust --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate help option." >> $(RESULTFILE)
	@cimtrust --help --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate version option." >> $(RESULTFILE)
	@cimtrust --version --version >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate -a option." >> $(RESULTFILE)
	@cimtrust -a -a -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testinter.cert -T a >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate -f option." >> $(RESULTFILE)
	@cimtrust -a -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testinter.cert -f $(PEGASUS_HOME)/testinter.cert -T a >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate -U option." >> $(RESULTFILE)
	@cimtrust -a -U $(CURRENT_USER) -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testinter.cert  -T a >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate -T option." >> $(RESULTFILE)
	@cimtrust -a -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testinter.cert -T a -T a >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid -T option." >> $(RESULTFILE)
	@cimtrust -a -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testinter.cert -T f >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, type option not specified." >> $(RESULTFILE)
	@cimtrust -a -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testchca.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing argument for -T option." >> $(RESULTFILE)
	@cimtrust -a -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testinter.cert -T >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing certuser for -T s option." >> $(RESULTFILE)
	@cimtrust -a -f $(PEGASUS_HOME)/testdn1.cert -T s >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing certuser for -T e option." >> $(RESULTFILE)
	@cimtrust -a -f $(PEGASUS_HOME)/testleaf.cert -T e >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, certfile not specified." >> $(RESULTFILE)
	@cimtrust -a -U $(CURRENT_USER) -T a >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, certfile value not specified." >> $(RESULTFILE)
	@cimtrust -a -U $(CURRENT_USER) -T a -f  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate -r option." >> $(RESULTFILE)
	@cimtrust -r -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing option issuerName." >> $(RESULTFILE)
	@cimtrust -r -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Missing value for serial number." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -n >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate option, issuer specified twice." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -i test -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate option, serial number specified twice." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -n 0 -n 1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Duplicate option, subject specified twice." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -S test -S test >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid remove option, incorrect serial number." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -n 2222 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid remove option, incorrect subject." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -S abcd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, issuer not specified." >> $(RESULTFILE)
	@cimtrust -r >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, issuer value not specified." >> $(RESULTFILE)
	@cimtrust -r -i  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Required option missing, serial number not specified." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine remove and certfile." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -n 0 -f abc >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine subject and serial number." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -S abc -n 1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine serial number and subject." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -n 1 -S abc >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine remove and certuser." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -n 0 -U abc >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine add and help." >> $(RESULTFILE)
	@cimtrust -a --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine remove and help." >> $(RESULTFILE)
	@cimtrust -r --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine list and help." >> $(RESULTFILE)
	@cimtrust -l --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine add and help." >> $(RESULTFILE)
	@cimtrust --help -a >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine remove and help." >> $(RESULTFILE)
	@cimtrust --help -r >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine list and help." >> $(RESULTFILE)
	@cimtrust --help -l >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Unexpected option, combine version and help." >> $(RESULTFILE)
	@cimtrust --version --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid option, invalid system user." >> $(RESULTFILE)
	@cimtrust -a -T a -U nosuchuser -f $(PEGASUS_HOME)/testinter.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid option, Duplicate cert user." >> $(RESULTFILE)
	@cimtrust -a -T a -U nosuchuser -U nosuchuser -f $(PEGASUS_HOME)/testinter.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Invalid option, -x." >> $(RESULTFILE)
	@cimtrust -x >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Command-line tests complete, remove the CA certificate from the truststore, this should succeed." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO)  +++++++++ Functional tests - Add operation testing +++++++++ >> $(RESULTFILE)
	@$(ECHO) "Add a certificate file that contains more than one certificate, this should error out." >> $(RESULTFILE)
	@cimtrust -a -T a -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testmulti.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ts -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testdn1.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect, this should succeed." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testdn1.cert $(PEGASUS_HOME)/testdn1.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the month1 self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ts -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testmonth1.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the month2 self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ts -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testmonth2.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the month3 self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ts -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testmonth3.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the month4 self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ts -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testmonth4.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the month5 self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ts -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testmonth5.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the month6 self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ts -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testmonth6.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the month7 self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ts -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testmonth7.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the month8 self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ts -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testmonth8.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the month9 self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ts -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testmonth9.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the month10 self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ts -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testmonth10.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the month11 self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ts -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testmonth11.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the month12 self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ts -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testmonth12.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "List the Certificates, should all succeed." >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -l -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Jan -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -l -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Feb -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -l -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Mar -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -l -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Apr -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -l -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-May -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -l -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Jun -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -l -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Jul -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -l -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Aug -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -l -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Sept -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -l -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Oct -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -l -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Nov -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -l -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Dec -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Remove the Certificates, should all succeed." >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Jan -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Feb -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Mar -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Apr -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-May -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Jun -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Jul -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Aug -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Sept -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Oct -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Nov -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-Dec -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO)  +++++++++ Functional tests - Delete operation testing +++++++++ >> $(RESULTFILE)
	@$(ECHO) "Delete the self-signed certificate from the truststore." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1 -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect, this should fail." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testdn1.cert $(PEGASUS_HOME)/testdn1.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Attempt to delete the certificate we just deleted, this should get a DNE error." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1 -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -T s -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testdn1.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Delete the self-signed certificate from the truststore by subject & issuer." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1 -S /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect, this should fail." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testdn1.cert $(PEGASUS_HOME)/testdn1.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Attempt to delete the certificate we just deleted, this should get a DNE error." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1 -S /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the deleted self-signed certificate to the truststore. This should succeed" >> $(RESULTFILE)
	@cimtrust -a -T s -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testdn1.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Delete the self-signed certificate from the truststore by subject & issuer. This should succeed" >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1 -S /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO)  +++++++++ Functional tests - List operation testing +++++++++ >> $(RESULTFILE)
	@$(ECHO) "Negative testcases." >> $(RESULTFILE)
	@$(ECHO) "Add the self-signed root certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ta -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testchca.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the intermediate CA signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -T a -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testinter.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List certificates, specify non-existent serial number. No certs listed." >> $(RESULTFILE)
	@cimtrust -l -i "/C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA" -n 1234 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List certificates, specify non-existent issuername. No certs listed." >> $(RESULTFILE)
	@cimtrust -l -i "/C=US/ST=Nevada/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA" >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List certificates, specify non-existent subject. No certs listed." >> $(RESULTFILE)
	@cimtrust -l -i "/C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA" -S "/C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=NoSuchCert" >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List certificates, specify only the serial number. This should fail." >> $(RESULTFILE)
	@cimtrust -l -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List certificates, specify only the subject. This should fail." >> $(RESULTFILE)
	@cimtrust -l -S "/C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA" >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List certificates, specify the only serial number. Should fail since issuer not specified." >> $(RESULTFILE)
	@cimtrust -l -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List certificates, specify both serial number & subject, this should fail." >> $(RESULTFILE)
	@cimtrust -l -i "/C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA" -n 0 -S "/C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA" >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List certificates, specify both subject & serial number, this should fail." >> $(RESULTFILE)
	@cimtrust -l -i "/C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA" -S "/C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA" -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Positive testcases." >> $(RESULTFILE)
	@$(ECHO) "List certificates, specify the issuername." >> $(RESULTFILE)
	@cimtrust -l -i "/C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA" >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List certificates, specify the issuername & subject." >> $(RESULTFILE)
	@cimtrust -l -i "/C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA" -S "/C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA" >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List certificates, specify the issuername & serial number." >> $(RESULTFILE)
	@cimtrust -l -i "/C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA" -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the CA certificate from the truststore, this should succeed." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to connect with the intermediate certificate issued by the CA, this should fail." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testinter.cert $(PEGASUS_HOME)/testinter.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to connect with the leaf certificate issued by the CA, this should fail." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testleaf.cert $(PEGASUS_HOME)/testleaf.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the intermediate CA certificate from the truststore, this should succeed." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -n 2 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) +++++++++ Functional tests - Certificate chain tests +++++++++ >> $(RESULTFILE)
	
	@$(ECHO) +++++++++ Single-level chain tests +++++++++ >> $(RESULTFILE)
	@$(ECHO) "Add the CA certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -T a -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testca1.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to connect with the 1st certificate issued by the CA, this should succeed." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testdn2.cert $(PEGASUS_HOME)/testdn2.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 2nd certificate issued by the CA, this should succeed." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testdn3.cert $(PEGASUS_HOME)/testdn3.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 3rd certificate issued by the CA, this should succeed." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testdn4.cert $(PEGASUS_HOME)/testdn4.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to connect with the 1st certificate issued by CA1, this should succeed." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testdn2.cert $(PEGASUS_HOME)/testdn2.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the second CA certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -T a -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testca2.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the self-signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -Ts -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testdn1.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with testdn1, this should succeed." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testdn1.cert $(PEGASUS_HOME)/testdn1.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to connect with testdup (same subject as testdn1) issued by CA2, this should succeed." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testdup.cert $(PEGASUS_HOME)/testdup.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Remove the CA1 certificate from the truststore, this should succeed." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=WASHINGTON/L=Seattle/O=OpenGroup/OU=OpenPegasus/CN=TestCA -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 1st certificate issued by the CA, this should fail." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testdn2.cert $(PEGASUS_HOME)/testdn2.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 2nd certificate issued by the CA, this should fail." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testdn3.cert $(PEGASUS_HOME)/testdn3.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Try to connect with the 3rd certificate issued by the CA, this should fail." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testdn4.cert $(PEGASUS_HOME)/testdn4.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the CA2 certificate from the truststore, this should succeed." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=OREGON/L=Portland/O=OpenGroup/OU=OpenPegasus/CN=TestCA -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the testdup certificate to the truststore, this should fail with duplicate subject error." >> $(RESULTFILE)
	@cimtrust -a -Ta -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testdup.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Delete the self-signed certificate from the truststore by subject & issuer. This should succeed" >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1 -S /C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) +++++++++ Functional tests - multi-level chain tests  +++++++++ >> $(RESULTFILE)
	@$(ECHO) "Add the root CA certificate to the truststore without an associated username." >> $(RESULTFILE)
	@cimtrust -a -T a -f $(PEGASUS_HOME)/testchca.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to connect, this should fail since the root CA does not have an associated username." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testinter.cert $(PEGASUS_HOME)/testinter.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the intermediate CA signed certificate to the truststore without an associated username." >> $(RESULTFILE)
	@cimtrust -a -T a -f $(PEGASUS_HOME)/testinter.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to connect, this should fail since the root CA does not have an associated username." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testinter.cert $(PEGASUS_HOME)/testinter.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the leaf CA signed certificate to the truststore with an associated username." >> $(RESULTFILE)
	@cimtrust -a -Te -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testleaf.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List the leaf certificate, specify the issuername." >> $(RESULTFILE)
	@cimtrust -l -i "/C=US/ST=Nevada/L=LasVegas/O=OpenGroup/OU=OpenPegasus/CN=TestInterCA" -n 2 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to connect, this should succeed since the leaf has an associated username." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testleaf.cert $(PEGASUS_HOME)/testleaf.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the CA certificate from the truststore, this should succeed." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the intermediate CA certificate from the truststore, this should succeed." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -n 2 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the leaf CA signed certificate from the truststore, this should succeed." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=Nevada/L=LasVegas/O=OpenGroup/OU=OpenPegasus/CN=TestInterCA -n 2 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the self-signed root certificate with an associated user to the truststore." >> $(RESULTFILE)
	@cimtrust -a -T a -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testchca.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to connect with root cert, this should succeed." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testchca.cert $(PEGASUS_HOME)/testchca.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to connect with intermediate cert, this should succeed." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testinter.cert $(PEGASUS_HOME)/testinter.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Add the intermediate CA signed certificate to the truststore." >> $(RESULTFILE)
	@cimtrust -a -T a -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testinter.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to connect with root cert, this should succeed." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testchca.cert $(PEGASUS_HOME)/testchca.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to connect with intermediate cert, this should succeed." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testinter.cert $(PEGASUS_HOME)/testinter.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to connect, this should succeed since the intermediate cert has associated username." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testleaf.cert $(PEGASUS_HOME)/testleaf.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the CA certificate from the truststore, this should succeed." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -n 0 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the intermediate CA certificate from the truststore, this should succeed." >> $(RESULTFILE)
	@cimtrust -r -i /C=US/ST=California/L=Sacramento/O=OpenGroup/OU=OpenPegasus/CN=TestChainCA -n 2 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO)  +++++++++ Funtional tests - Tests for special cases +++++++++ >> $(RESULTFILE)
	@$(ECHO) "Invalid X509 certificate." >> $(RESULTFILE)
	@cimtrust -a -T s -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testinvalid_x509.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Connect with an expired certificate -- should fail." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testexpired1.cert $(PEGASUS_HOME)/testexpired1.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
	@$(ECHO) "Add the expired certificate to the truststore -- should get an expiration error." >> $(RESULTFILE)
	@cimtrust -a -T s -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testexpired1.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)	
	
	@$(ECHO) "Connect with a not yet valid certificate -- should fail." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testnotyetvalid1.cert $(PEGASUS_HOME)/testnotyetvalid1.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)  
	
	@$(ECHO) "Add the not yet valid certificate to the truststore -- should get a not yet valid error." >> $(RESULTFILE)
	@cimtrust -a -T s -U $(CURRENT_USER) -f $(PEGASUS_HOME)/testnotyetvalid1.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)  

	@$(ECHO) "Connect with an untrusted certificate -- should fail." >> $(RESULTFILE)
	@TestCertClient $(PEGASUS_HOME)/testdn1.cert $(PEGASUS_HOME)/testdn1.key $(PEGASUS_HOME)/ssl.rnd >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Non-existent certificate file -- should fail." >> $(RESULTFILE)
	@cimtrust -a -U $(CURRENT_USER) -T a -f ./nosuchfile.cert >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)
	
ifndef PEGASUS_PAM_AUTHENTICATION
    ifneq ($(OS), zos)
	cimuser -r -u $(CURRENT_USER)
    endif
endif
	@$(ECHO) ++++cimtrust tests completed
endif

depend:

general:

strip-license:

prepend-license:
