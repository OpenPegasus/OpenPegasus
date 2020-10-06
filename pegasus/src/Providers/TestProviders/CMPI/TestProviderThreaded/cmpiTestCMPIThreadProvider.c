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


#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>

#if defined(CMPI_PLATFORM_WIN32_IX86_MSVC)
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#else
#if defined(CMPI_PLATFORM_AIX_RS_IBMCXX) \
    || defined(CMPI_PLATFORM_PASE_ISERIES_IBMCXX)
#include <time.h>
#endif
#include <sys/time.h>
#endif
#if defined(CMPI_PLATFORM_ZOS_ZSERIES_IBM)
#include <pthread.h>
#endif

/* ---------------------------------------------------------------------------*/
/* private declarations                                                       */
/* ---------------------------------------------------------------------------*/
#if defined(CMPI_PLATFORM_WIN32_IX86_MSVC)

struct timeval
{
  long int tv_sec;
  long int tv_usec;
};

void
gettimeofday (struct timeval *t, void *timezone)
{
  struct _timeb timebuffer;
  _ftime (&timebuffer);
  t->tv_sec = (long)timebuffer.time;
  t->tv_usec = 1000 * timebuffer.millitm;
}
#endif

static const CMPIBroker *_broker;
static int exitNow = 0;
static int threads = 0;

static CMPI_MUTEX_TYPE threadCntMutex;
static CMPI_THREAD_TYPE exitThr;

/* ---------------------------------------------------------------------------*/
/*                              Threads                                       */
/* ---------------------------------------------------------------------------*/

#if !defined(PEGASUS_DEBUG)
    static CMPI_THREAD_RETURN CMPI_THREAD_CDECL
bad_thread (void *args)
{
  // Loop forever...  Note that this point will
  // be cancelled only when the provider is unloaded. This can happend
  // during unload or cimserver shutdown.
  while (1)
    {
      // Call the cancellation point.

      _broker->xft->exitThread (0);
    }
  return (CMPI_THREAD_RETURN) 0;

}
#endif

static CMPI_THREAD_RETURN CMPI_THREAD_CDECL testExitThread (void *args)
{
  _broker->xft->exitThread(0);
  return (CMPI_THREAD_RETURN) 0;
}

static CMPI_THREAD_RETURN CMPI_THREAD_CDECL
good_thread (void *args)
{
  _broker->xft->lockMutex (threadCntMutex);
  threads++;
  _broker->xft->unlockMutex (threadCntMutex);
  while (exitNow == 0)
    {
      _broker->xft->threadSleep (25);
    }
  _broker->xft->lockMutex (threadCntMutex);
  threads--;
  _broker->xft->unlockMutex (threadCntMutex);
  return (CMPI_THREAD_RETURN) 0;

}
static CMPI_THREAD_RETURN CMPI_THREAD_CDECL
empty_thread (void *args)
{
  // Do nothing.
  return (CMPI_THREAD_RETURN) 0;
}

/* ---------------------------------------------------------------------------*/
/*                           Thread managament functions                      */
/* ---------------------------------------------------------------------------*/
int getThreadCount ()
{
  int threadCount = 0;
  _broker->xft->lockMutex (threadCntMutex);
   threadCount = threads;
  _broker->xft->unlockMutex (threadCntMutex);

  return threadCount;
}

void
initThreads ()
{

  threadCntMutex = _broker->xft->newMutex (0);

  // Spawn of a couple of threads
  _broker->xft->newThread (good_thread, NULL, 0);
  _broker->xft->newThread (good_thread, NULL, 0);
  _broker->xft->newThread (good_thread, NULL, 0);
  while(getThreadCount() < 3)
  {
    _broker->xft->threadSleep (25);
  }
// Only enable when the PEGASUS_DEBUG flag is not set. The
// PEGASUS_DEBUG is mostly used by developers, and when this
// flag is disabled the CIMServer kills the threads.
#if !defined(PEGASUS_DEBUG)
  _broker->xft->newThread (bad_thread, NULL, 0);
#endif
  _broker->xft->newThread (empty_thread, NULL, 0);

  exitThr = _broker->xft->newThread(testExitThread, NULL, 0);
}

void
deleteThreads ()
{
  CMPI_MUTEX_TYPE _mutex;
  CMPI_COND_TYPE _cond;
  struct timespec wait = { 0, 0 };
  struct timeval t;

  _mutex = _broker->xft->newMutex (0);
  _cond = _broker->xft->newCondition (0);

  exitNow = 1;

  // Wait until the number of good threads reaches zero.
  while (getThreadCount() != 0)
    {
      gettimeofday (&t, NULL);
      // Set the time wait to 2 seconds.
      wait.tv_sec = t.tv_sec + 2;
      wait.tv_nsec = 0;
      _broker->xft->lockMutex (_mutex);
      // Wait 1 second has expired or the condition has changed.
      _broker->xft->timedCondWait (_cond, _mutex, &wait);
      _broker->xft->unlockMutex (_mutex);
    }

  // Make sure to de-allocate the mutexes and conditions.
  _broker->xft->destroyMutex (_mutex);
  _broker->xft->destroyCondition (_cond);

  // join the exitThread
  if (exitThr)
  {
      _broker->xft->joinThread(exitThr, 0);
  }
}

CMPIStatus
TestCMPIThreadProviderCleanup (CMPIInstanceMI * mi, const CMPIContext * ctx,
                               CMPIBoolean term)
{

  deleteThreads ();
  _broker->xft->destroyMutex (threadCntMutex);
  CMReturn (CMPI_RC_OK);
}

CMPIStatus
TestCMPIThreadProviderEnumInstanceNames (CMPIInstanceMI * mi,
                                         const CMPIContext * ctx,
                                         const CMPIResult * rslt,
                                         const CMPIObjectPath * ref)
{
  CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus
TestCMPIThreadProviderEnumInstances (CMPIInstanceMI * mi,
                                     const CMPIContext * ctx,
                                     const CMPIResult * rslt,
                                     const CMPIObjectPath * ref,
                                     const char **properties)
{
  CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus
TestCMPIThreadProviderGetInstance (CMPIInstanceMI * mi,
                                   const CMPIContext * ctx,
                                   const CMPIResult * rslt,
                                   const CMPIObjectPath * cop,
                                   const char **properties)
{
  CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus
TestCMPIThreadProviderCreateInstance (CMPIInstanceMI * mi,
                                      const CMPIContext * ctx,
                                      const CMPIResult * rslt,
                                      const CMPIObjectPath * cop,
                                      const CMPIInstance * ci)
{
  CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus
TestCMPIThreadProviderModifyInstance (CMPIInstanceMI * mi,
                                      const CMPIContext * ctx,
                                      const CMPIResult * rslt,
                                      const CMPIObjectPath * cop,
                                      const CMPIInstance * ci,
                                      const char **properties)
{
  CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

CMPIStatus
TestCMPIThreadProviderDeleteInstance (CMPIInstanceMI * mi,
                                      const CMPIContext * ctx,
                                      const CMPIResult * rslt,
                                      const CMPIObjectPath * cop)
{
  CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

/*
CMPIStatus
TestCMPIThreadProviderExecQuery (CMPIInstanceMI * mi,
                                 const CMPIContext * ctx,
                                 const CMPIResult * rslt,
                                 const CMPIObjectPath * ref,
                                 const char *lang, const char *query)
{
  CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

*/

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/
/*
CMInstanceMIStub (TestCMPIThreadProvider,
                  TestCMPIThreadProvider, _broker, initThreads());
*/

static CMPIInstanceMIFT instMIFT__ =
  { 100, 100, "instance" "TestCMPIThreadProvider",
  TestCMPIThreadProviderCleanup,
  TestCMPIThreadProviderEnumInstanceNames,
  TestCMPIThreadProviderEnumInstances,
  TestCMPIThreadProviderGetInstance,
  TestCMPIThreadProviderCreateInstance,
  TestCMPIThreadProviderModifyInstance,
  TestCMPIThreadProviderDeleteInstance,
  NULL,                         //TestCMPIThreadProviderExecQuery,
};
CMPI_EXTERN_C CMPIInstanceMI *
TestCMPIThreadProvider_Create_InstanceMI (const CMPIBroker * brkr,
                                          const CMPIContext * ctx,
                                          CMPIStatus * rc)
{
  static CMPIInstanceMI mi = { ((void *) 0), &instMIFT__, };
  _broker = brkr;
  initThreads ();
  return &mi;
}

/* ---------------------------------------------------------------------------*/
/*             end of TestCMPIProvider                      */
/* ---------------------------------------------------------------------------*/
