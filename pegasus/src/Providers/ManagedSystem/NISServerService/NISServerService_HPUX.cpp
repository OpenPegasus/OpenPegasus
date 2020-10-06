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
//%////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
//Pegasus includes
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Exception.h>
#include "NTPProviderSecurity.h"
#include "NISServerServiceProvider.h"

// The follows includes are necessary to gethostbyaddr and gethostname
// functions
#include <ctype.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

//==============================================================================
//
// Class [NISServerService] methods
//
//==============================================================================

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
NISServerService::NISServerService()
{
    if (!getNISInfo())
        throw CIMObjectNotFoundException(
            "NISServerService can't create PG_NISServerService instance");
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
NISServerService::~NISServerService()
{
}

/*===========================================================================
  Methods HP-UX implementations
=============================================================================*/

//------------------------------------------------------------------------------
// FUNCTION: getHostName
//
// REMARKS: Resolves name servers
//
// PARAMETERS:  [IN] serverAddress -> string containing the IP address
//              [OUT] hostName -> string that will contain the server host name
//
// RETURN: TRUE if valid host IP, FALSE otherwise
//------------------------------------------------------------------------------
Boolean
NISServerService::getHostName(String serverAddress, String & hostName)
{
     Boolean ok = false;
     struct hostent *host;
     struct in_addr ia;

     hostName.clear();
     if ((ia.s_addr = inet_addr(serverAddress.getCString())) != INADDR_NONE)
     {
          host = gethostbyaddr((const char *)&ia,
                                sizeof(struct in_addr),
                                AF_INET);
          if(host != NULL) {
                hostName.assign(host->h_name);
                ok = true;
          }
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
NISServerService::getLocalHostName(String & hostName)
{
     char host[PEGASUS_MAXHOSTNAMELEN + 1];
     if (gethostname(host, sizeof(host)))
     {
          return false;
     }
     host[sizeof(host)-1] = 0;

     hostName.assign(host);
     return true;
}

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

     if ((he = gethostbyname(hostName)) != 0)
     {
         systemName.assign(he->h_name);
     }
     else
     {
         systemName.assign(hostName);
     }

     return true;
}

//------------------------------------------------------------------------------
// FUNCTION: getSystemName
//
// REMARKS:
//
// PARAMETERS:  [OUT] systemName -> string that will contain the system name
//
// RETURN: TRUE if successful , FALSE otherwise
//------------------------------------------------------------------------------
Boolean NISServerService::getSystemName(String& systemName)
{
    return getUtilGetHostName(systemName);
}

//------------------------------------------------------------------------------
// FUNCTION: AccessOk
//
// REMARKS: Status of context user
//
// PARAMETERS:    [IN]  context  -> pointer to Operation Context
//
// RETURN: TRUE, if user have privileges, otherwise FALSE
//------------------------------------------------------------------------------
Boolean
NISServerService::AccessOk(const OperationContext & context)
{
    NTPProviderSecurity sec(context); // Pointer defined into System.h file
    Boolean ok = sec.checkAccess(NIS_FILE_NAMESVRS,
                                  SEC_OPT_READ);
    return ok;
}

//------------------------------------------------------------------------------
// FUNCTION: getNISInfo
//
// REMARKS: This function return NIS informations
//
// PARAMETERS:
//
// RETURN: TRUE, if is ok.
//------------------------------------------------------------------------------
Boolean NISServerService::getNISInfo()
{
    FILE *fp;
    Boolean ok = false;
    int ps, ind = 0,
        count = 0;
    char buffer[1000];
    String strValue;
    String strBuffer;

    // Open name servers file
    if ((fp = fopen(NIS_FILE_NAMESVRS.getCString(), "r")) == NULL)
    {
        throw CIMOperationFailedException(
            "NISServerService can't open configuration file");
        return ok;
    }

    nisName.clear();
    nisServerWaitFlag = 0; // Set default to "Unknown"
    nisServerType = 2;     // Set default to "None"

    memset(buffer, 0, sizeof(buffer));
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        buffer[strlen(buffer) - 1] = 0;
        strBuffer.assign(buffer);

        ind = 0;
        ps = strBuffer.find(NIS_KEY_DOMAIN);  // Searching NIS DOMAIN
        if (ps < 0)
        {
            ind = 1;
            // verifying if it is MASTER server
            ps = strBuffer.find(NIS_KEY_MASTER_SERVER);
            if (ps < 0)
            {
                ind = 2;
                // verifying if it is SLAVE server
                ps = strBuffer.find(NIS_KEY_SLAVE_SERVER);
                if (ps < 0)
                {
                    // verifying if it is PLUS server
                    ps = strBuffer.find(NIS_KEY_PLUS_SERVER);
                    ind = 3;
                    if (ps < 0)
                    {
                        ind = 4;
                        // searching for wait server flag
                        ps = strBuffer.find(NIS_KEY_WAIT_SERVER);
                    }
                }
            }
        }

        if (ps < 0)
            continue;

        ps = strBuffer.find("=");
        if (ps < 0)
            continue;

        strValue.assign(strBuffer.subString(ps + 1));

        ok = true;
        switch(ind)
        {
            case 0:     // NIS domain name
                nisName.assign(strValue);
                break;
            case 1:         // NIS Master Server
                if (String::equalNoCase(strValue, "1"))
                {
                    nisServerType = 3;
                    count++;
                }
                break;
            case 2:     // NIS Slave Server
                if (String::equalNoCase(strValue, "1"))
                {
                    nisServerType = 4;
                    count++;
                }
                break;
            case 3:     // Server type
                if (!count)
                {
                    if (String::equalNoCase(strValue, "1"))
                        nisServerType = 0;
                    else
                        nisServerType = 1;
                }
                break;
            case 4:     // Wait NIS Server Flag
                if (String::equalNoCase(strValue, "TRUE"))
                    nisServerWaitFlag = 2;                    // Wait
                else if(String::equalNoCase(strValue, "FALSE"))
                    nisServerWaitFlag = 3;                    // No wait
                else
                    nisServerWaitFlag = 1;                    // Other
                break;
            default:
                ok = false;
                break;
        }
    }
    fclose(fp);
    return ok;
}


//------------------------------------------------------------------------------
// FUNCTION: getCreationClassName
//
// REMARKS: This property retrieve class name
//
// PARAMETERS: [OUT] strValue - class name
//
// RETURN: TRUE if is valid, FALSE otherwise
//------------------------------------------------------------------------------
Boolean NISServerService::getCreationClassName(String & strValue)
{
    strValue.assign(CLASS_NAME);
    return true;
}

//------------------------------------------------------------------------------
// FUNCTION: getName
//
// REMARKS: This property retrieve nis domain name
//
// PARAMETERS: [OUT] strValue - NIS domain name
//
// RETURN: TRUE if is valid, FALSE otherwise
//------------------------------------------------------------------------------
Boolean NISServerService::getName(String & strValue)
{
    strValue.assign(nisName);
    return true;
}

//------------------------------------------------------------------------------
// FUNCTION: getCaption
//
// REMARKS: This property retrieve caption
//
// PARAMETERS: [OUT] strValue - caption
//
// RETURN: TRUE if is valid, FALSE otherwise
//------------------------------------------------------------------------------
Boolean NISServerService::getCaption(String & strValue)
{
    strValue.assign(NIS_CAPTION);
    return true;
}

//------------------------------------------------------------------------------
// FUNCTION: getDescription
//
// REMARKS: This property retrieve description
//
// PARAMETERS: [OUT] strValue - description
//
// RETURN: TRUE if is valid, FALSE otherwise
//------------------------------------------------------------------------------
Boolean NISServerService::getDescription(String & strValue)
{
    strValue.assign(NIS_DESCRIPTION);
    return true;
}

//------------------------------------------------------------------------------
// FUNCTION: getServerWaitFlag
//
// REMARKS: This property retrieve wait flag value
//
// PARAMETERS: [OUT] strValue - return these values: 0 - Unknown,
//                                                   1 - Other
//                                                   2 - Wait
//                                                   3 - No wait.
//
// RETURN: TRUE if is valid, FALSE otherwise
//------------------------------------------------------------------------------
Boolean NISServerService::getServerWaitFlag(Uint16 & uintValue)
{
    uintValue = nisServerWaitFlag;
    return true;
}

//------------------------------------------------------------------------------
// FUNCTION: getServerType
//
// REMARKS: This property retrieve the follow values: 0-Unknown, 1-Other,
//          2-None, 3-NIS Master and 4-NIS Slave.
//
// PARAMETERS: [OUT] paramValue    -> return server type value
//
// RETURN: TRUE if is valid, FALSE otherwise
//------------------------------------------------------------------------------
Boolean NISServerService::getServerType(Uint16 & paramValue)
{
    paramValue = nisServerType;
    return true;
}
