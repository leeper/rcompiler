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

// File: StrictnessDFSolver.h
//
// Implements the OpenAnalysis CFG data flow problem interface for a
// function to determine whether a function is strict in each of its
// formal arguments. (A function is strict in argument v if there is a
// use of v on all paths.)
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef STRICTNESS_DF_SOLVER_H
#define STRICTNESS_DF_SOLVER_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

class OA::CFG::Interface;
class R_IRInterface;
class DefaultDFSet;

class StrictnessDFSolver : private OA::DataFlow::CFGDFProblem {
public:
  StrictnessDFSolver(OA::OA_ptr<R_IRInterface> _rir);
  ~StrictnessDFSolver();
  OA::OA_ptr<DefaultDFSet> perform_analysis(OA::ProcHandle proc, OA::OA_ptr<OA::CFG::Interface> cfg);
  void dump_node_maps();
  void dump_node_maps(ostream &os);

  // ----- callbacks for CFGDFProblem: initialization, meet, transfer -----
private:
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeTop();
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeBottom();

  void initializeNode(OA::OA_ptr<OA::CFG::Interface::Node> n);

  // CFGDFProblem says: OK to modify set1 and return it as result, because solver
  // only passes a tempSet in as set1
  OA::OA_ptr<OA::DataFlow::DataFlowSet>
  meet (OA::OA_ptr<OA::DataFlow::DataFlowSet> set1, OA::OA_ptr<OA::DataFlow::DataFlowSet> set2);

  // CFGDFProblem says: OK to modify in set and return it again as result because
  // solver clones the BB in sets
  OA::OA_ptr<OA::DataFlow::DataFlowSet> 
  transfer(OA::OA_ptr<OA::DataFlow::DataFlowSet> in, OA::StmtHandle stmt); 
  
private:
  OA::OA_ptr<R_IRInterface> m_ir;
  OA::OA_ptr<OA::CFG::Interface> m_cfg;
  OA::ProcHandle m_proc;
  OA::OA_ptr<DefaultDFSet> m_formal_args;
};

#endif // STRICTNESS_DF_SOLVER_H