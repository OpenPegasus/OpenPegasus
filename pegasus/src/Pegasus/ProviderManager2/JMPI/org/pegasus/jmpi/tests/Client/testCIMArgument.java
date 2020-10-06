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

import org.pegasus.jmpi.CIMArgument;
import org.pegasus.jmpi.CIMClient;
import org.pegasus.jmpi.CIMDataType;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMValue;

public class testCIMArgument
{
   private boolean DEBUG = false;

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "CIMArgument";
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
         fRet = testCIMArgument (cimClient);
      }

      return fRet;
   }

   private boolean testCIMArgument (CIMClient client)
   {
      if (client == null)
      {
         System.out.println ("FAILURE: testCIMArgument: client == null");

         return false;
      }

      // -----

      CIMArgument ca1      = null;
      CIMArgument ca2      = null;
      String      pszName  = "bob";
      String      pszValue = "lives";
      CIMValue    cv       = null;

      try
      {
         ca1 = new CIMArgument (pszName,
                                new CIMValue (new String (pszValue)));
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMArgument: ca1: caught " + e);

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMArgument: ca1 = " + ca1);
      }

      if (ca1 == null)
      {
         System.out.println ("FAILURE: testCIMArgument: ca1 == null");

         return false;
      }

      String pszRetValue = null;

      try
      {
         pszRetValue = (String)ca1.getValue ().getValue ();
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMArgument: ca1.getValue ().getValue (): caught " + e);

         return false;
      }

      if (pszRetValue == null)
      {
         System.out.println ("FAILURE: testCIMArgument: ca1.getValue ().getValue () == null");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMArgument: ca1.getName ().equals (pszName) = " + (ca1.getName ().equals (pszName)));
         System.out.println ("testCIMArgument: (ca1.getType ().getType () == CIMDataType.STRING) = " + (ca1.getType ().getType () == CIMDataType.STRING));
         System.out.println ("testCIMArgument: (pszRetValue.equals (pszValue)) = " + (pszRetValue.equals (pszValue)));
      }

      if (  !(ca1.getName ().equals (pszName))
         || !(ca1.getType ().getType () == CIMDataType.STRING)
         || !(pszRetValue.equals (pszValue))
         )
      {
         System.out.println ("FAILURE: testCIMArgument: ca1.getXX");

         return false;
      }

      // -----

      try
      {
         ca2 = new CIMArgument (pszName,
                                new CIMValue (new String (pszValue)));
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMArgument: ca1: caught " + e);

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMArgument: ca2 = " + ca2);
      }

      if (ca2 == null)
      {
         System.out.println ("FAILURE: testCIMArgument: ca2 == null");

         return false;
      }
/* @TBD
      if (!ca1.equals (ca2))
      {
         System.out.println ("FAILURE: testCIMArgument: ca1.equals (ca2)");

         return false;
      }
*/

      // -----

      if (ca1.toString () == null)
      {
         System.out.println ("FAILURE: testCIMArgument: ca1.toString () == null");

         return false;
      }

      // -----

      CIMArgument ca3 = null;

      ca3 = new CIMArgument ("None");

      if (DEBUG)
      {
         System.out.println ("testCIMArgument: ca3 = " + ca3);
      }

      if (ca3 == null)
      {
         System.out.println ("FAILURE: testCIMArgument: ca3 == null");

         return false;
      }

      // -----

      String pszNewName = "NewNone";

      ca3.setName (pszNewName);

      if (DEBUG)
      {
         System.out.println ("testCIMArgument: ca3 = " + ca3);
      }

      if (!ca3.getName ().equals (pszNewName))
      {
         System.out.println ("FAILURE: testCIMArgument: ca3.setName");

         return false;
      }

      // -----

      try
      {
         cv = new CIMValue (new Integer (42));
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMArgument: cv: caught " + e);

         return false;
      }

      ca3.setValue (cv);

      if (DEBUG)
      {
         System.out.println ("testCIMArgument: ca3.getType () = " + ca3.getType ());
         System.out.println ("testCIMArgument: (ca3.getType ().getType () == CIMDataType.SINT32) = " + (ca3.getType ().getType () == CIMDataType.SINT32));
      }

      if (!(ca3.getType ().getType () == CIMDataType.SINT32))
      {
         System.out.println ("FAILURE: testCIMArgument: ca3.setValue");

         return false;
      }

      // -----

      System.out.println ("SUCCESS: testCIMArgument");

      return true;
   }
}
