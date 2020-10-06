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

#include <Pegasus/Common/CIMNameCast.h>
#include "IndicationHandlerConstants.h"

PEGASUS_NAMESPACE_BEGIN

#ifdef PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT
const CIMName _PROPERTY_LSTNRDST_NAME =
    CIMNameCast("ListenerDestinationName");

const CIMName _PROPERTY_CREATIONTIME =
    CIMNameCast("CreationTime");

const CIMName _PROPERTY_SEQUENCECONTEXT =
    CIMNameCast("SequenceContext");

const CIMName _PROPERTY_NEXTSEQUENCENUMBER =
    CIMNameCast("NextSequenceNumber");

const CIMName _PROPERTY_MAXQUEUELENGTH =
    CIMNameCast("MaxQueueLength");

const CIMName _PROPERTY_SEQUENCEIDENTIFIERLIFETIME =
    CIMNameCast("SequenceIdentifierLifetime");

const CIMName _PROPERTY_CURRENTINDICATIONS =
    CIMNameCast("CurrentIndications");

const CIMName _PROPERTY_QUEUEFULLDROPPEDINDICATIONS =
    CIMNameCast("QueueFullDroppedIndications");

const CIMName _PROPERTY_LIFETIMEEXPIREDINDICATIONS =
    CIMNameCast("LifetimeExpiredIndications");

const CIMName _PROPERTY_RETRYATTEMPTSEXCEEDEDINDICATIONS =
    CIMNameCast("RetryAttemptsExceededIndications");

const CIMName _PROPERTY_SUBSCRIPTIONDISABLEDROPPEDINDICATIONS =
    CIMNameCast("SubscriptionDisableDroppedIndications");

const CIMName _PROPERTY_LASTSUCCESSFULDELIVERYTIME =
    CIMNameCast("LastSuccessfulDeliveryTime");

const CIMName _PROPERTY_DELIVERY_RETRYATTEMPTS =
    CIMNameCast("DeliveryRetryAttempts");

const CIMName _PROPERTY_DELIVERY_RETRYINTERVAL =
     CIMNameCast("DeliveryRetryInterval");

const CIMName _PROPERTY_SEQUENCENUMBER =
    CIMNameCast("SequenceNumber");

#endif // PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT

PEGASUS_NAMESPACE_END
