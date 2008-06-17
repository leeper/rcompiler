// -*- Mode: C++ -*-
//
// Copyright (c) 2008 Rice University
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

// File: SideEffectAnnotationMap.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "SideEffectAnnotationMap.h"

#include <OpenAnalysis/SideEffect/ManagerInterSideEffectStandard.hpp>
#include <OpenAnalysis/DataFlow/ManagerParamBindings.hpp>
#include <OpenAnalysis/Alias/ManagerInterAliasMapBasic.hpp>
#include <OpenAnalysis/Location/Location.hpp>
#include <OpenAnalysis/Location/NamedLoc.hpp>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/OACallGraphAnnotationMap.h>
#include <analysis/SideEffect.h>
#include <analysis/SimpleIterators.h>

#include <support/RccError.h>

using namespace OA;
using namespace HandleInterface;

namespace RAnnot {

//  ----- constructor/destructor ----- 
  
SideEffectAnnotationMap::SideEffectAnnotationMap()
{}
  
SideEffectAnnotationMap::~SideEffectAnnotationMap()
{}

// ----- singleton pattern -----

SideEffectAnnotationMap * SideEffectAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT SideEffectAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

void SideEffectAnnotationMap::create() {
  m_instance = new SideEffectAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

SideEffectAnnotationMap * SideEffectAnnotationMap::m_instance = 0;
PropertyHndlT SideEffectAnnotationMap::m_handle = "SideEffect";

// ----- computation -----

// compute all Var annotation information
void SideEffectAnnotationMap::compute() {
  FuncInfo * fi;
  OA_ptr<CFG::NodeInterface> node;
  StmtHandle stmt;

  compute_oa_side_effect();
  // now use m_side_effect to get info on expressions

  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_NODE(fi, node) {
      NODE_FOR_EACH_STATEMENT(node, stmt) {
	make_side_effect(fi, make_sexp(stmt));
      }  // next statement
    }  // next node

    PROC_FOR_EACH_CALL_SITE(fi, csi) {
      for(R_ListIterator arg_it(*csi); arg_it.isValid(); ++arg_it) {
	make_side_effect(fi, arg_it.current());
      }
    }
  }  // next function
}

void SideEffectAnnotationMap::compute_oa_side_effect() {
  // populate m_side_effect with OA side effect info

  OA_ptr<R_IRInterface> interface; interface = R_Analyst::get_instance()->get_interface();
  OA_ptr<CallGraph::CallGraphInterface> call_graph;
  call_graph = OACallGraphAnnotationMap::get_instance()->get_OA_call_graph();
  OA_ptr<Alias::InterAliasInterface> alias;
  alias = OACallGraphAnnotationMap::get_instance()->get_OA_alias();

  OA::SideEffect::ManagerInterSideEffectStandard solver(interface);
  // param bindings
  DataFlow::ManagerParamBindings pb_man(interface);
  OA_ptr<DataFlow::ParamBindings> param_bindings; param_bindings = pb_man.performAnalysis(call_graph);

  // intra side effect information
  OA_ptr<OA::SideEffect::ManagerSideEffectStandard> intra_man;
  intra_man = new OA::SideEffect::ManagerSideEffectStandard(interface);

  // compute side effect information
  m_side_effect = solver.performAnalysis(call_graph, param_bindings, alias, intra_man, DataFlow::ITERATIVE);
}

void SideEffectAnnotationMap::make_side_effect(const FuncInfo * const fi, const SEXP e) {
  SEXP cs;
  Var * var;
  ExpressionInfo * expr = getProperty(ExpressionInfo, e);
  SideEffect * annot = new SideEffect();

  // first grab local uses and defs

  EXPRESSION_FOR_EACH_MENTION(expr, var) {
    annot->insert_mention(fi, var);
  }

  // now grab interprocedural uses and defs from m_side_effect

  EXPRESSION_FOR_EACH_CALL_SITE(expr, cs) {
    OA_ptr<LocIterator> li;
    for(li = m_side_effect->getMODIterator(make_call_h(cs)); li->isValid(); ++(*li)) {
      OA_ptr<OA::Location> location; location = li->current();
      if (location->isaNamed()) {
	OA_ptr<NamedLoc> named_loc; named_loc = location.convert<NamedLoc>();
	annot->insert_def(named_loc);
      } else if (location->isaUnknown()) {
	annot->insert_def(location);
      } else {
	rcc_error("Unexpected location type");
      }
    }  // next MOD location
    for(li = m_side_effect->getREFIterator(make_call_h(cs)); li->isValid(); ++(*li)) {
      OA_ptr<OA::Location> location; location = li->current();
      if (location->isaNamed()) {
	OA_ptr<NamedLoc> named_loc; named_loc = location.convert<NamedLoc>();
	annot->insert_use(named_loc);
      } else if (location->isaUnknown()) {
	annot->insert_use(location);
      } else {
	rcc_error("Unexpected location type");
      }
    }  // next REF location
  }  // next call site in expression
  
  get_map()[e] = annot;
} 

} // end namespace RAnnot
