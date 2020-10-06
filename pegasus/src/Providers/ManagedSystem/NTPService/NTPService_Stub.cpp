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
//
// Modified By: Jair Francisco T. dos Santos (t.dos.santos.francisco@non.hp.com)
//==============================================================================
// Based on DNSService_Stub.cpp file
//%////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include "NTPService.h"

//------------------------------------------------------------------------------
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

//==============================================================================
//
// Class [NTPService] methods
//
//==============================================================================

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
NTPService::NTPService(void)
{
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
NTPService::~NTPService(void)
{
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
Boolean NTPService::getSystemName(String& systemName)
{
    return false;
}

//------------------------------------------------------------------------------
// FUNCTION: getNTPName
//
// REMARKS: returns the Name property
//
// PARAMETERS: [OUT] strValue -> string that will receive the NTP_Name property
//                                 value
//
// RETURN: TRUE (hard-coded property value)
//------------------------------------------------------------------------------
Boolean NTPService::getNTPName(String & strValue)
{
    return false;
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
Boolean NTPService::AccessOk(const OperationContext & context)
{
    return true;
}

//------------------------------------------------------------------------------
// FUNCTION: getNTPInfo
//
// REMARKS: Retrieves the NTP information from the "/etc/ntp.conf" file,
//            and sets private variables to hold the data read.
//
// RETURN:
//------------------------------------------------------------------------------
Boolean NTPService::getNTPInfo()
{
    return true;
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
Boolean NTPService::getLocalHostName(String & hostName)
{
    return true;
}

//------------------------------------------------------------------------------
// FUNCTION: getCaption
//
// REMARKS: returns the Caption property
//
// PARAMETERS: [OUT] strValue -> string that will receive the Caption property
//                                 value
//
// RETURN: TRUE
//------------------------------------------------------------------------------
Boolean NTPService::getCaption(String & strValue)
{
    return true;
}

//------------------------------------------------------------------------------
// FUNCTION: getDescription
//
// REMARKS: returns the Description property
//
// PARAMETERS: [OUT] strValue -> string that will receive the Description
//                                 property value
//
// RETURN: TRUE
//------------------------------------------------------------------------------
Boolean NTPService::getDescription(String & strValue)
{
    return true;
}

//------------------------------------------------------------------------------
// FUNCTION: getServerAddress
//
// REMARKS: returns the ServerAddress property
//
// PARAMETERS: [OUT] strValue -> string that will receive the ServerAddress
//                                 property value
//
// RETURN: TRUE
//------------------------------------------------------------------------------
Boolean NTPService::getServerAddress(Array<String> & strValue)
{
    return true;
}

