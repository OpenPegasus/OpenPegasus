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
// Modified By:  Adrian Duta
//               Mark Hamzy, hamzy@us.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

import java.util.*;
import java.math.BigInteger;

public class CIMValue
{
   private long cInst;

   private native long    _byte          (short   v,    boolean unsigned);
   private native long    _short         (int     v,    boolean unsigned);
   private native long    _makeInt       (long    v,    boolean unsigned);
   private native long    _long          (long    v,    boolean unsigned);
   private native long    _string        (String  v);
   private native long    _boolean       (boolean v);
   private native long    _float         (float   v);
   private native long    _double        (double  v);
   private native long    _cop           (long    v);
   private native long    _datetime      (long    v);
   private native long    _char16        (char    v);
   private native long    _object        (long    v);

   private native long    _byteArray     (short   v[],  boolean unsigned);
   private native long    _shortArray    (int     v[],  boolean unsigned);
   private native long    _intArray      (long    v[],  boolean unsigned);
   private native long    _longArray     (long    v[],  boolean unsigned);
   private native long    _stringArray   (String  v[]);
   private native long    _booleanArray  (boolean v[]);
   private native long    _floatArray    (float   v[]);
   private native long    _doubleArray   (double  v[]);
   private native long    _copArray      (long    v[]);
   private native long    _datetimeArray (long    v[]);
   private native long    _objectArray   (long    v[]);
   private native long    _char16Array   (char    v[]);

   private native Object  _getValue      (long    v) throws CIMException;
   private native int     _getType       (long    v);
   private native boolean _isArray       (long    v);
   private native String  _toString      (long    v);

   private native void    _finalize      (long    cv);

   protected void finalize ()
   {
      _finalize (cInst);
   }

   // This constructor should be public as well.
   public CIMValue (long v)
   {
      cInst = v;
   }

   public CIMValue (Object o) throws CIMException
   {
      cInst = calculateCInst (o);
   }

   public CIMValue (Object o, CIMDataType type)
      throws CIMException
   {
      cInst = calculateCInst (o);
   }

   private long calculateCInst (Object o)
      throws CIMException
   {
      long ciRet = 0;

      if (o == null)
         return 0;

      if (o instanceof Vector)
      {
         Vector v    = (Vector)o;
         Object o0   = v.elementAt (0);
         int    size = v.size ();

         if (o0 instanceof Number)
         {
            if (o0 instanceof UnsignedInt8)
            {
               short[] u8=new short[size];

               for (int i=0; i<size; i++)
                  u8[i]=((UnsignedInt8)v.elementAt(i)).shortValue ();

               ciRet = _byteArray (u8,true);
            }
            else if (o0 instanceof Byte)
            {
               short[] s8=new short[size];

               for (int i=0; i<size; i++)
                  s8[i]=((Byte)v.elementAt (i)).shortValue ();

               ciRet = _byteArray (s8,false);
            }
            else if (o0 instanceof UnsignedInt16)
            {
               int[] u16=new int[size];

               for (int i=0; i<size; i++)
                  u16[i]=((UnsignedInt16)v.elementAt (i)).intValue ();

               ciRet = _shortArray (u16,true);
            }
            else if (o0 instanceof Short)
            {
               int[] s16=new int[size];

               for (int i=0; i<size; i++)
                  s16[i]=((Short)v.elementAt (i)).intValue ();

               ciRet = _shortArray (s16,false);
            }
            else if (o0 instanceof UnsignedInt32)
            {
               long[] u32=new long[size];

               for (int i=0; i<size; i++)
                  u32[i]=((UnsignedInt32)v.elementAt (i)).longValue ();

               ciRet = _intArray (u32,true);
            }
            else if (o0 instanceof Integer)
            {
               long[] s32=new long[size];

               for (int i=0; i<size; i++)
                  s32[i]=((Integer)v.elementAt (i)).longValue ();

               ciRet = _intArray (s32,false);
            }
            else if (o0 instanceof UnsignedInt64)
            {
               long u64[]=new long[size];

               for (int i=0; i<size; i++)
                  u64[i]=((UnsignedInt64)v.elementAt (i)).longValue ();

               ciRet = _longArray (u64,true);
            }
            else if (o0 instanceof Long)
            {
               long s64[]=new long[size];

               for (int i=0; i<size; i++)
                  s64[i]=((Long)v.elementAt (i)).longValue ();

               ciRet = _longArray (s64,false);
            }
            else if (o0 instanceof Float)
            {
               float f[]=new float[size];

               for (int i=0; i<size; i++)
                  f[i]=((Float)v.elementAt (i)).floatValue ();

               ciRet = _floatArray (f);
            }
            else if (o0 instanceof Double)
            {
               double d[]=new double[size];

               for (int i=0; i<size; i++)
                  d[i]=((Double)v.elementAt (i)).doubleValue ();

               ciRet = _doubleArray (d);
            }
         }
         else if (o0 instanceof Boolean)
         {
            boolean b[]=new boolean[size];

            for (int i=0; i<size; i++)
                b[i]=((Boolean)v.elementAt (i)).booleanValue ();

            ciRet = _booleanArray (b);
         }
         else if (o0 instanceof String)
         {
            String s[]=new String[size];

            v.copyInto(s);

            ciRet = _stringArray (s);
         }
         else if (o0 instanceof CIMObjectPath)
         {
            long c[]=new long[size];

            for (int i=0; i<size; i++)
               c[i]=((CIMObjectPath)v.elementAt (i)).cInst ();

            ciRet = _copArray (c);
         }
         else if (o0 instanceof CIMDateTime)
         {
             long c[]=new long[size];

             for (int i=0; i<size; i++)
                c[i]=((CIMDateTime)v.elementAt (i)).cInst ();

             ciRet = _datetimeArray (c);
         }
         else if (o0 instanceof CIMObject)
         {
             long c[]=new long[size];

             for (int i=0; i<size; i++)
                c[i]=((CIMObject)v.elementAt (i)).cInst ();

             ciRet = _objectArray (c);
         }
         else if (o0 instanceof Character)
         {
             char c[]=new char[size];

             for (int i=0; i<size; i++)
                c[i]=((Character)v.elementAt (i)).charValue ();

             ciRet = _char16Array (c);
         }
      }
      else if (o instanceof Number)
      {
          if (o instanceof UnsignedInt8)
          {
             ciRet = _byte(((UnsignedInt8)o).shortValue (),true);
          }
          else if (o instanceof Byte)
          {
             ciRet = _byte(((Byte)o).byteValue (),false);
          }
          else if (o instanceof UnsignedInt16)
          {
             ciRet = _short(((UnsignedInt16)o).intValue (),true);
          }
          else if (o instanceof Short)
          {
             ciRet = _short(((Short)o).shortValue (),false);
          }
          else if (o instanceof UnsignedInt32)
          {
             ciRet = _makeInt(((UnsignedInt32)o).longValue (),true);
          }
          else if (o instanceof Integer)
          {
             ciRet = _makeInt(((Integer)o).intValue (),false);
          }
          else if (o instanceof UnsignedInt64)
          {
             ciRet = _long(((UnsignedInt64)o).longValue (),true);
          }
          else if (o instanceof Long)
          {
             ciRet = _long(((Long)o).longValue (),false);
          }
          else if (o instanceof Float)
          {
             ciRet = _float(((Float)o).floatValue ());
          }
          else if (o instanceof Double)
          {
             ciRet = _double(((Double)o).doubleValue ());
          }
      }
      else if (o instanceof Boolean)
      {
         ciRet = _boolean(((Boolean)o).booleanValue ());
      }
      else if (o instanceof String)
      {
         ciRet = _string((String)o);
      }
      else if (o instanceof CIMObjectPath)
      {
         ciRet = _cop(((CIMObjectPath)o).cInst ());
      }
      else if (o instanceof CIMDateTime)
      {
         ciRet = _datetime(((CIMDateTime)o).cInst ());
      }
      else if (o instanceof Character)
      {
         ciRet = _char16(((Character)o).charValue ());
      }
      else if (o instanceof CIMObject)
      {
         ciRet = _object(((CIMObject)o).cInst ());
      }

      if (ciRet == 0)
      {
          if (o instanceof Vector)
          {
              Vector v  = (Vector)o;
              Object o0 = v.elementAt (0);

              throw new CIMException (1, "CIMValue: unsupported type: Vector of "+o0.getClass());
          }
          else
          {
              throw new CIMException (1, "CIMValue: unsupported type: "+o.getClass());
          }
      }

      return ciRet;
   }

   public Object getValue ()
      throws CIMException
   {
      if (cInst == 0)
         return null;

      return getValue (true);
   }

   public Object getValue (boolean toVector)
      throws CIMException
   {
      if (cInst == 0)
         return null;

      Object resp = null;
      Object o=null;

      try
      {
         o = _getValue (cInst);

         if (o == null)
            return null;

         int     type    = _getType (cInst);
         boolean isArray = _isArray (cInst);

         if (isArray)
         {
            switch (type)
            {
            case CIMDataType.BOOLEAN:
            {
               Boolean b[]=(Boolean[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<b.length;i++)
                     ((Vector)resp).addElement(b[i]);
               }
               else
               {
                   resp = b;
               }
               break;
            }
            case CIMDataType.SINT8:
            {
               Byte s8[]=(Byte[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<s8.length;i++)
                     ((Vector)resp).addElement(s8[i]);
               }
               else
               {
                   resp = s8;
               }
               break;
            }
            case CIMDataType.UINT8:
            {
               UnsignedInt8 u8[]=(UnsignedInt8[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<u8.length;i++)
                     ((Vector)resp).addElement(u8[i]);
               }
               else
               {
                   resp = u8;
               }
               break;
            }
            case CIMDataType.SINT16:
            {
               Short s16[]=(Short[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<s16.length;i++)
                     ((Vector)resp).addElement(s16[i]);
               }
               else
               {
                   resp = s16;
               }
               break;
            }
            case CIMDataType.UINT16:
            {
               UnsignedInt16 u16[]=(UnsignedInt16[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<u16.length;i++)
                     ((Vector)resp).addElement(u16[i]);
               }
               else
               {
                   resp = u16;
               }
               break;
            }
            case CIMDataType.SINT32:
            {
               Integer s32[]=(Integer[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<s32.length;i++)
                     ((Vector)resp).addElement(s32[i]);
               }
               else
               {
                   resp = s32;
               }
               break;
            }
            case CIMDataType.UINT32:
            {
               UnsignedInt32 u32[]=(UnsignedInt32[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<u32.length;i++)
                     ((Vector)resp).addElement(u32[i]);
               }
               else
               {
                   resp = u32;
               }
               break;
            }
            case CIMDataType.SINT64:
            {
               Long s64[]=(Long[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<s64.length;i++)
                     ((Vector)resp).addElement(s64[i]);
               }
               else
               {
                   resp = s64;
               }
               break;
            }
            case CIMDataType.UINT64:
            {
               UnsignedInt64 u64[]=(UnsignedInt64[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<u64.length;i++)
                     ((Vector)resp).addElement(u64[i]);
               }
               else
               {
                   resp = u64;
               }
               break;
            }
            case CIMDataType.STRING:
            {
               String s[]=(String[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<s.length;i++)
                     ((Vector)resp).addElement(s[i]);
               }
               else
               {
                   resp = s;
               }
               break;
            }
            case CIMDataType.REAL32:
            {
               Float f[]=(Float[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<f.length;i++)
                     ((Vector)resp).addElement(f[i]);
               }
               else
               {
                   resp = f;
               }
               break;
            }
            case CIMDataType.REAL64:
            {
               Double d[]=(Double[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<d.length;i++)
                     ((Vector)resp).addElement(d[i]);
               }
               else
               {
                   resp = d;
               }
               break;
            }
            case CIMDataType.DATETIME:
            {
               CIMDateTime cdt[]=(CIMDateTime[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<cdt.length;i++)
                     ((Vector)resp).addElement(cdt[i]);
               }
               else
               {
                   resp = cdt;
               }
               break;
            }
            case CIMDataType.CHAR16:
            {
               Character c[]=(Character[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<c.length;i++)
                     ((Vector)resp).addElement(c[i]);
               }
               else
               {
                   resp = c;
               }
               break;
            }
            case CIMDataType.REFERENCE:
            {
               CIMObjectPath cop[]=(CIMObjectPath[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<cop.length;i++)
                     ((Vector)resp).addElement(cop[i]);
               }
               else
               {
                   resp = cop;
               }
               break;
            }
            case CIMDataType.OBJECT:
            {
               CIMObject co[]=(CIMObject[])o;

               if (toVector)
               {
                  resp = new Vector();
                  for (int i=0;i<co.length;i++)
                     ((Vector)resp).addElement(co[i]);
               }
               else
               {
                   resp = co;
               }
               break;
            }
            }
         }
         else
         {
             resp = o;
         }
      }
      catch (CIMException ce)
      {
         throw ce;
      }
      catch (Exception e)
      {
/////////e.printStackTrace ();
         throw new CIMException (1, e.getMessage ());
      }

      return resp;
   }

   public boolean isArray ()
   {
      if (cInst == 0)
         return false;

      return _isArray (cInst);
   }

   public String toString ()
   {
      if (cInst == 0)
         return null;

      return _toString (cInst);
   }

   protected long cInst ()
   {
      return cInst;
   }

   static {
      System.loadLibrary ("JMPIProviderManager");
   }
}
