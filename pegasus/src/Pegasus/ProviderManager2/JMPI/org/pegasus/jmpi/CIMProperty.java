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
// Author:      Adrian Schuur, schuur@de.ibm.com
//
// Modified By: Adrian Duta
//              Mark Hamzy, hamzy@us.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

public class CIMProperty
{
    public static final int PEG_NOT_FOUND = -1;

    private long cInst;

    private native long    _getValue        (long   ci);
    private native String  _getName         (long   ci);
    private native void    _setName         (long   ci,   String n);
    private native long    _property        (String name, long   v);
    private native long    _new             ();
    private native boolean _isReference     (long   ci);
    private native String  _getRefClassName (long   ci);
    private native long    _getType         (long   ci);
    private native long    _setType         (long   ci,   long   t);
    private native void    _setValue        (long   ci,   long   v);
    private native boolean _isArray         (long   ci);
    private native String  _getIdentifier   (long   ci);
    private native void    _addValue        (long   ci,   long   v);
    private native void    _addQualifier    (long   ci,   long   v);
    private native void    _finalize        (long   ci);
    private native int     _findQualifier   (long   ci,   String qualifier);
    private native long    _getQualifier    (long   ci,   int    index);

    protected void finalize ()
    {
       _finalize (cInst);
    }

    CIMProperty (long ci)
    {
       cInst = ci;
    }

    protected long cInst ()
    {
       return cInst;
    }

    public CIMProperty ()
    {
       cInst = _new ();
    }

    public CIMProperty (String name, CIMValue cv)
    {
        cInst = 0;

        if (cv.cInst () == 0)
       	    return;

        cInst = _property (name, cv.cInst ());
    }

    public CIMValue getValue ()
    {
        if (cInst == 0)
            return null;

        long ciValue = _getValue (cInst);

        if (ciValue != 0)
        {
           return new CIMValue (ciValue);
        }
        else
        {
           return null;
        }
    }

    public String getName ()
    {
        if (cInst == 0)
            return null;

        return _getName (cInst);
    }

    public void setName (String n)
    {
        if (cInst == 0)
            return;

        _setName (cInst, n);
    }

    public boolean isReference ()
    {
        if (cInst == 0)
            return false;

        return _isReference (cInst);
    }

    public CIMDataType getType ()
    {
        if (cInst == 0)
            return null;

        long ciDataType = _getType (cInst);

        if (ciDataType != 0)
        {
           return new CIMDataType (ciDataType, true);
        }
        else
        {
           return null;
        }
    }

    public void setType (CIMDataType dt)
    {
        if (cInst == 0 || dt.cInst () == 0)
            return;

        cInst = _setType (cInst, dt.cInst ());
    }

    public String getRefClassName ()
    {
        if (cInst == 0)
            return null;

        return _getRefClassName (cInst);
    }

    public String toString ()
    {
        if (cInst == 0)
            return null;

        return getType().toString() + " " + getName () + "=" + getValue ().toString () + ";";
    }

    public void setValue (CIMValue v)
    {
        if (cInst == 0 || v.cInst () == 0)
            return;

        _setValue (cInst, v.cInst ());
    }

    public void addValue (CIMValue v)
    {
        if (cInst == 0 || v.cInst () == 0)
            return;

        if (!_isArray (cInst))
           return;

        _addValue (cInst, v.cInst ());
    }

    public void addQualifier (CIMQualifier q)
    {
        if (cInst == 0 || q.cInst () == 0)
            return;

        _addQualifier (cInst, q.cInst ());
    }

    public boolean isArray ()
    {
        if (cInst == 0)
            return false;

        return _isArray (cInst);
    }

    public String getIdentifier ()
    {
        if (cInst == 0)
            return null;

        return _getIdentifier (cInst);
    }

    public int findQualifier (String qualifier)
    {
       if (cInst == 0)
          return 0;

       return _findQualifier (cInst, qualifier);
    }

    public CIMQualifier getQualifier (int index)
    {
       long ciQualifier = _getQualifier (cInst, index);

       if (ciQualifier != 0)
       {
          return new CIMQualifier (ciQualifier);
       }

       return null;
    }

    static {
        System.loadLibrary("JMPIProviderManager");
    }
}
