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

// Using the general CIMOM TestClient as an example, developed an
// NTPProvider test client to exercise the intrinsic methods of
// the PG_NTPService class:
// testing EnumerateInstanceNames, EnumerateInstances
// and GetInstance.

#include "NTPTestClient.h"

// include the appropriate NTPProvider-specific file for checking results
#if defined (PEGASUS_PLATFORM_HPUX_ACC)
# include "NTPTestClient_HPUX.cpp"
#elif defined (PEGASUS_OS_VMS)
# include "NTPTestClient_Vms.cpp"
#else
# include "NTPTestClient_Stub.cpp"
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

/**  Constructor for NTP Test Client
  */

NTPTestClient::NTPTestClient(CIMClient &client)
{
}

/**  Destructor for NTP Test Client
  */
NTPTestClient::~NTPTestClient()
{
}

/** ErrorExit - Print out the error message as an
    and get out.
    @param - Text for error message
    @return - None, Terminates the program
    @exception - This function terminates the program
*/
void NTPTestClient::errorExit(const String& message)
{
    cerr << "Error: " << message << endl;
    cerr << "Re-run with verbose for details (NTPTestClient verbose)" <<endl;
    exit(1);
}

// testLog method used for messages to really stand out
// for example, Test Start and Test Passed messages
void NTPTestClient::testLog(const String& message)
{
    cout << "++++ " << message << " ++++" << endl;
}

/**
   _validateKeys method of the NTP provider Test Client
  */
void NTPTestClient::_validateKeys(
    CIMObjectPath &cimRef,
    Boolean verboseTest)
{
    // don't have a try here - want it to be caught by caller
    String keyVal;
    CIMName keyName;
    Array<CIMKeyBinding> keyBindings = cimRef.getKeyBindings();

    if (verboseTest)
        cout << "Retrieved " << keyBindings.size() << " keys" <<endl;

    for (Uint32 j = 0; j < keyBindings.size(); j++)
    {
        keyName = keyBindings[j].getName();
        keyVal.assign(keyBindings[j].getValue());
        if (verboseTest)
            cout << "checking key " << keyName.getString() << endl;

        if ((keyName.equal("CreationClassName")) &&
            !goodCreationClassName(keyVal, verboseTest)) {
            errorExit ("CreationClassName not PG_NTPService");
        }
        else if (keyName.equal("Name") &&
                 !goodName(keyVal, verboseTest)) {
            errorExit ("Name not correct");
        }
    }
}

/**
   _validateProperties method of the NTP provider Test Client
  */
void NTPTestClient::_validateProperties(
    CIMInstance &inst,
    Boolean verboseTest)
{
    if (verboseTest)
        cout << "Checking " <<inst.getPropertyCount()<<" properties"<<endl;

    // loop through the properties
    for (Uint32 j=0; j < inst.getPropertyCount(); j++)
    {
         CIMName propertyName = inst.getProperty(j).getName();

         if(verboseTest)
             cout << "Property name: " << propertyName.getString() <<endl;

         if (propertyName.equal("CreationClassName"))
         {
             String propertyValue;
             inst.getProperty(j).getValue().get(propertyValue);
             if (goodCreationClassName(propertyValue,
                     verboseTest) == false)
             {
                 errorExit ("'CreationClassName' is not PG_NTPService");
             }
         }  // end if CreationClassName
         else if (propertyName.equal("Name"))
         {
             String propertyValue;
             inst.getProperty(j).getValue().get(propertyValue);
             if (goodName(propertyValue, verboseTest) == false)
             {
                 errorExit ("'Name' not correct");
             }
         }  // end if Name

         else if (propertyName.equal("Caption"))
         {
             String propertyValue;
             inst.getProperty(j).getValue().get(propertyValue);
             if (goodCaption(propertyValue, verboseTest) == false)
             {
                 errorExit ("'Caption' not correct");
             }
         }  // end if Caption

         else if (propertyName.equal("Description"))
         {
             String propertyValue;
             inst.getProperty(j).getValue().get(propertyValue);
             if (goodDescription(propertyValue, verboseTest) == false)
             {
                 errorExit ("'Description' not correct");
             }
         }  // end if Description

         else if (propertyName.equal("ServerAddress"))
         {
             Array<String> propertyValue;
             inst.getProperty(j).getValue().get(propertyValue);
             if (goodServerAddress(propertyValue, verboseTest) == false)
             {
                 errorExit ("'ServerAddress' not correct");
             }
         }  // end if ServerAddress
    }

}


/*
   testEnumerateInstanceNames of the NTP provider.
*/
void NTPTestClient::testEnumerateInstanceNames(
    CIMClient &client,
    Boolean verboseTest)
{
    try
    {
        Uint32  numberInstances;

        testLog("NTP Provider Test Start EnumerateInstanceNames");

        Array<CIMObjectPath> cimReferences =
            client.enumerateInstanceNames(NAMESPACE, CLASS_NAME);

        numberInstances = cimReferences.size();
        if (verboseTest)
            cout << numberInstances << " instances of " <<
                CLASS_NAME.getString() <<endl;

        for (Uint32 i = 0; i < cimReferences.size(); i++)
        {
            CIMName className = cimReferences[i].getClassName();
            if (!(className.equal(CLASS_NAME.getString())))
            {
                errorExit("EnumInstanceNames failed - wrong class");
            }

            _validateKeys(cimReferences[i], verboseTest);

        }   // end for looping through instances

        testLog("NTP Provider Test EnumInstanceNames Passed");
    }
    catch(Exception& e)
    {
        errorExit(e.getMessage());
    }
}

/*
   testEnumerateInstances of the NTP provider.
*/
void NTPTestClient::testEnumerateInstances(
    CIMClient &client,
    Boolean verboseTest)
{
    try
    {
        Boolean deepInheritance = true;
        Boolean localOnly = true;
        Boolean includeQualifiers = false;
        Boolean includeClassOrigin = false;
        Uint32 numberInstances;

        testLog("NTP Provider Test EnumerateInstances");

        Array<CIMInstance> cimNInstances =
            client.enumerateInstances(NAMESPACE,
                                      CLASS_NAME,
                                      deepInheritance,
                                      localOnly,
                                      includeQualifiers,
                                      includeClassOrigin);

        numberInstances = cimNInstances.size();
        if (verboseTest)
            cout << numberInstances << " instances of PG_NTPService" <<endl;

        for (Uint32 i = 0; i < cimNInstances.size(); i++)
        {
            CIMObjectPath instanceRef = cimNInstances[i].getPath ();

            if (verboseTest)
                cout<<"Instance ClassName is " <<
                    instanceRef.getClassName().getString() << endl;

            if( !(instanceRef.getClassName().equal( CLASS_NAME.getString() ) ) )
            {
                errorExit("EnumInstances failed");
            }

            // now validate the properties
            _validateProperties(cimNInstances[i], verboseTest);
        }       // end for looping through instances

        testLog("NTP Provider Test EnumInstances Passed");
    }  // end try

    catch(Exception& e)
    {
        errorExit(e.getMessage());
    }
}

/*
   testGetInstance of the NTP provider.
*/
void NTPTestClient::testGetInstance(
    CIMClient &client,
    Boolean verboseTest)
{
    CIMObjectPath  getTestRef;    //  will need an instance for Get

    try
    {
        testLog("NTP Provider Test GetInstance");

        // first do an EnumerateInstanceNames - select one to play with
        // doesn't hurt to keep testing enumerate :-)

        Boolean localOnly = true;
        Boolean includeQualifiers = false;
        Boolean includeClassOrigin = false;

        Array<CIMObjectPath> cimReferences =
            client.enumerateInstanceNames(NAMESPACE, CLASS_NAME);

        Uint32 numberInstances = cimReferences.size();
        if (verboseTest)
            cout << numberInstances << " instances of PG_NTPService" <<endl;

        for (Uint32 i = 0; i < cimReferences.size(); i++)
        {
            CIMName className = cimReferences[i].getClassName();
            if (!(className.equal(CLASS_NAME.getString())))
            {
                errorExit("EnumInstanceNames failed - wrong class");
            }
            // add in some content checks on the keys returned

            if (verboseTest)
                cout << " Validate keys of PG_NTPService ... " <<endl;

            _validateKeys(cimReferences[i], verboseTest);

            // let's just take the first instance found
            getTestRef = cimReferences[i];

        }   // end for looping through instances

        if (verboseTest)
            cout<<"EnumerateInstanceNames for GetInstance completed"<<endl;

        // now call GetInstance with the appropriate references
        CIMInstance getTestInstance = client.getInstance(NAMESPACE,
                                                         getTestRef,
                                                         localOnly,
                                                         includeQualifiers,
                                                         includeClassOrigin);

        // now validate the properties returned
        _validateProperties(getTestInstance, verboseTest);

        testLog("NTP Provider Test Get Instance passed ");
    }  // end try

    catch(Exception& e)
    {
        errorExit(e.getMessage());
    }
}

///////////////////////////////////////////////////////////////
//    MAIN
///////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    Boolean enumInst = true;
    Boolean enumInstNames = true;
    Boolean getInst = true;
    Boolean verboseTest = false;

    // check if have a "verbose" on the command line
    if (argv[1] != 0)
    {
        const char *arg = argv[1];
        if ((strcmp(arg, "-verbose") == 0) ||
            (strcmp(arg, "verbose") == 0))
            verboseTest = true;
    }

    // need to first connect to the CIMOM

    if (verboseTest)
        cout << "Starting NTP Client test" << endl;

    try
    {
        if (verboseTest)
            cout << "Create client" << endl;
        // specify the timeout value for the connection (if inactive)
        // in milliseconds, thus setting to one minute
        CIMClient client;
        client.setTimeout(60 * 1000);

        if (verboseTest)
            cout << "Client created" << endl;

        cout << "NTPTestClient connecting to local host" << endl;
        client.connectLocal();    // Connect to local host
        cout << "NTPTestClient Connected" << endl;

        NTPTestClient testClient(client);
        if (enumInstNames)
        {
            testClient.testEnumerateInstanceNames(client, verboseTest);
        }
        if (enumInst)
        {
            testClient.testEnumerateInstances(client, verboseTest);
        }
        if (getInst)
        {
            testClient.testGetInstance(client, verboseTest);
        }
        cout << "NTPTestClient disconnecting from CIMOM " << endl;
        client.disconnect();
    }
    catch (Exception& e)
    {
        cout << "---- NTP Provider Test Failed " << e.getMessage() << endl;
    }
    return 0;
}
