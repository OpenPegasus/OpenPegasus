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
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.UnsignedInt8;

public class testUnsignedInt8
{
   private boolean DEBUG     = false;
   private short   MIN_VALUE = 0;
   private short   MAX_VALUE = 255;

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "UnsignedInt8";
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
         fRet = testUnsignedInt8 (cimClient);
      }

      return fRet;
   }

   private boolean testUnsignedInt8 (CIMClient client)
   {
      if (client == null)
      {
         System.out.println ("FAILURE: testUnsignedInt8: client == null");

         return false;
      }

      // -----

      UnsignedInt8 uint8 = null;

      uint8 = new UnsignedInt8 ((short)0);

      if (uint8 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt8: uint8 == null (1)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testUnsignedInt8: uint8 = " + uint8);
      }

      // -----

      try
      {
         uint8 = new UnsignedInt8 ((short)-1);

         System.out.println ("FAILURE: testUnsignedInt8: uint8 != null (2)");

         return false;
      }
      catch (NumberFormatException e)
      {
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testUnsignedInt8: caught " + e + " (2)");

         return false;
      }

      // -----

      uint8 = new UnsignedInt8 (MIN_VALUE);

      if (uint8 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt8: uint8 == null (3)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testUnsignedInt8: uint8 = " + uint8);
      }

      // -----

      try
      {
         uint8 = new UnsignedInt8 ((short)(MIN_VALUE - 1));

         System.out.println ("FAILURE: testUnsignedInt8: uint8 != null (4)");

         return false;
      }
      catch (NumberFormatException e)
      {
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testUnsignedInt8: caught " + e + " (4)");

         return false;
      }

      // -----

      uint8 = new UnsignedInt8 (MAX_VALUE);

      if (uint8 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt8: uint8 == null (5)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testUnsignedInt8: uint8 = " + uint8);
      }

      // -----

      try
      {
         uint8 = new UnsignedInt8 ((short)(MAX_VALUE + 1));

         System.out.println ("FAILURE: testUnsignedInt8: uint8 != null (6)");

         return false;
      }
      catch (NumberFormatException e)
      {
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testUnsignedInt8: caught " + e + " (6)");

         return false;
      }

      // -----

      uint8 = new UnsignedInt8 ("0");

      if (uint8 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt8: uint8 == null (7)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testUnsignedInt8: uint8 = " + uint8);
      }

      // -----

      try
      {
         uint8 = new UnsignedInt8 ("-1");

         System.out.println ("FAILURE: testUnsignedInt8: uint8 != null (8)");

         return false;
      }
      catch (NumberFormatException e)
      {
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testUnsignedInt8: caught " + e + " (8)");

         return false;
      }

      // -----

      uint8 = new UnsignedInt8 (new Short (MAX_VALUE).toString ());

      if (uint8 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt8: uint8 == null (9)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testUnsignedInt8: uint8 = " + uint8);
      }

      // -----

      try
      {
         uint8 = new UnsignedInt8 (new Short ((short)(MAX_VALUE + 1)).toString ());

         System.out.println ("FAILURE: testUnsignedInt8: uint8 != null (10)");

         return false;
      }
      catch (NumberFormatException e)
      {
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testUnsignedInt8: caught " + e + " (10)");

         return false;
      }

      // -----

      System.out.println ("SUCCESS: testUnsignedInt8");

      return true;
   }
}
