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
//              Mark Hamzy, hamzy@us.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

import java.util.*;

public class CIMOMHandle implements ProviderCIMOMHandle
{
   private long   cInst;
   private String providerName;

   private native long _getClass               (long          cInst,
                                                long          ciCop,
                                                boolean       localOnly,
                                                boolean       includeQualifiers,
                                                boolean       includeClassOrigin,
                                                String[]      propertyList)
      throws CIMException;
   private native void _deleteClass            (long          cInst,
                                                long          ciCop)
      throws CIMException;
   private native void _createClass            (long          cInst,
                                                long          ciCop,
                                                long          ciClass)
      throws CIMException;
   private native void _setClass               (long          cInst,
                                                long          ciCop,
                                                long          ciClass)
      throws CIMException;
   private native long _enumerateClassNames    (long          cInst,
                                                long          ciCop,
                                                boolean       deep)
      throws CIMException;
   private native long _enumerateClasses       (long          cInst,
                                                long          ciCop,
                                                boolean       deep,
                                                boolean       localOnly,
                                                boolean       includeQualifier,
                                                boolean       includeClassOrigin)
      throws CIMException;

   private native long _getInstance            (long          cInst,
                                                long          ciCop,
                                                boolean       localOnly,
                                                boolean       includeQualifiers,
                                                boolean       includeClassOrigin,
                                                String[]      propertyList)
      throws CIMException;
   private native void _deleteInstance         (long          cInst,
                                                long          ciCop)
      throws CIMException;
   private native long _createInstance         (long          cInst,
                                                long          ciCop,
                                                long          ciInst)
      throws CIMException;
   private native void _modifyInstance         (long          cInst,
                                                long          ciCop,
                                                long          ciInst,
                                                boolean       includeQualifiers,
                                                String[]      propertyList)
      throws CIMException;
   private native long _enumerateInstanceNames (long          cInst,
                                                long          ciCop,
                                                boolean       deep)
      throws CIMException;
   private native long _enumerateInstances     (long          cInst,
                                                long          ciCop,
                                                boolean       deep,
                                                boolean       localOnly,
                                                boolean       includeQualifiers,
                                                boolean       includeClassOrigin,
                                                String[]      propertyList)
      throws CIMException;
   private native long _execQuery              (long          cInst,
                                                long          ciCop,
                                                String        query,
                                                String        ql)
      throws CIMException;

   private native long _getProperty            (long          cInst,
                                                long          ciCop,
                                                String        n)
      throws CIMException;
   private native void _setProperty            (long          cInst,
                                                long          ciCop,
                                                String        nm,
                                                long          val)
      throws CIMException;

   private native long _associatorNames        (long          cInst,
                                                long          ciCop,
                                                String        assocClass,
                                                String        resultClass,
                                                String        role,
                                                String        resultRole)
      throws CIMException;
   private native long _associators            (long          cInst,
                                                long          ciCop,
                                                String        assocClass,
                                                String        resultClass,
                                                String        role,
                                                String        resultRole,
                                                boolean       includeQualifiers,
                                                boolean       includeClassOrigin,
                                                String[]      propertyList)
      throws CIMException;
   private native long _referenceNames         (long          cInst,
                                                long          ciCop,
                                                String        resultClass,
                                                String        role)
      throws CIMException;
   private native long _references             (long          cInst,
                                                long          ciCop,
                                                String        resultClass,
                                                String        role,
                                                boolean       includeQualifiers,
                                                boolean       includeClassOrigin,
                                                String[]      propertyList)
      throws CIMException;
   private native long _invokeMethod           (long          cInst,
                                                long          ciCop,
                                                String        methodName,
                                                Vector        inParams,
                                                Vector        outParams)
      throws CIMException;
   private native long _invokeMethod24         (long          cInst,
                                                long          ciCop,
                                                String        methodName,
                                                CIMArgument[] inParams,
                                                CIMArgument[] outParams)
       throws CIMException;
   private native void _deliverEvent           (long          cInst,
                                                String        providerName,
                                                String        nameSpace,
                                                long          ciIndication)
      throws CIMException;
   private native void _finalize               (long          cInst);

   protected void finalize ()
   {
      _finalize(cInst);
   }

   protected long cInst ()
   {
      return cInst;
   }

   CIMOMHandle (long   ci,
                String name)
   {
      cInst        = ci;
      providerName = name;
   }

    /**
     * @deprecated
     * @see #getClasses(CIMObjectPath name, boolean localOnly, boolean includeQualifiers,
     *         boolean includeClassOrigin, String propertyList[])
     */
   public CIMClass getClass (CIMObjectPath name,
                             boolean       localOnly)
        throws CIMException
   {
      long ciClass = 0;

      if (cInst != 0)
      {
         ciClass = _getClass (cInst,
                              name.cInst (),
                              localOnly,
                              true,
                              true,
                              null);
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

   public CIMClass getClass (CIMObjectPath name,
                             boolean       localOnly,
                             boolean       includeQualifiers,
                             boolean       includeClassOrigin,
                             String[]      propertyList)
        throws CIMException
   {
      long ciClass = 0;

      if (cInst != 0)
      {
         ciClass = _getClass (cInst,
                              name.cInst (),
                              localOnly,
                              includeQualifiers,
                              includeClassOrigin,
                              propertyList);
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

   public void createClass (CIMObjectPath path,
                            CIMClass      cc)
        throws CIMException
   {
      if (cInst != 0)
      {
         _createClass (cInst,
                       path.cInst (),
                       cc.cInst ());
      }
   }

   public void setClass (CIMObjectPath path,
                         CIMClass      cc)
        throws CIMException
   {
      if (cInst != 0)
      {
         _setClass (cInst,
                    path.cInst (),
                    cc.cInst ());
      }
   }

   public void modifyClass (CIMObjectPath path,
                            CIMClass      modifiedClass)
        throws CIMException
   {
      setClass(path,
               modifiedClass);
   }

   public void deleteClass (CIMObjectPath cop)
        throws CIMException
   {
      if (cInst != 0)
      {
         _deleteClass (cInst,
                       cop.cInst ());
      }
   }

   public Enumeration enumClass (CIMObjectPath path,
                                 boolean       deep,
                                 boolean       localOnly)
        throws CIMException
   {
      long ciEnumeration = 0;

      if (cInst != 0)
      {
         ciEnumeration = _enumerateClasses (cInst,
                                            path.cInst (),
                                            deep,
                                            localOnly,
                                            true,
                                            true);
      }

      if (ciEnumeration != 0)
      {
         return new ClassEnumeration (ciEnumeration);
      }
      else
      {
         return null;
      }
   }

   public Enumeration enumerateClasses (CIMObjectPath name,
                                        boolean       deep,
                                        boolean       localOnly,
                                        boolean       includeQualifier,
                                        boolean       includeClassOrigin)
        throws CIMException
   {
      long ciEnumeration = 0;

      if (cInst != 0)
      {
         ciEnumeration = _enumerateClasses (cInst,
                                            name.cInst (),
                                            deep,
                                            localOnly,
                                            includeQualifier,
                                            includeClassOrigin);
      }

      if (ciEnumeration != 0)
      {
         return new ClassEnumeration (ciEnumeration);
      }
      else
      {
         return null;
      }
   }

   public Enumeration enumClass (CIMObjectPath path,
                                 boolean       deep)
        throws CIMException
   {
      return enumerateClassNames(path,
                                 deep);
   }

   public Enumeration enumerateClassNames (CIMObjectPath path,
                                           boolean       deepInheritance)
        throws CIMException
   {
      long ciEnumeration = 0;

      if (cInst != 0)
      {
         ciEnumeration = _enumerateClassNames (cInst,
                                               path.cInst (),
                                               deepInheritance);
      }

      if (ciEnumeration != 0)
      {
         return new PathEnumeration (ciEnumeration);
      }
      else
      {
         return null;
      }
   }

   public CIMInstance getInstance (CIMObjectPath path,
                                   boolean       localOnly)
        throws CIMException
   {
      return getInstance(path,
                         localOnly,
                         true,
                         false,
                         (String[])null);
   }

   public CIMInstance getInstance (CIMObjectPath path,
                                   boolean       localOnly,
                                   boolean       includeQualifiers,
                                   boolean       includeClassOrigin,
                                   String[]      propertyList)
        throws CIMException
   {
      long ciInstance = 0;

      if (cInst != 0)
      {
         ciInstance = _getInstance (cInst,
                                    path.cInst (),
                                    localOnly,
                                    includeQualifiers,
                                    includeClassOrigin,
                                    propertyList);
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

   public void deleteInstance (CIMObjectPath cop)
        throws CIMException
   {
      if (cInst != 0)
      {
         _deleteInstance (cInst,
                          cop.cInst());
      }
   }

   public CIMObjectPath createInstance (CIMObjectPath path,
                                        CIMInstance   ci)
        throws CIMException
   {
      long ciObjectPath = 0;

      if (cInst != 0)
      {
         ciObjectPath = _createInstance (cInst,
                                         path.cInst (),
                                         ci.cInst ());
      }

      if (ciObjectPath != 0)
      {
         return new CIMObjectPath (ciObjectPath);
      }
      else
      {
         return null;
      }
   }

   public void setInstance (CIMObjectPath path,
                            CIMInstance   ci)
        throws CIMException
   {
      modifyInstance(path,
                     ci,
                     true,
                     (String[])null);
   }

   public void modifyInstance (CIMObjectPath path,
                               CIMInstance   modifiedInstance,
                               boolean       includeQualifiers,
                               String[]      propertyList)
        throws CIMException
   {
      if (cInst != 0)
      {
         _modifyInstance (cInst,
                          path.cInst (),
                          modifiedInstance.cInst (),
                          includeQualifiers,
                          propertyList);
      }
   }

   public Enumeration enumInstances (CIMObjectPath path,
                                     boolean       deep)
        throws CIMException
   {
      long ciEnumeration = 0;

      if (cInst != 0)
      {
         ciEnumeration = _enumerateInstanceNames (cInst,
                                                  path.cInst (),
                                                  deep);
      }

      if (ciEnumeration != 0)
      {
         return new PathEnumeration (ciEnumeration);
      }
      else
      {
         return null;
      }
   }

   public Enumeration enumerateInstanceNames (CIMObjectPath path)
        throws CIMException
   {
       return enumInstances(path,
                            true);
   }

   public Enumeration enumInstances (CIMObjectPath cop,
                                     boolean       deep,
                                     boolean       localOnly)
         throws CIMException
   {
      return enumerateInstances(cop,
                                deep,
                                localOnly,
                                true,
                                true,
                                (String[])null);
   }

   public Enumeration enumerateInstances (CIMObjectPath path,
                                          boolean       deepInheritance,
                                          boolean       localOnly,
                                          boolean       includeQualifiers,
                                          boolean       includeClassOrigin,
                                          String[]      propertyList)
        throws CIMException
   {
      long ciEnumeration = 0;

      if (cInst != 0)
      {
         ciEnumeration = _enumerateInstances (cInst,
                                              path.cInst (),
                                              deepInheritance,
                                              localOnly,
                                              includeQualifiers,
                                              includeClassOrigin,
                                              propertyList);
      }

      if (ciEnumeration != 0)
      {
         return new InstEnumeration (ciEnumeration);
      }
      else
      {
         return null;
      }
   }

   public Enumeration execQuery (CIMObjectPath path,
                                 String        query,
                                 String        ql)
        throws CIMException
   {
      long ciEnumeration = 0;

      if (cInst != 0)
      {
         ciEnumeration = _execQuery (cInst,
                                     path.cInst (),
                                     query,
                                     ql);
      }

      if (ciEnumeration != 0)
      {
         return new InstEnumeration (ciEnumeration);
      }
      else
      {
         return null;
      }
   }

   public CIMValue getProperty (CIMObjectPath path,
                                String        propertyName)
        throws CIMException
   {
      long ciValue = 0;

      if (cInst != 0)
      {
         ciValue = _getProperty (cInst,
                                 path.cInst (),
                                 propertyName);
      }

      if (ciValue != 0)
      {
         return new CIMValue (ciValue);
      }
      else
      {
         return null;
      }
   }

   public void setProperty (CIMObjectPath path,
                            String        propertyName,
                            CIMValue      newValue)
        throws CIMException
   {
      if (cInst != 0)
      {
         _setProperty (cInst,
                       path.cInst (),
                       propertyName,
                       newValue.cInst ());
      }
   }

   public Enumeration associatorNames (CIMObjectPath path,
                                       String        assocClass,
                                       String        resultClass,
                                       String        role,
                                       String        resultRole)
        throws CIMException
   {
      long ciEnumeration = 0;

      if (cInst != 0)
      {
         ciEnumeration = _associatorNames (cInst,
                                           path.cInst (),
                                           assocClass,
                                           resultClass,
                                           role,
                                           resultRole);
      }

      if (ciEnumeration != 0)
      {
         return new PathEnumeration (ciEnumeration);
      }
      else
      {
         return null;
      }
   }

   public Enumeration associators(CIMObjectPath path,
                                  String        assocClass,
                                  String        resultClass,
                                  String        role,
                                  String        resultRole,
                                  boolean       includeQualifiers,
                                  boolean       includeClassOrigin,
                                  String[]      propertyList)
          throws CIMException
   {
      long ciEnumeration = 0;

      if (cInst != 0)
      {
         ciEnumeration = _associators (cInst,
                                       path.cInst (),
                                       assocClass,
                                       resultClass,
                                       role,
                                       resultRole,
                                       includeQualifiers,
                                       includeClassOrigin,
                                       propertyList);
      }

      if (ciEnumeration != 0)
      {
         return new InstEnumeration (ciEnumeration);
      }
      else
      {
         return null;
      }
   }

   public Enumeration referenceNames (CIMObjectPath path,
                                      String        resultClass,
                                      String        role)
        throws CIMException
   {
      long ciEnumeration = 0;

      if (cInst != 0)
      {
         ciEnumeration = _referenceNames (cInst,
                                          path.cInst (),
                                          resultClass,
                                          role);
      }

      if (ciEnumeration != 0)
      {
         return new PathEnumeration (ciEnumeration);
      }
      else
      {
         return null;
      }
   }

   public Enumeration references (CIMObjectPath path,
                                  String        resultClass,
                                  String        role,
                                  boolean       includeQualifiers,
                                  boolean       includeClassOrigin,
                                  String[]      propertyList)
        throws CIMException
   {
      long ciEnumeration = 0;

      if (cInst != 0)
      {
         ciEnumeration = _references (cInst,
                                      path.cInst (),
                                      resultClass,
                                      role,
                                      includeQualifiers,
                                      includeClassOrigin,
                                      propertyList);
      }

      if (ciEnumeration != 0)
      {
         return new InstEnumeration (ciEnumeration);
      }
      else
      {
         return null;
      }
   }

   public CIMValue invokeMethod (CIMObjectPath path,
                                 String        methodName,
                                 Vector        inParams,
                                 Vector        outParams)
        throws CIMException
   {
      long ciValue = 0;

      if (cInst != 0)
      {
         ciValue = _invokeMethod (cInst,
                                  path.cInst (),
                                  methodName,
                                  inParams,
                                  outParams);
      }

      if (ciValue != 0)
      {
         return new CIMValue (ciValue);
      }
      else
      {
         return null;
      }
   }

   public CIMValue invokeMethod (CIMObjectPath path,
                                 String        methodName,
                                 CIMArgument[] inParams,
                                 CIMArgument[] outParams)
        throws CIMException
   {
      throw new CIMException(CIMException.CIM_ERR_NOT_SUPPORTED);
      // return new CIMValue(_invokeMethod24(cInst,cNsInst,path.cInst (),methodName,inParams,outParams));
   }

   public void deliverEvent (String      ns,
                             CIMInstance ind)
        throws CIMException
   {
      if (cInst != 0)
      {
         _deliverEvent (cInst,
                        providerName,
                        ns,
                        ind.cInst ());
      }
   }

   static {
      System.loadLibrary("JMPIProviderManager");
   }
}
