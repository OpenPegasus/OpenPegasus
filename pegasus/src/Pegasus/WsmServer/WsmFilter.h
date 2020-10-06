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

#ifndef Pegasus_WsmFilter_h
#define Pegasus_WsmFilter_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/SharedPtr.h>
#include <Pegasus/WsmServer/WsmInstance.h>
#include <Pegasus/WsmServer/WsmSelectorSet.h>
#include <Pegasus/WsmServer/WsmEndpointReference.h>
#include <Pegasus/WsmServer/WsmUtils.h>
#include <Pegasus/WQL/WQLParser.h>

/*
    Define the enumeration Filter Dialects and information associated
    with each filter dialect in a single place.
*/

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_WSMSERVER_LINKAGE WsmFilter
{
public:
    // Define each of the implemented Filter dialects. NONE is the
    // default where there is no filter
    enum wsmFilterDialect
    {
        NONE,                   // No Filter
        WQL,                    // Microsoft defined WQL filter.
        ASSOCIATION             // Association filter per DSP0227 sect 8.2
    };

    // Define possible association filter types. Since there are
    // only two types and no possibility of error, only the two types
    // are defined. ASSOCIATED_INSTANCES corresponds to CIM/XML
    // associatiors operation and ASSOCIATION_INSTANCES to references operation
    enum AssocFilterType
    {
        ASSOCIATED_INSTANCES,
        ASSOCIATION_INSTANCES
    };

    WsmFilter();

    ~WsmFilter();

    // Dialect for this filter. The legal dialects are defined by
    // the wsmFilterDialect enum (ex. WQL and Association
    wsmFilterDialect filterDialect;

    // Parameters for WQL filter dialect
    struct
    {
        String queryLanguage;
        String query;
        SharedPtr<WQLSelectStatement> selectStatement;
    } WQLFilter;

    // Parameters for AssociationFilter dialect defined in DSP0227 Section 8.2.
    // AssociatedInstances and associationInstances use this
    // common structure.
    struct
    {
        // Type of association filter received,
        AssocFilterType assocFilterType;

        // object reference for the association call.  Must be an object
        // path (not just class) and include the namespace element
        WsmEndpointReference object;
        CIMName assocClassName;
        CIMName resultClassName;
        String role;
        String resultRole;

        // FUTURE: Only needed when we support fragments. Until then
        // this will be rejected.
        CIMPropertyList propertyList;
    }AssocFilter;

private:
    // Copy and assign not used.
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_WsmFilter_h */
