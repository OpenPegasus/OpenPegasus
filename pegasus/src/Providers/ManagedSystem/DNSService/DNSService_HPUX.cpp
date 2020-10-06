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

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/ProviderException.h>
#include "NTPProviderSecurity.h"
#include "DNSServiceProvider.h"

// The following includes are necessary to gethostbyaddr and gethostname
// functions
#include <ctype.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

//used by gethostname function
#include <unistd.h>
#include <sys/param.h>
#include <sys/getaccess.h>
#include <pwd.h>
#include <stdlib.h>

//------------------------------------------------------------------------------
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static const String DNS_FILE_CONFIG("/etc/resolv.conf");
static const String DNS_NAME("named");

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
// FUNCTION:  DNSFileOk
//
// REMARKS:
//
// PARAMETERS:
//
// RETURN: true.
//------------------------------------------------------------------------------
Boolean DNSFileOk()
{
    return true;
}

//==============================================================================
//
// Class [NTPService] Methods
//
//==============================================================================

//------------------------------------------------------------------------------
// FUNCTION:   DNSService
//
// REMARKS:    Constructor for DNSService Class
//------------------------------------------------------------------------------
DNSService::DNSService(void)
{
#ifdef DNSPROVIDER_DEBUG
    cout << "DNSService::DNSService()" << endl;
#endif

    if(!getDNSInfo())
        throw CIMObjectNotFoundException("DNSService "
                  "can't create PG_DNSService instance");
}

//------------------------------------------------------------------------------
// FUNCTION:   ~DNSService
//
// REMARKS:    Destructor for DNSService Class
//------------------------------------------------------------------------------
DNSService::~DNSService(void)
{
}


//------------------------------------------------------------------------------
// FUNCTION:   getCaption
//
// REMARKS:    Function to retrieve a Caption
//
// PARAMETERS: [OUT] capt -> string that will contain the Caption
//
// RETURN:     true, hardcoded
//------------------------------------------------------------------------------
Boolean DNSService::getCaption(String & capt)
{
    capt.assign(DNS_CAPTION);
    return true;
}

//------------------------------------------------------------------------------
// FUNCTION:   getDescription
//
// REMARKS:    Function to retrieve local host name
//
// PARAMETERS: [OUT] desc -> string that will contain the Description
//
// RETURN:     true, hardcoded
//------------------------------------------------------------------------------
Boolean DNSService::getDescription(String & desc)
{
    desc.assign(DNS_DESCRIPTION);
    return true;
}

//------------------------------------------------------------------------------
// FUNCTION: getSystemName
//
// REMARKS: Retrieves the local host name
//
// PARAMETERS:  [OUT] systemName -> string that will contain the host name
//
// RETURN: TRUE if local hostname is valid, FALSE otherwise
//------------------------------------------------------------------------------
Boolean DNSService::getSystemName(String & systemName)
{
   return getUtilGetHostName(systemName);
}

// Verify if found string in array
//------------------------------------------------------------------------------
// FUNCTION:   FindInArray
//
// REMARKS:    Verify if found string in array
//
// PARAMETERS:
//
// RETURN:
//------------------------------------------------------------------------------
Boolean DNSService::FindInArray(Array<String> src, String text)
{
    Boolean ok = false;
    int i;

    for(i=0; i<src.size(); i++) {
        if(src[i] == text) {
            ok = true;
            break;
        }
    }
    return ok;
}

//------------------------------------------------------------------------------
// FUNCTION:   getDNSName
//
// REMARKS:    returns the Name Property
//
// PARAMETERS: [OUT]  name -> string that will receive the NTP Name property
//
// RETURN:     true, if there's a DNS Name
//------------------------------------------------------------------------------
Boolean DNSService::getDNSName(String & name)
{
#ifdef DNSPROVIDER_DEBUG
    cout << "DNSService::getDNSName()" << endl;
#endif

    if (dnsName.size() != 0)
    {
#ifdef DNSPROVIDER_DEBUG
        cout << "DNSService::getDNSName() - dnsName = '" << dnsName <<
             "'" << endl;
#endif
        name.assign(dnsName);
    }
    else
    {
        // initialize dnsName to root domain(" ")
        dnsName.assign(" ");

        char host[PEGASUS_MAXHOSTNAMELEN + 1];

        if (gethostname(host,sizeof(host)) == 0)
        {
                host[sizeof(host)-1] = 0;
                String hostName(host);
                Uint32 domainIndex =0;

                if( (domainIndex=hostName.find('.')) != PEG_NOT_FOUND)
                {
                    dnsName=hostName.subString(domainIndex+1,PEG_NOT_FOUND);
                }
        }
        name.assign(dnsName);
    }
    return true;
}

//------------------------------------------------------------------------------
// FUNCTION:   getSearchList
//
// REMARKS:    return the SearchList property
//
// PARAMETERS: [OUT]  srclst -> the array of search entries
//
// RETURN:
//------------------------------------------------------------------------------
Boolean DNSService::getSearchList(Array<String> & srclst)
{
    srclst.clear();
    for(int i=0; i < dnsSearchList.size(); i++)
        srclst.append(dnsSearchList[i]);
    return true;
}

//------------------------------------------------------------------------------
// FUNCTION:   getAddresses
//
// REMARKS:    return the Addresses property
//
// PARAMETERS: [OUT]  addrlst -> the array of addresses
//
// RETURN:
//------------------------------------------------------------------------------
Boolean DNSService::getAddresses(Array<String> & addrlst)
{
    addrlst.clear();
    for(int i=0; i < dnsAddresses.size(); i++)
        addrlst.append(dnsAddresses[i]);
    return true;
}

#define SEARCHLIST 1
#define ADDRESSES 2

// Read domain name, addresses e search list from /etc/resolv.conf
Boolean
DNSService::getDNSInfo()
{
#ifdef DNSPROVIDER_DEBUG
    cout << "DNSService::getDNSInfo()" << endl;
#endif

    FILE *fp;
    int count = 0, ind = 0;
    char buffer[512];
    Boolean ok = false;
    String strBuffer;
    // Open file DNS Configuration File
    if ((fp = fopen(DNS_FILE_CONFIG.getCString(), "r")) == NULL)
    {
        throw CIMOperationFailedException(
            "DNSService: can't open configuration file.");
    }

    // Check configuration file contents.
    while (!feof(fp))
    {
        memset(buffer, 0, sizeof(buffer));
        fscanf(fp, "%511s", buffer);
        strBuffer.assign(buffer);

        // Verify if keys exist
        if(String::equalNoCase(strBuffer, DNS_ROLE_DOMAIN) ||
           String::equalNoCase(strBuffer, DNS_ROLE_SEARCH) ||
           String::equalNoCase(strBuffer, DNS_ROLE_NAMESERVER) )
             count++;
        //A minimum of one key should exist
        if (count >= 1)
        {
            ok = true;
            break;
        }
    }
    if(!ok)
    {
        fclose(fp);
        throw CIMOperationFailedException
                  ("DNSService: configuration file is corrupt.");
    }

    // Clear all attributes
    rewind(fp);
    dnsName.clear();
    dnsSearchList.clear();
    dnsAddresses.clear();

    // Retrieve DNS informations from file
    while(!feof(fp)) {
        memset(buffer, 0, sizeof(buffer));
        fscanf(fp, "%511s", buffer);
        if(!strlen(buffer))
            continue;
        if( buffer[0] == '#' )
        {
            // If it is a comment line then read the whole line and continue
            fgets(buffer,512,fp);
            continue;
        }
        strBuffer.assign(buffer);

        // Verify if key is domain name
        if(String::equalNoCase(strBuffer, DNS_ROLE_DOMAIN)) {
            fscanf(fp, "%511s", buffer);
            dnsName.assign(buffer);
#ifdef DNSPROVIDER_DEBUG
    cout << "DNSService::getDNSInfo() - buffer = `" << buffer << "'" << endl;
    cout << "DNSService::getDNSInfo() - dnsName = `" <<
         dnsName.getCString() << endl;
#endif
        }
        else
        {
            // Verify if key is search list
            if(String::equalNoCase(strBuffer, DNS_ROLE_SEARCH)) {
                ind = SEARCHLIST;
                continue;
            }
            // Verify if key is address (DNS server)
            else if(String::equalNoCase(strBuffer, DNS_ROLE_NAMESERVER)) {
                ind = ADDRESSES;
                continue;
            }
            else
            {
                switch (ind)
                {
                    case SEARCHLIST:
                        // Make sure not to add multiple identical entries
                        if (!FindInArray(dnsSearchList, strBuffer))
                        {
                            dnsSearchList.append(strBuffer);
                            // if there's not already a Domain Name, use
                            // the first Search entry.
                            if (dnsName.size() == 0)
                                dnsName.assign(strBuffer);
                        }
                        break;

                    case ADDRESSES:
                        if(!FindInArray(dnsAddresses, String(buffer)))
                            dnsAddresses.append(strBuffer);
                        break;

                    default:
                        ok = false;
                        break;
                }
            }
        }
        if(!ok)
            break;
    }
    fclose(fp);
    return ok;
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
Boolean DNSService::AccessOk(const OperationContext & context)
{
    NTPProviderSecurity sec(context);
    Boolean ok = sec.checkAccess(DNS_FILE_CONFIG,
                                  SEC_OPT_READ);
    return ok;
}
