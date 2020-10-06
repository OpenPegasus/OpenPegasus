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


// This file has the NISProvider-specific routines that will be called to get
// a validation of the CIM information vs. the current test system

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include "NISTestClient.h"

//------------------------------------------------------------------------------
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

//------------------------------------------------------------------------------
// FUNCTION: getUtilGetHostName
//
// REMARKS:
//
// PARAMETERS:  [OUT] systemName -> string that will contain the host name
//
// RETURN: TRUE if successful, FALSE otherwise
//------------------------------------------------------------------------------
static Boolean getUtilGetHostName(String& systemName)
{
     char    hostName[PEGASUS_MAXHOSTNAMELEN + 1];
     struct  hostent *he;

     if (gethostname(hostName, sizeof(hostName)) != 0)
     {
         return false;
     }
     hostName[sizeof(hostName)-1] = 0;

     // Now get the official hostname.  If this call fails then return
     // the value from gethostname().

     if (he=gethostbyname(hostName))
     {
         systemName.assign(he->h_name);
     }
     else
     {
         systemName.assign(hostName);
     }

     return true;
}

/**
    Retrieves the SystemName property value for the NIS Provider Test Client.
    Checks the specified value against the expected value and returns
    TRUE if the same, else FALSE.
*/
Boolean NISTestClient::goodSystemName(
    String & param,
    Boolean verbose)
{
   String host;
   getUtilGetHostName(host);

   if (verbose)
      cout<< "Checking " << param << " against " << host.getCString() << endl;
   return (String::equalNoCase(param, host));
}

/**
    Retrieves the SystemCreationClassName property value for the NIS Provider
    Test Client.  Checks the specified value against the expected value and
    returns TRUE if the same, else FALSE.
*/
Boolean NISTestClient::goodSystemCreationClassName(
    String & param,
    Boolean verbose)
{
   if(verbose)
      cout<< "Checking " << param << " against ";
      cout<< SYSTEM_CREATION_CLASS_NAME.getString() << endl;

   return (String::equalNoCase(param, SYSTEM_CREATION_CLASS_NAME.getString()));
}

/**
    Retrieves CreationClassName property value for the NIS Provider Test Client.
    Checks the specified value against the expected value and returns TRUE
    if the same, else FALSE.
*/
Boolean
NISTestClient::goodCreationClassName(String & param, Boolean verbose)
{
   if(verbose)
      cout<< "Checking " << param << " against ";
      cout<< CLASS_NAME.getString() <<endl;
   return (String::equalNoCase(param, CLASS_NAME.getString()));
}

/**
    Retrieves Name property value for the NIS Provider Test Client.
    Checks the specified value against the expected value and returns
    TRUE if the same, else FALSE
*/
Boolean
NISTestClient::goodName(String & param, Boolean verbose)
{
   FILE *fp;
   Boolean ok = false;
   int ps;
   char buffer[1000];
   String strValue;
   String strBuffer;

   if (verbose)
      cout<< "Checking Name property ... " <<endl;

   // Open file
   if ((fp = fopen(FILE_NAMESVRS.getCString(), "r")) == NULL)
       return ok;

    memset(buffer, 0, sizeof(buffer));
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        buffer[strlen(buffer) - 1] = 0;
        strBuffer.assign(buffer);

        ps = strBuffer.find(KEY_DOMAIN);
        if (ps < 0)
            continue;

        ps = strBuffer.find("=");
        if (ps < 0)
            continue;

        strValue = strBuffer.subString(ps + 1);
        if(String::equalNoCase(strValue, param) ||
           strValue.size() == 0)
           ok = true;

        break;
    }
    fclose(fp);
    if(verbose)
        cout << "Name = " << strValue << endl;
   return ok;
}

/**
    Retrieves the Caption property value for the NIS Provider Test Client.
    Checks the specified value against the expected value and returns
    TRUE if the same, else FALSE.
*/
Boolean
NISTestClient::goodCaption(String & param, Boolean verbose)
{
   if(verbose)
      cout<< "Checking " << param << " against " << CAPTION <<endl;

    return (String::equalNoCase(param, CAPTION));
}

/**
    Retrieves the Description property value for the NIS Provider Test Client.
    Checks the specified value against the expected value and returns TRUE
    if the same, else FALSE.
*/
Boolean
NISTestClient::goodDescription(String & param, Boolean verbose)
{
   if(verbose)
      cout<< "Checking " << param << " against " << DESCRIPTION <<endl;

   return (String::equalNoCase(param, DESCRIPTION));
}

/**
    Retrieves the ServerWaitFlag property value for the NIS Provider Test
    Client.  Checks the specified value against the expected value and returns
    TRUE if the same, else FALSE.
*/
Boolean NISTestClient::goodServerWaitFlag(
    Uint16 & param,
    Boolean verbose)
{
    FILE *fp;
    int ps;
    Boolean ok = false,
            found = false;
    char buffer[1000];
    Uint16 intValue = 0; // Set default to "Unknown"
    String strValue;
    String strBuffer;

    // Open file
    if ((fp = fopen(FILE_NAMESVRS.getCString(), "r")) == NULL)
        return ok;

    memset(buffer, 0, sizeof(buffer));
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        buffer[strlen(buffer) - 1] = 0;
        strBuffer.assign(buffer);

        found = true;
        ps = strBuffer.find(KEY_WAIT_SERVER);
        if (ps < 0)
            continue;

        ps = strBuffer.find("=");
        if (ps < 0)
            continue;

        strValue.assign(strBuffer.subString(ps + 1));
        if (String::equalNoCase(strValue, "TRUE"))
            intValue = 2;
        else if (String::equalNoCase(strValue, "FALSE"))
            intValue = 3;
        else
            intValue = 1;

        if (param == intValue)
            ok = true;

        break;
    }
    fclose(fp);

    if (verbose)
        cout<< "Checking " << param << " against " << intValue << endl;

    return found;
}

/**
    Retrieves the ServerType property value for the NIS Provider Test Client.
    Checks the specified value against the expected value and returns
    TRUE if the same, else FALSE.
*/
Boolean NISTestClient::goodServerType(
    Uint16 & param,
    Boolean verbose)
{
    FILE *fp;
    int ps, ind = 0;
    char buffer[1000];
    Boolean ok = false;
    String strValue;
    String strBuffer;
    Uint16 serverType = 2;  // Set default to "None";

    // Open file
    if ((fp = fopen(FILE_NAMESVRS.getCString(), "r")) == NULL)
        return ok;

    memset(buffer, 0, sizeof(buffer));
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        buffer[strlen(buffer) - 1] = 0;
        strBuffer.assign(buffer);

        ind = 0;
        ps = strBuffer.find(KEY_MASTER_SERVER);
        if (ps < 0)
        {
            ind = 1;
            ps = strBuffer.find(KEY_SLAVE_SERVER);
            if (ps < 0)
            {
                ind = 2;
                ps = strBuffer.find(KEY_PLUS_SERVER);
                if (ps < 0)
                    continue;
            }
        }

        ps = strBuffer.find("=");
        if (ps < 0)
            continue;

        ok = true;
        strValue.assign(strBuffer.subString(ps + 1));
        switch(ind)
        {
            case 0:
                if (String::equalNoCase(strValue, "1"))
                    serverType = 3;
                break;
            case 1:
                if (String::equalNoCase(strValue, "1"))
                    serverType = 4;
                break;
            case 2:
                if (serverType == 2)
                {
                    if (String::equalNoCase(strValue, "1"))
                        serverType = 0;
                    else
                        serverType = 1;
                }
                break;
            default:
                break;
        }
    }
    fclose(fp);

    if (param != serverType)
        return false;

    if (verbose)
        cout<< "Checking " << param << " against " << serverType << endl;
    return ok;
}
