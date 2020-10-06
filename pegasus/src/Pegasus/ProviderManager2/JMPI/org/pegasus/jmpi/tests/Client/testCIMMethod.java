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

public class testCIMMethod
{
   private boolean DEBUG = false;

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "CIMMethod";
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
         fRet = testCIMMethod (cimClient);
      }

      return fRet;
   }

   private boolean testCIMMethod (CIMClient client)
   {
      if (client == null)
      {
         System.out.println ("FAILURE: testCIMMethod: client == null");

         return false;
      }

      // -----

      CIMObjectPath cop = null;
      CIMClass      cc  = null;

      cop = new CIMObjectPath ("JMPIExpInstance_TestPropertyTypes",
                               "root/SampleProvider");

      if (DEBUG)
      {
         System.out.println ("testCIMMethod: cop = " + cop);
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
         System.out.println ("FAILURE: testCIMMethod: client.getClass: caught " + e);

         return false;
      }

      if (cc == null)
      {
         System.out.println ("FAILURE: testCIMMethod: cc == null");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMMethod: cc = " + cc);
      }

      // -----

      String    pszMethodName = "enableModifications";
      CIMMethod cm            = null;

      cm = cc.getMethod (pszMethodName);

      if (DEBUG)
      {
         System.out.println ("testCIMMethod: cm = " + cm);
      }

      if (cm == null)
      {
         System.out.println ("FAILURE: testCIMMethod: cc.getMethod ()");

         return false;
      }

      // -----

      int iType = 0;

      iType = cm.getType ();

      if (DEBUG)
      {
         System.out.println ("testCIMMethod: cm.getType () = " + iType);
      }

      if (iType != CIMDataType.BOOLEAN)
      {
         System.out.println ("FAILURE: testCIMMethod: cm.getType ()");

         return false;
      }

      // -----

      String pszName = null;

      pszName = cm.getName ();

      if (DEBUG)
      {
         System.out.println ("testCIMMethod: cm.getName () = " + pszName);
      }

      if (pszName == null)
      {
         System.out.println ("FAILURE: testCIMMethod: cm.getName () 1");

         return false;
      }

      if (!pszName.equals (pszMethodName))
      {
         System.out.println ("FAILURE: testCIMMethod: cm.getName () 2");

         return false;
      }

      // -----

      String pszParameterName = "fState";
      String pszBadParameter  = "bob";
      int    iParameter       = -1;

      iParameter = cm.findParameter (pszParameterName);

      if (DEBUG)
      {
         System.out.println ("testCIMMethod: cm.findParameter (" + pszParameterName + ") = " + iParameter);
      }

      if (iParameter != 0)
      {
         System.out.println ("FAILURE: testCIMMethod: cm.findParameter (" + pszParameterName + ")");

         return false;
      }

      iParameter = cm.findParameter (pszBadParameter);

      if (DEBUG)
      {
         System.out.println ("testCIMMethod: cm.findParameter (" + pszBadParameter + ") = " + iParameter);
      }

      if (iParameter != -1)
      {
         System.out.println ("FAILURE: testCIMMethod: cm.findParameter (" + pszBadParameter + ")");

         return false;
      }

      // -----

      CIMParameter cp = null;

      try
      {
         cp = cm.getParameter (0);
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMMethod: cm.getParameter (0) unknown exception " + e);

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMMethod: cm.getParameter (0) = " + cp);
      }

      if (cp == null)
      {
         System.out.println ("FAILURE: testCIMMethod: cm.getParameter (0)");

         return false;
      }

      try
      {
         cp = cm.getParameter (1);

         if (DEBUG)
         {
            System.out.println ("testCIMMethod: cm.getParameter (1) = " + cp);
         }
      }
      catch (CIMException e)
      {
         cp = null;

         if (DEBUG)
         {
            System.out.println ("testCIMMethod: caught = " + e);
         }

         if (e.getID () != CIMException.CIM_ERR_FAILED)
         {
            System.out.println ("FAILURE: testCIMMethod: cm.getParameter (1) 1");

            return false;
         }
      }

      if (cp != null)
      {
         System.out.println ("FAILURE: testCIMMethod: cm.getParameter (1) 2");

         return false;
      }

      // -----

      int iParameterCount = 0;

      iParameterCount = cm.getParameterCount ();

      if (DEBUG)
      {
         System.out.println ("testCIMMethod: cm.getParameterCount () = " + iParameterCount);
      }

      if (iParameterCount != 1)
      {
         System.out.println ("FAILURE: testCIMMethod: cm.getParameterCount ()");

         return false;
      }

      // -----

      System.out.println ("SUCCESS: testCIMMethod");

      return true;
   }
}
