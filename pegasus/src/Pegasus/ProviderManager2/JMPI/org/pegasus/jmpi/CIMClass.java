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
//              Magda
//              Andy
//              Mark Hamzy,    hamzy@us.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

import java.util.Vector;

/**
    Creates and instantiates a CIM class.  In CIM, a class object may
    be a class or an associator.  A CIM class must contain a name and
    may contain methods, properties, and qualifiers.  It is a template
    for creating a CIM instance.  A CIM class represents a collection
    of CIM instances, all of which support a common type (for example,
    a set of properties, methods, and associations).
 */

public class CIMClass implements CIMElement
{
   private long cInst;

   private native long    _newInstance      (long     cInst);
   private native long    _filterProperties (long     cInst,
                                             String[] pl,
                                             boolean  iq,
                                             boolean  ic,
                                             boolean  lo);
   private native String  _getName          (long     cInst);
   private native long    _getQualifier     (long     cInst,
                                             String   n);
   private native Vector  _getQualifiers    (long     cInst,
                                             Vector   vec);
   private native boolean _hasQualifier     (long     cInst,
                                             String   n);
   private native void    _addProperty      (long     cInst,
                                             long     p);
   private native void    _setProperties    (long     cInst,
                                             Vector   vec);
   private native long    _getProperty      (long     cInst,
                                             String   n);
   private native Vector  _getProperties    (long     cInst,
                                             Vector   vec);
   private native long    _new              (String   n);
   private native String  _getSuperClass    (long     cInst);
   private native Vector  _getKeys          (long     cInst,
                                             Vector   vec);
   private native long    _getMethod        (long     cInst,
                                             String   n);
   private native boolean _equals           (long     cInst,
                                             long     cInst1);
   private native boolean _isAssociation    (long     cInst);
   private native int     _findMethod       (long     cInst,
                                             String   name);
   private native long    _getMethodI       (long     cInst,
                                             int      iMethod);
   private native int     _getMethodCount   (long     cInst);
   private native void    _finalize         (long     cInst);

   protected void finalize ()
   {
      _finalize (cInst);
   }

   CIMClass (long ci)
   {
      cInst = ci;
   }

   protected long cInst ()
   {
      return cInst;
   }

   public CIMClass (String n)
   {
      cInst = _new (n);
   }

    /**
       Returns a new instance appropriately initialized
     */
   public CIMInstance newInstance ()
   {
      long ciInst = 0;

      if (cInst != 0)
      {
         ciInst = _newInstance (cInst);
      }

      if (ciInst != 0)
      {
         return new CIMInstance (ciInst);
      }
      else
      {
         return null;
      }
   }

   public CIMClass filterProperties (String  propertyList[],
                                     boolean includeQualifier,
                                     boolean includeClassOrigin)
   {
      long ciClass = 0;

      if (cInst != 0)
      {
         ciClass = _filterProperties (cInst,
                                      propertyList,
                                      includeQualifier,
                                      includeClassOrigin,
                                      false);
      }

      if (ciClass != 0)
      {
         return new CIMClass (ciClass);
      }
      else
      {
         return null;
      }
   }

   public CIMClass localElements ()
   {
      long ciClass = 0;

      if (cInst != 0)
      {
         ciClass = _filterProperties (cInst,
                                      null,
                                      false,
                                      false,
                                      true);
      }

      if (ciClass != 0)
      {
         return new CIMClass (ciClass);
      }
      else
      {
         return null;
      }
   }

   /**
    * getName - returns the name of this class
    *
    * @return String with class name.
    */
   public String getName ()
   {
      if (cInst != 0)
      {
         return _getName (cInst);
      }
      else
      {
         return null;
      }
   }

   public void setName (String n)
   {
      // not done yet
   }

   /**
    * getQualifier - get the specified CIM Qualifier in this class
    *
    * @param String name - The string name of the CIM qualifier.
    * @return CIMQualifier Returns the CIM Qualifier in this class else null
    */
   public CIMQualifier getQualifier (String n)
   {
      long ciQualifier = 0;

      if (cInst != 0)
      {
         ciQualifier = _getQualifier (cInst, n);
      }

      if (ciQualifier != 0)
      {
         return new CIMQualifier (ciQualifier);
      }
      else
      {
         return null;
      }
   }

   public Vector getQualifiers ()
   {
      Vector ret = new Vector ();

      if (cInst != 0)
      {
         _getQualifiers (cInst, ret);
      }

      return ret;
   }

   /**
    * Checks whether this class has the specified qualifier
    *
    * @return true if qualifier defined
    */
   public boolean hasQualifier (String n)
   {
      if (cInst != 0)
      {
         return _hasQualifier (cInst, n);
      }
      else
      {
         return false;
      }
   }

   /**
    * Gets the properties for this CIM class
    */
   public Vector getProperties ()
   {
      Vector ret = new Vector ();

      if (cInst != 0)
      {
         _getProperties (cInst, ret);
      }

      return ret;
   }

   public Vector getAllProperties ()
   {
      return getProperties ();
   }

   /**
    * getProperty - get the CIMProperty for the specified class
    *
    * @param String name - name of the property to get.
    * @return CIMProperty return the CIM property else if exist else Null
    * if the property does not exist
    */
   public CIMProperty getProperty (String n)
   {
      long ciProperty = 0;

      if (cInst != 0)
      {
         ciProperty = _getProperty (cInst, n);
      }

      if (ciProperty != 0)
      {
         return new CIMProperty (ciProperty);
      }
      else
      {
         return null;
      }
   }

   public void addProperty (CIMProperty p)
      throws CIMException
   {
      if (cInst != 0)
      {
         _addProperty (cInst, p.cInst ());
      }
   }

   public void setProperties (Vector v)
   {
      if (cInst != 0)
      {
         _setProperties (cInst, v);
      }
   }

   /**
    * getSuperClass - returns the name of the parent for this class
    *
    * @return String with parent class name.
    */
   public String getSuperClass ()
   {
      if (cInst != 0)
      {
         return _getSuperClass (cInst);
      }
      else
      {
         return null;
      }
   }

   /**
    * getKeys - Returns the properties that are keys for this class
    *
    * @return  Vector containing the list of key properties
    */
   public Vector getKeys ()
   {
      Vector ret = new Vector ();

      if (cInst != 0)
      {
         return _getKeys (cInst, ret);
      }

      return ret;
   }

   /**
    * getMethod
    *
    * @return the specified method
    */
   public CIMMethod getMethod (String n)
   {
      long ciMethod = 0;

      if (cInst != 0)
      {
         ciMethod = _getMethod (cInst, n);
      }

      if (ciMethod != 0)
      {
         return new CIMMethod (ciMethod);
      }
      else
      {
         return null;
      }
   }

   /**
    * Returns a String representation of the CIMClass.
    *
    * @return String empty or cimclass string
    */
   public String toString ()
   {
      return "@ CIMClass.toString() not implemented yet!";
   }

   public boolean equals (Object o)
   {
      if (!(o instanceof CIMClass))
         return(false);

      if (cInst == 0)
         return false;

      CIMClass clsToBeCompared = (CIMClass)o;

      if (  cInst == 0
         || clsToBeCompared.cInst () == 0
         )
      {
         System.out.println ("no cInst found!");

         return false;
      }

      boolean rv = _equals (cInst, clsToBeCompared.cInst ());

      return rv;
   }

   public boolean isAssociation ()
   {
      if (cInst != 0)
      {
         return _isAssociation (cInst);
      }

      return false;
   }

   public int findMethod (String name)
   {
      if (cInst != 0)
      {
         return _findMethod (cInst, name);
      }

      return -1;
   }

   public CIMMethod getMethod (int iMethod)
   {
      if (cInst != 0)
      {
         long ciMethod = _getMethodI (cInst, iMethod);

         if (ciMethod != 0)
         {
            return new CIMMethod (ciMethod);
         }
      }

      return null;
   }

   public int getMethodCount ()
   {
      if (cInst != 0)
      {
         return _getMethodCount (cInst);
      }

      return 0;
   }

   static {
       System.loadLibrary ("JMPIProviderManager");
   }
}
