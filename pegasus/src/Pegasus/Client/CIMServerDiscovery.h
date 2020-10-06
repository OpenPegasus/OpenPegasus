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

#ifndef Pegasus_CIMServerDiscovery_h
#define Pegasus_CIMServerDiscovery_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Attribute.h>
#include <Pegasus/Common/CIMServerDescription.h>
#include <Pegasus/Client/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

//*****************************************************************************
// SLPClientOptions
//*****************************************************************************

/** Purpose: allow some flexibility in the nature of the SLP
    srv requests, particularly to permit unicast da requests,
    passing of scopes and spi parameters etc.
*/

struct PEGASUS_CLIENT_LINKAGE SLPClientOptions
{
    char* target_address;
    char* local_interface;
    unsigned short target_port;
    char* spi;
    char* scopes;
    char* service_type;
    char* predicate;
    bool use_directory_agent;
    /**
        Constructs an empty object. User must strdup the
        string values during initialistation,
        which are then freed automatically during
        destruction.
    */
    SLPClientOptions();

    /**
        Destroys object and contained string values
    */
    ~SLPClientOptions();

    /**
        For debugging
    */
    void print() const;

private:
    SLPClientOptions(const SLPClientOptions& options);
    SLPClientOptions& operator=(const SLPClientOptions& options);
};


//*****************************************************************************
// CIMServerDiscoveryRep
//*****************************************************************************


class CIMServerDiscoveryRep
{
public:

  CIMServerDiscoveryRep();
  ~CIMServerDiscoveryRep();

  Array<CIMServerDescription> lookup(
      const Array<Attribute>& criteria,
      const SLPClientOptions* options = NULL);

};

/** This class provides the interface that a client uses to discover
    CIM Servers.
    ATTN: Add more descriptive material here KS 2 Oct 2003
*/
class PEGASUS_CLIENT_LINKAGE CIMServerDiscovery
{
public:

  /** Constructor for a CIMServerDiscovery object.
  */
  CIMServerDiscovery();

  /** Destructor for a CIMServerDiscovery object
  */
  ~CIMServerDiscovery();

  /** Lookup all WBEM servers
      @return  the return value is an array of connection descriptions.
  */
  Array<CIMServerDescription> lookup(const SLPClientOptions* options = NULL);

  /** Lookup specific WBEM servers
      @param   criteria   Criteria for specifing which CIM Servers to lookup.
      @return  the return value is array of connection descriptions.
  */
  Array<CIMServerDescription> lookup(
      const Array<Attribute>& criteria,
      const SLPClientOptions* options = NULL);

private:

  CIMServerDiscoveryRep* _rep;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMServerDiscovery_h */
