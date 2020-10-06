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
// Modified By: Mark Hamzy, hamzy@us.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

public class CIMArgument
{
   private long cInst;

   private native long           _new           ();
   private native long           _newS          (String name);
   private native long           _newSV         (String name, long ciValue);
   private native long           _getType       (long ciInst);
   private native void           _setValue      (long ciInst, long ciValue);
   private native long           _getValue      (long ciInst);
   private native String         _getName       (long ciInst);
   private native void           _setName       (long ciInst, String n);
   private native void           _finalize      (long ciInst);

   protected void finalize ()
   {
      _finalize (cInst);
   }

   CIMArgument (long ci)
   {
      cInst = ci;
   }

   protected long cInst ()
   {
      return cInst;
   }

   public CIMArgument ()
   {
      cInst = _new ();
   }

   public CIMArgument (String name)
   {
      cInst = _newS (name);
   }

   public CIMArgument (String name, CIMValue cv)
   {
      cInst = _newSV (name, cv.cInst ());
   }

   public CIMValue getValue ()
   {
      long ciValue = 0;

      if (cInst != 0)
      {
         ciValue = _getValue (cInst);
      }

      if (ciValue != 0)
      {
         return new CIMValue (ciValue);
      }
      else
      {
         return null;
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

   public void setName (String n)
   {
      if (cInst != 0)
      {
         _setName (cInst, n);
      }
   }

   public CIMDataType getType ()
   {
      long ciDataType = 0;

      if (cInst != 0)
      {
         ciDataType = _getType (cInst);
      }

      if (ciDataType != 0)
      {
         return new CIMDataType (ciDataType, true);
      }
      else
      {
         return null;
      }
   }

   public String toString ()
   {
      if (cInst != 0)
      {
         return getType ().toString () + " " + getName () + "=" + getValue ().toString () + ";";
      }
      else
      {
         return "null";
      }
   }

   public void setValue (CIMValue v)
   {
      if (cInst != 0)
      {
         _setValue (cInst, v.cInst ());
      }
   }

   static {
      System.loadLibrary ("JMPIProviderManager");
   }
}
