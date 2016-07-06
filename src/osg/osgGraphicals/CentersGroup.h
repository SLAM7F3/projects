// ==========================================================================
// Header file for CENTERSGROUP class
// ==========================================================================
// Last modified on 10/18/05; 12/18/05; 12/8/06; 1/3/07; 10/13/07
// ==========================================================================

#ifndef CENTERSGROUP_H
#define CENTERSGROUP_H

#include <iostream>
#include <vector>
#include <osg/MatrixTransform>
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "passes/PassesGroup.h"
#include "math/threevector.h"

class AnimationController;
class Center;
// class osg::Group;

class CentersGroup : public GraphicalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   CentersGroup(const int p_ndims,Pass* PI_ptr,
                AnimationController* AC_ptr=NULL,threevector* GO_ptr=NULL);
   virtual ~CentersGroup();

// Set & get methods:

   void toggle_spin_flag();
   Center* get_Center_ptr();
   osg::MatrixTransform* get_SpinTransform_ptr();
   void reset_center(const threevector& curr_voxel_worldspace_posn);
   void set_avg_delta_UVW_per_image(int i,const threevector& delta);
   threevector& get_avg_delta_UVW_per_image(int i);
   const threevector& get_avg_delta_UVW_per_image(int i) const;

// Animation methods:

   void increase_spin();
   void decrease_spin();
   void update_spin_transformation();
   void update_display();

  protected:

   std::vector<threevector> avg_delta_UVW_per_image;

  private:
  
   bool spin_flag;
   double theta,dtheta;
   threevector curr_center_posn;
   Center* Center_ptr;
   osg::Matrixd M;
   osg::ref_ptr<osg::MatrixTransform> SpinTransform_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void generate_center();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void CentersGroup::toggle_spin_flag()
{
   spin_flag=!spin_flag;
   std::cout << "spin_flag = " << spin_flag << std::endl;
}

inline Center* CentersGroup::get_Center_ptr()
{
   return Center_ptr;
}

inline osg::MatrixTransform* CentersGroup::get_SpinTransform_ptr()
{
   return SpinTransform_refptr.get();
}

inline void CentersGroup::set_avg_delta_UVW_per_image(
   int i,const threevector& delta)
{
   avg_delta_UVW_per_image[i]=delta;
}

inline threevector& CentersGroup::get_avg_delta_UVW_per_image(int i)
{
   return avg_delta_UVW_per_image[i];
}

inline const threevector& CentersGroup::get_avg_delta_UVW_per_image(int i) 
   const
{
   return avg_delta_UVW_per_image[i];
}



#endif // CentersGroup.h



