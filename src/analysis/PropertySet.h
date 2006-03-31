// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySet.h,v 1.6 2006/03/31 16:37:27 garvin Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/PropertySet.h,v $
//
// Purpose:
//   [The purpose of this file]
//
// Description:
//   [The set of functions, macros, etc. defined in the file]
//
//***************************************************************************

//************************* System Include Files ****************************

#include <iostream>
#include <map>

//**************************** R Include Files ******************************

//*************************** User Include Files ****************************

#include "PropertyHndl.h"
#include "AnnotationMap.h"

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RProp {

//****************************************************************************
// R Property Information
//****************************************************************************

// ---------------------------------------------------------------------------
// PropertySet: Associates arbitrary property names with
// AnnotationMaps.  For example, a property might be the result of an
// analysis pass over the R AST.
// ---------------------------------------------------------------------------
class PropertySet
  : public std::map<PropertyHndlT, RAnnot::AnnotationMap*>
{
public:
  // -------------------------------------------------------
  // constructor/destructor
  // -------------------------------------------------------
  PropertySet();
  ~PropertySet();

  void insert(PropertyHndlT propertyName, SEXP s,
	      RAnnot::AnnotationBase *annot, bool ownsAnnotations);

  RAnnot::AnnotationBase *lookup(PropertyHndlT propertyName, SEXP s);

  // -------------------------------------------------------
  // cloning (proscribe by hiding copy constructor and operator=)
  // -------------------------------------------------------
  
  // -------------------------------------------------------
  // iterator, find/insert, etc 
  // -------------------------------------------------------
  // use inherited std::map routines

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  std::ostream& dumpCout() const;
  std::ostream& dump(std::ostream& os) const;

private:
  PropertySet(const PropertySet& x);
  PropertySet& operator=(const PropertySet& x) { return *this; }

private:
};


//****************************************************************************

} // end of RProp namespace
