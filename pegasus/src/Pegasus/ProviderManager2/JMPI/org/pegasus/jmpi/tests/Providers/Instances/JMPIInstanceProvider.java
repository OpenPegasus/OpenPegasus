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
// Author: Adrian Schuur schuur@deibm.com
//
// Modified By: Mark Hamzy, hamzy@us.ibm.com
//
//
// This code is based on TestPropertyTypes.cpp
//
// Original Author: Yi Zhou (yi_zhou@hp.com)
//
// Original Modified By: Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//                       Sushma Fernandes, Hewlett-Packard Company
//                         (sushma_fernandes@hp.com)
//                       Carol Ann Krug Graves, Hewlett-Packard Company
//                        (carolann_graves@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////
package Instances;

import java.util.Vector;
import org.pegasus.jmpi.CIMClass;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMInstance;
import org.pegasus.jmpi.CIMOMHandle;
import org.pegasus.jmpi.CIMObjectPath;
import org.pegasus.jmpi.CIMValue;
import org.pegasus.jmpi.InstanceProvider;
import org.pegasus.jmpi.MethodProvider;

public class JMPIInstanceProvider
             implements InstanceProvider,
                        MethodProvider
{
   public void initialize (CIMOMHandle ch)
      throws CIMException
   {
      if (cip != null)
      {
         cip.cleanup ();
      }

      cip = new CommonInstanceProvider ("JMPIInstanceProvider",
                                        "root/SampleProvider",
                                        "JMPIInstance_TestElement",
                                        "JMPIInstance_TestPropertyTypes");

      if (cip != null)
      {
         cip.initialize (ch);
      }
   }

   public void cleanup ()
      throws CIMException
   {
      if (cip != null)
      {
         cip.cleanup ();

         cip = null;
      }
   }

   public CIMObjectPath createInstance (CIMObjectPath cop,
                                        CIMInstance   cimInstance)
      throws CIMException
   {
      if (cip != null)
      {
         return cip.createInstance (null,        // oc,
                                    cop,
                                    cimInstance);
      }
      else
      {
         return null;
      }
   }

   public CIMInstance getInstance (CIMObjectPath cop,
                                   CIMClass      cimClass,
                                   boolean       localOnly)
      throws CIMException
   {
      if (cip != null)
      {
         return cip.getInstance (null,               // oc,
                                 cop,
                                 cimClass,
                                 true,               // includeQualifiers,
                                 true,               // includeClassOrigin,
                                 localOnly,
                                 null);              // propertyList);
      }
      else
      {
         return null;
      }
   }

   public void setInstance (CIMObjectPath cop,
                            CIMInstance   cimInstance)
      throws CIMException
   {
      if (cip != null)
      {
         cip.setInstance (null,        // oc,
                          cop,
                          cimInstance,
                          true,        // includeQualifiers,
                          null);       // propertyList);
      }
   }

   public void deleteInstance (CIMObjectPath cop)
      throws CIMException
   {
      if (cip != null)
      {
         cip.deleteInstance (null, // oc,
                             cop);
      }
   }

   // enumerateInstanceNames
   public Vector enumInstances (CIMObjectPath cop,
                                boolean       deepInheritance,
                                CIMClass      cimClass)
      throws CIMException
   {
      if (cip != null)
      {
         return cip.enumerateInstanceNames (null,            // oc,
                                            cop,
                                            deepInheritance,
                                            cimClass);
      }
      else
      {
         return null;
      }
   }

   // enumerateInstances
   public Vector enumInstances (CIMObjectPath cop,
                                boolean       deepInheritance,
                                CIMClass      cimClass,
                                boolean       localOnly)
      throws CIMException
   {
      if (cip != null)
      {
         return cip.enumerateInstances (null,            // oc,
                                        cop,
                                        cimClass,
                                        deepInheritance,
                                        localOnly,
                                        true,            // includeQualifiers,
                                        true,            // includeClassOrigin,
                                        null);           // propertyList);
      }
      else
      {
         return null;
      }
   }

   public Vector execQuery (CIMObjectPath cop,
                            String        queryStatement,
                            int           queryLanguage,
                            CIMClass      cimClass)
      throws CIMException
   {
      if (cip != null)
      {
         String queryLanguageString = "WQL";        // @TBD

         return cip.execQuery (null,                // oc,
                               cop,
                               queryStatement,
                               queryLanguageString,
                               cimClass);
      }
      else
      {
         return null;
      }
   }

   public CIMValue invokeMethod (CIMObjectPath    cop,
                                 String           methodName,
                                 Vector           in,
                                 Vector           out)
      throws CIMException
   {
      if (cip != null)
      {
         return cip.invokeMethod (null,       // oc,
                                  cop,
                                  methodName,
                                  in,
                                  out);
      }
      else
      {
         return null;
      }
   }

   private CommonInstanceProvider cip = null;
}
