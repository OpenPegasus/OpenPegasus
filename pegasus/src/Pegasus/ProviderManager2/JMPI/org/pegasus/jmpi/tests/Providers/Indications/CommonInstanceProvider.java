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
package Indications;

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
                                     String instanceClassname)
   {
      this.providerName      = providerName;
      this.namespace         = namespace;
      this.instanceClassname = instanceClassname;
   }

   public void initialize (CIMOMHandle ch)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println (providerName + "::initialize: ch = " + ch);
      }

      this.ch = ch;
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

      // Esure that the namespace is valid
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
//////   CIMClass cc           = ch.getClass (cop, true);
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
      if (cop.getObjectName ().equalsIgnoreCase (instanceClassname))
      {
         // Ensure that the instance contains the required keys
         cop = validatePropertyTypeInstance (cimInstance, false);

         if (cop == null)
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         // Ensure that the property values are valid
         testPropertyTypeInstance (cimInstance);

         // Ensure that the requested object does not exist
         if (findCOPInPropertyTypes (cop) >= 0)
         {
            throw new CIMException (CIMException.CIM_ERR_ALREADY_EXISTS);
         }

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

      // Esure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (instanceClassname))
      {
         // Ensure that the keys are valid
         if (!validatePropertyTypePath (cop))
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         // Ensure that the request object exists
         int index = findCOPInPropertyTypes (cop);

         if (index < 0)
         {
            throw new CIMException (CIMException.CIM_ERR_NOT_FOUND);
         }

         return (CIMInstance)instances.elementAt (index);
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

      // Esure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (instanceClassname))
      {
         // Ensure that the property values are valid
         testPropertyTypeInstance (cimInstance);

         // Ensure that the keys are valid
         if (!validatePropertyTypePath (cop))
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         // Ensure that the request object exists
         int index = findCOPInPropertyTypes (cop);

         if (index < 0)
         {
            throw new CIMException (CIMException.CIM_ERR_NOT_FOUND);
         }

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

      // Esure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (instanceClassname))
      {
         // Ensure that the keys are valid
         if (!validatePropertyTypePath (cop))
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         // Ensure that the request object exists
         int index = findCOPInPropertyTypes (cop);

         if (index < 0)
         {
            throw new CIMException (CIMException.CIM_ERR_NOT_FOUND);
         }

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

      // Esure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (instanceClassname))
      {
         return paths;
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

      // Esure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (instanceClassname))
      {
         return instances;
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

      // Esure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (instanceClassname))
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

         // filter the instances
         for (int i = 0; i < instances.size (); i++)
         {
            if (  where == null
               || where.apply ((CIMInstance)instances.elementAt (i))
               )
            {
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
                                 Vector           in,
                                 Vector           out)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println (providerName + "::invokeMethod: oc         = " + oc);
         System.err.println (providerName + "::invokeMethod: cop        = " + cop);
         System.err.println (providerName + "::invokeMethod: methodName = " + methodName);
         System.err.print (providerName + "::invokeMethod: in         = ");
         if (in != null)
         {
            System.err.print ("{");
            for (int i = 0; i < in.size (); i++)
            {
               System.err.print (in.elementAt (i));
               if (i < in.size () - 1)
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
         System.err.print (providerName + "::invokeMethod: out        = ");
         if (out != null)
         {
            System.err.print ("{");
            for (int i = 0; i < out.size (); i++)
            {
               System.err.print (out.elementAt (i));
               if (i < out.size () - 1)
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

      // Esure that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (namespace))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (!cop.getObjectName ().equalsIgnoreCase (instanceClassname))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);
      }

      if (methodName.equalsIgnoreCase ("SendTestIndicationNormal"))
      {
         if (in == null)
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }
         else if (in.size () != 1)
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         CIMProperty cp = (CIMProperty)in.elementAt (0);

         if (!cp.getName ().equalsIgnoreCase ("indicationSendCount"))
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }
         else if (cp.getType ().getType () != CIMDataType.UINT64)
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }

         CIMValue      cvIndicationSendCount = cp.getValue ();
         UnsignedInt64 uiIndicationSendCount = (UnsignedInt64)cvIndicationSendCount.getValue ();

         if (DEBUG)
         {
            System.err.println ("JMPIExpIndicationProvider::invokeMethod: uiIndicationSendCount = " + uiIndicationSendCount);
         }

         CIMClass      ccIndication;
         CIMInstance   ciIndication;
         CIMObjectPath copIndication;

         ccIndication = ch.getClass (cop,
                                     true,
                                     true,
                                     true,
                                     null);
         ciIndication = ccIndication.newInstance ();

         ciIndication.setProperty ("InstanceId", new CIMValue (uiIndicationSendCount));
         ciIndication.setProperty ("PropertyString", new CIMValue ("Hello"));

         copIndication = createInstance (oc,
                                         cop,
                                         ciIndication);

         if (DEBUG)
         {
            System.err.println ("JMPIExpIndicationProvider::invokeMethod: deliverEvent: copIndication = " + copIndication);
            System.err.println ("JMPIExpIndicationProvider::invokeMethod: deliverEvent: ciIndication  = " + ciIndication);
         }

         ch.deliverEvent (copIndication.getNameSpace (),
                          ciIndication);

         return new CIMValue (new UnsignedInt32 ("0"));
      }
      else if (methodName.equalsIgnoreCase ("enableModifications"))
      {
         if (in == null)
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }
         else if (in.size () != 1)
         {
            throw new CIMException (CIMException.CIM_ERR_INVALID_PARAMETER);
         }
         else
         {
            String      name  = null;
            CIMValue    value = null;
            CIMDataType type  = null;
            Object      o0    = in.elementAt (0);

            if (DEBUG)
            {
               System.err.println (providerName + "::invokeMethod: o0 class   = " + o0.getClass ());
            }
            if (o0 instanceof CIMArgument)
            {
               CIMArgument ca = (CIMArgument)in.elementAt (0);

               name  = ca.getName ();
               value = ca.getValue ();
               type  = ca.getType ();
            }
            else if (o0 instanceof CIMProperty)
            {
               CIMProperty cp = (CIMProperty)in.elementAt (0);

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
         if (  in == null
            || in.size () == 0
            )
         {
            if (DEBUG)
            {
               System.err.println (providerName + "::invokeMethod: resetting the provider's state!");
            }

            paths                = new SortableVector (new CIMObjectPathComparer ());
            instances            = new SortableVector (new CIMInstanceComparer ());
            fEnableModifications = false;

            initialize (ch);

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

   private boolean validatePropertyTypePath (CIMObjectPath cop)
   {
      Vector keys = cop.getKeys ();

      if (keys.size () != INDICATION_KEYS.length)
      {
         if (DEBUG)
         {
            System.err.println (providerName + "::validatePropertyTypePath: Bad1: " + keys.size () + " != " + INDICATION_KEYS.length);
         }

         return false;
      }
      for (int i = 0, is = INDICATION_KEYS.length; i < is; i++)
      {
         boolean fFound = false;

         for (int j = 0, js = keys.size (); j < js; j++)
         {
            CIMProperty cp = (CIMProperty)keys.elementAt (j);

            if (INDICATION_KEYS[i].equalsIgnoreCase (cp.getName ()))
            {
               fFound = true;
            }
         }

         if (!fFound)
         {
            if (DEBUG)
            {
               System.err.println (providerName + "::validatePropertyTypePath: Bad2: " + INDICATION_KEYS[i] + " not in " + keys);
            }

            return false;
         }
      }

      return true;
   }

   private CIMObjectPath validatePropertyTypeInstance (CIMInstance cimInstance,
                                                       boolean     fStrict)
   {
      if (cimInstance.getPropertyCount () != INDICATION_PROPS.length)
      {
         if (DEBUG)
         {
            System.err.println (providerName + "::validatePropertyTypeInstance: Bad1: " + cimInstance.getPropertyCount () + " != " + INDICATION_PROPS.length);
         }

         if (fStrict)
         {
            return null;
         }
      }
      for (int i = 0, is = INDICATION_PROPS.length; i < is; i++)
      {
         CIMProperty cp = cimInstance.getProperty (INDICATION_PROPS[i]);
         if (cp == null)
         {
            if (DEBUG)
            {
               System.err.println (providerName + "::validatePropertyTypeInstance: Bad2: " + INDICATION_PROPS[i]);
            }

            if (fStrict)
            {
               return null;
            }
         }
      }

      CIMObjectPath ret = new CIMObjectPath (instanceClassname,
                                             namespace);

      for (int i = 0, is = INDICATION_KEYS.length; i < is; i++)
      {
         CIMProperty cp = cimInstance.getProperty (INDICATION_KEYS[i]);

         if (cp != null)
         {
            ret.addKey (INDICATION_KEYS[i],
                        cp.getValue ());
         }
         else
         {
            if (DEBUG)
            {
               System.err.println (providerName + "::validatePropertyTypeInstance: Bad3: " + INDICATION_KEYS[i]);
            }

            return null;
         }
      }

      return ret;
   }

   private int findCOPInPropertyTypes (CIMObjectPath cop1)
   {
      String path1 = cop1.toString ();

      for (int i = 0; i < paths.size (); i++)
      {
         CIMObjectPath cop2  = (CIMObjectPath)paths.elementAt (i);
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
       for (int i = 0; i < paths.size (); i++)
       {
           CIMObjectPath cop2     = (CIMObjectPath)paths.elementAt (i);
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

               // Compare the property values and save it for later
               hashCop1.put (cp.toString (), new Boolean (cpKeyValue.equals (copKeyValue)));
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
               return paths.indexOf (key);
           }
       }

       return -1;
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
   private String              namespace               = "";
   private String              instanceClassname       = "";
   private CIMOMHandle         ch                      = null;
   private SortableVector      paths                   = new SortableVector (new CIMObjectPathComparer ());
   private SortableVector      instances               = new SortableVector (new CIMInstanceComparer ());
   private boolean             fEnableModifications    = true;
   private final boolean       DEBUG                   = true;

   private final String[] INDICATION_KEYS = {
      "InstanceId"
   };
   private final String[] INDICATION_PROPS = {
      "InstanceId",
      "PropertyString",
   };
}
