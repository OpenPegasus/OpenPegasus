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
JAVAROOT = ../../..
ROOT = ../../../../../../..

include $(ROOT)/mak/config.mak

.SUFFIXES: .java .class .jar

.java.class:
	$(PEGASUS_JAVA_COMPILER) -classpath $(JAVAROOT) $*.java

PACKAGEDIR = org/pegasus/jmpi

CLASSES = \
	CIMClass.class \
	CIMInstance.class \
	CIMDataType.class \
	CIMDateTime.class \
	CIMElement.class \
	CIMException.class \
	CIMInstanceException.class \
	CIMInstance.class \
	CIMMethod.class \
	CIMNameSpace.class \
	CIMObject.class \
	CIMObjectPath.class \
	CIMOMHandle.class \
	ProviderCIMOMHandle.class \
	CIMProperty.class \
	CIMValue.class \
	CIMQualifier.class \
	CIMQualifierType.class \
	CIMDataType.class \
	CIMFlavor.class \
	CIMScope.class \
	CIMArgument.class \
	\
	OperationContext.class \
	\
	CIMProvider.class \
	CIMInstanceProvider.class \
	CIMInstanceProvider2.class \
	CIMAssociatorProvider.class \
	CIMAssociatorProvider2.class \
	CIMMethodProvider.class \
	CIMMethodProvider2.class \
	\
	InstanceProvider.class \
	InstanceProvider2.class \
	MethodProvider.class \
	MethodProvider2.class \
	AssociatorProvider.class \
	AssociatorProvider2.class \
	EventProvider.class \
	EventProvider2.class \
	PropertyProvider.class \
	PropertyProvider2.class \
	\
	SelectExp.class \
	FromExp.class \
	NonJoinExp.class \
	SelectList.class \
	JMPISelectList.class \
	QueryExp.class \
	JMPIQueryExp.class \
	WQLExp.class \
	\
	UnsignedNumber.class \
	UnsignedInt16.class \
	UnsignedInt32.class \
	UnsignedInt64.class \
	UnsignedInt8.class \
	\
	CIMNameSpace.class \
	CIMClient.class \
	ClassEnumeration.class \
	InstEnumeration.class \
	PathEnumeration.class \
	QualEnumeration.class \
	\
	JarClassLoader.class \
	\
	CIMParameter.class

PFXCLASSES=$(addprefix -C ../../.. org/pegasus/jmpi/,$(CLASSES))

all build: $(PEGASUS_HOME)/lib/JMPIImpl.jar $(CLASSES)

depend:

clean:
	-$(RM) *.class
	-$(RM) $(PEGASUS_HOME)/lib/JMPIImpl.jar

$(PEGASUS_HOME)/lib/JMPIImpl.jar: $(CLASSES)
	$(PEGASUS_JAVA_JAR) -cf $@ $(PFXCLASSES)

repository tests poststarttests:
