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
//
//%/////////////////////////////////////////////////////////////////////////////

#include "IPInfo.h"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static const CIMName          CLASS_NAME = CIMName          ("PG_IPRoute");
static const CIMNamespaceName NAMESPACE  = CIMNamespaceName ("root/cimv2");
#define HeaderFormat "%-16s %-10s %-16s %-16s %-16s"


////////////////////////////////////////////////////////////////////////////////
//  Constructor for IP Route Info
////////////////////////////////////////////////////////////////////////////////
IPRouteInfo::IPRouteInfo(CIMClient &client, Boolean enableDebug,
                  ostream& outPrintWriter, ostream& errPrintWriter)
{
    _enableDebug = enableDebug;

    try
    {
        Boolean deepInheritance = true;
        Boolean localOnly = true;
        Boolean includeQualifiers = false;
        Boolean includeClassOrigin = false;

        Array<CIMInstance> cimInstances =
            client.enumerateInstances(NAMESPACE, CLASS_NAME,
                                      deepInheritance,
                                      localOnly,
                                      includeQualifiers,
                                      includeClassOrigin);

        Uint32 numberInstances = cimInstances.size();

        if (_enableDebug)
    {
        outPrintWriter << numberInstances << " instances of " <<
                 CLASS_NAME.getString() << endl;
    }

    if (numberInstances > 0)
    {
        _gatherProperties(cimInstances[0]);
        _outputHeader(outPrintWriter);

        for (Uint32 i = 0; i < numberInstances; i++)
        {
            _gatherProperties(cimInstances[i]);
            _outputInstance(outPrintWriter);

         }   // end for looping through instances
    }
        else
    {
         outPrintWriter << "No instances of class "
                        << CLASS_NAME.getString() << endl;
    }

    }  // end try

    catch(Exception&)
    {
        errPrintWriter << "Error getting instances of class " <<
             CLASS_NAME.getString() << endl;
    }

}

////////////////////////////////////////////////////////////////////////////////
//  Destructor for IP Route Info
////////////////////////////////////////////////////////////////////////////////
IPRouteInfo::~IPRouteInfo(void)
{
}

////////////////////////////////////////////////////////////////////////////////
//  Gather Properities for IP Route Info
////////////////////////////////////////////////////////////////////////////////
void IPRouteInfo::_gatherProperties(CIMInstance &inst)
{
    _ipSCCN        = String::EMPTY;
    _ipSN          = String::EMPTY;
    _ipServiceCCN  = String::EMPTY;
    _ipServiceN    = String::EMPTY;
    _ipCCN         = String::EMPTY;
    _ipIPDestAddr  = String::EMPTY;
    _ipIPDestMask  = String::EMPTY;
    _ipAddrType    = 0;  // Unknown

    _ipCaption     = String::EMPTY;
    _ipDescription = String::EMPTY;
    _ipName        = String::EMPTY;
    _ipNextHop     = String::EMPTY;

    for (Uint32 j=0; j < inst.getPropertyCount(); j++)
    {
        CIMName propertyName = inst.getProperty(j).getName();

        // Properties that are also keys
        if (propertyName.equal("SystemCreationClassName"))
        {
            inst.getProperty(j).getValue().get(_ipSCCN);
        }
        else if (propertyName.equal("SystemName"))
        {
            inst.getProperty(j).getValue().get(_ipSN);
        }
        else if (propertyName.equal("ServiceCreationClassName"))
        {
            inst.getProperty(j).getValue().get(_ipServiceCCN);
        }
        else if (propertyName.equal("ServiceName"))
        {
            inst.getProperty(j).getValue().get(_ipServiceN);
        }
        else if (propertyName.equal("CreationClassName"))
        {
            inst.getProperty(j).getValue().get(_ipCCN);
        }
        else if (propertyName.equal("IPDestinationAddress"))
        {
            inst.getProperty(j).getValue().get(_ipIPDestAddr);
        }
        else if (propertyName.equal("IPDestinationMask"))
        {
        inst.getProperty(j).getValue().get(_ipIPDestMask);
        }
        else if (propertyName.equal("AddressType"))
        {
            inst.getProperty(j).getValue().get(_ipAddrType);
        }
        // Other properties
        else if (propertyName.equal("Caption"))
        {
            inst.getProperty(j).getValue().get(_ipCaption);
        }
        else if (propertyName.equal("Description"))
        {
            inst.getProperty(j).getValue().get(_ipDescription);
        }
        else if (propertyName.equal("Name"))
        {
            inst.getProperty(j).getValue().get(_ipName);
        }
        else if (propertyName.equal("NextHop"))
        {
            inst.getProperty(j).getValue().get(_ipNextHop);
        }
   } // end for loop through properties

}


////////////////////////////////////////////////////////////////////////////////
//  Header Section for IP Route Info
////////////////////////////////////////////////////////////////////////////////
void IPRouteInfo::_outputHeader(ostream &outPrintWriter)
{

    outPrintWriter << endl << ">>>> IP Route Information <<<<" << endl << endl;

    if (_ipSCCN.size() > 0)
        outPrintWriter << "System Creation Class Name  : " << _ipSCCN << endl;

    if (_ipSN.size() > 0)
        outPrintWriter << "System Name                 : " << _ipSN << endl;

    if (_ipServiceCCN.size() > 0)
        outPrintWriter << "Service Creation Class Name : "
                       << _ipServiceCCN << endl;

    if (_ipServiceN.size() > 0)
        outPrintWriter << "Service Name                : "
                       << _ipServiceN << endl;

    if (_ipCCN.size() > 0)
        outPrintWriter << "Creation Class Name         : " << _ipCCN << endl;

    char header[81];

    sprintf(header, HeaderFormat, "Route", "AddrType", "IP Dest Addr",
                                  "IP Dest Mask", "Next Hop");
    outPrintWriter << endl << header << endl;

}

////////////////////////////////////////////////////////////////////////////////
//  Output an instance of an IP Route Info
////////////////////////////////////////////////////////////////////////////////
void IPRouteInfo::_outputInstance(ostream &outPrintWriter)
{
    String _ipAT;

    if (_ipAddrType == 1)
        _ipAT = "IPv4";
    else if (_ipAddrType == 2)
        _ipAT = "IPv6";
    else
        _ipAT = "Unk";

    char row[81];

    sprintf(row, HeaderFormat, (const char *)_ipName.getCString(),
       (const char *)_ipAT.getCString(),
       (const char *)_ipIPDestAddr.getCString(),
       (const char *)_ipIPDestMask.getCString(),
       (const char *)_ipNextHop.getCString()
       );
    outPrintWriter << row << endl;

}
