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

// This file has the NTPProvider-specific routines that will be called to get
// a validation of the CIM information vs. the current test system

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
//used by <explain it>
#include <stdlib.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "NTPTestClient.h"

//------------------------------------------------------------------------------
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

//------------------------------------------------------------------------------
// FUNCTION: getHostName
//
// REMARKS: Resolves name servers
//
// PARAMETERS:  [IN] serverAddress -> string containing the IP address
//                [OUT] hostName -> string that will contain the name server
//
// RETURN: TRUE if valid host IP, FALSE otherwise
//------------------------------------------------------------------------------
Boolean NTPTestClient::getHostName(
    String serverAddress,
    String & hostName)
{
    Boolean ok = false;
    String strValue;
    struct hostent *host;
    struct in_addr ia;

    hostName.clear();
    if ((ia.s_addr = inet_addr(serverAddress.getCString())) != INADDR_NONE)
    {
        host = gethostbyaddr((const char *)&ia,
                              sizeof(struct in_addr),
                               AF_INET);
        if (host != NULL)
        {
            hostName.assign(host->h_name);
            ok = true;
        }
   }
   return ok;
}

//------------------------------------------------------------------------------
// FUNCTION: getHostAddress
//
// REMARKS: Resolves address servers
//
// PARAMETERS:  [IN] serverName     -> string containing the name server
//                [OUT] serverAddress -> string that will contain the IP server
//
// RETURN: TRUE if valid host name, FALSE otherwise
//------------------------------------------------------------------------------
Boolean NTPTestClient::getHostAddress(
    String serverName,
    String & serverAddress)
{
    Boolean ok = false;
    String strValue;
    struct hostent *host;
    struct in_addr ia;

    serverAddress.clear();
    host = gethostbyname(serverName.getCString());
    if (host != NULL)
    {
        ia = *(struct in_addr *)(host->h_addr);
        serverAddress.assign(inet_ntoa(ia));
        ok = true;
    }
    return ok;
}

//------------------------------------------------------------------------------
// FUNCTION: isHostAddress
//
// REMARKS: Verify if host is address
//
// PARAMETERS:  [IN] host -> string containing the host name or address
//
// RETURN: TRUE if host address, FALSE otherwise
//------------------------------------------------------------------------------
Boolean NTPTestClient::isHostAddress(String host)
{
    int ps;
    String strValue;
    Boolean ok = false;

    ps = host.find(".");
    if (ps > 0)
    {
        strValue.assign(host.subString(0, ps - 1));
        ok = (atoi(strValue.getCString()) > 0);
    }
    return ok;
}

//------------------------------------------------------------------------------
// FUNCTION: getLocalHostName
//
// REMARKS: Retrieves the local host name
//
// PARAMETERS:  [OUT] hostName -> string that will contain the local host name
//
// RETURN: TRUE if local hostname is valid, FALSE otherwise
//------------------------------------------------------------------------------
Boolean
NTPTestClient::getLocalHostName(String & hostName)
{
    char host[PEGASUS_MAXHOSTNAMELEN + 1];
    if(gethostname(host, sizeof(host)))
    {
        return false;
    }
    host[sizeof(host)-1] = 0;

    hostName.assign(host);
    return true;
}

// Searches for a specified string into an given array
Boolean NTPTestClient::FindInArray(
    Array<String> src,
    String text)
{
    Boolean ok = false;
    int i;

    for (i=0; i<src.size(); i++)
    {
        if (src[i] == text)
        {
            ok = true;
            break;
        }
    }
    return ok;
}

//------------------------------------------------------------------------------
// FUNCTION: trim
//
// REMARKS: Remove spaces left and right from string variable
//
// PARAMETERS:  [IN] strText -> string that will contain the string value
//                              [OUT] strOut -> string cleaned
//
// RETURN:
//------------------------------------------------------------------------------
void NTPTestClient::trim(
    String strText,
    String & strOut)
{
    int i, ps = -1;

    if (strText.size() == 0)
    {
        strOut.clear();
        return;
    }

    strOut.assign(strText);
    // Clean left string
    for (i=0; i <= strOut.size(); i++)
    {
        if (strOut.subString(i, 1) != " ")
        {
            ps = i;
            break;
        }
    }
    if (ps >= 0)
        strOut.assign(strOut.subString(ps));

    // Clean rigth string
    ps = -1;
    for (i=strOut.size(); i >= 0; i--)
    {
        if (strOut.subString(i, 1) != " ")
        {
            ps = i;
            break;
        }
    }
    if (ps >= 0)
        strOut.assign(strOut.subString(0, ps));
}

//------------------------------------------------------------------------------
// FUNCTION: piece
//
// REMARKS: Return value of numPos position from string with separator
//
// PARAMETERS:  [IN] strText -> string that will contain the string value
//                              [IN] strSep  -> string that will contain the
//                                              string separator
//                              [IN] numPos  -> number position
//                              [OUT] strOut -> value returned
//
// RETURN: TRUE if is OK, otherwise false
//------------------------------------------------------------------------------
Boolean NTPTestClient::piece(
    String strText,
    String strSep,
    int numPos,
    String &strOut)
{
    String strAux;
    int count = 1;
    int ps1 = 0;

    if (strText.size() == 0 || numPos < 1)
        return false;

    strAux.assign(strText);
    strOut.clear();
    while (count <= numPos)
    {
        ps1 = strAux.find(strSep);
        if (count == numPos)
        {
            trim(strAux.subString(0, ps1), strOut);
            break;
        }
        if (ps1 < 0)
        {
            if(count == numPos && strAux.size() > 0)
                trim(strAux, strOut);
            else
                strOut.clear();
            break;
        }
        strAux.assign(strAux.subString(ps1+1));
        count++;
    }
    return (strOut.size() > 0);
}

// Standard verify method to property <do not remove>
/*
    goodCreationClassName property retrieve class name
    method for the NTP Provider Test Client
    Checks the specified value against the expected value and
    returns TRUE if the same, else FALSE
*/
Boolean NTPTestClient::goodCreationClassName(
    String & ccn,
    Boolean verbose)
{
    if (verbose)
        cout << "Checking " << ccn << " against " << CLASS_NAME.getString() <<
            endl;

    return String::equalNoCase(ccn, CLASS_NAME.getString());
}

/*
    goodName property retrieve primary server name
    method for the NTP Provider Test Client
    Checks the specified value against the expected value and
    returns TRUE if the same, else FALSE
*/
Boolean NTPTestClient::goodName(
    String & name,
    Boolean verbose)
{
    if (verbose)
        cout << "Checking Name " << name << " against " << NTP_NAME << endl;
    return String::equalNoCase(NTP_NAME, name);
}

/*
    goodCaption property retrieve product caption
    method for the NTP Provider Test Client
    Checks the specified value against the expected value and
    returns TRUE if the same, else FALSE
*/
Boolean NTPTestClient::goodCaption(
    String & cap,
    Boolean verbose)
{
    if (verbose)
        cout << "Checking Caption " << cap << endl;

    return String::equalNoCase(cap, CAPTION);
}

/*
    goodDescription property retrieve product description
    method for the NTP Provider Test Client
    Checks the specified value against the expected value and
    returns TRUE if the same, else FALSE
*/
Boolean NTPTestClient::goodDescription(
    String & des,
    Boolean verbose)
{
    if (verbose)
        cout << "Checking Description " << des << endl;

    return String::equalNoCase(des, DESCRIPTION);
}

/*
    goodServerAddress property retrieve server addresses array, if is NTP client
    method for the NTP Provider Test Client
    Checks the specified value against the expected value and
    returns TRUE if the same, else FALSE
*/
Boolean NTPTestClient::goodServerAddress(
    Array<String> & srvAddress,
    Boolean verbose)
{
    FILE *fp;
    int i, j, ps,
        count = 0;
    char buffer[5000];
    long lstPos = 0;
    Boolean ok = false,
            okRet = false;
    String strValue;
    String strKey;
    String strHost;
    String strBuffer;
    Array<String> srvAddr;

    if (verbose)
        cout << "Checking ServerAddress array ... " << endl;

    // Open NTP file configuration
    if ((fp = fopen(FILE_CONFIG.getCString(), "r")) == NULL)
        return ok;

    srvAddr.clear();
    memset(buffer, 0, sizeof(buffer));
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        int bufferLength = strlen(buffer);
        if (bufferLength > 0)
        {
            buffer[bufferLength - 1] = 0;
        }
        strBuffer.assign(buffer);

        ps = strBuffer.find(ROLE_CLIENT);
        okRet = true;

        if (ps == 0)
        {
            okRet = true;
            fseek(fp, lstPos, SEEK_SET);
            fscanf(fp, "%4999s", buffer);
            strBuffer.assign(buffer);
            ps = strBuffer.find(ROLE_CLIENT);
            if(ps < 0) {
                lstPos = ftell(fp);
                continue;
            }

            fscanf(fp, "%4999s", buffer);
            strHost.assign(buffer);

            ok = false;
            okRet = true;
            for(i=0; i < srvAddr.size(); i++)
            {
                if (String::equalNoCase(srvAddr[i], strHost))
                {
                    ok = true;
                    break;
                }
            }
            if (!ok)
                srvAddr.append(strHost);
        }
        lstPos = ftell(fp);
    }
    fclose(fp);

    if (okRet && srvAddr.size() != srvAddress.size())
        okRet = false;

    if (okRet)
    {
        count = 0;
        for(i=0; i < srvAddress.size(); i++)
        {
            for(j=0; j < srvAddr.size(); j++)
            {
                if(String::equalNoCase(srvAddress[i], srvAddr[j]))
                {
                    if (verbose)
                        cout << "ServerAddress[" << i << "]: " <<
                            srvAddress[i] << endl;
                    count++;
                }
            }
        }
        okRet = (count == srvAddr.size());
    }
    return okRet;
}
