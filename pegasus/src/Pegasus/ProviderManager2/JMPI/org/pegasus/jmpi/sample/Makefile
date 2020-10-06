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

ROOT = ../../../../../../../..

include $(ROOT)/mak/config.mak

.SUFFIXES:
.SUFFIXES: .java .class

.java.class:
	$(PEGASUS_JAVA_COMPILER) -classpath $(PEGASUS_HOME)/lib/JMPIImpl.jar $*.java

PACKAGEDIR = org/pegasus/jmpi/sample/

JAR1        = TestJavaProvider.jar
JAR1CLASSES = \
   TestJavaProvider.class
JAR2        = TestJavaProviderA.jar
JAR2CLASSES = \
   TestJavaProviderA.class
JAR3        = TestJavaProviderB.jar
JAR3CLASSES = \
   TestJavaProviderB.class
JARS        = $(JAR1) $(JAR2) $(JAR3)

all build: $(addprefix $(PEGASUS_HOME)/lib/,$(JARS))

depend:

clean:
	$(RM) *.class
	$(RM) $(JARS)

repository:
	cimmofl -I. -nroot/cimv2 $(subst .jar,,$(JAR1)).mof
	cimmofl -I. -n$(PEGASUS_INTEROP_NAMESPACE) $(subst .jar,,$(JAR1))R.mof
	cimmofl -I. -nroot/cimv2 $(subst .jar,,$(JAR2)).mof
	cimmofl -I. -n$(PEGASUS_INTEROP_NAMESPACE) $(subst .jar,,$(JAR2))R.mof
	cimmofl -I. -nroot/cimv2 $(subst .jar,,$(JAR3)).mof
	cimmofl -I. -n$(PEGASUS_INTEROP_NAMESPACE) $(subst .jar,,$(JAR3))R.mof

$(PEGASUS_HOME)/lib/$(JAR1): $(JAR1CLASSES)
	$(PEGASUS_JAVA_JAR) -cf $@ $(addprefix -C ../../../.. $(PACKAGEDIR),$^)

$(PEGASUS_HOME)/lib/$(JAR2): $(JAR2CLASSES)
	$(PEGASUS_JAVA_JAR) -cf $@ $(addprefix -C ../../../.. $(PACKAGEDIR),$^)

$(PEGASUS_HOME)/lib/$(JAR3): $(JAR3CLASSES)
	$(PEGASUS_JAVA_JAR) -cf $@ $(addprefix -C ../../../.. $(PACKAGEDIR),$^)
