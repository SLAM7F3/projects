// ==========================================================================
// Header file for (pure virtual) Annotator class
// ==========================================================================
// Last updated on 1/21/07
// ==========================================================================

#ifndef ANNOTATOR_H
#define ANNOTATOR_H

#include <iostream>
#include "astro_geo/geopoint.h"

class Annotator 
{

  public:

// Initialization, constructor and destructor functions:

   Annotator();
   Annotator(const int p_ndims,int id);
   virtual ~Annotator();
   friend std::ostream& operator<< (
      std::ostream& outstream,const Annotator& a);

// Set & get member functions:

   void set_geopoint(const geopoint& gp);
   geopoint* get_geopoint_ptr();

  protected:

   geopoint* geopoint_ptr;

  private:

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Annotator& a);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void Annotator::set_geopoint(const geopoint& gp)
{
   *geopoint_ptr=gp;
}

inline geopoint* Annotator::get_geopoint_ptr()
{
   return geopoint_ptr;
}


#endif // Annotator.h


