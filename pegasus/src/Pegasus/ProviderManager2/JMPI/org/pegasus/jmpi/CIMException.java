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
// Modified By: Mark Hamzy, hamzy@us.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////


package org.pegasus.jmpi;

import java.lang.Exception;

public class CIMException  extends Exception {

   public static final String CIM_ERR_SUCCESS="CIM_ERR_SUCCESS";
   public static final String CIM_ERR_FAILED="CIM_ERR_FAILED";
   public static final String CIM_ERR_ACCESS_DENIED="CIM_ERR_ACCESS_DENIED";
   public static final String CIM_ERR_INVALID_NAMESPACE="CIM_ERR_INVALID_NAMESPACE";
   public static final String CIM_ERR_INVALID_PARAMETER="CIM_ERR_INVALID_PARAMETER";
   public static final String CIM_ERR_INVALID_CLASS="CIM_ERR_INVALID_CLASS";
   public static final String CIM_ERR_NOT_FOUND="CIM_ERR_NOT_FOUND";
   public static final String CIM_ERR_NOT_SUPPORTED="CIM_ERR_NOT_SUPPORTED";
   public static final String CIM_ERR_CLASS_HAS_CHILDREN="CIM_ERR_CLASS_HAS_CHILDREN";
   public static final String CIM_ERR_CLASS_HAS_INSTANCES="CIM_ERR_CLASS_HAS_INSTANCES";
   public static final String CIM_ERR_INVALID_SUPERCLASS="CIM_ERR_INVALID_SUPERCLASS";
   public static final String CIM_ERR_ALREADY_EXISTS="CIM_ERR_ALREADY_EXISTS";
   public static final String CIM_ERR_NO_SUCH_PROPERTY="CIM_ERR_NO_SUCH_PROPERTY";
   public static final String CIM_ERR_TYPE_MISMATCH="CIM_ERR_TYPE_MISMATCH";
   public static final String CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED="CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED";
   public static final String CIM_ERR_INVALID_QUERY="CIM_ERR_INVALID_QUERY";
   public static final String CIM_ERR_METHOD_NOT_AVAILABLE="CIM_ERR_METHOD_NOT_AVAILABLE";
   public static final String CIM_ERR_METHOD_NOT_FOUND="CIM_ERR_METHOD_NOT_FOUND";

   static String code2string[]={
      CIM_ERR_SUCCESS,
      CIM_ERR_FAILED,
      CIM_ERR_ACCESS_DENIED,
      CIM_ERR_INVALID_NAMESPACE,
      CIM_ERR_INVALID_PARAMETER,
      CIM_ERR_INVALID_CLASS,
      CIM_ERR_NOT_FOUND,
      CIM_ERR_NOT_SUPPORTED,
      CIM_ERR_CLASS_HAS_CHILDREN,
      CIM_ERR_CLASS_HAS_INSTANCES,
      CIM_ERR_INVALID_SUPERCLASS,
      CIM_ERR_ALREADY_EXISTS,
      CIM_ERR_NO_SUCH_PROPERTY,
      CIM_ERR_TYPE_MISMATCH,
      CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED,
      CIM_ERR_INVALID_QUERY,
      CIM_ERR_METHOD_NOT_AVAILABLE,
      CIM_ERR_METHOD_NOT_FOUND
   };

   protected String msg=null;
   protected String id;
   protected Object[] params = null;

   public CIMException() {
   }

   public CIMException(int code) {
      if (code>=0 && code<=17)
         id=code2string[code];
      else id=CIM_ERR_FAILED;
   }

   public CIMException(int code, String m) {
      if (code>=0 && code<=17)
         id=code2string[code];
      else id=CIM_ERR_FAILED;
      msg=m;
   }

   public CIMException(String strId, String m) {
      id=strId;
      msg=m;
   }

   public CIMException(String strId) {
      id=strId;
   }

   public CIMException(String strId, Object o1) {
      id=strId;
      params = new Object[1];
      params[0] = o1;
   }

   public CIMException(String strId, Object o1, Object o2) {
      id=strId;
      params = new Object[2];
      params[0] = o1;
      params[0] = o2;
   }

    public CIMException(String strId, Object o1, Object o2, Object o3) {
      id=strId;
      params = new Object[3];
      params[0] = o1;
      params[0] = o2;
      params[0] = o3;
   }

   public String getID() {
      return(id);
   }

   public int getCode() {
      for (int i=0; i<17; i++)
         if (id.equals(code2string[i])) return i;
      return 1;
   }

   public String toString() {
      if (msg!=null)
         return(id+" ("+msg+")");
      String str=id;
      if (params!=null) {
         for (int i=0,m=params.length; i<m; i++) {
            if (i==0) str=str+"\n";
            else str=str+",";
            str=str+params[i];
         }
      }
      return str;
   }

   public String getMessage() {
      if (msg!=null)
         return(msg);
      String str="";
      if (params!=null) {
         for (int i=0,m=params.length; i<m; i++) {
            if (i!=0) str=str+",";
            str=str+params[i];
         }
      }
      return str;
   }

   static {
   }
};
