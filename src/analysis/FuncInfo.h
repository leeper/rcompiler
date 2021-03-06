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

// File: FuncInfo.h
//
// Annotation for a function.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANNOTATION_FUNC_INFO_H
#define ANNOTATION_FUNC_INFO_H

#include <set>

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/CFG/CFG.hpp>

#include <support/trees/NonUniformDegreeTreeTmpl.h>

#include <analysis/AnnotationBase.h>
#include <analysis/BasicFuncInfo.h>
#include <analysis/FuncInfoAnnotationMap.h>
#include <analysis/LexicalScope.h>
#include <analysis/PropertyHndl.h>
#include <analysis/StrictnessResult.h>
#include <analysis/Var.h>

// iterator macros for convenience and readability. Be sure to declare
// the variables somewhere above the for loop. Note: don't put
// initializations in macros except inside for-loop initializers

#define PROC_FOR_EACH_MENTION(fi, mi) \
  for(RAnnot::FuncInfo::const_mention_iterator mi = (fi)->begin_mentions(); \
      mi != (fi)->end_mentions();					    \
      ++mi)

#define PROC_FOR_EACH_CALL_SITE(fi, csi) \
  for(RAnnot::FuncInfo::const_call_site_iterator csi = (fi)->begin_call_sites(); \
      csi != (fi)->end_call_sites();					         \
      ++csi)

namespace RAnnot {

//****************************************************************************
// Annotations: Function info
//****************************************************************************

// ---------------------------------------------------------------------------
// FuncInfo: 'Definition' information about a function
// ---------------------------------------------------------------------------
class FuncInfo : public NonUniformDegreeTreeNodeTmpl<FuncInfo>, 
		 public AnnotationBase
{
public:
  typedef SEXP                                             MentionT;

  typedef std::list<MentionT>                              MentionSetT;
  typedef MentionSetT::iterator                            mention_iterator;
  typedef MentionSetT::const_iterator                      const_mention_iterator;

  typedef SEXP                                             CallSiteT;
  typedef std::list<CallSiteT>                             CallSiteSetT;
  typedef CallSiteSetT::iterator                           call_site_iterator;
  typedef CallSiteSetT::const_iterator                     const_call_site_iterator;
public:
  explicit FuncInfo(FuncInfo * parent, BasicFuncInfo * bfe);
  virtual ~FuncInfo();

  // -------------------------------------------------------
  // cloning: not supported 
  // -------------------------------------------------------
  virtual AnnotationBase * clone();

  static PropertyHndlT handle();

  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------

  // ----- basic information (wrappers around BasicFuncInfo) -----

  SEXP get_sexp() const;

  /// cell containing first R name assigned
  SEXP get_first_name_c() const;

  /// name of C function
  const std::string & get_c_name(); // not const: fills in m_c_name if empty

  /// name of C variable storing the closure (CLOSXP)
  const std::string & get_closure() const;

  // ----- arguments (wrappers around BasicFuncInfo) -----

  unsigned int get_num_args() const;
  SEXP get_args() const; 
  bool is_arg(SEXP sym) const;
  
  /// get the cell containing the formal arg at the given position. Indexed from 1.
  SEXP get_arg(int position) const;

  /// find the numerical position of the formal with the given name. Indexed from 1.
  int find_arg_position(char * name) const;

  // has variable arguments
  bool get_has_var_args() const;

  // ----- CFG  (wrapper around BasicFuncInfo) -----

  OA::OA_ptr<OA::CFG::CFG> get_cfg() const;
  
  // ----- lexical scope and tree structure (wrappers around BasicFuncInfo) -----

  const FundefLexicalScope * get_scope() const;

  bool has_children() const;

  // ----- return statements  (wrappers around BasicFuncInfo) -----

  const std::set<SEXP> * get_implicit_returns() const;
  bool is_return(SEXP) const;
  SEXP return_value_c(const SEXP) const;
  
  // ----- context (wrapper around BasicFuncInfo) -----
  bool requires_context() const;

  // mention iterators
  mention_iterator begin_mentions();
  const_mention_iterator begin_mentions() const;
  mention_iterator end_mentions();
  const_mention_iterator end_mentions() const;
  
  // call site iterators
  call_site_iterator begin_call_sites();
  const_call_site_iterator begin_call_sites() const;
  call_site_iterator end_call_sites();
  const_call_site_iterator end_call_sites() const;

  OA::OA_ptr<Strictness::StrictnessResult> get_strictness() const;
  void set_strictness(OA::OA_ptr<Strictness::StrictnessResult> x);

  // FuncInfo delegates several methods to BasicFuncInfo
  BasicFuncInfo * get_basic();

  // debugging
  virtual std::ostream & dump(std::ostream & os) const;

private:

  // ----- add information about mentions and call sites -----

  void insert_mention(MentionT v);
  void insert_call_site(CallSiteT e);

  // ----- analysis -----

  void accum_implicit_returns(SEXP cell);

  /// Find each mention (use or def) and call site in the function
  void collect_mentions_and_call_sites();

  void analyze_strictness();

  void analyze_debuts();

  /// results of strictness analysis: strictness of formals, debuts, post-debut statements
  OA::OA_ptr<Strictness::StrictnessResult> m_strictness;

  MentionSetT m_mentions; // uses and defs inside function (NOT including nested functions)
  CallSiteSetT m_call_sites; // call sites inside function (NOT including nested functions)
  BasicFuncInfo * m_basic; // basic annotation; this is a wrapper around it
};

class FuncInfoChildIterator : public NonUniformDegreeTreeNodeChildIteratorTmpl<FuncInfo> {
public:
  explicit FuncInfoChildIterator(const FuncInfo * fi, bool firstToLast = true) :
    NonUniformDegreeTreeNodeChildIteratorTmpl<FuncInfo>(fi, firstToLast) {};
};

class FuncInfoIterator : public NonUniformDegreeTreeIteratorTmpl<FuncInfo> {
public:
  explicit FuncInfoIterator(const FuncInfo * fi, TraversalOrder torder = PreOrder,
			    NonUniformDegreeTreeEnumType how =
			    NON_UNIFORM_DEGREE_TREE_ENUM_ALL_NODES);
  
};

}

#endif
