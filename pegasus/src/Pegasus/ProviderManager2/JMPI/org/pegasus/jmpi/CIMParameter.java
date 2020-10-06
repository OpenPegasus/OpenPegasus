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
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

public class CIMParameter
{
   private long cInst;

   private native String  _getName               (long cInst);
   private native void    _setName               (long cInst, String name);
   private native boolean _isArray               (long cInst);
   private native int     _getArraySize          (long cInst);
   private native String  _getReferenceClassName (long cInst);
   private native long    _getType               (long cInst);
   private native void    _finalize              (long cInst);

   protected void finalize ()
   {
      _finalize (cInst);
   }

   protected long cInst ()
   {
      return cInst;
   }

   public CIMParameter (long ci)
   {
      cInst = ci;
   }

   public String getName ()
   {
      if (cInst != 0)
      {
         return _getName (cInst);
      }

      return null;
   }

   public boolean isArray ()
   {
      if (cInst != 0)
      {
         return _isArray (cInst);
      }

      return false;
   }

   public int getArraySize ()
   {
      if (cInst != 0)
      {
         return _getArraySize (cInst);
      }

      return 0;
   }

   public String getReferenceClassName ()
   {
      if (cInst != 0)
      {
         return _getReferenceClassName (cInst);
      }

      return null;
   }

   public CIMDataType getType ()
   {
      if (cInst != 0)
      {
         long ciDT = _getType (cInst);

         if (ciDT != 0)
         {
            return new CIMDataType (ciDT, true);
         }
      }

      return null;
   }

   static
   {
      System.loadLibrary ("JMPIProviderManager");
   }
}
