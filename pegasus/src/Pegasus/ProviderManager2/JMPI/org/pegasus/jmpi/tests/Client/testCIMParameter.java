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

package Client;

import org.pegasus.jmpi.CIMClass;
import org.pegasus.jmpi.CIMClient;
import org.pegasus.jmpi.CIMDataType;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMMethod;
import org.pegasus.jmpi.CIMObjectPath;
import org.pegasus.jmpi.CIMParameter;

public class testCIMParameter
{
   private boolean DEBUG = false;

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "CIMParameter";
   }

   public void setDebug (boolean fDebug)
   {
      DEBUG = fDebug;
   }

   public boolean main (String args[], CIMClient cimClient)
   {
      boolean fExecuted = false;
      boolean fRet      = true;

      for (int i = 0; i < args.length; i++)
      {
          if (args[i].equalsIgnoreCase ("debug"))
          {
              setDebug (true);
          }
      }

      if (!fExecuted)
      {
         fRet = testCIMParameter (cimClient);
      }

      return fRet;
   }

   private boolean testCIMParameter (CIMClient client)
   {
      if (client == null)
      {
         System.out.println ("FAILURE: testCIMParameter: client == null");

         return false;
      }

      // -----

      CIMObjectPath cop           = null;
      CIMClass      cc            = null;
      String        pszMethodName = "enableModifications";
      CIMMethod     cm            = null;
      CIMParameter  cp            = null;

      cop = new CIMObjectPath ("JMPIExpInstance_TestPropertyTypes",
                               "root/SampleProvider");

      if (DEBUG)
      {
         System.out.println ("testCIMParameter: cop = " + cop);
      }

      try
      {
         cc = client.getClass (cop,
                               true,  // propagated
                               true,  // includeQualifiers
                               true,  // includeClassOrigin
                               null); // propertyList
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMParameter: client.getClass: caught " + e);

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMParameter: cc = " + cc);
      }

      if (cc == null)
      {
         System.out.println ("FAILURE: testCIMParameter: cc == null");

         return false;
      }

      cm = cc.getMethod (pszMethodName);

      if (DEBUG)
      {
         System.out.println ("testCIMParameter: cm = " + cm);
      }

      if (cm == null)
      {
         System.out.println ("FAILURE: testCIMParameter: cm == null");

         return false;
      }

      try
      {
         cp = cm.getParameter (0);
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMParameter: cm.getParameter (0) unknown exception " + e);

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMParameter: cp = " + cp);
      }

      if (cp == null)
      {
         System.out.println ("FAILURE: testCIMParameter: cp == null");

         return false;
      }

      // -----

      String pszParameterName = "fState";
      String pszName          = null;

      pszName = cp.getName ();

      if (DEBUG)
      {
         System.out.println ("testCIMParameter: pszName = " + pszName);
      }

      if (pszName == null)
      {
         System.out.println ("FAILURE: testCIMParameter: cp.getName () 1");

         return false;
      }

      if (!pszName.equals (pszParameterName))
      {
         System.out.println ("FAILURE: testCIMParameter: cp.getName () 2");

         return false;
      }

      // -----

      if (DEBUG)
      {
         System.out.println ("testCIMParameter: cp.isArray () = " + cp.isArray ());
      }

      if (cp.isArray ())
      {
         System.out.println ("FAILURE: testCIMParameter: cp.isArray () 1");

         return false;
      }

      // @TBD - make a function that has an array parameter.  test if so here.

      // -----

      // @TBD - make a function that has an array parameter.  test the size here.

      // -----

      String pszReferenceClassName = null;

      pszReferenceClassName = cp.getReferenceClassName ();

      if (DEBUG)
      {
         System.out.println ("testCIMParameter: cp.getReferenceClassName () = " + pszReferenceClassName);
      }

      if (pszReferenceClassName == null)
      {
         System.out.println ("FAILURE: testCIMParameter: cp.getReferenceClassName () 1");

         return false;
      }

      if (!pszReferenceClassName.equals (""))
      {
         System.out.println ("FAILURE: testCIMParameter: cp.getReferenceClassName () 2");

         return false;
      }

      // -----

      CIMDataType cdt = null;

      cdt = cp.getType ();

      if (DEBUG)
      {
         System.out.println ("testCIMParameter: cp.getType () = " + cdt);
      }

      if (cdt == null)
      {
         System.out.println ("FAILURE: testCIMParameter: cp.getType () 1");

         return false;
      }

      if (cdt.getType () != CIMDataType.BOOLEAN)
      {
         System.out.println ("FAILURE: testCIMParameter: cp.getType () 2");

         return false;
      }

      // -----

      System.out.println ("SUCCESS: testCIMParameter");

      return true;
   }
}
