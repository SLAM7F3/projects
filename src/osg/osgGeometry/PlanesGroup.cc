// ==========================================================================
// PLANESGROUP class member function definitions
// ==========================================================================
// Last modified on 10/13/07; 11/27/07; 2/18/08; 4/5/14
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgFeatures/Feature.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "osg/osgGeometry/LineSegment.h"
#include "geometry/plane.h"
#include "osg/osgGeometry/PlanesGroup.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PlanesGroup::allocate_member_objects()
{
}		       

void PlanesGroup::initialize_member_objects()
{
   GraphicalsGroup_name="PlanesGroup";
   FeaturesGroup_3D_ptr=NULL;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<PlanesGroup>(
         this, &PlanesGroup::update_display));
}		       

PlanesGroup::PlanesGroup(Pass* PI_ptr):
   GraphicalsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

// ---------------------------------------------------------------------
PlanesGroup::~PlanesGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const PlanesGroup& G)
{
   return(outstream);
}

// ==========================================================================
// Plane generation methods
// ==========================================================================

// Member function generate_new_canonical_Plane returns a dynamically
// instantiated plane corresponding to the unit square.  All OSG
// planes are first created in this canonical form and subsequently
// scaled, rotated and translated via a call to
// Plane::set_scale_attitude_posn().

Plane* PlanesGroup::generate_new_canonical_Plane(int ID)
{
   threevector V1(0,0,0);
   threevector V2(1,0,0);
   threevector V3(0,1,0);
   plane p(V1,V2,V3);
   p.set_origin(threevector(0.5,0.5,0));

   Plane* curr_Plane_ptr=generate_new_Plane(p,ID);
   osg::Geode* geode_ptr=curr_Plane_ptr->generate_drawable_geode();
   curr_Plane_ptr->get_PAT_ptr()->addChild(geode_ptr);
   insert_graphical_PAT_into_OSGsubPAT(curr_Plane_ptr,Unsigned_Zero);
   return curr_Plane_ptr;
}

// ---------------------------------------------------------------------
// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Plane from all other graphical insertion
// and manipulation methods...

Plane* PlanesGroup::generate_new_Plane(const plane& p,int ID)
{
   Plane* curr_Plane_ptr;
   if (ID==-1)
   {
      curr_Plane_ptr=new Plane(
         p,get_next_unused_ID(),AnimationController_ptr);
   }
   else
   {
      curr_Plane_ptr=new Plane(p,ID,AnimationController_ptr);
   }
   curr_Plane_ptr->set_plane(p);
   GraphicalsGroup::insert_Graphical_into_list(curr_Plane_ptr);
   insert_graphical_PAT_into_OSGsubPAT(curr_Plane_ptr,Unsigned_Zero);

   return curr_Plane_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_planar_normal_segment takes in the ID
// label for some osg Plane.  It instantiates an osg LineSegment
// object which it aligns with the plane's normal vector.  The
// drawables within the LineSegment object (i.e. line body and arrow
// tip) are added to the Plane's geode.  Subsequent PAT calls which
// rotate, translate and scale the plane thus perform the
// corresponding operations on its normal vector.

void PlanesGroup::generate_planar_normal_segment(int ID)
{
   Plane* curr_Plane_ptr=get_ID_labeled_Plane_ptr(ID);
   if (curr_Plane_ptr != NULL)
   {
      curr_Plane_ptr->generate_canonical_normal_segment();
      LineSegment* n_segment_ptr=curr_Plane_ptr->get_n_segment_ptr();
      n_segment_ptr->generate_drawable_geode();
      n_segment_ptr->set_curr_color(colorfunc::grey);

      osg::Geode* nsegment_geode_ptr=n_segment_ptr->get_geode_ptr();
      int n_drawables=nsegment_geode_ptr->getNumDrawables();
      for (int n=0; n<n_drawables; n++)
      {
         curr_Plane_ptr->get_geode_ptr()->addDrawable(
            nsegment_geode_ptr->getDrawable(n));
      } // loop over drawables inside *n_segment_ptr
   } // curr_Plane_ptr != NULL conditional
}

// ---------------------------------------------------------------------
// Member function generate_plane_from_features queries the user to
// enter IDs for some set of features which this method assumes lie
// very close to within a plane.  The best planar fit to the features'
// XYZ locations is appended to the current PlanesGroup object.

void PlanesGroup::generate_plane_from_features()
{
   outputfunc::write_banner("Generating plane from 3D feature input:");

   threevector UVW,avg_feature_posn;
   vector<threevector> planar_points;
   
   while (true)
   {
      int curr_ID;
      cout << "Enter feature ID (negative to quit)" << endl;
      cin >> curr_ID;

      if (curr_ID < 0) break;
      Feature* curr_feature_ptr=FeaturesGroup_3D_ptr->
         get_ID_labeled_Feature_ptr(curr_ID);

      if (curr_feature_ptr->get_UVW_coords(
         get_curr_t(),get_passnumber(),UVW))
      {
         avg_feature_posn += UVW;
         planar_points.push_back(UVW);
      }
   } // loop over input features
   plane p(planar_points);
   avg_feature_posn /= planar_points.size();
   p.set_origin(avg_feature_posn);
   p.reset_ahat(planar_points[1]-planar_points[0]);
   p.compute_extremal_planar_coords(planar_points);

// Generate OSG Plane corresponding to geometrical plane p:

   Plane* curr_Plane_ptr=generate_new_canonical_Plane();
   generate_planar_normal_segment(curr_Plane_ptr->get_ID());
   curr_Plane_ptr->set_scale_attitude_posn(
      get_curr_t(),get_passnumber(),p);
}

// ---------------------------------------------------------------------
// Member function flip_planar_parity takes in an integer ID label for
// some plane.  It negates the a_hat and n_hat axes associated with
// this plane, and it resets the OSG Plane LineSegment corresponding
// to the normal direction.  

void PlanesGroup::flip_planar_parity(int ID)
{
   outputfunc::write_banner("Flipping planar parity");

   Plane* curr_Plane_ptr=get_ID_labeled_Plane_ptr(ID);
   if (curr_Plane_ptr != NULL)
   {
      plane* curr_plane_ptr=curr_Plane_ptr->get_plane_ptr();
      curr_plane_ptr->parity_flip_2D_coord_system();
      curr_Plane_ptr->set_scale_attitude_posn(
         get_curr_t(),get_passnumber(),*curr_plane_ptr);
   }
}
