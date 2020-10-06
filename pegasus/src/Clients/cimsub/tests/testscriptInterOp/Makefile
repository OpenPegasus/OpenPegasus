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
ROOT = ../../../../..

DIR = Clients/cimsub/tests/testscriptInterOp

include $(ROOT)/mak/config.mak
include $(ROOT)/mak/test.mak
include $(ROOT)/mak/commands.mak

RESULTFILE =  $(TMP_DIR)/result
ifeq ($(PEGASUS_ENABLE_CQL),true)
	ifeq ($(PEGASUS_INTEROP_NAMESPACE),$(NAMESPACE_ROOT_INTEROP))
		MASTERRESULTFILE = $(ROOT)/src/$(DIR)/result.master.rootinterop
	else
	ifeq ($(PEGASUS_INTEROP_NAMESPACE),$(NAMESPACE_INTEROP))
		MASTERRESULTFILE = $(ROOT)/src/$(DIR)/result.master.interop
	else
		MASTERRESULTFILE = $(ROOT)/src/$(DIR)/result.master
	endif
	endif
else
	ifeq ($(PEGASUS_INTEROP_NAMESPACE),$(NAMESPACE_ROOT_INTEROP))
		MASTERRESULTFILE = $(ROOT)/src/$(DIR)/result.master.nocql.rootinterop
	else
	ifeq ($(PEGASUS_INTEROP_NAMESPACE),$(NAMESPACE_INTEROP))
    	MASTERRESULTFILE = $(ROOT)/src/$(DIR)/result.master.nocql.interop
	else
    	MASTERRESULTFILE = $(ROOT)/src/$(DIR)/result.master.nocql
	endif
	endif
endif

tests:

poststarttests:

cleanup:
	@TestCimsubInterOp cleanup
	@$(MAKE) -i -s clean

clean:
	@$(RM) $(RESULTFILE)

#
# Create filters,handlers,and subscriptions required for running the tests
#
test_setup:

	@TestCimsubInterOp cleanup
	@TestCimsubInterOp register
	@TestCimsubInterOp test

cimsubTest_InterOp: test_setup
	@$(RM) $(RESULTFILE)
	@$(MAKE) -i -s cimsubTest1_ignoreerror
	@$(STRIPCRS)
	$(COMPARERESULTS)
	@$(ECHO) +++ Test passed +++
	@TestCimsubInterOp cleanup
	@$(RM) $(RESULTFILE)

cimsubTest1_ignoreerror:
	@$(ECHO) ++++cimsub interop tests
	@$(ECHO)  +++++++++ Command line testing +++++++++ >> $(RESULTFILE)

	@$(ECHO) "List handler with namespace and class." >> $(RESULTFILE)
	@cimsub -lh -H test/TestProvider:CIM_ListenerDestinationCIMXML.ListenerDestination05 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with handler only." >> $(RESULTFILE)
	@cimsub -ls -H test/TestProvider:CIM_ListenerDestinationCIMXML.ListenerDestination05 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with filter only." >> $(RESULTFILE)
	@cimsub -ls -F Filter03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Non-existent handler." >> $(RESULTFILE)
	@cimsub -rh -H no_handler_around >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove handler with different creation class." >> $(RESULTFILE)
	@cimsub -rh -H $(PEGASUS_INTEROP_NAMESPACE):Snmphandler3 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all subscriptions." >> $(RESULTFILE)
	@cimsub -ls >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all filters." >> $(RESULTFILE)
	@cimsub -lf  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all handlers." >> $(RESULTFILE)
	@cimsub -lh >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all subscriptions(verbose)." >> $(RESULTFILE)
	@cimsub -ls -v >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all filters(verbose)." >> $(RESULTFILE)
	@cimsub -lf -v >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all handlers(verbose)." >> $(RESULTFILE)
	@cimsub -lh -v >> $(RESULTFILE)  $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with filter and handler." >> $(RESULTFILE)
	@$(ECHO) "    Namespace is defaulted to $(PEGASUS_INTEROP_NAMESPACE)." >> $(RESULTFILE)
	@cimsub -ls -F Filter01 -H ListenerDestination01  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with filter and handler." >> $(RESULTFILE)
	@$(ECHO) "    Subscription does not exist in non-default namespace ." >> $(RESULTFILE)
	@cimsub -ls -n test/TestProvider -F Filter11 -H ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with valid namespace for handler." >> $(RESULTFILE)
	@$(ECHO) "    Subscription does not exist in non-default namespace ." >> $(RESULTFILE)
	@cimsub -ls -n test/TestProvider -F Filter11 -H test/TestProvider:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with no name for handler." >> $(RESULTFILE)
	@cimsub -ls -n $(PEGASUS_INTEROP_NAMESPACE) -F $(PEGASUS_INTEROP_NAMESPACE):Filter01 -H $(PEGASUS_INTEROP_NAMESPACE):CIM_IndicationHandlerCIMXML. >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List subscription with full handler path." >> $(RESULTFILE)
	@cimsub -ls -n $(PEGASUS_INTEROP_NAMESPACE) -F $(PEGASUS_INTEROP_NAMESPACE):Filter01 -H $(PEGASUS_INTEROP_NAMESPACE):CIM_IndicationHandlerCIMXML.Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Enable the already enabled test subscription." >> $(RESULTFILE)
	@cimsub -e -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Disable the test subscription." >> $(RESULTFILE)
	@cimsub -d -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to disable the test subscription again." >> $(RESULTFILE)
	@cimsub -d  -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Enable the test subscription." >> $(RESULTFILE)
	@cimsub -e -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -F Filter01 -H ListenerDestination01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -F root/cimv2:Filter12 -H root/SampleProvider:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -F root/cimv2:Filter12 -H test/TestProvider:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Disable the test subscription." >> $(RESULTFILE)
	@$(ECHO) "    Specify subscription namespace." >> $(RESULTFILE)
	@cimsub -d -n $(PEGASUS_INTEROP_NAMESPACE) -F root/cimv2:Filter12 -H test/TestProvider:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    Use default subscription namespace." >> $(RESULTFILE)
	@cimsub -ls -F root/cimv2:Filter12 -H test/TestProvider:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Re-enable the test subscription." >> $(RESULTFILE)
	@$(ECHO) "    Specify subscription namespace." >> $(RESULTFILE)
	@cimsub -e -n $(PEGASUS_INTEROP_NAMESPACE) -F root/cimv2:Filter12 -H test/TestProvider:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -F root/cimv2:Filter12 -H test/TestProvider:ListenerDestination06 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    Use all default namespaces." >> $(RESULTFILE)
	@cimsub -ls -F Filter01 -H CIM_IndicationHandlerCIMXML.Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove test subscription." >> $(RESULTFILE)
	@$(ECHO) "    Use all default namespaces." >> $(RESULTFILE)
	@cimsub -rs -F Filter01 -H CIM_IndicationHandlerCIMXML.Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    Use all default namespaces." >> $(RESULTFILE)
	@cimsub -ls -F Filter01 -H CIM_IndicationHandlerCIMXML.Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test filter." >> $(RESULTFILE)
	@$(ECHO) "    Use default namespace." >> $(RESULTFILE)
	@cimsub -lf -F Filter02 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List a filter." >> $(RESULTFILE)
	@$(ECHO) "    Specify namespace." >> $(RESULTFILE)
	@cimsub -lf -F $(PEGASUS_INTEROP_NAMESPACE):Filter03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to remove test filter." >> $(RESULTFILE)
	@$(ECHO) "    The filter is still referenced by a suscription." >> $(RESULTFILE)
	@cimsub -rf -F Filter02 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the test filter." >> $(RESULTFILE)
	@cimsub -ls -F Filter02 -H CIM_IndicationHandlerCIMXML.Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the test filter." >> $(RESULTFILE)
	@cimsub -rs -F Filter02 -H CIM_IndicationHandlerCIMXML.Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the test filter." >> $(RESULTFILE)
	@cimsub -ls -F Filter02 -H CIM_IndicationHandlerCIMXML.Handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to remove test filter." >> $(RESULTFILE)
	@$(ECHO) "    The filter should be removed." >> $(RESULTFILE)
	@cimsub -rf -F Filter02 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test filter." >> $(RESULTFILE)
	@cimsub -lf -F Filter02 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List persistent test handler." >> $(RESULTFILE)
	@$(ECHO) "    Use default namespace." >> $(RESULTFILE)
	@cimsub -lh -v -H CIM_IndicationHandlerCIMXML.Handler02p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Try to remove persistent test handler." >> $(RESULTFILE)
	@$(ECHO) "    The handler is still referenced by a subscription." >> $(RESULTFILE)
	@cimsub -rh -H CIM_IndicationHandlerCIMXML.Handler02p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the persistent test handler." >> $(RESULTFILE)
	@cimsub -ls -F Filter01 -H CIM_IndicationHandlerCIMXML.Handler02p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the persistent test handler." >> $(RESULTFILE)
	@cimsub -rs -F Filter01 -H CIM_IndicationHandlerCIMXML.Handler02p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the persistent test handler." >> $(RESULTFILE)
	@cimsub -ls -F Filter01 -H CIM_IndicationHandlerCIMXML.Handler02p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove persistent test handler." >> $(RESULTFILE)
	@$(ECHO) "    The handler should be removed." >> $(RESULTFILE)
	@cimsub -rh -H CIM_IndicationHandlerCIMXML.Handler02p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List persistent test handler." >> $(RESULTFILE)
	@cimsub -lh -H CIM_IndicationHandlerCIMXML.Handler02p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List transient test handler." >> $(RESULTFILE)
	@cimsub -lh -v -H CIM_IndicationHandlerCIMXML.Handler02t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the transient test handler." >> $(RESULTFILE)
	@cimsub -ls -F Filter01 -H CIM_IndicationHandlerCIMXML.Handler02t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the transient test handler." >> $(RESULTFILE)
	@cimsub -rh -H CIM_IndicationHandlerCIMXML.Handler02t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List transient test handler." >> $(RESULTFILE)
	@$(ECHO) "    The transient test handler should have been removed." >> $(RESULTFILE)
	@cimsub -lh -H CIM_IndicationHandlerCIMXML.Handler02t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    The test subscription should have been removed." >> $(RESULTFILE)
	@cimsub -ls -F Filter01 -H CIM_IndicationHandlerCIMXML.Handler02t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List transient test handler." >> $(RESULTFILE)
	@cimsub -lh -H ListenerDestination01t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    This is the only reference to the transient test handler." >> $(RESULTFILE)
	@cimsub -ls -F Filter01 -H ListenerDestination01t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the test subscription." >> $(RESULTFILE)
	@$(ECHO) "    The test handler is only referenced by this subscription." >> $(RESULTFILE)
	@cimsub -rs -F Filter01 -H ListenerDestination01t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@$(ECHO) "    The test subscription should have been removed." >> $(RESULTFILE)
	@cimsub -ls -F Filter01 -H ListenerDestination01t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List transient test handler." >> $(RESULTFILE)
	@$(ECHO) "    The transient test handler should still exist." >> $(RESULTFILE)
	@cimsub -lh -H ListenerDestination01t >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -F Filter09 -H ListenerDestination03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test handler." >> $(RESULTFILE)
	@cimsub -lh -H ListenerDestination03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test filter." >> $(RESULTFILE)
	@cimsub -lf -F Filter09 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Remove the test subscription,filter and handler." >> $(RESULTFILE)
	@cimsub -ra -F Filter09  -H ListenerDestination03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test subscription." >> $(RESULTFILE)
	@cimsub -ls -F Filter09 -H ListenerDestination03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test handler." >> $(RESULTFILE)
	@cimsub -lh -H ListenerDestination03 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List test filter." >> $(RESULTFILE)
	@cimsub -lf -F Filter09 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List an expired subscription." >> $(RESULTFILE)
	@cimsub -ls -v -F Filter15 -H Handler15 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Disable an expired subscription." >> $(RESULTFILE)
	@cimsub -d -F Filter15 -H Handler15 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List an expired subscription." >> $(RESULTFILE)
	@$(ECHO) "The subscription should have been removed." >> $(RESULTFILE)
	@cimsub -ls -v -F Filter15 -H Handler15 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)


	@$(ECHO) "Create filter." >> $(RESULTFILE)
	@cimsub -c f filteraa -Q "select * from IndicationStressTestClass" -N test/TestProvider,test/testIndSrcNS1 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List  all filters." >> $(RESULTFILE)
	@cimsub -lf >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Create Handler." >> $(RESULTFILE)
	@cimsub -c h handler01 -D /Pegasus_IndicationStressTestConsumer >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all Handlers." >> $(RESULTFILE)
	@cimsub -lh >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Create subscription." >> $(RESULTFILE)
	@cimsub -c s -F filteraa -H handler01>> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all subscriptions." >> $(RESULTFILE)
	@cimsub -ls >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Disable subscription." >> $(RESULTFILE)
	@cimsub -d -F filteraa -H handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List  all subscriptions." >> $(RESULTFILE)
	@cimsub -ls >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "Enable subscription." >> $(RESULTFILE)
	@cimsub -e -F filteraa -H handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all subscriptions." >> $(RESULTFILE)
	@cimsub -ls >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "delete subscription." >> $(RESULTFILE)
	@cimsub -r s -F filteraa -H handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "delete Filter." >> $(RESULTFILE)
	@cimsub -r f -F filteraa >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "delete Handler." >> $(RESULTFILE)
	@cimsub -r h -H handler01 >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "List all subscriptions." >> $(RESULTFILE)
	@cimsub -ls >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) "The batch execution for $(PEGASUS_INTEROP_NAMESPACE) namespace." >> $(RESULTFILE)
	@cimsub -b ../testbatchfile/batchnamespace.batch -n $(PEGASUS_INTEROP_NAMESPACE) >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) >> $(RESULTFILE)

	@$(ECHO) ++++cimsub interop tests completed


depend:

general:

strip-license:

prepend-license:
