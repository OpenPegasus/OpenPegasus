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
import org.pegasus.jmpi.UnsignedInt64;
import java.math.BigInteger;

public class testUnsignedInt64
{
   private boolean    DEBUG     = false;
   private BigInteger MIN_VALUE = new BigInteger ("0");
   private BigInteger MAX_VALUE = new BigInteger ("18446744073709551615");

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "UnsignedInt64";
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
         fRet = testUnsignedInt64 (cimClient);
      }

      return fRet;
   }

   private boolean testUnsignedInt64 (CIMClient client)
   {
      if (client == null)
      {
         System.out.println ("FAILURE: testUnsignedInt64: client == null");

         return false;
      }

      // -----

      UnsignedInt64 uint64 = null;

      uint64 = new UnsignedInt64 (MIN_VALUE);

      if (uint64 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt64: uint64 == null (1)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testUnsignedInt64: uint64 = " + uint64);
      }

      // -----

      try
      {
         uint64 = new UnsignedInt64 (MIN_VALUE.subtract (BigInteger.ONE));

         System.out.println ("FAILURE: testUnsignedInt64: uint64 != null (2)");

         return false;
      }
      catch (NumberFormatException e)
      {
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testUnsignedInt64: caught " + e + " (2)");

         return false;
      }

      // -----

      uint64 = new UnsignedInt64 (MAX_VALUE);

      if (uint64 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt64: uint64 == null (3)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testUnsignedInt64: uint64 = " + uint64);
      }

      // -----

      try
      {
         uint64 = new UnsignedInt64 (MAX_VALUE.add (BigInteger.ONE));

         System.out.println ("FAILURE: testUnsignedInt64: uint64 != null (4)");

         return false;
      }
      catch (NumberFormatException e)
      {
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testUnsignedInt64: caught " + e + " (4)");

         return false;
      }

      // -----

      uint64 = new UnsignedInt64 ("0");

      if (uint64 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt64: uint64 == null (5)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testUnsignedInt64: uint64 = " + uint64);
      }

      // -----

      try
      {
         uint64 = new UnsignedInt64 ("-1");

         System.out.println ("FAILURE: testUnsignedInt64: uint64 != null (6)");

         return false;
      }
      catch (NumberFormatException e)
      {
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testUnsignedInt64: caught " + e + " (6)");

         return false;
      }

      // -----

      UnsignedInt64 uint64A  = null;
      UnsignedInt64 uint64B1 = null;
      UnsignedInt64 uint64B2 = null;
      UnsignedInt64 uint64C  = null;

      uint64A  = new UnsignedInt64 (BigInteger.ZERO);
      uint64B1 = new UnsignedInt64 (BigInteger.ONE);
      uint64B2 = new UnsignedInt64 (BigInteger.ONE);
      uint64C  = new UnsignedInt64 (new BigInteger ("2"));

      if (uint64A == null)
      {
         System.out.println ("FAILURE: testUnsignedInt64: uint64A == null");

         return false;
      }
      if (uint64B1 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt64: uint64B1 == null");

         return false;
      }
      if (uint64B2 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt64: uint64B2 == null");

         return false;
      }
      if (uint64C == null)
      {
         System.out.println ("FAILURE: testUnsignedInt64: uint64C == null");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testUnsignedInt64: uint64A  = " + uint64A);
         System.out.println ("testUnsignedInt64: uint64B1 = " + uint64B1);
         System.out.println ("testUnsignedInt64: uint64B2 = " + uint64B2);
         System.out.println ("testUnsignedInt64: uint64C  = " + uint64C);

         System.out.println ("testUnsignedInt64: uint64A.compareTo (uint64C)   = " + uint64A.compareTo ((Object)uint64C));
         System.out.println ("testUnsignedInt64: uint64C.compareTo (uint64A)   = " + uint64C.compareTo ((Object)uint64A));
         System.out.println ("testUnsignedInt64: uint64B1.compareTo (uint64B2) = " + uint64B1.compareTo ((Object)uint64B2));
         System.out.println ("testUnsignedInt64: uint64B1.compareTo (uint64B2) = " + uint64B2.compareTo ((Object)uint64B1));
         System.out.println ("testUnsignedInt64: uint64B1.equals (uint64B2)    = " + uint64B1.equals (uint64B2));
         System.out.println ("testUnsignedInt64: uint64A.equals (uint64C)      = " + uint64A.equals (uint64C));
      }

      if (!(uint64A.compareTo ((Object)uint64C) < 0))
      {
         System.out.println ("FAILURE: testUnsignedInt64: uint64A.compareTo (uint64C) < 0");

         return false;
      }
      else if (!(uint64C.compareTo ((Object)uint64A) > 0))
      {
         System.out.println ("FAILURE: testUnsignedInt64: uint64C.compareTo (uint64A) > 0");

         return false;
      }
      else if (!(uint64B1.compareTo ((Object)uint64B2) == 0))
      {
         System.out.println ("FAILURE: testUnsignedInt64: uint64B1.compareTo (uint64B2) == 0");

         return false;
      }
      else if (!(uint64B2.compareTo ((Object)uint64B1) == 0))
      {
         System.out.println ("FAILURE: testUnsignedInt64: uint64B2.compareTo (uint64B1) == 0");

         return false;
      }
      else if (!(sgn (uint64A.compareTo ((Object)uint64C)) == -sgn (uint64C.compareTo ((Object)uint64A))))
      {
         System.out.println ("FAILURE: testUnsignedInt64: sgn (uint64A.compareTo (uint64C)) == -sgn (uint64C.compareTo (uint64A))");

         return false;
      }
      else if (!(((uint64A.compareTo ((Object)uint64B1) > 0) && (uint64B1.compareTo ((Object)uint64C) > 0)) == (uint64A.compareTo ((Object)uint64C) > 0)))
      {
         System.out.println ("FAILURE: testUnsignedInt64: ((uint64A.compareTo (uint64B1) > 0) && (uint64B1.compareTo (uint64C) > 0)) == (uint64A.compareTo (uint64C) > 0)");

         return false;
      }
      else if (!(sgn (uint64B1.compareTo ((Object)uint64A)) == sgn (uint64B2.compareTo ((Object)uint64A))))
      {
         System.out.println ("FAILURE: testUnsignedInt64: sgn (uint64B1.compareTo (uint64A)) == sgn (uint64B2.compareTo (uint64A))");

         return false;
      }
      else if (!(sgn (uint64B1.compareTo ((Object)uint64C)) == sgn (uint64B2.compareTo ((Object)uint64C))))
      {
         System.out.println ("FAILURE: testUnsignedInt64: sgn (uint64B1.compareTo (uint64C)) == sgn (uint64B2.compareTo (uint64C))");

         return false;
      }
      else if (!((uint64B1.compareTo ((Object)uint64B2) == 0) == (uint64B1.equals (uint64B2))))
      {
         System.out.println ("FAILURE: testUnsignedInt64: (uint64B1.compareTo (uint64B2) == 0) == (uint64B1.equals (uint64B2))");

         return false;
      }
      else if (!((uint64A.compareTo ((Object)uint64C) == 0) == (uint64A.equals (uint64C))))
      {
         System.out.println ("FAILURE: testUnsignedInt64: (uint64A.compareTo (uint64C) == 0) == (uint64A.equals (uint64C))");

         return false;
      }

      // -----

      if (  !uint64B1.equals (uint64B2)
         || !uint64B2.equals (uint64B1)
         )
      {
         System.out.println ("FAILURE: testUnsignedInt64: int64B1.equals");

         return false;
      }

      // -----

      System.out.println ("SUCCESS: testUnsignedInt64");

      return true;
   }

   private static int sgn (int x)
   {
      if (x < 0)
      {
         return -1;
      }
      else
      {
         return 1;
      }
   }
}
