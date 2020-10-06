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
// Modified By:
//
//==============================================================================
package org.pegasus.jmpi.sample;

import org.pegasus.jmpi.*;
import java.util.*;

public class TestJavaProviderB implements InstanceProvider2
{
   private Hashtable   data = new Hashtable ();
   private CIMOMHandle ch   = null;

   public TestJavaProviderB ()
   {
      System.err.println ("~~~ TestJavaProviderB::TestJavaProviderB()");
   }

   public void initialize (CIMOMHandle ch)
      throws CIMException
   {
      System.err.println ("~~~ TestJavaProviderB::Initialize()");

      this.ch = ch;
   }

   public void cleanup ()
      throws CIMException
   {
      System.err.println ("~~~ TestJavaProviderB::cleanup()");
   }

   public CIMObjectPath createInstance (OperationContext oc,
                                        CIMObjectPath    op,
                                        CIMInstance      ci)
      throws CIMException
   {
      System.err.println ("~~~ TestJavaProviderB::createInstance()");

      CIMProperty cpk = ci.getProperty ("Identifier");
      Object      k   = cpk.getValue ().getValue ();
      CIMProperty cpv = ci.getProperty ("Data");
      Object      d   = cpv.getValue ().getValue ();

      if (data.get (k) != null)
         throw new CIMException (CIMException.CIM_ERR_ALREADY_EXISTS);

      data.put ((String)k, (String)d);

      op.addKey ("Identifier",new CIMValue (k));
      op.addKey ("Data",new CIMValue (d));

      return op;
   }

   public CIMInstance getInstance (OperationContext oc,
                                   CIMObjectPath    op,
                                   CIMClass         cc,
                                   boolean          includeQualifiers,
                                   boolean          includeClassOrigin,
                                   String           propertyList[])
      throws CIMException
   {
      System.err.println ("~~~ TestJavaProviderB::getInstance(): op                 = " + op);
      System.err.println ("~~~ TestJavaProviderB::getInstance(): cc                 = " + cc);
      System.err.println ("~~~ TestJavaProviderB::getInstance(): includeQualifiers  = " + includeQualifiers);
      System.err.println ("~~~ TestJavaProviderB::getInstance(): includeClassOrigin = " + includeClassOrigin);
      System.err.println ("~~~ TestJavaProviderB::getInstance(): propertyList       = " + propertyList);

      Vector      vec = op.getKeys ();

      if (  vec == null
         || vec.size () == 0
         )
      {
         return null;
      }

      CIMProperty cp  = (CIMProperty)vec.elementAt (0);
      CIMValue    cv  = cp.getValue ();
      String      k   = (String)cv.getValue ();
      String      d   = (String)data.get (k);

      if (d != null)
      {
         CIMInstance ci = cc.newInstance ();

         ci.setProperty ("Identifier",cv);
         ci.setProperty ("Data",new CIMValue (d));

         return ci;
      }

      return null;
   }

   public void setInstance (OperationContext oc,
                            CIMObjectPath    cop,
                            CIMInstance      cimInstance)
      throws CIMException
   {
      System.err.println ("~~~ TestJavaProviderB::setInstance()");

      CIMProperty cpk = cimInstance.getProperty ("Identifier");
      Object      k   = cpk.getValue ().getValue ();
      CIMProperty cpv = cimInstance.getProperty ("Data");
      Object      d   = cpv.getValue ().getValue ();

      if (data.get (k) == null)
         throw new CIMException (CIMException.CIM_ERR_NOT_FOUND);

      data.put ((String)k, (String)d);
   }

   public void deleteInstance (OperationContext oc,
                               CIMObjectPath    cop)
      throws CIMException
   {
      System.err.println ("~~~ TestJavaProviderB::deleteInstance()");

      Vector      vec = cop.getKeys ();
      CIMProperty cp  = (CIMProperty)vec.elementAt (0);
      Object      k   = cp.getValue ().getValue ();

      if (data.get (k) == null)
         throw new CIMException (CIMException.CIM_ERR_NOT_FOUND);

      data.remove((String)k);
   }

   public Vector enumerateInstanceNames (OperationContext oc,
                                         CIMObjectPath    cop,
                                         CIMClass         cimClass)
      throws CIMException
   {
      System.err.println ("~~~ TestJavaProviderB::enumerateInstanceNames()");

      Enumeration en  = data.keys ();
      Vector      vec = new Vector ();

      while (en.hasMoreElements ())
      {
         String        k  = (String)en.nextElement ();
         String        d  = (String)data.get (k);
         CIMObjectPath op = new CIMObjectPath ();

         op.setObjectName (cop.getObjectName ());
         op.addKey ("Identifier", new CIMValue (k));

         vec.addElement (op);
      }

      return vec;
   }

   public Vector enumerateInstances (OperationContext oc,
                                     CIMObjectPath    cop,
                                     CIMClass         cimClass,
                                     boolean          includeQualifiers,
                                     boolean          includeClassOrigin,
                                     String           propertyList[])
      throws CIMException
   {
      System.err.println ("~~~ TestJavaProviderB::enumerateInstances()");

      CIMObjectPath copA = CIMObjectPath.toCop (cop.toString ());

      System.err.println (copA);

      copA.setObjectName ("TestJavaProviderA");

      System.err.println (copA);

      Enumeration enumAInstances = ch.enumerateInstances (copA,
                                                          true,
                                                          false,
                                                          includeQualifiers,
                                                          includeClassOrigin,
                                                          propertyList);

      System.err.println (enumAInstances);
try
{
   CIMInstance ciB = ch.getInstance (cop,
                                     false,
                                     includeQualifiers,
                                     includeClassOrigin,
                                     propertyList);

   System.err.println (ciB);
}
catch (CIMException e)
{
   System.err.println ("Ignoring: " + e);
}

Enumeration enumA2Instances = ch.enumerateInstances (cop,
                                                     true,
                                                     false,
                                                     includeQualifiers,
                                                     includeClassOrigin,
                                                     propertyList);

      Enumeration en  = data.keys ();
      Vector      vec = new Vector ();

      while (en.hasMoreElements ())
      {
         String      k  = (String)en.nextElement ();
         String      d  = (String)data.get (k);
         CIMInstance ci = cimClass.newInstance ();

         ci.setProperty ("Identifier",new CIMValue (k));
         ci.setProperty ("Data",new CIMValue (d));
         vec.addElement (ci);
      }

      return vec;
   }

   public Vector execQuery (OperationContext oc,
                            CIMObjectPath    cop,
                            CIMClass         cimClass,
                            String           queryStatement,
                            String           queryLanguage)
      throws CIMException
   {
      System.err.println ("~~~ TestJavaProviderB::execQuery()");

      return null;
   }
}
