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

// File: VarRefSet.h
//
// Set of VarRefs and iterator for VarRefs.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef VAR_REF_SET_H
#define VAR_REF_SET_H

#include <set>
#include <OpenAnalysis/Utils/OA_ptr.hpp>

#include "VarRef.h"

class R_VarRefSetIterator {
public:
  R_VarRefSetIterator(OA::OA_ptr<std::set<OA::OA_ptr<R_VarRef> > > _vars) : vars(_vars) {
    assert(!vars.ptrEqual(NULL));
    it = vars->begin();
  }
  OA::OA_ptr<R_VarRef> current() const;
  bool isValid() const;
  void operator++();
  void reset();

private:
  OA::OA_ptr<std::set<OA::OA_ptr<R_VarRef> > > vars;
  std::set<OA::OA_ptr<R_VarRef> >::iterator it;
};


/// A set of R_VarRefs.
class R_VarRefSet {
 public:
  R_VarRefSet() { vars = new std::set<OA::OA_ptr<R_VarRef> >; }
  void insert_ref(OA::OA_ptr<R_BodyVarRef> var);
  void insert_arg(OA::OA_ptr<R_ArgVarRef> var);
  void set_union(const R_VarRefSet & set2);
  void set_union(OA::OA_ptr<R_VarRefSet> set2);
  OA::OA_ptr<R_VarRefSetIterator> get_iterator() const;
  void dump();

  /// construct a VarRefSet from a list of formal arguments
  static OA::OA_ptr<R_VarRefSet> refs_from_arglist(SEXP arglist);

protected:
  friend class VarRefSetIterator;

  OA::OA_ptr<std::set<OA::OA_ptr<R_VarRef> > > vars;
};

#endif
