// ==========================================================================
// Voxel_coords member functions
// ==========================================================================
// Last updated on 2/21/06; 12/10/06
// ==========================================================================

#include "threeDgraphics/voxel_coords.h"

using std::ostream;
using std::cout;
using std::endl;

voxel_coords::voxel_coords()
{
   m=n=p=0;
   value=0;
}

voxel_coords::voxel_coords(unsigned int M,unsigned int N,unsigned int P)
{
   m=M;
   n=N;
   p=P;
   value=0;
}

ostream& operator<< (ostream& outstream,const voxel_coords& V)
{
   outstream << endl;
   outstream << V.m << endl;
   outstream << V.n << endl;
   outstream << V.p << endl;
   outstream << V.value << endl;
   return(outstream);
}


// Overload +=, -=, *= and /= operators:

void voxel_coords::operator+= (const voxel_coords& V)
{
   m += V.m;
   n += V.n;
   p += V.p;
}

void voxel_coords::operator-= (const voxel_coords& V)
{
   m -= V.m;
   n -= V.n;
   p -= V.p;
}

// Overload +, - operator:

voxel_coords operator+ (const voxel_coords& U,const voxel_coords& V)
{
   return voxel_coords(U.m+V.m,U.n+V.n,U.p+V.p);
}

voxel_coords operator- (const voxel_coords& U,const voxel_coords& V)
{
   return voxel_coords(U.m-V.m,U.n-V.n,U.p-V.p);
}

voxel_coords operator- (const voxel_coords& V)
{
   return voxel_coords(-V.m,-V.n,-V.p);
}
