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
# Note: .. included in path to compile package helper classes if necessary
	$(PEGASUS_JAVA_COMPILER) -classpath $(PEGASUS_HOME)/lib/JMPIImpl.jar$(PEGASUS_JAVA_CLASSPATH_DELIMITER).. $*.java

NAMESPACE          = root/SampleProvider
PACKAGEDIR         = Instances/
JAR1               = JMPIExpInstanceProvider.jar
JAR1CLASSES        = \
	JMPIExpInstanceProvider.class
JAR2               = JMPIInstanceProvider.jar
JAR2CLASSES        = \
	JMPIInstanceProvider.class
JAR3               = JMPICIMInstanceProvider.jar
JAR3CLASSES        = \
	JMPICIMInstanceProvider.class
JAR4               = JMPICIMInstance2Provider.jar
JAR4CLASSES        = \
	JMPICIMInstance2Provider.class
JARS               = $(JAR1) $(JAR2) $(JAR3) $(JAR4)

LOCAL_CLEAN_TARGET = \
	$(JAR1CLASSES) $(addprefix $(PEGASUS_HOME)/lib/,$(JAR1)) \
	$(JAR2CLASSES) $(addprefix $(PEGASUS_HOME)/lib/,$(JAR2)) \
	$(JAR3CLASSES) $(addprefix $(PEGASUS_HOME)/lib/,$(JAR3)) \
	$(JAR4CLASSES) $(addprefix $(PEGASUS_HOME)/lib/,$(JAR4)) \
	*.class

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
	@$(ECHO) +++++ Loading $(subst .jar,,$(JAR3)) into $(NAMESPACE) namespace ...
	@cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" -I. "-n$(NAMESPACE)" $(subst .jar,,$(JAR3)).mof
	@cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" -I. "-n$(INTEROPNS)" $(subst .jar,,$(JAR3))R.mof
	@$(ECHO) +++++ Loading $(subst .jar,,$(JAR4)) into $(NAMESPACE) namespace ...
	@cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" -I. "-n$(NAMESPACE)" $(subst .jar,,$(JAR4)).mof
	@cimmofl "-R$(REPOSITORY_DIR)" "-N$(REPOSITORY_NAME)" "-M$(REPOSITORY_MODE)" -I. "-n$(INTEROPNS)" $(subst .jar,,$(JAR4))R.mof

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
	@$(ECHO) +++++ Loading $(subst .jar,,$(JAR3)) into $(NAMESPACE) namespace ...
	@cimmof -I. "-n$(NAMESPACE)" $(subst .jar,,$(JAR3)).mof
	@cimmof -I. "-n$(INTEROPNS)" $(subst .jar,,$(JAR3))R.mof
	@$(ECHO) +++++ Loading $(subst .jar,,$(JAR4)) into $(NAMESPACE) namespace ...
	@cimmof -I. "-n$(NAMESPACE)" $(subst .jar,,$(JAR4)).mof
	@cimmof -I. "-n$(INTEROPNS)" $(subst .jar,,$(JAR4))R.mof

tests:

poststarttests:			$(addprefix $(PEGASUS_HOME)/lib/,$(JARS))
	cimcli im -n $(NAMESPACE) "JMPIExpInstance_TestPropertyTypes" "resetProvider"
	cimcli gi --sum -count 1 -n $(NAMESPACE) "JMPIExpInstance_TestPropertyTypes.CreationClassName=$(QUOTE)JMPIExpInstance_TestPropertyTypes$(QUOTE),InstanceId=1"
	cimcli gi --sum -count 1 -n $(NAMESPACE) "JMPIExpInstance_TestPropertyTypes.CreationClassName=$(QUOTE)JMPIExpInstance_TestPropertyTypes$(QUOTE),InstanceId=2"
	cimcli ni --sum -count 2 -n $(NAMESPACE) "JMPIExpInstance_TestPropertyTypes"
	cimcli ei --sum -count 2 -n $(NAMESPACE) "JMPIExpInstance_TestPropertyTypes"
	cimcli im -n $(NAMESPACE) "JMPIExpInstance_TestPropertyTypes" "enableModifications" "fState=true"

	cimcli im -n $(NAMESPACE) "JMPIInstance_TestPropertyTypes" "resetProvider"
	cimcli gi --sum -count 1 -n $(NAMESPACE) "JMPIInstance_TestPropertyTypes.CreationClassName=$(QUOTE)JMPIInstance_TestPropertyTypes$(QUOTE),InstanceId=1"
	cimcli gi --sum -count 1 -n $(NAMESPACE) "JMPIInstance_TestPropertyTypes.CreationClassName=$(QUOTE)JMPIInstance_TestPropertyTypes$(QUOTE),InstanceId=2"
	cimcli ni --sum -count 2 -n $(NAMESPACE) "JMPIInstance_TestPropertyTypes"
	cimcli ei --sum -count 2 -n $(NAMESPACE) "JMPIInstance_TestPropertyTypes"
	cimcli im -n $(NAMESPACE) "JMPIInstance_TestPropertyTypes" "enableModifications" "fState=true"

	cimcli im -n $(NAMESPACE) "JMPICIMInstance_TestPropertyTypes" "resetProvider"
	cimcli gi --sum -count 1 -n $(NAMESPACE) "JMPICIMInstance_TestPropertyTypes.CreationClassName=$(QUOTE)JMPICIMInstance_TestPropertyTypes$(QUOTE),InstanceId=1"
	cimcli gi --sum -count 1 -n $(NAMESPACE) "JMPICIMInstance_TestPropertyTypes.CreationClassName=$(QUOTE)JMPICIMInstance_TestPropertyTypes$(QUOTE),InstanceId=2"
	cimcli ni --sum -count 2 -n $(NAMESPACE) "JMPICIMInstance_TestPropertyTypes"
	cimcli ei --sum -count 2 -n $(NAMESPACE) "JMPICIMInstance_TestPropertyTypes"
	cimcli im -n $(NAMESPACE) "JMPICIMInstance_TestPropertyTypes" "enableModifications" "fState=true"

	cimcli im -n $(NAMESPACE) "JMPICIMInstance2_TestPropertyTypes" "resetProvider"
	cimcli gi --sum -count 1 -n $(NAMESPACE) "JMPICIMInstance2_TestPropertyTypes.CreationClassName=$(QUOTE)JMPICIMInstance2_TestPropertyTypes$(QUOTE),InstanceId=1"
	cimcli gi --sum -count 1 -n $(NAMESPACE) "JMPICIMInstance2_TestPropertyTypes.CreationClassName=$(QUOTE)JMPICIMInstance2_TestPropertyTypes$(QUOTE),InstanceId=2"
	cimcli ni --sum -count 2 -n $(NAMESPACE) "JMPICIMInstance2_TestPropertyTypes"
	cimcli ei --sum -count 2 -n $(NAMESPACE) "JMPICIMInstance2_TestPropertyTypes"
	cimcli im -n $(NAMESPACE) "JMPICIMInstance2_TestPropertyTypes" "enableModifications" "fState=true"

poststarttests2:
	cimcli xq -n $(NAMESPACE) "SELECT InstanceId FROM JMPIExpInstance_TestPropertyTypes WHERE InstanceId = 1" "WQL"

$(PEGASUS_HOME)/lib/$(JAR1):	$(JAR1CLASSES) CommonInstanceProvider.class
	make -f JarMakefile $@

$(PEGASUS_HOME)/lib/$(JAR2):	$(JAR2CLASSES) CommonInstanceProvider.class
	make -f JarMakefile $@

$(PEGASUS_HOME)/lib/$(JAR3):	$(JAR3CLASSES) CommonInstanceProvider.class
	make -f JarMakefile $@

$(PEGASUS_HOME)/lib/$(JAR4):	$(JAR4CLASSES) CommonInstanceProvider.class
	make -f JarMakefile $@

#include $(ROOT)/test/configend.mak
