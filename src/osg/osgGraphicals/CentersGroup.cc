// ==========================================================================
// CENTERSGROUP class member function definitions
// ==========================================================================
// Last modified on 1/26/07; 10/13/07; 10/29/08; 4/6/14
// ==========================================================================

#include <iostream>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/Center.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "math/rotation.h"
#include "osg/osgfuncs.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void CentersGroup::allocate_member_objects()
{
   SpinTransform_refptr=new osg::MatrixTransform();
   SpinTransform_refptr->setName("SpinTransform");
}		       

void CentersGroup::initialize_member_objects()
{
   GraphicalsGroup_name="CentersGroup";

   spin_flag=false;
   theta=0;
   dtheta=2.0*PI/180;	// rads
   curr_center_posn=threevector(0,0,0);
   for (unsigned int n=get_first_framenumber(); n<= get_last_framenumber(); 
        n++)
   {
      avg_delta_UVW_per_image.push_back(threevector(0,0,0));
   }

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<CentersGroup>(
         this, &CentersGroup::update_display));
}		       

CentersGroup::CentersGroup(
   const int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr,
   threevector* GO_ptr):
   GraphicalsGroup(p_ndims,PI_ptr,AC_ptr,GO_ptr)
{	
   allocate_member_objects();
   initialize_member_objects();
   generate_center();
}		       

CentersGroup::~CentersGroup()
{
}

// --------------------------------------------------------------------------
// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_feature from all other graphical insertion
// and manipulation methods...

void CentersGroup::generate_center()
{
   int ID=0;
   Center_ptr=new Center(get_ndims(),ID);
   GraphicalsGroup::insert_Graphical_into_list(Center_ptr);
}

// --------------------------------------------------------------------------
// Member function reset_center takes in some selected point and
// subtracts from it an image-dependent offset.  The difference
// approximately compensates for sensor motion and is used to
// stabilize video imagery in FeaturesGroup::update_display().

void CentersGroup::reset_center(const threevector& curr_voxel_worldspace_posn)
{
   int curr_imagenumber=static_cast<int>(get_curr_t());

   threevector curr_UVW;
   for (unsigned int n=get_first_framenumber(); n <= get_last_framenumber(); 
        n++)
   {
      double t=static_cast<double>(n);

      curr_center_posn=curr_voxel_worldspace_posn;
      if (get_ndims()==2)
      {

// Recall horizontal and vertical screen axes correspond to X and Z
// (and not X and Y) directions.  So we need to swap 1st and 2nd
// components in curr_center_posn threevector before combining with
// avg_delta_UVW_per_image:

         curr_center_posn.put(1,curr_center_posn.get(2));
         curr_center_posn.put(2,0);
      }
      
      Center_ptr->set_UVW_coords(
         t,get_passnumber(),curr_center_posn-avg_delta_UVW_per_image[n]
         +avg_delta_UVW_per_image[curr_imagenumber]);
//      cout << "image = " << n 
//           << " center_posn = " << curr_center_posn
//           << " avg_delta = " 
//           << avg_delta_UVW_per_image[n].get(0) << " "
//           << avg_delta_UVW_per_image[n].get(1) << endl;

   } // loop over index n labeling image numbers
}

// --------------------------------------------------------------------------
// Member function increase_spin

void CentersGroup::increase_spin()
{   
   dtheta *= 2.0;
}

void CentersGroup::decrease_spin()
{   
   dtheta *= 0.5;
}

// --------------------------------------------------------------------------
// Member function update_spin_transformation

void CentersGroup::update_spin_transformation()
{   
//   cout << "inside CG::update_spin_transformation()" << endl;

// In order to rotate an arbitrary vector p about some center vector
// C, we first need to translate p by -C, rotate the translated vector
// about the origin by rotation matrix R, and then translate back by
// +C.  If p and C are regarded as ROW vectors, then 

// 		       p ---> p R + (C - C R).

// Change sign of rotation if AnimationController's state = REVERSE or
// DECREMENT_FRAME:

   double sgn=1;
   if (AnimationController_ptr->getState()==2 ||
       AnimationController_ptr->getState()==4) sgn=-1;
   theta += sgn*dtheta;
//   cout << "theta = " << theta*180/PI << endl;

   rotation R(0,0,theta);
   threevector t=curr_center_posn-R.transpose()*curr_center_posn;

   M.set(
      R.get(0,0),R.get(0,1),R.get(0,2),0,
      R.get(1,0),R.get(1,1),R.get(1,2),0,
      R.get(2,0),R.get(2,1),R.get(2,2),0,
      t.get(0),t.get(1),t.get(2),1);
   
   SpinTransform_refptr->setMatrix(M);
}

// --------------------------------------------------------------------------
void CentersGroup::update_display()
{   
   if (spin_flag) update_spin_transformation();
   GraphicalsGroup::update_display();
}
