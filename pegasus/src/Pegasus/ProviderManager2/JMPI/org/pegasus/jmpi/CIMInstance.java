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
//
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

import java.util.*;

/**
    Creates and instantiates an instance of a CIM class.  Use this interface
    to describe a managed object that belongs to a particular class.
    Instances contain actual data.  Clients use CIMInstance in conjunction
    with the CIMClient instance methods like createInstance or setInstance
    to manipulate instances within a namespace.
 */
public class CIMInstance
             implements CIMElement
{
    private long   cInst;
    private String name;

    private native long   _new              ();
    private native long   _newCn            (String   n);
    private native long   _filterProperties (long     cInst,
                                             String[] pl,
                                             boolean  iq,
                                             boolean  ic,
                                             boolean  lo);
    private native void   _setName          (long     cInst,
                                             String   n);
    private native void   _setProperty      (long     cInst,
                                             String   n,
                                             long     vInst);
    private native void   _setProperties    (long     cInst,
                                             Vector   v);
    private native long   _getProperty      (long     cInst,
                                             String   n);
    private native Vector _getKeyValuePairs (long     cInst,
                                             Vector   vec);
    private native Vector _getProperties    (long     cInst,
                                             Vector   vec);
    private native String _getClassName     (long     cInst);
    private native long   _getQualifier     (long     cInst,
                                             String   n);
    private native long   _clone            (long     cInst);
    private native void   _finalize         (long     ci);
    private native long   _getObjectPath    (long     cInst);
    private native void   _setObjectPath    (long     cInst,
                                             long     ciCop);
    private native int    _getPropertyCount (long     cInst);
    private native long   _getPropertyI     (long     cInst,
                                             int      i);

    protected void finalize ()
    {
        _finalize (cInst);
    }

    CIMInstance (long ci)
    {
        cInst = ci;
    }

    protected long cInst ()
    {
        return cInst;
    }

    public CIMInstance (String cn)
    {
        name  = cn;
        cInst = _newCn (cn);
    }

    public CIMInstance filterProperties (String  propertyList[],
                                         boolean includeQualifier,
                                         boolean includeClassOrigin)
    {
       long ciInstance = 0;

       if (cInst != 0)
       {
          ciInstance = _filterProperties (cInst,
                                          propertyList,
                                          includeQualifier,
                                          includeClassOrigin,
                                          false);
       }

       if (ciInstance != 0)
       {
          return new CIMInstance (ciInstance);
       }
       else
       {
          return null;
       }
    }

    public CIMInstance localElements ()
    {
       long ciInstance = 0;

       if (cInst != 0)
       {
          ciInstance = _filterProperties (cInst,
                                          null,
                                          false,
                                          false,
                                          true);
       }

       if (ciInstance != 0)
       {
          return new CIMInstance (ciInstance);
       }
       else
       {
          return null;
       }
    }

    public void setName (String n)
    {
        name = n;

        if (cInst == 0)
            return;

        _setName (cInst, n);
    }

    public String getName ()
    {
        return name;
    }

    /**
         sets property value for the CIM value
         @param String property name to set
         @param CIMValue a CIMProperty value
      */
    public void setProperty(String n, CIMValue v)
       throws CIMException
    {
        /* Fix for 4019 */
        if (cInst == 0)
        {
           throw new CIMException (1, "Invalid CIMInstance");
        }
        if (v.cInst () == 0)
        {
           throw new CIMException (1, "Invalid CIMValue");
        }
        /* Fix for 4019 */

        _setProperty (cInst, n, v.cInst ());
    }

    public void setProperty(Vector v)
    {
        if (cInst == 0)
            return;

        _setProperties (cInst, v);
    }

    /**
         Returns a property as specified by the name.
         @param String name  - name of the property
         @return CIMProperty property object the specified name
      */
    public CIMProperty getProperty (String n)
    {
        if (cInst == 0)
            return null;

        long ciProperty = _getProperty (cInst, n);

        if (ciProperty != 0)
           return new CIMProperty (ciProperty);

        return null;
    }

    /**
        Returns the list of key-value pairs for this instance
        @return Vector  list of key-value pairs for this instance
     */
    public Vector getKeyValuePairs ()
    {
        if (cInst == 0)
            return null;

        return _getKeyValuePairs (cInst, new Vector ());
    }

    /**
        Gets the properties list
        @return Vector  properties list
     */
    public Vector getProperties ()
    {
        if (cInst == 0)
            return null;

        return _getProperties (cInst, new Vector ());
    }

    /**
        Returns the class name of the instance
        @return String with the class name.
     */
    public String getClassName()
    {
        if (cInst == 0)
            return null;

        return _getClassName (cInst);
    }

    /**
        getQualifiers - Retrieves the qualifier object defined for this
                        instance.
        @return Vector  list of qualifier objects for the CIMInstance.
     */
    public CIMQualifier getQualifier(String n)
    {
        if (cInst == 0)
            return null;

        long ciQualifier = _getQualifier(cInst,n);

        if (ciQualifier != 0)
            return new CIMQualifier(ciQualifier);

        return null;
    }

    /**
        Returns a String representation of the CIMInstance.
        @return String representation of the CIMInstance
     */
    public String toString ()
    {
        if (cInst == 0)
            return null;

        Vector       v   = getProperties ();
        StringBuffer str = new StringBuffer ("Instance of "+getClassName()+" {\n");

        for (int i = 0, m = v.size (); i < m; i++)
        {
            CIMProperty cp = (CIMProperty)v.elementAt (i);

            str.append ("  " + cp.toString () + "\n");
        }
        str.append ("};\n");

        return str.toString ();
    }

    public Object clone ()
    {
        if (cInst == 0)
            return null;

        long ciNew = _clone (cInst);

        if (ciNew != 0)
        {
            return new CIMInstance (ciNew);
        }
        else
        {
            return null;
        }
    }

    public CIMObjectPath getObjectPath ()
    {
        long ciCop = _getObjectPath (cInst);

        if (ciCop != 0)
        {
            return new CIMObjectPath (ciCop);
        }
        else
        {
            return null;
        }
    }

    public void setObjectPath (CIMObjectPath cop)
    {
       if (cInst != 0)
       {
          _setObjectPath (cInst, cop.cInst ());
       }
    }

    public int getPropertyCount ()
    {
       if (cInst != 0)
       {
          return _getPropertyCount (cInst);
       }
       else
       {
          return 0;
       }
    }

    public CIMProperty getProperty (int i)
    {
        long ciProperty = _getPropertyI (cInst, i);

        if (ciProperty != 0)
        {
            return new CIMProperty (ciProperty);
        }
        else
        {
            return null;
        }
    }

    static {
       System.loadLibrary("JMPIProviderManager");
    }
}
