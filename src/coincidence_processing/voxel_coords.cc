// ==========================================================================
// Voxel_coords member functions
// ==========================================================================
// Last updated on 3/28/04
// ==========================================================================

#include "coincidence_processing/voxel_coords.h"

using std::ostream;
using std::cout;
using std::endl;

ostream& operator<< (ostream& outstream,const voxel_coords& V)
{
   outstream << endl;
   outstream << V.m << endl;
   outstream << V.n << endl;
   outstream << V.p << endl;
   outstream << V.value << endl;
   return outstream;
}
