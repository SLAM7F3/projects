// ==========================================================================
// PhotoToursGroup class member function definitions
// ==========================================================================
// Last modified on 2/28/10; 3/1/10; 3/4/11
// ==========================================================================

#include <iomanip>
#include <set>
#include <string>
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osgModels/PhotoToursGroup.h"
#include "geometry/polyline.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PhotoToursGroup::allocate_member_objects()
{
}		       

void PhotoToursGroup::initialize_member_objects()
{
   GraphicalsGroup_name="PhotoToursGroup";
   PolyLinesGroup_ptr=NULL;
   conduct_tours_flag=false;
   camera_posns_kdtree_ptr=NULL;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<PhotoToursGroup>(
         this, &PhotoToursGroup::update_display));
}		       

PhotoToursGroup::PhotoToursGroup(
   OBSFRUSTAGROUP* OFG_ptr):
   GeometricalsGroup(3,OFG_ptr->get_pass_ptr(),
                     OFG_ptr->get_AnimationController_ptr(),
                     &(OFG_ptr->get_grid_world_origin()))
{	
   initialize_member_objects();
   allocate_member_objects();

   OBSFRUSTAGROUP_ptr=OFG_ptr;
}		       

PhotoToursGroup::PhotoToursGroup(
   OBSFRUSTAGROUP* OFG_ptr,PolyLinesGroup* PLG_ptr):
   GeometricalsGroup(3,OFG_ptr->get_pass_ptr(),
                     OFG_ptr->get_AnimationController_ptr(),
                     &(OFG_ptr->get_grid_world_origin()))
{	
   initialize_member_objects();
   allocate_member_objects();

   OBSFRUSTAGROUP_ptr=OFG_ptr;
   PolyLinesGroup_ptr=PLG_ptr;

   generate_camera_posns_kdtree();
}		       

PhotoToursGroup::~PhotoToursGroup()
{
//   cout << "inside OSBFRUSTAGROUP destructor, this = " << this << endl;

}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const PhotoToursGroup& f)
{
   return outstream;
}

// ==========================================================================
// Set & get methods
// ==========================================================================

// ==========================================================================
// PhotoTour creation and manipulation methods
// ==========================================================================

PhotoTour* PhotoToursGroup::generate_new_PhotoTour(int OSGsubPAT_ID,int ID)
{
//   cout << "inside PhotoToursGroup::generate_new_PhotoTour()" 
//        << endl;
   
   if (ID==-1) ID=get_next_unused_ID();
   PhotoTour* curr_PhotoTour_ptr=new PhotoTour(OBSFRUSTAGROUP_ptr,ID);
   initialize_new_PhotoTour(curr_PhotoTour_ptr,OSGsubPAT_ID);
   return curr_PhotoTour_ptr;
}

// ---------------------------------------------------------------------
void PhotoToursGroup::initialize_new_PhotoTour(
   PhotoTour* PhotoTour_ptr,int OSGsubPAT_ID)
{
//   cout << "inside PhotoToursGroup::initialize_new_PhotoTour()" << endl;
//   cout << "OSGsubPAT_ID = " << OSGsubPAT_ID << endl;

   GraphicalsGroup::insert_Graphical_into_list(PhotoTour_ptr);
   initialize_Graphical(PhotoTour_ptr);
   insert_graphical_PAT_into_OSGsubPAT(PhotoTour_ptr,OSGsubPAT_ID);
}

// --------------------------------------------------------------------------
// Member function destroy_all_PhotoTours() first fills an STL vector with
// PhotoTour pointers.  It then iterates over each vector entry and calls
// destroy_PhotoTour for each PhotoTour pointer.  On 5/3/08, we learned the
// hard and painful way that this two-step process is necessary in
// order to correctly purge all PhotoTours.

void PhotoToursGroup::destroy_all_PhotoTours()
{   
//   cout << "inside PhotoToursGroup::destroy_all_PhotoTours()" << endl;

   vector<int> PhotoTour_IDs;
   for (unsigned int p=0; p<get_n_Graphicals(); p++)
   {
      PhotoTour* PhotoTour_ptr=get_PhotoTour_ptr(p);
      PhotoTour_IDs.push_back(PhotoTour_ptr->get_ID());
   }

   for (unsigned int p=0; p<PhotoTour_IDs.size(); p++)
   {
      destroy_PhotoTour(PhotoTour_IDs[p]);
   }
}

// --------------------------------------------------------------------------
// Member function destroy_PhotoTour()

bool PhotoToursGroup::destroy_PhotoTour(int PhotoTour_ID)
{   
//   cout << "inside PhotoToursGroup::destroy_PhotoTour(PhotoTour_ID)" << endl;
   return destroy_Graphical(PhotoTour_ID);
}

// ==========================================================================
// Camera path construction and display methods
// ==========================================================================

// Member function generate_camera_posns_kdtree() instantiates and
// fills kdtree member *camera_posns_kdtree_ptr with the XY
// coordinates for each OBSFRUSTUM's camera position.

void PhotoToursGroup::generate_camera_posns_kdtree()
{
//   cout << "inside PhotoToursGroup::generate_camera_posns_kdtree()" << endl;

   vector<threevector> camera_posns;
   for (unsigned int n=0; n<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); n++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(n);
      int OBSFRUSTUM_ID=OBSFRUSTUM_ptr->get_ID();
      camera* camera_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_photo_camera_ptr(
         OBSFRUSTUM_ID);

      threevector curr_camera_posn=camera_ptr->get_world_posn();
      curr_camera_posn.put(2,OBSFRUSTUM_ID);
      camera_posns.push_back(curr_camera_posn);
//      cout << "n = " << n << " camera_posn = " << camera_posns.back() << endl;
   } // loop over index n labeling OBSFRUSTA

   delete camera_posns_kdtree_ptr;
   cout << "Calculating quadtree for camera posns" << endl;
   camera_posns_kdtree_ptr=kdtreefunc::generate_2D_kdtree(camera_posns);
//   cout << "*camera_posns_kdtree_ptr = " << *camera_posns_kdtree_ptr << endl;
   cout << "Finished calculating quadtree for camera posns" << endl;
}

// ---------------------------------------------------------------------
// Member function update_display()

void PhotoToursGroup::update_display()
{   
//   cout << "inside PhotoToursGroup::update_display()" << endl;
//   cout << "n_polylines = " << PolyLinesGroup_ptr->get_n_Graphicals() << endl;
//   cout << "n_PhotoTours = " << get_n_Graphicals() << endl;

   if (PolyLinesGroup_ptr != NULL &&
       PolyLinesGroup_ptr->get_n_Graphicals() > get_n_Graphicals())
   {
      PolyLine* PolyLine_ptr=static_cast<PolyLine*>(
         PolyLinesGroup_ptr->get_most_recently_added_Graphical_ptr());

      if (PolyLine_ptr != NULL)
      {
         if (PolyLine_ptr->get_entry_finished_flag())
         {
            PhotoTour* PhotoTour_ptr=generate_new_PhotoTour();
            polyline* phototour_polyline_ptr=PolyLine_ptr->get_polyline_ptr();
            PhotoTour_ptr->construct_camera_path(
               phototour_polyline_ptr,camera_posns_kdtree_ptr);
         } // PolyLine entry_finished_flag conditional

      } // PolyLine_ptr != NULl conditional
   } // n_PolyLines > n_PhotoTours conditional
   
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      PhotoTour* PhotoTour_ptr=get_PhotoTour_ptr(n);
      PhotoTour_ptr->conduct_virtual_tour();
   }

   GraphicalsGroup::update_display();
}

// ---------------------------------------------------------------------
// Member function generate_specified_tour() takes in an STL vector
// containing the IDs for some specified photo tour.  After destroying
// all existing PhotoTours, this method instantiates a new PhotoTour
// and sets its loop flag to false.

PhotoTour* PhotoToursGroup::generate_specified_tour(
   const vector<int>& tour_photo_IDs)
{   
//   cout << "inside PhotoToursGroup::generate_specified_tour()" << endl;

   destroy_all_PhotoTours();
   PhotoTour* PhotoTour_ptr=generate_new_PhotoTour();

   for (unsigned int i=0; i<tour_photo_IDs.size(); i++)
   {
      PhotoTour_ptr->get_ordered_OBSFRUSTUM_IDs().push_back(
         tour_photo_IDs[i]);
   }
   
   PhotoTour_ptr->get_AnimationController_ptr()->
      set_nframes(PhotoTour_ptr->get_ordered_OBSFRUSTUM_IDs().size()+1);

   PhotoTour_ptr->set_loop_to_start_flag(false);
   return PhotoTour_ptr;
}
