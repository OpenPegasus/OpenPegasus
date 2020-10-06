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

ROOT = ../../../../../../../../..

include $(ROOT)/mak/config.mak
include $(ROOT)/mak/configschema.mak
include $(ROOT)/test/config.mak

.SUFFIXES: .java .class .jar

.java.class:
# Note: .. included in path to compile package helper classes if necessary
	$(PEGASUS_JAVA_COMPILER) -classpath $(PEGASUS_HOME)/lib/JMPIImpl.jar$(PEGASUS_JAVA_CLASSPATH_DELIMITER).. $*.java

KNOWN_BUGS       = BugExistsExecQuery

PACKAGEDIR       = Client/
JAR1             = JMPITestClient.jar
JAR1CLASSES      = \
	RunTest.class \
	TestEntry.class
EXTRAJAR1CLASSES = \
        MyFilenameFilter.class
JAR1MANIFEST     = JMPITestClient.manifest
JAR2             = JMPITestClientTests.jar
JAR2CLASSES      = \
	testDataType.class \
	testExecQuery.class \
	testIndications.class \
	testPropertyProvider.class \
	testCIMClient.class \
	testCIMArgument.class \
	testCIMClass.class \
	testCIMDataType.class \
	testCIMDateTime.class \
	testCIMInstance.class \
	testCIMMethod.class \
	testCIMObjectPath.class \
	testCIMParameter.class \
	testCIMProperty.class \
	testCIMQualifier.class \
	testCIMQualifier.class \
	testUnsignedInt8.class \
	testUnsignedInt16.class \
	testUnsignedInt32.class \
	testUnsignedInt64.class
EXTRAJAR2CLASSES =
JARS             = $(JAR1) $(JAR2)

LOCAL_CLEAN_TARGET = \
	$(JAR1CLASSES) $(EXTRAJAR1CLASSES) $(addprefix $(PEGASUS_HOME)/lib/,$(JAR1)) \
	$(JAR2CLASSES) $(EXTRAJAR2CLASSES) $(addprefix $(PEGASUS_HOME)/lib/,$(JAR2))

all build: 			$(addprefix $(PEGASUS_HOME)/lib/,$(JARS))

depend:

clean:
	$(RM) $(LOCAL_CLEAN_TARGET)

repository:

tests:

poststarttests:			$(addprefix $(PEGASUS_HOME)/lib/,$(JARS))
	$(PEGASUS_JAVA_INTERPRETER) -classpath $(PEGASUS_HOME)/lib/JMPIImpl.jar$(PEGASUS_JAVA_CLASSPATH_DELIMITER)$(addprefix $(PEGASUS_HOME)/lib/,$(JAR1))$(PEGASUS_JAVA_CLASSPATH_DELIMITER)$(addprefix $(PEGASUS_HOME)/lib/,$(JAR2)) Client.RunTest $(KNOWN_BUGS)

$(PEGASUS_HOME)/lib/$(JAR1):	$(JAR1CLASSES) $(JAR1MANIFEST)
	$(PEGASUS_JAVA_JAR) -cfm $@ $(JAR1MANIFEST) $(addprefix -C .. $(PACKAGEDIR),$^) $(addprefix -C .. $(PACKAGEDIR),$(EXTRAJAR1CLASSES))

$(PEGASUS_HOME)/lib/$(JAR2):	$(JAR2CLASSES)
	$(PEGASUS_JAVA_JAR) -cf $@ $(addprefix -C .. $(PACKAGEDIR),$^) $(addprefix -C .. $(PACKAGEDIR),$(EXTRAJAR2CLASSES))

#include $(ROOT)/test/configend.mak
