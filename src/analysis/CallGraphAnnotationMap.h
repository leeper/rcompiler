// -*- Mode: C++ -*-

#ifndef CALL_GRAPH_ANNOTATION_MAP_H
#define CALL_GRAPH_ANNOTATION_MAP_H

// Represents the call graph of the program. To represent R's binding
// semantics, a call graph node can be a function definition
// ("fundef") or a coordinate (a name in a scope). This system
// reflects the fact that a call site in R can refer to more than one
// coordinate, and a coordinate can refer to more than one fundef
// (i.e., a name in a scope can be bound to more than one function).

#include <map>
#include <set>
#include <ostream>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/AnnotationMap.h>

class CallGraphEdge;
class CallGraphNode;
class CallGraphInfo;
class CallSiteCallGraphNode;
class CoordinateCallGraphNode;
class FundefCallGraphNode;
class LibraryCallGraphNode;

namespace RAnnot {

class CallGraphAnnotationMap : public AnnotationMap
{
public:
  virtual ~CallGraphAnnotationMap();

  // demand-driven analysis
  MyMappedT & operator[](const MyKeyT & k); // FIXME: remove this when refactoring is done
  MyMappedT get(const MyKeyT & k);
  bool is_computed();

  // singleton
  static CallGraphAnnotationMap * get_instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

  // iterators
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  void dump(std::ostream & os);

private:
  // ----- implement singleton pattern -----

  // private constructor for singleton pattern
  CallGraphAnnotationMap();

  void compute();

  static CallGraphAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
  static void create();

  // ----- creation methods -----
  //
  // return different derived classes of CallGraphNode. Create new if
  // needed; otherwise return existing node in a map.
  //
  FundefCallGraphNode * make_fundef_node(SEXP e);
  LibraryCallGraphNode * make_library_node(SEXP name, SEXP value);
  CoordinateCallGraphNode * make_coordinate_node(SEXP name, SEXP scope);
  CallSiteCallGraphNode * make_call_site_node(SEXP e);

  void add_edge(const CallGraphNode * const source, const CallGraphNode * const sink);
  
private:
  bool m_computed; // has our information been computed yet?
  std::map<const CallGraphNode *, CallGraphInfo *> m_node_map;
  std::set<const CallGraphEdge *> m_edge_set;

  std::map<MyKeyT, MyMappedT> m_traversed_map; // stores info on problems we've solved

  std::map<SEXP, FundefCallGraphNode *> m_fundef_map;
  std::map<SEXP, LibraryCallGraphNode *> m_library_map;
  std::map<std::pair<SEXP,SEXP>, CoordinateCallGraphNode *> m_coord_map;
  std::map<SEXP, CallSiteCallGraphNode *> m_call_site_map;
};

}  // end namespace RAnnot

#endif
