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
ROOT = ../../..

PEGASUS_ZOS_PROGRAM_OBJECT = yes

DIR = Clients/TestClient
include $(ROOT)/mak/config.mak

LIBRARIES = \
    pegclient \
    peggeneral \
    pegcommon

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY

PROGRAM = TestClient
SOURCES = TestClient.cpp

SUPPORT_IPV4_MAPPED_IPV6 = false

ifeq ($(OS_TYPE), unix)
SUPPORT_IPV4_MAPPED_IPV6 = true
endif

ifeq ($(OS_TYPE), vms)
SUPPORT_IPV4_MAPPED_IPV6 = true
endif

ifeq ($(PEGASUS_ENABLE_SLP),true)
     LIBRARIES += pegslp
endif

include $(ROOT)/mak/program.mak

tests:

poststarttests:
	$(PROGRAM)

# IPv6 Tests.

ifeq ($(PEGASUS_TEST_IPV6), true)
	@$(ECHO) "Testing with IPv6 loop-back address."
	$(PROGRAM) [::1]:5988
endif

#
# Testing for ipv4 mapped ipv6 addresses.
# ATTN: These tests are not avilable on windows platforms as windows does not
# support ipv4 mapped ipv6 addresses in XP and 2003. At present these tests
# are enabled only on Unix platforms.
#

ifeq ($(PEGASUS_TEST_IPV6), true)
ifeq ($(SUPPORT_IPV4_MAPPED_IPV6), true)
	@$(ECHO) "Testing with IPv4 Mapped IPv6 Addresses."
	$(PROGRAM) [::ffff:127.0.0.1]:5988
endif
endif
