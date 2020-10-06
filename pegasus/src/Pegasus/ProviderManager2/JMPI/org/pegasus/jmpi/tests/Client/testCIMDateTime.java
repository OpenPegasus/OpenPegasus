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
import org.pegasus.jmpi.CIMDateTime;
import org.pegasus.jmpi.CIMException;
import java.util.Date;
import java.util.TimeZone;
import java.util.Calendar;
import java.util.GregorianCalendar;

public class testCIMDateTime
{
   private boolean DEBUG = false;

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "CIMDateTime";
   }

   public void setDebug (boolean fDebug)
   {
      DEBUG = fDebug;
   }

   public boolean main (String args[], CIMClient cimClient)
      throws CIMException
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
         fRet = testCIMDateTime (cimClient);
      }

      return fRet;
   }

   private boolean testCIMDateTime (CIMClient client)
      throws CIMException
   {
      if (client == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: client == null");

         return false;
      }

      // -----

      String      pszDateTime = null;
      CIMDateTime cdt         = null;

      pszDateTime = "20060718170256.123456-360";

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime of " + pszDateTime);
      }

      cdt = new CIMDateTime (pszDateTime);

      if (cdt == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdt == null (1)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: cdt = " + cdt);
      }

      // -----

      String pszDateTimeRet = null;

      pszDateTimeRet = cdt.getCIMDateString ();

      if (pszDateTimeRet == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: pszDateTimeRet == null");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: pszDateTimeRet = " + pszDateTimeRet);
      }

      if (!pszDateTimeRet.equals (pszDateTime))
      {
         System.out.println ("FAILURE: testCIMDateTime: pszDateTimeRet.equals (pszDateTime)");

         return false;
      }

      // -----

      Date date = null;

      date = cdt.getJavaDate ();

      if (date == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: date == null");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: date = " + date.getTime ());
      }

      // FYI: Time here is in MILLIseconds
      if (date.getTime () != 1153263776123L)
      {
         System.out.println ("FAILURE: testCIMDateTime: date.getTime () != 1153263776123L");

         return false;
      }

      // -----

      CIMDateTime cdta = null;

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime of " + pszDateTime);
      }

      cdta = new CIMDateTime (pszDateTime);

      if (cdta == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdta == null (1)");

         return false;
      }

      // -----

      pszDateTime = "20060718170256.12345*-360";

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime of " + pszDateTime);
      }

      cdt = new CIMDateTime (pszDateTime);

      if (cdt == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdt == null (2)");

         return false;
      }

      // -----

      pszDateTime = "20060718170256.1234**-360";

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime of " + pszDateTime);
      }

      cdt = new CIMDateTime (pszDateTime);

      if (cdt == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdt == null (3)");

         return false;
      }

      // -----

      pszDateTime = "20060718170256.123***-360";

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime of " + pszDateTime);
      }

      cdt = new CIMDateTime (pszDateTime);

      if (cdt == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdt == null (4)");

         return false;
      }

      // -----

      pszDateTime = "20060718170256.12****-360";

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime of " + pszDateTime);
      }

      cdt = new CIMDateTime (pszDateTime);

      if (cdt == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdt == null (5)");

         return false;
      }

      // -----

      pszDateTime = "20060718170256.1*****-360";

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime of " + pszDateTime);
      }

      cdt = new CIMDateTime (pszDateTime);

      if (cdt == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdt == null (6)");

         return false;
      }

      // -----

      pszDateTime = "20060718170256.******-360";

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime of " + pszDateTime);
      }

      cdt = new CIMDateTime (pszDateTime);

      if (cdt == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdt == null (7)");

         return false;
      }

      // -----

      pszDateTime = "200607181702**.******-360";

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime of " + pszDateTime);
      }

      cdt = new CIMDateTime (pszDateTime);

      if (cdt == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdt == null (8)");

         return false;
      }

      // -----

      pszDateTime = "2006071817****.******-360";

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime of " + pszDateTime);
      }

      cdt = new CIMDateTime (pszDateTime);

      if (cdt == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdt == null (9)");

         return false;
      }

      // -----

      pszDateTime = "20060718******.******-360";

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime of " + pszDateTime);
      }

      cdt = new CIMDateTime (pszDateTime);

      if (cdt == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdt == null (10)");

         return false;
      }

      // -----

      pszDateTime = "200607********.******-360";

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime of " + pszDateTime);
      }

      cdt = new CIMDateTime (pszDateTime);

      if (cdt == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdt == null (11)");

         return false;
      }

      // -----

      pszDateTime = "2006**********.******-360";

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime of " + pszDateTime);
      }

      cdt = new CIMDateTime (pszDateTime);

      if (cdt == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdt == null (12)");

         return false;
      }

      // -----

      pszDateTime = "**************.******-360";

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime of " + pszDateTime);
      }

      cdt = new CIMDateTime (pszDateTime);

      if (cdt == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdt == null (13)");

         return false;
      }

      // -----

      String              pszDateTime1 = null;
      CIMDateTime cdta1        = null;
      String              pszDateTime2 = null;
      CIMDateTime cdta2a       = null;
      CIMDateTime cdta2b       = null;
      String              pszDateTime3 = null;
      CIMDateTime cdta3        = null;

      pszDateTime1 = "20060718170256.123455-360";
      pszDateTime2 = "20060718170256.123456-360";
      pszDateTime3 = "20060718170256.123457-360";

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: Creating a CIMDateTime1 of " + pszDateTime1);
         System.out.println ("testCIMDateTime: Creating a CIMDateTime2a/b of " + pszDateTime2);
         System.out.println ("testCIMDateTime: Creating a CIMDateTime3 of " + pszDateTime3);
      }

      cdta1  = new CIMDateTime (pszDateTime1);
      cdta2a = new CIMDateTime (pszDateTime2);
      cdta2b = new CIMDateTime (pszDateTime2);
      cdta3  = new CIMDateTime (pszDateTime3);

      if (cdta1 == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdta1 == null");

         return false;
      }
      else if (cdta2a == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdta2a == null");

         return false;
      }
      else if (cdta2b == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdta2b == null");

         return false;
      }
      else if (cdta3 == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdta3 == null");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: cdta1  = " + cdta1);
         System.out.println ("testCIMDateTime: cdta2a = " + cdta2a);
         System.out.println ("testCIMDateTime: cdta2b = " + cdta2b);
      }

      if (!(cdta1.before (cdta3)))
      {
         System.out.println ("FAILURE: testCIMDateTime: cdta1.before (cdta3)");

         return false;
      }
      else if (cdta3.before (cdta1))
      {
         System.out.println ("FAILURE: testCIMDateTime: cdta3.before (cdta1)");

         return false;
      }
      else if (cdta1.after (cdta3))
      {
         System.out.println ("FAILURE: testCIMDateTime: cdta1.after (cdta3)");

         return false;
      }
      else if (!(cdta3.after (cdta1)))
      {
         System.out.println ("FAILURE: testCIMDateTime: cdta3.after (cdta1)");

         return false;
      }
      else if (cdta2a.before (cdta2b))
      {
         System.out.println("FAILURE: testCIMDateTime: cdta2a.before (cdta2b)");

         return false;
      }
      else if (cdta2a.after (cdta2b))
      {
         System.out.println ("FAILURE: testCIMDateTime: cdta2a.after (cdta2b)");

         return false;
      }

      // -----

      cdta = new CIMDateTime ();

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: cdta  = " + cdta);
      }

      if (cdta == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdta == null (2)");

         return false;
      }

      // -----

      Calendar cal = new GregorianCalendar (TimeZone.getTimeZone ("Etc/GMT-1"));

      cdta = new CIMDateTime (cal.getTime ());

      if (DEBUG)
      {
         System.out.println ("testCIMDateTime: cdta  = " + cdta);
      }

      if (cdta == null)
      {
         System.out.println ("FAILURE: testCIMDateTime: cdta == null (3)");

         return false;
      }

      // -----

      String aszBadDates[] = {
         "x0060718170256.123456-360", //  0
         "2x060718170256.123456-360",
         "20x60718170256.123456-360",
         "200x0718170256.123456-360",
         "2006x718170256.123456-360",
         "20060x18170256.123456-360", //  5
         "200607x8170256.123456-360",
         "2006071x170256.123456-360",
         "20060718x70256.123456-360",
         "200607181x0256.123456-360",
         "2006071817x256.123456-360", // 10
         "20060718170x56.123456-360",
         "200607181702x6.123456-360",
         "2006071817025x.123456-360",
         "20060718170256x123456-360",
         "20060718170256.x23456-360", // 15
         "20060718170256.1x3456-360",
         "20060718170256.12x456-360",
         "20060718170256.123x56-360",
         "20060718170256.1234x6-360",
         "20060718170256.12345x-360", // 20
         "20060718170256.123456x360",
         "20060718170256.123456-x60",
         "20060718170256.123456-3x0",
         "20060718170256.123456-36x",
         "20060718170256.123456?360", // 25
         "20060718170256.123456-36",
         "20060718170256.123456-3601",
         "20060718170256.*12345-360",
         "20060718170256.**1234-360",
         "20060718170256.***123-360", // 30
         "20060718170256.****12-360",
         "20060718170256.*****1-360",
         "2006071817025*.******-360",
         "20060718170***.******-360",
         "200607181*****.******-360", // 35
         "2006071*******.******-360",
         "20060*********.******-360",
         "200***********.******-360",
         "20************.******-360",
         "2*************.******-360", // 40
         "20060718170256.123456-36*",
         "20060718170256.123456-3**",
         "20060718170256.123456-***",
      };

      for (int i = 0; i < aszBadDates.length; i++)
      {
         try
         {
            cdt = new CIMDateTime (aszBadDates[i]);

            System.out.println ("FAILURE: testCIMDateTime: aszBadDates[" + i + "] != null");

            return false;
         }
         catch (CIMException e)
         {
            if (e.getID ().equals (CIMException.CIM_ERR_FAILED))
            {
            }
            else
            {
               System.out.println ("FAILURE: testCIMDateTime: aszBadDates[" + i + "] caught Exception " + e);

               return false;
            }
         }
         catch (Exception e)
         {
            System.out.println ("FAILURE: testCIMDateTime: aszBadDates[" + i + "] caught Exception " + e);

            return false;
         }
      }

      // -----

      System.out.println ("SUCCESS: testCIMDateTime");

      return true;
   }
}
