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

#ifndef Pegasus_snmpDeliverTrap_h
#define Pegasus_snmpDeliverTrap_h

#include <iostream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/String.h>

PEGASUS_NAMESPACE_BEGIN

typedef struct _trapHeader
{
    char destination[255];
    char snmpType[10];
    char enterprise[255];
    char trapOid[255];
    int  variable_packets;
} trapHeader;

typedef struct _trapData
{
    char vbOid[255];
    char vbType[255];
    char vbValue[512];
} trapData;

class snmpDeliverTrap
{
public:

    snmpDeliverTrap() { }

    virtual ~snmpDeliverTrap() { }

    virtual void initialize() = 0;

    virtual void terminate() = 0;

    virtual void deliverTrap(
        const String& trapOid,
        const String& securityName,
        const String& targetHost,
        const Uint16& targetHostFormat,
        const String& otherTargetHostFormat,
        const Uint32& portNumber,
        const Uint16& snmpVersion,
        const String& engineID,
        const Uint8& snmpSecLevel,
        const Uint8& snmpSecAuthProto,
        const Array<Uint8>& snmpSecAuthKey,
        const Uint8& snmpSecPrivProto,
        const Array<Uint8>& snmpSecPrivKey,
        const Array<String>& vbOids,
        const Array<String>& vbTypes,
        const Array<String>& vbValues) = 0;
};

PEGASUS_NAMESPACE_END

#endif // Pegasus_snmpDeliverTrap_h
