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

import org.pegasus.jmpi.CIMClient;
import org.pegasus.jmpi.CIMClass;
import org.pegasus.jmpi.CIMDataType;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMFlavor;
import org.pegasus.jmpi.CIMObjectPath;
import org.pegasus.jmpi.CIMQualifier;
import org.pegasus.jmpi.CIMValue;

public class testCIMQualifier
{
   private boolean DEBUG = false;

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "CIMQualifier";
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
         fRet = testCIMQualifier (cimClient);
      }

      return fRet;
   }

   private boolean testCIMQualifier (CIMClient client)
   {
      if (client == null)
      {
         System.out.println ("FAILURE: testCIMQualifier: client == null");

         return false;
      }

      // -----

      CIMObjectPath cop = null;
      CIMClass      cc  = null;

      cop = new CIMObjectPath ("JMPIExpInstance_TestPropertyTypes",
                               "root/SampleProvider");

      if (DEBUG)
      {
         System.out.println ("testCIMQualifier: cop = " + cop);
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
         System.out.println ("FAILURE: testCIMQualifier: client.getClass: caught " + e);

         return false;
      }

      if (cc == null)
      {
         System.out.println ("FAILURE: testCIMQualifier: cc == null");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMQualifier: cc = " + cc);
      }

      // -----

      String       pszQualiferName = "Version";
      CIMQualifier cq              = null;

      cq = cc.getQualifier (pszQualiferName);

      if (cq == null)
      {
         System.out.println ("FAILURE: testCIMQualifier: getQualifier (String)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMQualifier: cq = " + cq);
      }

      // -----

      String retQualiferName = null;

      retQualiferName = cq.getName ();

      if (  retQualiferName == null
         || !retQualiferName.equals (pszQualiferName)
         )
      {
         System.out.println ("FAILURE: testCIMQualifier: cq.getName ()");

         return false;
      }

      // -----

      Object oValue = null;

      try
      {
         oValue = cq.getValue ().getValue ();
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testCIMQualifier: cq.getValue (): caught " + e);

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMQualifier: oValue = " + oValue);
      }

      if (  oValue == null
         || !(oValue instanceof String)
         )
      {
         System.out.println ("FAILURE: testCIMQualifier: cq.getValue ()");

         return false;
      }

      // -----

      CIMValue cv = null;

      try
      {
         cv = new CIMValue (new Integer (1));

         if (cv == null)
         {
            System.out.println ("FAILURE: testCIMQualifier: cq.setValue ()");

            return false;
         }

         cq.setValue (cv);

         oValue = cq.getValue ().getValue ();

         if (DEBUG)
         {
            System.out.println ("testCIMQualifier: oValue = " + oValue);
         }
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testCIMQualifier: cq.setValue (): caught " + e);

         return false;
      }

      // -----

      System.out.println ("SUCCESS: testCIMQualifier");

      return true;
   }
}
