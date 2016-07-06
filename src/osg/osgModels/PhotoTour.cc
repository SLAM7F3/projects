// ==========================================================================
// PhotoTour class member function definitions
// ==========================================================================
// Last updated on 3/1/10; 12/4/10; 3/4/11; 4/5/14
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osgModels/PhotoTour.h"
#include "geometry/polyline.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PhotoTour::allocate_member_objects()
{
}		       

void PhotoTour::initialize_member_objects()
{
   Graphical_name="PhotoTour";
   loop_to_start_flag=false;
   ordered_OBSFRUSTUM_counter=0;
   prev_OBSFRUSTUM_ID=-1;
   tour_length=0;
}		       

PhotoTour::PhotoTour(
   OBSFRUSTAGROUP* OFG_ptr,int id):
   Geometrical(3,id,OFG_ptr->get_AnimationController_ptr())
{	
//   cout << "inside PhotoTour simple constructor" << endl;

   allocate_member_objects();
   initialize_member_objects();
   OBSFRUSTAGROUP_ptr=OFG_ptr;
}

PhotoTour::~PhotoTour()
{
//   cout << "inside PhotoTour destructor" << endl;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const PhotoTour& f)
{
   outstream << "inside PhotoTour::operator<<" << endl;
   outstream << static_cast<const Geometrical&>(f) << endl;
   return(outstream);
}

// ==========================================================================
// Tour computation methods
// ==========================================================================

// Member function construct_camera_path() takes in
// *input_polyline_ptr which contains a desired path through a field
// of photos.  After resampling the polyline, this method performs
// quadtree searches for reconstructed camera positions lying close to
// the input path.  Candidate cameras must point in basically the same
// direction as local polyline edge directions.  Their absolute
// elevations and rolls must also be relatively close to zero.  Each
// new candidate camera must also lie sufficiently far ahead in the
// previous selected camera's forward hemisphere.  This method fills
// STL vector member ordered_OBSFRUSTUM_IDs with a set of
// reconstructed cameras which hopefully yield a virtual tour fairly
// close to the desired input path.

void PhotoTour::construct_camera_path(
   polyline* input_polyline_ptr,
   KDTree::KDTree<2, threevector>* camera_posns_kdtree_ptr)
{
//   cout << "inside PhotoTour::construct_camera_path()" << endl;
 
//   double ds=4;		// meters
   double ds=3;		// meters
//   double ds=2;		// meters
   double ds_fine=1; 	// meter
   vector<double> irregularly_sampled_fracs=
      input_polyline_ptr->compute_irregularly_spaced_edge_fracs(ds,ds_fine);

   int prev_best_OBSFRUSTUM_ID=-1;
   double Z_path=POSITIVEINFINITY;
   for (unsigned int n=0; n<irregularly_sampled_fracs.size(); n++)
   {
      double frac=irregularly_sampled_fracs[n];
//      cout << "n = " << n << " path frac = " << frac << endl;

      threevector curr_vertex(input_polyline_ptr->edge_point(frac));
//      cout << "curr_vertex = " << curr_vertex << endl;

      Z_path=basic_math::min(Z_path,curr_vertex.get(2));
      twovector curr_xy_posn(curr_vertex);
      threevector e_hat(input_polyline_ptr->edge_direction(frac));
      e_hat.put(2,0);
      twovector XYdir_hat=e_hat.unitvector();
      
//      cout << "n = " << n
//           << " X = " << curr_vertex.get(0)
//           << " Y = " << curr_vertex.get(1) << endl;
//      cout << "XYdir_hat = " << XYdir_hat << endl;
      
      double initial_search_radius=10; // meters
      const int n_closest_cameras=10;
      vector<threevector> closest_cameras;
      kdtreefunc::find_closest_nodes(
         camera_posns_kdtree_ptr,curr_xy_posn,initial_search_radius,
         n_closest_cameras,closest_cameras);

      camera* prev_best_camera_ptr=OBSFRUSTAGROUP_ptr->
         get_OBSFRUSTUM_photo_camera_ptr(prev_best_OBSFRUSTUM_ID);
      
      int curr_best_OBSFRUSTUM_ID=-1;
      double max_pointing_dotproduct=-1;
//      const double min_acceptable_pointing_dotproduct=cos(15.0*PI/180.0);
      const double min_acceptable_pointing_dotproduct=cos(12.0*PI/180.0);
//      cout << "min_acceptable_pointing_dotproduct = "
//           << min_acceptable_pointing_dotproduct << endl;
      
//      const double max_abs_el=15*PI/180;
      const double max_abs_el=12*PI/180;
//      const double max_abs_roll=12*PI/180;
      const double max_abs_roll=9*PI/180;

      const double max_delta_2D=10;	// meters
      for (unsigned int c=0; c<closest_cameras.size(); c++)
      {
         int curr_OBSFRUSTUM_ID=closest_cameras[c].get(2);
         camera* camera_ptr=OBSFRUSTAGROUP_ptr->
            get_OBSFRUSTUM_photo_camera_ptr(curr_OBSFRUSTUM_ID);

         double curr_el=camera_ptr->get_rel_el();
         if (fabs(curr_el) > max_abs_el) continue;
         double curr_roll=camera_ptr->get_rel_roll();
         if (fabs(curr_roll) > max_abs_roll) continue;
         
         threevector curr_camera_posn=camera_ptr->get_world_posn();
         twovector delta_2D_posn(curr_camera_posn-curr_vertex);
         double dist_2D_to_initial_path=delta_2D_posn.magnitude();
         if (dist_2D_to_initial_path > max_delta_2D) continue;

         threevector curr_camera_pointing=-camera_ptr->get_What();
         curr_camera_pointing.put(2,0);
         twovector camera_XY_pointing=curr_camera_pointing.unitvector();
//         cout << "camera_XY_pointing = " << camera_XY_pointing << endl;
//         cout << "XYdir_hat = " << XYdir_hat << endl;
         double curr_pointing_dotproduct=camera_XY_pointing.dot(XYdir_hat);
//         cout << "curr_pointing_dotproduct = "
//              << curr_pointing_dotproduct << endl;
         
         if (curr_pointing_dotproduct < min_acceptable_pointing_dotproduct)
            continue;

// In order to enforce a sense of forward movement, we require that
// the current camera lie some reasonable distance ahead in the
// forward hemisphere of the previous best camera:

         bool continue_flag=false;
         if (prev_best_camera_ptr != NULL)
         {
            threevector prev_best_camera_posn=prev_best_camera_ptr->
               get_world_posn();
            twovector displacement_XY(
               curr_camera_posn-prev_best_camera_posn);
            twovector prev_camera_pointing(
               -prev_best_camera_ptr->get_What());
            prev_camera_pointing=prev_camera_pointing.unitvector();
            double dotproduct=displacement_XY.dot(prev_camera_pointing);
            const double min_forward_displacement=2*ds_fine;
            if (dotproduct < min_forward_displacement) continue_flag=true;
         }
         if (continue_flag) continue;

         if (curr_pointing_dotproduct > max_pointing_dotproduct)
         {
            max_pointing_dotproduct=curr_pointing_dotproduct;
            curr_best_OBSFRUSTUM_ID=curr_OBSFRUSTUM_ID;
//            cout << "c = " << c
//                 << " 2D dist = " << dist_2D_to_initial_path
//                 << " pointing_dotproduct = " << curr_pointing_dotproduct 
//                 << endl;
         }
      } // loop over index c labeling closest cameras

      if (curr_best_OBSFRUSTUM_ID < 0) continue;

      prev_best_OBSFRUSTUM_ID=curr_best_OBSFRUSTUM_ID;
      
      if (ordered_OBSFRUSTUM_IDs.size()==0)
      {
         ordered_OBSFRUSTUM_IDs.push_back(curr_best_OBSFRUSTUM_ID);
      }
      else if (curr_best_OBSFRUSTUM_ID != ordered_OBSFRUSTUM_IDs.back())
      {
         ordered_OBSFRUSTUM_IDs.push_back(curr_best_OBSFRUSTUM_ID);
      }
   } // loop over index n labeling regularly spaced input path points

   for (unsigned int i=0; i<ordered_OBSFRUSTUM_IDs.size(); i++)
   {
      int curr_OBSFRUSTUM_ID=ordered_OBSFRUSTUM_IDs[i];
      cout << "i = " << i << " ordered OBSFRUSTUM ID = "
           << curr_OBSFRUSTUM_ID << endl;
      camera* curr_camera_ptr=OBSFRUSTAGROUP_ptr->
         get_OBSFRUSTUM_photo_camera_ptr(curr_OBSFRUSTUM_ID);
      double az=curr_camera_ptr->get_rel_az()*180/PI;
      double el=curr_camera_ptr->get_rel_el()*180/PI;
      double roll=curr_camera_ptr->get_rel_roll()*180/PI;
      cout << "az = " << az << " el = " << el
           << " roll = " << roll << endl;
   }

   AnimationController_ptr->set_nframes(ordered_OBSFRUSTUM_IDs.size()+1);
   loop_to_start_flag=true;
//   outputfunc::enter_continue_char();
}

// --------------------------------------------------------------------------
// Member function specify_tour_OBSFRUSTUM_IDs() fills member STL
// vector ordered_OBSFRUSTUM_IDs with the integer IDs within input
// OBSFRUSTUM_IDs.

void PhotoTour::specify_tour_OBSFRUSTUM_IDs(vector<int>& OBSFRUSTUM_IDs)
{
   cout << "inside PhotoTour::specify_tour_OBSFRUSTUM_IDs()" << endl;

   ordered_OBSFRUSTUM_IDs.clear();
   for (unsigned int i=0; i<OBSFRUSTUM_IDs.size(); i++)
   {
      ordered_OBSFRUSTUM_IDs.push_back(OBSFRUSTUM_IDs[i]);
   }

   AnimationController_ptr->set_nframes(ordered_OBSFRUSTUM_IDs.size()+1);
   loop_to_start_flag=true;
}

// --------------------------------------------------------------------------
// Member function get_tour_posns() loops over all OBSFRUSTA within
// the current photo tour.  It fills and returns an STL vector with
// positions of cameras along the tour.  This method also computes the
// tour's total length.

vector<threevector> PhotoTour::get_tour_posns()
{
//   cout << "inside PhotoTour::get_tour_posns()" << endl;

   tour_length=0;

   vector<threevector> tour_posns;
   for (unsigned int i=0; i<ordered_OBSFRUSTUM_IDs.size(); i++)
   {
      int curr_OBSFRUSTUM_ID=ordered_OBSFRUSTUM_IDs[i];      
      camera* curr_camera_ptr=OBSFRUSTAGROUP_ptr->
         get_OBSFRUSTUM_photo_camera_ptr(curr_OBSFRUSTUM_ID);
      threevector curr_camera_posn(curr_camera_ptr->get_world_posn());

      if (i > 0)
      {
         tour_length += (curr_camera_posn-tour_posns.back()).magnitude();
      }
      tour_posns.push_back(curr_camera_posn);      

   } // loop over index i labeling ordered OBSFRUSTUM IDs
   return tour_posns;
}

// --------------------------------------------------------------------------
// Member function get_tour_photo_IDs() returns an STL vector of
// integers containing the ordered list of photos within the current tour.
 
vector<int> PhotoTour::get_tour_photo_IDs() const
{
   cout << "inside PhotoTour::get_tour_photo_IDs()" << endl;

   vector<int> tour_photo_IDs;
   for (unsigned int i=0; i<ordered_OBSFRUSTUM_IDs.size(); i++)
   {
      int curr_OBSFRUSTUM_ID=ordered_OBSFRUSTUM_IDs[i];      
      photograph* curr_photo_ptr=OBSFRUSTAGROUP_ptr->
         get_OBSFRUSTUM_photograph_ptr(curr_OBSFRUSTUM_ID);
      tour_photo_IDs.push_back(curr_photo_ptr->get_ID());
      cout << "i = " << i << " tour_photo_ID = " << tour_photo_IDs.back()
           << endl;
   } // loop over index i labeling ordered OBSFRUSTUM IDs
   return tour_photo_IDs;
}

// --------------------------------------------------------------------------
// Member function conduct_virtual_tour() checks if the
// AnimationController's framenumber has increased.  If so, it
// instructs the virtual camera to enable cross fading and to fly to
// the next OBSFRUSTUM labeled by STL vector member
// ordered_OBSFRUSTUM_IDs.  When the virtual tour reaches its end, it
// loops back to its beginning.

void PhotoTour::conduct_virtual_tour()
{
//   cout << "inside PhotoTour::conduct_virtual_tour()" << endl;
//   cout << "curr frame = " << AnimationController_ptr->get_curr_framenumber()
//        << endl;
//   cout << "loop_to_start_flag = " << loop_to_start_flag << endl;
//   cout << "ordered_OBSFRUSTUM_counter = "
//        << ordered_OBSFRUSTUM_counter << endl;

   if (ordered_OBSFRUSTUM_IDs.size() < 2) return;

   if (!loop_to_start_flag && 
       ordered_OBSFRUSTUM_counter >= ordered_OBSFRUSTUM_IDs.size()) return;

   int prev_OBSFRUSTUM_framenumber=
      OBSFRUSTAGROUP_ptr->get_prev_OBSFRUSTUM_framenumber();
//   cout << "prev_OBSFRUSTUM_framenumber = "
//        << prev_OBSFRUSTUM_framenumber << endl;
//   cout << "curr_framenumber() = "
//        << OBSFRUSTAGROUP_ptr->get_curr_framenumber() << endl;
   
   if (int(OBSFRUSTAGROUP_ptr->get_curr_framenumber()) == 
       prev_OBSFRUSTUM_framenumber) return;

   OBSFRUSTAGROUP_ptr->set_prev_OBSFRUSTUM_framenumber(
      OBSFRUSTAGROUP_ptr->get_curr_framenumber());

   int curr_OBSFRUSTUM_ID=ordered_OBSFRUSTUM_IDs[ordered_OBSFRUSTUM_counter];

   cout << "curr_OBSFRUSTUM_ID = " << curr_OBSFRUSTUM_ID << endl;
//   cout << "prev_OBSFRUSTUM_ID = " << prev_OBSFRUSTUM_ID << endl;

   if (curr_OBSFRUSTUM_ID != prev_OBSFRUSTUM_ID)
   {
      OBSFRUSTAGROUP_ptr->set_cross_fading_flag(true);
      OBSFRUSTAGROUP_ptr->set_display_Pyramids_flag(false);
      OBSFRUSTAGROUP_ptr->fly_to_entered_OBSFRUSTUM(curr_OBSFRUSTUM_ID);
      prev_OBSFRUSTUM_ID=curr_OBSFRUSTUM_ID;
   }

   bool CM_3D_active_control_flag=OBSFRUSTAGROUP_ptr->get_CM_3D_ptr()->
      get_active_control_flag();
   cout << "CM_3D_active_control_flag = "
        << CM_3D_active_control_flag << endl;
   if (!CM_3D_active_control_flag)
   {
      AnimationController_ptr->setState(AnimationController::PAUSE);
   }
   else
   {
      AnimationController_ptr->setState(AnimationController::PLAY);
   }

//   cout << "AC state = " << AnimationController_ptr->getState() << endl;
//   cout << "ordered_OBSFRUSTUM_counter = "
//        << ordered_OBSFRUSTUM_counter << endl;

   cout << "ordered_OBSFRUSTUM_IDs.size() = "
        << ordered_OBSFRUSTUM_IDs.size() << endl;
   
//   if (AnimationController_ptr->getState()==AnimationController::PLAY)
   {
      ordered_OBSFRUSTUM_counter++;

// Restart virtual tour at its beginning when end is reached:

      if (ordered_OBSFRUSTUM_counter==ordered_OBSFRUSTUM_IDs.size() &&
          loop_to_start_flag)
      {
         ordered_OBSFRUSTUM_counter=0;
      }
   }

/*
// At beginning of virtual tour, AnimationController is paused.  When
// it is set to play, move virtual camera to photo #1 from photo #0:

   else if (AnimationController_ptr->getState()==
            AnimationController::PAUSE && ordered_OBSFRUSTUM_counter==0)
   {
      ordered_OBSFRUSTUM_counter++;
   } 
*/

}
