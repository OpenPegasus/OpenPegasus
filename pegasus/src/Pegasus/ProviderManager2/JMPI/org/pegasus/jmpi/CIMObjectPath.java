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
// Modified By: Magda
//
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

import java.util.*;

public class CIMObjectPath
{
   private long cInst;

   private native long        _new           ();
   private native long        _newCn         (String cn);
   private native long        _newCnNs       (String cn,
                                              String ns);
   private native long        _newCi         (long   cInst);
   private native long        _newCiNs       (long   cInst,
                                              String ns);
   private native void        _finalize      (long   cInst);
   private native String      _getNameSpace  (long   cInst);
   private native void        _setNameSpace  (long   cInst,
                                              String ns);
   private native String      _getHost       (long   cInst);
   private native void        _setHost       (long   cInst,
                                              String hn);
   private native String      _getObjectName (long   cInst);
   private native void        _setObjectName (long   cInst,
                                              String objectName);
   private native Vector      _getKeys       (long   cInst,
                                              Vector v);
   private native void        _setKeys       (long   cInst,
                                              Vector v);
   private native void        _addKey        (long   cInst,
                                              String key,
                                              long   vInst);
   private native String      _getKeyValue   (long   cInst,
                                              String keyValue);
   private native String      _toString      (long   cInst);
   private native long        _clone         (long   cInst);
   private static native long _set           (String copStr);

   CIMObjectPath (long ci)
   {
      cInst = ci;
   }

   protected long cInst ()
   {
      return cInst;
   }

   protected void finalize ()
   {
      _finalize (cInst);
   }

   public CIMObjectPath ()
   {
      cInst = _new ();
   }

   public CIMObjectPath (String className)
   {
      if (className == null)
         cInst = _new ();
      else
         cInst = _newCn (className);
   }

   public CIMObjectPath (String className, String nameSpace)
   {
      if (nameSpace == null)
         cInst = _newCn (className);
      else
         cInst = _newCnNs (className, nameSpace);
  }

   public CIMObjectPath (String className, Vector keyValuePairs)
   {
      cInst = _newCn (className);
      if (keyValuePairs != null)
      {
         if (cInst != 0)
         {
            _setKeys (cInst, keyValuePairs);
         }
      }
   }

   public CIMObjectPath (CIMInstance ci)
   {
      cInst = _newCi (ci.cInst ());
   }

   public CIMObjectPath (CIMInstance ci, String ns)
   {
      cInst = _newCiNs (ci.cInst (), ns);
   }

   public Object clone ()
   {
      long ciNew = _clone (cInst);

      if (ciNew != 0)
      {
         return new CIMObjectPath (ciNew);
      }
      else
      {
         return null;
      }
   }

   public String getHost ()
   {
      if (cInst != 0)
      {
         return _getHost (cInst);
      }
      else
      {
         return null;
      }
   }

   public void setHost (String hn)
   {
      if (cInst != 0)
      {
         _setHost (cInst, hn);
      }
   }

   public String getNameSpace ()
   {
      if (cInst != 0)
      {
         return _getNameSpace (cInst);
      }
      else
      {
         return null;
      }
   }

   public void setNameSpace (String ns)
   {
      if (cInst != 0)
      {
         _setNameSpace (cInst, ns);
      }
   }

   public String getObjectName ()
   {
      if (cInst != 0)
      {
         return _getObjectName (cInst);
      }
      else
      {
         return null;
      }
   }

   public void setObjectName (String objectName)
   {
      if (cInst != 0)
      {
         _setObjectName (cInst, objectName);
      }
   }

   public Vector getKeys ()
   {
      Vector ret = new Vector ();

      if (cInst != 0)
      {
         _getKeys (cInst, ret);
      }

      return ret;
   }

   public String getKeyValue (String keyName)
   {
      if (cInst != 0)
      {
         return _getKeyValue (cInst, keyName);
      }
      else
      {
         return null;
      }
   }

   public void setKeys (Vector keys)
   {
      if (cInst != 0)
      {
         _setKeys (cInst, keys);
      }
   }

   public void addKey (String key, CIMValue val)
   {
      if (cInst != 0)
      {
         _addKey (cInst, key, val.cInst ());
      }
   }

   public String toString ()
   {
      if (cInst != 0)
      {
         return _toString (cInst);
      }
      else
      {
         return null;
      }
   }

   public static CIMObjectPath toCop (String copStr)
   {
      long ciObjectPath = _set (copStr);

      if (ciObjectPath != 0)
      {
         return new CIMObjectPath (ciObjectPath);
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
