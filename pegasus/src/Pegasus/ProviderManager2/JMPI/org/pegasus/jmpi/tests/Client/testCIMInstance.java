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
import org.pegasus.jmpi.CIMQualifier;
import org.pegasus.jmpi.CIMValue;
import org.pegasus.jmpi.UnsignedInt8;
import org.pegasus.jmpi.UnsignedInt64;
import java.util.Vector;

public class testCIMInstance
{
   private boolean DEBUG = false;

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "CIMInstance";
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
         fRet = testCIMInstance (cimClient);
      }

      return fRet;
   }

   private boolean testCIMInstance (CIMClient client)
   {
      if (client == null)
      {
         System.out.println ("FAILURE: testCIMInstance: client == null");

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
         System.out.println ("FAILURE: testCIMInstance: caught " + e + ", in vectorPropsCop");

         return false;
      }

      cop = new CIMObjectPath (pszClassName,
                               vectorPropsCop);
      cop.setNameSpace ("root/SampleProvider");

      if (DEBUG)
      {
         System.out.println ("testCIMInstance: cop = " + cop);
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
         System.out.println ("FAILURE: testCIMInstance: client.getInstance: caught " + e);

         return false;
      }

      if (ci == null)
      {
         System.out.println ("FAILURE: testCIMInstance: ci == null");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMInstance: ci = " + ci);
      }

      // -----

      if (!ci.getClassName ().equals (pszClassName))
      {
         System.out.println ("FAILURE: testCIMInstance: ci.getClassName ()");

         return false;
      }

      // -----

      Vector keys = null;

      keys = ci.getKeyValuePairs ();

      if (DEBUG)
      {
         if (keys == null)
         {
            System.out.println ("testCIMInstance: keys = null");
         }
         else
         {
            System.out.println ("testCIMInstance: keys.size () = " + keys.size ());
         }
      }

      if (  keys == null
         || keys.size () != 2
         )
      {
         System.out.println ("FAILURE: testCIMInstance: ci.getKeyValuePairs ()");

         return false;
      }

      // -----

      CIMObjectPath copRet = null;

      copRet = ci.getObjectPath ();

      if (copRet == null)
      {
         System.out.println ("FAILURE: testCIMInstance: ci.getObjectPath ()");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMInstance: copRet = " + copRet);
      }

      // -----

      ci.setObjectPath (copRet);

      // -----

      Vector props = null;

      props = ci.getProperties ();

      if (DEBUG)
      {
         if (props == null)
         {
            System.out.println ("testCIMInstance: props = null");
         }
         else
         {
            System.out.println ("testCIMInstance: props.size () = " + props.size ());
            System.out.println ("testCIMInstance: ci.getPropertyCount () = " + ci.getPropertyCount ());
         }
      }

      if (  props == null
         || props.size () != 15
         || ci.getPropertyCount () != 15
         )
      {
         System.out.println ("FAILURE: testCIMInstance: ci.getProperties ()");

         return false;
      }

      // -----

      for (int i = 0; i < ci.getPropertyCount (); i++)
      {
         CIMProperty cp = ci.getProperty (i);

         if (cp == null)
         {
            System.out.println ("FAILURE: testCIMInstance: ci.getProperty (" + i + ")");

            return false;
         }
      }

      // -----

      String pszNewName = "bob";

      ci.setName (pszNewName);

      if (!ci.getName ().equals (pszNewName))
      {
         System.out.println ("FAILURE: testCIMInstance: ci.setName ()");

         return false;
      }

      // -----

      try
      {
         String      szPropertyName  = "PropertyString";
         String      szPropertyValue = "bob";
         CIMValue    cv              = null;
         CIMProperty cp              = null;

         cv = new CIMValue (szPropertyValue);
         cp = new CIMProperty (szPropertyName, cv);

         ci.setProperty (szPropertyName, cv);

         cp = ci.getProperty (szPropertyName);

         if (!cp.getValue ().getValue ().equals (szPropertyValue))
         {
            System.out.println ("FAILURE: testCIMInstance: ci.setProperty ()");

            return false;
         }
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testCIMInstance: ci.setProperty (): caught " + e);

         return false;
      }

      // -----

      try
      {
         String      szPropertyName  = "PropertyUint8";
         String      szPropertyValue = "bob";
         CIMValue    cv              = null;
         CIMProperty cp              = null;

         cv = new CIMValue (szPropertyValue);
         cp = new CIMProperty (szPropertyName, cv);

         ci.setProperty (szPropertyName, cv);

         System.out.println ("FAILURE: testCIMInstance: expecting failure: ci.setProperty ()");

         return false;
      }
      catch (Exception e)
      {
      }

      // -----

      try
      {
         Vector        vectorNewProps       = new Vector ();
         String        szProperty1Name      = "PropertyUint8";
         UnsignedInt8  uint8Property1Value  = new UnsignedInt8 ((short)42);
         String        szProperty2Name      = "PropertyUint64";
         UnsignedInt64 uint64Property2Value = new UnsignedInt64 ("24");
         CIMValue      cv                   = null;
         CIMProperty   cp                   = null;

         cv = new CIMValue (uint8Property1Value);
         cp = new CIMProperty (szProperty1Name, cv);

         vectorNewProps.addElement (cp);

         cv = new CIMValue (uint64Property2Value);
         cp = new CIMProperty (szProperty2Name, cv);

         vectorNewProps.addElement (cp);

         ci.setProperty (vectorNewProps);

         cp = ci.getProperty (szProperty1Name);

         if (!cp.getValue ().getValue ().equals (uint8Property1Value))
         {
            System.out.println ("FAILURE: testCIMInstance: ci.setProperty (vector)");

            return false;
         }
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testCIMInstance: ci.setProperty (vector): caught " + e);

         return false;
      }

      // -----

      try
      {
         Vector        vectorNewProps       = new Vector ();
         String        szProperty1Name      = "PropertyUint8";
         UnsignedInt8  uint8Property1Value  = new UnsignedInt8 ((short)42);
         String        szProperty2Name      = "PropertyUint32";
         UnsignedInt64 uint64Property2Value = new UnsignedInt64 ("24");
         CIMValue      cv                   = null;
         CIMProperty   cp                   = null;

         cv = new CIMValue (uint8Property1Value);
         cp = new CIMProperty (szProperty1Name, cv);

         vectorNewProps.addElement (cp);

         cv = new CIMValue (uint64Property2Value);
         cp = new CIMProperty (szProperty2Name, cv);

         vectorNewProps.addElement (cp);

         ci.setProperty (vectorNewProps);

         System.out.println ("FAILURE: testCIMInstance: expecting failure: ci.setProperty (vector)");

         return false;
      }
      catch (Exception e)
      {
      }

      // -----

      String pszRetClassName = null;

      pszRetClassName = ci.getClassName ();

      if (  pszRetClassName == null
         || !pszRetClassName.equals (pszClassName)
         )
      {
         System.out.println ("FAILURE: testCIMInstance: ci.getClassName ()");

         return false;
      }

      // -----

      CIMQualifier cq = null;

      cq = ci.getQualifier ("Description");

      if (DEBUG)
      {
         System.out.println ("testCIMInstance: cq = " + cq);
      }

      if (cq == null)
      {
         System.out.println ("FAILURE: testCIMInstance: ci.getQualifier ()");

         return false;
      }

      // -----

      CIMInstance ci2 = null;

      ci2 = (CIMInstance)ci.clone ();

      if (DEBUG)
      {
         System.out.println ("testCIMInstance: ci2 = " + ci2);
      }

      if (ci2 == null)
      {
         System.out.println ("FAILURE: testCIMInstance: ci.clone ()");

         return false;
      }

      // -----

      System.out.println ("SUCCESS: testCIMInstance");

      return true;
   }
}
