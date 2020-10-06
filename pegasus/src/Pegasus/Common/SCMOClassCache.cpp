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


#include <Pegasus/Common/SCMOClassCache.h>
#include <Pegasus/Common/CIMNameCast.h>

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;

SCMOClassCache* SCMOClassCache::_theInstance = 0;

void SCMOClassCache::destroy()
{
    delete _theInstance;
    _theInstance=0;
}

SCMOClassCache* SCMOClassCache::getInstance()
{
    if(_theInstance == NULL)
    {
        _theInstance = new SCMOClassCache();
    }
    return _theInstance;
}

SCMOClassCache::~SCMOClassCache()
{
    // Signal to all callers and work in progress that the SMOClassCache
    // will be destroyed soon.
    // As from now, no other caller can get the the lock. They are blocked out.
    _dying = true;

    // Cleanup the class cache
    for (Uint32 i = 0 ; i < PEGASUS_SCMO_CLASS_CACHE_SIZE; i++)
    {
        delete _theCache[i].data;
    }
}

Uint64 SCMOClassCache::_generateKey(
    const char* className,
    Uint32 classNameLen,
    const char* nameSpaceName,
    Uint32 nameSpaceNameLen)
{
    Uint64 key = 0;

    key  =  (Uint64(classNameLen) << 48 ) |
            (Uint64(className[0]) << 40 ) |
            (Uint64(className[classNameLen-1]) << 32 ) |
            (Uint64(nameSpaceNameLen) << 16 ) |
            (Uint64(nameSpaceName[0]) << 8 ) |
            Uint64(nameSpaceName[nameSpaceNameLen-1]);

    return key;
}

inline Boolean SCMOClassCache::_lockEntry(Uint32 index)
{
    // The lock is implemented as a spin loop, since the action to
    // verify for the right cache entry is very short.

    if ( _dying )
    {
        // The cache is going to be destroyed.
        // The caller will never get the lock.
        return false;
    }

    while ( true )
    {
        if ( _dying )
        {
            // The cache is going to be destroyed.
            // The caller will never get the lock.
            break;
        }

        // If the lock counter not 1,an other caller is reading the entry.
        if ( _theCache[index].lock.get() == 1 )
        {
            // Decrement the atomic lock counter and test if we do have lock:
            // _theCache[index].lock == 0
            if ( _theCache[index].lock.decAndTestIfZero() )
            {
                // We do have lock!
                return true;
            }
        }
        // I did not get the lock. So signal the scheduer to change the active
        // thread to allow other threads to proceed. This also prevents from
        // looping in a tight loop that causes a dead look due to the
        // lock obtaining thread does not get any time ot finsh his work.
#ifdef PEGASUS_DEBUG
        _contentionCount.inc();
#endif
        Threads::yield();
    }
    return false;
}

inline Boolean SCMOClassCache::_sameSCMOClass(
        const char* nsName,
        Uint32 nsNameLen,
        const char* className,
        Uint32 classNameLen,
        SCMOClass* theClass)
{
    if (_equalNoCaseUTF8Strings(
            theClass->cls.hdr->className,
            theClass->cls.base,
            className,
            classNameLen))
    {
        return _equalNoCaseUTF8Strings(
                theClass->cls.hdr->nameSpace,
                theClass->cls.base,
                nsName,
                nsNameLen);
    }

    return false;
}

SCMOClass SCMOClassCache::_addClassToCache(
        const char* nsName,
        Uint32 nsNameLen,
        const char* className,
        Uint32 classNameLen,
        Uint64 theKey)
{
    WriteLock modifyLock(_modifyCacheLock);

    if ( _dying )
    {
        // The cache is going to be destroyed.
        return SCMOClass();
    }

    Uint32 startIndex =_lastSuccessIndex % PEGASUS_SCMO_CLASS_CACHE_SIZE;
    Uint32 nextIndex = startIndex;
    // The number of used entries is form 0 to PEGASUS_SCMO_CLASS_CACHE_SIZE
    Uint32 usedEntries = _fillingLevel % (PEGASUS_SCMO_CLASS_CACHE_SIZE + 1);

    // This constallation would cause an infitloop below.
    // A miss read of global variables has happen
    if (nextIndex > usedEntries)
    {
        // start from the beginning
        startIndex = 0;
        nextIndex = 0;
    }

    // Check the cache if the class was already added while waiting for the
    // modifyLock.
    //
    // Note: The lock for each cache entry must not be obtained,
    //       because it is used to signal a modify operation, that some
    //       body is reading it. Due to we are the modify task, we know
    //       that we are reading the cache entries!
    //
    for (Uint32 i = 0; i < usedEntries; i++)
    {
        // Does the key match for the entry and the requested class ?
        if (0 != _theCache[nextIndex].key &&
            theKey == _theCache[nextIndex].key)
        {
            // To get sure we found the right class, compare name space
            // and class name.
            if (_sameSCMOClass(nsName,nsNameLen,className,classNameLen,
                               _theCache[nextIndex].data))
            {
                // The entry was added while waiting for the modify lock.
                // The modify lock is destroyed automaticaly !
#ifdef PEGASUS_DEBUG
                _cacheReadHit++;
#endif
                _lastSuccessIndex = nextIndex;
                return SCMOClass(*_theCache[nextIndex].data);
            }
        }

        // go to the next used entries.
        nextIndex = (nextIndex + 1) % usedEntries;
    }

    PEGASUS_ASSERT(_resolveCallBack);

#ifdef PEGASUS_DEBUG
    _cacheReadMiss++;
#endif

    SCMOClass tmp = _resolveCallBack(
         CIMNamespaceNameCast(String(nsName,nsNameLen)),
         CIMNameCast(String(className,classNameLen)));

    if (tmp.isEmpty())
    {
         // The requested class was not found !
         // The modify lock is destroyed automaticaly !
         return SCMOClass();
    }

    SCMOClass* scmoClass = new SCMOClass(tmp);

    _lastWrittenIndex = (_lastWrittenIndex + 1)%PEGASUS_SCMO_CLASS_CACHE_SIZE;

    // Ensure that nobody is reading the enty, so I can write.
    if (_lockEntry(_lastWrittenIndex))
    {
        _theCache[_lastWrittenIndex].key = theKey;

        // If the entry was reused, release old object form the cache.
        if (0 != _theCache[_lastWrittenIndex].data )
        {
#ifdef PEGASUS_DEBUG
            _cacheRemoveLRU++;
#endif
            delete _theCache[_lastWrittenIndex].data;
        }

        _theCache[_lastWrittenIndex].data = scmoClass;

        if (_fillingLevel  < PEGASUS_SCMO_CLASS_CACHE_SIZE)
        {
            _fillingLevel ++;
        }

        _lastSuccessIndex = _lastWrittenIndex;

        _unlockEntry(_lastWrittenIndex);
    }
    else
    {
        // The cache is going to be destroyed.
        // The lock can not be obtained.
        delete scmoClass;
        return SCMOClass();
    }

    // The modify lock is destroyed automaticaly !
    return SCMOClass(*scmoClass);
}

SCMOClass SCMOClassCache::getSCMOClass(
        const char* nsName,
        Uint32 nsNameLen,
        const char* className,
        Uint32 classNameLen)
{
    Uint64 theKey;

    // Due to the _lastSuccessIndex may contain an invalid value,
    // use the modulo to ensure it the index is in a valid range.
    Uint32 startIndex =_lastSuccessIndex % PEGASUS_SCMO_CLASS_CACHE_SIZE;
    Uint32 nextIndex = startIndex;
    // The number of used entries form 0 to PEGASUS_SCMO_CLASS_CACHE_SIZE
    Uint32 usedEntries = _fillingLevel % (PEGASUS_SCMO_CLASS_CACHE_SIZE + 1);

    // This constallation would cause an infitloop below.
    // A miss read of global variables has happen
    if (nextIndex > usedEntries)
    {
        // start from the beginning
        startIndex = 0;
        nextIndex = 0;
    }

    if (nsName && className && nsNameLen && classNameLen)
    {
      theKey = _generateKey(className,classNameLen,nsName,nsNameLen);

      // The counter i is never used as index.
      // It is used to ensure to look through all used entries.
      for (Uint32 i = 0; i < usedEntries; i++)
      {
          if(_lockEntry(nextIndex))
          {
              // does the key match for the entry and the requested class ?
              if ( 0 != _theCache[nextIndex].key &&
                  theKey == _theCache[nextIndex].key)
              {
                  // To get sure we found the right class, compare name space
                  // and class name.
                  if (_sameSCMOClass(nsName,nsNameLen,className,classNameLen,
                                     _theCache[nextIndex].data))
                  {
                      // Yes, we got it !
                      SCMOClass theClass(*_theCache[nextIndex].data);
                      _lastSuccessIndex = nextIndex;
#ifdef PEGASUS_DEBUG
                       _cacheReadHit++;
#endif
                      _unlockEntry(nextIndex);
                      return theClass;
                  }
              }
              // It was the wrong entry, go to the next.
              _unlockEntry(nextIndex);
          }
          else
          {
              // The cache is going to be destroyed.
              // The lock can not be obtained. Give up.
              return SCMOClass();
          }
          // It was the wrong entry, go to the next used entry..
          nextIndex = (nextIndex + 1) % usedEntries;

      }

      // If we end up here, the class is not in the cache !
      // We have to get it from the repositroy and add it into the cache.
      return _addClassToCache(nsName,nsNameLen,className,classNameLen,theKey);
    }

    return SCMOClass();
}

void SCMOClassCache::removeSCMOClass(
    CIMNamespaceName cimNameSpace,
    CIMName cimClassName)
{

    if (cimClassName.isNull() || cimNameSpace.isNull())
    {
        return ;
    }

   CString nsName = cimNameSpace.getString().getCString();
   Uint32 nsNameLen = strlen(nsName);
   CString clsName = cimClassName.getString().getCString();
   Uint32 clsNameLen = strlen(clsName);

   // The number of used entries form 0 to PEGASUS_SCMO_CLASS_CACHE_SIZE
   Uint32 usedEntries = _fillingLevel % (PEGASUS_SCMO_CLASS_CACHE_SIZE + 1);

   Uint64  theKey = _generateKey(clsName,clsNameLen,nsName,nsNameLen);

   // A straight forward loop through all used entries,
   // ignoring the last success.
   for (Uint32 i = 0; i < usedEntries; i++)
   {
       if(_lockEntry(i))
       {
           // does the key match for the entry and the requested class ?
           if ( 0 != _theCache[i].key && theKey == _theCache[i].key)
           {
               // To get sure we found the right class, compare name space
               // and class name.
               if (_sameSCMOClass(nsName,nsNameLen,clsName,clsNameLen,
                                  _theCache[i].data))
               {
                   // Yes, we got it !
                   _theCache[i].key = 0;
                   delete _theCache[i].data;
                   _theCache[i].data = 0;
                   _unlockEntry(i);
                   return;
               }
           }
           // It was the wrong entry, go to the next.
           _unlockEntry(i);
       }
       else
       {
           // We do not get the lock, if the cache is going to be destroyed.
           return;
       }
   }
}

void SCMOClassCache::clear()
{
    WriteLock modifyLock(_modifyCacheLock);

    if ( _dying )
    {
        // The cache is going to be destroyed.
        return ;
    }

    // The number of used entries form 0 to PEGASUS_SCMO_CLASS_CACHE_SIZE
    Uint32 usedEntries = _fillingLevel % (PEGASUS_SCMO_CLASS_CACHE_SIZE + 1);

    // A straight forwar loop through all used entries,
    // ignoring the last success.
    for (Uint32 i = 0; i < usedEntries; i++)
    {
        if(_lockEntry(i))
        {
            _theCache[i].key = 0;
            delete _theCache[i].data;
            _theCache[i].data = 0;
            _unlockEntry(i);
        }
        else
        {
            // We do not get the lock, if the cache is going to be destroyed.
            return;
        }
    }
    // Reset all controll data
    _fillingLevel = 0;
    _lastSuccessIndex = 0;
    _lastWrittenIndex = PEGASUS_SCMO_CLASS_CACHE_SIZE-1;


}
#ifdef PEGASUS_DEBUG
void SCMOClassCache::DisplayCacheStatistics()
{
    PEGASUS_STD(cout) << "SCMOClass Cache Statistics:" <<
        PEGASUS_STD(endl);
    PEGASUS_STD(cout) << "  Size (current/max): " <<
        _fillingLevel << "/" << PEGASUS_SCMO_CLASS_CACHE_SIZE <<
        PEGASUS_STD(endl);
    PEGASUS_STD(cout) << "  Requests satisfied from cache: " <<
        _cacheReadHit << PEGASUS_STD(endl);
    PEGASUS_STD(cout) << "  Requests *not* satisfied from cache: " <<
        _cacheReadMiss << " (implies write to cache)" << PEGASUS_STD(endl);
    PEGASUS_STD(cout) <<
        "  Cache entries \"aged out\" due to cache size constraints: " <<
        _cacheRemoveLRU << PEGASUS_STD(endl);
    PEGASUS_STD(cout) << "  Number of lock contentions: " <<
        _contentionCount.get() << PEGASUS_STD(endl);

}
#endif

PEGASUS_NAMESPACE_END
