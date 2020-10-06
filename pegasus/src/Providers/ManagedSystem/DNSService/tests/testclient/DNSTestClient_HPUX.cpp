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


// This file has the DNS-specific routines that will be called to get
// a validation of the CIM information vs. the current test system

// These are the HP-UX specific routines

// Note: some of the properties return information from the /etc/resolv.conf
//     file. We may have a mismatch when testing the information returned
//     by the provider against the information returned by the client
//     in case /etc/resolv.conf is being edited (provider sees one version
//     and client sees a different version).


#include <stdlib.h>
#include <ctype.h>
#include "DNSTestClient.h"


PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define DOMAINNAME 0
#define SEARCHLIST 1
#define ADDRESSES  2

// Verify if string is equal (Upper or Lower case)
Boolean DNSTestClient::IsEqual(char text[], const char txtcomp[])
{
    Boolean ok = true;
    int i;
    int max = strlen(text);
    char chin, chcmp;

    if (!max)
        return false;

    for (i=0; i<max; i++)
    {
        chin = toupper(text[i]);
        chcmp = toupper(txtcomp[i]);
        if (chin != chcmp)
        {
            ok = false;
            break;
        }
    }
    return ok;
}

// Verify if found string in array
Boolean DNSTestClient::FindInArray(Array<String> src, String text)
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

/*
   GoodCreationClassName method for the DNS Provider Test Client

   Checks the specified value against the expected value and
   returns TRUE if the same, else FALSE
*/
Boolean DNSTestClient::goodCreationClassName(
    const String &ccn,
    Boolean verbose)
{
    // This assumes the only right answer is PG_DNSAdminDomain

    if (verbose)
        cout << "Checking " << ccn << " against " << CLASS_NAME.getString() <<
            endl;
    return String::equalNoCase(ccn, CLASS_NAME.getString());
}

/*
   GoodName method for the DNS Provider Test Client

   Checks the specified value against the expected value and
   returns TRUE if the same, else FALSE

   Returns "domain" fields from /etc/resolv.conf file
*/
Boolean DNSTestClient::goodName(
    const String &name,
    Boolean verbose)
{
    FILE *arq;
    char buffer[150];
    String domain;

    domain.clear();

    if (verbose)
        cout<<"Checking " << name << " against DNS name"<<endl;

    // open resolv.conf file to read informations.
    if ((arq = fopen(RESOLV_CONF.getCString(), "r")) == NULL)
        return false;

    // Retrieve domain information from resolv.conf file
    while (!feof(arq))
    {
        memset(buffer, 0, sizeof(buffer));
        fscanf(arq, "%149s", buffer);

        if (!strlen(buffer))
            continue;

        if (IsEqual(buffer, "domain") ||
            IsEqual(buffer, "search"))
        {
            fscanf(arq, "%149s", buffer);
            domain.assign(buffer);
            break;
        }
    }
    fclose(arq);

    if (domain.size() == 0)
        return false;  // if can't get data to validate, fail

    if (verbose)
        cout<<" DNS name should be " << domain << endl;

    return String::equalNoCase(name, domain);
}

/* GoodCaption method for the DNS Provider Test Client

   Checks the specified value against the expected value
   and returns TRUE if the same, else FALSE
*/
Boolean DNSTestClient::goodCaption(
    const String &cap,
    Boolean verbose)
{
    // This assumes the only right answer is "DNS Admin Domain"

    if (verbose)
        cout << "Checking Caption " << cap << endl;
    return String::equalNoCase(cap, CAPTION);
}


/* GoodDescription method for the DNS Provider Test Client

   Checks the specified value against the expected value
   and returns TRUE if the same, else FALSE
*/
Boolean DNSTestClient::goodDescription(
    const String &des,
    Boolean verbose)
{
    // This assumes the only right answer is
    // "This is the PG_DNSAdminDomain object"

    if (verbose)
        cout << "Checking Description " << des << endl;
    return String::equalNoCase(des, DESCRIPTION);
}

/* GoodNAME_FORMAT method for the DNS Provider Test Client

   Checks the specified value against the expected value
   and returns TRUE if the same, else FALSE
*/
Boolean DNSTestClient::goodNameFormat(
    const String &nf,
    Boolean verbose)
{
    // This assumes the only right answer is "IP"

    if (verbose)
        cout << "Checking NameFormat " << nf << endl;
    return String::equalNoCase(nf, NAME_FORMAT);
}


/* GoodSearchList method for the DNS Provider Test Client

   Checks the specified values against the expected array
   values and returns TRUE if the same, else FALSE

   Returns "search" field from /etc/resolv.conf file
 */
Boolean DNSTestClient::goodSearchList(
    const Array<String> &src,
    Boolean verbose)
{
    Boolean ok = false;
    FILE *arq;
    int i, j, ind = 0, count = 0;
    char buffer[150];
    Array<String> srclist;

    // open resolv.conf file to retrieve informations
    if ((arq = fopen(RESOLV_CONF.getCString(), "r")) == NULL)
        return ok;

    // retrieve search list if exists
    while (!feof(arq))
    {
        memset(buffer, 0, sizeof(buffer));
        fscanf(arq, "%149s", buffer);

        if (!strlen(buffer))
            continue;

        // Verify if key is search
        if (IsEqual(buffer, "domain"))
        {
            ind = DOMAINNAME;
            continue;
        }
        else if (IsEqual(buffer, "nameserver"))
        {
            ind = ADDRESSES;
            continue;
        }
        else if (IsEqual(buffer, "search"))
        {
            ind = SEARCHLIST;
            continue;
        }
        else if (ind == SEARCHLIST)
        {
            if (verbose)
                cout << "Reading " << buffer << endl;

            if (!FindInArray(srclist, String(buffer)))
                srclist.append(String(buffer));
        }
        else
            continue;
    }
    fclose(arq);

    if (verbose)
    {
        for (i=0; i<srclist.size(); i++)
            cout << "Checking " << srclist[i] << " against SearchList " << endl;
    }

    // List all search list
    if (verbose)
    {
        cout << "Checking SearchList " << endl;
        for (i = 0; i < src.size(); i++)
            cout << src[i] << endl;
    }

    // Test size of srclist array
    if (srclist.size() == 0)
    {
        // Search list not exists
        ok = true;
    }
    else if (src.size() == srclist.size())
    {
        // Search list array have same size of source array
        if (verbose)
            cout <<"Searching search list " << endl;
        for (i=0; i<src.size(); i++)
        {
            for (j=0; j<srclist.size(); j++)
            {
                if (srclist[j] == src[i])
                    count++;
            }
        }
        if (count == src.size())
        {
            // Search list is ok
            ok = true;
        }
    }
    return ok;
}

/*
   GoodAddresses method for the DNS Provider Test Client

   Checks the specified values against the expected array values
   and returns TRUE if the same, else FALSE

   Returns "nameserver" field from /etc/resolv.conf file
*/
Boolean DNSTestClient::goodAddresses(
    const Array<String> &addr,
    Boolean verbose)
{
    FILE *arq;
    int i, j, ind = 0, count = 0;
    char buffer[150];
    Boolean ok = false;
    Array<String> addrlist;

    // open resolv.conf file to retrieve informations
    if ((arq = fopen(RESOLV_CONF.getCString(), "r")) == NULL)
        return ok;

    // retrieve addresses list
    while (!feof(arq))
    {
        memset(buffer, 0, sizeof(buffer));
        fscanf(arq, "%149s", buffer);

        if (!strlen(buffer))
            continue;

        // Verify if key is search
        if (IsEqual(buffer, "domain"))
        {
            ind = DOMAINNAME;
            continue;
        }
        else if (IsEqual(buffer, "search"))
        {
            ind = SEARCHLIST;
            continue;
        }
        else if (IsEqual(buffer, "nameserver"))
        {
            ind = ADDRESSES;
            continue;
        }
        else if (ind == ADDRESSES)
        {
            if (verbose)
                cout << "Reading " << buffer << endl;

            if (!FindInArray(addrlist, String(buffer)))
                addrlist.append(String(buffer));
        }
        else
            continue;
    }
    fclose(arq);

    if (verbose)
    {
        for (i=0; i<addrlist.size(); i++)
            cout << "Checking " << addrlist[i] << " against Addresses " << endl;
    }

    // List all search list
    if (verbose)
    {
        cout<<"Checking Addresses " << endl;
        for (i = 0; i < addr.size(); i++)
            cout << addr[i] << endl;
    }

    if (addrlist.size() == 0)
    {
        // Addresses list array not exists
        ok = true;
    }
    else if(addr.size() == addrlist.size())
    {
        if (verbose)
            cout <<"Searching address list " << endl;
        for (i=0; i<addr.size(); i++)
        {
            for (j=0; j<addrlist.size(); j++)
            {
                if (addrlist[j] == addr[i])
                    count++;
            }
        }
        if(count == addr.size())
        {
            // Addresses list array is ok
            ok = true;
        }
    }
    return ok;
}
