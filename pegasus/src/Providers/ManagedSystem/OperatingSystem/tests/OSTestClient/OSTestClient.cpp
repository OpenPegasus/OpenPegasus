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
// OperatingSystem test client to exercise the intrinsic methods of
// the CIM_OperatingSystem class and PG_OperatingSystem subclass:
// testing EnumerateInstanceNames, EnumerateInstances
// and GetInstance.  InvokeMethod is not currently tested.

#include "OSTestClient.h"

// include the appropriate OS-specific file for checking results
#if defined (PEGASUS_PLATFORM_HPUX_ACC)
# include "OSTestClient_HPUX.cpp"
#elif defined (PEGASUS_OS_VMS)
#  include "OSTestClient_VMS.cpp"
#elif defined (PEGASUS_OS_SOLARIS)
#  include "OSTestClient_Solaris.cpp"
#elif defined (PEGASUS_OS_LINUX)
#  include "OSTestClient_Linux.cpp"
# else
#  include "OSTestClient_Stub.cpp"
#endif

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define NAMESPACE "root/cimv2"
#define CLASSNAME "PG_OperatingSystem"

/**  Constructor for OS Test Client
  */

OSTestClient::OSTestClient(CIMClient &client)
{
   // aborted attempt to optimize by having the
   // client stored off vs. passed to each routine
   // Thwarted by the lack of a copy constructor on
   // the CIMClient class
   //    _osclient = client;
}

OSTestClient::~OSTestClient(void)
{
}

/** ErrorExit - Print out the error message as an
    and get out.
    @param - Text for error message
    @return - None, Terminates the program
    @exception - This function terminates the program
*/
void OSTestClient::errorExit(const String& message)
{
    cerr << "Error: " << message << endl;
    cerr << "Re-run with verbose for details (OSTestClient verbose)" <<endl;
    exit(1);
}

// testLog method used for messages to really stand out
// for example, Test Start and Test Passed messages

void OSTestClient::testLog(const String& message)
{
    cout << "++++ " << message << " ++++" << endl;

}

/**
   _validateKeys method of the OS provider Test Client
  */
void OSTestClient::_validateKeys(
    CIMObjectPath &cimRef,
    Boolean verboseTest)
{
   // don't have a try here - want it to be caught by caller

   Array<CIMKeyBinding> keyBindings = cimRef.getKeyBindings();

   if (verboseTest)
      cout << "Retrieved " << keyBindings.size() << " keys" <<endl;

   for (Uint32 j = 0; j < keyBindings.size(); j++)
   {
      CIMName keyName = keyBindings[j].getName();
      if (verboseTest)
         cout << "checking key " << keyName.getString() << endl;
      if (((keyName.equal ("CSCreationClassName")) &&
          (goodCSCreationClassName(
               keyBindings[j].getValue(),
               verboseTest) == false)))
      {
         errorExit ("CSCreationClassName not CIM_UnitaryComputerSystem");
      }
      else if ((keyName.equal ("CSName")) &&
               (goodCSName(keyBindings[j].getValue(),
                       verboseTest) == false))
      {
         errorExit ("CSName not correct");
      }
      else if ((keyName.equal ("CreationClassName")) &&
          (goodCreationClassName(
            keyBindings[j].getValue(),
            verboseTest) == false))
      {
         errorExit ("CreationClassName not correct");
      }
      else if ((keyName.equal ("Name")) &&
                (goodName(
                  keyBindings[j].getValue(),
                  verboseTest) == false))
      {
         errorExit ("Name not correct");
      }
    }  // end for j looping through properties
}

/**
   _validateProperties method of the OS provider Test Client
  */
void OSTestClient::_validateProperties(
    CIMInstance &inst,
    Boolean verboseTest)
{
   // don't have a try here - want it to be caught by caller

   if (verboseTest)
      cout << "Checking " <<inst.getPropertyCount()<<" properties"<<endl;

   // loop through the properties
   for (Uint32 j=0; j < inst.getPropertyCount(); j++)
   {
      CIMName propertyName = inst.getProperty(j).getName();
      if (propertyName.equal ("CSCreationClassName"))
      {
         String propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodCSCreationClassName(propertyValue,
             verboseTest) == false)
         {
            errorExit ("CSCreationClassName not CIM_UnitaryComputerSystem");
         }
      } // end if CSCreationClassName

      else if (propertyName.equal ("CSName"))
      {
         String propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodCSName(propertyValue, verboseTest) == false)
         {
            errorExit ("CSName not correct");
         }
      }  // end if CSName

      else if (propertyName.equal ("CreationClassName"))
      {
         String propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodCreationClassName(propertyValue,
                                   verboseTest) == false)
         {
            errorExit ("CreationClassName not CIM_OperatingSystem");
         }
      }  // end if CreationClassName
      else if (propertyName.equal ("Name"))
      {
         String propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodName(propertyValue, verboseTest) == false)
         {
            errorExit ("Name not correct");
         }
      }  // end if Name

      else if (propertyName.equal ("Caption"))
      {
         String propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodCaption(propertyValue, verboseTest) == false)
         {
            errorExit ("Caption not correct");
         }
      }   // end if Caption

      else if (propertyName.equal ("Description"))
      {
         String propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodDescription(propertyValue, verboseTest) == false)
         {
            errorExit ("Description not correct");
         }
      }   // end if Description

      else if (propertyName.equal ("InstallDate"))
      {
         CIMDateTime idate;
         inst.getProperty(j).getValue().get(idate);
         if (goodInstallDate(idate, verboseTest) == false)
         {
            errorExit ("InstallDate not correct");
         }
      }   //  end if InstallDate

      else if (propertyName.equal ("Status"))
      {
         String propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodStatus(propertyValue, verboseTest) == false)
         {
            errorExit ("Status not correct");
         }
      }   // end if Status

      else if (propertyName.equal ("OSType"))
      {
         Uint16 ostype;
         inst.getProperty(j).getValue().get(ostype);
         if (goodOSType(ostype, verboseTest) == false)
         {
            errorExit ("OSType not correct");
         }
      }   // end if OSType

      else if (propertyName.equal ("OtherTypeDescription"))
      {
         String propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodOtherTypeDescription(propertyValue,
                                      verboseTest) == false)
         {
            errorExit ("OtherTypeDescription not correct");
         }
      }   // end if OtherTypeDescription

      else if (propertyName.equal ("Version"))
      {
         String propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodVersion(propertyValue, verboseTest) == false)
         {
            errorExit ("Version not correct");
         }
      }   // end if Version

      else if (propertyName.equal ("LastBootUpTime"))
      {
         CIMDateTime bdate;
         inst.getProperty(j).getValue().get(bdate);
         if (goodLastBootUpTime(bdate, verboseTest) == false)
         {
            errorExit ("LastBootUpTime not correct");
         }
      }   // end if LastBootUpTime

      else if (propertyName.equal ("LocalDateTime"))
      {
         CIMDateTime ldate;
         inst.getProperty(j).getValue().get(ldate);
         if (goodLocalDateTime(ldate, verboseTest) == false)
         {
            errorExit ("LocalDateTime not correct");
         }
      }   // end if LocalDateTime

      else if (propertyName.equal ("CurrentTimeZone"))
      {
         Sint16 tz;
         inst.getProperty(j).getValue().get(tz);
         if (goodCurrentTimeZone(tz, verboseTest) == false)
         {
            errorExit ("CurrentTimeZone not correct");
         }
      }   // end if CurrentTimeZone

      else if (propertyName.equal ("NumberOfLicensedUsers"))
      {
         Uint32 numlusers;
         inst.getProperty(j).getValue().get(numlusers);
         if (goodNumberOfLicensedUsers(numlusers,
                                       verboseTest) == false)
         {
            errorExit ("NumberOfLicensedUsers not correct");
         }
      }   // end if numberOfLicensedUsers

      else if (propertyName.equal ("NumberOfUsers"))
      {
         Uint32 numUsers;
         inst.getProperty(j).getValue().get(numUsers);
         if (goodNumberOfUsers(numUsers,
                               verboseTest) == false)
         {
            errorExit ("NumberOfUsers not correct");
         }
      }   // end if NumberOfUsers

      else if (propertyName.equal ("NumberOfProcesses"))
      {
         Uint32 numProcs;
         inst.getProperty(j).getValue().get(numProcs);
         if (goodNumberOfProcesses(numProcs,
                               verboseTest) == false)
         {
            errorExit ("NumberOfProcesses not correct");
         }
      }   // end if NumberOfProcesses

      else if (propertyName.equal ("MaxNumberOfProcesses"))
      {
         Uint32 maxProcs;
         inst.getProperty(j).getValue().get(maxProcs);
         if (goodMaxNumberOfProcesses(maxProcs,
                               verboseTest) == false)
         {
            errorExit ("MaxNumberOfProcesses not correct");
         }
      }   // end if MaxNumberOfProcesses

      else if (propertyName.equal ("TotalSwapSpaceSize"))
      {
         Uint64 totalSwap;
         inst.getProperty(j).getValue().get(totalSwap);
         if (goodTotalSwapSpaceSize(totalSwap,
                                    verboseTest) == false)
         {
            errorExit ("TotalSwapSpaceSize not correct");
         }
      }   // end if TotalSwapSpaceSize

      else if (propertyName.equal ("TotalVirtualMemorySize"))
      {
         Uint64 totalVmem;
         inst.getProperty(j).getValue().get(totalVmem);
         if (goodTotalVirtualMemorySize(totalVmem,
                                    verboseTest) == false)
         {
            errorExit ("TotalVirtualMemorySize not correct");
         }
      }   // end if TotalVirtualMemorySize

      else if (propertyName.equal ("FreeVirtualMemory"))
      {
         Uint64 freeVmem;
         inst.getProperty(j).getValue().get(freeVmem);
         if (goodFreeVirtualMemory(freeVmem,
                                   verboseTest) == false)
         {
            errorExit ("FreeVirtualMemory not correct");
         }
      }   // end if FreeVirtualMemory

      else if (propertyName.equal ("FreePhysicalMemory"))
      {
         Uint64 freePmem;
         inst.getProperty(j).getValue().get(freePmem);
         if (goodFreePhysicalMemory(freePmem,
                                   verboseTest) == false)
         {
            errorExit ("FreePhysicalMemory not correct");
         }
      }   // end if FreePhysicalMemory

      else if (propertyName.equal ("TotalVisibleMemorySize"))
      {
         Uint64 totalVisMem;
         inst.getProperty(j).getValue().get(totalVisMem);
         if (goodTotalVisibleMemorySize(totalVisMem,
                                        verboseTest) == false)
         {
            errorExit ("TotalVisibleMemorySize not correct");
         }
      }   // end if TotalVisibleMemorySize

      else if (propertyName.equal ("SizeStoredInPagingFiles"))
      {
         Uint64 propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodSizeStoredInPagingFiles(propertyValue,
                                         verboseTest) == false)
         {
            errorExit ("SizeStoredInPagingFiles not correct");
         }
      }   // end if SizeStoredInPagingFiles

      else if (propertyName.equal ("FreeSpaceInPagingFiles"))
      {
         Uint64 propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodFreeSpaceInPagingFiles(propertyValue,
                                        verboseTest) == false)
         {
            errorExit ("FreeSpaceInPagingFiles not correct");
         }
      }   // end if FreeSpaceInPagingFiles

      else if (propertyName.equal ("MaxProcessMemorySize"))
      {
         Uint64 propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodMaxProcessMemorySize(propertyValue,
                                        verboseTest) == false)
         {
            errorExit ("MaxProcessMemorySize not correct");
         }
      }   // end if MaxProcessMemorySize

      else if (propertyName.equal ("Distributed"))
      {
         Boolean propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodDistributed(propertyValue,
                             verboseTest) == false)
         {
            errorExit ("Distributed not correct");
         }
      }   // end if Distributed

      else if (propertyName.equal ("MaxProcessesPerUser"))
      {
         Uint32 propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodMaxProcessesPerUser(propertyValue,
                                     verboseTest) == false)
         {
            errorExit ("MaxProcessesPerUser not correct");
         }
      }   // end if MaxProcessesPerUser

      else if (propertyName.equal ("OperatingSystemCapability"))
      {
         String propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodOSCapability(propertyValue,
                              verboseTest) == false)
         {
            errorExit ("OSCapability not correct");
         }
      }   // end if OSCapability

      else if (propertyName.equal ("SystemUpTime"))
      {
         Uint64 propertyValue;
         inst.getProperty(j).getValue().get(propertyValue);
         if (goodSystemUpTime(propertyValue,
                              verboseTest) == false)
         {
            errorExit ("SystemUpTime not correct");
         }
      }   // end if SystemUptime

   }  // end of for looping through properties
}

/*
   testEnumerateInstanceNames of the OS provider.
*/
void OSTestClient::testEnumerateInstanceNames(
    CIMClient &client,
    Boolean verboseTest)
{
    try
    {
        Uint32  numberInstances;
        String  propertyName;

        testLog("OS Provider Test Start EnumerateInstanceNames");

        Array<CIMObjectPath> cimReferences =
            client.enumerateInstanceNames(NAMESPACE, CLASSNAME);

        numberInstances = cimReferences.size();
        if (verboseTest)
            cout << numberInstances << " instances of PG_OperatingSystem" <<
                endl;

        for (Uint32 i = 0; i < cimReferences.size(); i++)
        {
            CIMName className = cimReferences[i].getClassName();
            if (!className.equal (CLASSNAME))
            {
                errorExit("EnumInstanceNames failed - wrong class");
            }

            _validateKeys(cimReferences[i], verboseTest);

        }   // end for looping through instances

        testLog("OS Provider Test EnumInstanceNames Passed");
    }
    catch (Exception& e)
    {
        errorExit(e.getMessage());
    }
}

/*
   testEnumerateInstances of the OS provider.
*/
void OSTestClient::testEnumerateInstances(
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

        testLog("OS Provider Test EnumerateInstances");

        Array<CIMInstance> cimNInstances =
            client.enumerateInstances(
                NAMESPACE,
                CLASSNAME,
                deepInheritance,
                localOnly,
                includeQualifiers,
                includeClassOrigin);

        numberInstances = cimNInstances.size();
        if (verboseTest)
            cout << numberInstances << " instances of PG_OperatingSystem" <<
                endl;
        for (Uint32 i = 0; i < cimNInstances.size(); i++)
        {
            CIMObjectPath instanceRef = cimNInstances[i].getPath();
            //String instanceRef =
            //    cimNInstances[i].getInstanceName().toString();
            if (verboseTest)
                cout << "Instance ClassName is " <<
                    instanceRef.getClassName().getString() << endl;
            if (!instanceRef.getClassName().equal (CLASSNAME))
            {
                errorExit("EnumInstances failed");
            }

            // now validate the properties
            _validateProperties(cimNInstances[i], verboseTest);
        }   // end for looping through instances

        testLog("OS Provider Test EnumInstances Passed");
    }
    catch (Exception& e)
    {
        errorExit(e.getMessage());
    }
}

/*
   testGetInstance of the OS provider.
*/
void OSTestClient::testGetInstance(
    CIMClient &client,
    Boolean verboseTest)
{
    CIMObjectPath  getTestRef;    //  will need an instance for Get

    try
    {
        testLog("OS Provider Test GetInstance");

        // first do an EnumerateInstanceNames - select one to play with
        // doesn't hurt to keep testing enumerate :-)

        Array<CIMObjectPath> cimReferences =
            client.enumerateInstanceNames(NAMESPACE, CLASSNAME);

        Uint32 numberInstances = cimReferences.size();
        if (verboseTest)
            cout << numberInstances << " instances of PG_OperatingSystem" <<
                endl;

        for (Uint32 i = 0; i < cimReferences.size(); i++)
        {
            CIMName className = cimReferences[i].getClassName();
            if (!className.equal (CLASSNAME))
            {
               errorExit("EnumInstanceNames failed - wrong class");
            }
            // add in some content checks on the keys returned

            _validateKeys(cimReferences[i], verboseTest);

            // let's just take the first instance found
            getTestRef = cimReferences[i];

        }   // end for looping through instances

        if (verboseTest)
            cout << "EnumerateInstanceNames for Get Instance completed" << endl;

        // now call GetInstance with the appropriate references
        CIMInstance getTestInstance = client.getInstance(NAMESPACE, getTestRef);

        // now validate the properties returned
        _validateProperties(getTestInstance, verboseTest);

        testLog("OS Provider Test Get Instance passed ");
    }
    catch(Exception& e)
    {
        errorExit(e.getMessage());
    }
}

///////////////////////////////////////////////////////////////
//    MAIN
///////////////////////////////////////////////////////////////

int main(int, char** argv)
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
    // use null string for user and password, port 5988

    if (verboseTest)
        cout << "Starting OS Client test" << endl;

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

        cout << "OSTestClient connecting via connectLocal() " << endl;
        client.connectLocal ();
        cout << "OSTestClient Connected" << endl;

        OSTestClient testClient(client);
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
        cout << "OSTestClient disconnecting from CIMOM " << endl;
        client.disconnect();
    }
    catch(Exception& e)
    {
        cout << "---- OS Provider Test Failed " << e.getMessage() << endl;
    }
    return 0;
}

