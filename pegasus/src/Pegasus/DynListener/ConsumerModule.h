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

#ifndef Pegasus_ConsumerModule_h
#define Pegasus_ConsumerModule_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/AtomicInt.h>

#include <Pegasus/General/DynamicLibrary.h>

#include <Pegasus/Provider/CIMProvider.h>
#include <Pegasus/Provider/CIMIndicationConsumerProvider.h>

#include <Pegasus/DynListener/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

// The ConsumerModule class represents the physical module, as defined by the
// operation, that contains a provider. This class effectively encapsulates the
// "physical" portion of a consumer.
//
// There can be multiple "logical" consumers in memory,
// but there is only one "physical" consumer.
// This class keeps track of how many logical consumers are using it.


class PEGASUS_DYNLISTENER_LINKAGE ConsumerModule
{
    friend class ConsumerManager;

public:
    virtual ~ConsumerModule(void);
    const String & getFileName(void) const;
    CIMIndicationConsumerProvider* load(
        const String & consumerName,
        const String & libraryPath);
    void unloadModule(void);

protected:
    DynamicLibrary _library;

private:
    ConsumerModule();
    const String & getConsumerName(void) const;
    virtual CIMIndicationConsumerProvider* getConsumer(void) const;

    String _consumerName;
    CIMIndicationConsumerProvider* _consumer;
};

inline const String & ConsumerModule::getFileName(void) const
{
    return _library.getFileName();
}

inline const String & ConsumerModule::getConsumerName(void) const
{
    return(_consumerName);
}

inline CIMIndicationConsumerProvider* ConsumerModule::getConsumer(void) const
{
    return(_consumer);
}

PEGASUS_NAMESPACE_END

#endif
