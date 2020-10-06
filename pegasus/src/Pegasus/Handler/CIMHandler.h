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

#ifndef PegasusHandler_Handler_h
#define PegasusHandler_Handler_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Handler/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
 * Indication export connection interface for the handlers.
 */

class PEGASUS_HANDLER_LINKAGE IndicationExportConnection
{
public:
    IndicationExportConnection() { };

    virtual ~IndicationExportConnection() { };
};

class PEGASUS_HANDLER_LINKAGE CIMHandler
{
public:

    CIMHandler() { };

    virtual ~CIMHandler() { };

    // Abstract method which each and every handler needs to be implemented.
    // Indication processing will instantiate IndicationDispatcher which
    // will look into Handler Table to load the appropriate handler. If
    // handler is already loaded then IndicationDispatcher will call this
    // method implemented in handler.
    virtual void handleIndication(
        const OperationContext& context,
        const String nameSpace,
        CIMInstance& indicationInstance,
        CIMInstance& indicationHandlerInstance,
        CIMInstance& indicationSubscriptionInstance,
        ContentLanguageList& contentLanguages) = 0;

    // IndicationExportConnection object is returned if requested incase of
    // successful indication delivery. Connection object SHOULD not be
    // returned incase of delivery failure.
    virtual void handleIndication(
        const OperationContext& context,
        const String &nameSpace,
        CIMInstance& indicationInstance,
        CIMInstance& indicationHandlerInstance,
        CIMInstance& indicationSubscriptionInstance,
        ContentLanguageList& contentLanguages,
        IndicationExportConnection **connection)
    {
        if (connection)
        {
            *connection = 0;
        }
        handleIndication(
            context,
            nameSpace,
            indicationInstance,
            indicationHandlerInstance,
            indicationSubscriptionInstance,
            contentLanguages);
    }

    // These are the method to initialize and terminate handler. Actual need
    // and implementation way these methods are yet to be finalized.

    virtual void initialize(CIMRepository* repository) = 0;
    virtual void terminate() = 0;
};

PEGASUS_NAMESPACE_END

#endif /* PegasusHandler_Handler_h */
