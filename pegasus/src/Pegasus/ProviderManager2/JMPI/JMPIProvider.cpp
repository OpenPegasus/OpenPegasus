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

#include "JMPIProvider.h"

#include <Pegasus/Common/Tracer.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIProvider.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIProviderModule.h>

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

#include "Convert.h"

// set current operations to 1 to prevent an unload
// until the provider has had a chance to initialize
JMPIProvider::JMPIProvider (const String       &name,
                            JMPIProviderModule *module,
                            ProviderVector     *mv)
{
   PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "JMPIProvider::JMPIProvider(name, module, mv)");

   _module               = module;
   _cimom_handle         = 0;
   _java_cimom_handle    = new CIMOMHandle ();
   _name                 = name;
   _no_unload            = false;
   _current_operations   = 1;
   _currentSubscriptions = 0;
   miVector              = *mv;
   jProvider             = mv->jProvider;
   jProviderClass        = mv->jProviderClass;
   noUnload              = false;
   cachedClass           = NULL;

   PEG_METHOD_EXIT();
}

JMPIProvider::JMPIProvider (JMPIProvider *pr)
{
   PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "JMPIProvider::JMPIProvider(pr)");

   _module               = pr->_module;
   _cimom_handle         = 0;
   _java_cimom_handle    = new CIMOMHandle ();
   _name                 = pr->_name;
   _no_unload            = pr->noUnload;
   _current_operations   = 1;
   _currentSubscriptions = 0;
   miVector              = pr->miVector;
   noUnload              = pr->noUnload;
   cachedClass           = NULL;

   PEG_METHOD_EXIT();
}

JMPIProvider::~JMPIProvider(void)
{
   PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "JMPIProvider::~JMPIProvider");

   delete _java_cimom_handle;
   delete cachedClass;

   PEG_METHOD_EXIT();
}

JMPIProvider::Status JMPIProvider::getStatus(void) const
{
    AutoMutex lock(_statusMutex);
    return(_status);
}

JMPIProviderModule *JMPIProvider::getModule(void) const
{
    return(_module);
}

String JMPIProvider::getName(void) const
{
    return(_name);
}

void JMPIProvider::initialize(CIMOMHandle& cimom)
{
    _status       = INITIALIZING;
    _cimom_handle = &cimom;

    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "JMPIProvider::Initialize");

    JvmVector *jv  = 0;
    JNIEnv    *env = JMPIjvm::attachThread(&jv);

    if (!env)
    {
       PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL1,
           "JMPIProvider:"
               "Could not initialize the JVM (Java Virtual Machine) "
               "runtime environment.");

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(
            CIM_ERR_FAILED,
            MessageLoaderParms(
                "ProviderManager.JMPI.JMPIProvider.INIT_JVM_FAILED",
                "Could not initialize the JVM (Java Virtual Machine)"
                    " runtime environment."));
    }

    // public abstract void initialize (org.pegasus.jmpi.CIMOMHandle ch)
    //        throws org.pegasus.jmpi.CIMException
    jmethodID id = env->GetMethodID((jclass)jProviderClass,
                                    "initialize",
                                    "(Lorg/pegasus/jmpi/CIMOMHandle;)V");

    PEG_TRACE(( TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "JVM id = %X",(long)id));

    JMPIjvm::checkException(env);

    if (id != NULL)
    {
       jstring jName = env->NewStringUTF(_name.getCString());

       JMPIjvm::checkException(env);

       jlong jCimomRef = DEBUG_ConvertCToJava(
                             CIMOMHandle*,
                             jlong,
                             _java_cimom_handle);
       jobject jch       = env->NewObject(jv->CIMOMHandleClassRef,
                                          JMPIjvm::jv.CIMOMHandleNewJSt,
                                          jCimomRef,
                                          jName);

       JMPIjvm::checkException(env);

       env->CallVoidMethod((jobject)jProvider,id,jch);

       JMPIjvm::checkException(env);
    }
    env->ExceptionClear();

    JMPIjvm::detachThread();

    _status             = INITIALIZED;
    _current_operations = 0;

    PEG_METHOD_EXIT();
    return;
}

Boolean JMPIProvider::tryTerminate(void)
{
    return false;
}

void JMPIProvider::_terminate(void)
{
}


void JMPIProvider::terminate(void)
{
}

/*
Boolean JMPIProvider::unload_ok(void)
{
   return false;
}
*/
//   force provider manager to keep in memory
void JMPIProvider::protect(void)
{
  // _no_unload++;
}

// allow provider manager to unload when idle
void JMPIProvider::unprotect(void)
{
  // _no_unload--;
}

Boolean JMPIProvider::testIfZeroAndIncrementSubscriptions ()
{
    AutoMutex lock (_currentSubscriptionsMutex);
    Boolean isZero = (_currentSubscriptions == 0);
    _currentSubscriptions++;

    return isZero;
}

Boolean JMPIProvider::decrementSubscriptionsAndTestIfZero ()
{
    AutoMutex lock (_currentSubscriptionsMutex);
    _currentSubscriptions--;
    Boolean isZero = (_currentSubscriptions == 0);

    return isZero;
}

Boolean JMPIProvider::testSubscriptions ()
{
    AutoMutex lock (_currentSubscriptionsMutex);
    Boolean currentSubscriptions = (_currentSubscriptions > 0);

    return currentSubscriptions;
}

void JMPIProvider::resetSubscriptions ()
{
    AutoMutex lock (_currentSubscriptionsMutex);
    _currentSubscriptions = 0;
}

void JMPIProvider::setProviderInstance (const CIMInstance & instance)
{
    _providerInstance = instance;
}

CIMInstance JMPIProvider::getProviderInstance ()
{
    return _providerInstance;
}

PEGASUS_NAMESPACE_END
