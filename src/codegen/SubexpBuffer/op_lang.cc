#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnalysisResults.h>
#include <analysis/CallGraphAnnotation.h>
#include <analysis/CallGraphNode.h>
#include <analysis/LibraryCallGraphNode.h>
#include <analysis/FundefCallGraphNode.h>
#include <analysis/UnknownValueCallGraphNode.h>
#include <analysis/HandleInterface.h>
#include <analysis/VarBinding.h>
#include <support/StringUtils.h>
#include <support/RccError.h>
#include <Visibility.h>
#include <ParseInfo.h>
#include <CodeGen.h>

using namespace std;
using namespace RAnnot;

Expression SubexpBuffer::op_lang(SEXP e, string rho, 
	   Protection resultProtection,
	   ResultStatus resultStatus)
{
  SEXP op;
  string out;
  Expression exp;
  SEXP lhs = CAR(e);
  SEXP r_args = CDR(e);
  if (TYPEOF(lhs) == SYMSXP) {
    string r_sym = CHAR(PRINTNAME(lhs));
    SEXP op = Rf_findFunUnboundOK(lhs, R_GlobalEnv, TRUE);
    if (TYPEOF(op) == SPECIALSXP) {
      return op_special(e, op, rho, resultProtection, resultStatus);
    }
    // see if call graph supplies a single definition
    CallGraphAnnotationMap * amap = CallGraphAnnotationMap::get_instance();
    // TODO: parametrize CallGraphAnnotationMap to avoid cast
    CallGraphAnnotation * ann = dynamic_cast<CallGraphAnnotation*>(amap->get(HandleInterface::make_mem_ref_h(e)));
    const CallGraphAnnotationMap::CallGraphNode * node = ann->get_singleton_if_exists();
    if (node) {
      // node is Fundef, Library, or UnknownValue node
      if (const CallGraphAnnotationMap::FundefCallGraphNode * cs = dynamic_cast<const CallGraphAnnotationMap::FundefCallGraphNode *>(node)) {
	string closure = getProperty(FuncInfo, cs->get_sexp())->get_closure();
	Expression closure_exp = Expression(closure, false, INVISIBLE, "");
	return op_clos_app(closure_exp, r_args, rho, resultProtection);
      } else if (const CallGraphAnnotationMap::LibraryCallGraphNode * lib = dynamic_cast<const CallGraphAnnotationMap::LibraryCallGraphNode *>(node)) {
	// it's from the R environment
	const SEXP op = lib->get_value();
	if (TYPEOF(op) == CLOSXP) {
	  Expression func = op_closure(op, rho, resultProtection);
	  return op_clos_app(func, r_args, rho, resultProtection);
	} else if (TYPEOF(op) == BUILTINSXP) {
	  return op_builtin(e, op, rho, resultProtection);
	} else {
	  rcc_error("Internal error: LANGSXP encountered non-function op");
	  return Expression::bogus_exp; // never reached
	}
      } else if (const CallGraphAnnotationMap::UnknownValueCallGraphNode * uv = dynamic_cast<const CallGraphAnnotationMap::UnknownValueCallGraphNode *>(node)) {
	Expression func = op_fun_use(e, rho);
	return op_clos_app(func, r_args, rho, resultProtection);
      } else {
	rcc_error("Internal error: in call graph, didn't find expected Fundef, Library, or UnknownValue node");
      }
    } else { // more than one possible function value; let op_fun_use decide
      Expression func = op_fun_use(e, rho);
      return op_clos_app(func, r_args, rho, resultProtection);
    }
  } else {  // left side is not a symbol
    // generate closure and application
    Expression op1;
    op1 = op_exp(e, rho, Unprotected);  // evaluate LHS
    return op_clos_app(op1, CDR(e), rho, resultProtection);
    // eval.c: 395
  }
}
