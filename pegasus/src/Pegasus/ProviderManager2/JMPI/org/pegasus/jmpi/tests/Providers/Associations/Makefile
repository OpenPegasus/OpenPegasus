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

ROOT = ../../../../../../../../../..

include $(ROOT)/mak/config.mak
include $(ROOT)/mak/configschema.mak
include $(ROOT)/test/config.mak

.SUFFIXES: .java .class .jar

.java.class:
	$(PEGASUS_JAVA_COMPILER) -classpath $(PEGASUS_HOME)/lib/JMPIImpl.jar $*.java

NAMESPACE          = root/SampleProvider
PACKAGEDIR         = Associations/
JAR1               = JMPIExpAssociationProvider.jar
JAR1CLASSES        = \
	JMPIExpAssociationProvider.class
EXTRAJAR1CLASSES   =
JAR2               = JMPIAssociationProvider.jar
JAR2CLASSES        = \
	JMPIAssociationProvider.class
EXTRAJAR2CLASSES   =
JARS               = $(JAR1) $(JAR2)

LOCAL_CLEAN_TARGET = \
	$(JAR1CLASSES) $(EXTRAJAR1CLASSES) $(addprefix $(PEGASUS_HOME)/lib/,$(JAR1)) \
	$(JAR2CLASSES) $(EXTRAJAR2CLASSES) $(addprefix $(PEGASUS_HOME)/lib/,$(JAR2))

QUOTE=\"

ifeq ($(OS_TYPE),vms)
QUOTE=""
endif

all build: 			$(addprefix $(PEGASUS_HOME)/lib/,$(JARS))

depend:

clean:
	$(RM) $(LOCAL_CLEAN_TARGET)

repository:			cimmofl_repository

cimmofl_repository:
	@$(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(NAMESPACE) namespace ...
	@cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(NAMESPACE)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@$(ECHO) +++++ Loading CIM_Event$(CIM_SCHEMA_VER) into $(NAMESPACE) namespace ...
	@cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" "-I$(CIM_SCHEMA_DIR)" "-n$(NAMESPACE)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Event$(CIM_SCHEMA_VER).mof
	@$(ECHO) +++++ Loading $(subst .jar,,$(JAR1)) into $(NAMESPACE) namespace ...
	@cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" -I. "-n$(NAMESPACE)" $(subst .jar,,$(JAR1)).mof
	@cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" -I. "-n$(INTEROPNS)" $(subst .jar,,$(JAR1))R.mof
	@$(ECHO) +++++ Loading $(subst .jar,,$(JAR2)) into $(NAMESPACE) namespace ...
	@cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" -I. "-n$(NAMESPACE)" $(subst .jar,,$(JAR2)).mof
	@cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" -I. "-n$(INTEROPNS)" $(subst .jar,,$(JAR2))R.mof

cimmof_repository:
	@$(ECHO) +++++ Loading CIM_Core$(CIM_SCHEMA_VER) into $(NAMESPACE) namespace ...
	@cimmof "-I$(CIM_SCHEMA_DIR)" "-n$(NAMESPACE)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Core$(CIM_SCHEMA_VER).mof
	@$(ECHO) +++++ Loading CIM_Event$(CIM_SCHEMA_VER) into $(NAMESPACE) namespace ...
	@cimmof "-I$(CIM_SCHEMA_DIR)" "-n$(NAMESPACE)" $(ALLOW_EXPERIMENTAL) $(CIM_SCHEMA_DIR)/CIM_Event$(CIM_SCHEMA_VER).mof
	@$(ECHO) +++++ Loading $(subst .jar,,$(JAR1)) into $(NAMESPACE) namespace ...
	@cimmof -I. "-n$(NAMESPACE)" $(subst .jar,,$(JAR1)).mof
	@cimmof -I. "-n$(INTEROPNS)" $(subst .jar,,$(JAR1))R.mof
	@$(ECHO) +++++ Loading $(subst .jar,,$(JAR2)) into $(NAMESPACE) namespace ...
	@cimmof -I. "-n$(NAMESPACE)" $(subst .jar,,$(JAR2)).mof
	@cimmof -I. "-n$(INTEROPNS)" $(subst .jar,,$(JAR2))R.mof

tests:

poststarttests:			$(addprefix $(PEGASUS_HOME)/lib/,$(JARS))
	@cimcli a  --sum -count 1 -n $(NAMESPACE) "JMPIExpAssociation_Teacher"
	@cimcli an --sum -count 1 -n $(NAMESPACE) "JMPIExpAssociation_Teacher"
	@cimcli r  --sum -count 2 -n $(NAMESPACE) "JMPIExpAssociation_Teacher"
	@cimcli rn --sum -count 2 -n $(NAMESPACE) "JMPIExpAssociation_Teacher"
	@cimcli a  --sum -count 3 -n $(NAMESPACE) -r Teaches "JMPIExpAssociation_Teacher.Name=$(QUOTE)Teacher1$(QUOTE),Identifier=1"
	@cimcli an --sum -count 3 -n $(NAMESPACE) -r Teaches "JMPIExpAssociation_Teacher.Name=$(QUOTE)Teacher1$(QUOTE),Identifier=1"
	@cimcli r  --sum -count 1 -n $(NAMESPACE) -r Teaches JMPIExpAssociation_Teacher
	@cimcli rn --sum -count 3 -n $(NAMESPACE) -r Teaches "JMPIExpAssociation_Teacher.Name=$(QUOTE)Teacher1$(QUOTE),Identifier=1"

	@cimcli a  --sum -count 1 -n $(NAMESPACE) "JMPIExpAssociation_Student"
	@cimcli an --sum -count 1 -n $(NAMESPACE) "JMPIExpAssociation_Student"
	@cimcli r  --sum -count 2 -n $(NAMESPACE) "JMPIExpAssociation_Student"
	@cimcli rn --sum -count 2 -n $(NAMESPACE) "JMPIExpAssociation_Student"
	@cimcli a  --sum -count 3 -n $(NAMESPACE) -r TaughtBy "JMPIExpAssociation_Student.Name=$(QUOTE)Student1$(QUOTE),Identifier=1"
	@cimcli an --sum -count 3 -n $(NAMESPACE) -r TaughtBy "JMPIExpAssociation_Student.Name=$(QUOTE)Student1$(QUOTE),Identifier=1"
	@cimcli r  --sum -count 1 -n $(NAMESPACE) -r TaughtBy JMPIExpAssociation_Student
	@cimcli rn --sum -count 3 -n $(NAMESPACE) -r TaughtBy "JMPIExpAssociation_Student.Name=$(QUOTE)Student1$(QUOTE),Identifier=1"

	@cimcli a  --sum -count 1 -n $(NAMESPACE) "JMPIAssociation_Teacher"
	@cimcli an --sum -count 1 -n $(NAMESPACE) "JMPIAssociation_Teacher"
	@cimcli r  --sum -count 2 -n $(NAMESPACE) "JMPIAssociation_Teacher"
	@cimcli rn --sum -count 2 -n $(NAMESPACE) "JMPIAssociation_Teacher"
	@cimcli a  --sum -count 3 -n $(NAMESPACE) -r Teaches "JMPIAssociation_Teacher.Name=$(QUOTE)Teacher1$(QUOTE),Identifier=1"
	@cimcli an --sum -count 3 -n $(NAMESPACE) -r Teaches "JMPIAssociation_Teacher.Name=$(QUOTE)Teacher1$(QUOTE),Identifier=1"
	@cimcli r  --sum -count 1 -n $(NAMESPACE) -r Teaches JMPIAssociation_Teacher
	@cimcli rn --sum -count 3 -n $(NAMESPACE) -r Teaches "JMPIAssociation_Teacher.Name=$(QUOTE)Teacher1$(QUOTE),Identifier=1"

	@cimcli a  --sum -count 1 -n $(NAMESPACE) "JMPIAssociation_Student"
	@cimcli an --sum -count 1 -n $(NAMESPACE) "JMPIAssociation_Student"
	@cimcli r  --sum -count 2 -n $(NAMESPACE) "JMPIAssociation_Student"
	@cimcli rn --sum -count 2 -n $(NAMESPACE) "JMPIAssociation_Student"
	@cimcli a  --sum -count 3 -n $(NAMESPACE) -r TaughtBy "JMPIAssociation_Student.Name=$(QUOTE)Student1$(QUOTE),Identifier=1"
	@cimcli an --sum -count 3 -n $(NAMESPACE) -r TaughtBy "JMPIAssociation_Student.Name=$(QUOTE)Student1$(QUOTE),Identifier=1"
	@cimcli r  --sum -count 1 -n $(NAMESPACE) -r TaughtBy JMPIAssociation_Student
	@cimcli rn --sum -count 3 -n $(NAMESPACE) -r TaughtBy "JMPIAssociation_Student.Name=$(QUOTE)Student1$(QUOTE),Identifier=1"

$(PEGASUS_HOME)/lib/$(JAR1):	$(JAR1CLASSES)
	$(PEGASUS_JAVA_JAR) -cf $@ $(addprefix -C .. $(PACKAGEDIR),$^) $(addprefix -C .. $(PACKAGEDIR),$(EXTRAJAR1CLASSES))

$(PEGASUS_HOME)/lib/$(JAR2):	$(JAR2CLASSES)
	$(PEGASUS_JAVA_JAR) -cf $@ $(addprefix -C .. $(PACKAGEDIR),$^) $(addprefix -C .. $(PACKAGEDIR),$(EXTRAJAR2CLASSES))

#include $(ROOT)/test/configend.mak
