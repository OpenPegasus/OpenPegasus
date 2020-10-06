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

//%////////////////////////////////////////////////////////////////////////////

/*
    Definitions for the DynamicRoutingTable that controls fixed registration
    of OpenPegasus control providers and services.
    NOTE: The actual Control Provider and Service registration definitions
    are contained in the DynamicRoutingTable .cpp file.

    DynamicRoutingTable is a singleton which includes the following
    public functions:
      - getRouting - Gets the provider routing for a particular class,namespace
        Asserts if called before the table is built. This is the function
        used to get routing information about Control Providers and
        OpenPegasus services
      - getRoutingTable - gets the pointer to the DynamicRoutingTable but
        does not actually create the table. Used to allow the
        CIMOperationRequestDispatcher to set an address for the table as part
        of initialization before it the table is created.
      - buildRoutingTable - Builds the DynamicRouting table hash table
        from the definitions defined in the cpp file.  This must be done
        late in initialization since this table includes queueId information
        which is not defined until almost the end of the OpenPegasus
        initialization sequence.
*/

#ifndef Pegasus_reg_table_h
#define Pegasus_reg_table_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/ArrayInter.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Server/Linkage.h>
#include <Pegasus/Common/AutoPtr.h>

PEGASUS_NAMESPACE_BEGIN

/*
    Class defining a single DynamicRoutingTable Record.
*/
class RegTableRecord
{
public:
    RegTableRecord(
        const CIMName& className,
        const CIMNamespaceName& namespaceName,
        const String& providerName,
        Uint32 serviceId);
    ~RegTableRecord();

    CIMName className;
    CIMNamespaceName namespaceName;
    String providerName;
    Uint32 serviceId;
private:
    RegTableRecord(const RegTableRecord&);
    RegTableRecord& operator=(const RegTableRecord&);
};

class PEGASUS_SERVER_LINKAGE DynamicRoutingTable
{
public:
    // Removes the hash table and pointer
    ~DynamicRoutingTable();

    // Get pointer to the singleton instance of DynamicRoutingTable.
    // Note that this does NOT build the table. It simply allocates a pointer
    // for the table if necessary and returns the pointer. This allows
    // initialization of the table pointer in the OpenPegasus initialization
    // sequence before the table is actually built.
    static DynamicRoutingTable* getRoutingTable();

    /* Get a single provider or service that can handle the className and
       namespace provided as parameters.
       If successful, returns provider in providerName and and queueId of
       the correct service for routing..
       @param CIMName className for which provider info wanted
       @param CIMNamespaceName namespace of request
       @param provider String defining provider name on return if the
       routing is to a ControlProvider. Empty String if service routing
       found.
       @param serviceId Uint32 queueId for routing to provider of service
       defined by CIMName and className if an entry is found.
       @return true if routing found. False if routing not found.
    */
    Boolean getRouting(
        const CIMName& className,
        const CIMNamespaceName& namespaceName,
        String& provider,
        Uint32 &serviceId) const;

    // Build the complete routing table. Builds the DynamicRoutingTable from
    // definition of Control Providers and Services in reg_table.cpp
    static void buildRoutingTable();

#ifdef PEGASUS_DEBUG
    void dumpRegTable();
#endif
private:
    // Private since the only way to construct the table is getRoutingTable
    // and buildRoutingTable.
    // Not a general singleton pattern because table is built during
    // initialization by buildRoutingTable() and only accessed subsequently.
    DynamicRoutingTable();
    DynamicRoutingTable(const DynamicRoutingTable& table);
    DynamicRoutingTable& operator=(const DynamicRoutingTable& table);

    void _insertRecord(
        const CIMName& className,
        const CIMNamespaceName& namespaceName,
        const String& provider,
        Uint32 serviceId);

    String _getRoutingKey(
        const CIMName& className,
        const CIMNamespaceName& namespaceName) const;

    String _getWildRoutingKey(
        const CIMName& className) const;

    typedef HashTable<String, RegTableRecord*,
        EqualNoCaseFunc, HashFunc<String> > RoutingTable;

    RoutingTable _routingTable;

    // pointer to the singleton instance of the DynamicRoutingTable.
    static AutoPtr<DynamicRoutingTable> _this;
#ifdef PEGASUS_DEBUG
    // flag to indication table actually built. DEBUG only since the
    // order should be fixed between getRouting and buildRoutingTable
    // functions
    static Boolean _tableInitialized;
#endif
};

PEGASUS_NAMESPACE_END

#endif // Pegasus_reg_table_h
