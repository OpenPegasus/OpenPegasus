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

#include <cstring>


// Using the general CIMOM TestClient as an example, developed an
// dnsinfo client that does an enumerateInstances of the
// PG_DNSService class and displays properties of interest.

// At this time, there is only one instance of PG_DNSService.

#include "DNSInfo.h"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define NAMESPACE CIMNamespaceName ("root/cimv2")
#define CLASSNAME CIMName ("PG_DNSService")

/**  Constructor for DNSInfo Client
  */

DNSInfo::DNSInfo()
{
}

DNSInfo::~DNSInfo()
{
}

/** ErrorExit - Print out the error message as an
    and get out.
    @param - Text for error message
    @return - None, Terminates the program
    @exception - This function terminates the program
*/
void DNSInfo::errorExit(const String& message)
{
    cerr << "dnsinfo error: " << message << endl;
    exit(1);
}

/** _usage method for osinfo - only accept one option
    -c for raw CIM formatting
*/
void DNSInfo::_usage()
{
    cerr << "Usage: osinfo [-c]" << endl;
    cerr << "Example:" << endl;
    cerr << "  dnsinfo " << endl;
    cerr << "  dnsinfo -c " << endl;
}

/**
   displayProperties method of the osinfo Test Client
  */
void DNSInfo::displayProperties()
{
   // interesting properties are stored off in class variables

   cout << "Domain Name System (DNS) Service Information" << endl;
   cout << endl;

   // expect to have values for the keys (even if Unknown)
   cout << "  System Name                : " << dnsSystemName << endl;
   cout << "  System Creation Class Name : " << dnsSystemCreationClassName <<
        endl;
   cout << "  Name                       : " << dnsName << endl;
   cout << "  Creation Class Name        : " << dnsCreationClassName << endl;
   cout << "  Caption                    : " << dnsCaption << endl;
   cout << "  Description                : " << dnsDescription << endl;

   for (int i = 0; i <dnsSearchList.size(); i++)
   {
      cout << "  Search List[" << i << "]             : " <<
           dnsSearchList[i] << endl;
   }

   for (int i = 0; i <dnsAddresses.size(); i++)
   {
      cout << "  Server Addresses[" << i << "]        : " <<
           dnsAddresses[i] << endl;
   }
}

/**
   gatherProperties method of the dnsinfo Test Client
  */
void DNSInfo::gatherProperties(CIMInstance &inst, Boolean cimFormat)
{
#ifdef DEBUG
   cout << "DNSInfo::gatherProperties()" << endl;
#endif
   // don't have a try here - want it to be caught by caller

   // loop through the properties
   for (Uint32 j=0; j < inst.getPropertyCount(); j++)
   {
      CIMName propertyName = inst.getProperty(j).getName();

      // only pull out those properties of interest
      if (propertyName.equal (CIMName ("SystemName")))
      {
         inst.getProperty(j).getValue().get(dnsSystemName);
      }  // end if SystemName

      if (propertyName.equal (CIMName ("SystemCreationClassName")))
      {
         inst.getProperty(j).getValue().get(dnsSystemCreationClassName);
      }  // end if SystemCreationClassName

      if (propertyName.equal (CIMName ("Name")))
      {
         inst.getProperty(j).getValue().get(dnsName);
      }  // end if Name

      if (propertyName.equal (CIMName ("CreationClassName")))
      {
         inst.getProperty(j).getValue().get(dnsCreationClassName);
      }  // end if CreationClassName

      if (propertyName.equal (CIMName ("Caption")))
      {
         inst.getProperty(j).getValue().get(dnsCaption);
      }  // end if Caption

      if (propertyName.equal (CIMName ("Description")))
      {
         inst.getProperty(j).getValue().get(dnsDescription);
      }  // end if Description

      if (propertyName.equal (CIMName ("SearchList")))
      {
         inst.getProperty(j).getValue().get(dnsSearchList);
      }  // end if SearchList

      if (propertyName.equal (CIMName ("Addresses")))
      {
         inst.getProperty(j).getValue().get(dnsAddresses);
      }  // end if ServerAddress
   }  // end of for looping through properties
}

/*
   getDNSInfo of the DNS provider.
*/
void DNSInfo::getDNSInfo(const int argc, const char** argv)
{

// ATTN-SLC-16-May-02-P1  enhance to take host & user info
//  Decided to keep local only for first release

    Boolean cimFormat = false;

    // before we even connect to CIMOM, make sure we're
    // syntactically valid

    if (argc > 2)
    {
       _usage();
       exit(1);
    }

    if (argc == 2)
    {
       // only support one option, -c for CIM formatting
       const char *opt = argv[1];

       if (strcmp(opt,"-c") == 0)
       {
          cimFormat = true;
       }
       else
       {
          _usage();
          exit(1);
       }
    }

    // need to first connect to the CIMOM

    try
    {
        // specify the timeout value for the connection (if inactive)
        // in milliseconds, thus setting to one minute
        CIMClient client;
        client.setTimeout(60 * 1000);
        client.connectLocal();

        Boolean deepInheritance = true;
        Boolean localOnly = true;
        Boolean includeQualifiers = false;
        Boolean includeClassOrigin = false;
        Uint32 numberInstances;

#ifdef DEBUG
        cout << "DNSInfo::getDNSInfo() - doing enumerateInstances . . ."
             << endl;
#endif

        Array<CIMInstance> cimNInstances =
           client.enumerateInstances(NAMESPACE, CLASSNAME,
                                         deepInheritance,
                         localOnly,  includeQualifiers,
                         includeClassOrigin );

#ifdef DEBUG
        cout << "DNSInfo::getDNSInfo() - enumerateInstances done" << endl;
#endif

        numberInstances = cimNInstances.size();

        // while we only have one instance (the running OS), we can take the
        // first instance.  When the OSProvider supports installed OSs as well,
        // will need to select the runningOS instance

        for (Uint32 i = 0; i < cimNInstances.size(); i++)
        {
           CIMObjectPath instanceRef = cimNInstances[i].getPath ();
           if ( !(instanceRef.getClassName().equal (CIMName (CLASSNAME))))
           {
              errorExit("EnumerateInstances failed");
           }

           // first gather the interesting properties
           gatherProperties(cimNInstances[i], cimFormat);

           // then display them
           displayProperties();

      }   // end for looping through instances

    }  // end try

    catch(Exception& e)
    {
      errorExit(e.getMessage());
    }

}

///////////////////////////////////////////////////////////////
//    MAIN
///////////////////////////////////////////////////////////////

int main(const int argc, const char** argv)
{
   DNSInfo dnsInfo;
   dnsInfo.getDNSInfo(argc, argv);
   return 0;
}

