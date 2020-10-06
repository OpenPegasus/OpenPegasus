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

import java.util.Enumeration;
import java.util.Vector;
import org.pegasus.jmpi.CIMArgument;
import org.pegasus.jmpi.CIMClass;
import org.pegasus.jmpi.CIMClient;
import org.pegasus.jmpi.CIMDateTime;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMInstance;
import org.pegasus.jmpi.CIMNameSpace;
import org.pegasus.jmpi.CIMObjectPath;
import org.pegasus.jmpi.CIMProperty;
import org.pegasus.jmpi.CIMQualifierType;
import org.pegasus.jmpi.CIMValue;
import org.pegasus.jmpi.UnsignedInt8;
import org.pegasus.jmpi.UnsignedInt16;
import org.pegasus.jmpi.UnsignedInt32;
import org.pegasus.jmpi.UnsignedInt64;

// cimcli dq -n root/SampleProvider JMPINewQualifierType

public class testCIMClient
{
   final String                classNameInstanceParent     = "JMPIExpInstance_TestElement";
   final String                classNameInstanceChild      = "JMPIExpInstance_TestPropertyTypes";
   final String                classNamePropertyChild      = "JMPIProperty_TestPropertyTypes";
   final String                classNameAssociationTeacher = "JMPIExpAssociation_Teacher";
   final String                nameSpaceClass              = "root/SampleProvider";
   final String                hostName                    = "localhost";

   private static boolean      DEBUG           = false;

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "CIMClient";
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
         fRet = testCIMClient (cimClient);
      }

      return fRet;
   }

   private boolean testCIMClient (CIMClient cimClient)
   {
      try
      {
         if (DEBUG)
         {
            System.err.println ("Get name space");
         }

         CIMNameSpace cns = cimClient.getNameSpace ();

         if (DEBUG)
         {
            System.err.println ("cns           = " + cns);
         }

         try
         {
            cns = new CIMNameSpace ();

            if (DEBUG)
            {
               System.err.println ("cns           = " + cns);
               System.err.println ("Set name space");
            }

            cns.setNameSpace ("test/newnamespace");

            if (DEBUG)
            {
               System.err.println ("Create name space");
            }

            cimClient.createNameSpace (cns);
         }
         catch (CIMException e)
         {
            if (e.getID () != CIMException.CIM_ERR_NOT_FOUND)
            {
               System.err.println ("Error: Exception: " + e);
               e.printStackTrace ();

               System.out.println ("FAIL (1): testCIMClient: createNameSpace");

               return false;
            }
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         cns = new CIMNameSpace ();

         if (DEBUG)
         {
            System.err.println ("cns           = " + cns);
            System.err.println ("Set name space");
         }

         cns.setNameSpace ("root/newnamespace");

         if (DEBUG)
         {
            System.err.println ("Create name space");
         }

         cimClient.createNameSpace (cns);

         if (DEBUG)
         {
            System.err.println ("Delete name space");
         }

         cimClient.deleteNameSpace (cns);

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         Enumeration   enm;
         CIMObjectPath cop;

         cop = new CIMObjectPath (classNameInstanceChild, "root");

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Enumerate name space: enumNameSpace");
         }

         // NOTE: This call only cares about the name space within cop.
         //       This is confusing and should be changed.
         enm = cimClient.enumNameSpace (cop, true);

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (2): testCIMClient: enumNameSpace");

            return false;
         }

         while (enm.hasMoreElements ())
         {
            String nameSpace = (String)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("nameSpace     = " + nameSpace);
            }
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Enumerate name space: enumerateNameSpaces");
         }

         // NOTE: This call only cares about the name space within cop.
         //       This is confusing and should be changed.
         enm = cimClient.enumerateNameSpaces (cop, true);

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (3): testCIMClient: enumNameSpace");

            return false;
         }

         while (enm.hasMoreElements ())
         {
            String nameSpace = (String)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("nameSpace     = " + nameSpace);
            }
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         cop = new CIMObjectPath (classNameInstanceParent, nameSpaceClass);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Enumerate class: enumClass");
         }

         enm = cimClient.enumClass (cop,
                                    true,   // deepInheritance
                                    false); // localOnly

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (4): testCIMClient: enumClass");

            return false;
         }

         CIMClass cc = (CIMClass)enm.nextElement ();

         if (DEBUG)
         {
            System.err.println ("cc            = " + cc);
         }

         // There should be only one class
         if (enm.hasMoreElements ())
         {
            System.out.println ("FAIL (5): testCIMClient: enumClass");

            return false;
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Enumerate class: enumerateClasses");
         }

         enm = cimClient.enumerateClasses (cop,
                                           true,   // deepInheritance
                                           false,  // localOnly
                                           true,   // includeQualifiers
                                           true);  // includeClassOrigin

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (6): testCIMClient: enumerateClasses");

            return false;
         }

         cc = (CIMClass)enm.nextElement ();

         if (DEBUG)
         {
            System.err.println ("cc            = " + cc);
         }

         // There should be only one class
         if (enm.hasMoreElements ())
         {
            System.out.println ("FAIL (7): testCIMClient: enumerateClasses");

            return false;
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Enumerate class: enumClass");
         }

         enm = cimClient.enumClass (cop,
                                    true);  // deepInheritance

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (8): testCIMClient: enumClass");

            return false;
         }

         CIMObjectPath copRet = (CIMObjectPath)enm.nextElement ();

         if (DEBUG)
         {
            System.err.println ("copRet        = " + copRet);
         }

         // There should be only one class
         if (enm.hasMoreElements ())
         {
            System.out.println ("FAIL (9): testCIMClient: enumClass");

            return false;
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Enumerate class: enumerateClassNames");
         }

         enm = cimClient.enumerateClassNames (cop,
                                              true);  // deepInheritance

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (10): testCIMClient: enumerateClassNames");

            return false;
         }

         // @TBD this object path doesn't have the hostname while the one
         //      above does.
         copRet = (CIMObjectPath)enm.nextElement ();

         if (DEBUG)
         {
            System.err.println ("copRet        = " + copRet);
         }

         // There should be only one object path
         if (enm.hasMoreElements ())
         {
            System.out.println ("FAIL (11): testCIMClient: enumerateClassNames");

            return false;
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         cop = new CIMObjectPath (classNameInstanceChild, nameSpaceClass);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Enumerate instance: enumInstances");
         }

         enm = cimClient.enumInstances (cop,
                                        true);  // deepInheritance

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (12): testCIMClient: enumInstances");

            return false;
         }

         while (enm.hasMoreElements ())
         {
            copRet = (CIMObjectPath)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("copRet        = " + copRet);
            }
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Enumerate instance: enumerateInstanceNames");
         }

         enm = cimClient.enumerateInstanceNames (cop);

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (13): testCIMClient: enumerateInstanceNames");

            return false;
         }

         while (enm.hasMoreElements ())
         {
            copRet = (CIMObjectPath)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("copRet        = " + copRet);
            }
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Enumerate instance: enumInstances");
         }

         enm = cimClient.enumInstances (cop,
                                        true,   // deepInheritance
                                        true);  // localOnly

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (14): testCIMClient: enumInstances");

            return false;
         }

         CIMInstance ci = null;

         while (enm.hasMoreElements ())
         {
            ci = (CIMInstance)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("ci            = " + ci);
            }
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Enumerate instance: enumerateInstances");
         }

         enm = cimClient.enumerateInstances (cop,
                                             true,   // deepInheritance
                                             true,   // localOnly
                                             true,   // includeQualifiers
                                             true,   // includeClassOrigin
                                             null);  // propertyList

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (15): testCIMClient: enumerateInstances");

            return false;
         }

         while (enm.hasMoreElements ())
         {
            ci = (CIMInstance)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("ci            = " + ci);
            }
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         cop = new CIMObjectPath (classNameInstanceChild, nameSpaceClass);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Get class: getClass (,)");
         }

         cc = cimClient.getClass (cop,
                                  true);  // localOnly

         if (DEBUG)
         {
            System.err.println ("cc            = " + cc);
         }

         if (cc == null)
         {
            System.out.println ("FAIL (16): testCIMClient: getClass");

            return false;
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Get class: getClass (,,,)");
         }

         cc = cimClient.getClass (cop,
                                  true,   // localOnly
                                  true,   // includeQualifiers,
                                  true,   // includeClassOrigin,
                                  null);  // propertyList

         if (DEBUG)
         {
            System.err.println ("cc            = " + cc);
         }

         if (cc == null)
         {
            System.out.println ("FAIL (17): testCIMClient: getClass");

            return false;
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         Vector kvp = new Vector ();

         kvp.addElement (new CIMProperty ("CreationClassName",
                                          new CIMValue (classNameInstanceChild)));
         kvp.addElement (new CIMProperty ("InstanceId",
                                          new CIMValue (new UnsignedInt64 ("1"))));

         cop = new CIMObjectPath (classNameInstanceChild, kvp);

         cop.setNameSpace (nameSpaceClass);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Get instance: getInstance (,)");
         }

         ci = cimClient.getInstance (cop,
                                     true);  // localOnly

         if (DEBUG)
         {
            System.err.println ("ci            = " + ci);
         }

         if (ci == null)
         {
            System.out.println ("FAIL (18): testCIMClient: getInstance");

            return false;
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Get instance: getInstance (,,,)");
         }

         ci = cimClient.getInstance (cop,
                                     true,   // localOnly
                                     true,   // includeQualifiers,
                                     true,   // includeClassOrigin,
                                     null);  // propertyList

         if (DEBUG)
         {
            System.err.println ("ci            = " + ci);
         }

         if (ci == null)
         {
            System.out.println ("FAIL (19): testCIMClient: getInstance");

            return false;
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Invoke method with vector");
         }

         Vector   vectorInParams  = new Vector ();
         Vector   vectorOutParams = new Vector ();
         CIMValue cv              = null;

         cv = cimClient.invokeMethod (cop,
                                      "sayHello",
                                      vectorInParams,
                                      vectorOutParams);

         if (DEBUG)
         {
            System.err.println ("cv            = " + cv);
         }

         if (  ci == null
            || !((String)cv.getValue ()).equals ("hello")
            )
         {
            System.out.println ("FAIL (20): testCIMClient: invokeMethod");

            return false;
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Invoke method with array");
         }

         CIMArgument[] arrayInParams  = null;
         CIMArgument[] arrayOutParams = null;

         cv = cimClient.invokeMethod (cop,
                                      "sayHello",
                                      arrayInParams,
                                      arrayOutParams);

         if (DEBUG)
         {
            System.err.println ("cv            = " + cv);
         }

         if (  ci == null
            || !((String)cv.getValue ()).equals ("hello")
            )
         {
            System.out.println ("FAIL (21): testCIMClient: invokeMethod");

            return false;
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         CIMQualifierType cqt = null;

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Enumerate qualifiers: enumQualifierTypes");
         }

         enm = cimClient.enumQualifierTypes (cop);

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (22): testCIMClient: enumQualifierTypes");

            return false;
         }

         while (enm.hasMoreElements ())
         {
            cqt = (CIMQualifierType)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("cqt           = " + cqt.getName ());
            }
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Enumerate qualifiers: enumerateQualifiers");
         }

         enm = cimClient.enumerateQualifiers (cop);

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (23): testCIMClient: enumerateQualifiers");

            return false;
         }

         while (enm.hasMoreElements ())
         {
            cqt = (CIMQualifierType)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("cqt           = " + cqt.getName ());
            }
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         String testQualifier = "JMPINewQualifierType";

         cop = new CIMObjectPath (testQualifier, nameSpaceClass);

         cqt = new CIMQualifierType ();
         cqt.setName (testQualifier);
         cqt.setValue (new CIMValue (new Boolean (true)));

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Create qualifier: createQualifierType");
         }

         cimClient.createQualifierType (cop, cqt);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Delete qualifier: deleteQualifierType");
         }

         cimClient.deleteQualifierType (cop);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         cqt = new CIMQualifierType ();
         cqt.setName (testQualifier);
         cqt.setValue (new CIMValue (new Boolean (true)));

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Create qualifier: createQualifierType");
         }

         cimClient.createQualifierType (cop, cqt);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Get qualifier: getQualifierType");
         }

         cqt = cimClient.getQualifierType (cop);

         if (DEBUG)
         {
            System.err.println ("cqt           = " + cqt);
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Get qualifier: getQualifier");
         }

         cqt = cimClient.getQualifier (cop);

         if (DEBUG)
         {
            System.err.println ("cqt           = " + cqt);
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------
/*
         // Error: Exception: CIM_ERR_NOT_SUPPORTED (CIM_ERR_NOT_SUPPORTED: The requested operation is not supported: "JMPINewQualifierType")
         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Set qualifier: setQualifierType");
         }

         cimClient.setQualifierType (cop, cqt);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("cqt           = " + cqt);
         }
*/
         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Delete qualifier: deleteQualifier");
         }

         cimClient.deleteQualifier (cop);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         String testClass = "JMPINewClass";

         cc  = new CIMClass (testClass);
         cop = new CIMObjectPath (testClass, nameSpaceClass);

         if (DEBUG)
         {
            System.err.println ("cc            = " + cc);
            System.err.println ("cop           = " + cop);
            System.err.println ("Create class");
         }

         cimClient.createClass (cop, cc);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Set class: setClass");
         }

         cimClient.setClass (cop, cc);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Set class: modifyClass");
         }

         cimClient.modifyClass (cop, cc);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Delete class");
         }

         cimClient.deleteClass (cop);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         kvp = new Vector ();

         kvp.addElement (new CIMProperty ("CreationClassName",
                                          new CIMValue (classNameInstanceChild)));
         kvp.addElement (new CIMProperty ("InstanceId",
                                          new CIMValue (new UnsignedInt64 ("3"))));
         kvp.addElement (new CIMProperty ("PropertyString",
                                          new CIMValue ("Third string")));
         kvp.addElement (new CIMProperty ("PropertyUint8",
                                          new CIMValue (new UnsignedInt8 ((short)123))));
         kvp.addElement (new CIMProperty ("PropertyUint16",
                                          new CIMValue (new UnsignedInt16 ((int)1603))));
         kvp.addElement (new CIMProperty ("PropertyUint32",
                                          new CIMValue (new UnsignedInt32 ((long)3203))));
         kvp.addElement (new CIMProperty ("PropertyUint64",
                                          new CIMValue (new UnsignedInt64 ("6403"))));
         kvp.addElement (new CIMProperty ("PropertySint8",
                                          new CIMValue (new Byte ((byte)-113))));
         kvp.addElement (new CIMProperty ("PropertySint16",
                                          new CIMValue (new Short ((short)-1603))));
         kvp.addElement (new CIMProperty ("PropertySint32",
                                          new CIMValue (new Integer (-3203))));
         kvp.addElement (new CIMProperty ("PropertySint64",
                                          new CIMValue (new Long (-6403))));
         kvp.addElement (new CIMProperty ("PropertyBoolean",
                                          new CIMValue (new Boolean (false))));
         kvp.addElement (new CIMProperty ("PropertyReal32",
                                          new CIMValue (new Float (3.33333333333))));
         kvp.addElement (new CIMProperty ("PropertyReal64",
                                          new CIMValue (new Double (3.13131313131313131313))));
         kvp.addElement (new CIMProperty ("PropertyDatetime",
                                          new CIMValue (new CIMDateTime ("20030303030333.000000:000"))));
         kvp.addElement (new CIMProperty ("PropertyChar16",
                                          new CIMValue (new Character ('3'))));

         cop = new CIMObjectPath (classNameInstanceChild, kvp);
         cop.setNameSpace (nameSpaceClass);

         ci = new CIMInstance (classNameInstanceChild);
         ci.setObjectPath (cop);

         CIMProperty cp = null;

         // @TBD
         enm = kvp.elements ();
         while (enm.hasMoreElements ())
         {
            cp = (CIMProperty)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("setting (" + cp.getName () + ", " + cp.getValue () + ")");
            }

            ci.setProperty (cp.getName (), cp.getValue ());
         }

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("ci            = " + ci);
            System.err.println ("Create instance");
         }

         CIMObjectPath copCreated = null;

         copCreated = cimClient.createInstance (cop, ci);

         if (DEBUG)
         {
            System.err.println ("copCreated    = " + copCreated);
         }

         // @TBD - the name space was not set in the returned cop!
         copCreated.setNameSpace (nameSpaceClass);

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         kvp = new Vector ();

         kvp.addElement (new CIMProperty ("CreationClassName",
                                          new CIMValue (classNameInstanceChild)));
         kvp.addElement (new CIMProperty ("InstanceId",
                                          new CIMValue (new UnsignedInt64 ("3"))));
         kvp.addElement (new CIMProperty ("PropertyString",
                                          new CIMValue ("Fourth string")));
         kvp.addElement (new CIMProperty ("PropertyUint8",
                                          new CIMValue (new UnsignedInt8 ((short)124))));
         kvp.addElement (new CIMProperty ("PropertyUint16",
                                          new CIMValue (new UnsignedInt16 ((int)1604))));
         kvp.addElement (new CIMProperty ("PropertyUint32",
                                          new CIMValue (new UnsignedInt32 ((long)4204))));
         kvp.addElement (new CIMProperty ("PropertyUint64",
                                          new CIMValue (new UnsignedInt64 ("6404"))));
         kvp.addElement (new CIMProperty ("PropertySint8",
                                          new CIMValue (new Byte ((byte)-114))));
         kvp.addElement (new CIMProperty ("PropertySint16",
                                          new CIMValue (new Short ((short)-1604))));
         kvp.addElement (new CIMProperty ("PropertySint42",
                                          new CIMValue (new Integer (-4204))));
         kvp.addElement (new CIMProperty ("PropertySint64",
                                          new CIMValue (new Long (-6404))));
         kvp.addElement (new CIMProperty ("PropertyBoolean",
                                          new CIMValue (new Boolean (false))));
         kvp.addElement (new CIMProperty ("PropertyReal42",
                                          new CIMValue (new Float (4.44444444444))));
         kvp.addElement (new CIMProperty ("PropertyReal64",
                                          new CIMValue (new Double (4.14141414141414141414))));
         kvp.addElement (new CIMProperty ("PropertyDatetime",
                                          new CIMValue (new CIMDateTime ("20040404040444.000000:000"))));
         kvp.addElement (new CIMProperty ("PropertyChar16",
                                          new CIMValue (new Character ('4'))));

         cop = new CIMObjectPath (classNameInstanceChild, kvp);
         cop.setNameSpace (nameSpaceClass);

         ci = new CIMInstance (classNameInstanceChild);
         ci.setObjectPath (cop);

         // @TBD
         enm = kvp.elements ();
         while (enm.hasMoreElements ())
         {
            cp = (CIMProperty)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("setting (" + cp.getName () + ", " + cp.getValue () + ")");
            }

            ci.setProperty (cp.getName (), cp.getValue ());
         }

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("ci            = " + ci);
            System.err.println ("Set instance: setInstance");
         }

         cimClient.setInstance (cop, ci);

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------
/*
         ci.setProperty ("PropertyUint8", new CIMValue (new UnsignedInt8 ((short)33)));

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("ci            = " + ci);
            System.err.println ("Set instance: modifyInstance");
         }

         cimClient.modifyInstance (cop,
                                   ci,
                                   true,  // includeQualifiers
                                   null); // propertyList
*/
         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         if (DEBUG)
         {
            System.err.println ("copCreated    = " + copCreated);
            System.err.println ("Delete instance");
         }

         cimClient.deleteInstance (copCreated);

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         kvp = new Vector ();

         kvp.addElement (new CIMProperty ("CreationClassName",
                                          new CIMValue (classNamePropertyChild)));
         kvp.addElement (new CIMProperty ("InstanceId",
                                          new CIMValue (new UnsignedInt64 ("1"))));

         cop = new CIMObjectPath (classNamePropertyChild, kvp);
         cop.setNameSpace (nameSpaceClass);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Set property");
         }

         String newString = "a new string";

         cimClient.setProperty (cop, "PropertyString", new CIMValue (newString));

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         kvp = new Vector ();

         kvp.addElement (new CIMProperty ("CreationClassName",
                                          new CIMValue (classNamePropertyChild)));
/////////                                 new CIMValue (classNameInstanceChild)));
         kvp.addElement (new CIMProperty ("InstanceId",
                                          new CIMValue (new UnsignedInt64 ("1"))));

         cop = new CIMObjectPath (classNamePropertyChild, kvp);
/////////cop = new CIMObjectPath (classNameInstanceChild, kvp);
         cop.setNameSpace (nameSpaceClass);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Get property");
         }

         cv = cimClient.getProperty (cop, "PropertyString");

         if (DEBUG)
         {
            System.err.println ("cv            = " + cv);
         }

         if (!((String)cv.getValue ()).equals (newString))
         {
            System.out.println ("FAIL (24): testCIMClient: setProperty/getProperty");

            return false;
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         String stringQuery         = "SELECT InstanceId FROM " + classNameInstanceChild + " WHERE InstanceId = 1";
         String stringQueryLanguage = "WQL";

         cop = new CIMObjectPath (classNameInstanceChild, nameSpaceClass);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Exec query");
         }

         try
         {
            enm = cimClient.execQuery (cop, stringQuery, stringQueryLanguage);

            if (DEBUG)
            {
               System.err.println ("enm           = " + enm);
            }

            if (  enm == null
               || !enm.hasMoreElements ()
               )
            {
               System.out.println ("FAIL (25): testCIMClient: execQuery");

               return false;
            }

            ci = (CIMInstance)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("ci            = " + ci);
            }

            // There should be only one class
            if (enm.hasMoreElements ())
            {
               System.out.println ("FAIL (26): testCIMClient: execQuery");

               return false;
            }
         }
         catch (CIMException e)
         {
            if (e.getID () != CIMException.CIM_ERR_NOT_SUPPORTED)
            {
               throw e;
            }
            else
            {
               System.out.println ("IGNORE: testCIMClient: execQuery CIM_ERR_NOT_SUPPORTED");
            }
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         String assocClass  = "";
         String resultClass = "";
         String role        = "Teaches";
         String resultRole  = "";

         kvp = new Vector ();

         kvp.addElement (new CIMProperty ("Name",
                                          new CIMValue ("Teacher1")));
         kvp.addElement (new CIMProperty ("Identifier",
                                          new CIMValue (new UnsignedInt8 ("1"))));

         cop = new CIMObjectPath (classNameAssociationTeacher, kvp);
         cop.setNameSpace (nameSpaceClass);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Associator Names");
         }

         enm = cimClient.associatorNames (cop,
                                          assocClass,
                                          resultClass,
                                          role,
                                          resultRole);

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (27): testCIMClient: associatorNames");

            return false;
         }

         while (enm.hasMoreElements ())
         {
            cop = (CIMObjectPath)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("cop           = " + cop);
            }
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         assocClass  = "";
         resultClass = "";
         role        = "Teaches";
         resultRole  = "";

         kvp = new Vector ();

         kvp.addElement (new CIMProperty ("Name",
                                          new CIMValue ("Teacher1")));
         kvp.addElement (new CIMProperty ("Identifier",
                                          new CIMValue (new UnsignedInt8 ("1"))));

         cop = new CIMObjectPath (classNameAssociationTeacher, kvp);
         cop.setNameSpace (nameSpaceClass);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Associators");
         }

         enm = cimClient.associators (cop,
                                      assocClass,
                                      resultClass,
                                      role,
                                      resultRole,
                                      true,       // includeQualifiers
                                      true,       // includeClassOrigin
                                      null);      // propertyList

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (28): testCIMClient: associators");

            return false;
         }

         while (enm.hasMoreElements ())
         {
            ci = (CIMInstance)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("ci            = " + ci);
            }
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         resultClass = "";
         role        = "Teaches";

         kvp = new Vector ();

         kvp.addElement (new CIMProperty ("Name",
                                          new CIMValue ("Teacher1")));
         kvp.addElement (new CIMProperty ("Identifier",
                                          new CIMValue (new UnsignedInt8 ("1"))));

         cop = new CIMObjectPath (classNameAssociationTeacher, kvp);
         cop.setNameSpace (nameSpaceClass);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("Reference Names");
         }

         enm = cimClient.referenceNames (cop,
                                         resultClass,
                                         role);

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (29): testCIMClient: referenceNames");

            return false;
         }

         while (enm.hasMoreElements ())
         {
            cop = (CIMObjectPath)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("cop           = " + cop);
            }
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         resultClass = "";
         role        = "Teaches";

         kvp = new Vector ();

         kvp.addElement (new CIMProperty ("Name",
                                          new CIMValue ("Teacher1")));
         kvp.addElement (new CIMProperty ("Identifier",
                                          new CIMValue (new UnsignedInt8 ("1"))));

         cop = new CIMObjectPath (classNameAssociationTeacher, kvp);
         cop.setNameSpace (nameSpaceClass);

         if (DEBUG)
         {
            System.err.println ("cop           = " + cop);
            System.err.println ("References");
         }

         enm = cimClient.references (cop,
                                     resultClass,
                                     role,
                                     true,       // includeQualifiers
                                     true,       // includeClassOrigin
                                     null);      // propertyList

         if (DEBUG)
         {
            System.err.println ("enm           = " + enm);
         }

         if (  enm == null
            || !enm.hasMoreElements ()
            )
         {
            System.out.println ("FAIL (30): testCIMClient: references");

            return false;
         }

         while (enm.hasMoreElements ())
         {
            ci = (CIMInstance)enm.nextElement ();

            if (DEBUG)
            {
               System.err.println ("ci            = " + ci);
            }
         }

         //8<--------8<--------8<--------8<--------8<--------8<--------8<--------

         System.out.println ("SUCCESS: testCIMClient");

         return true;
      }
      catch (CIMException e)
      {
          System.err.println ("Error: Exception: " + e);
          e.printStackTrace ();
      }

      return false;
   }
}
