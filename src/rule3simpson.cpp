/*
 *  HIntLib  -  Library for High-dimensional Numerical Integration
 *
 *  Copyright (C) 2002,03,04,05  Rudolf Schuerer <rudolf.schuerer@sbg.ac.at>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/**
 *  Rule 3 Simpson
 *
 *  Cubature rule of degree 3 with 3^n points.
 *  All points (except the center) are on the border of the hypercube.
 *
 *  It is also presented in
 *     A. H. Stoud. Approximate Calculation of Multiple Integrals (1971)
 *  as formula Cn: 3-6
 */

#define HINTLIB_LIBRARY_OBJECT

#include <HIntLib/rule3simpson.h>

#ifdef HINTLIB_USE_INTERFACE_IMPLEMENTATION
#pragma implementation
#endif

#include <HIntLib/defaultcubaturerulefactory.h>
#include <HIntLib/exception.h>
#include <HIntLib/hypercube.h>


namespace L = HIntLib;

using L::real;

namespace
{
   const real w0 = real(4) / real(3);
   const real w1 = real(1) / real(3);
}


/**
 *  The constructor is used primarily to initialize all the dimension dependent
 *  constatns and to allocate (dimension dependent) memory
 */

L::Rule3Simpson::Rule3Simpson (int dim)
   : OrbitRule (dim), oneDivTwoPowDim (real(1) / real(Index(1) << dim))
{
   checkDimensionNotZero (dim);

   int maxDim = logInt(std::numeric_limits<Index>::max(), Index(3));

   if (dim > maxDim)  throw DimensionTooHigh (dim, maxDim);
}


/**
 *  Do the actual function evaluation
 */

real L::Rule3Simpson::eval (Integrand &f, const Hypercube &h)
{
   return h.getVolume() * eval3powS (f, h.getCenter(), h.getWidth(), w0, w1)
        * oneDivTwoPowDim;
}

L::CubatureRuleFactory* L::Rule3Simpson::getFactory()
{
   return new DefaultCubatureRuleFactory<L::Rule3Simpson> ();
}

