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
// Author:      Adrian Schuur, schuur@de.ibm.com
//
// Modified By: Adrian Duta
//
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

import java.util.*;

class ClassEnumeration implements Enumeration
{
   private long cInst;
   private int  cur, max;

   private native long _getClass (long cInst, int pos);
   private native int  _size     (long cInst);

   protected long cInst ()
   {
      return cInst;
   }

   ClassEnumeration (long ci)
   {
      cInst = ci;
      max   = 0;
      if (cInst != 0)
      {
         max = _size (cInst);
      }
      cur   = 0;
   }

   public boolean hasMoreElements ()
   {
      return (cur < max);
   }

   public Object nextElement ()
   {
      if (cur >= max)
         return null;

      long ciClass = 0;

      if (cInst != 0)
      {
         ciClass = _getClass (cInst, cur++);
      }

      if (ciClass != 0)
      {
         return new CIMClass (ciClass);
      }
      else
      {
         return null;
      }
   }

   static {
       System.loadLibrary ("JMPIProviderManager");
   }
}
