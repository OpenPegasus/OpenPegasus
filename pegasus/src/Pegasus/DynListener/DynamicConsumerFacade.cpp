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
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By: Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//              Nitin Upasani, Hewlett-Packard Company (Nitin_Upasani@hp.com)
//              Mike Day, IBM (mdday@us.ibm.com)
//              Yi Zhou, Hewlett-Packard Company (yi_zhou@hp.com)
//              Heather Sterling, IBM (hsterl@us.ibm.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include "DynamicConsumerFacade.h"

#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/MessageLoader.h>


PEGASUS_NAMESPACE_BEGIN

class op_counter
{
public:
    op_counter(AtomicInt *counter)
    : _counter(counter)
    {
        (*_counter)++;
    }
    ~op_counter(void)
    {
        (*_counter)--;
    }
private:
    op_counter(void);
    AtomicInt *_counter;
};


template<class T>
inline T * getInterface(CIMIndicationConsumerProvider* consumer)
{
    T * p = dynamic_cast<T *>(consumer);

    if(p == 0)
    {
        //l10n
        //throw PEGASUS_CIM_EXCEPTION(
        //          CIM_ERR_NOT_SUPPORTED,
        //          "Invalid provider interface.");
        throw PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_NOT_SUPPORTED,
            MessageLoaderParms(
                "ProviderManager.ProviderFacade.INVALID_PROVIDER_INTERFACE",
                "Provider interface is not valid."));
    }

    return(p);
}

DynamicConsumerFacade::DynamicConsumerFacade(
    CIMIndicationConsumerProvider* consumer) : _consumer(consumer)
{
}

DynamicConsumerFacade::~DynamicConsumerFacade(void)
{
}

void DynamicConsumerFacade::initialize(CIMOMHandle & cimom)
{
    _consumer->initialize(cimom);
}

void DynamicConsumerFacade::terminate(void)
{
    _consumer->terminate();
}

// CIMIndicationConsumerProvider interface
void DynamicConsumerFacade::consumeIndication(
    const OperationContext & context,
    const String & destinationPath,
    const CIMInstance & indication)
{
    op_counter ops(&_current_operations);

    CIMIndicationConsumer * consumer = getInterface<CIMIndicationConsumer>
                                           (_consumer);

    consumer->consumeIndication(
        context,
        destinationPath,
        indication);
}

PEGASUS_NAMESPACE_END

