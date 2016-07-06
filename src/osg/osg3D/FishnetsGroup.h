// ==========================================================================
// Header file for FISHNETSGROUP class
// ==========================================================================
// Last modified on 12/1/11; 12/5/11; 12/6/11
// ==========================================================================

#ifndef FISHNETSGROUP_H
#define FISHNETSGROUP_H

#include <iostream>
#include <string>
#include "osg/osg3D/Fishnet.h"
#include "osg/osgGeometry/GeometricalsGroup.h"

class FishnetsGroup : public GeometricalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   FishnetsGroup(
      Pass* PI_ptr,threevector* GO_ptr=NULL);
   virtual ~FishnetsGroup();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const FishnetsGroup& FG);

// Set & get member functions:

   Fishnet* get_Fishnet_ptr(int n) const;
   void set_Zstart(double z);
   double get_Zstart() const;
   void set_Zstop(double z);

// Fishnet creation member functions:

   Fishnet* generate_new_Fishnet(bool fall_downwards_flag);
   
// Ground surface computation member functions:

   void compute_initial_surface_energy();
   void refine_ground_surface();
   void refine_Fishnet();
   void relax_ground_surface();

  protected:

  private:

   bool stop_ground_refinement_flag;
   int iter_counter,fishnet_refinement_counter;
   double Zstart,Zstop,min_frac_dE;
   double dz_frac_step;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const FishnetsGroup& C);

   void initialize_new_Fishnet(
      Fishnet* curr_fishnet_ptr,int OSGsubPAT_number=0);

   void update_display();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline Fishnet* FishnetsGroup::get_Fishnet_ptr(int n) const
{
   return dynamic_cast<Fishnet*>(get_Graphical_ptr(n));
}

inline void FishnetsGroup::set_Zstart(double z)
{
   Zstart=z;
}

inline double FishnetsGroup::get_Zstart() const
{
   return Zstart;
}

inline void FishnetsGroup::set_Zstop(double z)
{
   Zstop=z;
}



#endif // FishnetsGroup.h



