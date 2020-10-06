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
// Modified By:  Jenny Yu, Hewlett-Packard Company (jenny.yu@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// INCLUDES
//------------------------------------------------------------------------------
#include "NISServerService.h"

//------------------------------------------------------------------------------
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
NISServerService::NISServerService(void)
{
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
NISServerService::~NISServerService(void)
{
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
Boolean NISServerService::getLocalHostName(String & hostName)
{
     return false;
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
Boolean NISServerService::AccessOk(const OperationContext & context)
{
    return false;
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
    return false;
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
     return false;
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
     return false;
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
     return false;
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
     return false;
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
     return false;
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
     return false;
}

