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

import java.io.FilenameFilter;
import java.io.File;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import org.pegasus.jmpi.CIMClient;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMNameSpace;

/**
 *
 */
public class RunTest
{
   /**
    * This is the java equivalent of #define.  It is the starting pattern to
    * search for.
    */
   public static String       patternStart   = "test";
   /**
    * This is the java equivalent of #define.  It is the ending pattern to
    * search for.
    */
   public static final String patternEnd     = ".class";
   /**
    * Use the hypertext transfer protocol.
    */
   public static final int    PROTOCOL_HTTP  = 1;
   /**
    * Use the secure hypertext transfer protocol.
    */
   public static final int    PROTOCOL_HTTPS = 2;

   /**
    * This is the class constructor.
    *
    * @param args These are the command line arguments.
    */
   public RunTest (String[] args)
   {
   }

   public void setDebug (boolean fDebug)
   {
       DEBUG = fDebug;
   }

   /**
    * Class printing out method <code>System.out.println ()</code>
    *
    * @return String The human readable version.
    */
   public String toString ()
   {
      java.util.Hashtable h = new java.util.Hashtable ();
      Class               c = getClass ();
      Field[]             f = c.getDeclaredFields ();

      for (int i = 0; i < f.length; i++)
      {
         if (Modifier.isFinal (f[i].getModifiers ()))
            // Don't print #defines
            continue;

         try
         {
            Object o = f[i].get (this);

            if (o == null)
               o = "null";
            h.put (f[i].getName (), o);
         }
         catch (IllegalAccessException e)
         {
            h.put (f[i].getName (), "???");
         }
      }

      return c.getName () + "@" + this.hashCode () + h;
   }

   /**
    * This is the required main entry point.
    *
    * @param args These are the command line arguments.
    */
   public void Main (String[] args)
   {
      args_d = args;

      for (int i = 0; i < args.length; i++)
      {
         if (args[i].startsWith ("--protocol="))
         {
            if (args[i].equalsIgnoreCase ("--protocol=http"))
            {
               protocol_d = PROTOCOL_HTTP;
            }
            else if (args[i].equalsIgnoreCase ("--protocol=https"))
            {
               protocol_d = PROTOCOL_HTTPS;
            }
            else
            {
               System.out.println ("Error: Invalid protocol " + args[i]);

               System.exit (1);
            }
         }
      }

      if (protocol_d == PROTOCOL_HTTPS)
      {
         portNumber_d = 5989;
      }

      for (int i = 0; i < args.length; i++)
      {
          if (args[i].equalsIgnoreCase ("debug"))
          {
              setDebug (true);
          }
          else if (args[i].startsWith ("--patternStart="))
          {
             patternStart = args[i].substring (15);
          }
          else if (args[i].startsWith ("--port="))
          {
             String port = args[i].substring (7);

             portNumber_d = Integer.parseInt (port);
          }
      }

      htTests_d = findTests ();

      if (DEBUG)
      {
         System.err.println ("Main: " + htTests_d);
      }

      if (htTests_d == null)
      {
         System.out.println ("Error: Could not find any testcases!");

         System.exit (1);
      }

      Enumeration enm = htTests_d.elements ();

      if (!enm.hasMoreElements ())
      {
         System.out.println ("Error: Could not find any testcases!");

         System.exit (1);
      }

      while (enm.hasMoreElements ())
      {
         TestEntry te = (TestEntry)enm.nextElement ();

         if (!runTest (te))
         {
            System.exit (1);
         }
      }
   }

   /**
    * This is the required main entry point.
    *
    * @param args These are the command line arguments.
    */
   public static void main (String[] args)
   {
      RunTest p = new RunTest (args);

      p.Main (args);
   }

   private static String getHostname ()
   {
      try
      {
         InetAddress addr = InetAddress.getLocalHost ();

         // Get IP Address
         byte[] ipAddr = addr.getAddress ();

         // Get hostname
         return addr.getHostName ();
      }
      catch (UnknownHostException e)
      {
      }

      return "localhost";
   }

   private CIMClient connectToClient ()
   {
      CIMClient     cimClient        = null;
      String        username         = "";
      String        password         = "";
      String        urlAttach        = null;
      CIMNameSpace  clientNameSpace  = null;

      if (protocol_d == PROTOCOL_HTTP)
      {
         urlAttach = "http://" + hostName + ":" + portNumber_d;
      }
      else if (protocol_d == PROTOCOL_HTTPS)
      {
         urlAttach = "https://" + hostName + ":" + portNumber_d;
      }
      else
      {
         System.out.println ("Error: Invalid protocol specified " + protocol_d);

         System.exit (1);
      }

      clientNameSpace = new CIMNameSpace (urlAttach, nameSpaceInterOp);
      if (clientNameSpace == null)
      {
          System.out.println ("Error: Could not create a CIMNameSpace (url: '" + urlAttach + "', namespace: '" + nameSpaceInterOp + "');");

          System.exit (1);
      }

      try
      {
          cimClient = new CIMClient (clientNameSpace, username, password);
      }
      catch (Exception e)
      {
          System.out.println ("Caught " + e);
          e.printStackTrace ();
      }
      if (cimClient == null)
      {
          System.out.println ("Error: Could not create a CIMClient (namespace: '" + clientNameSpace + "', username: '" + username + "', password: '" + password + "');");

          System.exit (1);
      }

      if (DEBUG)
          System.err.println ("connectToClient: cimClient    = " + cimClient);

      return cimClient;
   }

   /**
    * This is called to notify a test class that it must execute
    *
    * @param szTestClassName This is the menu name of the test class.
    */
   private boolean runTest (TestEntry testEntry)
   {
      Object    currentTestObject = null;
      Class     currentTestClass  = null;
      String    szTestClassName   = testEntry.getClassName ();
      CIMClient cimClient         = null;

      System.out.println ("RUNNING: " + szTestClassName);

      try
      {
         cimClient = connectToClient ();

         // Try to load it
/////////currentTestClass = loader_d.loadClass (szTestClassName);
         currentTestClass = Class.forName (szTestClassName);

         // Success!  Try to instantiate it
         try
         {
            Class[]    paramTypes;
            Object[]   paramArgs;
            Method     method;
            Object     retVal;

            currentTestObject = currentTestClass.newInstance ();

            // Try to get the method
            try
            {
               method = findMethod (currentTestObject, "main");

               if (DEBUG)
               {
                  System.err.println ("runTest: main = " + method);
               }

               if (null != method)
               {
                  paramArgs = new Object[] {
                     args_d,
                     cimClient
                  };

                  // Try to dynamically call it
                  try
                  {
                     retVal = method.invoke (testEntry.getInstance (), paramArgs);
                  }
                  catch (Exception e)
                  {
                     System.out.println ("FAIL: " + szTestClassName + " 1: " + e + ", " + method);

                     if (DEBUG)
                     {
                        System.err.println ("runTest: tried invoke, caught " + e);

                        e.printStackTrace ();
                     }

                     return false;
                  }

                  Boolean bRet = (Boolean)retVal;

                  return bRet.booleanValue ();
               }
            }
            catch (Exception e)
            {
               System.out.println ("FAIL: " + szTestClassName + " 2: " + e);

               if (DEBUG)
               {
                  System.err.println ("FAIL: tried invoke on \"" + szTestClassName + "\", caught " + e);

                  e.printStackTrace ();
               }

               return false;
            }
         }
         catch (Exception e)
         {
            System.out.println ("FAIL: " + szTestClassName + " 3: " + e);

            if (DEBUG)
            {
               System.err.println ("runTest: caught " + e + " trying newInstance");

               e.printStackTrace ();
            }

            return false;
         }
      }
      catch (ClassNotFoundException e)
      {
         System.out.println ("FAIL: " + szTestClassName + " 4: " + e);

         if (DEBUG)
         {
            System.err.println ("runTest: loading '" + szTestClassName + "' failed!");

            e.printStackTrace ();
         }

         return false;
      }
      finally
      {
         if (cimClient != null)
         {
            try
            {
               cimClient.close ();
            }
            catch (CIMException e)
            {
               if (DEBUG)
               {
                  System.err.println ("runTest: Error: CIMException: " + e);

                  e.printStackTrace ();
               }
            }
         }
      }

      return true;
   }

   /**
    * This is called to create a list of test classes that are found
    * by searching the CLASSPATH.
    *
    * @return Hashtable The list of test classes that were found.
    */
   private Hashtable findTests ()
   {
      Hashtable h             = new Hashtable ();
      String    szClassPath   = System.getProperty ("java.class.path");
      String    szCurrentItem;
      int       iSeparator    = -1;

      if (DEBUG)
      {
         System.err.println ("findTests: " + szClassPath);
      }

      // Loop through the CLASSPATH
      szCurrentItem = szClassPath;
      do
      {
         // Is there a separator?
         iSeparator = szClassPath.indexOf (File.pathSeparatorChar);
         if (0 <= iSeparator)
         {
            // Yes. Split into first and rest.
            szCurrentItem = szClassPath.substring (0, iSeparator);
            szClassPath   = szClassPath.substring (iSeparator + 1, szClassPath.length ());
         }
         else
         {
            // Nope.
            szCurrentItem = szClassPath;
            szClassPath   = "";
         }

         if (szCurrentItem.equals (""))
            continue;

         String szLowerItem = szCurrentItem.toLowerCase ();

         if (szLowerItem.endsWith (".zip"))
         {
            if (DEBUG)
            {
               System.err.println ("findTests: ZIP: " + szCurrentItem);
            }

            try
            {
               ZipFile  zf = new ZipFile (szCurrentItem);

               for (Enumeration e = zf.entries (); e.hasMoreElements (); )
               {
                  ZipEntry ze        = (ZipEntry)e.nextElement ();
                  String szName      = ze.getName ();
                  String szMatchName;

                  if (DEBUG)
                  {
                     System.err.println ("findTests: ZIPFILE: " + szName);
                  }

                  szMatchName = MyFilenameFilter.matches (szName,
                                                          patternStart,
                                                          patternEnd);
                  if (szMatchName != null)
                  {
                     if (DEBUG)
                     {
                        System.err.println ("findTests: found zip class: "+szMatchName);
                     }

                     addTestEntry (h,
                                   szMatchName,
                                   new TestEntry (szMatchName,
                                                  TestEntry.ZIPFILE,
                                                  ze,
                                                  patternEnd));
                  }
               }
            }
            catch (IOException e)
            {
               System.err.println ("findTests: Caught " + e);

               e.printStackTrace ();
            }
         }
         else if (szLowerItem.endsWith (".jar"))
         {
            if (DEBUG)
            {
               System.err.println ("findTests: JAR: " + szCurrentItem);
            }

            try
            {
               JarFile jf = new JarFile (szCurrentItem);

               for (Enumeration e = jf.entries (); e.hasMoreElements (); )
               {
                  JarEntry je          = (JarEntry)e.nextElement ();
                  String   szName      = je.getName ();
                  String   szMatchName;

                  if (DEBUG)
                  {
                     System.err.println ("findTests: JARFILE: " + szName);
                  }

                  szMatchName = MyFilenameFilter.matches (szName,
                                                          patternStart,
                                                          patternEnd);
                  if (szMatchName != null)
                  {
                     if (DEBUG)
                     {
                        System.err.println ("findTests: found jar class: "+szMatchName);
                     }

                     addTestEntry (h,
                                   szMatchName,
                                   new TestEntry (szMatchName,
                                                  TestEntry.JARFILE,
                                                  je,
                                                  patternEnd));
                  }
               }
            }
            catch (IOException e)
            {
               System.err.println ("findTests: Caught " + e);

               e.printStackTrace ();
            }
         }
         else
         {
            if (DEBUG)
            {
               System.err.println ("findTests: FILE: " + szCurrentItem);
            }

            // Search through the subdirectory for files matching MyFilenameFilter ()
            recurseSubdirectories (new File (szCurrentItem), "", h);
         }

      } while (0 <= iSeparator);

      return h;
   }

   private void recurseSubdirectories (File file, String baseName, Hashtable h)
   {
      if (file.isDirectory ())
      {
         String[] files = file.list ();

         for (int i = 0; i < files.length; i++)
         {
            File subdir = new File (file.getPath () + File.separator + files[i]);

            if (subdir.isDirectory ())
            {
               String newBaseName;

               if (baseName.length () == 0)
               {
                  newBaseName = files[i];
               }
               else
               {
                  newBaseName = baseName + File.separator + files[i];
               }

               recurseSubdirectories (subdir, newBaseName, h);
            }
         }

         files = file.list (new MyFilenameFilter (patternStart, patternEnd));

         if (DEBUG)
         {
            System.err.print ("recurseSubdirectories: " + baseName + ", {");
            for (int i = 0; i < files.length; i++)
            {
               System.err.print (files[i]);
               if (i < files.length - 1)
               {
                  System.err.print (",");
               }
            }
            System.err.println ("}");
         }

         for (int i = 0; i < files.length; i++)
         {
            // The submenu name will be the name of the java class file without the
            // .class part
            String szMatchName     = new String (files[i].substring (0,
                                                                     files[i].length ()
                                                                     - patternEnd.length ()));
            String szFullClassName = baseName + "." + szMatchName;

            if (DEBUG)
            {
               System.err.println ("recurseSubdirectories: found class: " + szFullClassName);
            }

            addTestEntry (h,
                          szMatchName,
                          new TestEntry (szMatchName,
                                         TestEntry.FILE,
                                         szFullClassName,
                                         patternEnd));
         }
      }
   }

   /**
    * This will add an individual test entry to the list of test
    * entries.  It will do some validation that the class support all
    * of the APIs that we need it to support.
    *
    * @param  ht      This is the hash table to add into.
    * @param  szKey   This is the key to set for the hash table entry.
    * @param  de      This is the value to set for the hash table entry.
    * @return boolean The success return code.
    */
   private boolean addTestEntry (Hashtable ht, String szKey, TestEntry de)
   {
      String szClassName;
      Class  loadedClass;
      Object loadedObject;
      Method method;
      String szGroupName = null;

      // See if it already exists in the hashtable
      if (null != ht.get (szKey))
         // It does...
         return false;

      szClassName = de.getClassName ();

      // Try to load it
      try
      {
         if (DEBUG)
         {
            System.err.println ("addTestEntry: Getting class '" + szClassName + "'");
         }

/////////loadedClass = loader_d.loadClass (szClassName);
         loadedClass = Class.forName (szClassName);

         // Success!  Try to instantiate it
         try
         {
            loadedObject = loadedClass.newInstance ();

            // Success!
            // Now, try to see if it supports the required methods...
            method = findMethod (loadedObject, "main");
            if (null == method)
            {
               System.err.println ("addTestEntry: Object '" + szClassName + "' doest have method main");

               return false;
            }
            method = findMethod (loadedObject, "getGroup");
            if (null == method)
            {
               System.err.println ("addTestEntry: Object '" + szClassName + "' doest have method getGroup");

               return false;
            }

            // No parameters
            Object[] paramArgs = { };

            // Try to dynamically call it
            try
            {
               szGroupName = (String)method.invoke (loadedObject, paramArgs);

               if (DEBUG)
               {
                  System.err.println ("addTestEntry: getGroup () returns " + szGroupName);
               }
            }
            catch (Exception e)
            {
               System.err.println ("addTestEntry: tried invoke getGroup, caught " + e);

               e.printStackTrace ();

               return false;
            }

            // Update the group entry
            de.setGroup (szGroupName);

            // Update the instance
            de.setInstance (loadedObject);

            ht.put (szKey, de);

            return true;
         }
         catch (Exception e)
         {
            System.err.println ("addTestEntry: caught " + e + " trying newInstance");

            e.printStackTrace ();
         }
      }
      catch (ClassNotFoundException e)
      {
         System.err.println ("addTestEntry: loading '" + szClassName + "' failed!");

         e.printStackTrace ();
      }

      return false;
   }

   /**
    * This is for debugging use.  It prints out the methods in a class.
    *
    * @param o  The object instance of the class to print out.
    */
   private static void showMethods (Object o)
   {
      Class c = o.getClass ();

      Method[] theMethods = c.getMethods ();

      for (int i = 0; i < theMethods.length; i++)
      {
         String methodString = theMethods[i].getName ();

         System.out.println ("Name: " + methodString);

         String returnString = theMethods[i].getReturnType().getName ();

         System.out.println ("   Return Type: " + returnString);

         Class[] parameterTypes = theMethods[i].getParameterTypes ();

         System.out.print ("   Parameter Types:");

         for (int k = 0; k < parameterTypes.length; k ++)
         {
            String parameterString = parameterTypes[k].getName ();

            System.out.print (" " +  parameterString);
         }

         System.out.println ();
      }
   }

   /**
    * This will find a method entry for an object.
    *
    * @param  o            This is the object to search.
    * @param  szMethodName This is the name of the method.
    * @return Method       The method if found.
    */
   private Method findMethod (Object o, String szMethodName)
   {
      Method   method;
      Class    c          = o.getClass ();
      Method[] theMethods = c.getMethods ();

      for (int i = 0; i < theMethods.length; i++)
      {
         method = theMethods[i];

         String methodString = method.getName ();

         if (methodString.equals (szMethodName))
         {
            // Success!
            return method;
         }
      }

      // Failure!
      return null;
   }

////**
/// * This is the class loader to use to load test classes.
/// */
///private SimpleClassLoader   loader_d;
   /**
    * This is the list of test classes.
    */
   private Hashtable           htTests_d;
   private String[]            args_d;
   private boolean             DEBUG            = false;
   private final String        nameSpaceInterOp = "test/PG_InterOp";
   private final String        hostName         = "localhost";
   private int                 protocol_d       = PROTOCOL_HTTP;
   private int                 portNumber_d     = 5988;
}

/**
 * This is the filter class that returns .class entries that match patterns.
 *
 * @author  Mark Hamzy
 */
class MyFilenameFilter implements FilenameFilter
{
   /**
    * This is the class constructor.
    *
    * @param szStart This is the starting pattern that must match.
    * @param szEnd   This is the ending pattern that must match.
    */
   MyFilenameFilter (String szStart, String szEnd)
   {
      szStart_d = szStart;
      szEnd_d   = szEnd;
   }

   /**
    * This is a static function that will implements the matching
    * algorithm.
    *
    * @param  szName  This is the first string.
    * @param  szStart This is the starting pattern.
    * @param  szEnd   This is the ending pattern.
    * @return String  If it matches, then the part left after the start
    *                 and end are removed from the string.
    */
   public static String matches (String szName, String szStart, String szEnd)
   {
      int cbStart = szStart.length ();
      int cbEnd   = szEnd.length ();
      int cbName  = szName.length ();

      // name at least long enough to match following string subexpressions
      if (cbStart + cbEnd <= cbName)
      {
         int iSlash = szName.lastIndexOf ('/');
         if (0 == iSlash)
         {
            iSlash = szName.lastIndexOf ('\\');
         }
         if (0 < iSlash)
         {
            szName = szName.substring (iSlash + 1, cbName);
            cbName = szName.length ();
         }

         if (  szName.startsWith (szStart)
            && szName.endsWith (szEnd)
            && -1 == szName.indexOf ('$')
            )
            // it matches the "test*.class" pattern without $'s in it
            return szName.substring (0, cbName - cbEnd);
      }

      return null;
   }

   /**
    * This is required for the FilenameFilter interface.
    *
    * @param  dir     This is the directory entry.
    * @param  szName  This is the name to match.
    * @return boolean This is the return code of the function. true
    *                 means it matches.
    */
   public boolean accept (File dir, String szName)
   {
      return matches (szName, szStart_d, szEnd_d) != null;
   }

   /**
    * This is the starting pattern.
    */
   private String  szStart_d;
   /**
    * This is the ending pattern.
    */
   private String  szEnd_d;
}
