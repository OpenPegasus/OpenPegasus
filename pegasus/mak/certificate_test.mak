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

###############################################################################
#   This Makefile implements three main funtions:
#       make -f TestMakefile setupCBATestCertificates
#       make -f TestMakefile runCBATestSuites
#       make -f TestMakefile cleanupCBATestCertificates
###############################################################################

ifndef PEGASUS_HAS_SSL
setupCBATestCertificates: FORCE
	@$(ECHO) \
         "+++++ PEGASUS_HAS_SSL not defined: CBA test certificates not created."
cleanupCBATestCertificates: FORCE
	@$(ECHO) \
         "+++++ PEGASUS_HAS_SSL not defined: CBA test certificates not removed."
runCBATestSuites: FORCE
	@$(ECHO) \
         "+++++ PEGASUS_HAS_SSL not defined: CBA test suites not run."
else
ifdef PEGASUS_DISABLE_PRIVILEGED_TESTS
setupCBATestCertificates: FORCE
	@$(ECHO) \
         "+++++ PEGASUS_DISABLE_PRIVILEGED_TESTS defined: CBA test certificates not created."
cleanupCBATestCertificates: FORCE
	@$(ECHO) \
         "+++++ PEGASUS_DISABLE_PRIVILEGED_TESTS defined: CBA test certificates not created."
runCBATestSuites: FORCE
	@$(ECHO) \
         "+++++ PEGASUS_DISABLE_PRIVILEGED_TESTS defined: CBA test certificates not created."
else
ifndef PEGASUS_TEST_USER_DEFINED
setupCBATestCertificates: FORCE
	@$(ECHO) \
            "+++++ PEGASUS_ PEGASUS_TEST_USER_DEFINED not defined:" \
            "CBA test certificates not created."
cleanupCBATestCertificates: FORCE
	@$(ECHO) \
            "+++++ PEGASUS_ PEGASUS_TEST_USER_DEFINED not defined:" \
            "CBA test certificates not removed."
runCBATestSuites: FORCE
	@$(ECHO) \
            "+++++ PEGASUS_ PEGASUS_TEST_USER_DEFINED not defined:" \
            "CBA test suites not run."
else

###############################################################################
#   To add new certificates to this Makefile:
#   1.) Update the OpenPegasus Certificate Test Readme to include the
#       new certificate definition. This document is available in
#       in CVS (pegasus/mak/certificate_test_readme.html).
#   2.) Add the definition of the new certificate to the
#       "Certificate Definition" section in this Makefile.
#       Notes:
#          a. Please use the Certificate ID (CID) to generate the Certificate
#             Serial Number (i.e., 50000+CID).  Note: To avoid collisions with
#             other tests, all serial numbers are in the 50000 - 59999 range.
#   3.) Add the new certificate to the appropriate definition group.  See
#       the "Certificate Group Definition" section for more details.
###############################################################################

###############################################################################
# To use certificate_test.mak, the following variables need to be
# included in the enclosing Makefile.
#     ROOT = $(PEGASUS_ROOT)
#     include $(ROOT)/mak/config.mak
#     include $(ROOT)/mak/commands.mak
#     include $(ROOT)/mak/test.mak
#     .PHONY: FORCE
#     FORCE:
#
# To avoid conflicts with enclosing Makefiles, all Makefile options include
# the string "CBA" (e.g., setupCBATestCerticates).
###############################################################################
include $(ROOT)/mak/configschema.mak

ifndef PEGASUS_TEST_CERT_USER_ID
   PEGASUS_TEST_CERT_USER_ID = $(PEGASUS_TEST_USER_ID)
endif

ifndef PEGASUS_TEST_USER_INVLD_USR
   PEGASUS_TEST_USER_INVLD_USR = invalidUSER
endif

ifndef PEGASUS_TEST_USER_INVLD_PASS
   PEGASUS_TEST_USER_INVLD_PASS = invalidPWD
endif

ifndef PEGASUS_TEST_CERT_DIR
   PEGASUS_TEST_CERT_DIR = $(PEGASUS_HOME)/certDir
endif

ifndef PEGASUS_SSL_SERVER_TRUSTSTORE
   PEGASUS_SSL_SERVER_TRUSTSTORE = $(PEGASUS_HOME)/cimserver_trust
endif

PEGASUS_CBA_TEST_CA_DATABASE_FILE = $(PEGASUS_TEST_CERT_DIR)/databaseFile
PEGASUS_CBA_TEST_CA_SERIAL_FILE = $(PEGASUS_TEST_CERT_DIR)/serialFile
PEGASUS_CBA_TEST_CA_SSL_CNF_FILE = $(PEGASUS_TEST_CERT_DIR)/testOpenSSL.cnf

###############################################################################
#   Certificate Definitions
###############################################################################

PEGASUS_TEST_INVLD_CIM_NOT_B = "20040329074919.000000+000"
PEGASUS_TEST_INVLD_CERT_NOT_B = "040329074919Z"
PEGASUS_TEST_INVLD_CIM_NOT_A = "20050329074919.000000+000"
PEGASUS_TEST_INVLD_CERT_NOT_A = "050329074919Z"

PEGASUS_TEST_VLD_CIM_NOT_B = "20060329074919.000000+000"
PEGASUS_TEST_VLD_CERT_NOT_B = "060329074919Z"
PEGASUS_TEST_VLD_CIM_NOT_A = "20260329074919.000000+000"
PEGASUS_TEST_VLD_CERT_NOT_A = "260329074919Z"

# ID = 10: Define PEGASUS_SCERT_NOTTS_NO_USR
# Valid Certificate - Not In Truststore - No User
PEGASUS_SCERT_NOTTS_NO_USR = testSCertNotInTrustStoreNoUser
PEGASUS_SCERT_NOTTS_NO_USR_SUBJECT = \
    "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=NOTTS_NO_USER"
PEGASUS_SCERT_NOTTS_NO_USR_ISSUER_SYM = PEGASUS_SCERT_NOTTS_NO_USR
PEGASUS_SCERT_NOTTS_NO_USR_SERIAL_NO = 50010
PEGASUS_SCERT_NOTTS_NO_USR_SERIAL_NO_HEX = C35A
PEGASUS_SCERT_NOTTS_NO_USR_EXPIRED = NO
PEGASUS_SCERT_NOTTS_NO_USR_TYPE = s
PEGASUS_SCERT_NOTTS_NO_USR_IN_TS = NO

# ID = 40: Define PEGASUS_SCERT_IN_TS_NO_USR
# Valid Certificate - In Truststore - No User
PEGASUS_SCERT_IN_TS_NO_USR = testSCertInTrustStoreNoUser
PEGASUS_SCERT_IN_TS_NO_USR_SUBJECT = \
    "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=SCERT_IN_TS_NO_USER"
PEGASUS_SCERT_IN_TS_NO_USR_ISSUER_SYM = PEGASUS_SCERT_IN_TS_NO_USR
PEGASUS_SCERT_IN_TS_NO_USR_SERIAL_NO = 50040
PEGASUS_SCERT_IN_TS_NO_USR_SERIAL_NO_HEX = C378
PEGASUS_SCERT_IN_TS_NO_USR_USER_NAME =
PEGASUS_SCERT_IN_TS_NO_USR_HASH_NAME = \
    $(shell $(OPENSSL_COMMAND) x509 -hash -noout \
    -in $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_NO_USR).cert).0
PEGASUS_SCERT_IN_TS_NO_USR_EXPIRED = NO
PEGASUS_SCERT_IN_TS_NO_USR_TYPE = s
PEGASUS_SCERT_IN_TS_NO_USR_IN_TS = YES

# ID = 50: Define PEGASUS_SCERT_IN_TS_VLD_USR
# Valid Certificate - In Truststore - Valid User
PEGASUS_SCERT_IN_TS_VLD_USR = testSCertInTrustStoreValidUser
PEGASUS_SCERT_IN_TS_VLD_USR_SUBJECT = \
    "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=SCERT_IN_TS_VALID_USER"
PEGASUS_SCERT_IN_TS_VLD_USR_ISSUER_SYM = PEGASUS_SCERT_IN_TS_VLD_USR
PEGASUS_SCERT_IN_TS_VLD_USR_SERIAL_NO = 50050
PEGASUS_SCERT_IN_TS_VLD_USR_SERIAL_NO_HEX = C382
PEGASUS_SCERT_IN_TS_VLD_USR_USER_NAME = $(PEGASUS_TEST_CERT_USER_ID)
PEGASUS_SCERT_IN_TS_VLD_USR_EXPIRED = NO
PEGASUS_SCERT_IN_TS_VLD_USR_TYPE = s
PEGASUS_SCERT_IN_TS_VLD_USR_IN_TS = YES

# ID = 60: Define PEGASUS_SCERT_IN_TS_INVLD_USR
# Valid Certificate - In Truststore - Invalid User
PEGASUS_SCERT_IN_TS_INVLD_USR = testSCertInTrustStoreInvalidUser
PEGASUS_SCERT_IN_TS_INVLD_USR_SUBJECT = \
    "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=SCERT_IN_TS_INVALID_USER"
PEGASUS_SCERT_IN_TS_INVLD_USR_ISSUER_SYM = PEGASUS_SCERT_IN_TS_INVLD_USR
PEGASUS_SCERT_IN_TS_INVLD_USR_SERIAL_NO = 50060
PEGASUS_SCERT_IN_TS_INVLD_USR_SERIAL_NO_HEX = C38C
PEGASUS_SCERT_IN_TS_INVLD_USR_USER_NAME = \
    $(PEGASUS_TEST_USER_INVLD_USR)
PEGASUS_SCERT_IN_TS_INVLD_USR_HASH_NAME = \
    $(shell $(OPENSSL_COMMAND) x509 -hash -noout \
    -in $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).cert).0
PEGASUS_SCERT_IN_TS_INVLD_USR_EXPIRED = NO
PEGASUS_SCERT_IN_TS_INVLD_USR_TYPE = s
PEGASUS_SCERT_IN_TS_INVLD_USR_IN_TS = YES

# ID = 070: Define PEGASUS_EXP_SCERT_NOTTS_NO_USR
# Expired Certificate - In Truststore - No User
PEGASUS_EXP_SCERT_NOTTS_NO_USR = testExpiredSCertNotInTrustStoreNoUser
PEGASUS_EXP_SCERT_NOTTS_NO_USR_SUBJECT = \
    "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=EXP_SCERT_NOT_IN_TS_NO_USER"
PEGASUS_EXP_SCERT_NOTTS_NO_USR_ISSUER_SYM =  PEGASUS_EXP_SCERT_NOTTS_NO_USR
PEGASUS_EXP_SCERT_NOTTS_NO_USR_SERIAL_NO = 50070
PEGASUS_EXP_SCERT_NOTTS_NO_USR_SERIAL_NO_HEX = C396
PEGASUS_EXP_SCERT_NOTTS_NO_USR_USER_NAME =
PEGASUS_EXP_SCERT_NOTTS_NO_USR_HASH_NAME = \
    $(shell $(OPENSSL_COMMAND) x509 -hash -noout \
    -in $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).cert).0
PEGASUS_EXP_SCERT_NOTTS_NO_USR_EXPIRED = YES
PEGASUS_EXP_SCERT_NOTTS_NO_USR_TYPE = s
PEGASUS_EXP_SCERT_NOTTS_NO_USR_IN_TS = NO

# ID = 100: Define PEGASUS_EXP_SCERT_IN_TS_NO_USR
# Expired Certificate - In Truststore - No User
PEGASUS_EXP_SCERT_IN_TS_NO_USR = testExpiredSCertInTrustStoreNoUser
PEGASUS_EXP_SCERT_IN_TS_NO_USR_SUBJECT = \
    "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=EXP_SCERT_IN_TS_NO_USER"
PEGASUS_EXP_SCERT_IN_TS_NO_USR_ISSUER_SYM = PEGASUS_EXP_SCERT_IN_TS_NO_USR
PEGASUS_EXP_SCERT_IN_TS_NO_USR_SERIAL_NO = 50100
PEGASUS_EXP_SCERT_IN_TS_NO_USR_SERIAL_NO_HEX = C3B4
PEGASUS_EXP_SCERT_IN_TS_NO_USR_USER_NAME =
PEGASUS_EXP_SCERT_IN_TS_NO_USR_HASH_NAME = \
    $(shell $(OPENSSL_COMMAND) x509 -hash -noout \
    -in $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).cert).0
PEGASUS_EXP_SCERT_IN_TS_NO_USR_EXPIRED = YES
PEGASUS_EXP_SCERT_IN_TS_NO_USR_TYPE = s
PEGASUS_EXP_SCERT_IN_TS_NO_USR_IN_TS = YES

# ID = 110 : Define PEGASUS_EXP_SCERT_IN_TS_VLD_USR
# Expired Certificate - In Truststore - Valid User
PEGASUS_EXP_SCERT_IN_TS_VLD_USR = testExpiredSCertInTrustStoreValidUser
PEGASUS_EXP_SCERT_IN_TS_VLD_USR_SUBJECT = \
    "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=EXP_SCERT_IN_TS_VALID_USER"
PEGASUS_EXP_SCERT_IN_TS_VLD_USR_ISSUER_SYM = PEGASUS_EXP_SCERT_IN_TS_VLD_USR
PEGASUS_EXP_SCERT_IN_TS_VLD_USR_SERIAL_NO = 50110
PEGASUS_EXP_SCERT_IN_TS_VLD_USR_SERIAL_NO_HEX = C3BE
PEGASUS_EXP_SCERT_IN_TS_VLD_USR_USER_NAME = $(PEGASUS_TEST_CERT_USER_ID)
PEGASUS_EXP_SCERT_IN_TS_VLD_USR_HASH_NAME = \
    $(shell $(OPENSSL_COMMAND) x509 -hash -noout \
    -in $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).cert).0
PEGASUS_EXP_SCERT_IN_TS_VLD_USR_EXPIRED = YES
PEGASUS_EXP_SCERT_IN_TS_VLD_USR_TYPE = s
PEGASUS_EXP_SCERT_IN_TS_VLD_USR_IN_TS = YES

# ID = 120: Define PEGASUS_EXP_SCERT_IN_TS_INVLD_USR
# Expired Certificate - In Truststore - Invalid User
PEGASUS_EXP_SCERT_IN_TS_INVLD_USR = testExpiredSCertInTrustStoreInvalidUser
PEGASUS_EXP_SCERT_IN_TS_INVLD_USR_SUBJECT = \
    "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=EXP_SCERT_IN_TS_INVALID_USER"
PEGASUS_EXP_SCERT_IN_TS_INVLD_USR_ISSUER_SYM = PEGASUS_EXP_SCERT_IN_TS_INVLD_USR
PEGASUS_EXP_SCERT_IN_TS_INVLD_USR_SERIAL_NO = 50120
PEGASUS_EXP_SCERT_IN_TS_INVLD_USR_SERIAL_NO_HEX = C3C8
PEGASUS_EXP_SCERT_IN_TS_INVLD_USR_USER_NAME = \
    $(PEGASUS_TEST_USER_INVLD_USR)
PEGASUS_EXP_SCERT_IN_TS_INVLD_USR_HASH_NAME = \
    $(shell $(OPENSSL_COMMAND) x509 -hash -noout \
    -in $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_INVLD_USR).cert).0
PEGASUS_EXP_SCERT_IN_TS_INVLD_USR_EXPIRED = YES
PEGASUS_EXP_SCERT_IN_TS_INVLD_USR_TYPE = s
PEGASUS_EXP_SCERT_IN_TS_INVLD_USR_IN_TS = YES

# ID = 240: Define PEGASUS_RACERT_IN_TS_NO_USR
# Valid Certificate - In Truststore - No User
PEGASUS_RACERT_IN_TS_NO_USR = testRACertInTrustStoreNoUser
PEGASUS_RACERT_IN_TS_NO_USR_SUBJECT = \
    "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=RACERT_IN_TS_NO_USER"
PEGASUS_RACERT_IN_TS_NO_USR_ISSUER_SYM = PEGASUS_RACERT_IN_TS_NO_USR
PEGASUS_RACERT_IN_TS_NO_USR_SERIAL_NO = 50240
PEGASUS_RACERT_IN_TS_NO_USR_SERIAL_NO_HEX = C440
PEGASUS_RACERT_IN_TS_NO_USR_USER_NAME =
PEGASUS_RACERT_IN_TS_NO_USR_EXPIRED = NO
PEGASUS_RACERT_IN_TS_NO_USR_TYPE = a
PEGASUS_RACERT_IN_TS_NO_USR_IN_TS = YES

# ID = 610: Define PEGASUS_ECERT_NOTTS_NO_USR_RA0240
# Valid ECERT - Not In Truststore - No User
PEGASUS_ECERT_NOTTS_NO_USR_RA0240 = \
    testECertNotInTrustStoreNoUser_testRACertInTrustStoreNoUser
PEGASUS_ECERT_NOTTS_NO_USR_RA0240_SUBJECT = \
   "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=ECERT_NOT_IN_TS_NO_USER_RACERT_IN_TS"
PEGASUS_ECERT_NOTTS_NO_USR_RA0240_SERIAL_NO = 50610
PEGASUS_ECERT_NOTTS_NO_USR_RA0240_SERIAL_NO_HEX = C5B2
PEGASUS_ECERT_NOTTS_NO_USR_RA0240_ISSUER_SYM = PEGASUS_RACERT_IN_TS_NO_USR
PEGASUS_ECERT_NOTTS_NO_USR_RA0240_EXPIRED = NO
PEGASUS_ECERT_NOTTS_NO_USR_RA0240_TYPE = e
PEGASUS_ECERT_NOTTS_NO_USR_RA0240_IN_TS = NO

# ID = 620: Define PEGASUS_ECERT_NOTTS_VLD_USR_RA0240
# Valid ECERT - Not In Truststore - Valid User
PEGASUS_ECERT_NOTTS_VLD_USR_RA0240 = \
    testECertNotInTrustStoreValidUser_testRACertInTrustStoreValidUser
PEGASUS_ECERT_NOTTS_VLD_USR_RA0240_SUBJECT = \
    "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=ECERT_NOT_IN_TS_VALID_USER_RACERT_IN_TS"
PEGASUS_ECERT_NOTTS_VLD_USR_RA0240_SERIAL_NO = 50620
PEGASUS_ECERT_NOTTS_VLD_USR_RA0240_SERIAL_NO_HEX = C5BC
PEGASUS_ECERT_NOTTS_VLD_USR_RA0240_USER_NAME = \
   $(PEGASUS_TEST_CERT_USER_ID)
PEGASUS_ECERT_NOTTS_VLD_USR_RA0240_ISSUER_SYM = PEGASUS_RACERT_IN_TS_NO_USR
PEGASUS_ECERT_NOTTS_VLD_USR_RA0240_EXPIRED = NO
PEGASUS_ECERT_NOTTS_VLD_USR_RA0240_TYPE = e
PEGASUS_ECERT_NOTTS_VLD_USR_RA0240_IN_TS = NO
PEGASUS_ECERT_NOTTS_VLD_USR_RA0240_HASH_NAME = \
    $(shell $(OPENSSL_COMMAND) x509 -hash -noout \
    -in $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).cert).0

# ID = 630: Define PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240
# Valid ECERT - Not In Truststore - Invalid User
PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240 = \
    testECertNotInTrustStoreInvalidUser_testRACertInTrustStoreValidUser
PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240_SUBJECT = \
    "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=ECERT_NOT_IN_TS_INVALID_USER_RACERT_IN_TS"
PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240_SERIAL_NO = 50630
PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240_SERIAL_NO_HEX = C5C6
PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240_USER_NAME = \
    $(PEGASUS_TEST_USER_INVLD_USR)
PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240_HASH_NAME = \
    $(shell $(OPENSSL_COMMAND) x509 -hash -noout \
    -in $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).cert).0
PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240_ISSUER_SYM = PEGASUS_RACERT_IN_TS_NO_USR
PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240_EXPIRED = NO
PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240_TYPE = e
PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240_IN_TS = NO

# ID = 670: Define PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240
# Expired ECERT - Not In Truststore - No User
PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240 = \
    testExpiredECertNotInTrustStoreNoUser_testRACertInTrustStoreNoUser
PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240_SUBJECT = \
   "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=EXP_ECERT_NOT_IN_TS_NO_USER_RACERT_IN_TS"
PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240_SERIAL_NO = 50670
PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240_SERIAL_NO_HEX = C5EE
PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240_ISSUER_SYM = PEGASUS_RACERT_IN_TS_NO_USR
PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240_EXPIRED = YES
PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240_TYPE = e
PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240_IN_TS = NO

# ID = 680: Define PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240
# Expired ECERT - Not In Truststore - Valid User
PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240 = \
    testExpiredECertNotInTrustStoreValidUser_testRACertInTrustStoreValidUser
PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240_SUBJECT = \
    "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=EXP_ECERT_NOT_IN_TS_VALID_USER_RACERT_IN_TS"
PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240_SERIAL_NO = 50680
PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240_SERIAL_NO_HEX = C5F8
PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240_USER_NAME = \
   $(PEGASUS_TEST_CERT_USER_ID)
PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240_ISSUER_SYM = PEGASUS_RACERT_IN_TS_NO_USR
PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240_EXPIRED = YES
PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240_TYPE = e
PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240_IN_TS = NO
PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240_HASH_NAME = \
    $(shell $(OPENSSL_COMMAND) x509 -hash -noout -in \
    $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).cert).0

# ID = 690: Define PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240
# Expired ECERT - Not In Truststore - Invalid User
PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240 = \
    testExpiredECertNotInTrustStoreInvalidUser_testRACertInTrustStoreValidUser
PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240_SUBJECT = \
  "/C=US/ST=CA/L=SF/O=OG/OU=OP/CN=EXP_ECERT_NOT_IN_TS_INVALID_USER_RACERT_IN_TS"
PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240_SERIAL_NO = 50690
PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240_SERIAL_NO_HEX = C602
PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240_USER_NAME = \
    $(PEGASUS_TEST_USER_INVLD_USR)
PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240_HASH_NAME = \
    $(shell $(OPENSSL_COMMAND) x509 -hash -noout -in \
    $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).cert).0
PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240_ISSUER_SYM = \
     PEGASUS_RACERT_IN_TS_NO_USR
PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240_EXPIRED = YES
PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240_TYPE = e
PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240_IN_TS = NO

###############################################################################
#   Certificate Group Definitions
###############################################################################
###############################################################################
# When a new certificate is defined it should also be added to one and only
# one of the following three groups:
#
#  PEGASUS_VALIDATED_REG_XXXX certificates are valid certificates that can
#  be successfully registered using the cimtrust command.
#    PEGASUS_VALIDATED_REG_SCERT_NAME_SYMBOLS
#    PEGASUS_VALIDATED_REG_RACERT_NAME_SYMBOLS
#    PEGASUS_VALIDATED_REG_IACERT_NAME_SYMBOLS
#    PEGASUS_VALDIATED_REG_ECERT_NAME_SYMBOLS
#
#  PEGASUS_NOT_VALIDATED_REG_XXXX certificates are certificates that can
#  NOT be registered using the cimtrust command. To bypass the validity
#  tests performed by the Certificate Provider, the cimmofl command will
#  be used to register these certificates.
#    PEGASUS_NOT_VALIDATED_REG_SCERT_NAME_SYMBOLS
#    PEGASUS_NOT_VALIDATED_REG_RACERT_NAME_SYMBOLS
#    PEGASUS_NOT_VALIDATED_REG_IACERT_NAME_SYMBOLS
#    PEGASUS_NOT_VALIDATED_REG_ECERT_NAME_SYMBOLS
#
# PEGASUS_NOT_REG_XXXX certificates are not registered with the
# CIM Server. These certificates still need to be created.
###############################################################################

PEGASUS_VALIDATED_REG_SCERT_NAME_SYMBOLS = \
   PEGASUS_SCERT_IN_TS_VLD_USR

PEGASUS_VALIDATED_REG_RACERT_NAME_SYMBOLS = \
   PEGASUS_RACERT_IN_TS_NO_USR

PEGASUS_VALIDATED_REG_IACERT_NAME_SYMBOLS =

PEGASUS_VALIDATED_REG_ECERT_NAME_SYMBOLS =

PEGASUS_VALIDATED_REG_CERT_NAME_SYMBOLS = \
   $(PEGASUS_VALIDATED_REG_SCERT_NAME_SYMBOLS) \
   $(PEGASUS_VALIDATED_REG_RACERT_NAME_SYMBOLS) \
   $(PEGASUS_VALIDATED_REG_IACERT_NAME_SYMBOLS) \
   $(PEGASUS_VALIDATED_REG_ECERT_NAME_SYMBOLS)

PEGASUS_NOT_VALIDATED_REG_SCERT_NAME_SYMBOLS = \
   PEGASUS_SCERT_IN_TS_INVLD_USR \
   PEGASUS_SCERT_IN_TS_NO_USR \
   PEGASUS_EXP_SCERT_NOTTS_NO_USR \
   PEGASUS_EXP_SCERT_IN_TS_NO_USR \
   PEGASUS_EXP_SCERT_IN_TS_VLD_USR \
   PEGASUS_EXP_SCERT_IN_TS_INVLD_USR

PEGASUS_NOT_VALIDATED_REG_RACERT_NAME_SYMBOLS =

PEGASUS_NOT_VALIDATED_REG_IACERT_NAME_SYMBOLS =

PEGASUS_NOT_VALIDATED_REG_ECERT_NAME_SYMBOLS = \
   PEGASUS_ECERT_NOTTS_VLD_USR_RA0240 \
   PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240 \
   PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240 \
   PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240

PEGASUS_NOT_VALIDATED_REG_CERT_NAME_SYMBOLS = \
   $(PEGASUS_NOT_VALIDATED_REG_SCERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_VALIDATED_REG_RACERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_VALIDATED_REG_IACERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_VALIDATED_REG_ECERT_NAME_SYMBOLS)

PEGASUS_REG_CERT_NAME_SYMBOLS = \
   $(PEGASUS_VALIDATED_REG_CERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_VALIDATED_REG_CERT_NAME_SYMBOLS)

PEGASUS_NOT_REG_SCERT_NAME_SYMBOLS = \
   PEGASUS_SCERT_NOTTS_NO_USR

PEGASUS_NOT_REG_RACERT_NAME_SYMBOLS =

PEGASUS_NOT_REG_IACERT_NAME_SYMBOLS =

PEGASUS_NOT_REG_ECERT_NAME_SYMBOLS = \
   PEGASUS_ECERT_NOTTS_NO_USR_RA0240 \
   PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240

PEGASUS_NOT_REG_CERT_NAME_SYMBOLS = \
   $(PEGASUS_NOT_REG_SCERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_REG_RACERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_REG_IACERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_REG_ECERT_NAME_SYMBOLS)

PEGASUS_SCERT_NAME_SYMBOLS = \
   $(PEGASUS_VALIDATED_REG_SCERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_VALIDATED_REG_SCERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_REG_SCERT_NAME_SYMBOLS)

PEGASUS_RACERT_NAME_SYMBOLS = \
   $(PEGASUS_VALIDATED_REG_RACERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_VALIDATED_REG_RACERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_REG_RACERT_NAME_SYMBOLS)

PEGASUS_IACERT_NAME_SYMBOLS = \
   $(PEGASUS_VALIDATED_REG_IACERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_VALIDATED_REG_IACERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_REG_IACERT_NAME_SYMBOLS)

PEGASUS_ECERT_NAME_SYMBOLS = \
   $(PEGASUS_VALIDATED_REG_ECERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_VALIDATED_REG_ECERT_NAME_SYMBOLS) \
   $(PEGASUS_NOT_REG_ECERT_NAME_SYMBOLS)

PEGASUS_CERT_NAME_SYMBOLS = \
   $(PEGASUS_SCERT_NAME_SYMBOLS) \
   $(PEGASUS_RACERT_NAME_SYMBOLS) \
   $(PEGASUS_IACERT_NAME_SYMBOLS) \
   $(PEGASUS_ECERT_NAME_SYMBOLS)

###############################################################################
#  Certificate Operations
###############################################################################

createCBATestSSLConfigurationFile:
	@$(RM) $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "[ ca ]" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "default_ca = CA_default" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "[ CA_default ]" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "database = $(PEGASUS_CBA_TEST_CA_DATABASE_FILE)" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "serial = $(PEGASUS_CBA_TEST_CA_SERIAL_FILE)" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "[ policy_any ]" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "countryName = supplied" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "stateOrProvinceName = supplied" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "localityName = supplied" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "organizationName = supplied" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "organizationalUnitName = supplied" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "commonName = supplied" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "[ req ]" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "distinguished_name = req_distinguished_name" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "[ req_distinguished_name  ]" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "countryName=Country Name (2 letter code)" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "countryName_min = 2" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "countryName_max = 2" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "stateOrProvinceName = State or Province Name (full name)" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "localityName = Locality Name (eg, city)" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "organizationName = Organization Name (eg, company)" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "organizationalUnitName  = Organizational Unit Name (eg, section)" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "commonName = Common Name (eg. YOUR name)" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)
	@$(ECHO-E) "commonName_max = 64" \
             >> $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)

###############################################################################
#  The setupTestCerticate option can be used to configure the
#  test certificates used during the OpenPegasus tests.
#
#  make -f TestMakefile setupCBATestCertificates
#
###############################################################################
setupCBATestCertificates:
	$(MKDIRHIER) $(PEGASUS_TEST_CERT_DIR)
	$(RM) $(PEGASUS_CBA_TEST_CA_DATABASE_FILE)
	$(TOUCH) $(PEGASUS_CBA_TEST_CA_DATABASE_FILE)
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
            createCBATestSSLConfigurationFile
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
            createCBATestCertificates
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
            registerInvalidCBATestCertificates
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
            registerValidCBATestCertificates

###############################################################################
deleteCBATestCertificateFiles:
	$(RM) $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).key
	$(RM) $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).csr
	$(RM) $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).cert
	$(RM) $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof
	$(RM) $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)_SERIAL_NO_HEX).pem

deleteCBATestCertificate:
	cimtrust -r \
            -i $($($(TEST_NAME_SYM)_ISSUER_SYM)_SUBJECT) \
            -n $($(TEST_NAME_SYM)_SERIAL_NO)
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
             deleteCBATestCertificateFiles \
             TEST_CERT_NAME=$($(TEST_NAME_SYM))

###############################################################################
#  The deleteCBATestCertificates option removes the registered certificates
#  from the truststore using the cimtrust command.  In addition, it
#  removes any created certificate files from the $(PEGASUS_TEST_CERT_DIR)
#  directory.
#
#  make -f TestMakefile deleteCBATestCertificates
#
#  NOTE: This command ignores all errors when removing the certificates.
#  TODO: Enhance test to ignore only "CIM_ERR_NOT_FOUND" errors.
###############################################################################
deleteCBATestCertificates: FORCE
	$(foreach i, $(PEGASUS_REG_CERT_NAME_SYMBOLS), \
            $(MAKESH) $(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
                deleteCBATestCertificate -i \
                TEST_NAME_SYM=$(i) $(NL))
	$(foreach i, $(PEGASUS_NOT_REG_CERT_NAME_SYMBOLS), \
            $(MAKESH) $(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
                deleteCBATestCertificateFiles \
                TEST_NAME_SYM=$(i) $(NL))

###############################################################################
#  The cleanupCBATestCerticate option can be used to configure the
#  test certificates used during the OpenPegasus tests.
#
#  make -f TestMakefile cleanupCBATestCertificates
#
###############################################################################
cleanupCBATestCertificate_CONFIG_OPTIONS = \
    enableHttpsConnection=true  sslClientVerificationMode=optional
cleanupCBATestCertificate_TESTSUITE_CMDS = \
       $(MAKE)@@--directory=$(PEGASUS_ROOT)@@-f@@TestMakefile@@deleteCBATestCertificates

cleanupCBATestCertificates: FORCE
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile runTestSuite \
        CIMSERVER_CONFIG_OPTIONS="$(cleanupCBATestCertificate_CONFIG_OPTIONS)" \
        TESTSUITE_CMDS="$(cleanupCBATestCertificate_TESTSUITE_CMDS)"
	$(RM) $(PEGASUS_CBA_TEST_CA_DATABASE_FILE)
	$(RM) $(PEGASUS_CBA_TEST_CA_DATABASE_FILE).old
	$(RM) $(PEGASUS_CBA_TEST_CA_DATABASE_FILE).attr
	$(RM) $(PEGASUS_CBA_TEST_CA_DATABASE_FILE).attr.old
	$(RM) $(PEGASUS_CBA_TEST_CA_SERIAL_FILE)
	$(RM) $(PEGASUS_CBA_TEST_CA_SERIAL_FILE).old
	$(RM) $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE)

###############################################################################
#  The runCBATestSuite option can be used to run the
#  Certificate-Based Authentication (CBA) suite of tests.
#
#  make -f TestMakefile runCBATestSuites
#
###############################################################################
runCBATestSuites: FORCE
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile runCBATestConfiguration1
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile runCBATestConfiguration2
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile runCBATestConfiguration3

###############################################################################
#  The displayTestCerticate option uses the "openssl x509" command line tool
#  to display certificate information.  For example,
#
#  make -f TestMakefile displayCBATestCertificate \
#      TEST_CERT_NAME=testCertInTrustStoreValidUser
#
###############################################################################
displayCBATestCertificate: FORCE
	@$(ECHO) "*** $(PEGASUS_TEST_CERT_DIR)/$(TEST_CERT_NAME).cert ***"
	@$(OPENSSL_COMMAND) x509 \
             -in $(PEGASUS_TEST_CERT_DIR)/$(TEST_CERT_NAME).cert -noout -text

###############################################################################
#  The displayCBATestCerticates option uses the "openssl x509" command line
#  tool to display information about the configured OpenPegasus test
#  certificates.
#
#  make -f TestMakefile displayCBATestCertificates
#
###############################################################################
displayCBATestCertificates: FORCE
	@$(foreach i, $(PEGASUS_CERT_NAME_SYMBOLS), \
            $(MAKESH) $(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
                displayCBATestCertificate TEST_CERT_NAME=$($(i)) $(NL))

registerInvalidCBATestCertificates:
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile -i cimstop
	$(foreach i, $(PEGASUS_NOT_VALIDATED_REG_CERT_NAME_SYMBOLS), \
            $(MAKESH) $(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
                registerInvalidCBATestCertificate \
                TEST_NAME_SYM=$(i) \
                TEST_CERT_EXPIRED=$($(i)_EXPIRED) \
                TEST_CERT_IN_TS=$($(i)_IN_TS) $(NL))

###############################################################################
# The registerInvalidCBATestCertificate option is used to register an invalid
# certificate  or a valid certificate with an invalid user.
#    1. Generate mof file to create invalid PG_SSLCertificate instance.
#    2. Use cimmofl to create the instance.
#    3. Place certificate in the cimserver_truststore
###############################################################################

registerInvalidCBATestCertificate:
	@$(RM) $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof
	@$(ECHO-E) "instance of PG_SSLCertificate {" \
             >> $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof
	@$(ECHO-E) "    IssuerName         =" \
             "\"$($($(TEST_NAME_SYM)_ISSUER_SYM)_SUBJECT)\";" \
             >> $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof
	@$(ECHO-E) "    SerialNumber       =" \
             "\"$($(TEST_NAME_SYM)_SERIAL_NO)\";" \
             >> $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof
	@$(ECHO-E) "    SubjectName        =" \
             "\"$($(TEST_NAME_SYM)_SUBJECT)\";" \
             >> $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof
	@$(ECHO-E) "    RegisteredUserName =" \
             "\"$($(TEST_NAME_SYM)_USER_NAME)\";" \
             >> $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof
	@$(ECHO-E) "    TruststoreType     = 2;" \
             >> $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof
	@$(ECHO-E) "    TruststorePath     =" \
         "\"$(PEGASUS_SSL_SERVER_TRUSTSTORE)/$($(TEST_NAME_SYM)_HASH_NAME)\";" \
             >> $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof
ifeq ($(TEST_CERT_EXPIRED), YES)
	@$(ECHO-E) "    NotBefore          =" \
             "\"$(PEGASUS_TEST_INVLD_CIM_NOT_B)\";" \
             >> $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof
	@$(ECHO-E) "    NotAfter           =" \
             "\"$(PEGASUS_TEST_INVLD_CIM_NOT_A)\";" \
             >> $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof
else
	@$(ECHO-E) "    NotBefore          =" \
             "\"$(PEGASUS_TEST_VLD_CIM_NOT_B)\";" \
             >> $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof
	@$(ECHO-E) "    NotAfter           =" \
             "\"$(PEGASUS_TEST_VLD_CIM_NOT_A)\";" \
             >> $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof
endif
	@$(ECHO-E) "};" \
             >> $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof
	cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" \
            "-n$(INTERNALNS)" \
            "$(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).mof"
ifeq ($(TEST_CERT_IN_TS), YES)
	$(COPY) $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).cert \
            $(PEGASUS_SSL_SERVER_TRUSTSTORE)/$($(TEST_NAME_SYM)_HASH_NAME)
endif

signCBATestCertificate:
ifeq ($(TEST_CERT_EXPIRED), YES)
	$(OPENSSL_COMMAND) ca \
            -batch \
            -config $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE) \
            -cert $(PEGASUS_TEST_CERT_DIR)/$($(TEST_CERT_ISSUER_SYM)).cert \
            -keyfile $(PEGASUS_TEST_CERT_DIR)/$($(TEST_CERT_ISSUER_SYM)).key \
            -md md5 \
            -policy policy_any \
            -startdate $(PEGASUS_TEST_INVLD_CERT_NOT_B) \
            -enddate $(PEGASUS_TEST_INVLD_CERT_NOT_A) \
            -outdir $(PEGASUS_TEST_CERT_DIR) \
            -out $(PEGASUS_TEST_CERT_DIR)/$(TEST_CERT_NAME).cert \
            -in $(PEGASUS_TEST_CERT_DIR)/$(TEST_CERT_NAME).csr
else
	$(OPENSSL_COMMAND) ca \
            -batch \
            -config $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE) \
            -cert $(PEGASUS_TEST_CERT_DIR)/$($(TEST_CERT_ISSUER_SYM)).cert \
            -keyfile $(PEGASUS_TEST_CERT_DIR)/$($(TEST_CERT_ISSUER_SYM)).key \
            -md md5 \
            -policy policy_any \
            -startdate $(PEGASUS_TEST_VLD_CERT_NOT_B) \
            -enddate $(PEGASUS_TEST_VLD_CERT_NOT_A) \
            -outdir $(PEGASUS_TEST_CERT_DIR) \
            -out $(PEGASUS_TEST_CERT_DIR)/$(TEST_CERT_NAME).cert \
            -in $(PEGASUS_TEST_CERT_DIR)/$(TEST_CERT_NAME).csr
endif

createSelfSignedCBATestCertificate:
	$(RM) $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).key
	$(RM) $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).csr
	$(RM) $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).cert
	$(RM) $(PEGASUS_CBA_TEST_CA_SERIAL_FILE)
	@$(ECHO-E) "$($(TEST_NAME_SYM)_SERIAL_NO_HEX)" \
             >> $(PEGASUS_CBA_TEST_CA_SERIAL_FILE)
	$(OPENSSL_COMMAND) genrsa -out \
            $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).key 1024
	$(OPENSSL_COMMAND) req -new \
            -config $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE) \
            -subj "$($(TEST_NAME_SYM)_SUBJECT)" \
            -key $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).key \
            -out $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).csr
	$(OPENSSL_COMMAND) x509 \
            -in $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).csr \
            -req -signkey $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).key \
            -out $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).cert
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
            signCBATestCertificate \
            TEST_CERT_NAME=$($(TEST_NAME_SYM)) \
            TEST_CERT_ISSUER_SYM=$($(TEST_NAME_SYM)_ISSUER_SYM) \
            TEST_CERT_EXPIRED=$($(TEST_NAME_SYM)_EXPIRED) \
            TEST_CERT_IN_TS=$($(TEST_NAME_SYM)_IN_TS)

createSelfSignedCACBATestCertificate:
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
            createSelfSignedCBATestCertificate \
            TEST_NAME_SYM=$(TEST_NAME_SYM) \
            TEST_CERT_SET_SERIAL=$(TEST_NAME_SYM)

createCASignedCBATestCertificate:
	$(RM) $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).key
	$(RM) $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).csr
	$(RM) $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).cert
	$(RM) $(PEGASUS_CBA_TEST_CA_SERIAL_FILE)
	@$(ECHO-E) "$($(TEST_NAME_SYM)_SERIAL_NO_HEX)" \
             >> $(PEGASUS_CBA_TEST_CA_SERIAL_FILE)
	$(OPENSSL_COMMAND) genrsa -out \
            $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).key 1024
	$(OPENSSL_COMMAND) req -new \
            -config $(PEGASUS_CBA_TEST_CA_SSL_CNF_FILE) \
            -subj "$($(TEST_NAME_SYM)_SUBJECT)" \
            -key $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).key \
            -out $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).csr
	$(OPENSSL_COMMAND) x509 -req  \
          -in $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).csr \
          $(TEST_CERT_SET_SERIAL) \
          -CA $(PEGASUS_TEST_CERT_DIR)/$($($(TEST_NAME_SYM)_ISSUER_SYM)).cert \
          -CAkey \
             $(PEGASUS_TEST_CERT_DIR)/$($($(TEST_NAME_SYM)_ISSUER_SYM)).key \
          -out $(PEGASUS_TEST_CERT_DIR)/$($(TEST_NAME_SYM)).cert
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
            signCBATestCertificate \
            TEST_CERT_NAME=$($(TEST_NAME_SYM)) \
            TEST_CERT_ISSUER_SYM=$($(TEST_NAME_SYM)_ISSUER_SYM) \
            TEST_CERT_EXPIRED=$($(TEST_NAME_SYM)_EXPIRED) \
            TEST_CERT_IN_TS=$($(TEST_NAME_SYM)_IN_TS) \

registerCBATestCertificate:
ifdef TEST_CERT_USER
	cimtrust -a \
            -f $(PEGASUS_TEST_CERT_DIR)/$(TEST_CERT_NAME).cert \
            -U $(TEST_CERT_USER) \
	    -T $(TEST_CERT_TYPE)
else
	cimtrust -a \
            -f $(PEGASUS_TEST_CERT_DIR)/$(TEST_CERT_NAME).cert \
	    -T $(TEST_CERT_TYPE)
endif

registerValidCBATestCertificates_CONFIG_OPTIONS = \
    enableHttpsConnection=true sslClientVerificationMode=optional \
    enableAuthentication=true
registerValidCBATestCertificates_TESTSUITE_CMDS = \
       $(MAKE)@@--directory=$(PEGASUS_ROOT)@@-f@@TestMakefile@@internal_RegisterValidCBATestCertificates

registerValidCBATestCertificates: FORCE
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile runTestSuite \
            CIMSERVER_CONFIG_OPTIONS= \
                "$(registerValidCBATestCertificates_CONFIG_OPTIONS)" \
            TESTSUITE_CMDS="$(registerValidCBATestCertificates_TESTSUITE_CMDS)"

internal_RegisterValidCBATestCertificates:
	$(foreach i, $(PEGASUS_VALIDATED_REG_CERT_NAME_SYMBOLS), \
            $(MAKESH) $(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
                registerCBATestCertificate \
                TEST_CERT_NAME=$($(i)) \
		TEST_CERT_TYPE=$($(i)_TYPE) \
                TEST_CERT_USER=$($(i)_USER_NAME) $(NL))

###############################################################################
#  The createCBATestCertificates option creates the certificate files and
#  places them in $(PEGASUS_TEST_CERT_DIR) directory.
#
#  make -f TestMakefile createCBATestCertificates
#
###############################################################################
createCBATestCertificates:
ifeq ($(OPENSSL_SET_SERIAL_SUPPORTED),true)
	$(foreach i, $(PEGASUS_SCERT_NAME_SYMBOLS), \
            $(MAKESH) $(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
                createSelfSignedCBATestCertificate \
                TEST_NAME_SYM=$(i) \
                TEST_CERT_SET_SERIAL="-set_serial $($(i)_SERIAL_NO)" $(NL))
	$(foreach i, $(PEGASUS_RACERT_NAME_SYMBOLS), \
            $(MAKESH) $(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
                createSelfSignedCACBATestCertificate \
                TEST_NAME_SYM=$(i) \
                TEST_CERT_SET_SERIAL="-set_serial $($(i)_SERIAL_NO)" $(NL))
	$(foreach i, $(PEGASUS_ECERT_NAME_SYMBOLS), \
            $(MAKESH) $(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
                createCASignedCBATestCertificate \
                TEST_NAME_SYM=$(i) \
                TEST_CERT_SET_SERIAL="-set_serial $($(i)_SERIAL_NO)" $(NL))
else
	$(foreach i, $(PEGASUS_SCERT_NAME_SYMBOLS), \
            $(MAKESH) $(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
                createSelfSignedCBATestCertificate \
                TEST_NAME_SYM=$(i) $(NL))
	$(foreach i, $(PEGASUS_RACERT_NAME_SYMBOLS), \
            $(MAKESH) $(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
                createSelfSignedCACBATestCertificate \
                TEST_NAME_SYM=$(i) $(NL))
	$(foreach i, $(PEGASUS_ECERT_NAME_SYMBOLS), \
            $(MAKESH) $(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile \
                createCASignedCBATestCertificate \
                TEST_NAME_SYM=$(i) $(NL))
endif

###############################################################################
# TestCertClient Parameters
#     Parameter 1: Client Certification File
#     Parameter 2: Client Private Key File
#     Parameter 3: Random Key File
#     Parameter 4: User Name
#     Parameter 5: Password
#     Parameter 6: Expected Result
#     Parameter 7: Expected Identity
###############################################################################

###############################################################################
runCBATestConfiguration1_CONFIG_OPTIONS = \
       enableHttpsConnection=true sslClientVerificationMode=disabled \
       enableAuthentication=true
runCBATestConfiguration1_TESTSUITE_CMDS = \
       $(MAKE)@@--directory=$(PEGASUS_ROOT)@@-f@@TestMakefile@@internal_runCBATestConfiguration1

runCBATestConfiguration1: FORCE
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile runTestSuite \
         CIMSERVER_CONFIG_OPTIONS="$(runCBATestConfiguration1_CONFIG_OPTIONS)" \
         TESTSUITE_CMDS="$(runCBATestConfiguration1_TESTSUITE_CMDS)"

internal_runCBATestConfiguration1: FORCE
#
# TEST ID = 1
# Certificate ID = None
# Certificate: Not Sent
# Username/Password: Not Sent
	@$(ECHO) "Configuration 1: TestID = 1"
	@TestCertClient \
            NONE NONE CONFIG \
            NONE NONE \
            FAIL HTTP_401
#
# TestID = 2
# Certificate ID = None
# Certificate: Not Sent
# Username/Password: Invalid
	@$(ECHO) "Configuration 1: TestID = 2"
	@TestCertClient \
            NONE NONE CONFIG \
            $(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
             FAIL HTTP_401
#
# TestID = 3
# Certificate ID = None
# Certificate: Not Sent
# Username/Password: Valid
	@$(ECHO) "Configuration 1: TestID = 3"
	@TestCertClient \
            NONE NONE CONFIG \
            $(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 4
# Certificate ID = 10
# Certificate: Valid SCERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 1: TestID = 4"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).key \
            CONFIG NONE NONE\
            FAIL HTTP_401
#
# TestID = 5
# Certificate ID = 10
# Certificate: Valid SCERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 1: TestID = 5"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 6
# Certificate ID = 10
# Certificate: Valid SCERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 1: TestID = 6"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 7
# Certificate ID = 60
# Certificate: Valid SCERT, In TS, Invalid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 1: TestID = 7"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).key  \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 8
# Certificate ID = 60
# Certificate: Valid SCERT, In TS, Invalid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 1: TestID = 8"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 9
# Certificate ID = 60
# Certificate: Valid SCERT, In TS, Invalid User
# Username/Password: Valid
	@$(ECHO) "Configuration 1: TestID = 9"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 10
# Certificate ID = 50
# Certificate: Valid SCERT, In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 1: TestID = 10"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            NONE NONE \
            FAIL HTTP_401
#
# TestID = 11
# Certificate ID = 50
# Certificate: Valid SCERT, In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 1: TestID = 11"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 12
# Certificate ID = 50
# Certificate: Valid SCERT, In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 1: TestID = 12"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 13
# Certificate ID = 70
# Certificate: Invalid (EXPIRED) SCERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 1: TestID = 13"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).key \
           CONFIG \
           NONE NONE \
           FAIL HTTP_401
#
# TestID = 14
# Certificate ID = 70
# Certificate: Invalid (EXPIRED) SCERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 1: TestID = 14"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 15
# Certificate ID = 70
# Certificate: Invalid (EXPIRED) SCERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 1: TestID = 15"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 16
# Certificate ID = 100
# Certificate: Invalid (EXPIRED) SCERT, In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 1: TestID = 16"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).key \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 17
# Certificate ID = 100
# Certificate: Invalid (EXPIRED) SCERT, In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 1: TestID = 17"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 18
# Certificate ID = 100
# Certificate: Invalid (EXPIRED) SCERT, In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 1: TestID = 18"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 19
# Certificate ID = 110
# Certificate: Invalid (EXPIRED) SCERT, In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 1: TestID = 19"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).key  \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 20
# Certificate ID = 110
# Certificate: Invalid (EXPIRED) SCERT, In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 1: TestID = 20"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 21
# Certificate ID = 110
# Certificate: Invalid (EXPIRED) SCERT, In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 1: TestID = 21"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 104
# Certificate ID = 610
# Certificate: Valid ECERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 1: TestID = 104"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 105
# Certificate ID = 610
# Certificate: Valid ECERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 1: TestID = 105"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 106
# Certificate ID = 610
# Certificate: Valid ECERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 1: TestID = 106"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 107
# Certificate ID = 630
# Certificate: Valid ECERT, Not In TS, Invalid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 1: TestID = 107"
	@TestCertClient \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).cert \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).key \
         CONFIG NONE NONE \
         FAIL HTTP_401
#
# TestID = 108
# Certificate ID = 630
# Certificate: Valid ECERT, Not In TS, Invalid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 1: TestID = 108"
	@TestCertClient \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).cert \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).key \
         CONFIG \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
         $(PEGASUS_TEST_USER_INVLD_PASS) \
         FAIL HTTP_401
#
# TestID = 109
# Certificate ID = 630
# Certificate: Valid ECERT, Not In TS, Invalid User
# Username/Password: Valid
	@$(ECHO) "Configuration 1: TestID = 109"
	@TestCertClient \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).cert \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).key \
         CONFIG \
         $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
         PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 110
# Certificate ID = 620
# Certificate: Valid ECERT, Not In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 1: TestID = 110"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG NONE NONE \
           FAIL HTTP_401
#
# TestID = 111
# Certificate ID = 620
# Certificate: Valid ECERT, Not In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 1: TestID = 111"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
           $(PEGASUS_TEST_USER_INVLD_PASS) \
           FAIL HTTP_401
#
# TestID = 112
# Certificate ID = 620
# Certificate: Valid ECERT, Not In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 1: TestID = 112"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 113
# Certificate ID = 670
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 1: TestID = 113"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 114
# Certificate ID = 670
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 1: TestID = 114"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 115
# Certificate ID = 670
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 1: TestID = 115"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 116
# Certificate ID = 690
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Invalid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 1: TestID = 116"
	@TestCertClient \
             $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).cert \
             $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).key \
             CONFIG NONE NONE \
             FAIL HTTP_401
#
# TestID = 117
# Certificate ID = 690
# Certificate: Invalid (EXPIREd) ECERT, Not In TS, Invalid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 1: TestID = 117"
	@TestCertClient \
             $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).cert \
             $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).key \
             CONFIG \
             $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
             $(PEGASUS_TEST_USER_INVLD_PASS) \
             FAIL HTTP_401
#
# TestID = 118
# Certificate ID = 690
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Invalid User
# Username/Password: Valid
	@$(ECHO) "Configuration 1: TestID = 118"
	@TestCertClient \
             $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).cert \
             $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).key \
             CONFIG \
             $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
             PASS $(PEGASUS_TEST_USER_ID)

# TestID = 119
# Certificate ID = 680
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 1: TestID = 119"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG NONE NONE \
           FAIL HTTP_401
#
# TestID = 120
# Certificate ID = 680
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 1: TestID = 120"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
           $(PEGASUS_TEST_USER_INVLD_PASS) \
           FAIL HTTP_401
#
# TestID = 121
# Certificate ID = 680
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 1: TestID = 121"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
           PASS $(PEGASUS_TEST_USER_ID)

##############################################################################
ifndef PEGASUS_OVERRIDE_SSL_CERT_VERIFICATION_RESULT
   configurationType=2A
else
   configurationType=2B
endif

runCBATestConfiguration2_CONFIG_OPTIONS = \
      enableHttpsConnection=true sslClientVerificationMode=optional \
      enableAuthentication=true
runCBATestConfiguration2_TESTSUITE_CMDS = \
      $(MAKE)@@--directory=$(PEGASUS_ROOT)@@-f@@TestMakefile@@internal_runCBATestConfiguration$(configurationType)

runCBATestConfiguration2: FORCE
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile runTestSuite \
         CIMSERVER_CONFIG_OPTIONS="$(runCBATestConfiguration2_CONFIG_OPTIONS)" \
         TESTSUITE_CMDS="$(runCBATestConfiguration2_TESTSUITE_CMDS)"

###############################################################################
# PEGASUS_OVERRIDE_SSL_CERT_VERIFICATION_RESULT NOT DEFINED
###############################################################################
internal_runCBATestConfiguration2A: FORCE
#
# TEST ID = 1
# Certificate ID = None
# Certificate: Not Sent
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 1"
	@TestCertClient NONE NONE CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 2
# Certificate ID = None
# Certificate: Not Sent
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 2"
	@TestCertClient NONE NONE CONFIG $(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
             FAIL HTTP_401
#
# TestID = 3
# Certificate ID = None
# Certificate: Not Sent
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 3"
	@TestCertClient NONE NONE CONFIG $(PEGASUS_TEST_USER_ID)\
            $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 4
# Certificate ID = 10
# Certificate: Valid SCERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 4"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).key \
            CONFIG NONE NONE \
            FAIL CANNOT_CONNECT
#
# TestID = 5
# Certificate ID = 10
# Certificate: Valid SCERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 5"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 6
# Certificate ID = 10
# Certificate: Valid SCERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 6"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 7
# Certificate ID = 60
# Certificate: Valid SCERT, In TS, Invalid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 7"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).key  \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 8
# Certificate ID = 60
# Certificate: Valid SCERT, In TS, Invalid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 8"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 9
# Certificate ID = 60
# Certificate: Valid SCERT, In TS, Invalid User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 9"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL HTTP_401
#
# TestID = 10
# Certificate ID = 50
# Certificate: Valid SCERT, In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 10"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            NONE NONE \
            PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 11
# Certificate ID = 50
# Certificate: Valid SCERT, In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 11"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_INVLD_PASS) \
            PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 12
# Certificate ID = 50
# Certificate: Valid SCERT, In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 12"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 13
# Certificate ID = 70
# Certificate: Invalid (EXPIRED) SCERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 13"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).key \
           CONFIG \
           NONE NONE \
           FAIL CANNOT_CONNECT
#
# TestID = 14
# Certificate ID = 70
# Certificate: Invalid (EXPIRED) SCERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 14"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).key \
           CONFIG \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
           $(PEGASUS_TEST_USER_INVLD_PASS) \
           FAIL CANNOT_CONNECT
#
# TestID = 15
# Certificate ID = 70
# Certificate: Invalid (EXPIRED) SCERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 15"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 16
# Certificate ID = 100
# Certificate: Invalid (EXPIRED) SCERT, In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 16"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).key \
            CONFIG NONE NONE \
            FAIL CANNOT_CONNECT
#
# TestID = 17
# Certificate ID = 100
# Certificate: Invalid (EXPIRED) SCERT, In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 17"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 18
# Certificate ID = 100
# Certificate: Invalid (EXPIRED) SCERT, In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 18"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 19
# Certificate ID = 110
# Certificate: Invalid (EXPIRED) SCERT, In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 19"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).key  \
            CONFIG NONE NONE \
            FAIL CANNOT_CONNECT
#
# TestID = 20
# Certificate ID = 110
# Certificate: Invalid (EXPIRED) SCERT, In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 20"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 21
# Certificate ID = 110
# Certificate: Invalid (EXPIRED) SCERT, In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 21"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 104
# Certificate ID = 610
# Certificate: Valid ECERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 104"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 105
# Certificate ID = 610
# Certificate: Valid ECERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 105"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 106
# Certificate ID = 610
# Certificate: Valid ECERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 106"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL HTTP_401
#
# TestID = 107
# Certificate ID = 630
# Certificate: Valid ECERT, Not In TS, Invalid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 107"
	@TestCertClient \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).cert \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).key \
         CONFIG NONE NONE \
         FAIL HTTP_401
#
# TestID = 108
# Certificate ID = 630
# Certificate: Valid ECERT, Not In TS, Invalid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 108"
	@TestCertClient \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).cert \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).key \
         CONFIG \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
         $(PEGASUS_TEST_USER_INVLD_PASS) \
         FAIL HTTP_401
#
# TestID = 109
# Certificate ID = 630
# Certificate: Valid ECERT, Not In TS, Invalid User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 109"
	@TestCertClient \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).cert \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).key \
         CONFIG \
         $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
         FAIL HTTP_401
#
# TestID = 110
# Certificate ID = 620
# Certificate: Valid ECERT, Not In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 110"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG NONE NONE \
           PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 111
# Certificate ID = 620
# Certificate: Valid ECERT, Not In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 111"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
           $(PEGASUS_TEST_USER_INVLD_PASS) \
           PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 112
# Certificate ID = 620
# Certificate: Valid ECERT, Not In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 112"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
           PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 113
# Certificate ID = 670
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 113"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG NONE NONE \
            FAIL CANNOT_CONNECT
#
# TestID = 114
# Certificate ID = 670
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 114"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 115
# Certificate ID = 670
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 115"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 116
# Certificate ID = 690
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Invalid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 116"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).key \
            CONFIG NONE NONE \
            FAIL CANNOT_CONNECT
#
# TestID = 117
# Certificate ID = 690
# Certificate: Invalid (EXPIREd) ECERT, Not In TS, Invalid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 117"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 118
# Certificate ID = 690
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Invalid User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 118"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL CANNOT_CONNECT

# TestID = 119
# Certificate ID = 680
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 119"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG NONE NONE \
           FAIL CANNOT_CONNECT
#
# TestID = 120
# Certificate ID = 680
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 120"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
           $(PEGASUS_TEST_USER_INVLD_PASS) \
           FAIL CANNOT_CONNECT
#
# TestID = 121
# Certificate ID = 680
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 121"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
           FAIL CANNOT_CONNECT

###############################################################################
# PEGASUS_OVERRIDE_SSL_CERT_VERIFICATION_RESULT DEFINED
###############################################################################
internal_runCBATestConfiguration2B: FORCE
#
# TEST ID = 1
# Certificate ID = None
# Certificate: Not Sent
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 1"
	@TestCertClient NONE NONE CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 2
# Certificate ID = None
# Certificate: Not Sent
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 2"
	@TestCertClient NONE NONE CONFIG $(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
             FAIL HTTP_401
#
# TestID = 3
# Certificate ID = None
# Certificate: Not Sent
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 3"
	@TestCertClient NONE NONE CONFIG $(PEGASUS_TEST_USER_ID)\
            $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 4
# Certificate ID = 10
# Certificate: Valid SCERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 4"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).key \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 5
# Certificate ID = 10
# Certificate: Valid SCERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 5"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 6
# Certificate ID = 10
# Certificate: Valid SCERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 6"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 7
# Certificate ID = 60
# Certificate: Valid SCERT, In TS, Invalid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 7"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).key  \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 8
# Certificate ID = 60
# Certificate: Valid SCERT, In TS, Invalid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 8"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 9
# Certificate ID = 60
# Certificate: Valid SCERT, In TS, Invalid User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 9"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL HTTP_401
#
# TestID = 10
# Certificate ID = 50
# Certificate: Valid SCERT, In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 10"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            NONE NONE \
            PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 11
# Certificate ID = 50
# Certificate: Valid SCERT, In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 11"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_INVLD_PASS) \
            PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 12
# Certificate ID = 50
# Certificate: Valid SCERT, In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 12"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 13
# Certificate ID = 70
# Certificate: Invalid (EXPIRED) SCERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 13"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).key \
           CONFIG \
           NONE NONE \
           FAIL HTTP_401
#
# TestID = 14
# Certificate ID = 70
# Certificate: Invalid (EXPIRED) SCERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 14"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 15
# Certificate ID = 70
# Certificate: Invalid (EXPIRED) SCERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 15"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 16
# Certificate ID = 100
# Certificate: Invalid (EXPIRED) SCERT, In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 16"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).key \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 17
# Certificate ID = 100
# Certificate: Invalid (EXPIRED) SCERT, In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 17"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 18
# Certificate ID = 100
# Certificate: Invalid (EXPIRED) SCERT, In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 18"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 19
# Certificate ID = 110
# Certificate: Invalid (EXPIRED) SCERT, In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 19"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).key  \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 20
# Certificate ID = 110
# Certificate: Invalid (EXPIRED) SCERT, In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 20"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 21
# Certificate ID = 110
# Certificate: Invalid (EXPIRED) SCERT, In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 21"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 104
# Certificate ID = 610
# Certificate: Valid ECERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 104"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 105
# Certificate ID = 610
# Certificate: Valid ECERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 105"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 106
# Certificate ID = 610
# Certificate: Valid ECERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 106"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL HTTP_401
#
# TestID = 107
# Certificate ID = 630
# Certificate: Valid ECERT, Not In TS, Invalid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 107"
	@TestCertClient \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).cert \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).key \
         CONFIG NONE NONE \
         FAIL HTTP_401
#
# TestID = 108
# Certificate ID = 630
# Certificate: Valid ECERT, Not In TS, Invalid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 108"
	@TestCertClient \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).cert \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).key \
         CONFIG \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
         $(PEGASUS_TEST_USER_INVLD_PASS) \
         FAIL HTTP_401
#
# TestID = 109
# Certificate ID = 630
# Certificate: Valid ECERT, Not In TS, Invalid User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 109"
	@TestCertClient \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).cert \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).key \
         CONFIG \
         $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
         FAIL HTTP_401
#
# TestID = 110
# Certificate ID = 620
# Certificate: Valid ECERT, Not In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 110"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG NONE NONE \
           PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 111
# Certificate ID = 620
# Certificate: Valid ECERT, Not In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 111"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
           $(PEGASUS_TEST_USER_INVLD_PASS) \
           PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 112
# Certificate ID = 620
# Certificate: Valid ECERT, Not In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 1: TestID = 112"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
           PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 113
# Certificate ID = 670
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 113"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 114
# Certificate ID = 670
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 114"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 115
# Certificate ID = 670
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 115"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_USER_ID)
#
# TestID = 116
# Certificate ID = 690
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Invalid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 116"
	@TestCertClient \
             $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).cert \
             $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).key \
             CONFIG NONE NONE \
             FAIL HTTP_401
#
# TestID = 117
# Certificate ID = 690
# Certificate: Invalid (EXPIREd) ECERT, Not In TS, Invalid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 117"
	@TestCertClient \
             $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).cert \
             $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).key \
             CONFIG \
             $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
             $(PEGASUS_TEST_USER_INVLD_PASS) \
             FAIL HTTP_401
#
# TestID = 118
# Certificate ID = 690
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Invalid User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 118"
	@TestCertClient \
             $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).cert \
             $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).key \
             CONFIG \
             $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
             PASS $(PEGASUS_TEST_USER_ID)

# TestID = 119
# Certificate ID = 680
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 2: TestID = 119"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG NONE NONE \
           FAIL HTTP_401
#
# TestID = 120
# Certificate ID = 680
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 2: TestID = 120"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
           $(PEGASUS_TEST_USER_INVLD_PASS) \
           FAIL HTTP_401
#
# TestID = 121
# Certificate ID = 680
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 2: TestID = 121"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
           PASS $(PEGASUS_TEST_USER_ID)

###############################################################################
# Configuration 3
# enableAuthentication=true
# enableHttpsConnection=true
# sslClientVerificationMode = required
###############################################################################
runCBATestConfiguration3_CONFIG_OPTIONS = \
       enableHttpsConnection=true sslClientVerificationMode=required \
       enableAuthentication=true
runCBATestConfiguration3_TESTSUITE_CMDS = \
       $(MAKE)@@--directory=$(PEGASUS_ROOT)@@-f@@TestMakefile@@internal_runCBATestConfiguration3

runCBATestConfiguration3: FORCE
	$(MAKE) --directory=$(PEGASUS_ROOT) -f TestMakefile runTestSuite \
         CIMSERVER_CONFIG_OPTIONS="$(runCBATestConfiguration3_CONFIG_OPTIONS)" \
         TESTSUITE_CMDS="$(runCBATestConfiguration3_TESTSUITE_CMDS)"

internal_runCBATestConfiguration3: FORCE
#
# TEST ID = 1
# Certificate ID = None
# Certificate: Not Sent
# Username/Password: Not Sent
	@$(ECHO) "Configuration 3: TestID = 1"
	@TestCertClient NONE NONE CONFIG NONE NONE \
            FAIL CANNOT_CONNECT
#
# TestID = 2
# Certificate ID = None
# Certificate: Not Sent
# Username/Password: Invalid
	@$(ECHO) "Configuration 3: TestID = 2"
	@TestCertClient NONE NONE CONFIG $(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 3
# Certificate ID = None
# Certificate: Not Sent
# Username/Password: Valid
	@$(ECHO) "Configuration 3: TestID = 3"
	@TestCertClient NONE NONE CONFIG $(PEGASUS_TEST_USER_ID)\
            $(PEGASUS_TEST_USER_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 4
# Certificate ID = 10
# Certificate: Valid SCERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 3: TestID = 4"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).key \
            CONFIG NONE NONE \
            FAIL CANNOT_CONNECT
#
# TestID = 5
# Certificate ID = 10
# Certificate: Valid SCERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 3: TestID = 5"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 6
# Certificate ID = 10
# Certificate: Valid SCERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 3: TestID = 6"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 7
# Certificate ID = 60
# Certificate: Valid SCERT, In TS, Invalid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 3: TestID = 7"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).key  \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 8
# Certificate ID = 60
# Certificate: Valid SCERT, In TS, Invalid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 3: TestID = 8"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 9
# Certificate ID = 60
# Certificate: Valid SCERT, In TS, Invalid User
# Username/Password: Valid
	@$(ECHO) "Configuration 3: TestID = 9"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_INVLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL HTTP_401
#
# TestID = 10
# Certificate ID = 50
# Certificate: Valid SCERT, In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 3: TestID = 10"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            NONE NONE \
            PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 11
# Certificate ID = 50
# Certificate: Valid SCERT, In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 3: TestID = 11"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_INVLD_PASS) \
            PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 12
# Certificate ID = 50
# Certificate: Valid SCERT, In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 3: TestID = 12"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 13
# Certificate ID = 70
# Certificate: Invalid (EXPIRED) SCERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 3: TestID = 13"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).key \
           CONFIG \
           NONE NONE \
           FAIL CANNOT_CONNECT
#
# TestID = 14
# Certificate ID = 70
# Certificate: Invalid (EXPIRED) SCERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 3: TestID = 14"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 15
# Certificate ID = 70
# Certificate: Invalid (EXPIRED) SCERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 3: TestID = 15"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_NOTTS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 16
# Certificate ID = 100
# Certificate: Invalid (EXPIRED) SCERT, In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 3: TestID = 16"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).key \
            CONFIG NONE NONE \
            FAIL CANNOT_CONNECT
#
# TestID = 17
# Certificate ID = 100
# Certificate: Invalid (EXPIRED) SCERT, In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 3: TestID = 17"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 18
# Certificate ID = 100
# Certificate: Invalid (EXPIRED) SCERT, In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 3: TestID = 18"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_NO_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 19
# Certificate ID = 110
# Certificate: Invalid (EXPIRED) SCERT, In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 3: TestID = 19"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).key  \
            CONFIG NONE NONE \
            FAIL CANNOT_CONNECT
#
# TestID = 20
# Certificate ID = 110
# Certificate: Invalid (EXPIRED) SCERT, In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 3: TestID = 20"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 21
# Certificate ID = 110
# Certificate: Invalid (EXPIRED) SCERT, In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 3: TestID = 21"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_SCERT_IN_TS_VLD_USR).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 104
# Certificate ID = 610
# Certificate: Valid ECERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 3: TestID = 104"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG NONE NONE \
            FAIL HTTP_401
#
# TestID = 105
# Certificate ID = 610
# Certificate: Valid ECERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 3: TestID = 105"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL HTTP_401
#
# TestID = 106
# Certificate ID = 610
# Certificate: Valid ECERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 3: TestID = 106"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL HTTP_401
#
# TestID = 107
# Certificate ID = 630
# Certificate: Valid ECERT, Not In TS, Invalid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 3: TestID = 107"
	@TestCertClient \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).cert \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).key \
         CONFIG NONE NONE \
         FAIL HTTP_401
#
# TestID = 108
# Certificate ID = 630
# Certificate: Valid ECERT, Not In TS, Invalid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 3: TestID = 108"
	@TestCertClient \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).cert \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).key \
         CONFIG \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
         $(PEGASUS_TEST_USER_INVLD_PASS) \
         FAIL HTTP_401
#
# TestID = 109
# Certificate ID = 630
# Certificate: Valid ECERT, Not In TS, Invalid User
# Username/Password: Valid
	@$(ECHO) "Configuration 3: TestID = 109"
	@TestCertClient \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).cert \
         $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_INVLD_USR_RA0240).key \
         CONFIG \
         $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
         FAIL HTTP_401
#
# TestID = 110
# Certificate ID = 620
# Certificate: Valid ECERT, Not In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 3: TestID = 110"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG NONE NONE \
           PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 111
# Certificate ID = 620
# Certificate: Valid ECERT, Not In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 3: TestID = 111"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
           $(PEGASUS_TEST_USER_INVLD_PASS) \
           PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 112
# Certificate ID = 620
# Certificate: Valid ECERT, Not In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 3: TestID = 112"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
           PASS $(PEGASUS_TEST_CERT_USER_ID)
#
# TestID = 113
# Certificate ID = 670
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, No User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 3: TestID = 113"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG NONE NONE \
            FAIL CANNOT_CONNECT
#
# TestID = 114
# Certificate ID = 670
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, No User
# Username/Password: Invalid
	@$(ECHO) "Configuration 3: TestID = 114"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 115
# Certificate ID = 670
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, No User
# Username/Password: Valid
	@$(ECHO) "Configuration 3: TestID = 115"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_NO_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 116
# Certificate ID = 690
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Invalid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 3: TestID = 116"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).key \
            CONFIG NONE NONE \
            FAIL CANNOT_CONNECT
#
# TestID = 117
# Certificate ID = 690
# Certificate: Invalid (EXPIREd) ECERT, Not In TS, Invalid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 3: TestID = 117"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
            $(PEGASUS_TEST_USER_INVLD_PASS) \
            FAIL CANNOT_CONNECT
#
# TestID = 118
# Certificate ID = 690
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Invalid User
# Username/Password: Valid
	@$(ECHO) "Configuration 3: TestID = 118"
	@TestCertClient \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).cert \
            $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_INVLD_USR_RA0240).key \
            CONFIG \
            $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
            FAIL CANNOT_CONNECT

# TestID = 119
# Certificate ID = 680
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Valid User
# Username/Password: Not Sent
	@$(ECHO) "Configuration 3: TestID = 119"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG NONE NONE \
           FAIL CANNOT_CONNECT
#
# TestID = 120
# Certificate ID = 680
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Valid User
# Username/Password: Invalid
	@$(ECHO) "Configuration 3: TestID = 120"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_TEST_USER_ID) \
           $(PEGASUS_TEST_USER_INVLD_PASS) \
           FAIL CANNOT_CONNECT
#
# TestID = 121
# Certificate ID = 680
# Certificate: Invalid (EXPIRED) ECERT, Not In TS, Valid User
# Username/Password: Valid
	@$(ECHO) "Configuration 3: TestID = 121"
	@TestCertClient \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).cert \
           $(PEGASUS_TEST_CERT_DIR)/$(PEGASUS_EXP_ECERT_NOTTS_VLD_USR_RA0240).key \
           CONFIG \
           $(PEGASUS_TEST_USER_ID) $(PEGASUS_TEST_USER_PASS) \
           FAIL CANNOT_CONNECT

endif
endif
endif
