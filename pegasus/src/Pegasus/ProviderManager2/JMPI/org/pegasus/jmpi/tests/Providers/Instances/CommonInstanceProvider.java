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
// Modified By: Mark Hamzy, hamzy@us.ibm.com
//
//
// This code is based on TestPropertyTypes.cpp
//
// Original Author: Yi Zhou (yi_zhou@hp.com)
//
// Original Modified By: Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//                       Sushma Fernandes, Hewlett-Packard Company
//                         (sushma_fernandes@hp.com)
//                       Carol Ann Krug Graves, Hewlett-Packard Company
//                        (carolann_graves@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////
package Instances;

import java.util.Vector;
import java.util.HashMap;
import java.util.Iterator;
import org.pegasus.jmpi.CIMArgument;
import org.pegasus.jmpi.CIMClass;
import org.pegasus.jmpi.CIMDataType;
import org.pegasus.jmpi.CIMDateTime;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMInstance;
import org.pegasus.jmpi.CIMOMHandle;
import org.pegasus.jmpi.CIMObjectPath;
import org.pegasus.jmpi.CIMProperty;
import org.pegasus.jmpi.CIMQualifier;
import org.pegasus.jmpi.CIMValue;
import org.pegasus.jmpi.InstanceProvider;
import org.pegasus.jmpi.MethodProvider;
import org.pegasus.jmpi.OperationContext;
import org.pegasus.jmpi.SelectExp;
import org.pegasus.jmpi.SelectList;
import org.pegasus.jmpi.NonJoinExp;
import org.pegasus.jmpi.QueryExp;
import org.pegasus.jmpi.UnsignedInt16;
import org.pegasus.jmpi.UnsignedInt32;
import org.pegasus.jmpi.UnsignedInt64;
import org.pegasus.jmpi.UnsignedInt8;

public class CommonInstanceProvider
{
   protected CommonInstanceProvider (String providerName,
                                     String namespace,
                                     String testElementClassname,
                                     String propertyTypesClassname)
   {
      this.providerName           = providerName;
      this.namespace              = namespace;
      this.testElementClassname   = testElementClassname;
      this.propertyTypesClassname = propertyTypesClassname;
   }

   public void initialize (CIMOMHandle handle)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println (providerName + "::initialize: handle = " + handle);
      }

      this.handle = handle;

      CIMInstance   instance1 = new CIMInstance (propertyTypesClassname);
      CIMObjectPath cop1      = new CIMObjectPath (propertyTypesClassname,
                                                   namespace);

      instance1.setProperty ("CreationClassName",
                             new CIMValue (new String (propertyTypesClassname)));
      cop1.addKey ("CreationClassName",
                   new CIMValue (new String (propertyTypesClassname)));
      instance1.setProperty ("InstanceId",
                             new CIMValue (new UnsignedInt64 ("1")));
      cop1.addKey ("InstanceId",
                   new CIMValue (new UnsignedInt64 ("1")));
      instance1.setProperty ("PropertyString",
                             new CIMValue (new String (providerName + "_Instance1")));
      instance1.setProperty ("PropertyUint8",
                             new CIMValue (new UnsignedInt8 ((short)120)));
      instance1.setProperty ("PropertyUint16",
                             new CIMValue (new UnsignedInt16 ((int)1600)));
      instance1.setProperty ("PropertyUint32",
                             new CIMValue (new UnsignedInt32 ((long)3200)));
      instance1.setProperty ("PropertyUint64",
                             new CIMValue (new UnsignedInt64 ("6400")));
      instance1.setProperty ("PropertySint8",
                             new CIMValue (new Byte ((byte)-119)));
      instance1.setProperty ("PropertySint16",
                             new CIMValue (new Short ((short)-1600)));
      instance1.setProperty ("PropertySint32",
                             new CIMValue (new Integer (-3200)));
      instance1.setProperty ("PropertySint64",
                             new CIMValue (new Long (-6400)));
      instance1.setProperty ("PropertyBoolean",
                             new CIMValue (new Boolean (true)));
      instance1.setProperty ("PropertyReal32",
                             new CIMValue (new Float (1.12345670123)));
      instance1.setProperty ("PropertyReal64",
                             new CIMValue (new Double (1.12345678906543210123)));
      instance1.setProperty ("PropertyDatetime",
                             new CIMValue (new CIMDateTime ("20010515104354.000000:000")));

      instancesPropertyTypes.addElement (instance1);
      pathsPropertyTypes.addElement (cop1);

      CIMInstance   instance2 = new CIMInstance (propertyTypesClassname);
      CIMObjectPath cop2      = new CIMObjectPath (propertyTypesClassname,
                                                   namespace);

      instance2.setProperty ("CreationClassName",
                             new CIMValue (new String (propertyTypesClassname)));
      cop2.addKey ("CreationClassName",
                   new CIMValue (new String (propertyTypesClassname)));
      instance2.setProperty ("InstanceId",
                             new CIMValue (new UnsignedInt64 ("2")));
      cop2.addKey ("InstanceId",
                   new CIMValue (new UnsignedInt64 ("2")));
      instance2.setProperty ("PropertyString",
                             new CIMValue (new String (providerName + "_Instance2")));
      instance2.setProperty ("PropertyUint8",
                             new CIMValue (new UnsignedInt8 ((short)122)));
      instance2.setProperty ("PropertyUint16",
                             new CIMValue (new UnsignedInt16 ((int)1602)));
      instance2.setProperty ("PropertyUint32",
                             new CIMValue (new UnsignedInt32 ((long)3202)));
      instance2.setProperty ("PropertyUint64",
                             new CIMValue (new UnsignedInt64 ("6402")));
      instance2.setProperty ("PropertySint8",
                             new CIMValue (new Byte ((byte)-117)));
      instance2.setProperty ("PropertySint16",
                             new CIMValue (new Short ((short)-1602)));
      instance2.setProperty ("PropertySint32",
                             new CIMValue (new Integer (-3202)));
      instance2.setProperty ("PropertySint64",
                             new CIMValue (new Long (-6402)));
      instance2.setProperty ("PropertyBoolean",
                             new CIMValue (new Boolean (false)));
      instance2.setProperty ("PropertyReal32",
                             new CIMValue (new Float (2.12345670123)));
      instance2.setProperty ("PropertyReal64",
                             new CIMValue (new Double (2.12345678906543210123)));
      instance2.setProperty ("PropertyDatetime",
                             new CIMValue (new CIMDateTime ("20010515104354.000000:000")));

      instancesPropertyTypes.addElement (instance2);
      pathsPropertyTypes.addElement (cop2);

      pathsPropertyTypes.sort ();
      instancesPropertyTypes.sort ();
   }

   public void cleanup ()
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println (providerName + "::cleanup");
      }
   }

   public CIMObjectPath createInstance (OperationContext oc,
                                        CIMObjectPath    cop,
                                        CIMInstance      cimInstance)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println (providerName + "::createInstance: oc          = " + oc);
         System.err.println (providerName + "::createInstance: cop         = " + cop);
         System.err.println (providerName + "::createInstance: cimInstance = " + cimInstance);
      }

      // Ensure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

//////if (DEBUG)
//////{
//////   System.err.println (providerName + "::createInstance: cop2        = " + cimInstance.getObjectPath ());
//////   Vector        vectorKeyValuePairs = cimInstance.getKeyValuePairs ();
//////   CIMObjectPath cop3                = new CIMObjectPath (cimInstance.getClassName (),
//////                                                          vectorKeyValuePairs);
//////   System.err.println (providerName + "::createInstance: cop3        = " + cop3);
//////   System.err.println (providerName + "::createInstance: vectorKeyValuePairs = " + vectorKeyValuePairs);
//////   CIMClass cc           = handle.getClass (cop, true);
//////   Vector   ccQualifiers = cc.getQualifiers ();
//////   System.err.print (providerName + "::createInstance: cc.getQualifiers    = ");
//////   for (int i = 0; i < ccQualifiers.size (); i++)
//////   {
//////      System.err.print (((CIMQualifier)ccQualifiers.elementAt (i)).getName () + ", " + ((CIMQualifier)ccQualifiers.elementAt (i)).getValue ());
//////      if (i < ccQualifiers.size () - 1)
//////      {
//////         System.err.print (", ");
//////      }
//////   }
//////   System.err.println ("}");
//////
//////   CIMObjectPath cop4 = validatePropertyTypeInstance (cimInstance, false);
//////   System.err.println (providerName + "::createInstance: cop4        = " + cop4);
//////}

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (testElementClassname))
      {
         // Ensure that the instance contains the required keys
         cop = validateTestElementInstance (cimInstance, false);

         if (cop == null)
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         // Ensure that the property values are valid
         testTestElementInstance (cimInstance);

         // Determine if a property exists in the class
         if (cimInstance.getProperty ("PropertyUint8") == null)
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         // Ensure that the requested object does not exist
         if (findCOPInTestElements (cop) >= 0)
         {
            throw new CIMException (CIMException.CIM_ERR_ALREADY_EXISTS);
         }

         if (fEnableModifications)
         {
            pathsTestElements.addElement (cop);
            instancesTestElements.addElement (cimInstance);

            pathsTestElements.sort ();
            instancesTestElements.sort ();

            return cop;
         }
         else
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }
      }
      else if (cop.getObjectName ().equalsIgnoreCase (propertyTypesClassname))
      {
         // Ensure that the instance contains the required keys
         cop = validatePropertyTypeInstance (cimInstance, false);

         if (cop == null)
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         // Ensure that the property values are valid
         testPropertyTypeInstance (cimInstance);

         // Determine if a property exists in the class
         if (cimInstance.getProperty ("PropertyUint8") == null)
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         // Ensure that the requested object does not exist
         if (findCOPInPropertyTypesNew (cop) >= 0)
         {
            throw new CIMException (CIMException.CIM_ERR_ALREADY_EXISTS);
         }

         if (fEnableModifications)
         {
            pathsPropertyTypes.addElement (cop);
            instancesPropertyTypes.addElement (cimInstance);

            pathsPropertyTypes.sort ();
            instancesPropertyTypes.sort ();

            return cop;
         }
         else
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }
      }
      else
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);
      }
   }

   public CIMInstance getInstance (OperationContext oc,
                                   CIMObjectPath    cop,
                                   CIMClass         cimClass,
                                   boolean          includeQualifiers,
                                   boolean          includeClassOrigin,
                                   boolean          localOnly,
                                   String           propertyList[])
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println (providerName + "::getInstance: oc                 = " + oc);
         System.err.println (providerName + "::getInstance: cop                = " + cop);
         System.err.println (providerName + "::getInstance: cimClass           = " + cimClass);
         System.err.println (providerName + "::getInstance: includeQualifiers  = " + includeQualifiers);
         System.err.println (providerName + "::getInstance: includeClassOrigin = " + includeClassOrigin);
         System.err.println (providerName + "::getInstance: localOnly          = " + localOnly);
         System.err.println (providerName + "::getInstance: propertyList       = " + propertyList);
      }

      // Ensure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (testElementClassname))
      {
         // Ensure that the keys are valid
         if (!validateTestElementPath (cop))
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         // Ensure that the request object exists
         int index = findCOPInTestElements (cop);

         if (index < 0)
         {
            throw new CIMException (CIMException.CIM_ERR_NOT_FOUND);
         }

         return (CIMInstance)instancesTestElements.elementAt (index);
      }
      else if (cop.getObjectName ().equalsIgnoreCase (propertyTypesClassname))
      {
         // Ensure that the keys are valid
         if (!validatePropertyTypePath (cop))
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         // Ensure that the request object exists
         int index = findCOPInPropertyTypesNew (cop);

         if (index < 0)
         {
            throw new CIMException (CIMException.CIM_ERR_NOT_FOUND);
         }

         return (CIMInstance)instancesPropertyTypes.elementAt (index);
      }
      else
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);
      }
   }

   public void setInstance (OperationContext oc,
                            CIMObjectPath    cop,
                            CIMInstance      cimInstance,
                            boolean          includeQualifiers,
                            String[]         propertyList)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println (providerName + "::setInstance: oc                = " + oc);
         System.err.println (providerName + "::setInstance: cop               = " + cop);
         System.err.println (providerName + "::setInstance: cimInstance       = " + cimInstance);
         System.err.println (providerName + "::setInstance: includeQualifiers = " + includeQualifiers);
         System.err.println (providerName + "::setInstance: propertyList      = " + propertyList);
      }

      // Ensure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (testElementClassname))
      {
         // Ensure that the property values are valid
         testTestElementInstance (cimInstance);

         // Ensure that the keys are valid
         if (!validateTestElementPath (cop))
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         // Ensure that the request object exists
         int index = findCOPInTestElements (cop);

         if (index < 0)
         {
            throw new CIMException (CIMException.CIM_ERR_NOT_FOUND);
         }

         if (fEnableModifications)
         {
            pathsTestElements.removeElementAt (index);
            instancesTestElements.removeElementAt (index);

            pathsTestElements.addElement (cop);
            instancesTestElements.addElement (cimInstance);

            pathsTestElements.sort ();
            instancesTestElements.sort ();
         }
         else
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }
      }
      else if (cop.getObjectName ().equalsIgnoreCase (propertyTypesClassname))
      {
         // Ensure that the property values are valid
         testPropertyTypeInstance (cimInstance);

         // Ensure that the keys are valid
         if (!validatePropertyTypePath (cop))
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         // Ensure that the request object exists
         int index = findCOPInPropertyTypesNew (cop);

         if (index < 0)
         {
            throw new CIMException (CIMException.CIM_ERR_NOT_FOUND);
         }

         if (fEnableModifications)
         {
            pathsPropertyTypes.removeElementAt (index);
            instancesPropertyTypes.removeElementAt (index);

            pathsPropertyTypes.addElement (cop);
            instancesPropertyTypes.addElement (cimInstance);

            pathsPropertyTypes.sort ();
            instancesPropertyTypes.sort ();
         }
         else
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }
      }
      else
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }
   }

   public void deleteInstance (OperationContext oc,
                               CIMObjectPath    cop)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println (providerName + "::deleteInstance: oc  = " + oc);
         System.err.println (providerName + "::deleteInstance: cop = " + cop);
      }

      // Ensure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (testElementClassname))
      {
         // Ensure that the keys are valid
         if (!validateTestElementPath (cop))
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         // Ensure that the request object exists
         int index = findCOPInTestElements (cop);

         if (index < 0)
         {
            throw new CIMException (CIMException.CIM_ERR_NOT_FOUND);
         }

         if (fEnableModifications)
         {
            pathsTestElements.removeElementAt (index);
            instancesTestElements.removeElementAt (index);

            pathsTestElements.sort ();
            instancesTestElements.sort ();
         }
         else
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }
      }
      else if (cop.getObjectName ().equalsIgnoreCase (propertyTypesClassname))
      {
         // Ensure that the keys are valid
         if (!validatePropertyTypePath (cop))
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         // Ensure that the request object exists
         int index = findCOPInPropertyTypesNew (cop);

         if (index < 0)
         {
            throw new CIMException (CIMException.CIM_ERR_NOT_FOUND);
         }

         if (fEnableModifications)
         {
            pathsPropertyTypes.removeElementAt (index);
            instancesPropertyTypes.removeElementAt (index);

            pathsPropertyTypes.sort ();
            instancesPropertyTypes.sort ();
         }
         else
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }
      }
      else
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);
      }
   }

   public Vector enumerateInstanceNames (OperationContext oc,
                                         CIMObjectPath    cop,
                                         boolean          deepInheritance,
                                         CIMClass         cimClass)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println (providerName + "::enumerateInstanceNames: oc              = " + oc);
         System.err.println (providerName + "::enumerateInstanceNames: cop             = " + cop);
         System.err.println (providerName + "::enumerateInstanceNames: deepInheritance = " + deepInheritance);
         System.err.println (providerName + "::enumerateInstanceNames: cimClass        = " + cimClass);
      }

      // Ensure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (testElementClassname))
      {
         return pathsTestElements;
      }
      else if (cop.getObjectName ().equalsIgnoreCase (propertyTypesClassname))
      {
         return pathsPropertyTypes;
      }
      else
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);
      }
   }

   public Vector enumerateInstances (OperationContext oc,
                                     CIMObjectPath    cop,
                                     CIMClass         cimClass,
                                     boolean          deepInheritance,
                                     boolean          localOnly,
                                     boolean          includeQualifiers,
                                     boolean          includeClassOrigin,
                                     String           propertyList[])
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println (providerName + "::enumerateInstances: oc                 = " + oc);
         System.err.println (providerName + "::enumerateInstances: cop                = " + cop);
         System.err.println (providerName + "::enumerateInstances: cimClass           = " + cimClass);
         System.err.println (providerName + "::enumerateInstances: deepInheritance    = " + deepInheritance);
         System.err.println (providerName + "::enumerateInstances: localOnly          = " + localOnly);
         System.err.println (providerName + "::enumerateInstances: includeQualifiers  = " + includeQualifiers);
         System.err.println (providerName + "::enumerateInstances: includeClassOrigin = " + includeClassOrigin);
         System.err.println (providerName + "::enumerateInstances: propertyList       = " + propertyList);
      }

      // Ensure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (testElementClassname))
      {
         return instancesTestElements;
      }
      else if (cop.getObjectName ().equalsIgnoreCase (propertyTypesClassname))
      {
         return instancesPropertyTypes;
      }
      else
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);
      }
   }

   public Vector execQuery (OperationContext oc,
                            CIMObjectPath    cop,
                            String           queryStatement,
                            String           queryLanguage,
                            CIMClass         cimClass)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println (providerName + "::execQuery: oc              = " + oc);
         System.err.println (providerName + "::execQuery: cop             = " + cop);
         System.err.println (providerName + "::execQuery: queryStatement  = " + queryStatement);
         System.err.println (providerName + "::execQuery: queryLanguage   = " + queryLanguage);
         System.err.println (providerName + "::execQuery: cimClass        = " + cimClass);
      }

      // Ensure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (testElementClassname))
      {
         throw new CIMException (CIMException.CIM_ERR_NOT_SUPPORTED);
      }
      else if (cop.getObjectName ().equalsIgnoreCase (propertyTypesClassname))
      {
         if (!queryLanguage.equals ("WQL"))
         {
            throw new CIMException (CIMException.CIM_ERR_NOT_SUPPORTED);
         }

         SelectExp     q         = new SelectExp (queryStatement);
         SelectList    attrs     = q.getSelectList ();
         NonJoinExp    from      = (NonJoinExp)q.getFromClause ();
         QueryExp      where     = q.getWhereClause ();
         Vector        instances = enumerateInstances (oc,
                                                       cop,
                                                       cimClass,
                                                       false,    // deepInheritance,
                                                       true,     // localOnly,
                                                       true,     // includeQualifiers,
                                                       true,     // includeClassOrigin,
                                                       null);
         Vector        ret       = new Vector ();

         if (DEBUG)
         {
            System.err.println (providerName + "::execQuery: q                 = " + q);
            System.err.println (providerName + "::execQuery: attrs             = " + attrs);
            System.err.println (providerName + "::execQuery: from              = " + from);
            System.err.println (providerName + "::execQuery: where             = " + where);
            System.err.println (providerName + "::execQuery: instances.size () = " + instances.size ());
         }

         // filter the instances
         for (int i = 0; i < instances.size (); i++)
         {
            if (  where == null
               || where.apply ((CIMInstance)instances.elementAt (i))
               )
            {
               if (DEBUG)
               {
                  System.err.println (providerName + "::execQuery: adding instance " + instances.elementAt (i));
               }

               ret.addElement (attrs.apply ((CIMInstance)instances.elementAt (i)));
            }
         }

         return ret;
      }
      else
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);
      }
   }

   public CIMValue invokeMethod (OperationContext oc,
                                 CIMObjectPath    cop,
                                 String           methodName,
                                 Vector           inArgs,
                                 Vector           outArgs)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println (providerName + "::invokeMethod: oc         = " + oc);
         System.err.println (providerName + "::invokeMethod: cop        = " + cop);
         System.err.println (providerName + "::invokeMethod: methodName = " + methodName);
         System.err.print (providerName + "::invokeMethod: inArgs     = ");
         if (inArgs != null)
         {
            System.err.print ("{");
            for (int i = 0; i < inArgs.size (); i++)
            {
               System.err.print (inArgs.elementAt (i));
               if (i < inArgs.size () - 1)
               {
                  System.err.print (",");
               }
            }
            System.err.println ("}");
         }
         else
         {
            System.err.println ("null");
         }
         System.err.print (providerName + "::invokeMethod: outArgs    = ");
         if (outArgs != null)
         {
            System.err.print ("{");
            for (int i = 0; i < outArgs.size (); i++)
            {
               System.err.print (outArgs.elementAt (i));
               if (i < outArgs.size () - 1)
               {
                  System.err.print (",");
               }
            }
            System.err.println ("}");
         }
         else
         {
            System.err.println ("null");
         }
      }

      // Ensure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (!cop.getObjectName ().equalsIgnoreCase (propertyTypesClassname))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);
      }

      if (methodName.equalsIgnoreCase ("sayHello"))
      {
         return new CIMValue (new String ("hello"));
      }
      else if (methodName.equalsIgnoreCase ("enableModifications"))
      {
         if (inArgs == null)
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }
         else if (inArgs.size () != 1)
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }
         else
         {
            String      name  = null;
            CIMValue    value = null;
            CIMDataType type  = null;
            Object      o0    = inArgs.elementAt (0);

            if (DEBUG)
            {
               System.err.println (providerName + "::invokeMethod: o0 class   = " + o0.getClass ());
            }
            if (o0 instanceof CIMArgument)
            {
               CIMArgument ca = (CIMArgument)inArgs.elementAt (0);

               name  = ca.getName ();
               value = ca.getValue ();
               type  = ca.getType ();
            }
            else if (o0 instanceof CIMProperty)
            {
               CIMProperty cp = (CIMProperty)inArgs.elementAt (0);

               name  = cp.getName ();
               value = cp.getValue ();
               type  = cp.getType ();
            }

            if (!name.equalsIgnoreCase ("fState"))
            {
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            }
            else if (type.getType () != CIMDataType.BOOLEAN)
            {
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            }

            Boolean fNewState = (Boolean)value.getValue ();

            if (DEBUG)
            {
               System.err.println (providerName + "::invokeMethod: Setting fEnableModifications to " + fNewState);
            }

            fEnableModifications = fNewState.booleanValue ();

            return new CIMValue (new Boolean (true));
         }
      }
      else if (methodName.equalsIgnoreCase ("resetProvider"))
      {
         if (  inArgs == null
            || inArgs.size () == 0
            )
         {
            if (DEBUG)
            {
               System.err.println (providerName + "::invokeMethod: resetting the provider's state!");
            }

            pathsTestElements      = new SortableVector (new CIMObjectPathComparer ());
            instancesTestElements  = new SortableVector (new CIMInstanceComparer ());
            pathsPropertyTypes     = new SortableVector (new CIMObjectPathComparer ());
            instancesPropertyTypes = new SortableVector (new CIMInstanceComparer ());
            fEnableModifications   = false;

            initialize (handle);

            return new CIMValue (new Boolean (true));
         }
         else
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }
      }
      else
      {
         throw new CIMException (CIMException.CIM_ERR_METHOD_NOT_AVAILABLE);
      }
   }

   private boolean validateTestElementPath (CIMObjectPath cop)
   {
      return false;
   }

   private boolean validatePropertyTypePath (CIMObjectPath cop)
   {
      Vector keys = cop.getKeys ();

      if (keys.size () < PROPERTYTYPES_KEYS.length)
      {
         if (DEBUG)
         {
            System.err.println (providerName + "::validatePropertyTypePath: Bad1: " + keys.size () + " != " + PROPERTYTYPES_KEYS.length);
         }

         return false;
      }
      for (int i = 0, is = PROPERTYTYPES_KEYS.length; i < is; i++)
      {
         boolean fFound = false;

         for (int j = 0, js = keys.size (); j < js; j++)
         {
            CIMProperty cp = (CIMProperty)keys.elementAt (j);

            if (PROPERTYTYPES_KEYS[i].equalsIgnoreCase (cp.getName ()))
            {
               fFound = true;
            }
         }

         if (!fFound)
         {
            if (DEBUG)
            {
               System.err.println (providerName + "::validatePropertyTypePath: Bad2: " + PROPERTYTYPES_KEYS[i] + " not in " + keys);
            }

            return false;
         }
      }

      return true;
   }

   private CIMObjectPath validateTestElementInstance (CIMInstance cimInstance,
                                                      boolean     fStrict)
   {
      if (cimInstance.getPropertyCount () != TESTELEMENT_PROPS.length)
      {
         if (DEBUG)
         {
            System.err.println (providerName + "::validateTestElementInstance: Bad1: " + cimInstance.getPropertyCount () + " != " + TESTELEMENT_PROPS.length);
         }

         if (fStrict)
         {
            return null;
         }
      }
      for (int i = 0, is = TESTELEMENT_PROPS.length; i < is; i++)
      {
         CIMProperty cp = cimInstance.getProperty (TESTELEMENT_PROPS[i]);
         if (cp == null)
         {
            if (DEBUG)
            {
               System.err.println (providerName + "::validateTestElementInstance: Bad2: " + TESTELEMENT_PROPS[i]);
            }

            if (fStrict)
            {
               return null;
            }
         }
      }

      CIMObjectPath ret = new CIMObjectPath (propertyTypesClassname,
                                             namespace);

      for (int i = 0, is = TESTELEMENT_KEYS.length; i < is; i++)
      {
         CIMProperty cp = cimInstance.getProperty (TESTELEMENT_KEYS[i]);

         if (cp != null)
         {
            ret.addKey (TESTELEMENT_KEYS[i],
                        cp.getValue ());
         }
         else
         {
            if (DEBUG)
            {
               System.err.println (providerName + "::validateTestElementInstance: Bad3: " + TESTELEMENT_KEYS[i]);
            }

            return null;
         }
      }

      return ret;
   }

   private CIMObjectPath validatePropertyTypeInstance (CIMInstance cimInstance,
                                                       boolean     fStrict)
   {
      if (cimInstance.getPropertyCount () != PROPERTYTYPES_PROPS.length)
      {
         if (DEBUG)
         {
            System.err.println (providerName + "::validatePropertyTypeInstance: Bad1: " + cimInstance.getPropertyCount () + " != " + PROPERTYTYPES_PROPS.length);
         }

         if (fStrict)
         {
            return null;
         }
      }
      for (int i = 0, is = PROPERTYTYPES_PROPS.length; i < is; i++)
      {
         CIMProperty cp = cimInstance.getProperty (PROPERTYTYPES_PROPS[i]);
         if (cp == null)
         {
            if (DEBUG)
            {
               System.err.println (providerName + "::validatePropertyTypeInstance: Bad2: " + PROPERTYTYPES_PROPS[i]);
            }

            if (fStrict)
            {
               return null;
            }
         }
      }

      CIMObjectPath ret = new CIMObjectPath (propertyTypesClassname,
                                             namespace);

      for (int i = 0, is = PROPERTYTYPES_KEYS.length; i < is; i++)
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
               System.err.println (providerName + "::validatePropertyTypeInstance: Bad3: " + PROPERTYTYPES_KEYS[i]);
            }

            return null;
         }
      }

      return ret;
   }

   private int findCOPInTestElements (CIMObjectPath cop1)
   {
      String path1 = cop1.toString ();

      for (int i = 0; i < pathsTestElements.size (); i++)
      {
         CIMObjectPath cop2  = (CIMObjectPath)pathsTestElements.elementAt (i);
         String        path2 = cop2.toString ();

         if (DEBUG)
         {
            System.err.println (providerName + "::findCOPInTestElements: \"" + path1 + "\" == \"" + path2 + "\"");
         }

         if (path2.equalsIgnoreCase (path1))
         {
            if (DEBUG)
            {
               System.err.println (providerName + "::findCOPInTestElements: found!");
            }

            return i;
         }
      }

      return -1;
   }

   private int findCOPInPropertyTypes (CIMObjectPath cop1)
   {
      String path1 = cop1.toString ();

      for (int i = 0; i < pathsPropertyTypes.size (); i++)
      {
         CIMObjectPath cop2  = (CIMObjectPath)pathsPropertyTypes.elementAt (i);
         String        path2 = cop2.toString ();

         if (DEBUG)
         {
            System.err.println (providerName + "::findCOPInPropertyTypes: \"" + path1 + "\" == \"" + path2 + "\"");
         }

         if (path2.equalsIgnoreCase (path1))
         {
            if (DEBUG)
            {
               System.err.println (providerName + "::findCOPInPropertyTypes: found!");
            }

            return i;
         }
      }

      return -1;
   }

   private int findCOPInPropertyTypesNew (CIMObjectPath cop1)
   {
       HashMap hashPaths = new HashMap ();

       if (DEBUG)
       {
           System.out.println (providerName + "::findCOPInPropertyTypesNew: comparing \"" + cop1 + "\"");
       }

       // Iterate through each ObjectPath in our ObjectPaths
       for (int i = 0; i < pathsPropertyTypes.size (); i++)
       {
           CIMObjectPath cop2     = (CIMObjectPath)pathsPropertyTypes.elementAt (i);
           Vector        keysCop1 = cop1.getKeys ();
           HashMap       hashCop1 = new HashMap ();

           if (DEBUG)
           {
               System.out.println (providerName + "::findCOPInPropertyTypesNew: to \"" + cop2 + "\"");
           }

           // Iterate though each property in the ObjectPath
           for (int idxPath = 0; idxPath < keysCop1.size (); idxPath++)
           {
               CIMProperty cp          = (CIMProperty)keysCop1.elementAt (idxPath);
               String      cpKeyValue  = cp.getValue ().toString ();
               String      copKeyValue = cop2.getKeyValue (cp.getName ());

               if (DEBUG)
               {
                   System.out.println (providerName + "::findCOPInPropertyTypesNew: cpKeyValue  \"" + cpKeyValue + "\"");
                   System.out.println (providerName + "::findCOPInPropertyTypesNew: copKeyValue \"" + copKeyValue + "\"");
               }

               if (copKeyValue != null)
               {
                  // Compare the property values and save it for later
                  hashCop1.put (cp.toString (), new Boolean (cpKeyValue.equals (copKeyValue)));
               }
           }

           // Save the result of all of the property comparisons
           hashPaths.put (cop2, hashCop1);
       }

       if (DEBUG)
       {
           System.out.println (providerName + "::findCOPInPropertyTypesNew: hashPaths = " + hashPaths);
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
               System.out.println (providerName + "::findCOPInPropertyTypesNew: found = " + found + ", key = " + key);
           }

           // Were they all equal?
           if (found)
           {
               // Return the index of it
               return pathsPropertyTypes.indexOf (key);
           }
       }

       return -1;
   }

   private void testTestElementInstance (CIMInstance instanceObject)
      throws CIMException
   {
   }

   private void testPropertyTypeInstance (CIMInstance instanceObject)
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
         case CIMDataType.UINT8:
         {
            if (!(value instanceof UnsignedInt8))
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            if (((UnsignedInt8)value).intValue () >= 255)
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            break;
         }
         case CIMDataType.UINT16:
         {
            if (!(value instanceof UnsignedInt16))
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            if (((UnsignedInt16)value).intValue () >= 10000)
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            break;
         }
         case CIMDataType.UINT32:
         {
            if (!(value instanceof UnsignedInt32))
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            if (((UnsignedInt32)value).intValue () >= 10000000)
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            break;
         }
         case CIMDataType.UINT64:
         {
            if (!(value instanceof UnsignedInt64))
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            if (((UnsignedInt64)value).longValue () >= 1000000000)
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            break;
         }
         case CIMDataType.SINT8:
         {
            if (!(value instanceof Byte))
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            if (((Byte)value).intValue () <= -120)
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            break;
         }
         case CIMDataType.SINT16:
         {
            if (!(value instanceof Short))
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            if (((Short)value).intValue () < -10000)
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            break;
         }
         case CIMDataType.SINT32:
         {
            if (!(value instanceof Integer))
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            if (((Integer)value).intValue () <= -10000000)
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            break;
         }
         case CIMDataType.SINT64:
         {
            if (!(value instanceof Long))
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            if (((Long)value).intValue () <= -1000000000)
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            break;
         }
         case CIMDataType.REAL32:
         {
            if (!(value instanceof Float))
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            if (((Float)value).floatValue () >= 10000000.32)
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            break;
         }
         case CIMDataType.REAL64:
         {
            if (!(value instanceof Double))
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            if (((Double)value).doubleValue () >= 1000000000.64)
               throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
            break;
         }
         default:
         {
            break;
         }
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

   //
   private String              providerName            = "";
   private String              testElementClassname    = "";
   private String              propertyTypesClassname  = "";

   //
   private String              namespace               = "";
   private CIMOMHandle         handle                  = null;
   private SortableVector      pathsTestElements       = new SortableVector (new CIMObjectPathComparer ());
   private SortableVector      instancesTestElements   = new SortableVector (new CIMInstanceComparer ());
   private SortableVector      pathsPropertyTypes      = new SortableVector (new CIMObjectPathComparer ());
   private SortableVector      instancesPropertyTypes  = new SortableVector (new CIMInstanceComparer ());
   private boolean             fEnableModifications    = true;
   private final boolean       DEBUG                   = true;

   private final String[] TESTELEMENT_KEYS = {
      "CreationClassName",
      "InstanceId"
   };
   private final String[] TESTELEMENT_PROPS = {
      "CreationClassName",
      "InstanceId"
   };
   private final String[] PROPERTYTYPES_KEYS = {
      "CreationClassName",
      "InstanceId"
   };
   private final String[] PROPERTYTYPES_PROPS = {
      "CreationClassName",
      "InstanceId",
      "PropertyString",
      "PropertyUint8",
      "PropertyUint16",
      "PropertyUint32",
      "PropertyUint64",
      "PropertySint8",
      "PropertySint16",
      "PropertySint32",
      "PropertySint64",
      "PropertyBoolean",
      "PropertyReal32",
      "PropertyReal64",
      "PropertyDatetime",
      "PropertyChar16",
      "PropertyObject",
      "PropertyReference",
      "PropertyArrayUint8",
      "PropertyArrayUint16",
      "PropertyArrayUint32",
      "PropertyArrayUint64",
      "PropertyArraySint8",
      "PropertyArraySint16",
      "PropertyArraySint32",
      "PropertyArraySint64",
      "PropertyArrayBoolean",
      "PropertyArrayReal32",
      "PropertyArrayReal64",
      "PropertyArrayDatetime",
      "PropertyArrayChar16",
      "PropertyArrayObject"
   };
}
