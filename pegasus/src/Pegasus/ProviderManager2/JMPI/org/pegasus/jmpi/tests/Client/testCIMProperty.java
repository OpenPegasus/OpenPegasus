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
import org.pegasus.jmpi.CIMDataType;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMInstance;
import org.pegasus.jmpi.CIMObjectPath;
import org.pegasus.jmpi.CIMProperty;
import org.pegasus.jmpi.CIMValue;
import org.pegasus.jmpi.UnsignedInt64;
import java.util.Vector;

public class testCIMProperty
{
   private boolean DEBUG = false;

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "CIMProperty";
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
         fRet = testCIMProperty (cimClient);
      }

      return fRet;
   }

   private boolean testCIMProperty (CIMClient client)
   {
      if (client == null)
      {
         System.out.println ("FAILURE: testCIMProperty: client == null");

         return false;
      }

      // -----

      String        pszClassName   = "JMPIExpInstance_TestPropertyTypes";
      CIMInstance   ci             = null;
      CIMObjectPath cop            = null;
      Vector        vectorPropsCop = new Vector ();

      try
      {
         CIMProperty cp = null;
         CIMValue    cv = null;

         cv = new CIMValue (new String (pszClassName));
         cp = new CIMProperty ("CreationClassName",
                               cv);
         vectorPropsCop.addElement (cp);

         cv = new CIMValue (new UnsignedInt64 ("1"));
         cp = new CIMProperty ("InstanceId",
                               cv);
         vectorPropsCop.addElement (cp);
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testCIMProperty: caught " + e + ", in vectorPropsCop");

         return false;
      }

      cop = new CIMObjectPath (pszClassName,
                               vectorPropsCop);
      cop.setNameSpace ("root/SampleProvider");

      if (DEBUG)
      {
         System.out.println ("testCIMProperty: cop = " + cop);
      }

      try
      {
         ci = client.getInstance (cop,
                                  true,  // localOnly
                                  true,  //
                                  true,  // includeClassOrigin
                                  null); // propertyList
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMProperty: client.getInstance: caught " + e);

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMProperty: ci = " + ci);
      }

      if (ci == null)
      {
         System.out.println ("FAILURE: testCIMProperty: ci == null");

         return false;
      }

      // -----

      CIMProperty cp              = null;
      String      pszPropertyName = "PropertyString";

      cp = ci.getProperty (pszPropertyName);

      if (cp == null)
      {
         System.out.println ("FAILURE: testCIMProperty: ci.getProperty (String)");

         return false;
      }

      // -----

      String retPropertyName = null;

      retPropertyName = cp.getName ();

      if (  retPropertyName == null
         || !retPropertyName.equals (pszPropertyName)
         )
      {
         System.out.println ("FAILURE: testCIMProperty: cp.getName ()");

         return false;
      }

      // -----

      String pszNewName = "bob";
      CIMValue cimValue = null;
      CIMProperty newNameProperty = null;
      try
      {
          cimValue = new CIMValue (new UnsignedInt64("1"));
          newNameProperty = new CIMProperty ("fred", cimValue);
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testCIMProperty: caught " + e + ", in CIMValue and CIMProperty creation.");

         return false;
      }

      newNameProperty.setName (pszNewName);

      if (DEBUG)
      {
         System.out.println ("testCIMProperty: newNameProperty = " + newNameProperty);
      }

      retPropertyName = newNameProperty.getName ();

      if (  retPropertyName == null
         || !retPropertyName.equals (pszNewName)
         )
      {
         System.out.println ("FAILURE: testCIMProperty: retPropertyName.setName ()");

         return false;
      }

      // -----

      CIMDataType cdt = null;

      cdt = cp.getType ();

      if (DEBUG)
      {
         System.out.println ("testCIMProperty: cdt = " + cdt);
      }

      if (  cdt == null
         || cdt.getType () != CIMDataType.STRING
         || cdt.isArrayType () != false
         || cdt.getSize () != 0
         )
      {
         System.out.println ("FAILURE: testCIMProperty: cp.getType ()");

         return false;
      }

      // -----

      Object oValue = null;

      try
      {
         oValue = cp.getValue ().getValue ();
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testCIMProperty: cp.getValue (): caught " + e);

         return false;
      }

      if (  oValue == null
         || !(oValue instanceof String)
         )
      {
         System.out.println ("FAILURE: testCIMProperty: cp.getValue ()");

         return false;
      }

      // -----

      CIMValue cv = null;

      try
      {
         Vector cva = new Vector ();

         cva.addElement (new Integer (1));
         cva.addElement (new Integer (2));
         cva.addElement (new Integer (3));
         cv = new CIMValue (cva);

         if (DEBUG)
         {
            System.out.println ("testCIMProperty: cv = " + cv);
         }

         if (cv == null)
         {
            System.out.println ("FAILURE: testCIMProperty: cp.addValue (): cv == null");

            return false;
         }

         cp = new CIMProperty ("bob", cv);

         if (DEBUG)
         {
            System.out.println ("testCIMProperty: cp = " + cp);
            System.out.println ("testCIMProperty: cp.isArray () = " + cp.isArray ());
         }

         if (cp == null)
         {
            System.out.println ("FAILURE: testCIMProperty: cp.addValue (): cp == null");

            return false;
         }

         if (!cp.isArray ())
         {
            System.out.println ("FAILURE: testCIMProperty: cp.addValue (): !cp.isArray");

            return false;
         }

         cv = new CIMValue (new Integer (4));

         if (DEBUG)
         {
            System.out.println ("testCIMProperty: cv = " + cv);
         }

         if (cv == null)
         {
            System.out.println ("FAILURE: testCIMProperty: cp.addValue (): cv == null");

            return false;
         }

         cp.addValue (cv);

         if (DEBUG)
         {
            System.out.println ("testCIMProperty: cp = " + cp);
         }
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testCIMProperty: cp.addValue (): caught " + e);

         return false;
      }

      // -----

      try
      {
         cv = new CIMValue (cop);

         if (DEBUG)
         {
            System.out.println ("testCIMProperty: cv = " + cv);
         }

         if (cv == null)
         {
            System.out.println ("FAILURE: testCIMProperty: cp.getRefClassName (): cv == null");

            return false;
         }

         cp = new CIMProperty ("bob", cv);

         if (DEBUG)
         {
            System.out.println ("testCIMProperty: cp = " + cp);
         }

         if (cp == null)
         {
            System.out.println ("FAILURE: testCIMProperty: cp.getRefClassName (): cp == null");

            return false;
         }

         if (!cp.isReference ())
         {
            System.out.println ("FAILURE: testCIMProperty: cp.getRefClassName (): !cp.isReference");

            return false;
         }

         String pszRefClassName = null;

         pszRefClassName = cp.getRefClassName ();

         if (DEBUG)
         {
            System.out.println ("testCIMProperty: pszRefClassName = " + pszRefClassName);
         }

         if (  pszRefClassName == null
            || !pszRefClassName.equals (pszClassName)
            )
         {
            System.out.println ("FAILURE: testCIMProperty: cp.getRefClassName (): !pszRefClassName.equals (pszClassName)");

            return false;
         }
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testCIMProperty: cp.getRefClassName (): caught " + e);

         return false;
      }

      // -----

      System.out.println ("SUCCESS: testCIMProperty");

      return true;
   }
}
