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
// Author: Paulo F. Borges (pfborges@wowmail.com)
//
// Modified By:
//         Lyle Wilkinson, Hewlett-Packard Company <lyle_wilkinson@hp.com>
//
//%/////////////////////////////////////////////////////////////////////////////

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;


//------------------------------------------------------------------------------
// FUNCTION:   DNSFileOk
//
// REMARKS:    Verify DNS File exists and has appropriate contents
//
// PARAMETERS: none
//
// RETURN:     true, file okay, otherwise false
//------------------------------------------------------------------------------
Boolean DNSFileOk()
{
  return false;
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
DNSService::DNSService(void)
{
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
DNSService::~DNSService(void)
{
}

//------------------------------------------------------------------------------
// FUNCTION:   AccessOk
//
// REMARKS:    Verify if client can access file /etc/resolv.conf
//
// PARAMETERS: [IN] context -> pointer to Operation context
//
// RETURN:     true, if user has privileges, otherwise false
//------------------------------------------------------------------------------
Boolean DNSService::AccessOk(const OperationContext & context)
{
    return false;
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
// FUNCTION:   getSystemName
//
// REMARKS:    Function to retrieve local host name
//
// PARAMETERS: [OUT] systemName -> string that will contain the system name
//
// RETURN:     true, if successful, false otherwise
//------------------------------------------------------------------------------
Boolean DNSService::getSystemName(String & systemName)
{
    return false;
}


//------------------------------------------------------------------------------
// FUNCTION:   getDNSName
//
// REMARKS:    Function to get instance name
//
// PARAMETERS: [OUT] string -> string that contains the name property value
//
// RETURN:     true, if successful, false otherwise
//------------------------------------------------------------------------------
Boolean DNSService::getDNSName(String &) {
    return false;
}

//------------------------------------------------------------------------------
// FUNCTION:   getSearchList
//
// REMARKS:    Function to retrieve the search list, if it exists
//
// PARAMETERS: [OUT] string -> string that contains the SearchList property
//
// RETURN:     true, if successful, false otherwise
//------------------------------------------------------------------------------
Boolean DNSService::getSearchList(Array<String> &) {
    return false;
}

//------------------------------------------------------------------------------
// FUNCTION:   getAddresses
//
// REMARKS:    Function to retrieve the addresses, if any exists
//
// PARAMETERS: [OUT] string -> string that contains the Addresses property
//
// RETURN:     true, if successful, false otherwise
//------------------------------------------------------------------------------
Boolean DNSService::getAddresses(Array<String> &) {
    return false;
}

//------------------------------------------------------------------------------
// FUNCTION:   getDNSInfo
//
// REMARKS:    Read domain name, addresses, and search list from resolv.conf
//
// PARAMETERS: none.
//
// RETURN:     true, if successful, false otherwise
//------------------------------------------------------------------------------
Boolean DNSService::getDNSInfo(void)
{
    return false;
}
