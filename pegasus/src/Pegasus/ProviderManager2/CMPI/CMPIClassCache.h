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
#ifndef _CMPIClassCache_H_
#define _CMPIClassCache_H_

#include <Pegasus/Common/Config.h>

#ifndef PEGASUS_OS_TYPE_WINDOWS
#include <strings.h>
#endif

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/ReadWriteSem.h>
#include <Pegasus/Common/CharSet.h>
#include <Pegasus/Common/SCMOClass.h>
#include <Pegasus/Common/System.h>

PEGASUS_NAMESPACE_BEGIN

struct CMPI_Broker;

class ClassCacheEntry
{
public:
    ClassCacheEntry(
        const char* namespaceName,
        Uint32 namespaceNameLen,
        const char* className,
        Uint32 classNameLen)
    {
        nsLen = namespaceNameLen;
        clsLen = classNameLen;
        nsName = namespaceName;
        clsName = className;
        allocated = false;
    };

    ClassCacheEntry( const ClassCacheEntry& oldEntry)
    {
        nsLen = oldEntry.nsLen;
        nsName = (char*) malloc(nsLen+1);
        if (0 == nsName)
        {
            throw PEGASUS_STD(bad_alloc)();
        }
        memcpy((void*)nsName, oldEntry.nsName, nsLen+1);

        clsLen = oldEntry.clsLen;
        clsName =  (char*) malloc(clsLen+1);
        if (0 == clsName)
        {
            free((void*)nsName);
            throw PEGASUS_STD(bad_alloc)();
        }
        memcpy((void*)clsName, oldEntry.clsName, clsLen+1);

        allocated = true;
    };

    ~ClassCacheEntry()
    {
        if (allocated)
        {
            free((void*)clsName);
            free((void*)nsName);
        }
    }

    static Boolean equal(const ClassCacheEntry& x, const ClassCacheEntry& y)
    {
        return
            System::strncasecmp(x.clsName,x.clsLen,y.clsName,y.clsLen) &&
            System::strncasecmp(x.nsName,x.nsLen,y.nsName,y.nsLen);
    }

    static Uint32 hash(const ClassCacheEntry& entry)
    {
        // Simply use the lenght of the classname and namespace name as hash.
        return entry.clsLen+entry.nsLen;
    }

private:
    const char* nsName;
    Uint32 nsLen;
    const char* clsName;
    Uint32 clsLen;
    Boolean allocated;

};



class CMPIClassCache
{

public:
    CMPIClassCache()
    {
        _clsCacheSCMO = new ClassCacheSCMO();
    };

    // clean-up cache data
    ~CMPIClassCache();

    // a single function as point of control for now
    // target is to reduce the critical section as much as possible
    SCMOClass* getSCMOClass(
        const CMPI_Broker *mb,
        const char* nsName,
        Uint32 nsNameLen,
        const char* className,
        Uint32 classNameLen);

private:
    typedef HashTable<ClassCacheEntry, SCMOClass *,
        ClassCacheEntry, ClassCacheEntry> ClassCacheSCMO;

    ClassCacheSCMO * _clsCacheSCMO;
    // auto-initialisation due to being on the stack
    ReadWriteSem _rwsemClassCache;
};

PEGASUS_NAMESPACE_END

#endif
