// -*- Mode: C++ -*-
//
// Copyright (c) 2009 Rice University
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

// File: ReturnedDFSolver.h

// Author: John Garvin (garvin@cs.rice.edu)

#ifndef RETURNED_DF_SOLVER_H
#define RETURNED_DF_SOLVER_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

#include <analysis/ExpressionDFSet.h>
#include <analysis/VarRefFactory.h>

class OA::CFG::CFGInterface;
namespace OA { namespace DataFlow { class CFGDFSolver; } }
class R_IRInterface;
namespace RAnnot { class FuncInfo; }

class ReturnedDFSolver : private OA::DataFlow::CFGDFProblem {
public:
  explicit ReturnedDFSolver(OA::OA_ptr<R_IRInterface> rir);
  ~ReturnedDFSolver();
  OA::OA_ptr<ExpressionDFSet> perform_analysis(OA::ProcHandle proc,
					     OA::OA_ptr<OA::CFG::CFGInterface> cfg);
  OA::OA_ptr<ExpressionDFSet> perform_analysis(OA::ProcHandle proc,
					     OA::OA_ptr<OA::CFG::CFGInterface> cfg,
					     OA::OA_ptr<ExpressionDFSet> in_set);

  // ----- debugging -----
  void dump_node_maps();
  void dump_node_maps(std::ostream &os);

  static bool returned_predicate(SEXP call, int arg);
  
private:
  // ----- callbacks for CFGDFSolver -----
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeTop();
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeBottom();
  

  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeNodeIN(OA::OA_ptr<OA::CFG::NodeInterface> n);
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeNodeOUT(OA::OA_ptr<OA::CFG::NodeInterface> n);

  //! OK to modify set1 and return it as result, because solver
  //! only passes a tempSet in as set1
  OA::OA_ptr<OA::DataFlow::DataFlowSet> meet(OA::OA_ptr<OA::DataFlow::DataFlowSet> set1, 
					     OA::OA_ptr<OA::DataFlow::DataFlowSet> set2);

  /// intraprocedural
  OA::OA_ptr<OA::DataFlow::DataFlowSet> transfer(OA::OA_ptr<OA::DataFlow::DataFlowSet> in, 
						 OA::StmtHandle stmt);

  OA::OA_ptr<ExpressionDFSet> ret(SEXP cell, bool b, OA::OA_ptr<ExpressionDFSet> c);
  OA::OA_ptr<ExpressionDFSet> ret_curly_list(SEXP cell, bool b, OA::OA_ptr<ExpressionDFSet> c);
  OA::OA_ptr<ExpressionDFSet> make_universal_set();
  OA::OA_ptr<ExpressionDFSet> conservative_call(SEXP e, OA::OA_ptr<ExpressionDFSet> in);


private:
  OA::OA_ptr<R_IRInterface> m_ir;
  OA::OA_ptr<OA::CFG::CFGInterface> m_cfg;
  OA::ProcHandle m_proc;
  OA::OA_ptr<ExpressionDFSet> m_top;
  OA::OA_ptr<ExpressionDFSet> m_in;
  OA::OA_ptr<OA::DataFlow::CFGDFSolver> m_solver;
  VarRefFactory * const m_fact;
  RAnnot::FuncInfo * m_func_info;
};

#endif
