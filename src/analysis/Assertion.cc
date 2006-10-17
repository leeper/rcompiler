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

// File: Assertion.cc
//
// Processing compiler assertions. This is the way a programmer can
// assert that certain conditions hold, which may improve compilation.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/Utils.h>
#include <analysis/AnalysisResults.h>
#include <analysis/FormalArgInfo.h>

#include <ParseInfo.h>

#include "Assertion.h"

namespace RAnnot {

void process_assert(SEXP assertion, FuncInfo * fi) {
  if (is_rcc_assert(assertion)) {
    // .rcc.assert
    SEXP list = CDR(assertion);
    for (; list != R_NilValue; list = CDR(list)) {
      SEXP e = CAR(list);
      if (is_var(e)) {
	// assertion of the form:
	// .rcc.assert(foo)
	if (e == Rf_install("no.oo")) {
	  ParseInfo::set_allow_oo(false);
	} else if (e == Rf_install("no.envir.manip")) {
	  ParseInfo::set_allow_envir_manip(false);
	} else if (e == Rf_install("no.special.redef")) {
	  ParseInfo::set_allow_special_redef(false);
	} else if (e == Rf_install("no.builtin.redef")) {
	  ParseInfo::set_allow_builtin_redef(false);
	} else if (e == Rf_install("no.library.redef")) {
	  ParseInfo::set_allow_library_redef(false);
	}
	// NOTE: our analysis now handles user redefinition of
	// builtin, and library functions, so these assertions have no
	// effect.
      } else if (is_value_assert(e)) {
	// assertion that a formal argument is call-by-value, of the form:
	// .rcc.assert(value(foo))
	SEXP v = CADR(e);
	char* vname = CHAR(PRINTNAME(v));
	int position = fi->find_arg_position(vname);
	SEXP arg = fi->get_arg(position);
	FormalArgInfo* fargInfo = getProperty(FormalArgInfo, arg);
	fargInfo->setIsValue(true);
      }
    }
  } else if (is_rcc_assert_exp(assertion)) {
    // .rcc.assert.exp
    SEXP body_of_e = CDR(assertion);
    if (is_simple_assign(CDR(assertion))) // no attribute given in the assertion
      ;
    else {  // see if the attribute is "unique_definiton"
      SEXP body_of_e = CDR(assertion);
      SEXP header = CAR(body_of_e); // is_simple_assign(header) is true 
      SEXP attrib = CADR(body_of_e); 
      char *funcname;
      if (is_simple_assign(header)) { // get function name  
	funcname = CHAR(PRINTNAME(CADR(header)));
	if (attrib == Rf_install("unique_definition"))  {
	  func_unique_defintion.insert(std::make_pair(funcname,fi)); 
	}
      }
    }
  } else if (is_rcc_assert_sym(assertion)) {
    // .rcc.assert.sym
  }
}
  
}
