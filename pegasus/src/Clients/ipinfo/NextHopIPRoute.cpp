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

static const CIMName CLASS_PG_NEXT_HOP_IP_ROUTE = CIMName(
    "PG_NextHopIPRoute");
static const CIMName CLASS_CIM_NEXT_HOP_ROUTE = CIMName(
    "CIM_NextHopRoute");
static const CIMName CLASS_PG_ROUTE_USES_ENDPOINT = CIMName(
    "PG_RouteUsesEndpoint");
static const CIMName CLASS_PG_ASSOCIATED_NEXT_HOP = CIMName(
    "PG_AssociatedNextHop");

static const CIMName PROPERTY_NEXT_HOP = CIMName(
    "NextHop");
static const CIMName PROPERTY_IPV4ADDRESS = CIMName(
    "IPv4Address");
static const CIMName PROPERTY_IPV6ADDRESS = CIMName(
    "IPv6Address");
static const CIMName PROPERTY_ACCESS_INFO = CIMName(
    "AccessInfo");

static const CIMNamespaceName NAMESPACE  = CIMNamespaceName ("root/cimv2");
#define HeaderFormat "%-16s %-10s %-16s %-16s %-16s"


////////////////////////////////////////////////////////////////////////////////
//  Constructor for Next Hop IP Route Info
////////////////////////////////////////////////////////////////////////////////
NextHopIPRouteInfo::NextHopIPRouteInfo(
    CIMClient &client,
    Boolean enableDebug,
    ostream& outPrintWriter,
    ostream& errPrintWriter)
{
    _enableDebug = enableDebug;
    Array<CIMInstance> retInstances;

    try
    {
        Boolean deepInheritance = true;
        Boolean localOnly = true;
        Boolean includeQualifiers = false;
        Boolean includeClassOrigin = false;

        Array<CIMInstance> cimInstances = client.enumerateInstances(
            NAMESPACE,
            CLASS_PG_NEXT_HOP_IP_ROUTE,
            deepInheritance,
            localOnly,
            includeQualifiers,
            includeClassOrigin);

        Array<CIMObjectPath> nhiprRefs = client.enumerateInstanceNames(
            NAMESPACE,
            CLASS_CIM_NEXT_HOP_ROUTE);

        for (Uint16 i = 0; i<nhiprRefs.size(); i++)
        {
            CIMName resultClass;
            String role;
            String resultRole;
            CIMObjectPath _nhrRef = nhiprRefs[i];
            _nhrRef.setClassName(CLASS_CIM_NEXT_HOP_ROUTE);

            // Get the association instance of CIM_RouteUsesEndpoint
            // for each instance of CIM_NextHopRoute class
            Array<CIMObject> localRefs = client.associators(
                NAMESPACE,
                _nhrRef,
                CLASS_PG_ROUTE_USES_ENDPOINT,
                resultClass,
                role,
                resultRole);

            if (localRefs.size() == 1)
            {
                Uint32 index = localRefs[0].findProperty(PROPERTY_IPV4ADDRESS);
                if (index == PEG_NOT_FOUND)
                {
                    index = localRefs[0].findProperty(PROPERTY_IPV6ADDRESS);
                    if (index == PEG_NOT_FOUND)
                    {
                        errPrintWriter <<
                            "Error getting IPv4Address and IPv6Address " <<
                            "property: " << "not found!" << endl;
                    }
                    else
                    {
                        CIMInstance _cimInstance = cimInstances[i];
                        String _nextHop;
                        localRefs[0].getProperty(index).getValue().get(
                            _nextHop);
                        CIMProperty _nhProperty(
                            PROPERTY_NEXT_HOP,
                             CIMValue(_nextHop));
                        _cimInstance.addProperty(_nhProperty);
                        retInstances.append(_cimInstance);
                    }
                }
                else
                {
                    CIMInstance _cimInstance = cimInstances[i];
                    String _nextHop;
                    localRefs[0].getProperty(index).getValue().get(_nextHop);
                    CIMProperty _nhProperty(
                        PROPERTY_NEXT_HOP,
                         CIMValue(_nextHop));
                    _cimInstance.addProperty(_nhProperty);
                    retInstances.append(_cimInstance);
                }
            }
            else
            {
                // Get the association instance of CIM_AssociatedNextHop
                // for each instance of CIM_RemoteServiceAccessPoint class
                Array<CIMObject> remoteRefs = client.associators(
                    NAMESPACE,
                    _nhrRef,
                    CLASS_PG_ASSOCIATED_NEXT_HOP,
                    resultClass,
                    role,
                    resultRole);

                if (remoteRefs.size() == 1)
                {
                    Uint32 index = remoteRefs[0].findProperty(
                        PROPERTY_ACCESS_INFO);
                    if (index != PEG_NOT_FOUND)
                    {
                        CIMInstance _cimInstance = cimInstances[i];
                        String _accessInfo;
                        remoteRefs[0].getProperty(index).getValue().get(
                            _accessInfo);
                        Uint32 _sep = _accessInfo.find('/');
                        if (_sep != PEG_NOT_FOUND)
                        {
                            String _nextHop = _accessInfo.subString(0,_sep);
                            CIMProperty _nhProperty(
                                 PROPERTY_NEXT_HOP,
                                 CIMValue(_nextHop));
                            _cimInstance.addProperty(_nhProperty);
                            retInstances.append(_cimInstance);
                        }
                    }
                    else
                    {
                        errPrintWriter << "Error getting AccessInfo property: "
                            << "not found! " << endl;
                    }
                }
                else
                {
                      outPrintWriter << "Unexpected number of references "
                          << "for this instance of class CIM_NextHopRoute :"
                          << remoteRefs.size() << endl;
                }
            }
        }

        Uint32 numberInstances = retInstances.size();

        if (_enableDebug)
        {
            outPrintWriter << numberInstances << " instances of " <<
                CLASS_PG_NEXT_HOP_IP_ROUTE.getString() << endl;
        }

        if (numberInstances > 0)
        {
            _gatherProperties(retInstances[0]);
            _outputHeader(outPrintWriter);

            for (Uint32 i = 0; i < numberInstances; i++)
            {
                _gatherProperties(retInstances[i]);
                _outputInstance(outPrintWriter);

            }   // end for looping through instances.
        }
        else
        {
            outPrintWriter << "No instances of class "
                << CLASS_PG_NEXT_HOP_IP_ROUTE.getString() << endl;
        }

    }  // end try .
    catch(Exception& e)
    {
        errPrintWriter << "Error getting instances of class " <<
            CLASS_PG_NEXT_HOP_IP_ROUTE.getString() <<
            " " << e.getMessage() << endl;
    }

}

////////////////////////////////////////////////////////////////////////////////
//  Destructor for Next Hop IP Route Info
////////////////////////////////////////////////////////////////////////////////
NextHopIPRouteInfo::~NextHopIPRouteInfo(void)
{
}

////////////////////////////////////////////////////////////////////////////////
//  Gather Properities for Next Hop IP Route Info
////////////////////////////////////////////////////////////////////////////////
void NextHopIPRouteInfo::_gatherProperties(CIMInstance &inst)
{
    _ipInstanceID = String::EMPTY;

    _ipIPDestAddr = String::EMPTY;
    _ipIPDestMask = String::EMPTY;
    _ipAddrType = 0;  // Unknown
    _ipCaption = String::EMPTY;
    _ipDescription = String::EMPTY;
    _ipName = String::EMPTY;

    for (Uint32 j=0; j < inst.getPropertyCount(); j++)
    {
        CIMName propertyName = inst.getProperty(j).getName();

        // Properties that are also keys
        if (propertyName.equal("InstanceID"))
        {
            inst.getProperty(j).getValue().get(_ipInstanceID);
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
        else if (propertyName.equal("DestinationAddress"))
        {
            inst.getProperty(j).getValue().get(_ipIPDestAddr);
        }
        else if (propertyName.equal("DestinationMask"))
        {
            inst.getProperty(j).getValue().get(_ipIPDestMask);
        }
        else if (propertyName.equal("AddressType"))
        {
            inst.getProperty(j).getValue().get(_ipAddrType);
        }
        else if (propertyName.equal("PrefixLength"))
        {
            inst.getProperty(j).getValue().get(_ipPrefixLength);
        }
        else if (propertyName.equal(PROPERTY_NEXT_HOP))
        {
            inst.getProperty(j).getValue().get(_ipNextHop);
        }
   } // end for loop through properties

}


////////////////////////////////////////////////////////////////////////////////
//  Header Section for Next Hop IP Route Info
////////////////////////////////////////////////////////////////////////////////
void NextHopIPRouteInfo::_outputHeader(ostream &outPrintWriter)
{

    outPrintWriter << endl << ">>>> Next Hop IP Route Information <<<<"
        << endl << endl;

    if (_ipInstanceID.size() > 0)
    {
        outPrintWriter << "InstanceID         : " << _ipInstanceID << endl;
    }

    char header[81];

    sprintf(header, HeaderFormat, "Route", "AddrType", "IP Dest Addr",
        "DestMask/PrefLen", "NextHop");

    outPrintWriter << endl << header << endl;

}

////////////////////////////////////////////////////////////////////////////////
//  Output an instance of an Next Hop IP Route Info
////////////////////////////////////////////////////////////////////////////////
void NextHopIPRouteInfo::_outputInstance(ostream &outPrintWriter)
{
    char row[81];

    if (_ipAddrType == 1)
    {
        sprintf(
            row,
            HeaderFormat,
            (const char *)_ipName.getCString(),
            "IPv4",
            (const char *)_ipIPDestAddr.getCString(),
            (const char *)_ipIPDestMask.getCString(),
            (const char *)_ipNextHop.getCString());
    }
    else
    {
        if (_ipAddrType == 2)
        {
            String _ipt = "IPv6";

            if (_ipName.size() > 15)
            {
                sprintf(
                    row,
                    HeaderFormat,
                    (const char *)_ipName.getCString(),
                    "",
                    "",
                    "",
                    "");
                outPrintWriter << row << endl;
                _ipName.clear();
            }

            if (_ipIPDestAddr.size() > 15)
            {
                sprintf(
                    row,
                    HeaderFormat,
                    (const char *)_ipName.getCString(),
                    (const char *)_ipt.getCString(),
                    (const char *)_ipIPDestAddr.getCString(),
                    "",
                    "");
                outPrintWriter << row << endl;
                _ipIPDestAddr.clear();
                _ipt.clear();
            }

            char _pl[10];
            sprintf(_pl,"%d",_ipPrefixLength);

            sprintf(
                row,
                HeaderFormat,
                (const char *)_ipName.getCString(),
                (const char *)_ipt.getCString(),
                (const char *)_ipIPDestAddr.getCString(),
                _pl,
                (const char *)_ipNextHop.getCString());
        }
        else
        {
            sprintf(
                row,
                HeaderFormat,
                (const char *)_ipName.getCString(),
                "Unk",
                (const char *)_ipIPDestAddr.getCString(),
                (const char *)_ipIPDestMask.getCString(),
                (const char *)_ipNextHop.getCString());
        }
    }


    outPrintWriter << row << endl;

}
