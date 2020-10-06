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

/* This is a simplistic display program for the CIMOM performance
    characteristics.
    This version simply gets the instances of the performace class and displays
    the resulting average counts.
    TODO  KS
    1. Convert to use the correct class when it is available.
    2. Get the header information from the class, not fixed.
    3. Keep history and present so that there is a total
    4. Do Total so that we have overall counts.
    5. Do percentages
*/
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <stdlib.h>
#include <Pegasus/Common/HTTPConnector.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/HostAddress.h>

#include <Pegasus/Client/CIMClient.h>

#include <Pegasus/General/OptionManager.h>
#ifdef PEGASUS_OS_ZOS
#include <Pegasus/General/SetFileDescriptorToEBCDICEncoding.h>
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const String DEFAULT_NAMESPACE = "root/cimv2";

// The table represents the mapping from the enumerated types
// in the CIM_CIMOMStatisticalDate class ValueMap qualifier for the
// operationType property to the message type defined in the value for
// OperationType. This table must match the table in StatisticalData.cpp
//
const char* operationName[] =
{
    //                                   ValueMap Value
    //                                   from class CIM_StatisticalData
    //                                   -------------------
    "Unknown",                        //    0
    "Other",                          //    1
    "Batch",                          //    2
    "GetClass",                       //    3
    "GetInstance",                    //    4
    "DeleteClass",                    //    5
    "DeleteInstance",                 //    6
    "CreateClass",                    //    7
    "CreateInstance",                 //    8
    "ModifyClass",                    //    9
    "ModifyInstance",                 //   10
    "EnumerateClasses",               //   11
    "EnumerateClassNames",            //   12
    "EnumerateInstances",             //   13
    "EnumerateInstanceNames",         //   14
    "ExecQuery",                      //   15
    "Associators",                    //   16
    "AssociatorNames",                //   17
    "References",                     //   18
    "ReferenceNames",                 //   19
    "GetProperty",                    //   20
    "SetProperty",                    //   21
    "GetQualifier",                   //   22
    "SetQualifier",                   //   23
    "DeleteQualifier",                //   24
    "EnumerateQualifiers",            //   25
    "IndicationDelivery",             //   26
    "InvokeMethod"                    //   Not Present, use 1 ((:Other")
};

Uint32 operationNameSize = sizeof(operationName) /
    sizeof (operationName[0]);

/* Method to build an OptionManager object - which holds and organizes options
   and the properties */

void GetOptions(
    OptionManager& om,
    int& argc,
    char** argv,
    const String& testHome)
{
    static struct OptionRow optionsTable[] =
    //The values in the OptionRows below are:
    //optionname, defaultvalue, is required, type, domain, domainsize, flag,
    //  hlpmsg
    {
        {"port", "5988", false, Option::INTEGER, 0, 0, "p",
            "specifies port"},

        {"location", "localhost", false, Option::STRING, 0, 0, "h",
                "specifies hostname of system"},

        {"version", "false", false, Option::BOOLEAN, 0, 0, "-version",
                "Displays software Version "},

        {"help", "false", false, Option::BOOLEAN, 0, 0, "-help",
                "Prints help message with command line options "},

        {"user name","",false,Option::STRING, 0, 0, "u",
                "specifies user loging in"},

        {"password","",false,Option::STRING, 0, 0, "w",
                "login password for user"},

    };
    const Uint32 NUM_OPTIONS = sizeof(optionsTable) / sizeof(optionsTable[0]);

    om.registerOptions(optionsTable, NUM_OPTIONS);

    String configFile = "/CLTest.conf";

    if (FileSystem::exists(configFile))
    {
        om.mergeFile(configFile);
    }

    om.mergeCommandLine(argc, argv);

    om.checkRequiredOptions();
}


/* Method that maps from operation type to operation name.
*/

const char* opTypeToOpName(Uint16 type)
{
    const char* opName = NULL;
    if (type < operationNameSize)
    {
        opName = operationName[type];
    }
    else
    {
        opName = "Dummy Response";
    }
    return opName;
}

int main(int argc, char** argv)
{

#ifdef PEGASUS_OS_ZOS
    // for z/OS set stdout and stderr to EBCDIC
    setEBCDICEncoding(STDOUT_FILENO);
    setEBCDICEncoding(STDERR_FILENO);
#endif

    // Get options (from command line and from configuration file); this
    // removes corresponding options and their arguments from the command
    // line.

    OptionManager om;

    try
    {
        String testHome = ".";
        GetOptions(om, argc, argv, testHome);
        // om.print();
    }
    catch (Exception& e)
    {
        cerr << argv[0] << ": " << e.getMessage() << endl;
        String header = "Usage ";
        String trailer = "";
        om.printOptionsHelpTxt(header, trailer);
        exit(1);
    }

    // Establish the namespace from the input parameters
    String nameSpace = "root/cimv2";

    // Check to see if user asked for help (--help option)
    if (om.valueEquals("help", "true"))
    {
        String header = "Usage ";
        String trailer = "";
        om.printOptionsHelpTxt(header, trailer);
        exit(0);
    }
    else if (om.valueEquals("version", "true"))
    {
        cerr << "Version " << PEGASUS_PRODUCT_VERSION << endl;
        exit(0);
    }

    //Get hostname form (option manager) command line if none use default
    String location;
    om.lookupValue("location", location);
    HostAddress addr;
    if (!addr.setHostAddress(location))
    {
        cerr << "Invalid Locator : " << location << endl;
        exit(1);
    }

    // Get port number from (option manager) command line; if none use the
    // default.  The lookup will always be successful since the optionTable
    // has a default value for this option.
    String str_port;
    Uint32 port = 0;
    if (om.lookupValue("port", str_port))
    {
        port = (Uint32) atoi(str_port.getCString());
    }

    //Get user name and password
    String userN;
    String passW;
    om.lookupValue("user name", userN);
    om.lookupValue("pass word", passW);

    /*
    The next section of code connects to the server and enumerates all the
    instances of the CIM_CIMOMStatisticalData class. The instances are held in
    an Array named "instances". The output of cimperf is a table of averages.
    */


    String className = "CIM_CIMOMStatisticalData";
    CIMClient client;

    try
    {
        if (String::equal(location,"localhost"))
            client.connectLocal();
        else
            client.connect(location, port, userN, passW);
    }

    catch (Exception& e)
    {
        cerr << argv[0] << " Exception connecting to : " << location << endl;
        cerr << e.getMessage() << endl;
        exit(1);
    }


    try
    {
        Boolean localOnly = false;
        Boolean deepInheritance = false;
        Boolean includeQualifiers = false;
        Boolean includeClassOrigin = false;

        Array<CIMInstance> instances;
        instances = client.enumerateInstances(nameSpace,
            className,
            deepInheritance,
            localOnly,
            includeQualifiers,
            includeClassOrigin);

        // First print the header for table of values
        printf("%-25s%10s %10s %10s %10s %10s\n",
            "Operation", "Number of", "Server", "Provider", "Request",
            "Response");

        printf("%-25s%10s %10s %10s %10s %10s\n",
            "Type", "Requests", "Time", "Time", "Size", "Size");

        printf("%-25s%10s %10s %10s %10s %10s\n",
            " ", " ", "(usec)", "(usec)", "(bytes)", "(bytes)");

        printf("%-25s\n", "-------------------------------------------"
                          "------------------------------------");

        // This section of code loops through all the instances of
        // CIM_CIMOMStatisticalData (one for each intrinsic request type) and
        // gathers the NumberofOperations, CIMOMElapsedTime,
        // ProviderElapsedTime, ResponseSize and RequestSize for each instance.
        // Averages are abtained by dividing times and sizes by
        // NumberofOperatons.

        for (Uint32 inst = 0; inst < instances.size(); inst++)
        {
            CIMInstance instance = instances[inst];

            // Get the request type property for this instance.
            // Note that for the moment it is simply an integer.
            Uint32 pos;
            CIMProperty p;
            // Operation Type is decoded as const char*, hence type has
            // changed from string to const char*
            String statName;
            CIMValue v;
            Uint16 type;
            if ((pos = instance.findProperty("OperationType")) != PEG_NOT_FOUND)
            {
                p = (instance.getProperty(pos));
                v = p.getValue();
                if (v.getType() == CIMTYPE_UINT16)
                {
                    v.get(type);
                    if (type != 1)
                    {
                        statName = opTypeToOpName(type);
                    }

                    // 1 is Other type indicating that there is another
                    // property with the name.
                    else
                    {
                        if ((pos = instance.findProperty("OtherOperationType"))
                             != PEG_NOT_FOUND)
                        {
                            CIMProperty pOther = (instance.getProperty(pos));
                            CIMValue vOther = pOther.getValue();
                            if (vOther.getType() == CIMTYPE_STRING)
                            {
                                vOther.get(statName);
                            }
                        }
                        else
                        {
                            statName = "UNKNOWN";
                        }
                    }
                }
            }
            else
            {
                statName = "UNKNOWN";
            }

            // Get number of requests property - "NumberofOperations"
            Uint64 numberOfRequests = 0;
            if ((pos = instance.findProperty("NumberOfOperations")) !=
                PEG_NOT_FOUND)
            {

                p = (instance.getProperty(pos));
                v = p.getValue();

                if (v.getType() == CIMTYPE_UINT64)
                {
                    v.get(numberOfRequests);

                }
                else
                {
                    cerr << "NumberofOperations was not a CIMTYPE_SINT64 and"
                            " should be" << endl;
                }
            }
            else
            {
                cerr << "Could not find NumberofOperations" << endl;
            }

            // Get the total CIMOM Time property "CIMOMElapsedTime"
            // in order to calculate the averageCimomTime.
            CIMDateTime totalCimomTime;
            Sint64 averageCimomTime = 0;
            Uint64 totalCT = 0;

            if ((pos = instance.findProperty("CimomElapsedTime")) !=
                PEG_NOT_FOUND)
            {
                p = (instance.getProperty(pos));
                v = p.getValue();

                if (v.getType() == CIMTYPE_DATETIME)
                {
                    v.get(totalCimomTime);
                    totalCT = totalCimomTime.toMicroSeconds();
                }
                else
                {
                    cerr << "Error Property value " << "CimomElapsedTime" <<
                        endl;
                }
            }
            else
            {
                cerr << "Error Property " << "CimomElapsedTime" << endl;
            }

            if (numberOfRequests != 0)
            {
                averageCimomTime = totalCT / numberOfRequests;
            }

            // Get the total Provider Time property "ProviderElapsedTime"
            CIMDateTime totalProviderTime;
            Uint64 averageProviderTime = 0;
            Uint64 totalPT = 0;

            if ((pos = instance.findProperty("ProviderElapsedTime")) !=
                PEG_NOT_FOUND)
            {
                p = (instance.getProperty(pos));
                v = p.getValue();
                if (v.getType() == CIMTYPE_DATETIME)
                {
                    v.get(totalProviderTime);
                    totalPT = totalProviderTime.toMicroSeconds();
                }
                else
                {
                    cerr << "Error Property Vlaue " << "ProviderElapsedTime" <<
                        endl;
                }
            }
            else
            {
                cerr << "Error Property " << "ProviderElapsedTime" << endl;
            }

            if (numberOfRequests != 0)
            {
                averageProviderTime = totalPT / numberOfRequests;
            }

            // Get the total Response size property "ResponseSize"
            Uint64 totalResponseSize = 0;
            Uint64 averageResponseSize = 0;

            if ((pos = instance.findProperty("ResponseSize")) != PEG_NOT_FOUND)
            {
                p = (instance.getProperty(pos));
                v = p.getValue();

                if (v.getType() == CIMTYPE_UINT64)
                {
                    v.get(totalResponseSize);
                }
                else
                {
                    cerr << "RequestSize is not of type CIMTYPE_SINT64" <<
                        endl ;
                }
            }
            else
            {
                cerr << "Could not find ResponseSize property" << endl;
            }

            if (numberOfRequests != 0)
            {
                averageResponseSize =  totalResponseSize / numberOfRequests;
            }

            //Get the total request size property "RequestSize"
            Uint64 totalRequestSize = 0;
            Uint64 averageRequestSize = 0;

            if ((pos = instance.findProperty("RequestSize")) != PEG_NOT_FOUND)
            {
                p = (instance.getProperty(pos));
                v = p.getValue();

                if (v.getType() == CIMTYPE_UINT64)
                {
                    v.get(totalRequestSize);
                }
                else
                {
                    cerr << "RequestSize is not of type CIMTYPE_SINT64" << endl;
                }
            }
            else
            {
                cerr << "Could not find RequestSize property" << endl;
            }

            if (numberOfRequests != 0)
            {
                averageRequestSize = totalRequestSize / numberOfRequests;
            }

            //if StatisticalData::copyGSD is FALSE this will only return 0's

            printf("%-25s"
                "%10"  PEGASUS_64BIT_CONVERSION_WIDTH "u"
                "%11" PEGASUS_64BIT_CONVERSION_WIDTH "d"
                "%11" PEGASUS_64BIT_CONVERSION_WIDTH "u"
                "%11" PEGASUS_64BIT_CONVERSION_WIDTH "u"
                "%11" PEGASUS_64BIT_CONVERSION_WIDTH "u\n",
                (const char *)statName.getCString(),
                numberOfRequests,
                averageCimomTime,
                averageProviderTime,
                averageRequestSize,
                averageResponseSize);
        }
    }
    catch (Exception& e)
    {
        cerr << argv[0] << "Exception : " << e.getMessage() << endl;
        exit(1);
    }

    return 0;
}


