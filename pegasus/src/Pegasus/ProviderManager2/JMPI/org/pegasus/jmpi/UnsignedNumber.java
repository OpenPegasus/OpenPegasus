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
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

public class UnsignedNumber
       extends Number
{
    private long number;
    private long mask;

    public UnsignedNumber (long val, long mask, String msg)
       throws NumberFormatException
    {
       this.number = val;
       this.mask   = mask;

       if ((number & ~mask) != 0)
          throw new NumberFormatException (msg);
       else if (number < 0L)
          throw new NumberFormatException (msg);
    }

    public UnsignedNumber (String str, long mask, String msg)
       throws NumberFormatException
    {
       Long l = new Long (str);

       this.mask   = mask;
       this.number = l.longValue ();

       if ((number & ~mask) != 0)
          throw new NumberFormatException (msg);
       else if (number < 0L)
          throw new NumberFormatException (msg);
    }

    public String toString ()
    {
        return "" + (number & mask);
    }

    public int hashCode ()
    {
        return (int)(number & mask);
    }

    public boolean equals (Object o)
    {
        if (o instanceof UnsignedNumber)
           return (((UnsignedNumber)o).number & ((UnsignedNumber)o).mask) == (number & mask);

        return false;
    }

    public int intValue ()
    {
        return (int)(number & mask);
    }

    public long longValue ()
    {
        return (long)(number & mask);
    }

    public float floatValue ()
    {
        return (float)(number & mask);
    }

    public double doubleValue ()
    {
        return (double)(number & mask);
    }

    public byte byteValue ()
    {
        return (byte)(number & mask);
    }

    public short shortValue ()
    {
        return (short) (number & mask);
    }
}
