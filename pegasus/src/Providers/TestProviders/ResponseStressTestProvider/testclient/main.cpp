//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
//
//%/////////////////////////////////////////////////////////////////////////////

/*
    Test of  the ResponseStress Provider with operations that stress the
    CIM Operationresponse mechanism with both verylarge response objects and
    very large numbers of response objects.
    This client works specifically with the corresponding provider and
    depends on the information in the TST_ResponseStressTest class.
    It is only a test of the providers capability to execute its options.
*/
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/General/Stopwatch.h>
#include <Pegasus/Common/Pegasus_inl.h>
#include <Pegasus/General/Stopwatch.h>
#include <cstdarg>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define VCOUT if (verbose) cout

static const CIMNamespaceName NAMESPACE = CIMNamespaceName("test/TestProvider");

// test with two different classes.  Note that they are actually registered
// to the same provider but each has its own private area for the
// behavior parameters that are changed with the invokeMethod for
// the superclass
static const CIMName TEST_CLASS1 = CIMName("TST_ResponseStressTestCxx");
static const CIMName TEST_CLASS2 = CIMName("TST_ResponseStressTestCxx2");

static Boolean verbose;

/*
    This structure encapsulates all of the parameters for the get and
    set methods for the ResponseStressTest Class with functions for
    setting, getting, displaying  and comparing them.
    ClassName is part of definition to allow for the capability to
    operation on multiple classes. The ResponseStressTestCxx provider
    is capable of maintaining separate sets of parameters for different
    request classNames and namespaces.
*/
typedef struct MethodParameters
{
    MethodParameters(const CIMName& cl) :  _responseCount(0),_instanceSize(0),
        _countToFail(0), _failureStatusCode(CIM_ERR_SUCCESS), _delay(0),
        _className(cl)
    {}

    // Test with responseCount and instanceSize required parameters
    // and others optional
    MethodParameters(const CIMName& cl, Uint64 responseCount,
        Uint64 instanceSize,
        Uint64 countToFail = 0,
        Uint32 failureStatusCode = 0,
        Uint32 delay = 0)
        :
        _responseCount(responseCount),
        _instanceSize(instanceSize),
        _countToFail(countToFail),
        _failureStatusCode((CIMStatusCode)failureStatusCode),
        _delay(delay),
        _className(cl)
    {}

    MethodParameters(const CIMName& cl, Uint64 responseCount,
        Uint64 instanceSize,
        Uint64 countToFail,
        CIMStatusCode failureStatusCode,
        Uint32 delay = 0)
        :
        _responseCount(responseCount),
        _instanceSize(instanceSize),
        _countToFail(countToFail),
        _failureStatusCode(failureStatusCode),
        _delay(delay),
        _className(cl) {}

    // Sets the parameters back to what the provider original compile
    // setting should be.  If these are changed in the provider build
    // they should be changed here also.
    void setOrigParamValues()
    {
        _responseCount = 5;
        _instanceSize = 100;
        _countToFail = 0;
        _failureStatusCode = CIM_ERR_SUCCESS;
        _delay = 0;
    }

    // Tests values for equality, not the classname
    bool equal(const MethodParameters& mp)
    {
        if (_responseCount != mp._responseCount)
        {
            VCOUT << "Compare fail responseCount" << endl;
            return false;
        }
        else if(_instanceSize != mp._instanceSize)
        {
            VCOUT << "Compare fail instanceSize" << endl;
            return false;
        }
        else if(_countToFail != mp._countToFail)
        {
            VCOUT << "Compare fail countToFail" << endl;
            return false;
        }
        else if(_failureStatusCode != mp._failureStatusCode)
        {
            VCOUT << "Compare fail failureStatusCode" << endl;
            return false;
        }
        else if (_delay != mp._delay)
        {
            VCOUT << "Compare Delay parmaeters" << endl;
            return false;
        }
        return true;
    }

    void setResponseCount(Uint64 x)
    {
        _responseCount = x;
    }
    void setCountToFail(Uint64 x)
    {
        _countToFail = x;
    }
    void setFailureStatusCode(CIMStatusCode x)
    {
        _failureStatusCode = x;
    }
    void setFailureStatusCode(Uint32 x)
    {
        _failureStatusCode = (CIMStatusCode)x;
    }

    void setDelay(Uint32 x)
    {
        _delay = x;
    }
    String toString()
    {
        String rtn;
        rtn.appendPrintf("ClassName=%s responseCount=%llu instanceSize=%llu "
            "countToFail=%llu _failureStatusCond=%u (%s) _delay=%u",
            (const char*)_className.getString().getCString(),
            _responseCount, _instanceSize, _countToFail,
            (Uint32)_failureStatusCode,
            cimStatusCodeToString(_failureStatusCode),
            _delay);
        return rtn;
    }

    Uint64 _responseCount;
    Uint64 _instanceSize;
    Uint64 _countToFail;
    CIMStatusCode _failureStatusCode;
    Uint32 _delay;
    CIMName _className;

} methodParameters;

ClientOpPerformanceData returnedPerformanceData;

class ClientStatistics : public ClientOpPerformanceDataHandler
{
public:

    virtual void handleClientOpPerformanceData (
            const ClientOpPerformanceData & item)
    {
        if (!(0 <= item.operationType) || !(39 >= item.operationType))
        {
           cerr << "Operation type out of expected range in"
                        " ClientOpPerformanceData "
               << endl;
           exit(1);
        }
        returnedPerformanceData.operationType =  item.operationType;
        if (item.roundTripTime == 0)
        {
           cerr << "roundTripTime is incorrect in ClientOpPerformanceData "
               << endl;
        }
        returnedPerformanceData.roundTripTime =  item.roundTripTime;

        if (item.requestSize == 0)
        {
            cerr << "requestSize is incorrect in ClientOpPerformanceData "
                << endl;
        }
        returnedPerformanceData.requestSize =  item.requestSize;

        if (item.responseSize == 0)
        {
            cerr << "responseSize is incorrect in ClientOpPerformanceData "
                << endl;
        }
        returnedPerformanceData.responseSize =  item.responseSize;

        if (item.serverTimeKnown)
        {
            /* Bypass this because we are getting server times zero
            if (item.serverTime == 0)
            {
                cerr << "serverTime is incorrect in ClientOpPerformanceData "
                    << endl;
            }
            */
            returnedPerformanceData.serverTime =  item.serverTime;
            returnedPerformanceData.serverTimeKnown =  item.serverTimeKnown;
            returnedPerformanceData.roundTripTime =  item.roundTripTime;
        }
   }
   //returnedPerformanceData = item;   // Copy the data to public place
};

Boolean first = true;
void _displayTimes( Uint32 objCount, Uint32 objSize, Uint64 elapsed,
                    ClientOpPerformanceData& perfData)
{
    if (first)
    {
        if (!perfData.serverTimeKnown)
        {
            cout << "WARNING: Server statistics data may not be valid" << endl;
        }
        printf("Op ClientTime roundTripTime "
               "serverTime  ReqSize      RespSize ObjSize "
               " ObjCount\n");
        first = false;
    }

    printf("%2u %10lu %13lu %10lu %8lu %12lu  %7u %9u\n",
           perfData.operationType,
           (long unsigned int) elapsed,
           (long unsigned int)perfData.roundTripTime,
           (long unsigned int)perfData.serverTime,
           (long unsigned int) perfData.requestSize,
           (long unsigned int) perfData.responseSize,
           objSize, objCount);
}
// Enumerate the instances and confirm results
void enumerateInstances(CIMClient& client, const methodParameters& mp,
                        CIMPropertyList& pl)
{
    Stopwatch sw;
    sw.start();

    Array<CIMInstance> instances =
        client.enumerateInstances(NAMESPACE, mp._className,
                                  true, true, false, false, pl);

    sw.stop();
    Uint64 elapsed = sw.getElapsedUsec();

    _displayTimes(mp._responseCount, instances.size(), elapsed,
                  returnedPerformanceData);
    //sw.printElapsed();
//      Uint64 rtTotalTime = 0;
//      rtTotalTime = (returnedPerformanceData.roundTripTime);
//      Uint64 serverTime = returnedPerformanceData.serverTime;
    PEGASUS_TEST_ASSERT(instances.size() == mp._responseCount);

    // Confirm that the sequence numbers are monolithic increasing
    //
    Uint64 prevSequenceNumber = 0;
    for (Uint64 i = 0, n = instances.size() ;  i < n ; i++)
    {
        Uint32 pos;
        if ((pos = instances[i].findProperty("SequenceNumber"))
             != PEG_NOT_FOUND )
        {
            CIMProperty p = instances[i].getProperty(pos);
            CIMValue v = p.getValue();
            Uint64 sequenceNumber;
            v.get(sequenceNumber);
            //cout << "SequenceNumber = " << sequenceNumber
            //    << " prevSequenceNumber " << prevSequenceNumber << endl;

            PEGASUS_TEST_ASSERT(sequenceNumber == (prevSequenceNumber));
            prevSequenceNumber++;
        }
    }
}

// Test to validate instance names. Executes the enumerate and tests the
// number of responses against the expected response count
void enumerateInstanceNames(CIMClient& client, const methodParameters& mp)
{
    Array<CIMObjectPath> paths =
        client.enumerateInstanceNames(NAMESPACE, mp._className);

    VCOUT << "count = " << paths.size() << endl;
    if (verbose)
    {
        cout << endl;
        for (Uint32 i = 0; i < paths.size() ; i++)
        {
            cout << paths[i].toString() << endl;
        }
    }
    PEGASUS_TEST_ASSERT(paths.size() == mp._responseCount);
}

// set the instanceSize and response count parameters in the
// client
void set(CIMClient& client, const methodParameters& mp)
{
    Array<CIMParamValue> InParams;

    Array<CIMParamValue> outParams;

    InParams.append(CIMParamValue("ResponseCount", mp._responseCount));
    InParams.append(CIMParamValue("Size", mp._instanceSize));
    InParams.append(CIMParamValue("CountToFail", mp._countToFail));
    InParams.append(CIMParamValue("FailureStatusCode",
        (Uint32)mp._failureStatusCode));
    InParams.append(CIMParamValue("Delay", mp._delay));

    CIMValue returnValue = client.invokeMethod(
        NAMESPACE,
        CIMObjectPath(String::EMPTY,
                      CIMNamespaceName(),
                      CIMName(mp._className)),
        CIMName("set"),
        InParams,
        outParams);

    PEGASUS_TEST_ASSERT(outParams.size() == 0);

    Uint32 rc;
    returnValue.get(rc);

    PEGASUS_TEST_ASSERT(rc == 0);
}

// Issue the reset invoke method operation and wait for good response.
// for a single Class.  The mp parameter defines the class
void reset(CIMClient& client,  const methodParameters& mp)
{
    Array<CIMParamValue> InParams;
    Array<CIMParamValue> outParams;

    CIMValue returnValue = client.invokeMethod(
        NAMESPACE,
        CIMObjectPath(String::EMPTY,
            CIMNamespaceName(),
            CIMName(mp._className)),
        CIMName("reset"),
        InParams,
        outParams);

    Uint32 rc;
    returnValue.get(rc);
    PEGASUS_TEST_ASSERT(rc == 0);
}
// get the current provider test parameters.
void get(CIMClient& client, methodParameters& mp)
{
    Array<CIMParamValue> InParams;
    Array<CIMParamValue> outParams;

    CIMValue returnValue = client.invokeMethod(
        NAMESPACE,
        CIMObjectPath(String::EMPTY,
            CIMNamespaceName(),
            CIMName(mp._className)),
        CIMName("get"),
        InParams,
        outParams);

    Uint32 rc;
    returnValue.get(rc);
    PEGASUS_TEST_ASSERT(rc == 0);

    for(Uint32 i = 0; i < outParams.size(); ++i)
    {
        String paramName = outParams[i].getParameterName();
        CIMValue v = outParams[i].getValue();

        if(paramName =="ResponseCount")
        {
            v.get(mp._responseCount);
        }
        else if(paramName =="Size")
        {
            v.get(mp._instanceSize);
        }
        else if(paramName =="CountToFail")
        {
            v.get(mp._countToFail);
        }
        else if(paramName =="FailureStatusCode")
        {
            Uint32 x;
            v.get(x);
            mp._failureStatusCode = (CIMStatusCode)x;
        }
        else if(paramName =="Delay")
        {
            v.get(mp._delay);
        }
        else
        {
            cout <<"Error: unknown parameter name = " << paramName << endl;
            PEGASUS_TEST_ASSERT(false);
        }
    }
}

// Confirms that the provider parameters are set to the original values
// for a single className.
// This is the only place in the test client that the original parameter
// values should appear
// If they are incorrect, it sets them to orig value but still returns
// false since we are really using this to confirm that they are set
// right.

bool confirmOriginalValues(CIMClient& client, const CIMName& className)
{
    // This object which should be set to the original parameters defined
    // for the provider
    methodParameters orig(className);
    orig.setOrigParamValues();

    VCOUT << "orig = " << orig.toString() << endl;

    methodParameters rtn(className);
    get(client, rtn);
    VCOUT << "getRtn = " << rtn.toString() << endl;

    // This confirms that our orig is really the provider defaults.
    if (!rtn.equal(orig))
    {
        cerr << "Error in test of default method parameters"
            << " for class " << className.getString()
            << ". Expected " << orig.toString()
            << ". Received " << rtn.toString() << endl;

        set(client, orig);
        return false;
    }
    return true;
}
// fTest for the set, get, and reset methods for a single className.
void testSetAndGetMethods(CIMClient& client, const CIMName& className)
{
    // Normally this tests and if wrong, sets them. Therefore no
    // assert. This covers case were for some reason, they were set
    // wrong by previous test, etc.
    confirmOriginalValues(client, className);

    // They have just been set to default. They  MUST be right now
    PEGASUS_TEST_ASSERT(confirmOriginalValues(client, className));

    // Test changes to count and size
    methodParameters test1(className,1500, 2000);
    set(client, test1);

    methodParameters test1Rtn = test1;
    get(client, test1Rtn);
    PEGASUS_TEST_ASSERT(test1.equal(test1Rtn));

    // set back to origin
    methodParameters orig(className);
    orig.setOrigParamValues();
    set(client, orig);

    methodParameters test2Rtn(className);
    get(client, test2Rtn);
    PEGASUS_TEST_ASSERT(test2Rtn.equal(orig));

    // Test to confirm that the reset works.
    // First set all parameters
    methodParameters x1(className, 2000, 3000, CIM_ERR_FAILED, 4, 900);
    set(client, x1);
    // Reset
    reset(client, x1);
    // get and compare with the orig definition.
    get(client, x1);
    PEGASUS_TEST_ASSERT(x1.equal(orig));

    // assumes provider set to orig.
    methodParameters testFailureParams = orig;
    testFailureParams.setFailureStatusCode(CIM_ERR_FAILED);
    testFailureParams.setCountToFail(4);

    set(client, testFailureParams);
    methodParameters testFailureParamsRtn = testFailureParams;
    get(client, testFailureParamsRtn);
    PEGASUS_TEST_ASSERT(testFailureParams.equal(testFailureParamsRtn));

    // reset to the original parameters.
    set(client, orig);

    methodParameters rtn = orig;
    get(client, rtn);
    VCOUT << "getRtn = " << rtn.toString() << endl;

    // test setting delay parameter
    methodParameters testDelayParams = orig;
    testDelayParams.setDelay(10);
    testDelayParams.setResponseCount(290);

    methodParameters testDelayParamsRtn = testDelayParams;
    set(client, testDelayParams);
    get(client,testDelayParamsRtn);
    PEGASUS_TEST_ASSERT(testDelayParamsRtn._delay == 10);
    VCOUT << "testDelayParams" << testDelayParamsRtn.toString() << endl;
    PEGASUS_TEST_ASSERT(testDelayParams.equal(testDelayParamsRtn));

    // test if this actually delays. Delay should be about 10 seconds.
    VCOUT << "Expect a delay of about 20 sec here" << endl;
    Stopwatch timer;

    timer.start();
    enumerateInstanceNames(client, testDelayParams);
    timer.stop();

    // test if delay gt 9 sec.
    PEGASUS_TEST_ASSERT(timer.getElapsed() > (double)9);

    // This confirms that our orig is really the provider defaults.

    // reset to the original parameters.
    set(client, orig);
    get(client, rtn);
    VCOUT << "getRtn = " << rtn.toString() << endl;
    PEGASUS_TEST_ASSERT(rtn.equal(orig));

    VCOUT << "Passed testSetAndGetMethods." << endl;
}

// reset all of the parameters for both classes to the original values
void resetAllParameters(CIMClient& client)
{
    methodParameters mpreset1(TEST_CLASS1);
    reset(client, mpreset1);
    confirmOriginalValues(client, TEST_CLASS1);

    methodParameters mpreset2(TEST_CLASS2);
    reset(client, mpreset2);
    confirmOriginalValues(client, TEST_CLASS2);
}
void testSetAndGetMethodIndependence(CIMClient& client,
                                     const CIMName& cl1,
                                     const CIMName& cl2 )
{
    methodParameters orig(cl1);
    orig.setOrigParamValues();
    resetAllParameters(client);
    methodParameters mp1(cl1, 2000, 3000, CIM_ERR_FAILED, 4, 900);
    methodParameters mp2(cl2);
    set(client, mp1);
    get(client, mp2);

    // confirm that the other class parameters has not changed.
    PEGASUS_TEST_ASSERT(!mp1.equal(mp2));
    PEGASUS_TEST_ASSERT(mp2.equal(orig));
    reset(client, mp1);

    // Set for both classes and test that correct values are returned for
    // each

    methodParameters mp3(cl1, 2222, 33333, CIM_ERR_SUCCESS, 3, 9999);

    methodParameters mp4(cl2, 9000, 4000, CIM_ERR_FAILED, 5, 1100);
    set(client, mp3);
    set(client, mp4);
    methodParameters rtnmp3(cl1);
    methodParameters rtnmp4(cl2);
    get(client, rtnmp3);
    get(client, rtnmp4);
    PEGASUS_TEST_ASSERT(mp3.equal(rtnmp3));
    PEGASUS_TEST_ASSERT(mp4.equal(rtnmp4));

    resetAllParameters(client);

    get(client, rtnmp3);
    get(client, rtnmp4);

    PEGASUS_TEST_ASSERT(rtnmp3.equal(rtnmp4));
    PEGASUS_TEST_ASSERT(rtnmp3.equal(orig));
}

int main(int argc, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    CIMClient client;

    // Register for Client statistics.
    ClientStatistics statistics = ClientStatistics();

    client.registerClientOpPerformanceDataHandler(statistics);
    try
    {
        client.connectLocal();
        //client.connect("localhost", 5988, String(), String());
    }
    catch (Exception& e)
    {
        cerr << "Error: Connect Failure " << e.getMessage() << endl;
        exit(1);
    }

    try
    {
        // if no argument conduct simple standard test to validate the
        // operation of the provider.
        if (argc <=1)
        {
            // test the invoke methods for the two defined classes
            testSetAndGetMethods(client, TEST_CLASS1);
            testSetAndGetMethods(client, TEST_CLASS2);

            // test that they are independently settable.
            testSetAndGetMethodIndependence(client, TEST_CLASS1, TEST_CLASS2);

            // Execute Enumerate and confirm results
            // set test parameters to 150 = size, 20 = response count
            methodParameters mp(TEST_CLASS1,20,150);

            set(client, mp);

            enumerateInstanceNames(client, mp);

            CIMPropertyList pl;
            enumerateInstances(client, mp, pl);

            Array<CIMName> pn;
            pn.append("Pattern");
            pn.append("s1");
            pn.append("Id");
            pn.append("SequenceNumber");
            pl.set(pn);
            enumerateInstances(client, mp, pl);
        }
        // This is a real stress test and should be only used for manual
        // test, not the nightly tests.
        else if (strcmp(argv[1],"test") == 0)
        {
            Array<Uint64> objSize;
            objSize.append(100);
            objSize.append(1000);
            objSize.append(10000);
            objSize.append(50000);

            Array<Uint64> objCount;
            objCount.append(100);
            objCount.append(1000);
            objCount.append(10000);
            objCount.append(50000);
            objCount.append(100000);
            objCount.append(200000);
            objCount.append(250000);
            for (Uint32 x = 0; x < objSize.size() ; x++)
            {
                for(Uint32 y = 0; y < objCount.size(); y++)
                {
                    methodParameters mp(TEST_CLASS1,
                                        objSize[x], objCount[y]);
                    set(client, mp);
                    CIMPropertyList pl1;
                    enumerateInstances(client, mp, pl1);

                    Array<CIMName> pn;
                    pn.append("Pattern");
                    pn.append("s1");
                    pn.append("Id");
                    pn.append("SequenceNumber");
                    pl1.set(pn);
                    enumerateInstances(client, mp, pl1);
                }
            }
        }

    }
    catch (Exception& e)
    {
        cerr << "Error: Connect or standard tests " << e.getMessage() << endl;
        exit(1);
    }

    VCOUT << "Start Error reponse Tests. " << endl;
    // test to confirm that the countToFailure and failureStatusCode
    // work. This definition should fail after 30 objects returned.
    methodParameters mpFail(TEST_CLASS1,
                            20,150,10, CIM_ERR_FAILED);

    set(client, mpFail);
    try
    {
        CIMPropertyList pl;
        enumerateInstances(client, mpFail, pl);
        PEGASUS_TEST_ASSERT(false);    // should not reach here.
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_FAILED)
        {
            cout << "ERROR: expected CIM_ERR_FAILED. got " << e.getCode()
                << e.getMessage() << endl;
            PEGASUS_TEST_ASSERT(false);
        }
    }

    try
    {
        enumerateInstanceNames(client, mpFail);
        PEGASUS_TEST_ASSERT(false);
    }
    catch (CIMException& e)
    {
        if (e.getCode() != CIM_ERR_FAILED)
        {
            cout << "ERROR: expected CIM_ERR_FAILED. got " << e.getCode()
                << e.getMessage() << endl;
            PEGASUS_TEST_ASSERT(false);
        }
    }

    mpFail.setCountToFail(155);    // Should not fail
    set(client, mpFail);

    try
    {
        enumerateInstanceNames(client, mpFail);
    }
    catch (CIMException& e)
    {
        cout << "ERROR: Count should be greater than number of resposnes.got "
             << e.getCode() << e.getMessage() << endl;
        PEGASUS_TEST_ASSERT(false);
    }

    // reset provider to default in case another test.

    resetAllParameters(client);

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
