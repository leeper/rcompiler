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

// File: CScope.h
//
// Represents a scope in C code; produces unique variable names.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef C_SCOPE_H
#define C_SCOPE_H

#include <string>

class CScope {
public:
  explicit CScope(std::string _prefix);
  const std::string new_label() const;
private:
  const std::string m_prefix;
  static unsigned int m_id;
};

#endif
