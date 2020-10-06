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
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMInstance;
import org.pegasus.jmpi.CIMObjectPath;
import java.util.Enumeration;
import java.util.Vector;

public class testExecQuery
{
    private final String        className           = "JMPIExpInstance_TestPropertyTypes";
    private final String        nameSpaceClass      = "root/SampleProvider";

    private static boolean      DEBUG               = false;

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
            else if (args[i].equalsIgnoreCase ("BugExistsExecQuery"))
            {
               fBugExistsExecQuery = true;
            }
        }

        if (!fExecuted)
            return runTests (cimClient);

        return false;
    }

    public boolean runTests (CIMClient cimClient)
    {
       try
       {
          return runTest (cimClient);
       }
       catch (Exception e)
       {
          System.out.println ("Caught " + e);

          e.printStackTrace ();

          return false;
       }
    }

    private boolean runTest (CIMClient cimClient)
       throws CIMException
    {
        String        queryStatement = null;
        String        queryLanguage  = null;
        CIMObjectPath cimObjectPath  = new CIMObjectPath (className, nameSpaceClass);
        Enumeration   enm            = null;

        queryStatement = "Select * from " + className + " where InstanceId = 1";
        queryLanguage  = "WQL";

        if (DEBUG)
            System.out.println ("execQuery (" + cimObjectPath + ", " + queryStatement + ", " + queryLanguage);

        try
        {
           enm = cimClient.execQuery (cimObjectPath,
                                      queryStatement,
                                      queryLanguage);
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
              System.out.println ("IGNORE: testExecQuery: enm == null");

              return true;
           }
           else
           {
              System.out.println ("FAIL: testExecQuery: enm == null");

              return false;
           }
        }

        if (DEBUG)
            System.out.println ("enm = " + enm + ", hasMoreElements = " + enm.hasMoreElements ());

        while (enm.hasMoreElements())
        {
           CIMInstance cimInstanceEQ = (CIMInstance)(enm.nextElement());

           System.out.println (cimInstanceEQ);
        }

        queryStatement = "Select * from " + className + " where InstanceId = 2";
        queryLanguage  = "WQL";

        if (DEBUG)
            System.out.println ("execQuery (" + cimObjectPath + ", " + queryStatement + ", " + queryLanguage);

        try
        {
           enm = cimClient.execQuery (cimObjectPath,
                                      queryStatement,
                                      queryLanguage);
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
           System.out.println ("FAIL: testExecQuery: enm == null");

           return false;
        }
        if (DEBUG)
            System.out.println ("enm = " + enm + ", hasMoreElements = " + enm.hasMoreElements ());

        while (enm.hasMoreElements())
        {
           CIMInstance cimInstanceEQ = (CIMInstance)(enm.nextElement());

           System.out.println (cimInstanceEQ);
        }

        System.out.println ("SUCCESS: testExecQuery");

        return true;
    }
}
