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

#include "IPInfo.h"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static const CIMName          CLASS_NAME = CIMName ("PG_BindsIPToLANEndpoint");
static const CIMNamespaceName NAMESPACE  = CIMNamespaceName ("root/cimv2");
#define HeaderFormat "%-22s %-22s %-15s"


////////////////////////////////////////////////////////////////////////////////
//  Constructor for Binds IP To LAN Endpoint Association
////////////////////////////////////////////////////////////////////////////////
BIPTLEpInfo::BIPTLEpInfo(CIMClient &client, Boolean enableDebug,
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
            outPrintWriter << numberInstances << " instances of "
                << CLASS_NAME.getString() << endl;
        }

        if (numberInstances > 0)
        {
            _gatherProperties(cimInstances[0], outPrintWriter);
            _outputHeader(outPrintWriter);

            for (Uint32 i = 0; i < numberInstances; i++)
            {
                _gatherProperties(cimInstances[i], outPrintWriter);
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
        errPrintWriter << "Error getting instances of class "
            << CLASS_NAME.getString() << endl;
    }

}

////////////////////////////////////////////////////////////////////////////////
//  Destructor for Binds IP To LAN Endpoint Association
////////////////////////////////////////////////////////////////////////////////
BIPTLEpInfo::~BIPTLEpInfo(void)
{

}

////////////////////////////////////////////////////////////////////////////////
//  Gather Properities from Association
////////////////////////////////////////////////////////////////////////////////
void BIPTLEpInfo::_gatherProperties(CIMInstance &inst, ostream& outPrintWriter)
{
    _ipLEPName   = String::EMPTY;
    _ipIPPEpName = String::EMPTY;
    _ipLEPCCN    = String::EMPTY;
    _ipIPPEpCCN  = String::EMPTY;

    _ipFrameType = 0;  // Unknown

    // Extract the properties
    for (Uint32 j=0; j < inst.getPropertyCount(); j++)
    {
        CIMName propertyName = inst.getProperty(j).getName();

        if (propertyName.equal("FrameType"))
        inst.getProperty(j).getValue().get(_ipFrameType);

    } // end for loop through properties


    // Extract the key bindings

    CIMObjectPath path = inst.getPath();

    CIMName keyName;
    Array<CIMKeyBinding> kb = path.getKeyBindings();

    Uint32 numKeys = kb.size();

    if (_enableDebug)
        outPrintWriter << "Retrieved " << numKeys
                       << " keys in association." << endl;

    for (Uint32 j=0; j < numKeys; j++)
    {
        keyName = kb[j].getName();

        if (keyName.equal("Antecedent"))
        {
            CIMObjectPath _Ant = kb[j].getValue();
            _extractFromKey(_Ant, _ipLEPCCN, _ipLEPName, outPrintWriter);
        }

        else if (keyName.equal("Dependent"))
        {
            CIMObjectPath _Dep = kb[j].getValue();
            _extractFromKey(_Dep, _ipIPPEpCCN, _ipIPPEpName, outPrintWriter);
        }

    } // end for loop through keys

}


////////////////////////////////////////////////////////////////////////////////
//  Extract Information from a Key
////////////////////////////////////////////////////////////////////////////////
void BIPTLEpInfo::_extractFromKey(CIMObjectPath &ref, String &ccn,
                                String &name, ostream &outPrintWriter)
{
    CIMName keyName;
    Array<CIMKeyBinding> kb = ref.getKeyBindings();

    Uint32 numKeys = kb.size();

    if (_enableDebug)
        outPrintWriter << "Retrieved " << numKeys << " keys in reference `"
        << ref.getClassName().getString() << "'." << endl;

    for (Uint32 j=0; j < numKeys; j++)
    {
        keyName = kb[j].getName();

        if (keyName.equal("Name"))
            name = kb[j].getValue();

        else if (keyName.equal("CreationClassName"))
            ccn = kb[j].getValue();

    } // end for loop through keys

}

////////////////////////////////////////////////////////////////////////////////
//  Header Section for Association
////////////////////////////////////////////////////////////////////////////////
void BIPTLEpInfo::_outputHeader(ostream &outPrintWriter)
{

    outPrintWriter << endl << ">>>> IP Associations to LAN Endpoint <<<<"
        << endl << endl;

    if (_ipLEPCCN.size() > 0)
        outPrintWriter << "LAN Endpoint CCN : " << _ipLEPCCN << endl;

    if (_ipIPPEpCCN.size() > 0)
        outPrintWriter << "IP Endpoint CCN  : " << _ipIPPEpCCN << endl;

    char header[81];

    sprintf(header, HeaderFormat, "LAN Endpoint", "IP Protocol Endpoint",
            "Frame Type");
    outPrintWriter << endl << header << endl;

}

////////////////////////////////////////////////////////////////////////////////
//  Output an instance of the Association
////////////////////////////////////////////////////////////////////////////////
void BIPTLEpInfo::_outputInstance(ostream &outPrintWriter)
{
    String _ipFT;

    if (_ipFrameType == 1)
        _ipFT = "Ethernet";
    else if (_ipFrameType == 0)
        _ipFT = "Unknown";
    else
        _ipFT = "Other";

    char row[81];

    sprintf(row, HeaderFormat, (const char *)_ipLEPName.getCString(),
        (const char *)_ipIPPEpName.getCString(),
        (const char *)_ipFT.getCString()
        );
    outPrintWriter << row << endl;

}
