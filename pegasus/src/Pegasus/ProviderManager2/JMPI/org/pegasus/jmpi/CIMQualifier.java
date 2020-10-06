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
// Modified By: Adrian Schuur, schuur@de.ibm.com
//              Mark Hamzy,    hamzy@us.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

public class CIMQualifier
{
   private long cInst;

   private native long   _new      (String name);
   private native String _getName  (long   ci);
   private native long   _getValue (long   ci);
   private native void   _setValue (long   ci,
                                    long   value);
   private native void   _finalize (long   ci);

   protected void finalize ()
   {
      _finalize (cInst);
   }

   protected long cInst ()
   {
      return cInst;
   }

   CIMQualifier (long ci)
   {
      cInst = ci;
   }

   public CIMQualifier (String iname)
   {
      cInst = _new (iname);
   }

   public CIMValue getValue ()
   {
      long ciValue = _getValue (cInst);

      if (ciValue != 0)
      {
         try
         {
             return new CIMValue (ciValue);
         }
         catch (Exception e)
         {
             return null;
         }
      }
      else
      {
         return null;
      }
   }

   public void setValue (CIMValue value)
   {
      if (cInst != 0)
      {
         _setValue (cInst, value.cInst ());
      }
   }

   public String getName ()
   {
      if (cInst != 0)
      {
         return _getName (cInst);
      }
      else
      {
         return null;
      }
   }

   static {
      System.loadLibrary("JMPIProviderManager");
   }
}
