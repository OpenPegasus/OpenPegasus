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

public class TestJavaProvider implements InstanceProvider2
{
   private Hashtable   data = new Hashtable ();
   private CIMOMHandle ch   = null;

   public TestJavaProvider ()
   {
      System.err.println ("~~~ TestJavaProvider::TestJavaProvider()");
   }

   public void initialize (CIMOMHandle ch)
      throws CIMException
   {
      System.err.println ("~~~ TestJavaProvider::Initialize()");

      this.ch = ch;
   }

   public void cleanup ()
      throws CIMException
   {
      System.err.println ("~~~ TestJavaProvider::cleanup()");
   }

   private void testOperationContext (OperationContext oc)
   {
      try
      {
         Object o = oc.get (OperationContext.IdentityContainer, "userName");
         if (o != null)
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (IdentityContainer, userName): \"" + o + "\"");
         else
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (IdentityContainer, userName): null");
      }
      catch (Exception e)
      {
         System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (IdentityContainer, userName): caught " + e);
      }
      try
      {
         Object o = oc.get (OperationContext.SubscriptionInstanceContainer, "subscriptionInstance");
         if (o != null)
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionInstanceContainer, subscriptionInstance): \"" + o + "\"");
         else
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionInstanceContainer, subscriptionInstance): null");
      }
      catch (Exception e)
      {
         System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionInstanceContainer, subscriptionInstance): caught " + e);
      }
      try
      {
         Object o = oc.get (OperationContext.SubscriptionInstanceNamesContainer, "subscriptionInstanceNames");
         if (o != null)
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionInstanceNamesContainer, subscriptionInstanceNames): \"" + o + "\"");
         else
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionInstanceNamesContainer, subscriptionInstanceNames): null");
      }
      catch (Exception e)
      {
         System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionInstanceNamesContainer, subscriptionInstanceNames): caught " + e);
      }
      try
      {
         Object o = oc.get (OperationContext.SubscriptionFilterConditionContainer, "filterCondition");
         if (o != null)
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterConditionContainer, filterCondition): \"" + o + "\"");
         else
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterConditionContainer, filterCondition): null");
      }
      catch (Exception e)
      {
         System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterConditionContainer, filterCondition): caught " + e);
      }
      try
      {
         Object o = oc.get (OperationContext.SubscriptionFilterConditionContainer, "queryLanguage");
         if (o != null)
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterConditionContainer, queryLanguage): \"" + o + "\"");
         else
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterConditionContainer, queryLanguage): null");
      }
      catch (Exception e)
      {
         System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterConditionContainer, queryLanguage): caught " + e);
      }
      try
      {
         Object o = oc.get (OperationContext.SubscriptionFilterQueryContainer, "filterQuery");
         if (o != null)
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterQueryContainer, filterQuery): \"" + o + "\"");
         else
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterQueryContainer, filterQuery): null");
      }
      catch (Exception e)
      {
         System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterQueryContainer, filterQuery): caught " + e);
      }
      try
      {
         Object o = oc.get (OperationContext.SubscriptionFilterQueryContainer, "queryLanguage");
         if (o != null)
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterQueryContainer, queryLanguage): \"" + o + "\"");
         else
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterQueryContainer, queryLanguage): null");
      }
      catch (Exception e)
      {
         System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterQueryContainer, queryLanguage): caught " + e);
      }
      try
      {
         Object o = oc.get (OperationContext.SubscriptionFilterQueryContainer, "sourceNameSpace");
         if (o != null)
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterQueryContainer, sourceNameSpace): \"" + o + "\"");
         else
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterQueryContainer, sourceNameSpace): null");
      }
      catch (Exception e)
      {
         System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SubscriptionFilterQueryContainer, sourceNameSpace): caught " + e);
      }
      try
      {
         Object o = oc.get (OperationContext.SnmpTrapOidContainer, "snmpTrapOid");
         if (o != null)
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SnmpTrapOidContainer, snmpTrapOid): \"" + o + "\"");
         else
            System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SnmpTrapOidContainer, snmpTrapOid): null");
      }
      catch (Exception e)
      {
         System.err.println ("~~~ TestJavaProvider::testOperationContext: oc.get (SnmpTrapOidContainer, snmpTrapOid): caught " + e);
      }
   }

   public CIMObjectPath createInstance (OperationContext oc,
                                        CIMObjectPath    op,
                                        CIMInstance      ci)
      throws CIMException
   {
      System.err.println ("~~~ TestJavaProvider::createInstance()");

      testOperationContext (oc);

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
      System.err.println ("~~~ TestJavaProvider::getInstance()");

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
      System.err.println ("~~~ TestJavaProvider::setInstance()");

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
      System.err.println ("~~~ TestJavaProvider::deleteInstance()");

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
      System.err.println ("~~~ TestJavaProvider::enumerateInstanceNames()");

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
      System.err.println ("~~~ TestJavaProvider::enumerateInstances()");

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
      System.err.println ("~~~ TestJavaProvider::execQuery()");

      return null;
   }
}
