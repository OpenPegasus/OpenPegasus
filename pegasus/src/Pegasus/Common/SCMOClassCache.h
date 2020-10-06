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
// This code implements part of PEP#348 - The CMPI infrastructure using SCMO
// (Single Chunk Memory Objects).
// The design document can be found on the OpenPegasus website openpegasus.org
// at https://collaboration.opengroup.org/pegasus/pp/documents/21210/PEP_348.pdf
//
//%/////////////////////////////////////////////////////////////////////////////
#ifndef _SCMOClassCache_H_
#define _SCMOClassCache_H_

#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/ReadWriteSem.h>
#include <Pegasus/Common/SCMOClass.h>

PEGASUS_NAMESPACE_BEGIN

typedef SCMOClass (*SCMOClassCacheCallbackPtr)(
        const CIMNamespaceName& nameSpace,
        const CIMName& className);

#define PEGASUS_SCMO_CLASS_CACHE_SIZE 32

struct SCMBClassCacheEntry
{
    // Spin-lock to serialize the access to the entry
    AtomicInt lock;
    // The key to identify the entry
    Uint64    key;
    // Pointer to the cached SCMOClass
    SCMOClass* data;
};

class PEGASUS_COMMON_LINKAGE SCMOClassCache
{

public:

    /**
     * This function returns the SCMOClass for the given class name and
     * name space.
     * @param ndName The UTF8 encoded name space. '\0' terminated
     * @param nsNameLan The strlen of ndName ( without '\0')
     * @param className The UTF8 encoded class name. '\0' terminated
     * @param nsNameLan The strlen of className ( without '\0')
     * @return A pointer to SCMOClass. If the class was not found, an empty
     *         SCMOClass is returned.  This can be checked by using the
     *         SCMOClass.isEmpty() method.
     **/
    SCMOClass getSCMOClass(
        const char* nsName,
        Uint32 nsNameLen,
        const char* className,
        Uint32 classNameLen);

    /**
     * Removes the named SCMOClass from the cache.
     * @param cimNameSpace The name space name of the SCMOClass to remove.
     * @param cimClassName The class name of the SCMOClass to remove.
     **/
    void removeSCMOClass(CIMNamespaceName cimNameSpace,CIMName cimClassName);

    /**
     * Clears the whole cache.
     * This should be only done at modification of a class.
     * This may invalidate subclass definitions in the cache.
     * Since class modification is relatively rare, we just flush the entire
     * cache rather than specifically evicting subclass definitions.
     **/
    void clear();

    /**
     * Returns the pointer to an instance of SCMOClassCache.
     */
    static SCMOClassCache* getInstance();

    /**
     * Set the call back function for the SCMOClass to retrieve CIMClasses.
     * @param clb The static call back function.
     */
    void setCallBack(SCMOClassCacheCallbackPtr clb)
    {
       _resolveCallBack = clb;
    }

    static void destroy();

#ifdef PEGASUS_DEBUG
void DisplayCacheStatistics();
#endif

private:

    // Singleton instance pointer
    static SCMOClassCache* _theInstance;

    // The call back function pointer to get CIMClass's
    SCMOClassCacheCallbackPtr _resolveCallBack;

    // The cache array
    SCMBClassCacheEntry _theCache[PEGASUS_SCMO_CLASS_CACHE_SIZE];

    // Lock to prevent parallel modifications of the cache.
    ReadWriteSem _modifyCacheLock;

    // Last successful read index.
    Uint32 _lastSuccessIndex;

    // Last successful written cache index.
    Uint32 _lastWrittenIndex;

    // Counter of used cache entries.
    Uint32 _fillingLevel;

    // Indicator for destruction of the cache.
    Boolean _dying;

#ifdef PEGASUS_DEBUG
    // Statistical data
    Uint32 _cacheReadHit;
    Uint32 _cacheReadMiss;
    Uint32 _cacheRemoveLRU;
    AtomicInt _contentionCount;
#endif

    SCMOClassCache()
        : _resolveCallBack(NULL),
          _lastSuccessIndex(0),
          _lastWrittenIndex(PEGASUS_SCMO_CLASS_CACHE_SIZE-1),
          _fillingLevel(0),
          _dying(false)
    {
        // intialize the the cache
        for (Uint32 i = 0 ; i < PEGASUS_SCMO_CLASS_CACHE_SIZE; i++)
        {
            _theCache[i].data = 0;
            _theCache[i].key = 0;
            // set the lock counter to 1 to allow one next user to enter.
            _theCache[i].lock.set(1);
        }
#ifdef PEGASUS_DEBUG
        // Statistical data
        _cacheReadHit = 0;
        _cacheReadMiss = 0;
        _cacheRemoveLRU = 0;
        _contentionCount.set(0);
#endif

    };

    // clean-up cache data
    ~SCMOClassCache();

    Uint64 _generateKey(
        const char* className,
        Uint32 classNameLen,
        const char* nameSpaceNameLen,
        Uint32 nameSpaceName);

    Boolean _sameSCMOClass(
        const char* nsName,
        Uint32 nsNameLen,
        const char* className,
        Uint32 classNameLen,
        SCMOClass* theClass);

    SCMOClass _addClassToCache(
            const char* nsName,
            Uint32 nsNameLen,
            const char* className,
            Uint32 classNameLen,
            Uint64 theKey);


    /**
     * Get a lock on a cache entry.
     * @return true if the lock was optained
     *         false if the lock was NOT optained, give up !!!
     **/
    Boolean _lockEntry(Uint32 index);

    void _unlockEntry(Uint32 index)
    {
        // set the lock counter to 1 to allow one next user to enter
        // the critical section.
        _theCache[index].lock.set(1);

    };
};

PEGASUS_NAMESPACE_END

#endif
