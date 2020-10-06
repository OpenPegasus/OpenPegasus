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

#include "JMPIProviderModule.h"

#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIProvider.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIProviderManager.h>

PEGASUS_NAMESPACE_BEGIN

JMPIProviderModule::JMPIProviderModule (const String & fileName,
                                        const String & interfaceName)
    : _fileName (fileName),
      _interfaceName (interfaceName)
{
   Uint32 i = fileName.find (".jar:");

   if (i == PEG_NOT_FOUND)
   {
      String msg = "Invalid Location format for Java providers: " + fileName;

      throw PEGASUS_CIM_EXCEPTION_L (CIM_ERR_FAILED, msg);
   }

   String jar = fileName.subString (0, i + 4);

   _className = fileName.subString (i + 5);

   if (jar[0] != '/')
      _fileName = JMPIProviderManager::resolveFileName (jar);
   else
      _fileName = jar;
}

JMPIProviderModule::~JMPIProviderModule (void)
{
}

ProviderVector JMPIProviderModule::load (const String & providerName)
{
   ProviderVector  pv  = { 0, 0 };
   JvmVector      *jv  = NULL;
   JNIEnv         *env = JMPIjvm::attachThread (&jv);

   if (env)
   {
      pv.jProvider = JMPIjvm::getProvider (env,
                                           _fileName,
                                           _className,
                                           providerName.getCString (),
                                           &pv.jProviderClass);
      JMPIjvm::detachThread ();
   }
   else
   {
      throw PEGASUS_CIM_EXCEPTION_L(
          CIM_ERR_FAILED,
          MessageLoaderParms(
              "ProviderManager.JMPI.INIT_JVM_FAILED",
              "Could not initialize the JVM (Java Virtual Machine) "
                  "runtime environment."));
   }

   if (pv.jProvider == 0)
   {
      String s0 = "ProviderLoadFailure";

      throw Exception(MessageLoaderParms(
          "ProviderManager.JMPI.JMPIProviderModule.CANNOT_LOAD_LIBRARY",
          "$0 ($1:$2): Cannot load library.",
          s0,
          _fileName,
          providerName));
   }

   return pv;
}

void JMPIProviderModule::unloadModule (void)
{
}

PEGASUS_NAMESPACE_END
