// ==========================================================================
// Header for voxel_coords structure
// ==========================================================================
// Last updated on 3/28/04; 4/5/14
// ==========================================================================

#ifndef VOXEL_COORDS_H
#define VOXEL_COORDS_H

#include <iostream>

struct voxel_coords
{	
   unsigned int m,n,p;
   float value;

// ---------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------

   voxel_coords();
   voxel_coords(unsigned int M,unsigned int N,unsigned int P);

   friend std::ostream& operator<< 
   (std::ostream& outstream,const voxel_coords& V);

   void operator+= (const voxel_coords& V);
   void operator-= (const voxel_coords& V);

   friend voxel_coords operator+ (
      const voxel_coords& U,const voxel_coords& V);
   friend voxel_coords operator- (
      const voxel_coords& U,const voxel_coords& V);
   friend voxel_coords operator- (const voxel_coords& V);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline voxel_coords::voxel_coords()
{
   m=n=p=0;
   value=0;
}

inline voxel_coords::voxel_coords(unsigned int M,unsigned int N,unsigned int P)
{
   m=M;
   n=N;
   p=P;
   value=0;
}

// Overload +=, -=, *= and /= operators:

inline void voxel_coords::operator+= (const voxel_coords& V)
{
   m += V.m;
   n += V.n;
   p += V.p;
}

inline void voxel_coords::operator-= (const voxel_coords& V)
{
   m -= V.m;
   n -= V.n;
   p -= V.p;
}

// Overload +, - operator:

inline voxel_coords operator+ (const voxel_coords& U,const voxel_coords& V)
{
   return voxel_coords(U.m+V.m,U.n+V.n,U.p+V.p);
}

inline voxel_coords operator- (const voxel_coords& U,const voxel_coords& V)
{
   return voxel_coords(U.m-V.m,U.n-V.n,U.p-V.p);
}

inline voxel_coords operator- (const voxel_coords& V)
{
   return voxel_coords(-V.m,-V.n,-V.p);
}

#endif //  VOXEL_COORDS_H
