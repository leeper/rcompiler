#include <list>

#include <support/DumpMacros.h>
#include <support/RccError.h>
#include <support/StringUtils.h>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/AnnotationBase.h>
#include <analysis/CallGraphAnnotation.h>
#include <analysis/CallGraphEdge.h>
#include <analysis/CallGraphNode.h>
#include <analysis/CallGraphInfo.h>
#include <analysis/FundefCallGraphNode.h>
#include <analysis/LibraryCallGraphNode.h>
#include <analysis/CoordinateCallGraphNode.h>
#include <analysis/CallSiteCallGraphNode.h>
#include <analysis/HandleInterface.h>
#include <analysis/SymbolTable.h>
#include <analysis/VarBinding.h>
#include <analysis/VarInfo.h>

#include "CallGraphAnnotationMap.h"

namespace RAnnot {

// ----- type definitions for readability -----
  
typedef CallGraphAnnotationMap::MyKeyT MyKeyT;
typedef CallGraphAnnotationMap::MyMappedT MyMappedT;
typedef CallGraphAnnotationMap::iterator iterator;
typedef CallGraphAnnotationMap::const_iterator const_iterator;

typedef CallGraphAnnotationMap::NodeListT NodeListT;
typedef CallGraphAnnotationMap::NodeSetT NodeSetT;
typedef CallGraphAnnotationMap::NodeMapT NodeMapT;
typedef CallGraphAnnotationMap::EdgeSetT EdgeSetT;

typedef CallGraphAnnotationMap::FundefCallGraphNode FundefCallGraphNode;
typedef CallGraphAnnotationMap::CallSiteCallGraphNode CallSiteCallGraphNode;
typedef CallGraphAnnotationMap::CoordinateCallGraphNode CoordinateCallGraphNode;
typedef CallGraphAnnotationMap::LibraryCallGraphNode LibraryCallGraphNode;

// ----- constructor, destructor -----

CallGraphAnnotationMap::CallGraphAnnotationMap()
  : m_computed(false), m_node_map(), m_edge_set(), m_traversed_map(),
    m_fundef_map(), m_library_map(), m_coord_map(), m_call_site_map()
{}
  
CallGraphAnnotationMap::~CallGraphAnnotationMap() {
  NodeMapT::const_iterator i;
  
  // delete CallGraphInfo objects
  for(i = m_node_map.begin(); i != m_node_map.end(); ++i) {
    delete(i->second);
  }
  
  // delete edges
  EdgeSetT::const_iterator j;
  for(j = m_edge_set.begin(); j != m_edge_set.end(); ++j) {
    delete(*j);
  }
}  
  
// ----- singleton pattern -----

CallGraphAnnotationMap * CallGraphAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}
  
PropertyHndlT CallGraphAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}
  
void CallGraphAnnotationMap::create() {
  m_instance = new CallGraphAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}
  
CallGraphAnnotationMap * CallGraphAnnotationMap::m_instance = 0;
PropertyHndlT CallGraphAnnotationMap::m_handle = "CallGraph";
  
//  ----- demand-driven analysis ----- 

// Subscripting is here temporarily to allow PutProperty -->
// PropertySet::insert to work right.
// FIXME: delete this when fully refactored to disallow insertion from outside.
MyMappedT & CallGraphAnnotationMap::operator[](const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  return m_traversed_map[k];
}

// Perform the computation if necessary and returns the requested
// data.
MyMappedT CallGraphAnnotationMap::get(const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  // after computing, an annotation ought to exist for every valid
  // key. If not, it's an error
  std::map<MyKeyT, MyMappedT>::const_iterator annot = m_traversed_map.find(k);
  if (annot == m_traversed_map.end()) {
    rcc_error("Possible invalid key not found in CallGraph map");
  }

  return annot->second;
}
  
bool CallGraphAnnotationMap::is_computed() {
  return m_computed;
}

//  ----- iterators ----- 

iterator CallGraphAnnotationMap::begin() { return m_traversed_map.begin(); }
iterator CallGraphAnnotationMap::end() { return m_traversed_map.end(); }
const_iterator CallGraphAnnotationMap::begin() const { return m_traversed_map.begin(); }
const_iterator CallGraphAnnotationMap::end() const { return m_traversed_map.end(); }

// ----- creation -----

FundefCallGraphNode * CallGraphAnnotationMap::make_fundef_node(SEXP e) {
  assert(is_fundef(e));
  FundefCallGraphNode * node;
  std::map<SEXP, FundefCallGraphNode *>::const_iterator it = m_fundef_map.find(e);
  if (it == m_fundef_map.end()) {  // not yet in map
    node = new FundefCallGraphNode(e);
    m_node_map[node] = new CallGraphInfo();
    m_fundef_map[e] = node;
  } else {
    node = it->second;
  }
  return node;
}

LibraryCallGraphNode * CallGraphAnnotationMap::make_library_node(SEXP name, SEXP value) {
  assert(is_var(name));
  assert(TYPEOF(value) == CLOSXP ||
	 TYPEOF(value) == BUILTINSXP ||
	 TYPEOF(value) == SPECIALSXP);
  LibraryCallGraphNode * node;
  std::map<SEXP, LibraryCallGraphNode *>::const_iterator it = m_library_map.find(name);
  if (it == m_library_map.end()) {  // not yet in map
    node = new LibraryCallGraphNode(name, value);
    m_node_map[node] = new CallGraphInfo();
    m_library_map[name] = node;
  } else {
    node = it->second;
  }
  return node;
}

CoordinateCallGraphNode * CallGraphAnnotationMap::make_coordinate_node(SEXP name, SEXP scope) {
  assert(is_var(name));
  assert(is_fundef(scope));
  CoordinateCallGraphNode * node;
  std::map<std::pair<SEXP, SEXP>, CoordinateCallGraphNode *>::const_iterator it;
  it = m_coord_map.find(std::make_pair(name, scope));
  if (it == m_coord_map.end()) {
    node = new CoordinateCallGraphNode(name, scope);
    m_node_map[node] = new CallGraphInfo();
    m_coord_map[std::make_pair(name, scope)] = node;
  } else {
    node = it->second;
  }
  return node;
}

CallSiteCallGraphNode * CallGraphAnnotationMap::make_call_site_node(SEXP e) {
  assert(is_call(e));
  CallSiteCallGraphNode * node;
  std::map<SEXP, CallSiteCallGraphNode *>::const_iterator it = m_call_site_map.find(e);
  if (it == m_call_site_map.end()) {  // not yet in map
    node = new CallSiteCallGraphNode(e);
    m_node_map[node] = new CallGraphInfo();
    m_call_site_map[e] = node;
  } else {
    node = it->second;
  }
  return node; 
}

void CallGraphAnnotationMap::add_edge(const CallGraphNode * const source, const CallGraphNode * const sink) {
  CallGraphEdge * edge = new CallGraphEdge(source, sink);
  m_edge_set.insert(edge);
  CallGraphInfo * source_info = m_node_map[source];
  CallGraphInfo * sink_info = m_node_map[sink];
  assert(source_info != 0);
  assert(sink_info != 0);
  source_info->insert_call_out(edge);
  sink_info->insert_call_in(edge);
}

// ----- computation -----  

MyMappedT CallGraphAnnotationMap::get_call_bindings(MyKeyT cs) {
  std::map<MyKeyT, MyMappedT>::const_iterator it = m_traversed_map.find(cs);
  if (it != m_traversed_map.end()) {  // already computed and stored in map
    return it->second;
  } else {
    SEXP r_cs = HandleInterface::make_sexp(cs);
    const CallSiteCallGraphNode * const cs_node = make_call_site_node(r_cs);
    
    // search graph, accumulate fundefs/library functions
    NodeListT worklist;
    NodeSetT visited;
    
    // new annotation
    CallGraphAnnotation * ann = new CallGraphAnnotation();
    
    // start with worklist containing the given call site
    worklist.push_back(cs_node);
    
    while(!worklist.empty()) {
      const CallGraphNode * c = worklist.front();
      worklist.pop_front();
      if (visited.find(c) == visited.end()) {
	visited.insert(c);
	c->get_call_bindings(worklist, visited, ann);
      }
    }
    return ann;
  }
}

void CallGraphAnnotationMap::compute() {
  NodeListT worklist;
  NodeSetT visited;

  // start with worklist containing one entry, the whole program
  SEXP program = CAR(assign_rhs_c(R_Analyst::get_instance()->get_program()));
  FundefCallGraphNode * root_node = make_fundef_node(program);
  worklist.push_back(root_node);

  while(!worklist.empty()) {
    const CallGraphNode * c = worklist.front();
    worklist.pop_front();
    if (visited.find(c) == visited.end()) {
      visited.insert(c);
      c->compute(worklist, visited);
    }
  }

  //  for each procedure entry point (just the big proc if one executable):
  //    add to worklist as new call graph node
  //  for each node in worklist:
  //    mark node visited
  //    if it's a Fundef:
  //      for each call site:
  //        add edge (fundef, call site)
  //        if call site node hasn't been visited, add to worklist
  //    else if worklist item is a CallSite:
  //      if LHS is a simple name:
  //        for each scope in VarBinding:
  //          add name, scope as new coordinate node if not already present
  //          add edge (call site, coordinate)
  //          if coordinate node hasn't been visited, add to worklist
  //      else [LHS is not a simple name]:
  //        add edge (call site, any-coordinate)
  //    else if worklist item is a Coordinate:
  //      look up name, scope in SymbolTable, get list of defs
  //      for each def:
  //        if RHS is a fundef:
  //          add fundef as fundef node if not already present
  //          add edge (Coordinate, fundef)
  //          if fundef hasn't been visited, add to worklist
  //        else:
  //          add edge (Coordinate, any-fundef) NOTE: look up DSystem's name
}

void CallGraphAnnotationMap::dump(std::ostream & os) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  beginObjDump(os, CallGraph);

  NodeMapT::const_iterator node_it;
  CallGraphInfo::const_iterator edge_it;
  const CallGraphEdge * edge;
  for(node_it = m_node_map.begin(); node_it != m_node_map.end(); ++node_it) {
    const CallGraphNode * node = node_it->first;
    assert(node != 0);
    dumpObj(os, node);
    CallGraphInfo * const info = node_it->second;
    assert(info != 0);
    info->dump(os);
    os << std::endl;
  }

  endObjDump(os, CallGraph);
}

void CallGraphAnnotationMap::dumpdot(std::ostream & os) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }

  os << "digraph CallGraph" << " {" << std::endl;

  // dump nodes
  NodeMapT::const_iterator node_it;
  const CallGraphNode * node;
  unsigned int node_num;
  for(node_it = m_node_map.begin(); node_it != m_node_map.end(); ++node_it) {
    node = node_it->first;
    node_num = node->get_id();
    os << node_num << " [ label=\"" << node_num << "\\n";
    std::ostringstream ss;
    node->dump_string(ss);
    os << escape(ss.str());
    os << "\\n\" ];" << std::endl;
    os.flush();
  }

  // dump edges
  EdgeSetT::const_iterator edge_it;
  unsigned int source_num, sink_num;
  for(edge_it = m_edge_set.begin(); edge_it != m_edge_set.end(); ++edge_it) {
    source_num = (*edge_it)->get_source()->get_id();
    sink_num = (*edge_it)->get_sink()->get_id();
    os << source_num << " -> " << sink_num << ";" << std::endl;
    os.flush();
  }

  os << "}" << std::endl;
  os.flush();
}

} // end namespace RAnnot
