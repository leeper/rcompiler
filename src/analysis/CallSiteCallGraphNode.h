// -*- Mode: C++ -*-

#ifndef CALL_SITE_CALL_GRAPH_NODE_H
#define CALL_SITE_CALL_GRAPH_NODE_H

#include <ostream>

#include <include/R/R_RInternals.h>

#include <analysis/CallGraphNode.h>

namespace RAnnot {

class CallGraphAnnotationMap::CallSiteCallGraphNode : public CallGraphNode {
public:
  explicit CallSiteCallGraphNode(const SEXP def);
  virtual ~CallSiteCallGraphNode();

  const OA::IRHandle get_handle() const;
  const SEXP get_sexp() const;

  void compute(CallGraphAnnotationMap::NodeListT & worklist,
	       CallGraphAnnotationMap::NodeSetT & visited) const;

  void get_call_bindings(CallGraphAnnotationMap::NodeListT & worklist,
			 CallGraphAnnotationMap::NodeSetT & visited,
			 CallGraphAnnotation * ann) const;

  void dump(std::ostream & os) const;
  void dump_string(std::ostream & os) const;
private:
  const SEXP m_cs;
};

} // end namespace RAnnot

#endif