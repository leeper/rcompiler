// -*- Mode: C++ -*-
//
// Copyright (c) 2006 Rice University
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 

// File: Var.h
//
// Annotation for a variable reference (includes uses and defs)
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANNOTATION_VAR_H
#define ANNOTATION_VAR_H

#include <cassert>
#include <ostream>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationBase.h>
#include <analysis/BasicVar.h>
#include <analysis/LocalityType.h>
#include <analysis/PropertyHndl.h>
#include <analysis/VarVisitor.h>

namespace RAnnot {

// ---------------------------------------------------------------------------
// Var: A variable reference (includes uses and defs)
// ---------------------------------------------------------------------------
class Var
  : public AnnotationBase
{
public:
  typedef BasicVar::UseDefT UseDefT;
  typedef BasicVar::MayMustT MayMustT;

public:
  explicit Var(BasicVar * basic);
  virtual ~Var();
  
  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------
  
  // use/def type
  UseDefT get_use_def_type() const;

  // may/must type
  MayMustT get_may_must_type() const;

  // scope type
  Locality::LocalityType get_scope_type() const;
  void set_scope_type(Locality::LocalityType x);

  // Mention (cons cell that contains the name)
  SEXP get_mention_c() const;

  bool is_first_on_some_path() const;
  void set_first_on_some_path(bool x);

  SEXP get_name() const;

  static PropertyHndlT handle();

  void accept(VarVisitor * v);

#if 0
  // This was an attempt to implement a generic visitor pattern
  // parametrized by the return value. Didn't quite work for a variety
  // of reasons.

  /// generic method for visitor pattern; allows visitors of any
  /// return type
  template<class R> R accept(VarVisitor<R> * v);

  /// general version of accept to be implemented by derived classes
  virtual void * v_accept(VarVisitor<void *>* v) = 0;
#endif

  // -------------------------------------------------------
  // cloning: do not support
  // -------------------------------------------------------
  virtual Var * clone();

  // -------------------------------------------------------
  // code generation
  // -------------------------------------------------------
  //  generating a handle that can be used for accessing the value:
  //    if binding is unknown
  //      find the scope in which the var will be bound
  //      update the value in the proper scope
  //    if binding is condtional
  //      update full/empty bit
  
  //   genCode - phi: generate a handle that can be used to access value
  //     if all reaching defs do not have the same loc binding
  //       generate new handle bound to proper location
  //     if all reaching defs have same loc binding, do nothing
  //  void genCodeInit();
  //  void genCodeRHS();
  //  void genCodeLHS();

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream & dump(std::ostream & os) const;

private:
  Locality::LocalityType m_scope_type;
  bool m_first_on_some_path;
  BasicVar * m_basic;
};

}

#endif
