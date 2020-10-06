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

import java.util.Vector;
import org.pegasus.jmpi.CIMClass;
import org.pegasus.jmpi.CIMClient;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMMethod;
import org.pegasus.jmpi.CIMObjectPath;
import org.pegasus.jmpi.CIMProperty;
import org.pegasus.jmpi.CIMQualifier;
import org.pegasus.jmpi.CIMValue;

public class testCIMClass
{
   private boolean DEBUG = false;

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "CIMClass";
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
         fRet = testCIMClass (cimClient);
      }

      return fRet;
   }

   private boolean testCIMClass (CIMClient client)
   {
      if (client == null)
      {
         System.out.println ("FAILURE: testCIMClass: client == null");

         return false;
      }

      // -----

      CIMObjectPath cop                       = null;
      String        pszPropertyTypesClassName = "JMPIExpInstance_TestPropertyTypes";
      String        pszElementClassName       = "JMPIExpInstance_TestElement";
      CIMClass      ccPropertyTypes           = null;
      CIMClass      ccElement                 = null;

      cop = new CIMObjectPath (pszPropertyTypesClassName,
                               "root/SampleProvider");

      if (DEBUG)
      {
         System.out.println ("testCIMClass: cop = " + cop);
      }

      try
      {
         ccPropertyTypes = client.getClass (cop,
                                            true,  // propagated
                                            true,  // includeQualifiers
                                            true,  // includeClassOrigin
                                            null); // propertyList
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMClass: client.getClass: caught " + e);

         return false;
      }

      if (ccPropertyTypes == null)
      {
         System.out.println ("FAILURE: testCIMClass: ccPropertyTypes == null");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMClass: ccPropertyTypes = " + ccPropertyTypes);
      }

      cop = new CIMObjectPath (pszElementClassName,
                               "root/SampleProvider");

      if (DEBUG)
      {
         System.out.println ("testCIMClass: cop = " + cop);
      }

      try
      {
         ccElement = client.getClass (cop,
                                      true,  // propagated
                                      true,  // includeQualifiers
                                      true,  // includeClassOrigin
                                      null); // propertyList
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMClass: client.getClass: caught " + e);

         return false;
      }

      if (ccPropertyTypes == null)
      {
         System.out.println ("FAILURE: testCIMClass: ccPropertyTypes == null");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMClass: ccPropertyTypes = " + ccPropertyTypes);
      }

      // -----

      String pszRetClassName = null;

      pszRetClassName = ccPropertyTypes.getName ();

      if (!pszRetClassName.equals (pszPropertyTypesClassName))
      {
         System.out.println ("FAILURE: testCIMClass: ccPropertyTypes.getName");

         return false;
      }

      // -----

      Vector vectorQualifiers = null;

      vectorQualifiers = ccPropertyTypes.getQualifiers ();

      if (DEBUG)
      {
         if (vectorQualifiers == null)
         {
            System.out.println ("testCIMClass: vectorQualifiers == null");
         }
         else
         {
            System.out.println ("testCIMClass: vectorQualifiers.size () = " + vectorQualifiers.size ());
            for (int i = 0; i < vectorQualifiers.size (); i++)
            {
               System.out.println ("testCIMClass: vectorQualifiers[" + i + "] = " + vectorQualifiers.elementAt (i));
            }
         }
      }

      if (  vectorQualifiers == null
         || vectorQualifiers.size () != 3
         )
      {
         System.out.println ("FAILURE: testCIMClass: ccPropertyTypes.getQualifiers");

         return false;
      }

      // -----

      for (int i = 0; i < vectorQualifiers.size (); i++)
      {
         CIMQualifier cq = (CIMQualifier)vectorQualifiers.elementAt (i);

         if (!ccPropertyTypes.hasQualifier (cq.getName ()))
         {
            System.out.println ("FAILURE: testCIMClass: ccPropertyTypes.hasQualifier (" + i + ")");

            return false;
         }
      }

      // -----

      Vector vectorProperties = null;

      vectorProperties = ccPropertyTypes.getProperties ();

      if (DEBUG)
      {
         if (vectorProperties == null)
         {
            System.out.println ("testCIMClass: vectorProperties == null");
         }
         else
         {
            System.out.println ("testCIMClass: vectorProperties.size () = " + vectorProperties.size ());
            for (int i = 0; i < vectorProperties.size (); i++)
            {
               System.out.println ("testCIMClass: vectorProperties[" + i + "] = " + vectorProperties.elementAt (i));
            }
         }
      }

      if (  vectorProperties == null
         || vectorProperties.size () != 29
         )
      {
         System.out.println ("FAILURE: testCIMClass: ccPropertyTypes.getProperties");

         return false;
      }

      // -----

      for (int i = 0; i < vectorProperties.size (); i++)
      {
         CIMProperty cp = (CIMProperty)vectorProperties.elementAt (i);

         if (ccPropertyTypes.getProperty (cp.getName ()) == null)
         {
            System.out.println ("FAILURE: testCIMClass: ccPropertyTypes.getProperty (" + i + ")");

            return false;
         }
      }

      // -----

      String      pszProperty = "bob";
      CIMValue    cvProperty  = null;
      CIMProperty cp          = null;

      try
      {
         cvProperty = new CIMValue (new Integer (42));
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMClass: cvProperty: caught " + e);

         return false;
      }

      cp = new CIMProperty (pszProperty, cvProperty);

      if (DEBUG)
      {
         System.out.println ("testCIMClass: cp = " + cp);
      }

      if (cp == null)
      {
         System.out.println ("FAILURE: testCIMClass: cp == null");

         return false;
      }

      try
      {
         ccPropertyTypes.addProperty (cp);
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMClass: ccPropertyTypes.addProperty (): not expecting exception" + e);

         return false;
      }

      if (ccPropertyTypes.getProperty (pszProperty) == null)
      {
         System.out.println ("FAILURE: testCIMClass: ccPropertyTypes.getProperty (pszProperty) == null");

         return false;
      }

      try
      {
         ccPropertyTypes.addProperty (cp);

         System.out.println ("FAILURE: testCIMClass: ccPropertyTypes.addProperty (): expecting exception");

         return false;
      }
      catch (CIMException e)
      {
      }

      // -----

      Vector newProperties = new Vector ();

      try
      {
         cvProperty = new CIMValue (new Integer (1));
         cp         = new CIMProperty ("newProp1", cvProperty);

         newProperties.addElement (cp);

         cvProperty = new CIMValue (new Integer (2));
         cp         = new CIMProperty ("newProp2", cvProperty);

         newProperties.addElement (cp);
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMClass: newProperties: caught " + e);

         return false;
      }

      ccPropertyTypes.setProperties (newProperties);

      // -----

      String pszSuperClass = null;

      pszSuperClass = ccPropertyTypes.getSuperClass ();

      if (DEBUG)
      {
         System.out.println ("testCIMClass: pszSuperClass = " + pszSuperClass);
      }

      if (pszSuperClass == null)
      {
         System.out.println ("FAILURE: testCIMClass: getSuperClass");

         return false;
      }

      // -----

      Vector vectorKeys = null;

      vectorKeys = ccElement.getKeys ();

      if (DEBUG)
      {
         if (vectorKeys == null)
         {
            System.out.println ("testCIMClass: vectorKeys == null");
         }
         else
         {
            System.out.println ("testCIMClass: vectorKeys.size () = " + vectorKeys.size ());
            for (int i = 0; i < vectorKeys.size (); i++)
            {
               System.out.println ("testCIMClass: vectorKeys[" + i + "] = " + vectorKeys.elementAt (i));
            }
         }
      }

      if (  vectorKeys == null
         || vectorKeys.size () != 2
         )
      {
         System.out.println ("FAILURE: testCIMClass: getKeys");

         return false;
      }

      // -----

      CIMMethod cm = null;

      cm = ccPropertyTypes.getMethod ("sayHello");

      if (DEBUG)
      {
         System.out.println ("testCIMClass: cm = " + cm);
      }

      if (cm == null)
      {
         System.out.println ("FAILURE: testCIMClass: getMethod (\"sayHello\")");

         return false;
      }

      cm = ccPropertyTypes.getMethod ("enableModifications");

      if (DEBUG)
      {
         System.out.println ("testCIMClass: cm = " + cm);
      }

      if (cm == null)
      {
         System.out.println ("FAILURE: testCIMClass: getMethod (\"enableModifications\")");

         return false;
      }

      cm = ccPropertyTypes.getMethod ("bob");

      if (DEBUG)
      {
         System.out.println ("testCIMClass: cm = " + cm);
      }

      if (cm != null)
      {
         System.out.println ("FAILURE: testCIMClass: getMethod (\"bob\")");

         return false;
      }

      // -----

      CIMClass cc1 = null;
      CIMClass cc2 = null;

      cop = new CIMObjectPath (pszPropertyTypesClassName,
                               "root/SampleProvider");

      if (DEBUG)
      {
         System.out.println ("testCIMClass: cop = " + cop);
      }

      try
      {
         cc1 = client.getClass (cop,
                                true,  // propagated
                                true,  // includeQualifiers
                                true,  // includeClassOrigin
                                null); // propertyList
         cc2 = client.getClass (cop,
                                true,  // propagated
                                true,  // includeQualifiers
                                true,  // includeClassOrigin
                                null); // propertyList
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMClass: client.getClass: caught " + e);

         return false;
      }

      if (  cc1 == null
         || cc2 == null
         )
      {
         System.out.println ("FAILURE: testCIMClass: cc1 == null or cc2 == null");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMClass: cc1 = " + cc1);
         System.out.println ("testCIMClass: cc2 = " + cc2);
      }

      if (DEBUG)
      {
         System.out.println ("testCIMClass: ccPropertyTypes.equals (ccElement) = " + ccPropertyTypes.equals (ccElement));
         System.out.println ("testCIMClass: ccPropertyTypes.equals (ccPropertyTypes) = " + ccPropertyTypes.equals (ccPropertyTypes));
         System.out.println ("testCIMClass: cc1.equals (ccPropertyTypes) = " + cc1.equals (ccPropertyTypes));
         System.out.println ("testCIMClass: cc1.equals (cc1) = " + cc1.equals (cc1));
         System.out.println ("testCIMClass: cc2.equals (cc2) = " + cc2.equals (cc2));
         System.out.println ("testCIMClass: cc1.equals (cc2) = " + cc1.equals (cc2));
      }

      if (   (ccPropertyTypes.equals (ccElement))
         || !(ccPropertyTypes.equals (ccPropertyTypes))
         ||  (cc1.equals (ccPropertyTypes))
         || !(cc1.equals (cc1))
         || !(cc2.equals (cc2))
         || !(cc1.equals (cc2))
         )
      {
         System.out.println ("FAILURE: testCIMClass: equals");

         return false;
      }

      // -----

      CIMClass ccTeacherStudent = null;

      cop = new CIMObjectPath ("JMPIExpAssociation_TeacherStudent",
                               "root/SampleProvider");

      if (DEBUG)
      {
         System.out.println ("testCIMClass: cop = " + cop);
      }

      try
      {
         ccTeacherStudent = client.getClass (cop,
                                             true,  // propagated
                                             true,  // includeQualifiers
                                             true,  // includeClassOrigin
                                             null); // propertyList
      }
      catch (CIMException e)
      {
         System.out.println ("FAILURE: testCIMClass: client.getClass: caught " + e);

         return false;
      }

      if (ccTeacherStudent == null)
      {
         System.out.println ("FAILURE: testCIMClass: ccTeacherStudent == null");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testCIMClass: ccTeacherStudent = " + ccTeacherStudent);
         System.out.println ("testCIMClass: ccPropertyTypes.isAssociation () = " + ccPropertyTypes.isAssociation ());
         System.out.println ("testCIMClass: ccTeacherStudent.isAssociation () = " + ccTeacherStudent.isAssociation ());
      }

      if (ccPropertyTypes.isAssociation ())
      {
         System.out.println ("FAILURE: testCIMClass: isAssociation () 1");

         return false;
      }

      if (!ccTeacherStudent.isAssociation ())
      {
         System.out.println ("FAILURE: testCIMClass: isAssociation () 2");

         return false;
      }

      // -----

      String pszMethodName1 = "sayHello";
      String pszMethodName2 = "enableModifications";
      String pszMethodName3 = "bob";
      int    iMethod1       = -1;
      int    iMethod2       = -1;
      int    iMethod3       = -1;

      iMethod1 = ccPropertyTypes.findMethod (pszMethodName1);
      iMethod2 = ccPropertyTypes.findMethod (pszMethodName2);
      iMethod3 = ccPropertyTypes.findMethod (pszMethodName3);

      if (DEBUG)
      {
         System.out.println ("testCIMClass: ccPropertyTypes.findMethod (" + pszMethodName1 + ") = " + iMethod1);
         System.out.println ("testCIMClass: ccPropertyTypes.findMethod (" + pszMethodName2 + ") = " + iMethod2);
         System.out.println ("testCIMClass: ccPropertyTypes.findMethod (" + pszMethodName3 + ") = " + iMethod3);
      }

      if (iMethod1 != 0)
      {
         System.out.println ("FAILURE: testCIMClass: findMethod () 1");

         return false;
      }

      if (iMethod2 != 1)
      {
         System.out.println ("FAILURE: testCIMClass: findMethod () 2");

         return false;
      }

      if (iMethod3 != -1)
      {
         System.out.println ("FAILURE: testCIMClass: findMethod () 3");

         return false;
      }

      // -----

      CIMMethod cm1 = null;
      CIMMethod cm2 = null;
      CIMMethod cm3 = null;

      cm1 = ccPropertyTypes.getMethod (iMethod1);
      cm2 = ccPropertyTypes.getMethod (iMethod2);
      cm3 = ccPropertyTypes.getMethod (iMethod3);

      if (DEBUG)
      {
         System.out.println ("testCIMClass: getMethod (" + iMethod1 + ") = " + cm1);
         System.out.println ("testCIMClass: getMethod (" + iMethod2 + ") = " + cm2);
         System.out.println ("testCIMClass: getMethod (" + iMethod3 + ") = " + cm3);
      }

      if (cm1 == null)
      {
         System.out.println ("FAILURE: testCIMClass: getMethod () 1");

         return false;
      }

      if (cm2 == null)
      {
         System.out.println ("FAILURE: testCIMClass: getMethod () 2");

         return false;
      }

      if (cm3 != null)
      {
         System.out.println ("FAILURE: testCIMClass: getMethod () 3");

         return false;
      }

      // -----

      if (DEBUG)
      {
         System.out.println ("testCIMClass: ccPropertyTypes.getMethodCount () = " + ccPropertyTypes.getMethodCount ());
      }

      if (ccPropertyTypes.getMethodCount () != 3)
      {
         System.out.println ("FAILURE: testCIMClass: getMethodCount ()");

         return false;
      }

      // -----

      System.out.println ("SUCCESS: testCIMClass");

      return true;
   }
}
