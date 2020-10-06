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


#include "ComputerSystemProvider.h"
#include "ComputerSystem.h"
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Constants.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

ComputerSystem::ComputerSystem()
{
}

ComputerSystem::~ComputerSystem()
{
}


Boolean ComputerSystem::getCaption(CIMProperty& p)
{
    // hardcoded
    p = CIMProperty(PROPERTY_CAPTION, String(CAPTION));

    return true;
}

Boolean ComputerSystem::getDescription(CIMProperty& p)
{
    // hardcoded
    p = CIMProperty(PROPERTY_DESCRIPTION, String(DESCRIPTION));
    return true;
}

Boolean ComputerSystem::getInstallDate(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getCreationClassName(CIMProperty& p)
{
    // can vary, depending on class
    p = CIMProperty(PROPERTY_CREATION_CLASS_NAME,
        PEGASUS_CLASSNAME_PG_COMPUTERSYSTEM.getString());
    return true;
}

Boolean ComputerSystem::getName(CIMProperty& p)
{
    p = CIMProperty(PROPERTY_NAME,String(getHostName()));
    return true;
}

Boolean ComputerSystem::getStatus(CIMProperty& p)
{
  // hardcoded
  p = CIMProperty(PROPERTY_STATUS,String(STATUS));
  return true;
}

Boolean ComputerSystem::getOperationalStatus(CIMProperty& p)
{
  Array<Uint16> opStatus;
  opStatus.append(2); // OK
  p = CIMProperty(PROPERTY_OPERATIONAL_STATUS, opStatus);
  return true;
}

Boolean ComputerSystem::getStatusDescriptions(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getNameFormat(CIMProperty& p)
{
  p = CIMProperty(PROPERTY_NAME_FORMAT,String(NAME_FORMAT));
  return true;
}

Boolean ComputerSystem::getPrimaryOwnerName(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::setPrimaryOwnerName(const String&)
{
  return false;
}

Boolean ComputerSystem::getPrimaryOwnerContact(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::setPrimaryOwnerContact(const String&)
{
  return false;
}

Boolean ComputerSystem::getRoles(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getOtherIdentifyingInfo(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getIdentifyingDescriptions(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getDedicated(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getResetCapability(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getPowerManagementCapabilities(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getInitialLoadInfo(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getLastLoadInfo(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getPowerManagementSupported(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getPowerState(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getWakeUpType(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getPrimaryOwnerPager(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::setPrimaryOwnerPager(const String&)
{
  return false;
}

Boolean ComputerSystem::getSecondaryOwnerName(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::setSecondaryOwnerName(const String&)
{
  return false;
}

Boolean ComputerSystem::getSecondaryOwnerContact(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::setSecondaryOwnerContact(const String&)
{
  return false;
}

Boolean ComputerSystem::getSecondaryOwnerPager(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::setSecondaryOwnerPager(const String&)
{
  return false;
}

Boolean ComputerSystem::getSerialNumber(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getIdentificationNumber(CIMProperty& p)
{
  return false;
}

Boolean ComputerSystem::getElementName(CIMProperty& p)
{
  // We're just going to re-use the caption
  p = CIMProperty(PROPERTY_ELEMENTNAME, String(CAPTION));
  return true;
}

void ComputerSystem::initialize(void)
{
}

String ComputerSystem::getHostName(void)
{
    return System::getHostName();
}

