// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/AnnotationMap.h,v 1.2 2006/04/26 22:09:45 garvin Exp $

#ifndef ANNOTATION_MAP_H
#define ANNOTATION_MAP_H

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/AnnotationMap.h,v $
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

#include <include/R/R_RInternals.h>

//*************************** User Include Files ****************************

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <analysis/Annotation.h>

namespace RAnnot {

//****************************************************************************
// R Property Information
//****************************************************************************

// ---------------------------------------------------------------------------
// AnnotationMap: A mapping of IRHandles to Annotations.
// ---------------------------------------------------------------------------
class AnnotationMap
{  
public:
  // -------------------------------------------------------
  // type definitions
  // -------------------------------------------------------
  typedef OA::IRHandle MyKeyT;
  typedef RAnnot::AnnotationBase * MyMappedT;
  typedef std::map<MyKeyT, MyMappedT>::iterator iterator;
  typedef std::map<MyKeyT, MyMappedT>::const_iterator const_iterator;

  // -------------------------------------------------------
  // constructor/destructor
  // -------------------------------------------------------
  AnnotationMap();
  virtual ~AnnotationMap();

  // -------------------------------------------------------
  // iterators
  // -------------------------------------------------------
  virtual iterator begin() = 0;
  virtual const_iterator begin() const = 0;
  virtual iterator end() = 0;
  virtual const_iterator end() const = 0;

  // -------------------------------------------------------
  // get the annotation given a key (performs analysis if necessary)
  // -------------------------------------------------------
  virtual MyMappedT & operator[](const MyKeyT & k) = 0; // FIXME: remove when refactoring is done
  virtual MyMappedT get(const MyKeyT & k) = 0;
  
  // -------------------------------------------------------
  // demand-driven analysis
  // -------------------------------------------------------
  virtual bool is_computed() = 0;
  
  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dumpCout() const;
  virtual std::ostream& dump(std::ostream& os) const;

  // prevent cloning
private:
  AnnotationMap(const AnnotationMap &);
  AnnotationMap & operator=(const AnnotationMap &);

};


//****************************************************************************

} // end of RAnnot namespace

#endif