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

#ifndef ClientOpPerformanceDataHandler_h
#define ClientOpPerformanceDataHandler_h

#include <Pegasus/Common/CIMOperationType.h>
#include <Pegasus/Client/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    A ClientOpPerformanceData object contains performance data for a single
    CIM operation.  It is used by the ClientOpPerformanceDataHandler callback.
*/
struct PEGASUS_CLIENT_LINKAGE ClientOpPerformanceData
{
    /** Identifies the operation type for the statistical information
        provided.
    */
    CIMOperationType operationType;

    /** Indicates whether the serverTime member contains valid data.  This
        flag is true if a WBEMServerResponseTime value is received from the
        CIM Server, false otherwise.
    */
    Boolean serverTimeKnown;

    /** Contains the number of microseconds elapsed during the CIM Server
        processing of the request.
        WARNING: This elapsed time may be constrained by limitations in the
        granularity of the system clock on certain platforms.
    */
    Uint64 serverTime;

    /** Contains the number of microseconds elapsed during the complete
        processing of the request, including time spent on the network and
        in the CIM Server (serverTime).
        WARNING: This elapsed time may be constrained by limitations in the
        granularity of the system clock on certain platforms.
    */
    Uint64 roundTripTime;

    /** Contains the size of the request message (in bytes).
    */
    Uint64 requestSize;

    /** Contains the size of the response message (in bytes).
    */
    Uint64 responseSize;
};


/** A ClientOpPerformanceDataHandler subclass object may be registered with a
    CIMClient object by a client application.  The subclass object must not
    be destructed while it is registered with a CIMClient object, so it is
    recommended that these objects have the same scope.
*/
class PEGASUS_CLIENT_LINKAGE ClientOpPerformanceDataHandler
{
public:

    virtual ~ClientOpPerformanceDataHandler();

    /**
        Processes client operation performance data.  When a
        ClientOpPerformanceDataHandler subclass object is registered with a
        CIMClient object, this method is called by the CIMClient object with
        performance data for each completed CIM operation.

        This method may, for example, accumulate the performance data to
        calculate average processing times for multiple operations.

        Exceptions thrown by this method are not caught by the CIMClient.
        Therefore, a client application would receive an exception from this
        method in place of CIM operation response data.

        @param item A ClientOpPerformanceData object containing performance
        data for a single CIM operation.
    */
    virtual void handleClientOpPerformanceData(
        const ClientOpPerformanceData& item) = 0;
};


/*

The following example shows how a ClientOpPerformanceDataHandler callback may
be used by a client application.

// A ClientOpPerformanceDataHandler implementation that simply prints the
// data from the ClientOpPerformanceData object.

class ClientStatisticsAccumulator : public ClientOpPerformanceDataHandler
{
public:

    virtual void handleClientOpPerformanceData(
        const ClientOpPerformanceData& item)
    {
        cout << "ClientStatisticsAccumulator data:" << endl;
        cout << " operationType is " << (Uint32)item.operationType << endl;
        cout << " serverTime is " << (Uint32)item.serverTime << endl;
        cout << " roundTripTime is " << (Uint32)item.roundTripTime << endl;
        cout << " requestSize is " << (Uint32)item.requestSize << endl;
        cout << " responseSize is " << (Uint32)item.responseSize << endl;
        if (item.serverTimeKnown)
        {
            cout << " serverTimeKnown is true" << endl;
        }
        else
        {
            cout << " serverTimeKnown is false" << endl;
        }
    }
};

int main(int argc, char** argv)
{
    // Establish the namespace from the input parameters
    String nameSpace = "root/cimv2";

    //Get hostname
    String location = "localhost";

    //Get port number
    Uint32 port = 5988;

    //Get user name and password
    String userN;
    String passW;

    // Connect to the server

    String className = "PG_ComputerSystem";
    CIMClient client;
    // Note: The ClientStatisticsAccumulator object should have the same
    // scope as the CIMClient object.
    ClientStatisticsAccumulator accumulator;

    try
    {
        client.connect(location, port, userN, passW);
    }
    catch (Exception& e)
    {
        cerr << argv[0] << "Exception connecting to: " << location << endl;
        cerr << e.getMessage() << endl;
        exit(1);
    }

    ///////////////////////////////////////////////////
    // Register callback and EnumerateInstances
    /////////////////////////////////////////////////////

    client.registerClientOpPerformanceDataHandler(accumulator);

    try
    {
        Array<CIMObjectPath> instances;

        // Note: Completion of this CIMClient operation will invoke the
        // ClientOpPerformanceDataHandler callback.
        instances = client.enumerateInstanceNames(nameSpace, className);
    }
    catch (Exception& e)
    {
        cerr << "Exception: " << e.getMessage() << endl;
        exit(1);
    }

    return 0;
}

*/


PEGASUS_NAMESPACE_END

#endif /* ClientOpPerformanceDataHandler_h */


