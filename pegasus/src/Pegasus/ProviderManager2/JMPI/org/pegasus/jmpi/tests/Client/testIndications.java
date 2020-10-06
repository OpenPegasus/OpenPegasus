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

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Enumeration;
import java.util.Vector;
import org.pegasus.jmpi.CIMClass;
import org.pegasus.jmpi.CIMClient;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMInstance;
import org.pegasus.jmpi.CIMNameSpace;
import org.pegasus.jmpi.CIMObjectPath;
import org.pegasus.jmpi.CIMProperty;
import org.pegasus.jmpi.CIMValue;
import org.pegasus.jmpi.UnsignedInt16;
import org.pegasus.jmpi.UnsignedInt32;
import org.pegasus.jmpi.UnsignedInt64;

public class testIndications
{
   private final static String className           = "JMPIExpIndication";
   private final static String nameSpaceClass      = "root/SampleProvider";
   private final static String nameSpaceInterOp    = "root/PG_InterOp";
   private final static String nameSpaceCIMV2      = "root/cimv2";

   private static boolean      DEBUG               = false;
   private static int          iNumIndications     = 10;
   private static boolean      DELETE              = true;

   private static boolean      fBugExistsExecQuery = false;

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "instances";
   }

   public void setDebug (boolean fDebug)
   {
      DEBUG = fDebug;
   }

   public boolean main (String args[], CIMClient cimClient)
   {
      boolean fExecuted = false;

      for (int i = 0; i < args.length; i++)
      {
         if (args[i].equalsIgnoreCase ("debug"))
         {
            setDebug (true);
         }
         else if (args[i].equalsIgnoreCase ("noDebug"))
         {
            DEBUG = false;
         }
         else if (args[i].equalsIgnoreCase ("numIndications"))
         {
            if (i + 1 < args.length)
            {
               iNumIndications = new Integer (args[i+1]).intValue ();

               i++;
            }
         }
         else if (args[i].equalsIgnoreCase ("delete"))
         {
            DELETE = true;
         }
         else if (args[i].equalsIgnoreCase ("noDelete"))
         {
            DELETE = false;
         }
         else if (args[i].equalsIgnoreCase ("BugExistsExecQuery"))
         {
            fBugExistsExecQuery = true;
         }
      }

      if (!fExecuted)
          return runTests (cimClient);

      return false;
   }

   private static CIMObjectPath createInstance (CIMClient   cc,
                                                CIMInstance ciToCreate,
                                                String      nameSpace)
      throws Exception
   {
      CIMObjectPath copToCreate   = null;
      Vector        keyValuePairs = null;
      CIMInstance   ciToReturn    = null;
      CIMObjectPath copToReturn   = null;

      try
      {
         if (DEBUG)
         {
            System.err.println ("testIndications::createInstance: ciToCreate = " + ciToCreate);
         }

         keyValuePairs = ciToCreate.getKeyValuePairs ();

         if (DEBUG)
         {
            System.err.println ("testIndications::createInstance: keyValuePairs = " + keyValuePairs);
         }

         if (keyValuePairs.size () == 0)
         {
////////////@BUG
////////////throw new Exception ("No keys returned for " + ciToCreate.getClassName ());

            keyValuePairs = ciToCreate.getProperties ();

            if (DEBUG)
            {
               System.err.println ("testIndications::createInstance: no keyValuePairs returned.  switching to keyValuePairs = " + keyValuePairs);
            }

            for (int i = 0; i < keyValuePairs.size (); i++)
            {
               CIMProperty cp = (CIMProperty)keyValuePairs.elementAt (i);
               CIMValue    cv = cp.getValue ();
               System.err.println (cv.isArray ());
            }
         }

/////////@BUG
/////////copToCreate = new CIMObjectPath (ciToCreate,
/////////                                 nameSpace);

         copToCreate = new CIMObjectPath (ciToCreate.getClassName (),
                                          keyValuePairs);

         copToCreate.setNameSpace (nameSpace);

         if (DEBUG)
         {
            System.err.println ("testIndications::createInstance: copToCreate = " + copToCreate);
         }

         cc.createInstance (copToCreate,
                            ciToCreate);
      }
      catch (CIMException e)
      {
         if (e.getID ().equals ("CIM_ERR_ALREADY_EXISTS"))
         {
            if (DEBUG)
            {
               System.err.println ("testIndications::createInstance: Instance already exists calling getInstance (" + copToCreate + ")");
            }
         }
         else
         {
            System.err.println ("testIndications::createInstance: Caught " + e);

            throw e;
         }
      }

      ciToReturn = cc.getInstance (copToCreate,
                                   false);

      keyValuePairs = ciToReturn.getKeyValuePairs ();
      if (keyValuePairs.size () == 0)
      {
         keyValuePairs = ciToReturn.getProperties ();
      }

      copToReturn = new CIMObjectPath (ciToReturn.getClassName (),
                                       keyValuePairs);

      copToReturn.setNameSpace (nameSpace);

      return copToReturn;
   }

   private static String getHostname ()
   {
      try
      {
         InetAddress addr = InetAddress.getLocalHost ();

         // Get IP Address
         byte[] ipAddr = addr.getAddress ();

         // Get the canonical hostname
         return addr.getCanonicalHostName ();
      }
      catch (UnknownHostException e)
      {
      }

      return "localhost";
   }

   private static String getSystemName (CIMClient cc)
   {
      Enumeration   enm;
      CIMObjectPath cop;

      try
      {
         cop = new CIMObjectPath ("PG_NameSpace", "root/PG_InterOp");
         enm = cc.enumInstances (cop,
                                 true,   // deepInheritance
                                 true);  // localOnly

         if (DEBUG)
         {
            System.out.println ("testIndications::getSystemName cop = " + cop);
            System.out.println ("testIndications::getSystemName enm = " + enm);
         }

         if (  enm != null
            && enm.hasMoreElements ()
            )
         {
            CIMInstance ci = (CIMInstance)enm.nextElement ();
            CIMProperty cp = null;
            CIMValue    cv = null;

            if (DEBUG)
            {
               System.out.println ("testIndications::getSystemName ci = " + ci);
            }

            if (ci != null)
            {
               cp = ci.getProperty ("SystemName");
            }

            if (DEBUG)
            {
               System.out.println ("testIndications::getSystemName cp = " + cp);
            }

            if (cp != null)
            {
               cv = cp.getValue ();
            }

            if (DEBUG)
            {
               System.out.println ("testIndications::getSystemName cv = " + cv);
            }

            if (cv != null)
            {
               return (String)cv.getValue ();
            }
         }
      }
      catch (Exception e)
      {
         e.printStackTrace ();
      }

      return getHostname ();
   }

   private static UnsignedInt64 findNextIndicationNumber (CIMClient cc)
      throws CIMException
   {
      for (int i = 1; i < 1000; i++)
      {
         UnsignedInt64 ui64ToTry = new UnsignedInt64 (new Integer (i).toString ());
         try
         {
            CIMObjectPath copInstance   = null;
            CIMInstance   ciIndication  = null;
            Vector        keyValuePairs = new Vector ();

            keyValuePairs.addElement (new CIMProperty ("InstanceId",
                                                       new CIMValue (ui64ToTry)));

            copInstance = new CIMObjectPath (className,
                                             keyValuePairs);
            copInstance.setNameSpace (nameSpaceClass);

            ciIndication = cc.getInstance (copInstance,
                                           false);
         }
         catch (CIMException e)
         {
            if (e.getID ().equals ("CIM_ERR_NOT_FOUND"))
            {
               return ui64ToTry;
            }
            else if (!e.getID ().equals ("CIM_ERR_ALREADY_EXISTS"))
            {
               throw e;
            }
         }
      }

      return new UnsignedInt64 ("1000");
   }

   private static CIMObjectPath testCreateClassIndicationFilter (CIMClient cc)
      throws Exception
   {
      CIMInstance ciFilter = new CIMInstance("CIM_IndicationFilter");
      ciFilter.setObjectPath(
          new CIMObjectPath("CIM_IndicationFilter",nameSpaceInterOp));

      CIMObjectPath copFilter = null;

      ciFilter.setProperty(
          "CreationClassName",
          new CIMValue ("CIM_IndicationFilter"));

      ciFilter.setProperty(
          "SystemCreationClassName",
          new CIMValue ("CIM_ComputerSystem"));

      ciFilter.setProperty(
          "Name",
          new CIMValue ("classCreationFilter"));

      ciFilter.setProperty(
          "SystemName",
          new CIMValue (getSystemName (cc)));

      ciFilter.setProperty(
          "SourceNamespace",
          new CIMValue (nameSpaceClass));

      ciFilter.setProperty(
          "Query",
          new CIMValue("SELECT * FROM " + className));

      ciFilter.setProperty(
          "QueryLanguage",
          new CIMValue("DMTF:CQL"));

      try
      {
         copFilter = createInstance (cc,
                                     ciFilter,
                                     nameSpaceInterOp);
      }
      catch (Exception e)
      {
         System.err.println ("testIndications::testCreateClassIndicationFilter: Exception: " + e);

         e.printStackTrace ();
      }

      if (DEBUG)
      {
         System.err.println ("testIndications::testCreateClassIndicationFilter: copFilter = " + copFilter);
      }

      if (copFilter != null)
      {
         System.out.println ("SUCCESS: testCreateClassIndicationFilter");
      }
      else
      {
         System.out.println ("FAIL: testCreateClassIndicationFilter");
      }

      return copFilter;
   }

   private static CIMObjectPath testCreateClassIndicationHandler (CIMClient cc)
      throws Exception
   {
      CIMInstance   ciHandler  = null;
      CIMObjectPath copHandler = null;

      if (true)
      {
         // CIM_IndicationHandlerCIMXML
         ciHandler = new CIMInstance("CIM_IndicationHandlerCIMXML");
         ciHandler.setObjectPath(
             new CIMObjectPath("CIM_IndicationHandlerCIMXML",nameSpaceInterOp));

         ciHandler.setProperty ("CreationClassName",
                                new CIMValue ("CIM_IndicationHandlerCIMXML"));
         ciHandler.setProperty ("SystemName", new CIMValue (getSystemName (cc)));
         ciHandler.setProperty ("Destination", new CIMValue ("http://" + getHostname () + ":2005/CIMListener/Pegasus_SimpleDisplayConsumer"));
      }
      else if (false)
      {
         // CIM_ListenerDestinationCIMXML
      }
      else if (false)
      {
         // PG_IndicationHandlerSNMPMapper
      }
      else if (false)
      {
         // PG_ListenerDestinationSystemLog
         ciHandler = new CIMInstance("PG_ListenerDestinationSystemLog");
         ciHandler.setObjectPath(
             new CIMObjectPath(
                 "PG_ListenerDestinationSystemLog",
                 nameSpaceInterOp));

         ciHandler.setProperty(
             "CreationClassName",
             new CIMValue ("PG_ListenerDestinationSystemLog"));
      }
      else if (false)
      {
         // PG_ListenerDestinationEmail
         ciHandler = new CIMInstance("PG_ListenerDestinationEmail");
         ciHandler.setObjectPath(
             new CIMObjectPath("PG_ListenerDestinationEmail",nameSpaceInterOp));

         ciHandler.setProperty ("CreationClassName",
                                new CIMValue ("PG_ListenerDestinationEmail"));

         Vector vectorMailTo = new Vector ();

         vectorMailTo.addElement ("root@" + getHostname ());

         ciHandler.setProperty ("MailTo",
                                new CIMValue (vectorMailTo));

         ciHandler.setProperty ("MailSubject",
                                new CIMValue ("Hello Indication"));

         // terminate called after throwing an instance of 'Pegasus::TypeMismatchException'
      }

      ciHandler.setProperty ("Name",
                             new CIMValue ("classCreationHandler"));
      ciHandler.setProperty ("SystemCreationClassName",
                             new CIMValue ("CIM_ComputerSystem"));

      try
      {
         copHandler = createInstance (cc,
                                      ciHandler,
                                      nameSpaceInterOp);
      }
      catch (Exception e)
      {
         if (DEBUG)
         {
            System.err.println ("testIndications::testCreateClassIndicationHandler: Exception: " + e);

            e.printStackTrace ();
         }
      }

      if (DEBUG)
      {
         System.err.println ("testIndications::testCreateClassIndicationHandler: copHandler = " + copHandler);
      }

      if (copHandler != null)
      {
         System.out.println ("SUCCESS: testCreateClassIndicationHandler");
      }
      else
      {
         System.out.println ("FAIL: testCreateClassIndicationHandler");
      }

      return copHandler;
   }

   private static CIMObjectPath testCreateClassIndicationSubscription (CIMClient     cc,
                                                                       CIMObjectPath copFilter,
                                                                       CIMObjectPath copHandler)
      throws Exception
   {
      CIMInstance   ciSubscription  = new CIMInstance("CIM_IndicationSubscription");
      ciSubscription.setObjectPath(new CIMObjectPath ("CIM_IndicationSubscription",nameSpaceInterOp));
      ciSubscription.setProperty ("Filter", new CIMValue (copFilter));
      ciSubscription.setProperty ("Handler", new CIMValue (copHandler));
      ciSubscription.setProperty ("SubscriptionState", new CIMValue (new UnsignedInt16 ("2")));

      CIMObjectPath copSubscription = null;
      try
      {
         copSubscription = createInstance (cc,
                                           ciSubscription,
                                           nameSpaceInterOp);
      }
      catch (Exception e)
      {
         if (DEBUG)
         {
            System.err.println ("testIndications::testCreateClassIndicationHandler: Exception: " + e);

            e.printStackTrace ();
         }
      }

      if (DEBUG)
      {
         System.err.println ("testIndications::testCreateClassIndicationHandler: copSubscription = " + copSubscription);
      }

      if (copSubscription != null)
      {
         System.out.println ("SUCCESS: testCreateClassIndicationSubscription");
      }
      else
      {
         System.out.println ("FAIL: testCreateClassIndicationSubscription");
      }

      return copSubscription;
   }

   private static boolean testCreateIndication (CIMClient cc,
                                                int       iInstanceId)
      throws Exception
   {
      CIMObjectPath copTestIndication    = new CIMObjectPath (className,
                                                              nameSpaceClass);
      String        methodName           = "SendTestIndicationNormal";
      UnsignedInt64 ui64IndicationNumber = findNextIndicationNumber (cc);
      Vector        inArgs               = new Vector ();
      Vector        outArgs              = new Vector ();
      CIMValue      cvRet                = null;

      inArgs.addElement (new CIMProperty ("indicationSendCount",
                                          new CIMValue (ui64IndicationNumber)));

      if (DEBUG)
      {
         System.err.println ("testIndications::testCreateIndication: Calling " + methodName + " (" + inArgs + ", " + outArgs + ")");
      }

      try
      {
         cvRet = cc.invokeMethod (copTestIndication,
                                  methodName,
                                  inArgs,
                                  outArgs);
      }
      catch (Exception e)
      {
         if (DEBUG)
         {
            System.err.println ("Caught " + e);
            e.printStackTrace ();
         }
      }

      if (cvRet == null)
      {
         System.out.println ("FAIL: testCreateIndication: ret == null");

         return false;
      }
      else if (((UnsignedInt32)cvRet.getValue ()).intValue () != 0)
      {
         System.out.println ("FAIL: testCreateIndication: ret (" + cvRet.getValue () + ") != 0");

         return false;
      }
      else
      {
         System.out.println ("SUCCESS: testCreateIndication (" + ui64IndicationNumber + ")");

         return true;
      }
   }

   private static boolean testExecQueryIndication (CIMClient cc)
   {
      CIMObjectPath copTestIndication  = new CIMObjectPath (className,
                                                            nameSpaceClass);
      Enumeration   enm                = null;
      int           iInstancesReturned = 0;

      try
      {
         enm = cc.execQuery (copTestIndication,
                             "SELECT InstanceId FROM " + className + " WHERE InstanceId = 1",
                             "WQL");
      }
      catch (Exception e)
      {
         if (DEBUG)
         {
            System.err.println ("Caught " + e);
            e.printStackTrace ();
         }
      }

      if (enm == null)
      {
         if (fBugExistsExecQuery)
         {
            System.out.println ("IGNORE: testExecQueryIndication: enm == null");

            return true;
         }
         else
         {
            System.out.println ("FAIL: testExecQueryIndication: enm == null");

            return false;
         }
      }

      if (DEBUG)
      {
         System.err.println ("enm.hasMoreElements () = " + enm.hasMoreElements ());
      }

      while (enm.hasMoreElements ())
      {
         CIMInstance elm = (CIMInstance)enm.nextElement ();

         if (DEBUG)
         {
            System.err.println ("elm = " + elm);
         }

         iInstancesReturned++;
      }

      if (iInstancesReturned == 1)
      {
         System.out.println ("SUCCESS: testExecQueryIndication = 1");
      }
      else
      {
         System.out.println ("FAIL: testExecQueryIndication iInstancesReturned != 1 (" + iInstancesReturned + ")");

         return false;
      }

      iInstancesReturned = 0;

      try
      {
         enm = cc.execQuery (copTestIndication,
                             "SELECT InstanceId FROM " + className + " WHERE InstanceId <= 5",
                             "WQL");
      }
      catch (Exception e)
      {
         if (DEBUG)
         {
            System.err.println ("Caught " + e);
            e.printStackTrace ();
         }
      }

      if (enm == null)
      {
         System.out.println ("FAIL: testExecQueryIndication: enm == null");

         return false;
      }

      if (DEBUG)
      {
         System.err.println ("enm.hasMoreElements () = " + enm.hasMoreElements ());
      }

      while (enm.hasMoreElements ())
      {
         CIMInstance elm = (CIMInstance)enm.nextElement ();

         if (DEBUG)
         {
            System.err.println ("elm = " + elm);
         }

         iInstancesReturned++;
      }

      if (iInstancesReturned == 5)
      {
         System.out.println ("SUCCESS: testExecQueryIndication <= 5");
      }
      else
      {
         System.out.println ("FAIL: testExecQueryIndication iInstancesReturned != 5 (" + iInstancesReturned + ")");

         return false;
      }

      return true;
   }

   private static boolean testDeleteClassIndicationSubscription (CIMClient     cc,
                                                                 CIMObjectPath copSubscription)
   {
      try
      {
         cc.deleteInstance (copSubscription);

         System.out.println ("SUCCESS: testDeleteClassIndicationSubscription");

         return true;
      }
      catch (Exception e)
      {
         if (DEBUG)
         {
            System.err.println ("Caught " + e);
            e.printStackTrace ();
         }
      }

      System.out.println ("FAIL: testDeleteClassIndicationSubscription");

      return false;
   }

   private static boolean testDeleteClassIndicationHandler (CIMClient     cc,
                                                            CIMObjectPath copHandler)
   {
      try
      {
         cc.deleteInstance (copHandler);

         System.out.println ("SUCCESS: testDeleteClassIndicationHandler");

         return true;
      }
      catch (Exception e)
      {
         if (DEBUG)
         {
            System.err.println ("Caught " + e);
            e.printStackTrace ();
         }
      }

      System.out.println ("FAIL: testDeleteClassIndicationHandler");

      return false;
   }

   private static boolean testDeleteClassIndicationFilter (CIMClient     cc,
                                                           CIMObjectPath copFilter)
   {
      try
      {
         cc.deleteInstance (copFilter);

         System.out.println ("SUCCESS: testDeleteClassIndicationFilter");

         return true;
      }
      catch (Exception e)
      {
         if (DEBUG)
         {
            System.err.println ("Caught " + e);
            e.printStackTrace ();
         }
      }

      System.out.println ("FAIL: testDeleteClassIndicationFilter");

      return false;
   }

   private static boolean runTests (CIMClient cc)
   {
      CIMObjectPath copFilter       = null;
      CIMObjectPath copHandler      = null;
      CIMObjectPath copSubscription = null;

      try
      {
         copFilter = testCreateClassIndicationFilter (cc);

         if (copFilter == null)
         {
            return false;
         }

         copHandler = testCreateClassIndicationHandler (cc);

         if (copHandler == null)
         {
            return false;
         }

         copSubscription = testCreateClassIndicationSubscription (cc, copFilter, copHandler);

         if (copSubscription == null)
         {
            return false;
         }

         for (int i = 1; i <= iNumIndications; i++)
         {
            if (!testCreateIndication (cc, i))
            {
               return false;
            }
         }

         testExecQueryIndication (cc);

         if (DELETE)
         {
            testDeleteClassIndicationSubscription (cc, copSubscription);
            testDeleteClassIndicationHandler (cc, copHandler);
            testDeleteClassIndicationFilter (cc, copFilter);
         }

         return true;
      }
      catch (Exception e)
      {
         if (DEBUG)
         {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
         }
      }

      return false;
   }
}
