/*
 *  HIntLib  -  Library for High-dimensional Numerical Integration 
 *
 *  Copyright (C) 2002  Rudolf Schuerer <rudolf.schuerer@sbg.ac.at>
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

#include <algorithm>

#include <HIntLib/digitalnetgen.h>

#include <HIntLib/generatormatrixvirtual.h>
#include <HIntLib/exception.h>
#include <HIntLib/hlalgorithm.h>

namespace HIntLib
{

/**
 *  The constructor initializes all member variables
 */

template<class A, typename S>
DigitalNetGen<A,S>::DigitalNetGen (
   const A &_arith, const GeneratorMatrix &_c, const Hypercube &_h,
   int _m, Index index, bool equi, Truncation _trunc) 
: QRNSequenceBase (_h),
  DigitalNet (_c.getBase(), _m),
  arith (_arith),
  scalArith (arith.getScalarAlgebra()),
  base (_c.getBase()),
  prec (min4 (
     (_trunc == FULL) ? _c.getPrec() : m,  // what we want
     int(digitsRepresentable (S(scalArith.size()))),
                            // what we can get for this S
     int (HINTLIB_MN ceil (
           HINTLIB_MN log(2.0) / HINTLIB_MN log(double(scalArith.size()))
               * double(std::numeric_limits<real>::digits - 1))),
                                                // what can be stored in real
     _c.getPrec())  // what's in the matrix
     ),
  c (AdjustPrec (prec, DiscardDimensions (getDimension(),
              NetFromSequence (m, equi, _c))),
     arith.dimension()),
  vecPrec(c.getVecPrec()),
  vecBase(c.getVecBase()),
  x      (getDimension() * vecPrec),
  xStart (getDimension() * vecPrec, 0),
  trunc (_trunc),
  ss (h),  // will be initialized later
  trivialScale (1.0 / HINTLIB_MN pow(real(base), int (prec)))
{
   if (int(arith.size()) != c.getVecBase())  throw FIXME (__FILE__, __LINE__);
   if (int(arith.dimension()) != c.getVec())
         throw FIXME (__FILE__, __LINE__);

   setCube (h);
 
#if 0
   c.print(std::cerr);
   std::cerr
      << " Prec 1: " << ((_trunc == FULL) ? _c.getPrec() : m) << "\n"
         " Prec 2: " << digitsRepresentable (S(scalArith.size())) << "\n"
         " Prec 3: "
      << HINTLIB_MN ceil (
            HINTLIB_MN log(2.0) / HINTLIB_MN log(double(scalArith.size()))
               * double(std::numeric_limits<real>::digits - 1)) << "\n"
         " Prec 4: " << _c.getPrec() << "\n\n"
         " base=" << int (base)
      << " prec=" << prec
      << " vecBase=" << vecBase
      << " vecPrec=" << vecPrec
      << " h=" << h << "\n"
         " arith.size()=" << arith.size()
      << " arith.dimension()=" << arith.dimension() << "\n"
         " scalArith.size()=" << scalArith.size() << "\n"
         " trivalScale=" << trivialScale << " " << 1/trivialScale << '\n';
#endif

   // Initialize xStart vector

   if (index > 0)  throw InternalError (__FILE__, __LINE__);   // FIXME

#if 0
   const int dd = equi;

   int i = m;
   while (index)
   {
      if (index & 1)
      {
         for (int d = dd; d < c.getDimension(); ++d)
         {
            xStart [d] ^= c(d-dd,i);
         }
      }

      index /= 2;
      ++i;
   }
#endif
}


/**
 *  setCube()
 *
 *  Reinitializes the ShiftScale for a new destination cube
 */

template<class A, typename S>
void DigitalNetGen<A,S>::setCube (const Hypercube &h)
{
   const real shift = (trunc == CENTER) ? (real(-1) / real(base)) : 0;
   ss.set (h, shift, HINTLIB_MN pow(real(base), int (prec)) + shift);
}


/**
 *  randomize()
 */

template<class A, typename S>
void DigitalNetGen<A,S>::randomize (PRNG &g)
{
   for (int i = 0; i < vecPrec * getDimension(); ++i)
   {
      xStart [i] = g.equidist (int (vecBase));
   }
}


/**
 *  copyXtoP()
 *
 *  Copies the content of the array _x_ to the real array _point_
 */

template<class A, typename S>
void DigitalNetGen<A,S>::copyXtoP (real* point)
{
   for (int d = 0; d < getDimension(); ++d)
   {
      S sum = 0;

      for (int b = 0; b < vecPrec; ++b)
      {
         sum = vecBase * sum + x[d * vecPrec + b];
      }

      point[d] = ss[d] (sum);
   }
}

template<class A, typename S>
void DigitalNetGen<A,S>::copyXtoPDontScale (real* point)
{
   for (int d = 0; d < getDimension(); ++d)
   {
      S sum = 0;

      for (int b = 0; b < vecPrec; ++b)
      {
         sum = vecBase * sum + x[d * vecPrec + b];
      }

      point[d] = sum * trivialScale;
   }
}


/**
 *  resetX()
 */

template<class A, typename S>
void DigitalNetGen<A,S>::resetX (Index nn)
{
   std::copy (&xStart[0], &xStart[getDimension() * vecPrec], &x[0]);
   const Index bas = base;
   const int p = vecPrec;

   for (int r = 0; nn != 0; ++r)
   {
      typename A::scalar_type digit = nn % bas; nn /= bas;

      if (! scalArith.is0(digit))
      {
         for (int d = 0; d < getDimension(); ++d)
         {
            // x [d] ^= c(d,r);

            for (int b = 0; b < p; ++b)
            {
               arith.addTo (x [d * p + b], arith.mul(c(d,r,b), digit));
            }
         }
      }
   }
}


/**
 *  getOptimalNumber()
 */

template<class A, typename S>
Index DigitalNetGen<A,S>::getOptimalNumber(Index max) const
{
   if (max > getSize())  return getSize();

   return (max == 0) ? 0 : roundDownToPower (max, Index(base));
}


/********************  Normal  ***********************************************/

/**
 *  Normal :: updateX()
 *
 *  The next method updates the array x depending on n and the vectors in c
 *  Once x is updated, these values are converted to real numbers and returned.
 */

template<class A, typename S>
void HIntLib::DigitalNetGenNormal<A,S>::updateX ()
{
   Index n1 = this->n;
   Index n2 = ++(this->n);
   const Index bas = this->base;
   const int p = this->vecPrec;
   const int DIM = this->getDimension();

   for (int r = 0; n1 != n2; ++r)
   {
      typename A::scalar_type digit = this->scalArith.sub (n2 % bas, n1 % bas);

      n1 /= bas; n2 /= bas;

      for (int d = 0; d != DIM; ++d)
      {
         // x [d] ^= c(d,r);

         for (int b = 0; b != p; ++b)
         {
            this->arith.addTo (this->x [d * p + b],
			       this->arith.mul(this->c(d,r,b), digit));
         }
      }
   }
} 


/*******************  Gray  **************************************************/


/**
 *  Gray :: updateX()
 *
 *  The next method updates the array x depending on n and the vectors in c
 *  Once x is updated, these values are converted to real numbers and returned.
 */

template<class A, typename S>
void HIntLib::DigitalNetGenGray<A,S>::updateX ()
{
   const Index bas = this->base;
   const int p = this->vecPrec;
   const int DIM = this->getDimension();
   Index n1 = this->n;
   Index n2 = ++(this->n);

   int r = -1;
   typename A::scalar_type rem1, rem2;

   do
   {
      rem1 = n1 % bas; n1 /= bas;
      rem2 = n2 % bas; n2 /= bas;
      ++r;
   }
   while (n1 != n2);

   const typename A::scalar_type digit = this->scalArith.sub (rem2, rem1);
   
   for (int d = 0; d != DIM; ++d)
   {
      // x [d] ^= c(d,r);

      for (int b = 0; b != p; ++b)
      {
         this->arith.addTo (this->x [d * p + b],
			    this->arith.mul(this->c(d,r,b), digit));
      }
   }
} 


/********************  Cyclic Gray  ******************************************/


/**
 *  Cyclic Gray :: updateX()
 *
 *  The next method updates the array x depending on n and the vectors in c
 *  Once x is updated, these values are converted to real numbers and returned.
 */

template<class A, typename S>
void HIntLib::DigitalNetGenCyclicGray<A,S>::updateX ()
{
   const Index bas = this->base;
   const int p = this->vecPrec;
   const int DIM = this->getDimension();
   Index n1 = this->n;
   Index n2 = ++(this->n);

   int r = 0;

   // Determine digit that has to be incremented 

   while ((n1 /= bas) != (n2 /= bas))  ++r;

   for (int d = 0; d != DIM; ++d)
   {
      // x [d] ^= c(d,r);

      for (int b = 0; b != p; ++b)
      {
         this->arith.addTo (this->x [d * p + b], this->c(d,r,b));
      }
   }
} 

#define HINTLIB_INSTANTIATE_DIGITALNETGEN(X) \
   template class DigitalNetGen<X,Index>; \
   template void DigitalNetGenNormal<X,Index>::updateX (); \
   template void DigitalNetGenGray<X,Index>::updateX (); \
   template void DigitalNetGenCyclicGray<X,Index>::updateX ();


#if 0

/**
 * Digital Net QMC Routines
 */

//  Contructor

DigitalNet2QMCRoutinesBase::DigitalNet2QMCRoutinesBase (
   const GeneratorMatrix2<Index>* _dm, bool e, DigitalNet::Truncation t,
   Index i, bool f)
: dm(_dm), m(-1), equi(e), trunc(t), index (i), fast(f) {}

//  get Optimal Number()

Index DigitalNet2QMCRoutinesBase::getOptimalNumber
   (Index max, const Hypercube &)
{
   m = ms1(max);

   if (m == -1)  return 0;
   else return Index(1) << m;
}

// checkSize()

void DigitalNet2QMCRoutinesBase::checkSize (
   const DigitalNet2<Index> &net, Index end)
{
   if (end > net.getSize()) throw NumbersExhausted();
}

// integrate()

void DigitalNet2QMCRoutines<HIntLib::real>::integrate (
   Function &f, const Hypercube &h, Index begin, Index end,
   Statistic<real,real,Index>& stat)
{
   if (m < 0)  return;

   if (fast)
   {
      DigitalNet2Gray<Index> net (*dm, h, m, index, equi, trunc);
      checkSize(net, end);
      qmcIntegration (net, f, begin, end, stat);
   }
   else
   {
      DigitalNet2Normal<Index> net (*dm, h, m, index, equi, trunc);
      checkSize(net, end);
      qmcIntegration (net, f, begin, end, stat);
   }
}

#endif
}
