#include <string>
#include <map>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_vector(SEXP vec) {
  int len = Rf_length(vec);
  switch(TYPEOF(vec)) {
  case LGLSXP:
    if (len == 1) {
      int value = INTEGER(vec)[0];
      map<int,string>::iterator pr = ParseInfo::sc_logical_map.find(value);
      if (pr == ParseInfo::sc_logical_map.end()) {  // not found
	string var = ParseInfo::global_constants->appl1("ScalarLogical",
					    i_to_s(value));
	ParseInfo::sc_logical_map.insert(pair<int,string>(value, var));
	return Expression(var, FALSE, VISIBLE, "");
      } else {
	return Expression(pr->second, FALSE, VISIBLE, "");
      }
    } else {
      ParseInfo::flag_problem();
      return Expression("<<unimplemented logical vector>>",
			FALSE, INVISIBLE, "");
    }
    break;
  case INTSXP:
    if (len == 1) {
      int value = INTEGER(vec)[0];
      map<int,string>::iterator pr = ParseInfo::sc_integer_map.find(value);
      if (pr == ParseInfo::sc_integer_map.end()) {  // not found
	string var = ParseInfo::global_constants->appl1("ScalarInteger",
					    i_to_s(value));
	ParseInfo::sc_integer_map.insert(pair<int,string>(value, var));
	return Expression(var, FALSE, VISIBLE, "");
      } else {
	return Expression(pr->second, FALSE, VISIBLE, "");
      }
    } else {
      ParseInfo::flag_problem();
      return Expression("<<unimplemented integer vector>>",
			FALSE, INVISIBLE, "");
    }
    break;
  case REALSXP:
    if (len == 1) {
      double value = REAL(vec)[0];
      map<double,string>::iterator pr = ParseInfo::sc_real_map.find(value);
      if (pr == ParseInfo::sc_real_map.end()) {  // not found
	string var = ParseInfo::global_constants->appl1("ScalarReal",
					    d_to_s(value));
	ParseInfo::sc_real_map.insert(pair<double,string>(value, var));
	return Expression(var, FALSE, VISIBLE, "");
      } else {
	return Expression(pr->second, FALSE, VISIBLE, "");
      }
    } else {
      ParseInfo::flag_problem();
      return Expression("<<unimplemented real vector>>",
			FALSE, INVISIBLE, "");
    }
    break;
  case CPLXSXP:
    if (len == 1) {
      Rcomplex value = COMPLEX(vec)[0];
      string var = ParseInfo::global_constants->appl1("ScalarComplex",
					  c_to_s(value));
      return Expression(var, FALSE, VISIBLE, "");
    } else {
      ParseInfo::flag_problem();
      return Expression("<<unimplemented complex vector>>",
			FALSE, INVISIBLE, "");
    }
    break;
  default:
    err("Internal error: op_vector encountered bad vector type\n");
    return Expression::bogus_exp; // not reached
    break;
  }
}
