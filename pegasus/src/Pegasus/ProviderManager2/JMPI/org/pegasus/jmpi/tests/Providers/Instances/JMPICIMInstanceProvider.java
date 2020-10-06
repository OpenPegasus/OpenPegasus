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
// Author:      Mark Hamzy, hamzy@us.ibm.com
//
// Modified By: Mark Hamzy, hamzy@us.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////
package Instances;

import java.util.Vector;
import java.util.Arrays;
import org.pegasus.jmpi.CIMArgument;
import org.pegasus.jmpi.CIMClass;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMInstance;
import org.pegasus.jmpi.CIMOMHandle;
import org.pegasus.jmpi.CIMObjectPath;
import org.pegasus.jmpi.CIMValue;
import org.pegasus.jmpi.CIMInstanceProvider;
import org.pegasus.jmpi.CIMMethodProvider;

public class JMPICIMInstanceProvider
             implements CIMInstanceProvider,
                        CIMMethodProvider
{
   public void initialize (CIMOMHandle ch)
      throws CIMException
   {
      if (cip != null)
      {
         cip.cleanup ();
      }

      cip = new CommonInstanceProvider ("JMPICIMInstanceProvider",
                                        "root/SampleProvider",
                                        "JMPICIMInstance_TestElement",
                                        "JMPICIMInstance_TestPropertyTypes");

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
                                   boolean       localOnly,
                                   boolean       includeQualifiers,
                                   boolean       includeClassOrigin,
                                   String[]      propertyList,
                                   CIMClass      cimClass)
      throws CIMException
   {
      if (cip != null)
      {
         return cip.getInstance (null,               // oc,
                                 cop,
                                 cimClass,
                                 includeQualifiers,
                                 includeClassOrigin,
                                 localOnly,
                                 propertyList);
      }
      else
      {
         return null;
      }
   }

   public void setInstance (CIMObjectPath cop,
                            CIMInstance   cimInstance,
                            boolean       includeQualifiers,
                            String[]      propertyList)
      throws CIMException
   {
      if (cip != null)
      {
         cip.setInstance (null,             // oc,
                          cop,
                          cimInstance,
                          includeQualifiers,
                          propertyList);
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

   public CIMObjectPath[] enumerateInstanceNames (CIMObjectPath cop,
                                                  CIMClass      cimClass)
      throws CIMException
   {
      if (cip != null)
      {
         Vector          retVector = null;
         CIMObjectPath[] retArray  = null;

         retVector = cip.enumerateInstanceNames (null,            // oc,
                                                 cop,
                                                 false,           // deepInheritance,
                                                 cimClass);

         retArray = new CIMObjectPath [retVector.size ()];

         System.arraycopy (retVector.toArray (), 0, retArray, 0, retVector.size ());

         return retArray;
      }
      else
      {
         return null;
      }
   }

   public CIMInstance[] enumerateInstances (CIMObjectPath cop,
                                            boolean       localOnly,
                                            boolean       includeQualifiers,
                                            boolean       includeClassOrigin,
                                            String[]      propertyList,
                                            CIMClass      cimClass)
      throws CIMException
   {
      if (cip != null)
      {
         Vector        retVector = null;
         CIMInstance[] retArray  = null;

         retVector = cip.enumerateInstances (null,               // oc,
                                             cop,
                                             cimClass,
                                             false,              // deepInheritance,
                                             localOnly,
                                             includeQualifiers,
                                             includeClassOrigin,
                                             propertyList);

         retArray = new CIMInstance [retVector.size ()];

         System.arraycopy (retVector.toArray (), 0, retArray, 0, retVector.size ());

         return retArray;
      }
      else
      {
         return null;
      }
   }

   public CIMInstance[] execQuery (CIMObjectPath cop,
                                   String        queryStatement,
                                   String        queryLanguage,
                                   CIMClass      cimClass)
      throws CIMException
   {
      if (cip != null)
      {
         Vector        retVector = null;
         CIMInstance[] retArray  = null;

         retVector = cip.execQuery (null,                // oc,
                                    cop,
                                    queryStatement,
                                    queryLanguage,
                                    cimClass);

         retArray = new CIMInstance [retVector.size ()];

         System.arraycopy (retVector.toArray (), 0, retArray, 0, retVector.size ());

         return retArray;
      }
      else
      {
         return null;
      }
   }

   public CIMValue invokeMethod (CIMObjectPath    cop,
                                 String           methodName,
                                 CIMArgument[]    inArgs,
                                 CIMArgument[]    outArgs)
      throws CIMException
   {
      if (cip != null)
      {
         Vector in  = new Vector (Arrays.asList (inArgs));
         Vector out = new Vector (Arrays.asList (outArgs));

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
