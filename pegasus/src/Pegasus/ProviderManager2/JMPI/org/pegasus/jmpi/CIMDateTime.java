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
// Author:      Adrian Duta
//
// Modified By: Magda
//              Mark Hamzy, hamzy@us.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

/**
    Creates and instantiates a Java object that represents the date and time.
 */
public class CIMDateTime
{
   private long cInst;

   private native long    _datetime        (String n);
   private native long    _datetimeempty   ();
   private native boolean _after           (long   c,
                                            long   d);
   private native boolean _before          (long   c,
                                            long   d);
   private native void    _finalize        (long   cInst);
   private native String  _getCIMString    (long   cInst);
   private native long    _getMicroseconds (long   cInst);

   private static String  GMT            = "GMT";
   private static int     UTC_MULTIPLIER = 60000;

   protected void finalize ()
   {
      _finalize (cInst);
   }

   protected long cInst ()
   {
      return cInst;
   }

   CIMDateTime (long ci)
   {
      cInst = ci;
   }

   public CIMDateTime ()
   {
      cInst = _datetimeempty ();
   }

   /**
    * Builds a CIMDateTime from a String that is compliant with the
    * CIM specification for absolute or interval date/times
    *
    * @param inStr The date/time as defined by the CIM specification
    *
    */
   public CIMDateTime (String n)
      throws CIMException
   {
      cInst = _datetime (n);
   }

   /**
    * Builds a CIMDateTime from an existing date
    *
    * @param inDate The date to initialize the CIMDateType from
    */
   public CIMDateTime (Date inDate)
   {
      String dateTime = getDateString (inDate);

      cInst = _datetime (dateTime);
   }

   public boolean after (CIMDateTime d)
   {
      if (cInst != 0)
      {
         return _after (cInst, d.cInst ());
      }
      else
      {
         return false;
      }
   }

   public boolean before (CIMDateTime d)
   {
      if (cInst != 0)
      {
         return _before (cInst, d.cInst ());
      }
      else
      {
         return false;
      }
   }

   public String getCIMDateString ()
   {
      if (cInst != 0)
      {
         return _getCIMString (cInst);
      }
      else
      {
         return null;
      }
   }

   public Date getJavaDate ()
   {
      if (cInst == 0)
      {
         return null;
      }

      long timeMicros = _getMicroseconds (cInst);
      long timeMillis = timeMicros / 1000;

      return new Date (timeMillis);
   }

   private String getDateString (Date d)
   {
      Calendar cal = Calendar.getInstance (TimeZone.getTimeZone (GMT));

      cal.setTime (d);

      int UTCOffset = (( cal.get(Calendar.ZONE_OFFSET)
                       + cal.get(Calendar.DST_OFFSET)
                       )
                       /
                       UTC_MULTIPLIER
                      );

      StringBuffer newString = new StringBuffer ();

      newString.append (padZero (4, cal.get (Calendar.YEAR)));
      newString.append (padZero (2, cal.get (Calendar.MONTH) + 1));
      newString.append (padZero (2, cal.get (Calendar.DAY_OF_MONTH)));
      newString.append (padZero (2, cal.get (Calendar.HOUR_OF_DAY)));
      newString.append (padZero (2, cal.get (Calendar.MINUTE)));
      newString.append (padZero (2, cal.get (Calendar.SECOND)));
      newString.append (".");

      //millisecond to microsecond
      newString.append (padZero (6, cal.get (Calendar.MILLISECOND)));

      if (UTCOffset > 0)
      {
         newString.append ("+");
      }
      else
      {
         newString.append ("-");
      }

      newString.append (padZero (3, Math.abs (UTCOffset)));

      return newString.toString ();
   }

   private String padZero (int length,
                           int value)
   {
      String buf   = String.valueOf (value);
      int    delta = length - buf.length ();

      if (delta < 1)
      {
         return buf;
      }

      String prefix = "0";

      for (int i = 1; i < delta; i++)
      {
         prefix = prefix + "0";
      }

      return prefix + buf;
   }

   static {
      System.loadLibrary ("JMPIProviderManager");
   }
}
