/* Copyright (c) 2003 John Garvin 
 *
 * July 11, 2003 
 *
 * Parses R code, turns into C code that uses internal R functions.
 * Attempts to output some code in regular C rather than using R
 * functions.
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#include <iostream>
#include <sstream>
#include <string>
#include <map>

#include <rinternals.h>

std::string make_symbol(SEXP e);
std::string make_type(int t);
std::string indent(std::string str);
std::string i_to_s(const int i);
std::string d_to_s(double d);
std::string c_to_s(Rcomplex c);
std::string escape(std::string str);
std::string make_c_id(std::string s);
std::string quote(std::string str);
std::string unp(std::string str);
std::string strip_suffix(std::string str);
int filename_pos(std::string str);
void err(std::string message);