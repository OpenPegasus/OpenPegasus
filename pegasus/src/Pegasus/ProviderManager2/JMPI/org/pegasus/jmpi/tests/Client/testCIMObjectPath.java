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

public class testCIMObjectPath
{
   private boolean DEBUG = false;

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "CIMObjectPath";
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
         fRet = testCIMObjectPath (cimClient);
      }

      return fRet;
   }

   private boolean testCIMObjectPath (CIMClient client)
   {
      if (client == null)
      {
         System.out.println ("FAILURE: testCIMObjectPath: client == null");

         return false;
      }

      // -----

      String        pszHostName    = "localhost";
      String        pszClassName   = "JMPIExpInstance_TestPropertyTypes";
      String        pszNamespace   = "root/SampleProvider";
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
         System.out.println ("FAILURE: testCIMObjectPath: caught " + e + ", in vectorPropsCop");

         return false;
      }

      cop = new CIMObjectPath (pszClassName,
                               vectorPropsCop);
      cop.setNameSpace (pszNamespace);

      // -----

      if (DEBUG)
      {
         System.out.println ("testCIMObjectPath: cop = " + cop);
      }

      if (cop == null)
      {
         System.out.println ("FAILURE: testCIMObjectPath: cop == null");

         return false;
      }

      CIMInstance ci = null;

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

      if (DEBUG)
      {
         System.out.println ("testCIMInstance: ci = " + ci);
      }

      if (ci == null)
      {
         System.out.println ("FAILURE: testCIMInstance: ci == null");

         return false;
      }

      // -----

      String retHostName = null;

      retHostName = cop.getHost ();

      if (DEBUG)
      {
         System.out.println ("testCIMObjectPath: retHostName = " + retHostName);
      }

      if (retHostName == null)
      {
         System.out.println ("FAILURE: testCIMObjectPath: cop.getHost ()");

         return false;
      }

      // -----

      cop.setHost (pszHostName);

      retHostName = cop.getHost ();

      if (DEBUG)
      {
         System.out.println ("testCIMObjectPath: retHostName = " + retHostName);
      }

      if (  retHostName == null
          || !retHostName.equals (pszHostName)
         )
      {
         System.out.println ("FAILURE: testCIMObjectPath: cop.setHost/getHost ()");

         return false;
      }

      // -----

      String pszValueRet = null;

      pszValueRet = cop.getKeyValue ("InstanceId");

      if (DEBUG)
      {
         System.out.println ("testCIMObjectPath: pszValueRet = " + pszValueRet);
      }

      if (  pszValueRet == null
         || !pszValueRet.equals ("1")
         )
      {
         System.out.println ("FAILURE: testCIMObjectPath: cop.getKeyValue ()");

         return false;
      }

      // -----

      Vector keys = null;

      keys = cop.getKeys ();

      if (DEBUG)
      {
         if (keys == null)
         {
            System.out.println ("testCIMObjectPath: keys = null");
         }
         else
         {
            System.out.println ("testCIMObjectPath: keys.size () = " + keys.size ());
         }
      }

      if (  keys == null
         || keys.size () != 2
         )
      {
         System.out.println ("FAILURE: testCIMObjectPath: cop.getKeyValuePairs ()");

         return false;
      }

      // -----

      CIMValue cv = null;

      try
      {
         cv = new CIMValue (new UnsignedInt64 ("88"));

         if (DEBUG)
         {
            System.out.println ("testCIMObjectPath: cv = " + cv);
         }

         cop.addKey ("bob", cv);

         if (DEBUG)
         {
            System.out.println ("testCIMObjectPath: cop = " + cop);
         }
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMObjectPath: cop.addKey (): caught " + e);

         return false;
      }

      // -----

      Vector vectorNewKeys = new Vector ();

      try
      {
         CIMProperty cp = null;

         cv = new CIMValue (new UnsignedInt64 ("1"));
         cp = new CIMProperty ("Bob1",
                               cv);
         vectorNewKeys.addElement (cp);

         cv = new CIMValue (new UnsignedInt64 ("2"));
         cp = new CIMProperty ("Bob2",
                               cv);
         vectorNewKeys.addElement (cp);

         cv = new CIMValue (new UnsignedInt64 ("3"));
         cp = new CIMProperty ("Bob3",
                               cv);
         vectorNewKeys.addElement (cp);

         cv = new CIMValue (new UnsignedInt64 ("4"));
         cp = new CIMProperty ("Bob4",
                               cv);
         vectorNewKeys.addElement (cp);

         cop.setKeys (vectorNewKeys);

         if (DEBUG)
         {
            System.out.println ("testCIMObjectPath: cop = " + cop);
         }

         vectorNewKeys = cop.getKeys ();

         if (  vectorNewKeys == null
            || vectorNewKeys.size () != 4
            )
         {
            System.out.println ("FAILURE: testCIMObjectPath: cop.setKeys ()");

            return false;
         }
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMObjectPath: cop.addKey (): caught " + e);

         return false;
      }

      // -----

      String pszNamespaceRet = null;

      pszNamespaceRet = cop.getNameSpace ();

      if (  pszNamespaceRet == null
         || !pszNamespace.equals (pszNamespaceRet)
         )
      {
         System.out.println ("FAILURE: testCIMObjectPath: cop.getNameSpace ()");

         return false;
      }

      // -----

      String pszNewNameSpace    = "root/bob";
      String pszNewNameSpaceRet = null;

      cop.setNameSpace (pszNewNameSpace);

      pszNewNameSpaceRet = cop.getNameSpace ();

      if (DEBUG)
      {
         System.out.println ("testCIMObjectPath: pszNewNameSpaceRet = " + pszNewNameSpaceRet);
      }

      if (  pszNewNameSpaceRet == null
         || !pszNewNameSpaceRet.equals (pszNewNameSpace)
         )
      {
         System.out.println ("FAILURE: testCIMObjectPath: cop.setNameSpace/getNameSpace ()");

         return false;
      }

      // -----

      String pszClassNameRet = null;

      pszClassNameRet = cop.getObjectName ();

      if (  pszClassNameRet == null
         || !pszClassName.equals (pszClassNameRet)
         )
      {
         System.out.println ("FAILURE: testCIMObjectPath: cop.getObjectName ()");

         return false;
      }

      // -----

      String pszNewClassName    = "bob";
      String pszNewClassNameRet = null;

      cop.setObjectName (pszNewClassName);

      pszNewClassNameRet = cop.getObjectName ();

      if (DEBUG)
      {
         System.out.println ("testCIMObjectPath: pszNewClassNameRet = " + pszNewClassNameRet);
      }

      if (  pszNewClassNameRet == null
         || !pszNewClassNameRet.equals (pszNewClassName)
         )
      {
         System.out.println ("FAILURE: testCIMObjectPath: cop.setObjectName/getObjectName ()");

         return false;
      }

      // -----

      cop = new CIMObjectPath ();

      if (DEBUG)
      {
         System.out.println ("testCIMObjectPath: cop = " + cop);
      }

      if (cop == null)
      {
         System.out.println ("FAILURE: testCIMObjectPath: new CIMObjectPath ()");

         return false;
      }

      // -----

      cop = new CIMObjectPath (pszClassName);

      if (DEBUG)
      {
         System.out.println ("testCIMObjectPath: cop = " + cop);
      }

      if (cop == null)
      {
         System.out.println ("FAILURE: testCIMObjectPath: new CIMObjectPath (pszClassName)");

         return false;
      }

      // -----

      cop = new CIMObjectPath (pszClassName,
                               pszNamespace);

      if (DEBUG)
      {
         System.out.println ("testCIMObjectPath: cop = " + cop);
      }

      if (cop == null)
      {
         System.out.println ("FAILURE: testCIMObjectPath: new CIMObjectPath (pszClassName, pszNamespace)");

         return false;
      }

      // -----

      cop = new CIMObjectPath (ci);

      if (DEBUG)
      {
         System.out.println ("testCIMObjectPath: cop = " + cop);
      }

      if (cop == null)
      {
         System.out.println ("FAILURE: testCIMObjectPath: new CIMObjectPath (ci)");

         return false;
      }

      // -----

      cop = new CIMObjectPath (ci, cop.getNameSpace ());

      if (DEBUG)
      {
         System.out.println ("testCIMObjectPath: cop = " + cop);
      }

      if (cop == null)
      {
         System.out.println ("FAILURE: testCIMObjectPath: new CIMObjectPath (ci)");

         return false;
      }

      // -----

      System.out.println ("SUCCESS: testCIMObjectPath");

      return true;
   }
}
