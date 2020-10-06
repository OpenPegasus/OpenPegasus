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

import java.math.BigInteger;

/**
 * The UnsignedInt64 class wraps the value of an uint64. This
 * class was created to represent an uint64 data type as defined
 * by the CIM Infrastructure Specification. The specification is
 * available from the DMTF (Distributed Management Task Force)
 * at http://dmtf.org/
 */
public class UnsignedInt64
       extends BigInteger
       implements java.io.Serializable,
                  java.lang.Comparable
{
   /**
    * The minimum value of an UnsignedInt64.
    */
   public static final BigInteger UINT64MIN = new BigInteger ("0");
   /**
    * The maximum value of an UnsignedInt64.
    */
   public static final BigInteger UINT64MAX = new BigInteger ("18446744073709551615");
   /**
    * The mask to cover possible values.
    */
   public static final BigInteger MAXMASK   = new BigInteger ("ffffffffffffffff", 16);

   protected BigInteger bi;

   /**
    * Constructs an unsigned 64-bit integer object for the specified
    * BigInteger. Only the lower 64 bits are considered.
    *
    * @param val - the BigInteger to be represented as an unsigned 64-bit
    * integer.
    * @throws NumberFormatException - if the number is out of range.
    */
   public UnsignedInt64 (BigInteger val)
      throws NumberFormatException
   {
      super (val.and (MAXMASK).toString ());

      bi = val;

      if (  val.compareTo (BigInteger.ZERO) < 0
         || val.compareTo (UINT64MAX) > 0
         )
         throw new NumberFormatException (val + " Not an unsigned 64 bit integer");
   }

   /**
    * Constructs an unsigned 64-bit integer object from the specified string.
    * Only the lower 64 bits are considered.
    *
    * @param value - the String to be represented as an unsigned 64-bit
    * integer.
    * @throws NumberFormatException - if the number is out of range.
    */
   public UnsignedInt64 (String str)
      throws NumberFormatException
   {
      super (str);

      bi = new BigInteger (str);

      if (  bi.compareTo (BigInteger.ZERO) < 0
         || bi.compareTo (UINT64MAX) > 0
         )
         throw new java.lang.NumberFormatException (bi + " Not an unsigned 64 bit integer");
   }

   /**
    * Compares this UnsignedInt64 with the specified UnsignedInt64.
    * This method is provided in preference to individual methods
    * for each of the six boolean comparison operators (<, ==, >,
    * >=, !=, <=). The suggested idiom for performing these
    * comparisons is: (x.compareTo(y) <op> 0), where <op>  is one
    * of the six comparison operators.
    *
    * @param val - Object to which this UnsignedInt64 is to be
    * compared. Throws a ClassCastException if the input object is
    * not an UnsignedInt32.
    * @return -1, 0 or 1 as this UnsignedInt64 is numerically
    * less than, equal to, or greater than val.
    */
   public int compareTo (Object val)
      throws ClassCastException
   {
      if (!(val instanceof UnsignedInt64))
         throw new ClassCastException ();

      UnsignedInt64 that = (UnsignedInt64)val;

      return this.bi.compareTo (that.bi);
   }
}
