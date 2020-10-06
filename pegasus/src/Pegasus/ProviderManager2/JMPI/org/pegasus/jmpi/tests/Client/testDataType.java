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

import java.math.BigInteger;
import java.util.Vector;
import org.pegasus.jmpi.CIMClass;
import org.pegasus.jmpi.CIMClient;
import org.pegasus.jmpi.CIMDataType;
import org.pegasus.jmpi.CIMDateTime;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMInstance;
import org.pegasus.jmpi.CIMObject;
import org.pegasus.jmpi.CIMObjectPath;
import org.pegasus.jmpi.CIMProperty;
import org.pegasus.jmpi.CIMValue;
import org.pegasus.jmpi.UnsignedInt16;
import org.pegasus.jmpi.UnsignedInt32;
import org.pegasus.jmpi.UnsignedInt64;
import org.pegasus.jmpi.UnsignedInt8;

public class testDataType
{
    private final String className        = "JMPIExpInstance_TestPropertyTypes";
    private final String nameSpaceClass   = "root/SampleProvider";

    private boolean      DEBUG            = false;

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
       throws CIMException,
              Exception
    {
        CIMObjectPath copTest       = null;
        CIMInstance   cimInstance   = null;
        boolean       fRet          = true;

        copTest = new CIMObjectPath (className, nameSpaceClass);
        if (copTest == null)
        {
            System.err.println ("ERROR: Could not create a CIMObjectPath ('" + className + "', '" + nameSpaceClass + "');");

            return false;
        }

        copTest.addKey ("CreationClassName", new CIMValue (new String (className)));
        copTest.addKey ("InstanceId", new CIMValue (new UnsignedInt64 ("1")));

        if (DEBUG)
        {
           System.err.println ("copTest      = " + copTest);
        }

        cimInstance = cimClient.getInstance (copTest, false);

        if (DEBUG)
        {
           System.err.println ("cimInstance = " + cimInstance);
        }

        if (runTestUINT8 (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: UINT8.");
        }
        else
        {
           System.out.println ("ERROR: UINT8 failed!");

           return false;
        }
        if (runTestSINT8 (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: SINT8.");
        }
        else
        {
           System.out.println ("ERROR: SINT8 failed!");

           return false;
        }
        if (runTestUINT16 (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: UINT16.");
        }
        else
        {
           System.out.println ("ERROR: UINT16 failed!");

           return false;
        }
        if (runTestSINT16 (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: SINT16.");
        }
        else
        {
           System.out.println ("ERROR: SINT16 failed!");

           return false;
        }
        if (runTestUINT32 (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: UINT32.");
        }
        else
        {
           System.out.println ("ERROR: UINT32 failed!");

           return false;
        }
        if (runTestSINT32 (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: SINT32.");
        }
        else
        {
           System.out.println ("ERROR: SINT32 failed!");

           return false;
        }
        if (runTestUINT64 (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: UINT64.");
        }
        else
        {
           System.out.println ("ERROR: UINT64 failed!");

           return false;
        }
        if (runTestSINT64 (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: SINT64.");
        }
        else
        {
           System.out.println ("ERROR: SINT64 failed!");

           return false;
        }
        if (runTestSTRING (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: STRING.");
        }
        else
        {
           System.out.println ("ERROR: STRING failed!");

           return false;
        }
        if (runTestBOOLEAN (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: BOOLEAN.");
        }
        else
        {
           System.out.println ("ERROR: BOOLEAN failed!");

           return false;
        }
        if (runTestREAL32 (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: REAL32.");
        }
        else
        {
           System.out.println ("ERROR: REAL32 failed!");

           return false;
        }
        if (runTestREAL64 (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: REAL64.");
        }
        else
        {
           System.out.println ("ERROR: REAL64 failed!");

           return false;
        }
        if (runTestDATETIME (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: DATETIME.");
        }
        else
        {
           System.out.println ("ERROR: DATETIME failed!");

           return false;
        }
        if (runTestCHAR16 (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: CHAR16.");
        }
        else
        {
           System.out.println ("ERROR: CHAR16 failed!");

           return false;
        }
        if (runTestINSTANCE (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: INSTANCE.");
        }
        else
        {
           System.out.println ("ERROR: INSTANCE failed!");

           return false;
        }
        if (runTestCLASS (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: CLASS.");
        }
        else
        {
           System.out.println ("ERROR: CLASS failed!");

           return false;
        }
        if (runTestUINT8_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: UINT8_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: UINT8_ARRAY failed!");

           return false;
        }
        if (runTestSINT8_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: SINT8_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: SINT8_ARRAY failed!");

           return false;
        }
        if (runTestUINT16_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: UINT16_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: UINT16_ARRAY failed!");

           return false;
        }
        if (runTestSINT16_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: SINT16_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: SINT16_ARRAY failed!");

           return false;
        }
        if (runTestUINT32_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: UINT32_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: UINT32_ARRAY failed!");

           return false;
        }
        if (runTestSINT32_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: SINT32_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: SINT32_ARRAY failed!");

           return false;
        }
        if (runTestUINT64_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: UINT64_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: UINT64_ARRAY failed!");

           return false;
        }
        if (runTestSINT64_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: SINT64_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: SINT64_ARRAY failed!");

           return false;
        }
        if (runTestSTRING_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: STRING_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: STRING_ARRAY failed!");

           return false;
        }
        if (runTestBOOLEAN_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: BOOLEAN_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: BOOLEAN_ARRAY failed!");

           return false;
        }
        if (runTestREAL32_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: REAL32_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: REAL32_ARRAY failed!");

           return false;
        }
        if (runTestREAL64_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: REAL64_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: REAL64_ARRAY failed!");

           return false;
        }
        if (runTestDATETIME_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: DATETIME_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: DATETIME_ARRAY failed!");

           return false;
        }
        if (runTestCHAR16_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: CHAR16_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: CHAR16_ARRAY failed!");

           return false;
        }
        if (runTestINSTANCE_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: INSTANCE_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: INSTANCE_ARRAY failed!");

           return false;
        }
        if (runTestCLASS_ARRAY (cimClient, cimInstance, copTest))
        {
           System.out.println ("SUCCESS: CLASS_ARRAY.");
        }
        else
        {
           System.out.println ("ERROR: CLASS_ARRAY failed!");
        }

        return fRet;
    }

    private boolean runTestUINT8 (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set UINT8
            CIMValue cv = null;

            cv = new CIMValue (new UnsignedInt8 ((byte)16), new CIMDataType (CIMDataType.UINT8));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a uint8");
            }

            cimInstance.setProperty ("PropertyUint8", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get UINT8
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyUint8");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            UnsignedInt8 ui8         = (UnsignedInt8)o;

            if (DEBUG)
                System.err.println ("ui8         = " + ui8);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestSINT8 (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set SINT8
            CIMValue cv = null;

            cv = new CIMValue (new Byte ((byte)-42), new CIMDataType (CIMDataType.SINT8));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a sint8");
            }

            cimInstance.setProperty ("PropertySint8", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get SINT8
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertySint8");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Byte b                   = (Byte)o;

            if (DEBUG)
                System.err.println ("b           = " + b);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestUINT16 (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set UINT16
            CIMValue cv = null;

            cv = new CIMValue (new UnsignedInt16 ((int)9831), new CIMDataType (CIMDataType.UINT16));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a Uint16");
            }

            cimInstance.setProperty ("PropertyUint16", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get UINT16
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyUint16");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            UnsignedInt16 ui16       = (UnsignedInt16)o;

            if (DEBUG)
                System.err.println ("ui16        = " + ui16);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestSINT16 (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set SINT16
            CIMValue cv = null;

            cv = new CIMValue (new Short ((short)-1578), new CIMDataType (CIMDataType.SINT16));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a sint16");
            }

            cimInstance.setProperty ("PropertySint16", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get SINT16
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertySint16");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Short s                  = (Short)o;

            if (DEBUG)
                System.err.println ("s           = " + s);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestUINT32 (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set UINT32
            CIMValue cv = null;

            cv = new CIMValue (new UnsignedInt32 (33000L), new CIMDataType (CIMDataType.UINT32));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a Uint32");
            }

            cimInstance.setProperty ("PropertyUint32", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get UINT32
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyUint32");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            UnsignedInt32 ui32       = (UnsignedInt32)o;

            if (DEBUG)
                System.err.println ("ui32        = " + ui32);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestSINT32 (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set SINT32
            CIMValue cv = null;

            cv = new CIMValue (new Integer (-45000), new CIMDataType (CIMDataType.SINT32));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a Sint32");
            }

            cimInstance.setProperty ("PropertySint32", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get SINT32
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertySint32");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Integer i                = (Integer)o;

            if (DEBUG)
                System.err.println ("i           = " + i);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestUINT64 (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set UINT64
            CIMValue cv = null;

            cv = new CIMValue (new UnsignedInt64 (new BigInteger ("77000000")), new CIMDataType (CIMDataType.UINT64));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a Uint64");
            }

            cimInstance.setProperty ("PropertyUint64", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get UINT64
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyUint64");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            UnsignedInt64 ui64       = (UnsignedInt64)o;

            if (DEBUG)
                System.err.println ("ui64        = " + ui64);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestSINT64 (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set SINT64
            CIMValue cv = null;

            cv = new CIMValue (new Long ((long)-99000000), new CIMDataType (CIMDataType.SINT64));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a Sint64");
            }

            cimInstance.setProperty ("PropertySint64", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get SINT64
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertySint64");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Long l                   = (Long)o;

            if (DEBUG)
                System.err.println ("l           = " + l);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestSTRING (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set STRING
            CIMValue cv = null;

            cv = new CIMValue ("Bob lives", new CIMDataType (CIMDataType.STRING));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a String");
            }

            cimInstance.setProperty ("PropertyString", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get STRING
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyString");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            String s                 = (String)o;

            if (DEBUG)
                System.err.println ("s           = " + s);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestBOOLEAN (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set BOOLEAN
            CIMValue cv = null;

            cv = new CIMValue (new Boolean (false), new CIMDataType (CIMDataType.BOOLEAN));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a Boolean");
            }

            cimInstance.setProperty ("PropertyBoolean", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get BOOLEAN
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyBoolean");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Boolean b                = (Boolean)o;

            if (DEBUG)
                System.err.println ("b           = " + b);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestREAL32 (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set REAL32
            CIMValue cv = null;

            cv = new CIMValue (new Float (3.14159), new CIMDataType (CIMDataType.REAL32));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a Real32");
            }

            cimInstance.setProperty ("PropertyReal32", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get REAL32
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyReal32");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Float f                  = (Float)o;

            if (DEBUG)
                System.err.println ("f           = " + f);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestREAL64 (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set REAL64
            CIMValue cv = null;

            cv = new CIMValue (new Double (2.718281828), new CIMDataType (CIMDataType.REAL64));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a Real64");
            }

            cimInstance.setProperty ("PropertyReal64", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get REAL64
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyReal64");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Double d                 = (Double)o;

            if (DEBUG)
                System.err.println ("d           = " + d);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestDATETIME (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set DATETIME
            CIMValue cv = null;

            cv = new CIMValue (new CIMDateTime ("20051216161710.000000:000"), new CIMDataType (CIMDataType.DATETIME));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a DateTime");
            }

            cimInstance.setProperty ("PropertyDateTime", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get DATETIME
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyDateTime");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            CIMDateTime cdt          = (CIMDateTime)o;

            if (DEBUG)
                System.err.println ("char        = " + cdt);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestCHAR16 (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set CHAR16
            CIMValue cv = null;

            cv = new CIMValue (new Character ('!'), new CIMDataType (CIMDataType.CHAR16));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a Char16");
            }

            cimInstance.setProperty ("PropertyChar16", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get CHAR16
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyChar16");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Character   ch           = (Character)o;

            if (DEBUG)
                System.err.println ("char        = " + ch);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestINSTANCE (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set INSTANCE
            CIMValue cv = null;

            cv = new CIMValue (new CIMObject (cimInstance), new CIMDataType (CIMDataType.OBJECT));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an object");
            }

            cimInstance.setProperty ("PropertyObject", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get INSTANCE
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyObject");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            CIMObject   cimObject    = (CIMObject)o;
            CIMInstance cimInstance2 = cimObject.getCIMInstance ();

            if (DEBUG)
                System.err.println ("cimInstance2 = " + cimInstance2);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestCLASS (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set CLASS
            CIMValue cv       = null;
            CIMClass cimClass = null;

            cimClass = cimClient.getClass (cop, false);

            if (DEBUG)
                System.err.println ("cimClass     = " + cimClass);

            cv = new CIMValue (new CIMObject (cimClass), new CIMDataType (CIMDataType.OBJECT));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting a class");
            }

            cimInstance.setProperty ("PropertyObject", cv);

            if (DEBUG)
                System.err.println ("cimInstance = " + cimInstance);

            // get CLASS
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyObject");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            CIMObject   cimObject = (CIMObject)o;
            CIMClass    cimClass2 = cimObject.getCIMClass ();

            if (DEBUG)
                System.err.println ("cimClass2    = " + cimClass2);

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestUINT8_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set UINT8_ARRAY
            CIMValue cv          = null;
            Vector   vectorUINT8 = new Vector ();

            vectorUINT8.addElement (new UnsignedInt8 ((byte)64));
            vectorUINT8.addElement (new UnsignedInt8 ((byte)127));
            vectorUINT8.addElement (new UnsignedInt8 ((byte)10));

            cv = new CIMValue (vectorUINT8, new CIMDataType (CIMDataType.UINT8_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of uint8s");
            }

            cimInstance.setProperty ("PropertyArrayUint8", cv);

            // get UINT8_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArrayUint8");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            UnsignedInt8 ui8[]       = (UnsignedInt8[])o;

            if (DEBUG)
            {
                System.err.print ("ui8         = [");
                for (int i = 0; i < ui8.length; i++)
                {
                    System.err.print (ui8[i]);
                    if (i < ui8.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestSINT8_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set SINT8_ARRAY
            CIMValue cv          = null;
            Vector   vectorSINT8 = new Vector ();

            vectorSINT8.addElement (new Byte ((byte)0));
            vectorSINT8.addElement (new Byte ((byte)-128));
            vectorSINT8.addElement (new Byte ((byte)-77));

            cv = new CIMValue (vectorSINT8, new CIMDataType (CIMDataType.SINT8_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of sint8s");
            }

            cimInstance.setProperty ("PropertyArraySint8", cv);

            // get SINT8_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArraySint8");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Byte si8[] = (Byte[])o;

            if (DEBUG)
            {
                System.err.print ("si8         = [");
                for (int i = 0; i < si8.length; i++)
                {
                    System.err.print (si8[i]);
                    if (i < si8.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestUINT16_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set UINT16_ARRAY
            CIMValue cv           = null;
            Vector   vectorUINT16 = new Vector ();

            vectorUINT16.addElement (new UnsignedInt16 ((int)65535));
            vectorUINT16.addElement (new UnsignedInt16 ((int)33333));
            vectorUINT16.addElement (new UnsignedInt16 ((int)11111));

            cv = new CIMValue (vectorUINT16, new CIMDataType (CIMDataType.UINT16_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of uint16s");
            }

            cimInstance.setProperty ("PropertyArrayUint16", cv);

            // get UINT16_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArrayUint16");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            UnsignedInt16 ui16[] = (UnsignedInt16[])o;

            if (DEBUG)
            {
                System.err.print ("ui16        = [");
                for (int i = 0; i < ui16.length; i++)
                {
                    System.err.print (ui16[i]);
                    if (i < ui16.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestSINT16_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set SINT16_ARRAY
            CIMValue cv           = null;
            Vector   vectorSINT16 = new Vector ();

            vectorSINT16.addElement (new Short ((byte)0));
            vectorSINT16.addElement (new Short ((byte)-6173));
            vectorSINT16.addElement (new Short ((byte)-32768));

            cv = new CIMValue (vectorSINT16, new CIMDataType (CIMDataType.SINT16_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of sint16s");
            }

            cimInstance.setProperty ("PropertyArraySint16", cv);

            // get SINT16_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArraySint16");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Short si16[] = (Short[])o;

            if (DEBUG)
            {
                System.err.print ("si16        = [");
                for (int i = 0; i < si16.length; i++)
                {
                    System.err.print (si16[i]);
                    if (i < si16.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestUINT32_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set UINT32_ARRAY
            CIMValue cv           = null;
            Vector   vectorUINT32 = new Vector ();

            vectorUINT32.addElement (new UnsignedInt32 (4294967295L));
            vectorUINT32.addElement (new UnsignedInt32 (2222222222L));
            vectorUINT32.addElement (new UnsignedInt32 (108L));

            cv = new CIMValue (vectorUINT32, new CIMDataType (CIMDataType.UINT32_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of uint32s");
            }

            cimInstance.setProperty ("PropertyArrayUint32", cv);

            // get UINT32_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArrayUint32");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            UnsignedInt32 ui32[] = (UnsignedInt32[])o;

            if (DEBUG)
            {
                System.err.print ("ui32        = [");
                for (int i = 0; i < ui32.length; i++)
                {
                    System.err.print (ui32[i]);
                    if (i < ui32.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestSINT32_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set SINT32_ARRAY
            CIMValue cv           = null;
            Vector   vectorSINT32 = new Vector ();

            vectorSINT32.addElement (new Integer (-12345));
            vectorSINT32.addElement (new Integer (2147483647));
            vectorSINT32.addElement (new Integer (-2147483648));

            cv = new CIMValue (vectorSINT32, new CIMDataType (CIMDataType.SINT32_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of sint32s");
            }

            cimInstance.setProperty ("PropertyArraySint32", cv);

            // get SINT32_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArraySint32");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Integer si32[] = (Integer[])o;

            if (DEBUG)
            {
                System.err.print ("si32        = [");
                for (int i = 0; i < si32.length; i++)
                {
                    System.err.print (si32[i]);
                    if (i < si32.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestUINT64_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set UINT64_ARRAY
            String   apszValues[] = {
               "7482743276264381934",
               "11122233344455566677",
               "18446744073709551615"
            };
            CIMValue cv           = null;
            Vector   vectorUINT64 = new Vector ();
            int      idxValue     = 0;

            vectorUINT64.addElement (new UnsignedInt64 (new BigInteger (apszValues[idxValue++])));
            vectorUINT64.addElement (new UnsignedInt64 (new BigInteger (apszValues[idxValue++])));
            vectorUINT64.addElement (new UnsignedInt64 (new BigInteger (apszValues[idxValue++])));

            cv = new CIMValue (vectorUINT64, new CIMDataType (CIMDataType.UINT64_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of uint64s");
            }

            cimInstance.setProperty ("PropertyArrayUint64", cv);

            // get UINT64_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArrayUint64");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            UnsignedInt64 aui64[] = (UnsignedInt64[])o;

            if (DEBUG)
            {
                System.err.print ("aui64       = [");
                for (int i = 0; i < aui64.length; i++)
                {
                    System.err.print (aui64[i]);
                    if (i < aui64.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            for (int i = 0; i < aui64.length; i++)
            {
               BigInteger    bi   = new BigInteger (apszValues[i]);
               UnsignedInt64 ui64 = new UnsignedInt64 (bi);

               if (aui64[i].compareTo ((Object)ui64) != 0)
               {
                  return false;
               }
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestSINT64_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set SINT64_ARRAY
            CIMValue cv = null;

            Vector vectorSINT64 = new Vector ();

            vectorSINT64.addElement (new Long (1736513846286344121L));
            vectorSINT64.addElement (new Long (-9223372036854775808L));
            vectorSINT64.addElement (new Long (9223372036854775807L));

            cv = new CIMValue (vectorSINT64, new CIMDataType (CIMDataType.SINT64_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of sint64s");
            }

            cimInstance.setProperty ("PropertyArraySint64", cv);

            // get SINT64_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArraySint64");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Long si64[] = (Long[])o;

            if (DEBUG)
            {
                System.err.print ("si64        = [");
                for (int i = 0; i < si64.length; i++)
                {
                    System.err.print (si64[i]);
                    if (i < si64.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestSTRING_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set STRING_ARRAY
            CIMValue cv           = null;
            Vector   vectorSTRING = new Vector ();

            vectorSTRING.addElement (new String ("Hello World."));
            vectorSTRING.addElement (new String ("This is a test."));
            vectorSTRING.addElement (new String ("Bob lives!"));

            cv = new CIMValue (vectorSTRING, new CIMDataType (CIMDataType.STRING_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of strings");
            }

            cimInstance.setProperty ("PropertyArrayString", cv);

            // get STRING_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArrayString");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            String string[] = (String[])o;

            if (DEBUG)
            {
                System.err.print ("string      = [");
                for (int i = 0; i < string.length; i++)
                {
                    System.err.print (string[i]);
                    if (i < string.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestBOOLEAN_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set BOOLEAN_ARRAY
            CIMValue cv            = null;
            Vector   vectorBOOLEAN = new Vector ();

            vectorBOOLEAN.addElement (new Boolean (true));
            vectorBOOLEAN.addElement (new Boolean (false));
            vectorBOOLEAN.addElement (new Boolean (true));

            cv = new CIMValue (vectorBOOLEAN, new CIMDataType (CIMDataType.BOOLEAN_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of booleans");
            }

            cimInstance.setProperty ("PropertyArrayBoolean", cv);

            // get BOOLEAN_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArrayBoolean");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Boolean b[] = (Boolean[])o;

            if (DEBUG)
            {
                System.err.print ("b           = [");
                for (int i = 0; i < b.length; i++)
                {
                    System.err.print (b[i]);
                    if (i < b.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestREAL32_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set REAL32_ARRAY
            CIMValue cv           = null;
            Vector   vectorREAL32 = new Vector ();

            vectorREAL32.addElement (new Float (1.23456789012345678e-01));
            vectorREAL32.addElement (new Float (1.40129846432481707e-45));
            vectorREAL32.addElement (new Float (3.40282346638528860e+38));

            cv = new CIMValue (vectorREAL32, new CIMDataType (CIMDataType.REAL32_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of REAL32s");
            }

            cimInstance.setProperty ("PropertyArrayREAL32", cv);

            // get REAL32_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArrayREAL32");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Float f[] = (Float[])o;

            if (DEBUG)
            {
                System.err.print ("f           = [");
                for (int i = 0; i < f.length; i++)
                {
                    System.err.print (f[i]);
                    if (i < f.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestREAL64_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set REAL64_ARRAY
            CIMValue cv           = null;
            Vector   vectorREAL64 = new Vector ();

            vectorREAL64.addElement (new Double (9.876));
            vectorREAL64.addElement (new Double (4.94065645841246544e-324d));
            vectorREAL64.addElement (new Double (1.79769313486231570e+308d));

            cv = new CIMValue (vectorREAL64, new CIMDataType (CIMDataType.REAL64_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of REAL64s");
            }

            cimInstance.setProperty ("PropertyArrayREAL64", cv);

            // get REAL64_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArrayREAL64");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Double d[] = (Double[])o;

            if (DEBUG)
            {
                System.err.print ("d           = [");
                for (int i = 0; i < d.length; i++)
                {
                    System.err.print (d[i]);
                    if (i < d.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestDATETIME_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set DATETIME_ARRAY
            CIMValue cv             = null;
            Vector   vectorDATETIME = new Vector ();

            vectorDATETIME.addElement (new CIMDateTime ("20051216161710.000000:000"));
            vectorDATETIME.addElement (new CIMDateTime ("20041115151609.100000:000"));
            vectorDATETIME.addElement (new CIMDateTime ("20031014141508.200000:000"));

            cv = new CIMValue (vectorDATETIME, new CIMDataType (CIMDataType.DATETIME_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of datetime");
            }

            cimInstance.setProperty ("PropertyArrayDatetime", cv);

            // get DATETIME_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArrayDatetime");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            CIMDateTime cdt[] = (CIMDateTime[])o;

            if (DEBUG)
            {
                System.err.print ("cdt         = [");
                for (int i = 0; i < cdt.length; i++)
                {
                    System.err.print (cdt[i]);
                    if (i < cdt.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestCHAR16_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set CHAR16_ARRAY
            CIMValue cv           = null;
            Vector   vectorCHAR16 = new Vector ();

            vectorCHAR16.addElement (new Character ('B'));
            vectorCHAR16.addElement (new Character ('o'));
            vectorCHAR16.addElement (new Character ('b'));

            cv = new CIMValue (vectorCHAR16, new CIMDataType (CIMDataType.CHAR16_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of char16");
            }

            cimInstance.setProperty ("PropertyArrayChar16", cv);

            // get CHAR16_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArrayChar16");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            Character c[] = (Character[])o;

            if (DEBUG)
            {
                System.err.print ("c           = [");
                for (int i = 0; i < c.length; i++)
                {
                    System.err.print (c[i]);
                    if (i < c.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestINSTANCE_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set INSTANCE_ARRAY
            CIMValue cv             = null;
            Vector   vectorINSTANCE = new Vector ();

            vectorINSTANCE.addElement (new CIMObject (cimInstance));
            vectorINSTANCE.addElement (new CIMObject (cimInstance));
            vectorINSTANCE.addElement (new CIMObject (cimInstance));

            cv = new CIMValue (vectorINSTANCE, new CIMDataType (CIMDataType.OBJECT_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of objects");
            }

            cimInstance.setProperty ("PropertyArrayObject", cv);

            // get INSTANCE_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArrayObject");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            CIMObject co[] = (CIMObject[])o;

            for (int i = 0; i < co.length; i++)
            {
                CIMInstance cimInstance2 = co[i].getCIMInstance ();

                if (cimInstance2 == null)
                    return false;
            }

            if (DEBUG)
            {
                System.err.print ("co          = [");
                for (int i = 0; i < co.length; i++)
                {
                    System.err.print (co[i]);
                    if (i < co.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }

    private boolean runTestCLASS_ARRAY (CIMClient cimClient, CIMInstance cimInstance, CIMObjectPath cop)
    {
        try
        {
            // set CLASS_ARRAY
            CIMValue cv          = null;
            CIMClass cimClass    = null;
            Vector   vectorCLASS = new Vector ();

            cimClass = cimClient.getClass (cop, false);

            if (DEBUG)
                System.err.println ("cimClass     = " + cimClass);

            vectorCLASS.addElement (new CIMObject (cimClass));
            vectorCLASS.addElement (new CIMObject (cimClass));
            vectorCLASS.addElement (new CIMObject (cimClass));

            cv = new CIMValue (vectorCLASS, new CIMDataType (CIMDataType.OBJECT_ARRAY));

            if (DEBUG)
            {
                System.err.println ("cv           = " + cv);
                System.err.println ("Setting an array of objects");
            }

            cimInstance.setProperty ("PropertyArrayObject", cv);

            // get CLASS_ARRAY
            CIMProperty cp = null;

            cp = cimInstance.getProperty ("PropertyArrayObject");

            if (DEBUG)
                System.err.println ("cp          = " + cp);

            cv = cp.getValue ();

            if (DEBUG)
                System.err.println ("cv          = " + cv);

            Object o = cv.getValue (false);

            if (DEBUG)
                System.err.println ("o           = " + o);

            CIMObject co[] = (CIMObject[])o;

            for (int i = 0; i < co.length; i++)
            {
                CIMClass cimClass2 = co[i].getCIMClass ();

                if (cimClass2 == null)
                    return false;
            }

            if (DEBUG)
            {
                System.err.print ("co          = [");
                for (int i = 0; i < co.length; i++)
                {
                    System.err.print (co[i]);
                    if (i < co.length - 1)
                        System.err.print (",");
                }
                System.err.println ("]");
            }

            return true;
        }
        catch (Exception e)
        {
            System.out.println ("Caught " + e);
            e.printStackTrace ();
            return false;
        }
    }
}
