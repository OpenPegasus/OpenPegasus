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
#include "JMPILocalProviderManager.h"

#include <Pegasus/Common/Time.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <Pegasus/ProviderManager2/ProviderManager.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIProvider.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIProviderModule.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

JMPILocalProviderManager::JMPILocalProviderManager(void)
    : _idle_timeout(PEGASUS_PROVIDER_IDLE_TIMEOUT_SECONDS)
{
}

JMPILocalProviderManager::~JMPILocalProviderManager(void)
{
    Uint32 ccode;

    _provider_ctrl(UNLOAD_ALL_PROVIDERS, this, &ccode);
}

Sint32 JMPILocalProviderManager::_provider_ctrl(
    CTRL code,
    void *parm,
    void *ret)
{
    static Uint32 quantum;
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
         "JMPILocalProviderManager::_provider_ctrl");

    Sint32 ccode = 0;
    CTRL_STRINGS *parms = reinterpret_cast<CTRL_STRINGS *>(parm);

    switch(code)
    {

    case GET_PROVIDER:
        {
            String providerName = *(parms->providerName);
            String moduleFileName = *(parms->fileName);
            String interfaceName = *(parms->interfaceName);

            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                 "GET_PROVIDER providerName = %s, "
                     "moduleFileName = %s , interfaceName = %s",
                 (const char*)providerName.getCString(),
                 (const char*)moduleFileName.getCString(),
                 (const char*)interfaceName.getCString()));

            if (providerName.size () == 0)
            {
               PEG_METHOD_EXIT();
               throw NullPointer();
            }

            JMPIProvider::OpProviderHolder* ph =
               reinterpret_cast< JMPIProvider::OpProviderHolder* >( ret );
            JMPIProviderModule *newModule = NULL;
            JMPIProviderModule *module = NULL;
            JMPIProvider *newProvider = NULL;
            JMPIProvider *provider = NULL;
            ProviderVector base;

            try
            {
               AutoMutex lock (_providerTableMutex);

               if (true == _providers.lookup(providerName, provider))
               {
                   PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                        "Found JMPIProvider %s in JMPIProvider Manager Cache",
                        (const char*)providerName.getCString()));
                   PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                        "SetProvider(%p) at provider holder.",provider));

                   ph->SetProvider(provider);

///////////////////ph->GetProvider().update_idle_timer();
                   break;
               }

               PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                    "Creating JMPIProvider %s",
                    (const char*)providerName.getCString()));

               if (false == _modules.lookup(moduleFileName, module))
               {
                   PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                        "Creating JMPIProvider Module %s",
                        (const char*)moduleFileName.getCString()));

                   newModule = new JMPIProviderModule(moduleFileName,
                                                      interfaceName);
                   if (0 == newModule)
                   {
                       PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                            "New JMPIProviderModule is NULL!");
                       PEG_METHOD_EXIT();
                       throw NullPointer();
                   }

                   module = newModule;

                   _modules.insert(moduleFileName, module);
               }
               else
               {
                   PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                        "Using Cached JMPIProvider Module %s",
                        (const char*)moduleFileName.getCString()));
               }

               PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                    "Loading/Linking JMPIProvider Module %s",
                    (const char*)moduleFileName.getCString()));

               try
               {
                   base = module->load(moduleFileName);
               }
               catch(...)
               {
                   PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                                    "Exception caught Loading/Linking"
                            "JMPIProvider Module %s",
                        (const char*)moduleFileName.getCString()));
                   PEG_METHOD_EXIT();
                   throw;
               }

               // create provider module
               newProvider = new JMPIProvider(providerName, module, &base);
               if (0 == newProvider)
               {
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                                    "New JMPIProvider is NULL!");
                   PEG_METHOD_EXIT();
                   throw NullPointer();
               }

               provider = newProvider;

               if (0 == (provider->_cimom_handle = new CIMOMHandle()))
               {
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                                    "CIMOMhandle is NULL!");
                   PEG_METHOD_EXIT();
                   throw NullPointer();
               }

               provider->_quantum = 0;

               {
                  AutoMutex lock(provider->_statusMutex);

                  PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                       "Loading JMPIProvider %s",
                       (const char*)provider->_name.getCString()));
                  try
                  {
                      provider->initialize(*(provider->_cimom_handle));
                  }
                  catch(CIMException &cimExcept)
                  {
                      PEG_TRACE((TRC_DISCARDED_DATA,Tracer::LEVEL1,
                          "Calling provider->initialize caused exception: %s",
                          (const char*)cimExcept.getMessage().getCString()));

                      throw;
                  }
                  catch(Exception &except)
                  {
                      PEG_TRACE((TRC_DISCARDED_DATA,Tracer::LEVEL1,
                          "Calling provider->initialize caused exception: %s",
                          (const char*)except.getMessage().getCString()));

                      throw PEGASUS_CIM_EXCEPTION(CIM_ERR_FAILED,
                                                  except.getMessage());
                  }
                  catch(...)
                  {
                      PEG_TRACE_CSTRING(TRC_DISCARDED_DATA,Tracer::LEVEL1,
                                       "Exception caught calling initialize!");

                      throw UninitializedObjectException();
                  }
               }

///////////////provider->update_idle_timer();

               _providers.insert(providerName, provider);
            }
            catch (...)
            {
               if (newModule)
               {
                  _modules.remove(moduleFileName);
               }

               delete newModule;

               if (newProvider)
               {
                  delete newProvider->_cimom_handle;
               }

               delete newProvider;
               PEG_METHOD_EXIT();
               throw;
            }

            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                 "SetProvider(%p) at provider holder.",provider));

            ph->SetProvider( provider );
            break;
        }

    case UNLOAD_PROVIDER:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                "UNLOAD_PROVIDER");
            break;
        }

    case LOOKUP_PROVIDER:
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                 "LOOKUP_PROVIDER providerName = %s ",
                 (const char*)*(parms->providerName)->getCString()));

            AutoMutex lock (_providerTableMutex);

            if (true == _providers.lookup(*(parms->providerName),
                                   *(reinterpret_cast<JMPIProvider * *>(ret))))
            {
                PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                     "Found JMPIProvider %s in JMPIProvider Manager Cache",
                     (const char*)*(parms->providerName)->getCString()));
            }
            else
            {
                PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL2,
                     "Could not find JMPIProvider %s in "
                         "JMPIProvider Manager Cache",
                     (const char*)*(parms->providerName)->getCString()));
                ccode = -1;
            }

            break;
        }

    case LOOKUP_MODULE:
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                 "LOOKUP_MODULE moduleName = %s ",
                 (const char*)*(parms->fileName)->getCString()));

            AutoMutex lock (_providerTableMutex);

            if (false  == _modules.lookup(*(parms->fileName),
                *(reinterpret_cast<JMPIProviderModule * *>(ret))))
            {
                PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL2,
                     "Could not find JMPIProvider Module %s in "
                         "JMPIProvider Manager Cache",
                     (const char*)*(parms->fileName)->getCString()));
                ccode = -1;
            }

            break;
        }

    case INSERT_PROVIDER:
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                 "INSERT_PROVIDER providerName = %s ",
                 (const char*)*(parms->providerName)->getCString()));

            AutoMutex lock (_providerTableMutex);

            if (false  == _providers.insert(*(parms->providerName),
                                    *reinterpret_cast<JMPIProvider * *>(parm)))
            {
                PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                     "Can not insert JMPIProvider %s into "
                         "JMPIProvider Manager Cache",
                     (const char*)*(parms->providerName)->getCString()));
                ccode = -1;
            }
            break;
        }

    case INSERT_MODULE:
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                 "INSERT_MODULE moduleName = %s ",
                 (const char*)*(parms->fileName)->getCString()));

            AutoMutex lock (_providerTableMutex);

            if(false  == _modules.insert(*(parms->fileName),
                              *reinterpret_cast<JMPIProviderModule * *>(parm)))
            {
                PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                     "Can not insert JMPIProvider Module %s into "
                         "JMPIProvider Manager Cache",
                     (const char*)*(parms->fileName)->getCString()));
                ccode = -1;
            }
            break;
        }

    case REMOVE_PROVIDER:
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                 "REMOVE_PROVIDER providerName = %s ",
                 (const char*)*(parms->providerName)->getCString()));

            AutoMutex lock (_providerTableMutex);

            if (false == _providers.remove(*(parms->providerName)))
            {
                PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                     "Can not remove JMPIProvider %s from "
                         "JMPIProvider Manager Cache",
                     (const char*)*(parms->providerName)->getCString()));
                ccode = -1;
            }

            break;
        }

    case REMOVE_MODULE:
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                 "REMOVE_MODULE moduleName = %s ",
                 (const char*)*(parms->fileName)->getCString()));

            AutoMutex lock (_providerTableMutex);

            if (false == _modules.remove(*(parms->fileName)))
            {
                PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
                     "Can not remove JMPIProvider Module %s from "
                         "JMPIProvider Manager Cache",
                     (const char*)*(parms->fileName)->getCString()));
                ccode = -1;
            }
            break;
        }

    case UNLOAD_ALL_PROVIDERS:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                "UNLOAD_ALL_PROVIDERS");
           JMPIjvm::destroyJVM();
           break;
        }

    case UNLOAD_IDLE_PROVIDERS:
         {
           PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER,Tracer::LEVEL3,
               "UNLOAD_IDLE_PROVIDERS");
           break;
        }

    case UNLOAD_IDLE_MODULES:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER,Tracer::LEVEL3,
                "UNLOAD_IDLE_MODULES");
           break;
        }

    default:
        {
            PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
                 "Provider Control: Unknown function code %d.",
                 code));
           ccode = -1;
           break;
        }
    }
    PEG_METHOD_EXIT();
    return(ccode);
}

JMPIProvider * JMPILocalProviderManager::_getResolver
   (const String & fileName, const String & interfaceType)
{
    JMPIProvider *pr;
    String id(interfaceType+String("::")+fileName);
    if (true == _resolvers.lookup(id,pr)) {
       return pr;
    }
    return NULL;
}

JMPIProvider::OpProviderHolder JMPILocalProviderManager::getProvider(
    const String & fileName,
    const String & providerName,
    const String & interfaceName)
{
    JMPIProvider::OpProviderHolder ph;
    CTRL_STRINGS strings;
    Sint32 ccode;
    strings.fileName = &fileName;
    strings.providerName = &providerName;
    strings.interfaceName = &interfaceName;

    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "ProviderManager::getProvider");

    try {
        ccode = _provider_ctrl( GET_PROVIDER, &strings, &ph );
    }
    catch (const Exception &e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA,Tracer::LEVEL1,
             "Can not get provider. Caught exception: %s",
            (const char*)e.getMessage().getCString()));
        PEG_METHOD_EXIT();
        throw;
    }
    catch(...) {
        PEG_TRACE_CSTRING(TRC_DISCARDED_DATA,Tracer::LEVEL1,
             "Can not get provider. Caught unknown exception,");
        PEG_METHOD_EXIT();
        throw;
    }

    PEG_METHOD_EXIT();
    return(ph);
}

void JMPILocalProviderManager::unloadProvider(
    const String & fileName,
    const String & providerName)
{
    CTRL_STRINGS strings;
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER, "ProviderManager::unloadProvider");
    strings.fileName = &fileName;
    strings.providerName = &providerName;
    _provider_ctrl(UNLOAD_PROVIDER, &strings, (void *)0);
    PEG_METHOD_EXIT();
}

void JMPILocalProviderManager::shutdownAllProviders(void)
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "ProviderManager::shutdownAllProviders");
    _provider_ctrl(UNLOAD_ALL_PROVIDERS, (void *)this, (void *)0);
    PEG_METHOD_EXIT();
}


Boolean JMPILocalProviderManager::hasActiveProviders()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "ProviderManager::hasActiveProviders");

    AutoMutex lock (_providerTableMutex);

    Boolean fRet = _providers.size() > 0;

    PEG_METHOD_EXIT();
    return fRet;
}

void JMPILocalProviderManager::unloadIdleProviders()
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "ProviderManager::unloadIdleProviders");

    try
    {
        AutoMutex lock(_providerTableMutex);

        _provider_ctrl(UNLOAD_IDLE_PROVIDERS, this, (void *)0);
    }
    catch(...)
    {
        PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
            "Caught unexpected exception from UNLOAD_IDLE_PROVIDERS.");
    }

    PEG_METHOD_EXIT();
}

Array <JMPIProvider *>
JMPILocalProviderManager::getIndicationProvidersToEnable ()
{
    PEG_METHOD_ENTER (TRC_PROVIDERMANAGER,
        "JMPILocalProviderManager::getIndicationProvidersToEnable");

    Array <JMPIProvider *> enableProviders;

    try
    {
        AutoMutex lock (_providerTableMutex);

        PEG_TRACE((
            TRC_PROVIDERMANAGER,
            Tracer::LEVEL4,
            "Number of providers in _providers table = %d",
            _providers.size ()));

        //
        // Iterate through the _providers table
        //
        for (ProviderTable::Iterator i = _providers.start (); i != 0; i++)
        {
            //
            //  Enable any indication provider with current subscriptions
            //
            JMPIProvider * provider = i.value ();
            if (provider->testSubscriptions ())
            {
                enableProviders.append (provider);
            }
        }
    }
    catch (CIMException & e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1, "CIMException: %s",
            (const char*)e.getMessage().getCString()));
    }
    catch (Exception & e)
    {
        PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1, "Exception: %s",
            (const char*)e.getMessage().getCString()));
    }
    catch (...)
    {
        PEG_TRACE_CSTRING (TRC_DISCARDED_DATA, Tracer::LEVEL1,
            "Unexpected error in getIndicationProvidersToEnable");
    }

    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "Number of indication providers to enable = %d",
        enableProviders.size ()));

    PEG_METHOD_EXIT ();
    return enableProviders;
}


PEGASUS_NAMESPACE_END
