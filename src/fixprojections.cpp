/*
 *  HIntLib  -  Library for High-dimensional Numerical Integration
 *
 *  Copyright (C) 2002  Rudolf Sch�rer <rudolf.schuerer@sbg.ac.at>
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

#define HINTLIB_LIBRARY_OBJECT

#include <iostream>
#include <iomanip>
#include <memory>

#include <HIntLib/tparameter.h>
#include <HIntLib/minmaxfinder.h>
#include <HIntLib/array.h>

namespace L = HIntLib;


namespace
{

using std::cout;
using std::endl;

typedef L::GeneratorMatrixGen<unsigned char> GM;


/**
 *  tParameter1()
 *
 *  Calculates the maximum of the t-parameters of all 1-dimensional projections
 */

unsigned tParameter1 (const GM& gm)
{
   const unsigned DIM = gm.getDimension();

   L::MaxFinder<unsigned> mf;

   L::GMCopy copy; copy.dim(1);
   GM m (gm, copy);

   for (unsigned i = 1; i < DIM; ++i)
   {
      assign (gm, i, m, 0);
      mf << tParameter (m);
   }

   return mf.getMaximum();
}


/**
 *  tParameter2()
 *
 *  Calculates the maximum of the t-parameters of all 2-dimensional projections
 */

unsigned tParameter2 (const GM& gm)
{
   const unsigned DIM = gm.getDimension();

   L::MaxFinder<unsigned> mf;

   L::GMCopy copy; copy.dim(2);
   GM m (gm, copy);

   for (unsigned i = 1; i < DIM; ++i)
   {
      assign (gm, i, m, 0);

      for (unsigned j = 0; j < i; ++j)
      {
         assign (gm, j, m, 1);
         mf << tParameter (m);
      }
   }

   return mf.getMaximum();
}


/**
 *  tParameter3()
 *
 *  Calculates the maximum of the t-parameter of all 2-dimensional projections
 */

unsigned tParameter3 (const GM& gm)
{
   const unsigned DIM = gm.getDimension();

   if (DIM < 3)  return tParameter2 (gm);

   L::MaxFinder<unsigned> mf;

   L::GMCopy copy; copy.dim(3);
   GM m (gm, copy);

   for (unsigned i = 2; i < DIM; ++i)
   {
      assign (gm, i, m, 0);

      for (unsigned j = 1; j < i; ++j)
      {
         assign (gm, j, m, 1);

         for (unsigned k = 0; k < j; ++k)
         {
            assign (gm, k, m, 2);
            mf << tParameter (m);
         }
      }
   }

   return mf.getMaximum();
}


/**
 *  copyToMatrix()
 *
 *  Copies the first num (or M) vectors of a generator matrix to the given
 *  array.
 */

inline
unsigned char*
copyToMatrix (const GM& gm, unsigned dim, unsigned num, unsigned char* m)
{
   const unsigned M = gm.getM();

   for (unsigned r = 0; r < M; ++r)
   {
      const unsigned char* in = gm (dim,r);
      const unsigned char* inub = gm (dim, r) + num;

      unsigned char* out = m++;

      while (in != inub)
      {
         *out = *in++;
         out += M;
      }
   }

   return m - M + M * num;
}

inline
unsigned char*
copyToMatrix (const GM& gm, unsigned dim, unsigned char* m)
{
   return copyToMatrix (gm, dim, gm.getM(), m);
}


/**
 *  copyFromMatrix()
 */

void copyFromMatrix (GM& gm, unsigned dim, const unsigned char* m)
{
   const unsigned M = gm.getM();

   for (unsigned r = 0; r < M; ++r)
   {
      unsigned char* p  = gm(dim, r);

      for (unsigned b = 0; b < M; ++b)  *p++ = m [b * M + r];
   }
}


/**
 *  AchievableThickness
 */

class AchievableThickness
{
public:
   AchievableThickness (const GM&);

   int get() const  { return data0; }
   int get (unsigned d) const  { return data1 [d]; }
   int get (unsigned d1, unsigned d2) const
      { return data2 [(d1 < d2) ? (d1 * DIM + d2) : (d2 * DIM + d1)]; }

   int getMax (unsigned d) const;

   bool set (unsigned d1, unsigned d2, int x);

   void init   (const GM&, unsigned);
   void update (const GM&, int);

   void print() const;

private:
   const unsigned DIM;
   const unsigned M;

   L::Array<unsigned char> m;
   std::auto_ptr<L::LinearAlgebra> la;

   L::Array<int> data2;
   L::Array<int> data1;
   int data0;
};

AchievableThickness::AchievableThickness (const GM& gm)
   : DIM (gm.getDimension()), M (gm.getM()),
     m (M * M),
     la (L::LinearAlgebra::make(gm.getBase())),
     data2 (DIM * DIM, M), data1 (DIM, M), data0 (M)
{}

int AchievableThickness::getMax (unsigned d) const
{
   L::MaxFinder<int> mf;

   for (unsigned i = 0; i < DIM; ++i)
   {
      if (i != d)  mf << get (i, d);
   }
   return mf.getMaximum();
}

bool AchievableThickness::set (unsigned d1, unsigned d2, int x)
{
   int& r = data2 [(d1 < d2) ? (d1 * DIM + d2) : (d2 * DIM + d1)];

   if (x < r)
   {
      r = x;

      if (x < data1[d1])  data1[d1] = x;
      if (x < data1[d2])  data1[d2] = x;
      if (x < data0)  data0 = x;

      return true;
   }
   else  return false;
}

void AchievableThickness::init (const GM& gm, unsigned max)
{
   for (unsigned d1 = 1; d1 < DIM; ++d1)
   for (unsigned d2 = 0; d2 < d1;  ++d2)
   {
      for (unsigned k = 1; k <= std::min (unsigned (2 * max), M); ++k)
      {
         for (unsigned k1 = 0; k1 <= k; ++k1)
         {
            if (k1 > max || k - k1 > max)  continue;

            unsigned char* p = m.begin();
            p = copyToMatrix (gm, d1, k1,     p);
                copyToMatrix (gm, d2, k - k1, p);

            if (! la->isLinearlyIndependent(m.begin(), k, M))
            {
               set (d1, d2, k - 1);
               goto failed;
            }
         }
      }

      failed: ;
   }
}

void AchievableThickness::update (const GM& gm, int max)
{
   for (unsigned d1 = 1; d1 < DIM; ++d1)
   for (unsigned d2 = 0; d2 < d1;  ++d2)
   {
      {
         if (get(d1,d2) < max) continue;

         unsigned char* p = m.begin();
         p = copyToMatrix (gm, d1, max,              p);
             copyToMatrix (gm, d2, get(d1,d2) - max, p);

         int num = la->numLinearlyIndependentVectors (m.begin(), get(d1,d2), M);
         if (num < 2 * max)  set (d1, d2, num);
      }

      {
         if (get(d1,d2) < max) continue;

         unsigned char* p = m.begin();
         p = copyToMatrix (gm, d2, max,              p);
             copyToMatrix (gm, d1, get(d1,d2) - max, p);

         int num = la->numLinearlyIndependentVectors (m.begin(), get(d1,d2), M);
         if (num < 2 * max)  set (d1, d2, num);
      }
   }
}

void AchievableThickness::print() const
{
   cout << "AchievableThickness\n" << get() << '\n';
   cout << "    ";
   for (unsigned i = 0; i < DIM; ++i)  cout << std::setw(3) << i;
   cout << "\n    ";
   for (unsigned i = 0; i < DIM; ++i)  cout << std::setw(3) << get(i);
   cout << "\n    ";
   for (unsigned i = 0; i < DIM; ++i)  cout << std::setw(3) << getMax(i);
   cout << "\n\n";
   for (unsigned i = 0; i < DIM - 1; ++i)
   {
      cout << std::setw (3) << i << ' ';
      for (unsigned j = 0; j <= i; ++j)  cout << "   ";
      for (unsigned j = i + 1; j < DIM; ++j)  cout << std::setw(3) << get(i,j);
      cout << '\n';
   }
}


/**
 *  fullCorrect()
 *
 *  Correct all 2-dimensional projections simultaniously
 */

inline
bool increment (unsigned char* begin, unsigned char* end, unsigned base)
{
   while (*begin == base - 1)
   {
      *begin = 0;
      if (++begin == end)  return false;
   }

   ++ *begin;
   return true;
}

bool
fullCorrect (
      GM& gm, unsigned d, int out, unsigned char* work,
      bool specialAlgo, AchievableThickness& at)
{
   const unsigned DEB = 0;

   if (DEB > 1) cout << endl;
   if (DEB > 0)
   {
      cout << "Doing dimension " << d << ", out = " << out;
      if (specialAlgo)  cout << ", special algo";
      cout << endl;
   }

#if 0  // no, we need to make sure that a new l.i. vector is appended!
   if (maxBreadth == 0)
   {
      if (DEB > 1) cout << "breadth > 0 cannot be achieved!" << endl;
      return 0;
   }
#endif
 
   // Geometry of the matrix

   const unsigned M   = gm.getM();
   const unsigned M2  = M * M;
   const unsigned DIM = gm.getDimension();
   const unsigned B   = gm.getBase();

   std::auto_ptr<L::LinearAlgebra> la (L::LinearAlgebra::make (B));

   // we need  m^2 (3 + dim) + 2 M  bytes

   unsigned char* const trans1     = work;
   unsigned char* const trans2     = trans1 + M2;
   unsigned char* const tm         = trans2 + M2;
   unsigned char* const m          = tm + M2 * DIM;
   unsigned char* const candidate  = m + M2;
   unsigned char* const optimalVec = candidate + M;

   // determine breadth resulting from original vector

   int originalBreadth;

   if (specialAlgo)
   {
      // special algo ignores the original vector
      originalBreadth = -1;
   }
   else
   {
      originalBreadth = M;

      for (unsigned d2 = 0; d2 < DIM; ++d2)
      {
         if (d == d2)  continue;

         unsigned char* p = m;
         p = copyToMatrix (gm, d,  out + 1,       p);
             copyToMatrix (gm, d2, M - (out + 1), p);

         int num = la->numLinearlyIndependentVectors (m, M, M);
         if (num < out + 1)
         {
            originalBreadth = -1;
            break;
         }

         int localBreadth = num - (out + 1);
         
         if (originalBreadth > localBreadth)  originalBreadth = localBreadth;
      }

      if (DEB > 1)  cout << "Original vector has breadth " << originalBreadth
                         << endl;

      // If the original value results in a sufficient breadth, we return
      // immediately

      if (originalBreadth >= out + 1)
      {
         if (DEB > 1)  cout << "Inital breadth sufficient!" << endl;
         return false;
      }
   }

   int optimalBreadth = originalBreadth;

   // Build transformation matrix, tranforming E into original matrix

   copyToMatrix (gm, d, out, trans2);
   if (int (la->basisSupplement (trans2, out, M)) < out)
   {
      cout << "Error basisSupplement()";
      exit(1);
   }

   // Construct inverse transformation, tranforming original matrix into E

   std::copy (trans2, trans2 + M2, trans1);
   if (! la->matrixInverse (trans1, M))
   {
      cout << "Error matrixInverse()" <<endl;
      exit (1);
   }

   // Transform all generator matrices

   for (unsigned dd = 0; dd < DIM; ++dd)
   {
      copyToMatrix (gm, dd, M, m);
      la->matrixMul (m, trans1, M, m + dd * M2);
   }

   const int MM = M - out;

   if (specialAlgo)
   {
      // Entries 0,..,M-1 contain num dims for truncated breadth
      // Entires M,..,2M-1 contain num dims for untracted breadth
      L::Array<unsigned> optimalNumDimensions (2 * M, 0u);
      L::Array<unsigned> numDimensions (2 * M);

      L::Array<int> breadths (DIM);
      L::Array<int> optimalBreadths (DIM);

      // try all possible row vectors

      std::fill (candidate, candidate + MM, 0);
      while (increment(candidate, candidate + MM, B))
      {
         // determine maximum mumber of l.i. vectors with each dimension
            
         std::fill (numDimensions.begin(), numDimensions.begin() + 2 * M, 0);

         for (unsigned d2 = 0; d2 < DIM; ++d2)
         {
            if (d == d2)  continue;

            // copy test vector and transformed matrix

            unsigned char* p = m;
            p = std::copy (candidate, candidate + MM, p);
            for (int b = 0; b < MM - 1; ++b)
            {
               const unsigned char* in = tm + d2 * M2 + b * M;
               p = std::copy (in + out, in + M, p);
            }

            // count linearly independent vectors for this dimension

            int localBreadth
               = la->numLinearlyIndependentVectors (m, MM, MM) - 1;
            breadths[d2] = localBreadth;

            // Discard extra precision

            int localBreadthTrunc =
               std::min (localBreadth, std::max (at.get (d, d2) - out - 1, 0));

            // update combined result

            for (int i = 1; i <= localBreadthTrunc; ++i) ++numDimensions[i];
            for (int i = 1; i <= localBreadth; ++i)      ++numDimensions[M + i];
         }

         // is the new vector superior to previous results?

         for (unsigned i = 1; i < 2 * M; ++i)
         {
            if (numDimensions[i] > optimalNumDimensions[i])  break;
            if (numDimensions[i] < optimalNumDimensions[i])  goto vectorFailed;
         }

//         optimalBreadth = breadth;

         // we got a new optimal breadth!

         std::copy (candidate, candidate + MM, optimalVec);
         std::copy (numDimensions.begin(), numDimensions.begin() + 2 * M,
                    optimalNumDimensions.begin());
         std::copy (breadths.begin(), breadths.begin() + DIM,
                    optimalBreadths.begin());

         if (DEB > 1)
         {
            cout << "New optimal num dims:";
            for (unsigned i = 1; i < 2 * M; ++i)
            {
               cout << ' ' << optimalNumDimensions[i];
            }
            cout << ", breadth = " << optimalBreadth << endl;
         }

   vectorFailed: ;
      }

      if (DEB > 1)  cout << "Breadths:";
      for (unsigned i = 0; i < DIM; ++i)
      {
         if (i == d)  continue;
         if (DEB > 1)  cout << ' ' << optimalBreadths[i];
         at.set (d, i, optimalBreadths[i] + out + 1);
      }
      if (DEB > 1) cout << endl;

      optimalBreadth = out;
   }
   else   // normal algo
   {
      for (int breadth = optimalBreadth + 1; breadth <= out + 1; ++breadth)
      {
         if (DEB > 1)
         {
            cout << " Trying breadth " << breadth << " ... " << std::flush;
         }

         // try all possible row vectors

         std::fill (candidate, candidate + MM, 0);
         while (increment(candidate, candidate + MM, B))
         {
            // check linear independence with each matrix
            
            for (unsigned d2 = 0; d2 < DIM; ++d2)
            {
               if (d == d2)  continue;

               unsigned char* p = m;
               p = std::copy (candidate, candidate + MM, p);
               for (int b = 0; b < breadth; ++b)
               {
                  const unsigned char* in = tm + d2 * M2 + b * M;
                  p = std::copy (in + out, in + M, p);
               }

               if (! la->isLinearlyIndependent(m, breadth + 1, MM))
               {
                  goto vectorFailed1;
               }
            }

            // vector passed all tests

            std::copy (candidate, candidate + MM, optimalVec);
            optimalBreadth = breadth;
            break;

            vectorFailed1: ;
         }

         // if we have not found a vector, we quit the search

         if (optimalBreadth != breadth)
         {
            if (DEB > 1) cout << "failed" << endl;
            break;
         }
         else
         {
            if (DEB > 1) cout << "success" << endl;
         }
      }
   }

   // Do we have to use the original vector?

   if (DEB > 1)  cout << "Final breadth: " << optimalBreadth << endl;

   if (optimalBreadth <= originalBreadth)
   {
      if (DEB > 1) cout << "Original vector is all right!" << endl;

      return false;
   }
   else
   {
      if (DEB > 1)
      {
         cout << "Transformed output vector " << out << " is (";
         for (int i = 0; i < MM; ++i)  cout << int (optimalVec[i]);
         cout << ')' << endl;
      }

      // transform back vector

      la->matrixMul (optimalVec, trans2 + out * M, 1, MM, M, m);

      // copy new vector back to generator matrix

      if (DEB > 1)
      {
         cout << "Output vector " << out << " is (";
         for (unsigned i = 0; i < M; ++i)  cout << int (m[i]);
         cout << ')' << endl;
      }

      for (unsigned i = 0; i < M; ++i)  gm.setd (d, i, out, m[i]);

      return true;
   }
}

}  // anonimous namespace

void L::fixTwoDimensionalProjections (GM& gm)
{
   const unsigned DEB = 0;

   const unsigned M = gm.getM();
   const unsigned DIM = gm.getDimension();

   if (gm.getTotalPrec() < M)  throw 1;

   Array<unsigned char> work ((3 + DIM) * M * M + 2 * M);

   const int initialOut = std::max (
         std::max (M - tParameter3(gm), 2u) - 2,  // what is taboo
         (M - tParameter2(gm)) / 2                // what is alright already
   );

   AchievableThickness at (gm);
   at.init (gm, initialOut);
   if (DEB > 2)  at.print();

   Array<bool> dimensionDone (DIM);

   for (int out = initialOut; out < int (M); ++out)
   {
      if (DEB > 1)  cout << "out = " << out << endl;

      std::fill (dimensionDone.begin(), dimensionDone.begin() + DIM, false);
      bool changes;

      do
      {
         changes = false;

         for (unsigned d = 0; d < DIM; ++d)
         {
            if (dimensionDone [d])  continue;
            if (DEB > 1)  cout << d << ": ";

            // Check if the coordinate is independent

            if (at.getMax (d) <= 2 * out + 1)
            {
               if (DEB > 1)  cout << '^';
               fullCorrect (gm, d, out, work.begin(), true, at);
               dimensionDone [d] = true;
            }
            else   // non-independent coordinate
            {
               bool c = fullCorrect (gm, d, out, work.begin(), false, at);
               changes |= c;
               if (DEB > 1)  cout << (c ? '*' : '-');
            }
            if (DEB > 1)  cout << endl;
         }
      }
      while (changes);

      at.update (gm, out + 1);
      if (DEB > 2)  at.print();
   }
}


/**
 *  fixOneDimensionalProjections()
 */

void L::fixOneDimensionalProjections (GM& gm)
{
   const unsigned M = gm.getM();
   const unsigned DIM = gm.getDimension();

   if (gm.getTotalPrec() < M)  throw 1;

   std::auto_ptr<LinearAlgebra> la (LinearAlgebra::make (gm.getBase()));

   Array<unsigned char> m (M * M);

   for (unsigned d = 0; d < DIM; ++d)
   {
      copyToMatrix   (gm, d, m.begin());
      la->basisSupplement (m.begin(), M, M);
      copyFromMatrix (gm, d, m.begin());
   }
}


