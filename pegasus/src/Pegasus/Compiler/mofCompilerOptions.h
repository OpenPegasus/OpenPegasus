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


//
// Header for a class to hold options available to users of the
// cimmof embeddable compiler.
//
//
//
// Defines the mofCompilerOptions class, which contains all you
// want to know about the command line that invoked the compiler,
// including
//             The include (-I) paths
//             The repository to talk to (-R)
//             Other compiler options
//                 syntax check only (-E)
//                 suppress warnings (-w)
//             The files to be compiled.
//
// ATTN: BB P3  Mar 2001 This needs to be reworked so that the same options
// offered by the Microsoft compiler are supported.  They allow modes for
//      -- Marking as an error an attempt to create an existing object
//      -- Marking as an error an attempt to create an existing object
//         with different elements (i.e. overwriting)

#ifndef _MOFCOMPILEROPTIONS_H_
#define _MOFCOMPILEROPTIONS_H_

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Compiler/compilerCommonDefs.h>
#include <Pegasus/Compiler/Linkage.h>
#include <iostream>

PEGASUS_USING_PEGASUS;

class PEGASUS_COMPILER_LINKAGE mofCompilerOptions {
 private:
  Array<String> _include_paths;
  Array<String> _filespecs;
  String       _repository;
  String        _repository_name;
  String        _repository_mode;
  String                _namespacePath;
  Boolean         _syntax_only;
  Boolean         _is_local;
  Boolean         _suppress_warnings;
  Boolean         _suppress_all_messages;
  Boolean         _emit_xml;
  compilerCommonDefs::operationType _ot;
  Boolean         _trace;
  Boolean     _update_class;
  Boolean     _allow_experimental;
  Boolean     _allow_version;
#ifdef PEGASUS_OS_PASE
  Boolean         _quiet;
#endif
  Boolean         _no_usage_warning;
  Boolean         _mrr;
  Boolean         _discard;

  PEGASUS_STD(ostream)      *_traceos;
  PEGASUS_STD(ostream)      *_erroros;
  PEGASUS_STD(ostream)      *_warningos;
 public:
  mofCompilerOptions() : _repository(String::EMPTY),
    _repository_name(String::EMPTY),
    _repository_mode(String::EMPTY),
    _namespacePath(""),
    _syntax_only(false),
    _is_local(false),
    _suppress_warnings(false),
    _suppress_all_messages(false),
    _emit_xml(false),
    _ot(compilerCommonDefs::USE_REPOSITORY),
    _trace(false),
    _no_usage_warning(false),
    _mrr(false),
    _discard(false),
    _traceos(0),
    _erroros(0),
    _warningos(0)
    {;}

  ~mofCompilerOptions() {;}

  void add_include_path(const String& path) { _include_paths.append(path); }
  const Array<String> &get_include_paths() const { return _include_paths; }
  void add_filespecs(const String &spec) { _filespecs.append(spec); }
  const Array<String>& get_include_path_list() const {return _include_paths; }
  const Array<String>& get_filespec_list() const { return _filespecs; }
  void  set_repository(const String &repository) { \
    _repository = repository; }
  const String &get_repository() const { return _repository; }
  void  set_repository_name(const String &repository_name) { \
    _repository_name = repository_name; }
  const String &get_repository_name() const { return _repository_name; }
  void  set_repository_mode(const String &repository_mode) { \
    _repository_mode = repository_mode; }
  const String &get_repository_mode() const { return _repository_mode; }
  void set_syntax_only() { _syntax_only = true; }
  void reset_syntax_only() { _syntax_only = false; }
  Boolean syntax_only() const { return _syntax_only; }
  void set_is_local() { _is_local = true; }
  void reset_is_local() { _is_local = false; }
  Boolean is_local() const { return _is_local; }
  void set_suppress_warnings() { _suppress_warnings = true; }
  void reset_suppress_warnings() { _suppress_warnings = false; }
  Boolean suppress_warnings() const { return _suppress_warnings; }
  void set_suppress_all_messages() { _suppress_all_messages = true; }
  void reset_suppress_all_messages() { _suppress_all_messages = false; }
  Boolean suppress_all_messages() const { return _suppress_all_messages; }
  void set_trace() { _trace = true; }
  void reset_trace() { _trace = false; }
  Boolean trace() const { return _trace; }
  void set_update_class() { _update_class = true; }
  void reset_update_class() { _update_class = false; }
  Boolean update_class() const { return _update_class; }
  void set_allow_experimental() { _allow_experimental = true; }
  void reset_allow_experimental() { _allow_experimental = false; }
  Boolean allow_experimental() const { return _allow_experimental; }
  void set_allow_version() { _allow_version = true; }
  void reset_allow_version() { _allow_version = false; }
  Boolean allow_version() const { return _allow_version; }
#ifdef PEGASUS_OS_PASE
  void set_quiet() { _quiet = true; }
  void reset_quiet() { _quiet = false; }
  Boolean quiet() const { return _quiet; }
#endif
  void set_operationType(compilerCommonDefs::operationType ot) { _ot = ot; }
  void reset_operationType() { _ot = compilerCommonDefs::USE_REPOSITORY; }
  compilerCommonDefs::operationType operationType() const { return _ot; }
  void set_xmloutput() { _emit_xml = true; }
  void reset_xmloutput() { _emit_xml = false; }
  Boolean xml_output() const { return _emit_xml; }
  void set_traceos(PEGASUS_STD(ostream) &os) { _traceos = &os; }
  void reset_traceos() { _traceos = 0; }
  PEGASUS_STD(ostream) &traceos() const
  {
      return _traceos ? (PEGASUS_STD(ostream)&)*_traceos :
          (PEGASUS_STD(ostream)&)PEGASUS_STD(cout);
  }
  void set_erroros(PEGASUS_STD(ostream) &os) { _erroros = &os; }
  void reset_erroros() { _erroros = 0; }
  PEGASUS_STD(ostream) &erroros() const
  {
      return _erroros ? (PEGASUS_STD(ostream)&)*_erroros :
          (PEGASUS_STD(ostream)&)PEGASUS_STD(cerr);
  }
  void set_warningos(PEGASUS_STD(ostream &os)) { _warningos = &os; }
  void reset_warningos() { _warningos = 0; }
  PEGASUS_STD(ostream) &warningos() const
  {
      return _warningos ? (PEGASUS_STD(ostream)&)*_warningos :
          (PEGASUS_STD(ostream)&)PEGASUS_STD(cerr);
  }
  void set_namespacePath(const String &path) { _namespacePath = path; }
  const String &get_namespacePath() const  { return _namespacePath; }
  void set_no_usage_warning() { _no_usage_warning = true; }
  Boolean get_no_usage_warning() const { return _no_usage_warning; }
  void set_mrr() { _mrr = true; }
  void reset_mrr() { _mrr = false; }
  Boolean mrr() const { return _mrr; }
  void set_discard() { _discard = true; }
  void reset_discard() { _discard = false; }
  Boolean discard() const { return _discard; }
};

#endif
