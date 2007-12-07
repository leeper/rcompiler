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

// File: VarBindingAnnotationMap.h
//
// Maps each variable to a VarBinding that describes its binding
// scopes. Owns the values in its map and must delete them in destructor.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <vector>

#include <support/RccError.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/PropertySet.h>
#include <analysis/SymbolTable.h>
#include <analysis/Utils.h>
#include <analysis/Var.h>
#include <analysis/VarBinding.h>
#include <analysis/VarBindingAnnotationMap.h>
#include <analysis/VarInfo.h>

#include "VarBindingAnnotationMap.h"

using namespace RAnnot;
using namespace HandleInterface;

// ----- forward declarations of file-scope functions

static bool defined_local_in_scope(Var * v, FuncInfo * s);

namespace RAnnot {

// ----- typedefs for readability -----

typedef VarBindingAnnotationMap::MyKeyT MyKeyT;
typedef VarBindingAnnotationMap::MyMappedT MyMappedT;
typedef VarBindingAnnotationMap::iterator iterator;
typedef VarBindingAnnotationMap::const_iterator const_iterator;

// ----- constructor/destructor -----

VarBindingAnnotationMap::VarBindingAnnotationMap() {}

VarBindingAnnotationMap::~VarBindingAnnotationMap() {
  delete_map_values();
}

// ----- singleton pattern -----

VarBindingAnnotationMap * VarBindingAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT VarBindingAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

void VarBindingAnnotationMap::create() {
  m_instance = new VarBindingAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

VarBindingAnnotationMap * VarBindingAnnotationMap::m_instance = 0;
PropertyHndlT VarBindingAnnotationMap::m_handle = "VarBinding";

// ----- computation -----

/// Each Var is bound in some subset of the sequence of ancestor
/// scopes of its containing function. Compute the subsequence of
/// scopes in which each Var is bound and add the (Var,sequence) pair
/// into the annotation map.
void VarBindingAnnotationMap::compute() {
  create_var_bindings();
  populate_symbol_tables();
}
  
void VarBindingAnnotationMap::create_var_bindings() {
  FuncInfo * global = R_Analyst::get_instance()->get_scope_tree_root();
  // for each function
  FuncInfoIterator fii(global);
  for( ; fii.IsValid(); ++fii) {
    FuncInfo * fi = fii.Current();
    assert(fi != 0);

    // first, create VarBindings for formal args; each one has just
    // one scope, which is the current procedure
    for(int i = 1; i <= fi->get_num_args(); i++) {
      Var * var = getProperty(Var, fi->get_arg(i));
      VarBinding * binding = new VarBinding();
      binding->insert(fi->get_scope());
      get_map()[fi->get_arg(i)] = binding;
    }
    
    // now create bindings for mentions in the function body
    // for each mention
    FuncInfo::mention_iterator mi;
    for (mi = fi->begin_mentions(); mi != fi->end_mentions(); ++mi) {
      Var * v = *mi;
      v = getProperty(Var, v->getMention_c());
      // TODO: should make sure we always get the data-flow-solved
      // version of the Var. Shouldn't have to loop through
      // getProperty!
      VarBinding * scopes = new VarBinding();
      switch(v->getScopeType()) {
      case Locality_LOCAL:
	// bound in current scope only
	scopes->insert(fi->get_scope());
	break;
      case Locality_BOTTOM:
	// bound in current scope and one or more ancestors
	scopes->insert(fi->get_scope());
	// FALLTHROUGH
      case Locality_FREE:
	// start at this scope's parent; iterate upward through ancestors
	for(FuncInfo * a = fi->Parent(); a != 0; a = a->Parent()) {
	  if (defined_local_in_scope(v,a)) {
	    scopes->insert(a->get_scope());
	  }
	}
	
	// for R internal names, add the library scope
	if (is_library(CAR(v->getMention_c()))) {
	  scopes->insert(R_Analyst::get_instance()->get_library_scope());
	}
	
	// double-arrow (<<-) definitions declare the name in the
	// global scope if it's not local in some middle scope. Just
	// check for a def; since we're here, we already know the
	// mention is non-local.
	//
	// TODO: in the latest version of R, this is no longer the
	// case.
	if (v->getUseDefType() == Var::Var_DEF && !scopes->is_global()) {
	  scopes->insert(R_Analyst::get_instance()->get_global_scope());
	}

	// record unbound names
	if (scopes->begin() == scopes->end()) {
	  scopes->insert(UnboundLexicalScope::get_instance());
	}

	break;
      default:
	rcc_error("Unknown locality type encountered");
	break;
      }
      // whether global or not...
      get_map()[v->getMention_c()] = scopes;
    }  // next mention
  }  // next function
}

void VarBindingAnnotationMap::populate_symbol_tables() {
  // for each (mention, VarBinding) pair in our map
  std::map<MyKeyT, MyMappedT>::const_iterator iter;
  for(iter = begin(); iter != end(); ++iter) {
    // TODO: refactor AnnotationMaps to avoid downcasting
    VarBinding * vb = dynamic_cast<VarBinding *>(iter->second);
    Var * var = getProperty(Var, iter->first);
    SEXP name = var->getName();
    // for each scope in the VarBinding
    VarBinding::const_iterator scope_iter;
    for(scope_iter = vb->begin(); scope_iter != vb->end(); ++scope_iter) {
      LexicalScope * scope = *scope_iter;
      // in the scope's symbol table, associate the name with a VarInfo
      SymbolTable * st = scope->get_symbol_table();
      VarInfo * vi = st->find_or_create(name, scope);
      // each VarInfo has a list of definitions. If this mention is a
      // def, add it to the appropriate VarInfo if it's not there
      // already.
      if(DefVar * def = dynamic_cast<DefVar *>(var)) {
	vi->insert_def(def);
      }
    }
  }
}
  
}  // end namespace RAnnot

/// Is the given variable defined as local in the given scope?
static bool defined_local_in_scope(Var * v, FuncInfo * s) {

  // look at formal args
  for(int i = 1; i <= s->get_num_args(); i++) { // args are indexed from 1
    Var * formal = getProperty(Var, s->get_arg(i));
    // all formal args are local and def
    if (formal->getName() == v->getName()) {
      return true;
    }
  }

  // look at mentions
  FuncInfo::mention_iterator mi;
  for(mi = s->begin_mentions(); mi != s->end_mentions(); ++mi) {
    Var * m = *mi;
    if (m->getUseDefType() == Var::Var_DEF  &&
	m->getScopeType() == Locality_LOCAL &&
	m->getName() == v->getName())
    {
      return true;
    }
  }
  return false;
}
