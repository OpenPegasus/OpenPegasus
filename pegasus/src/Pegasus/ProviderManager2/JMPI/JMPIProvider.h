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

#ifndef Pegasus_JMPIProvider_h
#define Pegasus_JMPIProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMOMHandle.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMAssociationProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
#include <Pegasus/Common/AtomicInt.h>

#include <Pegasus/Server/Linkage.h>
#include <Pegasus/ProviderManager2/JMPI/Linkage.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIImpl.h>

PEGASUS_NAMESPACE_BEGIN

class JMPIProviderModule;

struct ProviderVector {
   jclass jProviderClass;
   jobject jProvider;
};

// The JMPIProvider class represents the logical provider extracted from a
// provider module. It is wrapped in a facade to stabalize the interface
// and is directly tied to a module.

class PEGASUS_JMPIPM_LINKAGE JMPIProvider :
                       public virtual CIMProvider
{
public:

    enum Status
    {
        UNKNOWN,
        INITIALIZING,
        INITIALIZED,
        TERMINATING,
        TERMINATED
    };

public:


    class pm_service_op_lock {
    private:
       pm_service_op_lock(void);
    public:
       pm_service_op_lock(JMPIProvider *provider) : _provider(provider)
          { _provider->protect(); }
       ~pm_service_op_lock(void)
          { _provider->unprotect(); }
       JMPIProvider * _provider;
    };

 //  typedef JMPIProviderFacade Base;

    JMPIProvider(const String & name,
        JMPIProviderModule *module,
        ProviderVector *mv);
    JMPIProvider(JMPIProvider*);

    virtual ~JMPIProvider(void);

    virtual void initialize(CIMOMHandle & cimom);

    virtual Boolean tryTerminate(void);
    virtual void terminate(void);
    virtual void _terminate(void);

    Status getStatus(void) const;
    String getName(void) const;

    JMPIProviderModule *getModule(void) const;

//    virtual Boolean unload_ok(void);

//   force provider manager to keep in memory
    virtual void protect(void);
// allow provider manager to unload when idle
    virtual void unprotect(void);

    /**
        Increments the count of current subscriptions for this provider, and
        determines if there were no current subscriptions before the increment.
        If there were no current subscriptions before the increment, the first
        subscription has been created, and the provider's enableIndications
        method should be called.

        @return  True, if before the increment there were no current
                       subscriptions for this provider;
                 False, otherwise
     */
    Boolean testIfZeroAndIncrementSubscriptions ();

    /**
        Decrements the count of current subscriptions for this provider, and
        determines if there are no current subscriptions after the decrement.
        If there are no current subscriptions after the decrement, the last
        subscription has been deleted, and the provider's disableIndications
        method should be called.

        @return  True, if after the decrement there are no current subscriptions
                       for this provider;
                 False, otherwise
     */
    Boolean decrementSubscriptionsAndTestIfZero ();

    /**
        Determines if there are current subscriptions for this provider.

        @return  True, if there is at least one current subscription
                       for this provider;
                 False, otherwise
     */
    Boolean testSubscriptions ();

    /**
        Resets the count of current subscriptions for the indication provider.
     */
    void resetSubscriptions ();

    /**
        Sets the provider instance for the provider.

        Note: the provider instance is set only for an indication provider, and
        is set when a Create Subscription request is processed for the provider.

        @param  instance  the Provider CIMInstance for the provider
     */
    void setProviderInstance (const CIMInstance & instance);

    /**
        Gets the provider instance for the provider.

        Note: the provider instance is set only for an indication provider, and
        only if a Create Subscription request has been processed for the
        provider.

        @return  the Provider CIMInstance for the provider
     */
    CIMInstance getProviderInstance ();

protected:
    Status _status;
    JMPIProviderModule *_module;
    ProviderVector miVector;
    Boolean noUnload;
    CIMClass *cachedClass;

private:
    friend class JMPILocalProviderManager;
    friend class JMPIProviderManager;
    class OpProviderHolder;
    friend class OpProviderHolder;
    mutable Mutex _cimomMutex;
    /* NOTE:  This is a C++ provider only handle which is currently
    **        used for calls to ->getClass ().  getClass returns
    **        immediately with data and is therefore safe to lock.
    */
    CIMOMHandle *_cimom_handle;
    /* NOTE:  This is the java provider's handle and is left
    **        untouched.
    */
    CIMOMHandle *_java_cimom_handle;
    void *jProviderClass,*jProvider;
    String _name;
    AtomicInt _no_unload;
    Uint32 _quantum;
    AtomicInt _current_operations;
    mutable Mutex _statusMutex;

    /**
        Count of current subscriptions for this provider.  Access to this
        data member is controlled by the _currentSubscriptionsLock.
     */
    Uint32 _currentSubscriptions;

    /**
        A mutex to control access to the _currentSubscriptions member variable.
        Before any access (test, increment, decrement or reset) of the
        _currentSubscriptions member variable, the _currentSubscriptionsMutex is
        first locked.
     */
    Mutex _currentSubscriptionsMutex;

    /**
        The Provider CIMInstance for the provider.
        The Provider CIMInstance is set only for indication providers, and only
        if a Create Subscription request has been processed for the provider.
        The Provider CIMInstance is needed in order to construct the
        EnableIndicationsResponseHandler to send to the indication provider
        when the provider's enableIndications() method is called.
        The Provider CIMInstance is needed in the
        EnableIndicationsResponseHandler in order to construct the Process
        Indication request when an indication is delivered by the provider.
        The Provider CIMInstance is needed in the Process Indication request
        to enable the Indication Service to determine if the provider that
        generated the indication accepted a matching subscription.
     */
    CIMInstance _providerInstance;
//};


//
// Used to encapsulate the incrementing/decrementing of the _current_operations
// for a JMPIProvider so it won't be unloaded during operations.
//

   class OpProviderHolder
   {
   private:
       JMPIProvider* _provider;

   public:
       OpProviderHolder(): _provider( NULL )
       {
       }
       OpProviderHolder( const OpProviderHolder& p ): _provider( NULL )
       {
           SetProvider( p._provider );
       }
       OpProviderHolder( JMPIProvider* p ): _provider( NULL )
       {
           SetProvider( p );
       }
       ~OpProviderHolder()
       {
           UnSetProvider();
       }
       JMPIProvider& GetProvider()
       {
           return(*_provider);
       }

       OpProviderHolder& operator=( const OpProviderHolder& x )
       {
           if(this == &x)
               return(*this);
           SetProvider( x._provider );

           return(*this);
       }

       void SetProvider( JMPIProvider* p )
       {
           UnSetProvider();
           if(p)
           {
               _provider = p;
               _provider->_current_operations++;
           }
       }

       void UnSetProvider()
       {
           if(_provider)
           {
               _provider->_current_operations--;
               _provider = NULL;
           }
       }
   };
};

PEGASUS_NAMESPACE_END

#endif
