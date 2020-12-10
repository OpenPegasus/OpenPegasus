#//%2006////////////////////////////////////////////////////////////////////////
#//
#// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
#// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
#// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
#// IBM Corp.; EMC Corporation, The Open Group.
#// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
#// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
#// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
#// EMC Corporation; VERITAS Software Corporation; The Open Group.
#// Copyright (c) 2006 Hewlett-Packard Development Company, L.P.; IBM Corp.;
#// EMC Corporation; Symantec Corporation; The Open Group.
#//
#// Permission is hereby granted, free of charge, to any person obtaining a copy
#// of this software and associated documentation files (the "Software"), to
#// deal in the Software without restriction, including without limitation the
#// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
#// sell copies of the Software, and to permit persons to whom the Software is
#// furnished to do so, subject to the following conditions:
#//
#// THE ABOVE COPYRIGHT NOTICE AND THIS PERMISSION NOTICE SHALL BE INCLUDED IN
#// ALL COPIES OR SUBSTANTIAL PORTIONS OF THE SOFTWARE. THE SOFTWARE IS PROVIDED
#// "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
#// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
#// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
#// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
#// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#//
#//==============================================================================

##
##	Notes on using and modifying this script
##  1. Using the ResponseStressCxx provider
##      This script heavily uses the ResponseStressCxx provider which can be
##      modified for varying behavior  the set methodincluding:
##          - Number and size of responses
##          - Delay between responses
##          - Returning error codes
##      However, when changed this behavior affects all subsequent operations
##      Therefore to keep some sense of control, please reset the behavior
##      after each test that sets the behavior of this provider using the
##      reset invoke method.
##

ROOT = ../../../../..
DIR = Pegasus/Client/tests/pullop
include $(ROOT)/mak/config.mak
include ../libraries.mak

LIBRARIES += peggetoopt

EXTRA_INCLUDES = $(SYS_INCLUDES)

PROGRAM = pullop
CLI = cimcli

SOURCES = pullop.cpp

include $(ROOT)/mak/program.mak

##############################
## Test cmpi provider logistics
CMPISample_CLASS_ID = CMPISample_InstanceProviderClass -n root/SampleProvider

## CMPI association provider logistical information
CMPIASSOCOBJ =  CMPI_TEST_Person.name=\"Melvin\"
CMPIASSOCOBJ_INST_ID = CMPI_TEST_Person.name=\"Melvin\" -n test/TestProvider

## ResponseStressTest provider class and namespace
NStressCxx = test/testProvider
CStressCxx = TST_ResponseStressTestCxx
STRESSCXX_CLASS_ID = $(CStressCxx) -n test/testProvider

## Simple C++ Provider classes and namespaces
CxxSE_CLASS_ID = Sample_InstanceProviderClass -n root/SampleProvider

## C++ association Provider Object and Namespace identification
CXXASSOC_INST_ID = Test_CLITestProviderClass.Id=\"Mike\" -n test/TestProvider

HOSTID = -H localhost
CIMCLIHOSTID = -l localhost

## Display definition for pullop.
DSP=-v 2

## Unit test. This is not a compelete test since this is a test program
## Simply assures that we do not crash when help executed.
tests:
	pullop -h
	$(PROGRAM) --help

##################################################################
##
##  All poststart tests
##
poststarttests:
        ## reset the Stress provider behavior to the default
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

	@$(ECHO) Enumerate and EnumerateNames tests that go to Completion
	make simpleTests

	@$(ECHO) Test close in process of delivery
	make testclose

	@$(ECHO) Execute stresstests. NOTE Modifies ResponseStressCxx
	make stresstests

	@$(ECHO) Test timeout in process of delivery
	make testtimeout

	@$(ECHO) Error tests. Tests simple parameter errors.
	make errortests

	@$(ECHO) Error tests. Test responses with provider delays.
	make poststarttests_testDelayedResponses

	@$(ECHO) +++++ All pullop Pull Operation tests Passed.


########################################################################
## Error tests. Each test is for a single CIMException type and
## returns that error.
errortests:
	make filterParamError
	make pullClassParamError
	make continueOnErrorError
	make invalidAssocObjectPath
	make maxPullObjectCountError
	make testexceedMaxObjectCountLimit
	@$(ECHO) +++++ pullop errortests Passed.

## execute simple Enumerate and associator tests with a variety of
## sizes, request parameters, etc. Includes both C++ and cmpi providers.
simpleTests:
	@$(ECHO)  Enumerate and EnumerateNames tests that go to Completion
	make smallTestEnumerateInstances
	make smallTestEnumerateInstanceNames
	make smallCmpiEnumerate
	make smallCmpiEnumerateNames
	make peManagedElementcimv2
	make penManagedElementcimv2
	make peManagedElementInterop
	make penManagedElementInterop
	make enumerateMaxObjectSizeVariations
	make enumeratePathsMaxObjectSizeVariations
	make enumerateEmptyResponse

	@$(ECHO)  Associator and Reference tests that go to completion
	make SimpleRefAndAssoc
	make SimpleAssociatorTest1
	make SimpleAssociatorTest2
	make SimpleAssociatorTest3
	make pcmpir1
	make pcmpirn1
	make pcmpian1
	make pcmpia1

	@$(ECHO) +++++ pullop SimpleTests passed

## Enumerate a class that is a C++ Provider. Very small enumerate
## Normally returns about 3 instances. Also tests returning with property
## variations.
smallTestEnumerateInstances:
## Return all with initial request
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 5 -N 5  -C -T $(HOSTID)
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 5 -N 5 -C -T $(HOSTID) -P Message
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 5 -N 5 -C -T $(HOSTID) -P ""
## Return with probably one pull
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 1 -N 5 -C -T $(HOSTID)
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 1 -N 5 -C -T $(HOSTID) -P Message
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 1 -N 5 -C -T $(HOSTID) -P ""

## test variations of open max object count and pull max object count
enumerateMaxObjectSizeVariations:
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 1 -N 1 -C -T $(HOSTID)
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 2 -N 2 -C -T $(HOSTID)
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 3 -N 3 -C -T $(HOSTID)
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 0 -N 1 -C -T $(HOSTID)
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 0 -N 3 -C -T $(HOSTID)
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 0 -N 10 -C -T $(HOSTID)
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 0 -N 100 -C -T $(HOSTID)
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 100 -N 10 -C -T $(HOSTID)
	$(PROGRAM) e $(CxxSE_CLASS_ID) -M 500 -N 100 -C -T $(HOSTID)

enumeratePathsMaxObjectSizeVariations:
	$(PROGRAM) en $(CxxSE_CLASS_ID) -M 1 -N 1 -C -T $(HOSTID)
	$(PROGRAM) en $(CxxSE_CLASS_ID) -M 2 -N 2 -C -T $(HOSTID)
	$(PROGRAM) en $(CxxSE_CLASS_ID) -M 3 -N 3 -C -T $(HOSTID)
	$(PROGRAM) en $(CxxSE_CLASS_ID) -M 0 -N 1 -C -T $(HOSTID)
	$(PROGRAM) en $(CxxSE_CLASS_ID) -M 0 -N 3 -C -T $(HOSTID)
	$(PROGRAM) en $(CxxSE_CLASS_ID) -M 0 -N 10 -C -T $(HOSTID)
	$(PROGRAM) en $(CxxSE_CLASS_ID) -M 0 -N 100 -C -T $(HOSTID)
	$(PROGRAM) en $(CxxSE_CLASS_ID) -M 100 -N 10 -C -T $(HOSTID)
	$(PROGRAM) en $(CxxSE_CLASS_ID) -M 500 -N 100 -C -T $(HOSTID)

## enumerate instance names of a class in a C++ provider
smallTestEnumerateInstanceNames:
	$(PROGRAM) en $(CxxSE_CLASS_ID) -M 1 -N 1 -C $(HOSTID)
	$(PROGRAM) en $(CxxSE_CLASS_ID) -M 5 -N 5 -C -T $(HOSTID)
	$(PROGRAM) en $(CxxSE_CLASS_ID) -M 0 -N 1 -C -T $(HOSTID)

## enumerate from cmpi providers. There are 3 tests here to include
## property list variations
smallCmpiEnumerate:
	$(PROGRAM) e $(CMPISample_CLASS_ID) -M 1 -N 1 -C -T $(HOSTID)
	$(PROGRAM) e $(CMPISample_CLASS_ID) -M 1 -N 1 -C -T $(HOSTID) -P Message
	$(PROGRAM) e $(CMPISample_CLASS_ID) -M 1 -N 1 -C -T $(HOSTID) -P ""

## enumerate from cmpi providers
smallCmpiEnumerateNames:
	$(PROGRAM) en $(CMPISample_CLASS_ID) -M 1 -N 1 -C -T $(HOSTID)

## Simple pull ref and assoc operations.
SimpleRefAndAssoc:
	$(PROGRAM) r $(CXXASSOC_INST_ID) -C $(HOSTID)
	$(PROGRAM) rn $(CXXASSOC_INST_ID) -C $(HOSTID)
	$(PROGRAM) a $(CXXASSOC_INST_ID) -C $(HOSTID)
	$(PROGRAM) an $(CXXASSOC_INST_ID) -C $(HOSTID)

## returns a single association object and asks for one object per request
## compares results with assoc request so counts should match.  This
SimpleAssociatorTest1:
## this test confirms that we are getting one object
	cimcli a $(CXXASSOC_INST_ID) --sum -count 1
	$(PROGRAM) a $(CXXASSOC_INST_ID) -M 1 -N 1 -T  -C $(HOSTID)
	$(PROGRAM) an $(CXXASSOC_INST_ID) -M 1 -N 1 -T  -C $(HOSTID)
	$(PROGRAM) r $(CXXASSOC_INST_ID) -M 1 -N 1 -T  -C $(HOSTID)
	$(PROGRAM) rn $(CXXASSOC_INST_ID) -M 1 -N 1 -T  -C $(HOSTID)

## Request class that returns a single object but ask for more than one in
## response
SimpleAssociatorTest2: pa2 pan2 pr2 prn2
pa2:
	$(PROGRAM) a $(CXXASSOC_INST_ID) -C $(HOSTID)

pan2:
	$(PROGRAM) an $(CXXASSOC_INST_ID) -C $(HOSTID)

pr2:
	$(PROGRAM) rn $(CXXASSOC_INST_ID) -C $(HOSTID)

prn2:
	$(PROGRAM) rn $(CXXASSOC_INST_ID) -C $(HOSTID)

## Request class that returns objects with some in repository
SimpleAssociatorTest3: pa3 pan3 pr3 prn3
pa3:
	$(PROGRAM) a -c Person.name=\"Mike\" -n root/SampleProvider -N 1 -N 1 -C $(HOSTID)
pan3:
	$(PROGRAM) an -c Person.name=\"Mike\" -n root/SampleProvider -N 1 -N 1  -C $(HOSTID)
pr3:
	$(PROGRAM) a -c Person.name=\"Mike\" -n root/SampleProvider -N 1 -N 1  -C $(HOSTID)
prn3:
	$(PROGRAM) an -c Person.name=\"Mike\" -n root/SampleProvider -N 1 -N 1  -C $(HOSTID)

######################################################################
## Tests that enumerate through a complete namespace from top level
## Some of the other namespaces generate errors because they have providers
## that generate specific errors on response.

peManagedElement: peManagedElementcimv2

peManagedElementcimv2:
	$(PROGRAM) e -c CIM_ManagedElement -M 100 -N 100 -v 5 -n root/cimv2 $(HOSTID)

peManagedElementInterop:
	$(PROGRAM) e -c CIM_ManagedElement -M 9 -N 1 -C -n root/PG_InterOp $(HOSTID)

#########################################################
penManagedElement: penManagedElementcimv2

penManagedElementcimv2:
	$(PROGRAM) en -c CIM_ManagedElement -M 10 -N 10 -C $(HOSTID)

penManagedElementInterop:
	$(PROGRAM) en -c CIM_ManagedElement -M 9 -N 1 -C -n root/PG_InterOp $(HOSTID)

######################################################################
## Test Empty response, returns nothing.
enumerateEmptyResponse:
	$(PROGRAM) e -c CIM_ConcreteIdentity -n root/cimv2 $(HOSTID)
	$(PROGRAM) en -c CIM_ConcreteIdentity -n root/cimv2 $(HOSTID)

####################################################################
## tests with large scale response provider
## Execute the large scale test with selected properties in response.
## issue because there is one property that may differ between instances,
## the interval property that defines intervals between the delivery
## of subsequent instances to the CIMOM and therefore is not constant.
## Included list of properties to return to make this test valid.
## CIMCLi is called before each test to set the response size and number
## of responses requested
## Calling the invoke method before starting each test forces
## the provider to be loaded which eliminates startup differences.
## Set large client timeout here because with the stress test sizes, they
## fail in valgrind with smaller client timeout.

stresstests:
	make stressEnumpei
	make stressEnumei
	make stressEnum1
	make stressEnumName1
	make stressEnumName2
	make stressEnum2
##      KS_FUTURE Test 3 disabled because there are
##      client limits on size of responses in some test environments.  There
##      is a manual test (target testSizeLimits) that tests up to a million
##      objects in the response. Please run that test to determine your system 
##      limits on operation response size.
##	make stressEnum3
##	make stressEnumName3
	make stressVariation
	@$(ECHO) +++++ pullop stresstests Passed.

## test with 10,000 instances of 10,000 bytes each.
## cimcli stress tests pull and non-pull
stressEnumpei:
	cimcli im $(STRESSCXX_CLASS_ID) set Size=10000 \
	    ResponseCount=10000 $(CIMCLIHOSTID)
	cimcli pei $(STRESSCXX_CLASS_ID) -pl Id,Pattern,s1,SequenceNumber --sum
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

stressEnumei:
	cimcli im $(STRESSCXX_CLASS_ID) set Size=10000 \
	    ResponseCount=10000 $(CIMCLIHOSTID)
	cimcli ei $(STRESSCXX_CLASS_ID) -pl Id,Pattern,s1,SequenceNumber --sum
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

## pullop stress tests
stressEnum1:
	cimcli im $(STRESSCXX_CLASS_ID) set Size=10000 \
	    ResponseCount=10000 $(CIMCLIHOSTID)
	$(PROGRAM) e -c $(STRESSCXX_CLASS_ID) -C -y 120 \
	    -P Pattern -P s1 -P Id -P SequenceNumber \
	    -T $(DSP) $(HOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

## test with 100,000 returned instances of 1000 bytes each
stressEnum2:
	cimcli im $(STRESSCXX_CLASS_ID) set Size=1000 ResponseCount=30000 $(CIMCLIHOSTID)
	$(PROGRAM) e -c $(STRESSCXX_CLASS_ID) -C -y 120  \
	    -P Pattern -P s1 -P Id -P SequenceNumber -T $(DSP) $(HOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

## test with 100,000 returned instances of 1000 bytes each
stressEnum3:
	cimcli im $(STRESSCXX_CLASS_ID) set Size=1000  \
	    ResponseCount=100000 $(CIMCLIHOSTID)
	$(PROGRAM) e -c $(STRESSCXX_CLASS_ID)  -C -y 120  \
	    -P Pattern -P s1 -P Id -P SequenceNumber \
	    -T $(DSP) $(HOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

## get 10,000 object paths. Size does not count since we are returning
## object paths only
stressEnumName1:
	cimcli im $(STRESSCXX_CLASS_ID) set Size=10000 \
	    ResponseCount=10000 $(CIMCLIHOSTID)
	$(PROGRAM) en -c $(STRESSCXX_CLASS_ID) -C -y 120 -T $(DSP) $(HOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

## get 30,000 object paths, Size does not count since we are returning
## object paths only
stressEnumName2:
	cimcli im $(STRESSCXX_CLASS_ID) set Size=1000 ResponseCount=30000 $(CIMCLIHOSTID)
	$(PROGRAM) en -c $(STRESSCXX_CLASS_ID) -C -y 120 -T $(DSP) $(HOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

## get 100,000 object paths, Size does not count since we are returning
## object paths only
stressEnumName3:
	cimcli im $(STRESSCXX_CLASS_ID) set  Size=1000 ResponseCount=100000 $(CIMCLIHOSTID)
	$(PROGRAM) en -c $(STRESSCXX_CLASS_ID) -C -y 120 -T $(DSP) $(HOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

## test of variation of response  max size form very small to large
## test run with provider that returns 10,000 instances of size ~1000 bytes
stressVariation:
	cimcli im $(STRESSCXX_CLASS_ID) set  Size=1000 ResponseCount=10000 $(CIMCLIHOSTID)
	$(PROGRAM) e -c $(STRESSCXX_CLASS_ID) -M 10 -N 10 -C -T -P Pattern -P s1 -P Id -P SequenceNumber -T $(DSP) $(HOSTID)
	$(PROGRAM) e -c $(STRESSCXX_CLASS_ID) -M 100 -N 100 -C -T -P Pattern -P s1 -P Id -P SequenceNumber -T $(DSP) $(HOSTID)
	$(PROGRAM) e -c $(STRESSCXX_CLASS_ID) -M 1000 -N 1000 -C -T -P Pattern -P s1 -P Id -P SequenceNumber -T $(DSP) $(HOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

########################################################################
### Error Tests
## These tests actually return exit 0 because the -R option reverses the
## exit codes so the following should actually return positive exits for
## make
##
pullClassParamError:
	$(PROGRAM) e -c blah -R $(HOSTID)
	$(PROGRAM) en -c blah -R $(HOSTID)

## Generates an error because reqested object count too large. Currently
## maximum is fixed at 10000
maxPullObjectCountError:
	$(PROGRAM) e -c $(STRESSCXX_CLASS_ID) -M 99999 -N 99999 -C -R -T $(DSP) $(HOSTID)

# Test for invalid filter query language definitions
filterParamError:
	$(PROGRAM) e Test_Person -f abc -R $(HOSTID)
	$(PROGRAM) e Test_Person -l WQL -R $(HOSTID)
	$(PROGRAM) en Test_Person -f abc -R $(HOSTID)
	$(PROGRAM) en Test_Person -l WQL -R $(HOSTID)
	$(PROGRAM) r Test_Person -f abc -R $(HOSTID)
	$(PROGRAM) r Test_Person -l WQL -R $(HOSTID)
	$(PROGRAM) rn Test_Person -f abc -R $(HOSTID)
	$(PROGRAM) rn Test_Person -l WQL -R $(HOSTID)
	$(PROGRAM) a Test_Person -f abc -R $(HOSTID)
	$(PROGRAM) a Test_Person -l WQL -R $(HOSTID)
	$(PROGRAM) an Test_Person -f abc -R $(HOSTID)
	$(PROGRAM) an Test_Person -l WQL -R $(HOSTID)

## test continueOnError Parameter.  Should return error in all cases since
## continueOnError not supported
continueOnErrorError:
	$(PROGRAM) e Test_Person  -R -x $(HOSTID)
	$(PROGRAM) en Test_Person  -R -x $(HOSTID)
	$(PROGRAM) r $(CXXASSOC_INST_ID) -R -x $(HOSTID)
	$(PROGRAM) rn $(CXXASSOC_INST_ID) -R -x $(HOSTID)
	$(PROGRAM) a $(CXXASSOC_INST_ID) -R -x $(HOSTID)
	$(PROGRAM) an $(CXXASSOC_INST_ID) -R -x $(HOSTID)

# test for invalid objectPath (class only) on associaton requests
# The pull operations only handle instance paths
invalidAssocObjectPath:
	$(PROGRAM) r -c Test_Person -R $(HOSTID)
	$(PROGRAM) rn -c Test_Person -R $(HOSTID)
	$(PROGRAM) a -c Test_Person -R $(HOSTID)
	$(PROGRAM) an -c Test_Person -R $(HOSTID)

#########################################################################
#### Test various timeouts.
####

testtimeout: testClientTimeoutCLI testClientTimeoutPullop testInteropTimeout

#### Client timeout. Test case where client timeout closes client between
#### pull operations. This is a blackbox test. It tests the client but does
#### not actually test the internals to determine that the server completely
#### cleans up the enumeration contexts (ex. trace and check that contexts are
#### removed). Sets delay between pull calls to 20 sec and client timeout to
#### 10 sec.
testClientTimeoutCLI:
	cimcli im $(STRESSCXX_CLASS_ID) set Size=100 ResponseCount=1000 $(CIMCLIHOSTID)
	cimcli pei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) -pullDelay 20 -mo 10 \
        --timeout 10 --expExit 21
	cimcli pni $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) -pullDelay 20 -mo 10 \
        --timeout 20 --expExit 21
	cimcli im $(STRESSCXX_CLASS_ID) reset  $(CIMCLIHOSTID)

#### -t interop timeout(sec), -y client timeout(sec) -s (client interop sleep
#### between operations. -T show results of detailed timing of operations
testClientTimeoutPullop:
	cimcli im $(STRESSCXX_CLASS_ID) set Size=100 ResponseCount=1000 $(CIMCLIHOSTID)
	$(PROGRAM) e -c $(STRESSCXX_CLASS_ID)  -M 1 -N 1 -t 2 -T -y 5 -s 10 -R $(HOSTID)
	$(PROGRAM) e -c $(STRESSCXX_CLASS_ID)  -M 1 -N 1 -t 2 -T -y 5 -s 10 -R $(HOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) reset  $(CIMCLIHOSTID)

#### Interop timeout test. Set interoptime to 2 sec and sleep to 10 sec. This
#### test should timeout after the first pull. (-s defines time to sleep
#### between operations for the provider)
testInteropTimeout:
	$(PROGRAM) e -c $(STRESSCXX_CLASS_ID)  -M 1 -N 1 -t 2 -T -t 5 -s 10 -R $(HOSTID)
	$(PROGRAM) en -c $(STRESSCXX_CLASS_ID)  -M 1 -N 1 -t 2 -T -t 5 -s 10 -R $(HOSTID)

## corresponding test but with large response size. Not part of poststart
## tests because it changes the parameters of the stress test provider
testBigTimeout:
	cimcli im $(STRESSCXX_CLASS_ID) set Size=100 ResponseCount=1000 $(CIMCLIHOSTID)
	$(PROGRAM) e -c $(STRESSCXX_CLASS_ID) -M 1 -N 1 -t 2 -T -t 5 -s 10 -R $(HOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) reset  $(CIMCLIHOSTID)

#################################
#### zeroRequest test. Set count on maxObjectCount for pull operations to zero
#### Should get exception after about 1000 pull requests.
testexceedMaxObjectCountLimit:
	cimcli im $(STRESSCXX_CLASS_ID) set Size=10000 ResponseCount=10000 $(CIMCLIHOSTID)
	$(PROGRAM) e -c $(STRESSCXX_CLASS_ID) -M 1 -N 0 -T -R $(HOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) reset  $(CIMCLIHOSTID)
##################################
## CMPI Provider tests
pcmpia1:
	$(PROGRAM) a $(CMPIASSOCOBJ_INST_ID) -M 1 -N 1  -C -T $(HOSTID)

pcmpian1:
	$(PROGRAM) an $(CMPIASSOCOBJ_INST_ID) -M 1 -N 1 -C -T $(HOSTID)

#######################################################################
#### Close tests
### Set the the size and value of responses to get large number of
### responses. Then call pullop to get small set of objects and then
### close (the -X option which issues a close after 4 pull operations)

testclose: testenumclose testenumpathsclose
testenumclose:
	cimcli im $(STRESSCXX_CLASS_ID) set Size=10000 \
	    ResponseCount=10000 $(CIMCLIHOSTID)
	$(PROGRAM) e -c $(STRESSCXX_CLASS_ID) -M 9 -N 10 -X 4 $(HOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

testenumpathsclose:
	cimcli im $(STRESSCXX_CLASS_ID) set -n $(NStressCxx) Size=10000 \
	    ResponseCount=10000 $(CIMCLIHOSTID)
	$(PROGRAM) en -c $(STRESSCXX_CLASS_ID) -M 9 -N 10 -X 4 $(HOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

####################################################
##
##     Manual Test targets used during development
##
###################################################

## test all operations against all classes in namespace root/cimv2
## kept manual because generates extensive output.
allOperationsCIMV2:
	$(PROGRAM) all -n root/cimv2 -C -T $(DSP) $(HOSTID)

allOperationsInterop:
	$(PROGRAM) all -n root/PG_InterOp -C -T $(DSP) $(HOSTID)

clirep:
	cimcli pei Test_Person -n test/TestProvider $(CIMCLIHOSTID)

cliprov:
	cimcli pei Sample_InstanceProviderClass -n root/SampleProvider $(CIMCLIHOSTID)

pr: pr1 pr2

e1:
	cimcli ei Sample_InstanceProviderClass -n root/SampleProvider $(CIMCLIHOSTID)

e2:
	cimcli ei Test_Person -n test/TestProvider $(CIMCLIHOSTID)

r2:
	cimcli r $(CXXASSOC_INST_ID) $(CIMCLIHOSTID)

pe: pullOneEnumerateInstances pe2

## Tests for reference names and pull referencenames.
r1:
	cimcli r $(CXXASSOC_INST_ID) $(CIMCLIHOSTID)
rn1:
	cimcli rn $(CXXASSOC_INST_ID) $(CIMCLIHOSTID)
an1:
	cimcli an $(CXXASSOC_INST_ID) $(CIMCLIHOSTID)

##########################
##
## NOTE: classes that do not exist should not generate error,
## only classes that are considered invalid CIMNames (characters that cannot
## be in a CIMName
checkinvalidroleparam:
	cimcli a CIM_Door -r help
	cimcli a CIM_Door -r ==== --expExit 4
	cimcli a CIM_Door -rr help
	cimcli a CIM_Door -rr ==== --expExit 4
	cimcli an CIM_Door -r help
	cimcli an CIM_Door -r ==== --expExit 4
	cimcli an CIM_Door -rr help
	cimcli an CIM_Door -rr ==== --expExit 4
	cimcli r CIM_Door -r help
	cimcli r CIM_Door -r ==== --expExit 4
	cimcli rn CIM_Door -r help
	cimcli rn CIM_Door -r ==== --expExit 4
###Corresponding tests for instances
#### KS_ISSUE the following tests do return exception.  There is a
#### difference in the server in testing role for classes and instances
#### which applies to the pull operations also
####	cimcli rn Test_CLITestProviderClass.Id=\"Mike\" -r help
####	cimcli prn Test_CLITestProviderClass.Id=\"Mike\" -r help

################################
##
## Test Client memory limits with large operations
## This test was written to allow testing for possible memory problems
## in the system with individual operations and in particular clients
## where pullop could run the client out of memory
## It tests with response counts from 1 to 600,000 objects and executes
## cimcli cimcli-pull, and pullop both with and without comparison to
## see at what point something fails.
testSizeLimits:
	$(ECHO) +++test with size 1,000, responseCount 1
	cimcli im $(STRESSCXX_CLASS_ID) set  Size=1000 \
	    ResponseCount=1 $(CIMCLIHOSTID)
	cimcli ei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t
	cimcli pei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -y 120 -T $(DSP) $(HOSTID)
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -C -y 120 -T $(DSP) $(HOSTID)

	$(ECHO) +++test with size 1,000, responseCount 1,000
	cimcli im $(STRESSCXX_CLASS_ID) set Size=1000 \
	    ResponseCount=1000 $(CIMCLIHOSTID)
	cimcli ei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t
	cimcli pei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -y 120 -T $(DSP) $(HOSTID)
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -C -y 120 -T $(DSP) $(HOSTID)

	$(ECHO) +++test with size 1,000, responseCount 10,000
	cimcli im $(STRESSCXX_CLASS_ID) set Size=1000 \
	    ResponseCount=10000 $(CIMCLIHOSTID)
	cimcli ei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t
	cimcli pei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -y 120 -T $(DSP) $(HOSTID)
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -C -y 120 -T $(DSP) $(HOSTID)

	$(ECHO) +++test with size 1,000, responseCount 25,000
	cimcli im $(STRESSCXX_CLASS_ID) set Size=1000 \
	    ResponseCount=25000 $(CIMCLIHOSTID)
	cimcli ei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t
	cimcli pei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -y 120 -T $(DSP) $(HOSTID)
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -C -y 120 -T $(DSP) $(HOSTID)

	$(ECHO) +++test with size 1,000, responseCount 50,000
	cimcli im $(STRESSCXX_CLASS_ID) set Size=1000 \
	    ResponseCount=50000 $(CIMCLIHOSTID)
	cimcli ei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t
	cimcli pei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -y 120 -T $(DSP) $(HOSTID)
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -C -y 120 -T $(DSP) $(HOSTID)

	$(ECHO) +++test with size 1,000, responseCount 75,000
	cimcli im $(STRESSCXX_CLASS_ID) set Size=1000 \
	    ResponseCount=75000 $(CIMCLIHOSTID)
	cimcli ei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t
	cimcli pei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -y 120 -T $(DSP) $(HOSTID)
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -C -y 120 -T $(DSP) $(HOSTID)

	$(ECHO) +++test with size 1,000, responseCount 100,000
	cimcli im $(STRESSCXX_CLASS_ID) set Size=1000 \
	    ResponseCount=100000 $(CIMCLIHOSTID)
	cimcli ei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t
	cimcli pei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -y 120 -T $(DSP) $(HOSTID)
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -C -y 120 -T $(DSP) $(HOSTID)

	$(ECHO) +++test with size 1,000, responseCount 300,000
	cimcli im $(STRESSCXX_CLASS_ID) set  Size=1000 \
	    ResponseCount=300000 $(CIMCLIHOSTID)
	cimcli ei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t --timeout 240
	cimcli pei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t --timeout 240
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -y 240 -T $(DSP) $(HOSTID)
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -C -y 240 -T $(DSP) $(HOSTID)

	$(ECHO) +++test with size 1,000, responseCount 600,000
	cimcli im $(STRESSCXX_CLASS_ID) set Size=1000 \
	    ResponseCount=600000 $(CIMCLIHOSTID)
	cimcli ei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t --timeout 300
	cimcli pei $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID) --sum --t --timeout 300
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -y 300 -T $(DSP) $(HOSTID)
	$(PROGRAM) e $(STRESSCXX_CLASS_ID) -C -y 300 -T $(DSP) $(HOSTID)

	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)
	$(ECHO) +++pullop test target testSizeLimits passed successfully

################################
## Manual cimcli tests used during development

rsgc:
	cimcli gc $(STRESSCXX_CLASS_ID) $(CIMCLIHOSTID)

rsget:
	cimcli im $(STRESSCXX_CLASS_ID) get $(CIMCLIHOSTID)

rsset:
	cimcli im $(STRESSCXX_CLASS_ID) set Size=10000 \
	     ResponseCount=10000 $(CIMCLIHOSTID)

rsset1:
	cimcli im set $(STRESSCXX_CLASS_ID) -n $(NStressCxx) $(CIMCLIHOSTID)

## force the provider to reset to its default parameters
resetStressTestProvider:
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

## cimcli enumerateInstances
rsei: rsset
	cimcli ei $(STRESSCXX_CLASS_ID) get -n $(NStressCxx) --sum --t $(CIMCLIHOSTID)

rseip:
	cimcli pei $(STRESSCXX_CLASS_ID) get --sum --t $(CIMCLIHOSTID)

## pull the number of objects defined by rsset with Timing and comparison
rseip1: rsset
	$(PROGRAM) e -c $(STRESSCXX_CLASS_ID) -M 9 -N 100 -T -C $(HOSTID)

rseipn:
	cimcli ni $(STRESSCXX_CLASS_ID) get --sum $(CIMCLIHOSTID)

###################################################################
##	Tests of delaying responses from the Providers
##      Some of these are manual only because the take a long time to
##      complete
#####################################################################

poststarttests_testDelayedResponses: testNoDelay testSmallDelayedResponse \
                                    testDelayedResponse
testNoDelay:
	cimcli im $(STRESSCXX_CLASS_ID) set ResponseCount=500 Delay=0 $(CIMCLIHOSTID)
	cimcli pei $(STRESSCXX_CLASS_ID) --sum -count 500 --t $(CIMCLIHOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

## test for responses that are delayed but not so long that
## zero length responses are sent
testSmallDelayedResponse:
	$(ECHO) This test should take about 5 seconds total
	cimcli im $(STRESSCXX_CLASS_ID) set ResponseCount=500 Delay=5 $(CIMCLIHOSTID) 
	cimcli pei $(STRESSCXX_CLASS_ID) --sum -v -count 500 --t $(CIMCLIHOSTID)
        ## reset to restore zero delay
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

## special test for responses that are delayed so that some zero length
## responses are sent This should take about 45 seconds to complete
## and return multiple zero-object responses
testDelayedResponse:
	$(ECHO) This test should take about 45 seconds.
	cimcli im $(STRESSCXX_CLASS_ID) set ResponseCount=295 Delay=45 $(CIMCLIHOSTID)
	cimcli pei $(STRESSCXX_CLASS_ID) --sum -v -count 295 --t $(CIMCLIHOSTID)
	# Restore zero delay
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

## Manual test only for now
## special test for responses that are delayed so that some zero length
## responses are sent This should take about 135 seconds to complete
## and return multiple zero-object responses
testMediumDelayedResponse:
	$(ECHO) This test should take about 135 seconds.
	cimcli im $(STRESSCXX_CLASS_ID) set ResponseCount=595 Delay=45 \
           $(CIMCLIHOSTID)
	cimcli pei $(STRESSCXX_CLASS_ID) --sum -v -count 595 \
	   --t $(CIMCLIHOSTID)
	# Restore zero delay
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)

## special test for responses that are delayed so that some zero length
## responses are sent This should many minutes (ex. 11) and end in an error
testMassiveDelayedResponse:
	$(ECHO) This test should take many minutes.
	cimcli im $(STRESSCXX_CLASS_ID) set ResponseCount=595 Delay=700 \
           $(CIMCLIHOSTID)
	cimcli pei $(STRESSCXX_CLASS_ID) --sum -v -count 295 \
	   --t $(CIMCLIHOSTID)
	# Restore zero delay
	cimcli im $(STRESSCXX_CLASS_ID) reset  $(CIMCLIHOSTID)

##################################################
## Test with pull operations in cimcli
##
cimclipei:
	cimcli pei $(STRESSCXX_CLASS_ID) -mo 1 --sum $(CIMCLIHOSTID)
	cimcli pni $(STRESSCXX_CLASS_ID) -mo 1 --sum  $(CIMCLIHOSTID)

cmpigc:
	cimcli gc $(CMPICL) -n root/SampleProvider $(CIMCLIHOSTID)

## NONTest (i.e. no test criteria) of CMPI provider enumerate Instances
## DELETE THIS
cmpie1:
	cimcli pei $(CMPICL) -n root/SampleProvider $(CIMCLIHOSTID)
	cimcli pei $(CMPICL) -n root/SampleProvider $(CIMCLIHOSTID) -pl Message
	cimcli pei $(CMPICL) -n root/SampleProvider $(CIMCLIHOSTID) -pl ""

## reference tests
clicmpir1:
	cimcli pr $(CMPIASSOCOBJ) -n test/TestProvider $(CIMCLIHOSTID)

pcmpir1:
	$(PROGRAM) r -M 1 -N 1 -c $(CMPIASSOCOBJ) -n test/TestProvider -C -T $(HOSTID)

pcmpirn1:
	$(PROGRAM) rn -M 1 -N 1 -c $(CMPIASSOCOBJ) -n test/TestProvider -C -T  $(HOSTID)

## Reference Operations Simple tests

clicmpirn1:
	cimcli rn $(CMPIASSOCOBJ) -n test/TestProvider $(CIMCLIHOSTID)

clicmpipr1:
	cimcli r $(CMPIASSOCOBJ) -n test/TestProvider $(CIMCLIHOSTID)

clicmpiprn1:
	cimcli rn $(CMPIASSOCOBJ) -n test/TestProvider $(CIMCLIHOSTID)

clicmpir2:
	cimcli r CMPI_TEST_Person.name=\"Melvin\" -rc CMPI_TEST_Racing -n test/TestProvider $(CIMCLIHOSTID)

## Manual test to assure that the reset works as it should
testSetReset:

	cimcli im $(STRESSCXX_CLASS_ID) get $(CIMCLIHOSTID)	
	cimcli im $(STRESSCXX_CLASS_ID) set Size=100 ResponseCount=1000 $(CIMCLIHOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) get $(CIMCLIHOSTID)	
	cimcli im $(STRESSCXX_CLASS_ID) reset $(CIMCLIHOSTID)
	cimcli im $(STRESSCXX_CLASS_ID) get $(CIMCLIHOSTID)	

