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
// Author:      Mark Hamzy, hamzy@us.ibm.com
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////
package Properties;

import java.util.Vector;
import java.util.HashMap;
import java.util.Iterator;
import org.pegasus.jmpi.CIMClass;
import org.pegasus.jmpi.CIMDataType;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMInstance;
import org.pegasus.jmpi.CIMObjectPath;
import org.pegasus.jmpi.CIMOMHandle;
import org.pegasus.jmpi.CIMProperty;
import org.pegasus.jmpi.CIMValue;
import org.pegasus.jmpi.InstanceProvider;
import org.pegasus.jmpi.PropertyProvider;
import org.pegasus.jmpi.UnsignedInt8;
import org.pegasus.jmpi.UnsignedInt16;
import org.pegasus.jmpi.UnsignedInt32;
import org.pegasus.jmpi.UnsignedInt64;

public class JMPIPropertyProvider
             implements InstanceProvider,
                        PropertyProvider
{
   private CIMOMHandle         ch                      = null;
   private SortableVector      paths                   = new SortableVector (new CIMObjectPathComparer ());
   private SortableVector      instances               = new SortableVector (new CIMInstanceComparer ());
   private boolean             fEnableModifications    = true;
   private final boolean       DEBUG                   = true;

   public class ConvertibleVector extends Vector
   {
      public Object buildArray (Class type)
      {
         Object copy = java.lang.reflect.Array.newInstance (type, elementCount);

         System.arraycopy (elementData, 0, copy, 0, elementCount);

         return copy;
      }

      // Assumes there's at least one element and it's not null!
      public Object buildArray ()
      {
         if (elementCount > 0 && elementData[0] != null)
         {
            return buildArray (elementData[0].getClass ());
         }
         else
         {
            throw new IllegalArgumentException ("cannot convert to array");
         }
      }
   }

   private interface Compare
   {
      abstract boolean lessThan        (Object lhs, Object rhs);
      abstract boolean lessThanOrEqual (Object lhs, Object rhs);
   }

   private class SortableVector
                 extends Vector
   {
      private Compare compare;

      public SortableVector (Compare compare)
      {
         this.compare = compare;
      }

      public void sort ()
      {
         quickSort (0, size() - 1);
      }

      private void quickSort (int left, int right)
      {
         if (right > left)
         {
            Object o1 = elementAt (right);
            int    i  = left - 1;
            int    j  = right;

            while (true)
            {
               while (compare.lessThan (elementAt (++i), o1))
                  ;

               while (j > 0)
                  if(compare.lessThanOrEqual (elementAt (--j), o1))
                     break;

               if (i >= j)
                  break;

               swap (i, j);
            }

            swap (i , right);
            quickSort (left, i - 1);
            quickSort (i + 1, right);
         }
      }

      private void swap (int loc1, int loc2)
      {
         Object tmp = elementAt(loc1);

         setElementAt (elementAt (loc2), loc1);
         setElementAt (tmp, loc2);
      }
   }

   private class CIMInstanceComparer
                 implements Compare
   {
      private int getInstanceId (Object o)
      {
         try
         {
            CIMInstance       ci = (CIMInstance)o;
            CIMProperty       cp = ci.getProperty ("InstanceId");
            UnsignedInt64     id = (UnsignedInt64)cp.getValue ().getValue ();

            return id.intValue ();
         }
         catch (Exception e)
         {
            return 0;
         }
      }

      public boolean lessThan (Object lhs, Object rhs)
      {
         return getInstanceId (lhs) < getInstanceId (rhs);
      }

      public boolean lessThanOrEqual (Object lhs, Object rhs)
      {
         return getInstanceId (lhs) <= getInstanceId (rhs);
      }
   }

   private class CIMObjectPathComparer
                 implements Compare
   {
      private int getInstanceId (Object o)
      {
         try
         {
            CIMObjectPath     cop   = (CIMObjectPath)o;
            String            value = cop.getKeyValue ("InstanceId");
            UnsignedInt64     id    = new UnsignedInt64 (value);

            return id.intValue ();
         }
         catch (Exception e)
         {
            return 0;
         }
      }

      public boolean lessThan (Object lhs, Object rhs)
      {
         return getInstanceId (lhs) < getInstanceId (rhs);
      }

      public boolean lessThanOrEqual (Object lhs, Object rhs)
      {
         return getInstanceId (lhs) <= getInstanceId (rhs);
      }
   }

   public void initialize (CIMOMHandle ch)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIPropertyProvider::initialize: ch = " + ch);
      }

      this.ch = ch;

      CIMInstance instance1 = new CIMInstance (CLASS_PROPERTYTYPES);

      instance1.setProperty ("CreationClassName", new CIMValue (new String (CLASS_PROPERTYTYPES)));
      instance1.setProperty ("InstanceId",        new CIMValue (new UnsignedInt64 ("1")));
      instance1.setProperty ("PropertyString",    new CIMValue (new String ("A first property string.")));

      instances.addElement (instance1);

      CIMInstance instance2 = new CIMInstance (CLASS_PROPERTYTYPES);

      instance2.setProperty ("CreationClassName", new CIMValue (new String (CLASS_PROPERTYTYPES)));
      instance2.setProperty ("InstanceId",        new CIMValue (new UnsignedInt64 ("2")));
      instance2.setProperty ("PropertyString",    new CIMValue (new String ("The second property string.")));

      instances.addElement (instance2);

      CIMObjectPath cop1 = new CIMObjectPath (CLASS_PROPERTYTYPES, NAMESPACE);

      cop1.addKey ("CreationClassName", new CIMValue (new String (CLASS_PROPERTYTYPES)));
      cop1.addKey ("InstanceId",        new CIMValue (new UnsignedInt64 ("1")));
      cop1.addKey ("PropertyString",    new CIMValue (new String ("A first property string.")));

      paths.addElement (cop1);

      CIMObjectPath cop2 = new CIMObjectPath (CLASS_PROPERTYTYPES, NAMESPACE);

      cop2.addKey ("CreationClassName", new CIMValue (new String (CLASS_PROPERTYTYPES)));
      cop2.addKey ("InstanceId",        new CIMValue (new UnsignedInt64 ("2")));
      cop2.addKey ("PropertyString",    new CIMValue (new String ("The second property string.")));

      paths.addElement (cop2);
   }

   public void cleanup ()
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIPropertyProvider::cleanup");
      }
   }

   public CIMObjectPath createInstance (CIMObjectPath cop,
                                        CIMInstance   cimInstance)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIPropertyProvider::createInstance: cop          = " + cop);
         System.err.println ("JMPIPropertyProvider::createInstance: cimInstance  = " + cimInstance);
      }

      // ensure the Namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (NAMESPACE))
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);

      // ensure the class existing in the specified namespace
      if (!cop.getObjectName ().equalsIgnoreCase (CLASS_PROPERTYTYPES))
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);

      // Ensure that the instance contains the required keys
      cop = validateInstance (cimInstance, false);

      if (cop == null)
         throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);

      // ensure the property values are valid
      testPropertyTypesValue (cimInstance);

      // ensure the requested object does not exist
      if (findObjectPath (cop) >= 0)
         throw new CIMException (CIMException.CIM_ERR_ALREADY_EXISTS);

      if (fEnableModifications)
      {
         paths.addElement (cop);
         instances.addElement (cimInstance);

         paths.sort ();
         instances.sort ();

         return cop;
      }
      else
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
      }
   }

   public CIMInstance getInstance (CIMObjectPath cop,
                                   CIMClass      cimClass,
                                   boolean       localOnly)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIPropertyProvider::getInstance: cop       = " + cop);
         System.err.println ("JMPIPropertyProvider::getInstance: cimClass  = " + cimClass);
         System.err.println ("JMPIPropertyProvider::getInstance: localOnly = " + localOnly);
      }

      // ensure the Namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (NAMESPACE))
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);

      // ensure the class existing in the specified namespace
      if (!cop.getObjectName ().equalsIgnoreCase (CLASS_PROPERTYTYPES))
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);

      // ensure the InstanceId key is valid
      Vector keys = cop.getKeys ();
      int    i;

      for (i = 0;
               i < keys.size ()
           && !((CIMProperty)keys.elementAt (i)).getName ().equalsIgnoreCase ("InstanceId");
           i++)
      {
      }

      if (i == keys.size ())
         throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);

      // ensure the request object exists
      int index = findObjectPath (cop);

      if (index < 0)
         throw new CIMException (CIMException.CIM_ERR_NOT_FOUND);

      return (CIMInstance)instances.elementAt (index);
   }

   public void setInstance (CIMObjectPath cop,
                            CIMInstance   cimInstance)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIPropertyProvider::setInstance: cop         = " + cop);
         System.err.println ("JMPIPropertyProvider::setInstance: cimInstance = " + cimInstance);
      }

      // ensure the Namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (NAMESPACE))
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);

      // ensure the class existing in the specified namespace
      if (!cop.getObjectName ().equalsIgnoreCase (CLASS_PROPERTYTYPES))
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);

      // ensure the property values are valid
      testPropertyTypesValue (cimInstance);

      // ensure the request object exists
      int index = findObjectPath (cop);

      if (index < 0)
         throw new CIMException (CIMException.CIM_ERR_NOT_FOUND);

      if (fEnableModifications)
      {
         paths.removeElementAt (index);
         instances.removeElementAt (index);

         paths.addElement (cop);
         instances.addElement (cimInstance);

         paths.sort ();
         instances.sort ();
      }
      else
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
      }
   }

   public void deleteInstance (CIMObjectPath cop)
      throws CIMException
   {
      if (DEBUG)
         System.err.println ("JMPIPropertyProvider::deleteInstance: cop = " + cop);

      // ensure the Namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (NAMESPACE))
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);

      // ensure the class existing in the specified namespace
      if (!cop.getObjectName ().equalsIgnoreCase (CLASS_PROPERTYTYPES))
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);

      // ensure the request object exists
      int index = findObjectPath (cop);

      if (index < 0)
         throw new CIMException (CIMException.CIM_ERR_NOT_FOUND);

      if (fEnableModifications)
      {
         paths.removeElementAt (index);
         instances.removeElementAt (index);

         paths.sort ();
         instances.sort ();
      }
      else
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
      }
   }

   public Vector enumInstances (CIMObjectPath cop,
                                boolean       deep,
                                CIMClass      cimClass)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIPropertyProvider::enumInstances: cop      = " + cop);
         System.err.println ("JMPIPropertyProvider::enumInstances: deep     = " + deep);
         System.err.println ("JMPIPropertyProvider::enumInstances: cimClass = " + cimClass);
      }

      // ensure the Namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (NAMESPACE))
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);

      // ensure the class existing in the specified namespace
      if (!cop.getObjectName ().equalsIgnoreCase (CLASS_PROPERTYTYPES))
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);

      return paths;
   }

   public Vector enumInstances (CIMObjectPath cop,
                                boolean       deep,
                                CIMClass      cimClass,
                                boolean       localOnly)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIPropertyProvider::enumInstances: cop       = " + cop);
         System.err.println ("JMPIPropertyProvider::enumInstances: deep      = " + deep);
         System.err.println ("JMPIPropertyProvider::enumInstances: cimClass  = " + cimClass);
         System.err.println ("JMPIPropertyProvider::enumInstances: localOnly = " + localOnly);
      }

      // ensure the Namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (NAMESPACE))
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);

      // ensure the class existing in the specified namespace
      if (!cop.getObjectName ().equalsIgnoreCase (CLASS_PROPERTYTYPES))
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);

      return instances;
   }

   public Vector execQuery (CIMObjectPath    cop,
                            String           queryStatement,
                            int              queryLanguage,
                            CIMClass         cimClass)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIPropertyProvider::execQuery: cop            = " + cop);
         System.err.println ("JMPIPropertyProvider::execQuery: queryStatement = " + queryStatement);
         System.err.println ("JMPIPropertyProvider::execQuery: queryLanguage  = " + queryLanguage);
         System.err.println ("JMPIPropertyProvider::execQuery: cimClass       = " + cimClass);
      }

      // ensure the Namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (NAMESPACE))
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);

      // ensure the class existing in the specified namespace
      if (!cop.getObjectName ().equalsIgnoreCase (CLASS_PROPERTYTYPES))
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);

      throw new CIMException (CIMException.CIM_ERR_NOT_SUPPORTED);
   }

   public void setPropertyValue (CIMObjectPath cop,
                                 String        oclass,
                                 String        pName,
                                 CIMValue      val)
   {
      if (DEBUG)
      {
         System.err.println ("JMPIPropertyProvider::setPropertyValue: cop    = " + cop   );
         System.err.println ("JMPIPropertyProvider::setPropertyValue: oclass = " + oclass);
         System.err.println ("JMPIPropertyProvider::setPropertyValue: pName  = " + pName );
         System.err.println ("JMPIPropertyProvider::setPropertyValue: val    = " + val   );
      }

      try
      {
         CIMInstance cimInstance = getInstance (cop, new CIMClass (oclass), true);

         if (DEBUG)
         {
            System.err.println ("JMPIPropertyProvider::setPropertyValue: cimInstance     = " + cimInstance);
         }

         CIMProperty cp = cimInstance.getProperty (pName);

         if (DEBUG)
         {
            System.err.println ("JMPIPropertyProvider::setPropertyValue: cp     = " + cp);
         }

         cp.setValue (val);

         if (DEBUG)
         {
            System.err.println ("JMPIPropertyProvider::setPropertyValue: cp     = " + cp);
         }
      }
      catch (Exception e)
      {
         System.err.println ("JMPIPropertyProvider::setPropertyValue: caught = " + e);
      }
   }

   public CIMValue getPropertyValue (CIMObjectPath cop,
                                     String        oclass,
                                     String        pName)
   {
      CIMValue cv = null;

      if (DEBUG)
      {
         System.err.println ("JMPIPropertyProvider::setPropertyValue: cop    = " + cop   );
         System.err.println ("JMPIPropertyProvider::setPropertyValue: oclass = " + oclass);
         System.err.println ("JMPIPropertyProvider::setPropertyValue: pName  = " + pName );
      }

      try
      {
         CIMInstance cimInstance = getInstance (cop, new CIMClass (oclass), true);

         if (DEBUG)
         {
            System.err.println ("JMPIPropertyProvider::setPropertyValue: cimInstance     = " + cimInstance);
         }

         CIMProperty cp = cimInstance.getProperty (pName);

         if (DEBUG)
         {
            System.err.println ("JMPIPropertyProvider::setPropertyValue: cp     = " + cp);
         }

         if (cp != null)
         {
            cv = cp.getValue ();
         }

         if (DEBUG)
         {
            System.err.println ("JMPIPropertyProvider::setPropertyValue: cv     = " + cv);
         }
      }
      catch (Exception e)
      {
         System.err.println ("JMPIPropertyProvider::setPropertyValue: caught = " + e);
      }

      return cv;
   }

   private CIMObjectPath validateInstance (CIMInstance cimInstance,
                                           boolean     fStrict)
   {
      if (cimInstance.getPropertyCount () != PROPERTYTYPES_PROPS.length)
      {
         if (DEBUG)
         {
            System.err.println ("JMPIPropertyProvider::validateInstance: Bad1: " + cimInstance.getPropertyCount () + " != " + PROPERTYTYPES_PROPS.length);
         }

         if (fStrict)
         {
            return null;
         }
      }
      for (int i = 0, m = PROPERTYTYPES_PROPS.length; i < m; i++)
      {
         CIMProperty cp = cimInstance.getProperty (PROPERTYTYPES_PROPS[i]);
         if (cp == null)
         {
            if (DEBUG)
            {
               System.err.println ("JMPIPropertyProvider::validateInstance: Bad2: " + PROPERTYTYPES_PROPS[i]);
            }

            if (fStrict)
            {
               return null;
            }
         }
      }

      CIMObjectPath ret = new CIMObjectPath (CLASS_PROPERTYTYPES,
                                             NAMESPACE);

      for (int i = 0, m = PROPERTYTYPES_KEYS.length; i < m; i++)
      {
         CIMProperty cp = cimInstance.getProperty (PROPERTYTYPES_KEYS[i]);

         if (cp != null)
         {
            ret.addKey (PROPERTYTYPES_KEYS[i],
                        cp.getValue ());
         }
         else
         {
            if (DEBUG)
            {
               System.err.println ("JMPIPropertyProvider::validateInstance: Bad3: " + PROPERTYTYPES_KEYS[i]);
            }

            return null;
         }
      }

      return ret;
   }

   private int findObjectPath (CIMObjectPath path)
   {
       HashMap hashPaths = new HashMap ();
       String  p         = path.toString ();

       if (DEBUG)
       {
           System.out.println ("JMPIPropertyProvider::findObjectPath: comparing \"" + p + "\"");
       }

       // Iterate through each ObjectPath in our ObjectPaths
       for (int i = 0; i < paths.size (); i++)
       {
           HashMap       hashCop  = new HashMap ();
           CIMObjectPath cop      = (CIMObjectPath)paths.elementAt (i);
           Vector        keysPath = path.getKeys ();

           if (DEBUG)
           {
               System.out.println ("JMPIPropertyProvider::findObjectPath: to \"" + cop + "\"");
           }

           // Iterate though each property in the ObjectPath
           for (int idxPath = 0; idxPath < keysPath.size (); idxPath++)
           {
               CIMProperty cp          = (CIMProperty)keysPath.elementAt (idxPath);
               String      cpKeyValue  = cp.getValue ().toString ();
               String      copKeyValue = cop.getKeyValue (cp.getName ());

               if (DEBUG)
               {
                   System.out.println ("JMPIPropertyProvider::findObjectPath: cpKeyValue  \"" + cpKeyValue + "\"");
                   System.out.println ("JMPIPropertyProvider::findObjectPath: copKeyValue \"" + copKeyValue + "\"");
               }

               // Compare the property values and save it for later
               hashCop.put (cp.toString (), new Boolean (cpKeyValue.equals (copKeyValue)));
           }

           // Save the result of all of the property comparisons
           hashPaths.put (cop, hashCop);
       }

       if (DEBUG)
       {
           System.out.println ("JMPIPropertyProvider::findObjectPath: hashPaths = " + hashPaths);
       }

       Iterator itrHashPaths = hashPaths.keySet ().iterator ();

       // Iterate through all of our results
       while (itrHashPaths.hasNext ())
       {
           Object   key     = itrHashPaths.next ();
           HashMap  hash    = (HashMap)hashPaths.get (key);
           boolean  found   = true;
           Iterator itrHash = hash.values ().iterator ();

           while (itrHash.hasNext ())
           {
               if (!((Boolean)itrHash.next ()).booleanValue ())
               {
                   found = false;
               }
           }

           if (DEBUG)
           {
               System.out.println ("JMPIPropertyProvider::findObjectPath: found = " + found + ", key = " + key);
           }

           // Were they all equal?
           if (found)
           {
               // Return the index of it
               return paths.indexOf (key);
           }
       }

       return -1;
   }

   private void testPropertyTypesValue (CIMInstance instanceObject)
      throws CIMException
   {
      Vector properties     = instanceObject.getProperties ();
      int    iPropertyCount = properties.size ();

      for (int j = 0; j < iPropertyCount; j++)
      {
         CIMProperty property      = (CIMProperty)properties.elementAt (j);
         String      propertyName  = property.getName ();
         CIMValue    propertyValue = property.getValue ();
         Object      value         = propertyValue.getValue ();
         int         type          = property.getType ().getType ();

         switch (type)
         {
         case CIMDataType.UINT64:
            if (!(value instanceof UnsignedInt64))
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            if (((UnsignedInt64)value).longValue () >= 1000000000)
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            break;
         default:
            break;
         }
      }
   }

   private static final String NAMESPACE               = "root/SampleProvider";
   private static final String NAMESPACE_INTEROP       = "root/PG_InterOp";

   // Class names
   private static final String CLASS_PROPERTYTYPES     = "JMPIProperty_TestPropertyTypes";

   private final String[] PROPERTYTYPES_KEYS = {
      "CreationClassName",
      "InstanceId"
   };
   private final String[] PROPERTYTYPES_PROPS = {
      "CreationClassName",
      "InstanceId",
      "PropertyString",
   };
}
