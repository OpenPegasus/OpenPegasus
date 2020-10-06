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

#include <Pegasus/ProviderManager2/CMPI/CMPIClassCache.h>
#include <Pegasus/ProviderManager2/CMPI/CMPI_Broker.h>
#include <Pegasus/ProviderManager2/CMPI/CMPI_ContextArgs.h>
#include <Pegasus/ProviderManager2/CMPI/CMPI_ThreadContext.h>
#include <Pegasus/Provider/CIMOMHandleRep.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/SCMOClassCache.h>

PEGASUS_NAMESPACE_BEGIN

CMPIClassCache::~CMPIClassCache()
{
    // Cleanup the class cache
    ClassCacheSCMO::Iterator i2=_clsCacheSCMO->start();
    for (; i2; i2++)
    {
        delete i2.value();
    }
    delete _clsCacheSCMO;
}


SCMOClass* CMPIClassCache::getSCMOClass(
    const CMPI_Broker *mb,
    const char* nsName,
    Uint32 nsNameLen,
    const char* className,
    Uint32 classNameLen)
{
    if (!(nsName && className))
    {
        return 0;
    }
    SCMOClass *scmoClass = 0;
    ClassCacheEntry key(nsName,nsNameLen,className,classNameLen);
    {
        ReadLock readLock(_rwsemClassCache);
        if (_clsCacheSCMO->lookup(key,scmoClass))
        {
            return scmoClass;
        }
    }
    try
    {
        WriteLock writeLock(_rwsemClassCache);
        if (_clsCacheSCMO->lookup(key,scmoClass))
        {
            return scmoClass;
        }
        SCMOClassCache* scmoCache = SCMOClassCache::getInstance();

        SCMOClass tmp = scmoCache->getSCMOClass(
            nsName, nsNameLen, className, classNameLen);

        if (tmp.isEmpty())
        {
            // Do not add to cache, when we failed to retrieve a real class
            return 0;
        }

        SCMOClass* scmoClass = new SCMOClass(tmp);
        _clsCacheSCMO->insert(key,scmoClass);
        return scmoClass;
    }
    catch (const CIMException &e)
    {
        PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
            "Exception in CMPIClassCache::getClass() : %s",
            (const char*)e.getMessage().getCString()));
    }
    catch (const Exception &e)
    {
        PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
             "Exception in CMPIClassCache::getClass() : %s",
             (const char*)e.getMessage().getCString()));
    }
    catch (...)
    {
        PEG_TRACE((TRC_CMPIPROVIDERINTERFACE,Tracer::LEVEL1,
            "Unknown Exception in CMPIClassCache::getClass()"));
    }
    return 0;
}

PEGASUS_NAMESPACE_END
