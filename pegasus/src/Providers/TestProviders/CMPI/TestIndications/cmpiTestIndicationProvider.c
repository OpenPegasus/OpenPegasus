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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#ifdef PEGASUS_OS_ZOS
#include <pthread.h>
#endif

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Pegasus/Provider/CMPI/cmpimacs.h>
#include <Pegasus/Provider/CMPI/cmpios.h>
#include <Providers/TestProviders/CMPI/TestUtilLib/cmpiUtilLib.h>

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

#define _IndClassName "TestCMPI_Indication"
#define _Namespace    "test/TestProvider"
#define _ProviderLocation \
    "/src/Providers/TestProviders/CMPI/TestIndications/tests/"
#define _LogExtension ".log"

#ifdef CMPI_VER_100
static const CMPIBroker *_broker;
#else
static CMPIBroker *_broker;
#endif
static CMPI_MUTEX_TYPE _mutex;
static CMPI_COND_TYPE _cond;
static int _thread_runs = 0;
static int _thread_active = 0;

/* ---------------------------------------------------------------------------*/
/* private declarations                                                       */
/* ---------------------------------------------------------------------------*/
#if defined(CMPI_PLATFORM_WIN32_IX86_MSVC)

struct timeval
{
    long int tv_sec;
    long int tv_usec;
};

    static void
gettimeofday (struct timeval *t, void *timezone)
{
    struct _timeb timebuffer;
    _ftime (&timebuffer);
    t->tv_sec = (long)timebuffer.time;
    t->tv_usec = 1000 * timebuffer.millitm;
}
#endif
/* ---------------------------------------------------------------------------*/
/*                       CMPI Accessor function                        */
/* ---------------------------------------------------------------------------*/
/*
   CMPIData
   instance_accessor (const char *name, void *param)
   {

   CMPIData data = { 0, CMPI_null, {0} };
   CMPIStatus rc = { CMPI_RC_OK, NULL };
   const CMPIBroker *broker = (CMPIBroker *) param;

   PROV_LOG ("      -- Instance_accessor (Property: %s)", name);

   if (strcmp ("PropertyA", name) == 0)
   {
   data.type = CMPI_string;
   data.state = CMPI_goodValue;
   data.value.string = CMNewString (broker, "AccessorPropertyA", &rc);
   }
   else if (strcmp ("PropertyB", name) == 0)
   {
   data.type = CMPI_string;
   data.state = CMPI_goodValue;
   data.value.string = CMNewString (broker, "AccessorPropertyB", &rc);
   }
   else if (strcmp ("c", name) == 0 && (strlen(name)==1))
   {
   data.type = CMPI_chars;
   data.state = CMPI_goodValue;
   data.value.chars = "c";
   }
   else if (strcmp ("n64", name) == 0)
   {
   data.type = CMPI_uint64;
   data.state = CMPI_goodValue;
   data.value.uint64 = 64;
   }
   else if (strcmp ("n32", name) == 0)
   {
   data.type = CMPI_uint32;
   data.state = CMPI_goodValue;
   data.value.uint64 = 32;
   }
   else if (strcmp ("n16", name) == 0)
   {
   data.type = CMPI_uint16;
   data.state = CMPI_goodValue;
   data.value.uint16 = 16;
   }
   else if (strcmp ("n8", name) == 0)
   {
   data.type = CMPI_uint8;
   data.state = CMPI_goodValue;
   data.value.uint16 = 8;
   }
   else if (strcmp ("s64", name) == 0)
   {
   data.type = CMPI_sint64;
   data.state = CMPI_goodValue;
   data.value.sint64 = 0xFFFFFFFF;
   }
   else if (strcmp ("s32", name) == 0)
   {
   data.type = CMPI_sint32;
   data.state = CMPI_goodValue;
   data.value.sint32 = 0xDEADBEEF;
   }
   else if (strcmp ("s16", name) == 0)
   {
   data.type = CMPI_sint16;
   data.state = CMPI_goodValue;
   data.value.sint16 = 0xFFFF;
   }
else if (strcmp ("s8", name) == 0)
{
    data.type = CMPI_sint8;
    data.state = CMPI_goodValue;
    data.value.sint16 = 0xFF;
}
else if (strcmp ("r64", name) == 0)
{
    data.type = CMPI_real64;
    data.state = CMPI_goodValue;
    data.value.real64 = 3.1415678928283;
}
else if (strcmp ("r32", name) == 0)
{
    data.type = CMPI_real32;
    data.state = CMPI_goodValue;
    data.value.real32 = 1.23;
}

else if (strcmp ("b", name) == 0 && (strlen(name)==1))
{
    data.type = CMPI_boolean;
    data.state = CMPI_goodValue;
    data.value.boolean = 1;
}
else if ((strcmp ("s", name) == 0) && (strlen(name) == 1))
{
    data.type = CMPI_string;
    data.state = CMPI_goodValue;
    data.value.string = CMNewString (broker, "s", &rc);
}
else if ((strcmp("d", name) == 0) && strlen(name) == 1)
{
    data.type = CMPI_dateTime;
    data.state = CMPI_goodValue;
    data.value.dateTime = CMNewDateTime(broker, &rc);
}
check_CMPIStatus(rc);
return data;
}
*/

/* ---------------------------------------------------------------------------*/
/*                       CMPI Helper function                        */
/* ---------------------------------------------------------------------------*/


    static CMPIObjectPath *
make_ObjectPath (const CMPIBroker * broker, const char *ns, const char *clss)
{
    CMPIObjectPath *objPath = NULL;
    CMPIStatus rc = { CMPI_RC_OK, NULL };

    PROV_LOG ("--- make_ObjectPath");
    objPath = CMNewObjectPath (broker, ns, clss, &rc);

    check_CMPIStatus(rc);
    return objPath;
}
    static CMPIInstance *
make_InstanceWithProperties (const CMPIBroker * broker,
        const CMPIObjectPath * objPath)
{
    CMPIInstance *inst = NULL;
    CMPIStatus rc_Inst = { CMPI_RC_OK, NULL };
    CMPIStatus rc_String = { CMPI_RC_OK, NULL };
    CMPIData prop_data = { 0, CMPI_null, {0} };
    CMPIValue       val;
    unsigned int idx;
    CMPIString *name = NULL;
    const char *_name = NULL;
    char * tmpstring;
    PROV_LOG ("-- make_Instance");
    inst = CMNewInstance (broker, objPath, &rc_Inst);

    if (rc_Inst.rc == CMPI_RC_ERR_NOT_FOUND)
    {
        PROV_LOG (" --- Class %s is not found in the %s namespace!",
                _IndClassName, _Namespace);
        PROV_LOG (" --- Aborting!!! ");
        goto exit;
    }

    /*
     * These functions belowe are used to populate the instance
     */
    PROV_LOG ("-- CMGetPropertyCount: %d", CMGetPropertyCount (inst, &rc_Inst));
    check_CMPIStatus (rc_Inst);
    // Set each property.
    for (idx = 0; idx < CMGetPropertyCount (inst, &rc_Inst); idx++)
    {
        check_CMPIStatus (rc_Inst);
        name = NULL;
        prop_data = CMGetPropertyAt (inst, idx, &name, &rc_Inst);
        check_CMPIStatus (rc_Inst);
        if (name)
        {
            _name = CMGetCharsPtr(name, &rc_String);
            check_CMPIStatus(rc_String);
        }
        // Really dumb way of doing it.
        // Just set each property with its own property name.
        if (prop_data.type == CMPI_string)
        {
            CMSetProperty (inst, _name,
                    _name, CMPI_chars);
        }
        else if (prop_data.type == CMPI_char16)
        {
            val.char16 = 'c';
            CMSetProperty (inst, _name,
                    &val, CMPI_char16);
        }
        else if (prop_data.type == CMPI_uint64)
        {
            val.uint64 = 64;
            CMSetProperty(inst, _name,
                    &val, CMPI_uint64);
        }
        else if (prop_data.type == CMPI_uint32)
        {
            val.uint32 = 32;
            CMSetProperty(inst, _name,
                    &val, CMPI_uint32);
        }
        else if (prop_data.type == CMPI_uint16)
        {
            val.uint16 = 16;
            CMSetProperty(inst, _name,
                    &val, CMPI_uint16);
        }
        else if (prop_data.type == CMPI_uint8)
        {
            val.uint8 = 8;
            CMSetProperty(inst, _name,
                    &val, CMPI_uint8);
        }
        else if (prop_data.type == CMPI_sint64)
        {
            val.sint64 = 0xFFFFFFFF;
            CMSetProperty(inst, _name,
                    &val, CMPI_sint64);
        }
        else if (prop_data.type == CMPI_sint32)
        {
            val.sint32 = 0xDEADBEEF;
            CMSetProperty(inst, _name,
                    &val, CMPI_sint32);
        }
        else if (prop_data.type == CMPI_sint16)
        {
            val.sint16 = (CMPISint16)0xFFFF;
            CMSetProperty(inst, _name,
                    &val, CMPI_sint16);
        }
        else if (prop_data.type == CMPI_sint8)
        {
            val.sint8 = (CMPISint8)0xFF;
            CMSetProperty(inst, _name,
                    &val, CMPI_sint8);
        }
        else if (prop_data.type == CMPI_real64)
        {
            val.real64 = 3.1415678928283 ;
            CMSetProperty(inst, _name,
                    &val, CMPI_real64);
        }
        else if (prop_data.type == CMPI_real32)
        {
            val.real32 = 1.23F ;
            CMSetProperty(inst, _name,
                    &val, CMPI_real32);
        }
        else if (prop_data.type == CMPI_dateTime)
        {
            val.dateTime = CMNewDateTime(broker,NULL);

            CMSetProperty (inst, CMGetCharsPtr (name, &rc_String),
                    &val, CMPI_dateTime);

        }
        else if (prop_data.type == CMPI_boolean)
        {
            val.boolean = 1 ;
            CMSetProperty(inst, _name,
                    &val, CMPI_boolean);
        }
        name = NULL;
        prop_data = CMGetPropertyAt (inst, idx, &name, &rc_Inst);
        check_CMPIStatus (rc_Inst);
        tmpstring = _CMPIValueToString (prop_data);
        PROV_LOG ("-- %d: %s(%s: %s: %s) [%s]", idx,
                CMGetCharsPtr (name, &rc_String),
                _CMPITypeName (prop_data.type),
                strCMPIValueState (prop_data.state),
                (prop_data.type == CMPI_dateTime) ? "":
                tmpstring,
                strCMPIStatus (rc_Inst));
        free(tmpstring);

        check_CMPIStatus (rc_Inst);
        check_CMPIStatus (rc_String);
    }
exit:
    return inst;
}
    static void
expand_projection (CMPISelectExp * clone, CMPIArray * projection)
{
    CMPIStatus rc_Array = { CMPI_RC_OK, NULL };
    CMPIStatus rc_String = { CMPI_RC_OK, NULL };
    /* General purpose counters */
    unsigned int idx;
    CMPICount cnt;
    CMPIData data = { 0, CMPI_null, {0} };

    if (clone)
    {
        if (projection)
        {
            PROV_LOG ("---    #A Projection list is: ");
            cnt = CMGetArrayCount (projection, &rc_Array);
            check_CMPIStatus (rc_Array);
            PROV_LOG ("---    #B CMGetArrayCount, %d", cnt);
            for (idx = 0; idx < cnt; idx++)
            {
                PROV_LOG ("---        #C CMGetArrayElementAt");
                data = CMGetArrayElementAt (projection, idx, &rc_Array);
                check_CMPIStatus (rc_Array);
                PROV_LOG ("----       type is : %d", data.type);
                if (data.type == CMPI_string)
                {
                    PROV_LOG ("----       %s (string)",
                            CMGetCharsPtr (data.value.string, &rc_String));
                    check_CMPIStatus (rc_String);
                }
            }
        }
        else
        {
            PROV_LOG (
              "---    #A No projection list, meaning it is SELECT * .... ");
        }
    }
}


    static void
run_test (const CMPIBroker * broker,
        const CMPIContext * ctx,
        const char *query,
        const char *lang,
        const CMPIInstance * inst)
{
    CMPISelectExp *new_se = NULL;
    CMPIArray *projection = NULL;
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIBoolean evalRes = CMPI_false;
    new_se = CMNewSelectExp (broker, query, lang, &projection, &rc);
    check_CMPIStatus (rc);
    if (new_se)
    {
        expand_projection (new_se, projection);
        /* The accessor function and the instance
           should provide the same exact properties */
        evalRes = evaluate (new_se, inst, instance_accessor, (void *)broker);
        if (projection)
        {
            CMRelease (projection);
        }
        CMRelease (new_se);
        new_se = NULL;
    }
    if (evalRes == CMPI_true)
    {
        PROV_LOG("Instance evaluated to true. Delievering indication.");
        rc = CBDeliverIndication(broker,ctx,_Namespace, inst);
        check_CMPIStatus (rc);
    }
}

/* -------------------------------------------------------------------------*/
/*                       Thread function                                    */
/* -------------------------------------------------------------------------*/

    static CMPI_THREAD_RETURN CMPI_THREAD_CDECL
thread (void *args)
{
    CMPIString *str = NULL;
    CMPIString *str_clone = NULL;
    CMPIStatus rc = { CMPI_RC_OK, NULL };
    CMPIStatus rc_String = { CMPI_RC_OK, NULL };
    CMPISelectExp *new_se = NULL;
    CMPIInstance *inst = NULL;
    CMPIObjectPath *objPath = NULL;
    /* General purpose counters */
    /* Thread specific data, passed in via arguments. This could also
       be passed via thread specific data.. */
    void **arguments = (void **) args;
    CMPIContext *ctx;
    CMPISelectExp *se;
    char *ns;
    const char *query = NULL;
    CMPIBroker *broker;

    // Copy over the CMPISelectExp,
    // CMPIContext, CMPIBroker and the ns from the argument.
    se = (CMPISelectExp *) arguments[0];
    ctx = (CMPIContext *) arguments[1];
    broker = (CMPIBroker *) arguments[2];
    ns = (char *) arguments[3];

    free (arguments);

    _thread_active = 1;
    // Get the CMPIContext and attach it to this thread.
    rc = CBAttachThread (broker, ctx);

    // Wait until we get the green signal.
    while (_thread_runs == 0)
    {
        broker->xft->threadSleep (1000);
    }
    check_CMPIStatus (rc);

    //PROV_LOG_OPEN (_IndClassName, _ProviderLocation);

    PROV_LOG ("--- %s CMPI thread(void *) running", _IndClassName);


    // This is how an indication provider would work.
    //  Create an objectpath + instance and use CMEvaluateSelExp
    //  OR
    //  use CMEvaluateSelExpUsingAccessor and pass in the function that would
    // provide the properties values.

    PROV_LOG ("-- #1 MakeObjectPath");
    // Create instance

    objPath = make_ObjectPath (broker, _Namespace, _IndClassName);
    PROV_LOG ("-- #2 MakeInstance");
    inst = make_InstanceWithProperties (broker, objPath);

    if (inst != NULL)
    {
        /*
           This functionality is not used in indication providers,
           but instead in ExecQuery provider API (instance providers).
           But for the sake of completness
           this functionality is also used here. */

        PROV_LOG ("-- #3 CMGetSelExpString");
        str = CMGetSelExpString (se, &rc);
        check_CMPIStatus (rc);
        if (str)
        {
            str_clone = CMClone(str, &rc);
            if (str_clone) CMRelease(str_clone);
            query = CMGetCharsPtr (str, &rc_String);
        }
        PROV_LOG ("-- #3.1 Query is [%s]", query);
        PROV_LOG ("-- #3.2 Class is [%s]", ns);

        PROV_LOG ("-- #4 Clone of CMPISelectExp");
        new_se = CMClone (se, &rc);
        check_CMPIStatus (rc);
        if (new_se)
        {
            CMRelease (new_se);
            new_se = NULL;
        }
        PROV_LOG ("-- #5.0 Evaluate instance with passed in CMPISelectExp ");
        // Evaluate the CMPISelectExp passed in.
        if (evaluate (se, inst, instance_accessor,
                    (void *)broker) == CMPI_true)
        {
            PROV_LOG("Instance evaluated to true. Delievering indication.");
            rc = CBDeliverIndication(broker,ctx,_Namespace, inst);
            check_CMPIStatus (rc);
        }
        // Create a couple of badly written ones.
        PROV_LOG ("-- #5.1 CMNewSelectExp with wrong query (CQL)");
        run_test (broker, ctx, "Wrong query", "DMTF:CQL", inst);

        PROV_LOG ("-- #5.2 CMNewSelectExp with wrong query (WQL)");
        run_test (broker, ctx, "Wrong query", "WQL", inst);

        PROV_LOG ("-- #5.3 CMNewSelectExp with wrong query lang");
        run_test (broker, ctx, query, "Nonexist query language", inst);

        PROV_LOG ("-- #5.5 CMNewSelectExp with WQL");
        run_test (broker, ctx, query, "WQL", inst);

        PROV_LOG ("-- #5.6 CMNewSelectExp with CQL");
        run_test (broker, ctx, query, "DMTF:CQL", inst);
    }
    PROV_LOG ("--- CBDetachThread called");
    rc = CBDetachThread (broker, ctx);
    free (ns);
    PROV_LOG ("---- %s [%s]", strCMPIStatus (rc),
            (rc.msg == 0) ? "" : CMGetCharsPtr (rc.msg, &rc_String));

    PROV_LOG ("--- %s CMPI thread(void *) exited", _IndClassName);

    _thread_active = 0;
    _thread_runs = 0;
    return (CMPI_THREAD_RETURN) 0;
}

/*
 * Initialize function called by the stub macro function.
 */
    void
init ()
{
    _mutex = _broker->xft->newMutex (0);
    _cond = _broker->xft->newCondition (0);
}

    void
waitUntilThreadIsDone ()
{

    struct timespec wait = { 0, 0 };
    struct timeval t;

    while (_thread_runs == 1)
    {
        // Have to wait until the thread is started. On the first
        // run the value is zero, so we skip right through it.
        gettimeofday (&t, NULL);
        // Set the time wait to 1 second.
        wait.tv_sec = t.tv_sec + 1;
        wait.tv_nsec = 0;
        _broker->xft->lockMutex (_mutex);
        // Wait 1 second has expired or the condition has changed.
        _broker->xft->timedCondWait (_cond, _mutex, &wait);
        _broker->xft->unlockMutex (_mutex);
    }
}

/* -------------------------------------------------------------------------*/
/*                       Indication Provider Interface                      */
/* -------------------------------------------------------------------------*/

#ifdef CMPI_VER_100
    CMPIStatus
TestCMPIIndicationProviderIndicationCleanup (CMPIIndicationMI * mi,
        const CMPIContext * ctx,
        CMPIBoolean term)
#else
    CMPIStatus
TestCMPIIndicationProviderIndicationCleanup (CMPIIndicationMI * mi,
        CMPIContext * ctx)
#endif
{

    //PROV_LOG ("--- %s CMPI IndicationCleanup() called", _IndClassName);

    // Release the mutex and condition
    _broker->xft->destroyMutex (_mutex);
    _broker->xft->destroyCondition (_cond);
    _mutex = NULL;
    _cond = NULL;
    //PROV_LOG ("--- %s CMPI IndicationCleanup() exited", _IndClassName);
    CMReturn (CMPI_RC_OK);
}

#ifdef CMPI_VER_100
/* Note: In the CMPI spec the CMPIResult parameter is not passed anymore. */
    CMPIStatus
TestCMPIIndicationProviderAuthorizeFilter (CMPIIndicationMI * mi,
        const CMPIContext * ctx,
        const CMPISelectExp * se,
        const char *ns,
        const CMPIObjectPath * op,
        const char *user)
#else
    CMPIStatus
TestCMPIIndicationProviderAuthorizeFilter (CMPIIndicationMI * mi,
        CMPIContext * ctx,
        CMPIResult * rslt,
        CMPISelectExp * se,
        const char *ns,
        CMPIObjectPath * op,
        const char *user)
#endif
{
    PROV_LOG ("--- %s CMPI AuthorizeFilter() called", _IndClassName);
    /* we don't object */
    if (strcmp (ns, _IndClassName) == 0)
    {
        PROV_LOG ("--- %s Correct class", _IndClassName);
    }
    else
    {
        CMReturn (CMPI_RC_ERR_INVALID_CLASS);
    }
    PROV_LOG ("--- %s CMPI AuthorizeFilter() exited", _IndClassName);

    CMReturn (CMPI_RC_OK);
}

#ifdef CMPI_VER_100
/* Note: In the CMPI spec the CMPIResult parameter is not passed anymore. */
    CMPIStatus
TestCMPIIndicationProviderMustPoll (CMPIIndicationMI * mi,
        const CMPIContext * ctx,
        const CMPISelectExp * se,
        const char *ns, const CMPIObjectPath * op)
#else
    CMPIStatus
TestCMPIIndicationProviderMustPoll (CMPIIndicationMI * mi,
        CMPIContext * ctx,
        CMPIResult * rslt,
        CMPISelectExp * se,
        const char *ns, CMPIObjectPath * op)
#endif
{
    PROV_LOG ("--- %s CMPI MustPoll() called", _IndClassName);

    PROV_LOG ("--- %s CMPI MustPoll() exited", _IndClassName);

    /* no polling */
    CMReturn (CMPI_RC_ERR_NOT_SUPPORTED);
}

#ifdef CMPI_VER_100
/* Note: In the CMPI spec the CMPIResult parameter is not passed anymore. */
    CMPIStatus
TestCMPIIndicationProviderActivateFilter (CMPIIndicationMI * mi,
        const CMPIContext * ctx,
        const CMPISelectExp * se,
        const char *clsName,
        const CMPIObjectPath * op,
        CMPIBoolean firstActivation)
#else
    CMPIStatus
TestCMPIIndicationProviderActivateFilter (CMPIIndicationMI * mi,
        CMPIContext * ctx,
        CMPIResult * rslt,
        CMPISelectExp * se,
        const char *clsName,
        CMPIObjectPath * op,
        CMPIBoolean firstActivation)
#endif
{
    CMPIContext *context;
    //void  **arguments[4];

    const void **arguments = (const void **) malloc (4 * sizeof (char *));

    PROV_LOG_OPEN (_IndClassName, _ProviderLocation);
    PROV_LOG ("--- %s CMPI ActivateFilter() called", _IndClassName);

    // Get a new CMPI_Context which the thread will use.
    context = CBPrepareAttachThread (_broker, ctx);

    // We have to pass in the parameters some way. We are passing the
    // addresses of them  via the void pointer to the thread.
    // This could also be achieved via passing it thread-specific data.
    arguments[0] = se;
    arguments[1] = context;
    arguments[2] = _broker;
    arguments[3] = strdup (clsName);
    // Spawn of a new thread!
    _broker->xft->newThread (thread, (void *) arguments, 0);

    PROV_LOG ("--- %s CMPI ActivateFilter() exited", _IndClassName);
    CMReturn (CMPI_RC_OK);
}

#ifdef CMPI_VER_100
/* Note: In the CMPI spec the CMPIResult parameter is not passed anymore. */
    CMPIStatus
TestCMPIIndicationProviderDeActivateFilter (CMPIIndicationMI * mi,
        const CMPIContext * ctx,
        const CMPISelectExp * se,
        const char *clsName,
        const CMPIObjectPath * op,
        CMPIBoolean lastActivation)
#else
    CMPIStatus
TestCMPIIndicationProviderDeActivateFilter (CMPIIndicationMI * mi,
        CMPIContext * ctx,
        CMPIResult * rslt,
        CMPISelectExp * se,
        const char *clsName,
        CMPIObjectPath * op,
        CMPIBoolean lastActivation)
#endif
{
    waitUntilThreadIsDone ();
    PROV_LOG ("--- %s CMPI DeActivateFilter() entered", _IndClassName);
    PROV_LOG ("--- %s CMPI DeActivateFilter() exited", _IndClassName);
    CMReturn (CMPI_RC_OK);
}

#ifdef CMPI_VER_100
    CMPIStatus
TestCMPIIndicationProviderEnableIndications (CMPIIndicationMI * mi,
        const CMPIContext * ctx)
#else
    CMPIStatus
TestCMPIIndicationProviderEnableIndications (CMPIIndicationMI * mi)
#endif
{
    PROV_LOG ("--- %s CMPI EnableIndication() entered", _IndClassName);
    _broker->xft->lockMutex (_mutex);
    _thread_runs = 1;
    _broker->xft->unlockMutex (_mutex);

    waitUntilThreadIsDone ();
    PROV_LOG ("--- %s CMPI EnableIndication() exited", _IndClassName);
    CMReturn (CMPI_RC_OK);
}

#ifdef CMPI_VER_100
    CMPIStatus
TestCMPIIndicationProviderDisableIndications (CMPIIndicationMI * mi,
        const CMPIContext * ctx)
#else
    CMPIStatus
TestCMPIIndicationProviderDisableIndications (CMPIIndicationMI * mi)
#endif
{
    PROV_LOG ("--- %s CMPI DisableIndication() entered", _IndClassName);

    _thread_runs = 0;
    waitUntilThreadIsDone ();
    PROV_LOG ("--- %s CMPI DisableIndication() exited", _IndClassName);
    PROV_LOG_CLOSE ();
    CMReturn (CMPI_RC_OK);
}

/* -------------------------------------------------------------------------*/
/*                              Provider Factory                            */
/* -------------------------------------------------------------------------*/

/* Note that we call 'init()' function
*/
CMIndicationMIStub (TestCMPIIndicationProvider,
        TestCMPIIndicationProvider, _broker, init ())
    /* ---------------------------------------------------------------------*/
    /*             end of TestCMPIProvider                                  */
    /* ---------------------------------------------------------------------*/
