// -*- Mode: C++ -*-
//
// Copyright (c) 2007 Rice University
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

// File: OACallGraphAnnotationMap.h
//
// Represents the OpenAnalysis call graph of the program. Owns the
// values in its map, so they must be deleted in the destructor.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef OA_CALL_GRAPH_ANNOTATION_MAP_H
#define OA_CALL_GRAPH_ANNOTATION_MAP_H

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/CallGraph/CallGraphInterface.hpp>
#include <OpenAnalysis/Alias/InterAliasInterface.hpp>

#include <analysis/DefaultAnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace OA {
  namespace SideEffect {
    class InterSideEffectStandard;
  }
}

namespace RAnnot {

class OACallGraphAnnotationMap : public DefaultAnnotationMap
{
public:
  // ----- destructor -----
  virtual ~OACallGraphAnnotationMap();

  // ----- demand-driven analysis -----

  // overrides DefaultAnnotationMap::get
  virtual MyMappedT get(const MyKeyT & k);

  // ----- implement singleton pattern -----

  static OACallGraphAnnotationMap * instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

  // ----- debugging -----

  /// dump debugging information about the call graph
  void dump(std::ostream & os);

  /// dump the call graph in dot form
  void dumpdot(std::ostream & os);

  // ----- access to OA call graph and alias info -----
  OA::OA_ptr<OA::CallGraph::CallGraphInterface> get_OA_call_graph();
  OA::OA_ptr<OA::Alias::InterAliasInterface> get_OA_alias();

private:
  // ----- implement singleton pattern -----

  // private constructor for singleton pattern
  explicit OACallGraphAnnotationMap();

  static OACallGraphAnnotationMap * s_instance;
  static PropertyHndlT s_handle;
  static void create();
  
  // ----- compute call graph -----

  void compute();

private:
  OA::OA_ptr<OA::CallGraph::CallGraphInterface> m_call_graph;
  OA::OA_ptr<OA::SideEffect::InterSideEffectStandard> m_side_effect; // side effect information
  OA::OA_ptr<OA::Alias::InterAliasInterface> m_alias;                // alias information
};

}

#endif
