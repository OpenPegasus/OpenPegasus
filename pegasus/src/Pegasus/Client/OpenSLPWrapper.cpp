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

#include "OpenSLPWrapper.h"
#include <ctype.h>

PEGASUS_NAMESPACE_BEGIN

// preprocessAttrList basically strips and folds the whitespace in the supplied
// list. It overwrites the existing list with its output as it goes.
//
// (The output will always be equal or shorter to the original in length).
//
// The input string is of the form:
//      [WS]Tag[WS] and
//      [WS]([WS]Tag[WS]=[WS]Attr[EWS]Val[WS],[WS]val2[WS])[WS]
// The input string consists of a 1 or more instances of the above strings
// in a comma separated list.
// [WS] = WhiteSpace and is deleted
// [EWS] = Embedded WhiteSpace and will be folded to a single space.

static void preprocessAttrList(char* list)
{
    bool eatWhiteSpace = false;
    char* ptr = list;
    char* wptr = list;
    char last;

    while (*ptr != '\0')
    {
        // Outer switch case deals with everything outsde of brackets ( )
        switch(*ptr)
        {
        // We delete all whitespace outside of ( )
        case ' ':
        case '\t':
            last = ' ';
            break;

        case '(':
            // Start of a ( ) section
            last = '(';
            *wptr++ = *ptr;
            // Process everything until closing )
            while (*++ptr != '\0' && *ptr != ')')
            {
                switch(*ptr)
                {
                case ' ':
                case '\t':
                    // if we're deleting WS
                    if (eatWhiteSpace == true)
                    {
                        last = ' ';
                        break;
                    }
                    // if we're folding WS
                    if (last == ' ')
                    {
                        break;
                    }
                    last = ' ';
                    *wptr++ = ' ';
                    break;

                case ',':
                case '=':
                    // delete any preceding WS.
                    // It will have been folded to a
                    // single space
                    if (last == ' ')
                    {
                        wptr--;
                    }
                    *wptr++ = *ptr;
                    last = *ptr;
                    // flag to delete any following WS
                    eatWhiteSpace = true;
                    break;

                default:
                    // tag or attribute character
                    // flag to fold embedded WS
                    eatWhiteSpace = false;
                    last = *ptr;
                    *wptr++ = *ptr;
                }
            }

            // delete any WS before the closing )
            if (*ptr == ')' && last == ' ')
            {
                wptr--;
            }

            // we're outside the ( ), so flag to delete WS
            eatWhiteSpace = true;
            *wptr++ = *ptr;
            break;

        default:
            // Not WS, not inside ( ), so just copy it
            *wptr++ = *ptr;
        }
        ptr++;
    }
    // Make sure "new" string is properly terminated.
    *wptr = '\0';
}

// Assumes a list that has already been whitespace stripped/folded
// Overwrites input string with output.

static void preprocessAttrValList(char* list)
{
    char* wptr = list;
    char* ptr = list;
    char last = '\0';
    bool opaque = false;

    while (*ptr != '\0')
    {
        if (opaque == true)
        {
            *wptr++ = *ptr++;
            continue;
        }
#ifdef STRIP_QUOTES
        if (*ptr == '"')
        {
            ptr++;
            continue;
        }
#endif

        // Escape character or opaque sequence
        if (*ptr == '\\')
        {
            if (*(ptr+1) != '\0' && *(ptr+2) != '\0')
            {
                if (*(ptr+1) == 'F' && *(ptr+2) == 'F')
                {
                    opaque = true;
                    *wptr++ = *ptr++;
                }
                else
                {
                    // Allow any character to be escaped
                    // rfc2608 says to only escape
                    // reserved characters
                    //
                    Uint8 n = *++ptr;
                    if (isdigit(*ptr))
                        n = (*ptr - '0');
                    else if (isupper(*ptr))
                        n = (*ptr - 'A' + 10);
                    else
                        n = (*ptr - 'a' + 10);
                    *wptr = ((n & 0xf) << 4);

                    // Do second nibble
                    n = *++ptr;
                    if (isdigit(*ptr))
                        n = (*ptr - '0');
                    else if (isupper(*ptr))
                        n = (*ptr - 'A' + 10);
                    else
                        n = (*ptr - 'a' + 10);
                    *wptr += (n & 0xf);
                    wptr++;
                    ptr++;
                    continue;
                }
            }
        }
        // nothing special, just copy it
        *wptr++ = *ptr++;
    }
    *wptr = '\0';
}

static SLPBoolean wbemSrvUrlCallback(
    SLPHandle hslp,
    const char* srvurl,
    unsigned short lifetime,
    SLPError errcode,
    void* cookie)
{
    if (errcode == SLP_OK)
    {
        Array<String> *url_cookie = static_cast<Array<String> *>(cookie);
        url_cookie->append(srvurl);
    }

    return SLP_TRUE;
}

static SLPBoolean wbemAttrCallback(
    SLPHandle hslp,
    const char* attrlist,
    SLPError errcode,
    void* cookie)
{
    if (errcode == SLP_OK)
    {
        Array<Attribute> *attr_cookie = static_cast<Array<Attribute> *>(cookie);

        // Allocate memory to hold working copy of list
        char *list = new char[strlen(attrlist) + 1];

        if (list == (char *)NULL)
        {
            // Ignore out of memory, just go.
            return SLP_TRUE;
        }

        strcpy (list, attrlist);

        // Remove/fold whitespace
        preprocessAttrList(list);

        char *end = list + strlen(list);
        char *ptr = list;
        char *nptr;

        while (ptr <= end)
        {
            if (*ptr == '(')
            {
                ptr++;
                nptr = ptr;
                while (nptr <= end && *nptr != '=')
                {
                    nptr++;
                }
                *nptr = '\0';
                preprocessAttrValList(ptr);
                Attribute newAttr(ptr);
                ptr = nptr+1;
                while (nptr <= end)
                {
                    if (*nptr == ',' || *nptr == ')')
                    {
                        bool isBracket = false;
                        if (*nptr == ')')
                        {
                            isBracket = true;
                        }
                        *nptr = '\0';
                        preprocessAttrValList(ptr);
                        newAttr.addValue(ptr);
                        ptr = nptr + 1;
                        if (isBracket)
                        {
                            break;
                        }
                    }
                    else
                    {
                        nptr++;
                    }
                }
                attr_cookie->append(newAttr);
            }

            if (*ptr == ',')
            {
                ptr++;
                continue;
            }

            nptr = ptr;
            while (nptr <= end && *nptr != ',')
            {
                nptr++;
            }
            *nptr = '\0';
            attr_cookie->append(Attribute(ptr));
            ptr = nptr+1;
            continue;
        }
        delete [] list;
    }
    return SLP_TRUE;
}

CIMServerDiscoveryRep::CIMServerDiscoveryRep()
{
}

CIMServerDiscoveryRep::~CIMServerDiscoveryRep()
{
}

Array<CIMServerDescription> CIMServerDiscoveryRep::lookup(
    const Array<Attribute>& criteria,
    const SLPClientOptions* options)
{
    SLPError result;
    SLPHandle hslp;
    Array<CIMServerDescription> connections;

    // SLPOpen()
    // @param1 - language - NULL is the default locale
    // @param2 - async - FALSE is synchronous slp handle
    // @param3 - handle - pointer to slp handle
    if (SLPOpen(NULL, SLP_FALSE, &hslp) == SLP_OK)
    {
        Attribute attrServiceId;
        for (Uint32 idx=0; idx<criteria.size(); idx++)
        {
            if (criteria[idx].getTag() == PEG_WBEM_SLP_SERVICE_ID)
            {
                attrServiceId = criteria[idx];
            }
        }

        String serviceType(PEG_WBEM_SLP_TYPE);
        Array <String> serviceAttrList = attrServiceId.getValues();

        if (serviceAttrList.size() != 0 && serviceAttrList[0] != String::EMPTY)
        {
            serviceType = serviceAttrList[0];
        }

        // SLPFindSrvs()
        // @param1 - handle - slp handle
        // @param2 - service type - wbem
        // @param3 - scope list - NULL is all localhost can query
        // @param4 - filter - NULL is all that match type
        // @param5 - pointer to custom data to use in callback
        Array<String> urls;
        result = SLPFindSrvs(
            hslp,
            (const char *)serviceType.getCString(),
            NULL,
            NULL,
            wbemSrvUrlCallback,
            (void *)&urls);

        if (result == SLP_OK)
        {
            for (Uint32 i=0; i<urls.size(); i++)
            {
                CIMServerDescription connection(urls[i]);

                Array<Attribute> attributes;

                // SLPFindAttrs()
                // @param1 - handle - slp handle
                // @param2 - service url or type
                // @param3 - scope list - NULL is all localhost can query
                // @param4 - attribute list - NULL is all attributes
                // @param5 - pointer to custom data to use in callback
                result = SLPFindAttrs(
                    hslp,
                    (const char *)urls[i].getCString(),
                    NULL,
                    NULL,
                    wbemAttrCallback,
                    (void *)&attributes);

                // SLPParseSrvURL()
                // @param1 - url - obtained from SLPFindSrvs()
                // @param2 - parsed url - output param
                SLPSrvURL *pSrvUrl = NULL;
                if (SLP_OK == SLPParseSrvURL(
                        (char *)((const char *)urls[i].getCString()),
                        &pSrvUrl))
                {
                    // add to the end to protect against existing attributes
                    // of the same name.
                    Attribute tmpAttr(PEG_CUSTOM_ATTR_HOST);
                    tmpAttr.addValue(pSrvUrl->s_pcHost);
                    attributes.append(tmpAttr);

                    Attribute tmpAttr2(PEG_CUSTOM_ATTR_PORT);
                    CIMValue value(Uint32(pSrvUrl->s_iPort));
                    tmpAttr2.addValue(value.toString());
                    attributes.append(tmpAttr2);

                    // free up slp library memory
                    SLPFree(pSrvUrl);
                }
                connection.setAttributes(attributes);
                connections.append(connection);
            }
        }

        // SLPClose()
        // @param1 - handle - slp handle
        SLPClose(hslp);
    }

    return connections;
}

PEGASUS_NAMESPACE_END
