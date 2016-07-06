// ==========================================================================
// Header for voxel_coords class
// ==========================================================================
// Last updated on 2/21/06; 12/10/06
// ==========================================================================

#ifndef VOXEL_COORDS_H
#define VOXEL_COORDS_H

#include <iostream>

class voxel_coords
{	

  public:

   voxel_coords();
   voxel_coords(unsigned int M,unsigned int N,unsigned int P);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const voxel_coords& V);

// Set & get member functions:

   int get(int i) const;

   void operator+= (const voxel_coords& V);
   void operator-= (const voxel_coords& V);

   friend voxel_coords operator+ (
      const voxel_coords& U,const voxel_coords& V);
   friend voxel_coords operator- (
      const voxel_coords& U,const voxel_coords& V);
   friend voxel_coords operator- (const voxel_coords& V);

  private: 

   unsigned int m,n,p;
   double value;
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int voxel_coords::get(int i) const
{
   switch(i)
   {
      case 0: 
         return m;
         break;
      case 1:
         return n;
         break;
      case 2:
         return p;
         break;
      default:
         std::cout << "Error in voxel_coords::get_coord, i = " << i
                   << std::endl;
         return 0;
   }
}



#endif //  VOXEL_COORDS_H
