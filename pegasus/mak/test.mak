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

# The following are the user id's and passwords to be used by the tests.
# If they are not defined in the environment then they will be assigned
# here to the defaults. There are two user ID's and the coresponding
# passwords. One is for a reqular user that does not require priviledged
# (root) access and the other set for a user that has privileged (root) access.
#
# Use the folowing in a test that does not require privileged access
#
#        PEGASUS_TEST_USER_ID
#        PEGASUS_TEST_USER_PASS
#
# Use the following in test that requires a privileged user's access
#
#       PEGASUS_TEST_PRIV_USER_ID
#       PEGASUS_TEST_PRIV_USER_PASS
#
# Currently PEGASUS_TEST_USER_ID and PEGASUS_TEST_USER_PASS both default to
# guest if they are not specified within the users environment.
#
# Currently (Feb. 10, 2006) PEGASUS_TEST_PRIV_USER_ID and
# PEGASUS_TEST_PRIV_USER_PASS are not used and therefore they have no
# default value. When they are taken into use it may be desirable to have
# a default value that will never be used to make errors when they are not
# assigned a valid value more meaningful. An error of the form
# "guestroot user does not exist" rather than "user does not exist"
# would make it easier to find the problem because "guestroot" could be
# found in the tree and determined to come from the default value of
# PEGASUS_TEST_PRIVUSER_ID.       JR Wunderlich Feb 10, 2006
#

ifndef PEGASUS_TEST_USER_ID
PEGASUS_TEST_USER_ID=guest
endif

ifndef PEGASUS_TEST_USER_PASS
PEGASUS_TEST_USER_PASS=guest
endif

# ifndef PEGASUS_TEST_PRIV_USER_ID
# PEGASUS_TEST_PRIV_USER_ID=guestroot
# endif

# ifndef PEGASUS_TEST_PRIV_USER_PASS
# PEGASUS_TEST_PRIV_USER_PASS=guestroot
# endif




ifeq ($(OS_TYPE),windows)
    STRIPCRS = stripcrs $(RESULTFILE) $(MASTERRESULTFILE)
else
    ifeq  ($(OS_TYPE), vms)
        STRIPCRS = stripcrs $(RESULTFILE) $(MASTERRESULTFILE)
    else
        STRIPCRS =
    endif
endif

STRIPL = stripline

MU = mu

REDIRECTERROR = 2>&1


ifdef DIFF
    COMPARERESULTS = @$(DIFF) $(MASTERRESULTFILE) $(RESULTFILE)
    COMPARERESULTS_DS = @$(call DIFFSORT,$(MASTERRESULTFILE),$(RESULTFILE))
else
    COMPARERESULTS = @$(ECHO) "Results not compared."
    COMPARERESULTS_DS = @$(ECHO) "Results not compared."
endif

ifdef PEGASUS_TMP
    TMP_DIR = $(subst \,/,$(PEGASUS_TMP))
else
    TMP_DIR = .
endif

default:

#l10n
messages: $(ERROR)
