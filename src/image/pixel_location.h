// ==========================================================================
// Header file for pixel_location class
// ==========================================================================
// Last modified on 7/16/12
// ==========================================================================

#ifndef PIXEL_LOCATION_H
#define PIXEL_LOCATION_H

#include <set>
#include <vector>
#include "pool/objpool.h"

class pixel_location: public ObjectPool< pixel_location >
{

  public:

   pixel_location();
   pixel_location(int px,int py,int image_width,int image_height);
   pixel_location(const pixel_location& pl);
   ~pixel_location();
   pixel_location& operator= (const pixel_location& pl);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const pixel_location& pl);

// Set and get methods:

   int get_px() const;
   int get_py() const;
   int get_ID() const;
   void set_intensity(double z);
   int get_intensity() const;

   std::vector<int> get_four_neighbor_IDs();

  private: 

   int px,py;
   int image_width,image_height,ID;
   double intensity;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const pixel_location& pl);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int pixel_location::get_px() const
{
   return px;
}

inline int pixel_location::get_py() const
{
   return py;
}

inline int pixel_location::get_ID() const
{
   return ID;
}

inline void pixel_location::set_intensity(double z)
{
   intensity=z;
}

inline int pixel_location::get_intensity() const
{
   return intensity;
}



#endif  // pixel_location.h
