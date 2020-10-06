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

// The nisinfo CIM client does an enumerateInstances of the
// PG_NISServerService class and displays properties of interest.

// At this time, there is only one instance of PG_NISServerService.

#include "NISInfo.h"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define NAMESPACE CIMNamespaceName ("root/cimv2")
#define CLASSNAME CIMName ("PG_NISServerService")

/**  Constructor
*/
NISInfo::NISInfo()
{
}

NISInfo::~NISInfo()
{
}

/** ErrorExit - Print out the error message and get out.
    @param      Text for error message
    @return     None, Terminates the program
    @exception  This function terminates the program
*/
void NISInfo::errorExit(const String& message)
{
    cerr << "nisinfo error: " << message << endl;
    exit(1);
}

/** _usage method for nisinfo
*/
void NISInfo::_usage()
{
  cerr << "Usage: nisinfo " << endl;
}

/**
   displayProperties method of the nisinfo Test Client
*/
void NISInfo::displayProperties()
{
   // interesting properties are stored off in class variables
   String strTit;
   Array<String> cTit;

   cout << "Network Information Service (NIS) Information" << endl;
   cout << endl;

   // expect to have values for the properties (even if Unknown) (do not remove)

   cout << " SystemCreationClassName: " << nisSystemCreationClassName << endl;
   cout << " SystemName             : " << nisSystemName << endl;
   cout << " CreationClassName      : " << nisCreationClassName << endl;
   cout << " Name                   : " << nisName << endl;
   cout << " Caption                : " << nisCaption << endl;
   cout << " Description            : " << nisDescription << endl;

   cTit.clear();
   cTit.append("Unknown");
   cTit.append("Other");
   cTit.append("None");
   cTit.append("NIS Master");
   cTit.append("NIS Slave");
   cout << " ServerType             : "
        << CIMValue(nisServerType).toString().getCString()
        << " (" << cTit[nisServerType].getCString() << ")" << endl;

   cTit.clear();
   cTit.append("Unknown");
   cTit.append("Other");
   cTit.append("Wait");
   cTit.append("No Wait");
   cout << " ServerWaitFlag         : "
        << CIMValue(nisServerWaitFlag).toString().getCString()
        << " (" << cTit[nisServerWaitFlag].getCString()  << ")" << endl;
}

/**
   gatherProperties method of the nisinfo Test Client
*/
void NISInfo::gatherProperties(CIMInstance &inst)
{
#ifdef DEBUG
   cout << "NISInfo::gatherProperties()" << endl;
#endif
   // don't have a try here - want it to be caught by caller

   // loop through the properties
   for (Uint32 j=0; j < inst.getPropertyCount(); j++)
   {
      CIMName propertyName = inst.getProperty(j).getName();

      // only pull out those properties of interest  (do not remove)

      if (propertyName.equal (CIMName ("SystemCreationClassName")))
      {
            inst.getProperty(j).getValue().get(nisSystemCreationClassName);
      }  // end if SystemCreationClassName

      if (propertyName.equal (CIMName ("SystemName")))
      {
            inst.getProperty(j).getValue().get(nisSystemName);
      }  // end if SystemName

      if (propertyName.equal (CIMName ("CreationClassName")))
      {
            inst.getProperty(j).getValue().get(nisCreationClassName);
      }  // end if CreationClassName

      if (propertyName.equal (CIMName ("Name")))
      {
            inst.getProperty(j).getValue().get(nisName);
      }  // end if Name

      if (propertyName.equal (CIMName ("Caption")))
      {
            inst.getProperty(j).getValue().get(nisCaption);
      }  // end if Caption

      if (propertyName.equal (CIMName ("Description")))
      {
            inst.getProperty(j).getValue().get(nisDescription);
      }  // end if Description

      if (propertyName.equal (CIMName ("ServerType")))
      {
            inst.getProperty(j).getValue().get(nisServerType);
      }  // end if ServerType

      if (propertyName.equal (CIMName ("ServerWaitFlag")))
      {
            inst.getProperty(j).getValue().get(nisServerWaitFlag);
      }  // end if ServerWaitFlag
   }  // end of for looping through properties
}

/*
   getNISInfo of the NIS provider.
*/
void NISInfo::getNISInfo(const int argc, const char** argv)
{
    String hostname;
    String user;
    String passwd;

    // before we even connect to CIMOM, make sure we're
    // syntactically valid

    if (argc > 1)
    {
       _usage();
       exit(1);
    }

    // need to first connect to the CIMOM

    try
    {
        // specify the timeout value for the connection (if inactive)
        // in milliseconds, thus setting to one minute
        CIMClient client;
        client.setTimeout(120 * 1000);

    client.connectLocal();

        Boolean deepInheritance = true;
        Boolean localOnly = true;
        Boolean includeQualifiers = false;
        Boolean includeClassOrigin = false;
        Uint32 numberInstances;

#ifdef DEBUG
        cout << "NISInfo::getNISInfo() - doing enumerateInstances . . ."
             << endl;
#endif

        Array<CIMInstance> cimNInstances =
           client.enumerateInstances(NAMESPACE, CLASSNAME,
                                     deepInheritance,
                                     localOnly,
                                     includeQualifiers,
                                     includeClassOrigin );

#ifdef DEBUG
        cout << "NISInfo::getNISInfo() - enumerateInstances done" << endl;
#endif

        numberInstances = cimNInstances.size();

        for (Uint32 i = 0; i < cimNInstances.size(); i++)
        {
           CIMObjectPath instanceRef = cimNInstances[i].getPath ();
           if ( !(instanceRef.getClassName().equal (CIMName (CLASSNAME))))
           {
              errorExit("EnumerateInstances failed");
           }

           // first gather the interesting properties
           gatherProperties(cimNInstances[i]);

           // then display them
           displayProperties();

      }   // end for looping through instances
    }
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
   NISInfo nisInfo;
   nisInfo.getNISInfo(argc, argv);
   return 0;
}

