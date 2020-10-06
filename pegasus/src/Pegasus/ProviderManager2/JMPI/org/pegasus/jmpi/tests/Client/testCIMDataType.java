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

import org.pegasus.jmpi.CIMClass;
import org.pegasus.jmpi.CIMDataType;
import org.pegasus.jmpi.CIMDateTime;
import org.pegasus.jmpi.CIMObjectPath;
import org.pegasus.jmpi.UnsignedInt8;
import org.pegasus.jmpi.UnsignedInt16;
import org.pegasus.jmpi.UnsignedInt32;
import org.pegasus.jmpi.UnsignedInt64;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMClient;

public class testCIMDataType
{
   private boolean DEBUG = false;

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "CIMDataType";
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
         fRet = testCIMDataType (cimClient);
      }

      return fRet;
   }

   private boolean testCIMDataType (CIMClient client)
   {
      if (client == null)
      {
         System.out.println ("FAILURE: testCIMDataType: client == null");

         return false;
      }

      // -----

      CIMObjectPath cop = null;
      CIMClass      cc  = null;

      cop = new CIMObjectPath ("JMPIExpInstance_TestPropertyTypes",
                               "root/SampleProvider");

      if (DEBUG)
      {
         System.out.println ("testCIMDataType: cop = " + cop);
      }

      try
      {
         cc = client.getClass (cop,
                               true,  // propagated
                               true,  // includeQualifiers
                               true,  // includeClassOrigin
                               null); // propertyList
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMDataType: client.getClass: caught " + e);

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMDataType: cc = " + cc);
      }

      if (cc == null)
      {
         System.out.println ("FAILURE: testCIMDataType: cc == null");

         return false;
      }

      // -----

      String  aszPropertyNames[] = {
         "PropertyString",
         "PropertyUint8",
         "PropertyUint16",
         "PropertyUint32",
         "PropertyUint64",
         "PropertySint8",
         "PropertySint16",
         "PropertySint32",
         "PropertySint64",
         "PropertyBoolean",
         "PropertyReal32",
         "PropertyReal64",
         "PropertyDatetime",
         "PropertyChar16",
         "PropertyArrayUint8",
         "PropertyArrayUint16",
         "PropertyArrayUint32",
         "PropertyArrayUint64",
         "PropertyArraySint8",
         "PropertyArraySint16",
         "PropertyArraySint32",
         "PropertyArraySint64",
         "PropertyArrayBoolean",
         "PropertyArrayReal32",
         "PropertyArrayReal64",
         "PropertyArrayDatetime",
         "PropertyArrayChar16"
      };
      int     aiDataTypes[] = {
         CIMDataType.STRING,
         CIMDataType.UINT8,
         CIMDataType.UINT16,
         CIMDataType.UINT32,
         CIMDataType.UINT64,
         CIMDataType.SINT8,
         CIMDataType.SINT16,
         CIMDataType.SINT32,
         CIMDataType.SINT64,
         CIMDataType.BOOLEAN,
         CIMDataType.REAL32,
         CIMDataType.REAL64,
         CIMDataType.DATETIME,
         CIMDataType.CHAR16,
         CIMDataType.UINT8,
         CIMDataType.UINT16,
         CIMDataType.UINT32,
         CIMDataType.UINT64,
         CIMDataType.SINT8,
         CIMDataType.SINT16,
         CIMDataType.SINT32,
         CIMDataType.SINT64,
         CIMDataType.BOOLEAN,
         CIMDataType.REAL32,
         CIMDataType.REAL64,
         CIMDataType.DATETIME,
         CIMDataType.CHAR16
      };
      boolean afIsArrays[] = {
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         false,
         true,
         true,
         true,
         true,
         true,
         true,
         true,
         true,
         true,
         true,
         true,
         true,
         true,
      };
      int     aiArraySizes[] = {
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0,
         0
      };

      if (  aszPropertyNames.length != aiDataTypes.length
         || aiDataTypes.length != afIsArrays.length
         || afIsArrays.length != aiArraySizes.length
         || aiArraySizes.length != aszPropertyNames.length
         )
      {
         System.out.println ("FAILURE: testCIMDataType: arrays not the same length (1)");

         return false;
      }

      for ( int i = 0;
               i < aszPropertyNames.length
            && i < aiDataTypes.length
            && i < afIsArrays.length
            && i < aiArraySizes.length
            ;
            i++
          )
      {
         CIMDataType cdt = cc.getProperty (aszPropertyNames[i]).getType ();

         if (cdt.getType () != aiDataTypes[i])
         {
            System.out.println ("FAILURE: testCIMDataType: cdt.getType () for " + aszPropertyNames[i]);

            return false;
         }
         if (cdt.isArrayType () != afIsArrays[i])
         {
            System.out.println ("FAILURE: testCIMDataType: cdt.isArrayType () for " + aszPropertyNames[i]);

            return false;
         }
         if (cdt.getSize () != aiArraySizes[i])
         {
            System.out.println ("FAILURE: testCIMDataType: cdt.getSize () for " + aszPropertyNames[i]);

            return false;
         }
      }

      // -----

      System.out.println ("SUCCESS: testCIMDataType");

      return true;
   }
}
