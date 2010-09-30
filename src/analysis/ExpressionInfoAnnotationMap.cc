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

// File: ExpressionInfoAnnotationMap.cc
//
// Maps expression SEXPs to ExpressionInfo annotations.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <map>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>

#include <support/Debug.h>
#include <support/RccError.h>

#include <analysis/Analyst.h>
#include <analysis/AnalysisResults.h>
#include <analysis/BasicFuncInfo.h>
#include <analysis/BasicFuncInfoAnnotationMap.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/PropertySet.h>
#include <analysis/SexpTraversal.h>

#include "ExpressionInfoAnnotationMap.h"

static bool debug;

using namespace HandleInterface;

namespace RAnnot {

// type definitions for readability
typedef ExpressionInfoAnnotationMap::MyKeyT MyKeyT;
typedef ExpressionInfoAnnotationMap::MyMappedT MyMappedT;
typedef ExpressionInfoAnnotationMap::iterator iterator;
typedef ExpressionInfoAnnotationMap::const_iterator const_iterator;

// ----- constructor/destructor ----- 

ExpressionInfoAnnotationMap::ExpressionInfoAnnotationMap()
{
  RCC_DEBUG("RCC_ExpressionInfoAnnotationMap", debug);
}

ExpressionInfoAnnotationMap::~ExpressionInfoAnnotationMap() {
  std::map<MyKeyT, MyMappedT>::const_iterator iter;
  for(iter = get_map().begin(); iter != get_map().end(); ++iter) {
    delete(iter->second);
  }
}

// ----- computation -----

void ExpressionInfoAnnotationMap::compute() {
  SexpTraversal::instance();
#if 0
  BasicFuncInfo * bfi;
  OA::OA_ptr<OA::CFG::NodeInterface> node;
  OA::StmtHandle stmt;

  // we need to make expressions out of statements, call sites, and actual arguments
  FOR_EACH_BASIC_PROC(bfi) {
    PROC_FOR_EACH_NODE(bfi, node) {
      NODE_FOR_EACH_STATEMENT(node, stmt) {
	// statements
	ExpressionInfo * annot = make_annot(make_sexp(stmt));
	if (is_for(CAR(make_sexp(stmt)))) {
	  make_annot(for_range_c(CAR(make_sexp(stmt))));
	}
	for(ExpressionInfo::const_call_site_iterator csi = annot->begin_call_sites(); csi != annot->end_call_sites(); ++csi) {
	  // call sites
	  make_annot(*csi);
	  // LHS
	  make_annot(CAR(*csi));
	  for(R_CallArgsIterator arg_it(CAR(*csi)); arg_it.isValid(); ++arg_it) {
	    // actual arguments
	    make_annot(arg_it.current());
	  }
	}
      }
    }
  }
#endif
}

#if 0
ExpressionInfo * ExpressionInfoAnnotationMap::make_annot(const MyKeyT & k) {
  if (debug) {
    std::cout << "ExpressionInfoAnnotationMap analyzing the following expression:" << std::endl;
    Rf_PrintValue(CAR(k));
  }

  ExpressionInfo * annot = new ExpressionInfo(k);
  build_ud_rhs(annot, k, BasicVar::Var_MUST, true);
  get_map()[k] = annot;
  return annot;
}
#endif

// ----- singleton pattern -----

ExpressionInfoAnnotationMap * ExpressionInfoAnnotationMap::instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT ExpressionInfoAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

// Create the singleton instance and register the map in PropertySet
// for getProperty
void ExpressionInfoAnnotationMap::create() {
  s_instance = new ExpressionInfoAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

/// Traverse the given SEXP (not an lvalue) and set variable
/// annotations with local syntactic information.
/// cell          = a cons cell whose CAR is the expression we're talking about
/// may_must_type = whether var uses should be may-use or must-use
///                 (usually must-use; may-use if we're looking at an actual argument
///                 to a call-by-need function)
/// is_stmt       = true if we're calling build_ud_rhs from outside, false if sub-SEXP
void ExpressionInfoAnnotationMap::build_ud_rhs(ExpressionInfo * ei,
					       const SEXP cell,
					       Var::MayMustT may_must_type,
					       bool is_stmt)
{
  assert(is_cons(cell));
  SEXP e = CAR(cell);
  if (is_const(e)) {
    // ignore
  } else if (e == R_MissingArg) {
    // ignore
  } else if (is_var(e)) {
    make_use_var(ei, cell, UseVar::UseVar_ARGUMENT, BasicVar::Var_MUST, Locality::Locality_TOP);
  } else if (is_local_assign(e)) {
    build_ud_lhs(ei, assign_lhs_c(e), assign_rhs_c(e), BasicVar::Var_MUST, IN_LOCAL_ASSIGN);
    build_ud_rhs(ei, assign_rhs_c(e), BasicVar::Var_MUST, false);
  } else if (is_free_assign(e)) {
    build_ud_lhs(ei, assign_lhs_c(e), assign_rhs_c(e), BasicVar::Var_MUST, IN_FREE_ASSIGN);
    build_ud_rhs(ei, assign_rhs_c(e), BasicVar::Var_MUST, false);
  } else if (is_fundef(e)) {
    // ignore
  } else if (is_struct_field(e)) {
    build_ud_rhs(ei, CDR(e), BasicVar::Var_MUST, false);
  } else if (is_subscript(e)) {
    ei->insert_call_site(cell);
    // use of '[' symbol
    make_use_var(ei, e, UseVar::UseVar_FUNCTION, BasicVar::Var_MUST, Locality::Locality_TOP);
    // left side of subscript
    build_ud_rhs(ei, subscript_lhs_c(e), BasicVar::Var_MUST, false);
    // subscript args
    for (SEXP sub_c = subscript_first_sub_c(e); sub_c != R_NilValue; sub_c = CDR(sub_c)) {
      build_ud_rhs(ei, sub_c, BasicVar::Var_MUST, false);
    }
  } else if (is_if(e)) {
    build_ud_rhs(ei, if_cond_c(e), BasicVar::Var_MUST, false);
    if (!is_stmt) {
      build_ud_rhs(ei, if_truebody_c(e), BasicVar::Var_MAY, false);
      if (CAR(if_falsebody_c(e)) != R_NilValue) build_ud_rhs(ei, if_falsebody_c(e), BasicVar::Var_MAY, false);
    }
  } else if (is_for(e)) {
    // defines the induction variable, uses the range
    build_ud_lhs(ei, for_iv_c(e), for_range_c(e), BasicVar::Var_MUST, IN_LOCAL_ASSIGN);
    build_ud_rhs(ei, for_range_c(e), BasicVar::Var_MUST, false);
  } else if (is_loop_header(e)) {
    // TODO
    // currently this case cannot happen
  } else if (is_loop_increment(e)) {
    // TODO
    // currently this case cannot happen
  } else if (is_while(e)) {
    build_ud_rhs(ei, while_cond_c(e), BasicVar::Var_MUST, false);
  } else if (is_repeat(e)) {
    // ignore
  } else if (is_paren_exp(e)) {
    build_ud_rhs(ei, paren_body_c(e), BasicVar::Var_MUST, false);
  } else if (is_curly_list(e)) {
    for (SEXP stmt = CDR(e); stmt != R_NilValue; stmt = CDR(stmt)) {
      build_ud_rhs(ei, stmt, BasicVar::Var_MUST, false);
    }
  } else if (TYPEOF(e) == LANGSXP) {   // regular function call
    ei->insert_call_site(cell);
    if (is_var(CAR(e))) {
      make_use_var(ei, e, UseVar::UseVar_FUNCTION, BasicVar::Var_MUST, Locality::Locality_TOP);
    } else {
      build_ud_rhs(ei, e, BasicVar::Var_MUST, false);
    }
    // recur on args
    // Because R functions are call-by-need, var uses that appear in
    // args passed to them are MAY-use. BUILTINSXP functions are CBV,
    // so vars passed as args should stay MUST.
    for (SEXP stmt = CDR(e); stmt != R_NilValue; stmt = CDR(stmt)) {
      if (is_var(CAR(e)) && TYPEOF(Rf_findFunUnboundOK(CAR(e), R_GlobalEnv, TRUE)) == BUILTINSXP) {
	build_ud_rhs(ei, stmt, BasicVar::Var_MUST, false);
      } else {
	build_ud_rhs(ei, stmt, BasicVar::Var_MAY, false);
      }
    }
  } else {
    assert(0);
  }  
}

/// Traverse the SEXP contained in the given cons cell, assuming that
/// it's an lvalue. Set variable annotations with local syntactic
/// information.
/// cell          = a cons cell whose CAR is the lvalue we're talking about
/// rhs           = a cons cell whose CAR is the right side of the assignment statement
/// may_must_type = whether we are in a may-def or a must-def
/// lhs_type      = whether we are in a local or free assignment
void ExpressionInfoAnnotationMap::build_ud_lhs(ExpressionInfo * ei,
					       const SEXP cell,
					       const SEXP rhs_c,
					       Var::MayMustT may_must_type,
					       LhsType lhs_type)
{
  assert(is_cons(cell));
  assert(is_cons(rhs_c));
  SEXP e = CAR(cell);
  if (is_var(e) || Rf_isString(e)) {
    if (Rf_isString(e)) {
      SETCAR(cell, Rf_install(CHAR(STRING_ELT(e, 0))));
      e = CAR(cell);
    }
    Locality::LocalityType locality;
    if (lhs_type == IN_LOCAL_ASSIGN) {
      locality = Locality::Locality_LOCAL;
    } else {
      locality = Locality::Locality_FREE;
    }
    make_def_var(ei, cell, DefVar::DefVar_ASSIGN, may_must_type, locality, rhs_c);
  } else if (is_struct_field(e)) {
    build_ud_lhs(ei, struct_field_lhs_c(e), rhs_c, BasicVar::Var_MAY, lhs_type);
  } else if (is_subscript(e)) {
    build_ud_lhs(ei, subscript_lhs_c(e), rhs_c, BasicVar::Var_MAY, lhs_type);
    for (SEXP sub_c = subscript_first_sub_c(e); sub_c != R_NilValue; sub_c = CDR(sub_c)) {    
      build_ud_rhs(ei, sub_c, BasicVar::Var_MUST, false);
    }
  } else if (TYPEOF(e) == LANGSXP) {  // regular function call
    // Function application as lvalue. Examples: dim(x) <- foo, attr(x, "dim") <- c(2, 5)
    //
    // TODO: We should really be checking if the function is valid;
    // only some functions applied to arguments make a valid lvalue.
    build_ud_rhs(ei, e, BasicVar::Var_MUST, false);                 // assignment function
    build_ud_lhs(ei, CDR(e), rhs_c, BasicVar::Var_MAY, lhs_type);   // first arg is lvalue
    SEXP arg = CDDR(e);                                    // other args are rvalues if they exist
    while (arg != R_NilValue) {
      build_ud_rhs(ei, arg, BasicVar::Var_MUST, false);
      arg = CDR(arg);
    }
  } else {
    assert(0);
  }
}

void ExpressionInfoAnnotationMap::make_use_var(ExpressionInfo * ei,
					       SEXP cell,
					       UseVar::PositionT pos,
					       BasicVar::MayMustT mmt,
					       Locality::LocalityType lt)
{
  UseVar * use = new UseVar(cell, pos, mmt, lt);
  putProperty(BasicVar, cell, use);
  ei->insert_use(cell);
}

void ExpressionInfoAnnotationMap::make_def_var(ExpressionInfo * ei,
					       SEXP cell,
					       DefVar::SourceT source,
					       BasicVar::MayMustT mmt,
					       Locality::LocalityType lt,
					       SEXP rhs_c)
{
  DefVar * def = new DefVar(cell, source, mmt, lt, rhs_c);
  putProperty(BasicVar, cell, def);
  ei->insert_def(cell);
}

ExpressionInfoAnnotationMap * ExpressionInfoAnnotationMap::s_instance = 0;
PropertyHndlT ExpressionInfoAnnotationMap::s_handle = "ExpressionInfo";

}
