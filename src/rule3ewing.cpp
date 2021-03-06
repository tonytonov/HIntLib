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
 *  Rule 3 Ewing
 *
 *  Cubature rule of degree  3  with  2^n + 1  points.
 *  All points are inside the hypercube.
 *
 *  It is also presented in
 *     A. H. Stoud. Approximate Calculation of Multiple Integrals (1971)
 *  as formula Cn: 3-5
 */

#define HINTLIB_LIBRARY_OBJECT

#include <HIntLib/rule3ewing.h>

#ifdef HINTLIB_USE_INTERFACE_IMPLEMENTATION
#pragma implementation
#endif

#include <HIntLib/defaultcubaturerulefactory.h>
#include <HIntLib/exception.h>
#include <HIntLib/hypercube.h>


namespace L = HIntLib;

using L::real;

/**
 *  The constructor is used primarily to initialize all the dimension dependent
 *  constaints
 */

L::Rule3Ewing::Rule3Ewing (int dim)
   : OrbitRule (dim),
     b1 (real(1) / (real(3) * real (Index(1) << dim)))
{
   checkDimensionNotZero (dim);
   checkDimensionLeq<std::numeric_limits<Index>::digits - 1> (dim);
}

namespace
{
   const real b0 = real(2) / real(3);
}


/**
 *  Do the actual function evaluation
 */

L::real L::Rule3Ewing::eval (Integrand &f, const Hypercube &h)
{
   const real* center = h.getCenter();

   setCenter (center);

   return h.getVolume() *
   (   b0 * eval0_0   (f)
     + b1 * evalR_Rfs (f, center, h.getWidth())
   );
}

L::CubatureRuleFactory* L::Rule3Ewing::getFactory()
{
   return new DefaultCubatureRuleFactory<L::Rule3Ewing> ();
}

