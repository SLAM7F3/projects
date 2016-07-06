// ==========================================================================
// tracks_group class member function definitions
// ==========================================================================
// Last updated on 10/11/09; 12/4/10; 3/20/13; 4/5/14
// ==========================================================================

#include <iostream>
#include <string>
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "track/tracks_group.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:

void tracks_group::allocate_member_objects()
{
   track_ptrs_map_ptr=new TRACK_PTRS_MAP;
   track_labels_map_ptr=new TRACK_LABELS_MAP;
}

void tracks_group::initialize_member_objects()
{
   prev_framenumber=-1;
   start_time=0;
   destroy_all_tracks();
}

tracks_group::tracks_group()
{
   allocate_member_objects();
   initialize_member_objects();
}

tracks_group::~tracks_group()
{
   destroy_all_tracks();
   delete track_ptrs_map_ptr;
   delete track_labels_map_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const tracks_group& tg)
{
   outstream << "n_tracks = " << tg.get_n_tracks() << endl;

   vector<track*> track_ptrs=tg.get_all_track_ptrs();
   for (unsigned int n=0; n<track_ptrs.size(); n++)
   {
      const track* curr_track_ptr=track_ptrs[n];
      outstream << "n = " << n << " curr_track_ptr = " << curr_track_ptr
                << endl;
      if (curr_track_ptr != NULL)
      {
         outstream << "*curr_track_ptr = " << *curr_track_ptr << endl;
      }
   }
   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

vector<track*> tracks_group::get_all_track_ptrs() const
{
   vector<track*> track_ptrs;
   for (TRACK_PTRS_MAP::iterator iter=track_ptrs_map_ptr->begin();
        iter != track_ptrs_map_ptr->end(); ++iter)
   {
      track_ptrs.push_back(iter->second);
   }
   return track_ptrs;
}

// ==========================================================================
// Track generation and propagation member functions
// ==========================================================================

track* tracks_group::generate_new_track(int ID)
{
//   cout << "=====================================================" << endl;
   if (ID==-1) ID=get_n_tracks();

//   cout << "inside tracks_group::generate_new_track, ID = " << ID << endl;

   track* curr_track_ptr=new track(ID);
   (*track_ptrs_map_ptr)[ID]=curr_track_ptr;

//   cout << "get_n_tracks() = " << get_n_tracks() << endl;
   return curr_track_ptr;
}

// ---------------------------------------------------------------------
// Member function get_track_ID takes in integer track_label_ID which
// we assume was assigned based upon Bluegrass vehicle labels.  If the
// input corresponds to a genuine track, this method returns the
// corresponding track ID extracted from *track_labels_map_ptr.
// Otherwise, it returns -1.

int tracks_group::get_track_ID(int track_label_ID)
{
//   cout << "inside tracks_group::get_track_ID(int track_label_ID)" << endl;

   int ID=-1;
   TRACK_LABELS_MAP::iterator track_label_iter=track_labels_map_ptr->find(
      track_label_ID);
   if (track_label_iter != track_labels_map_ptr->end())
   {
      ID=track_label_iter->second;
   }
   return ID;
}

track* tracks_group::get_track_given_label(int track_label_ID)
{
//   cout << "inside tracks_group::get_track_given_label()" << endl;
   int track_ID=get_track_ID(track_label_ID);
//   cout << "track_label_ID = " << track_label_ID
//        << " track_ID = " << track_ID << endl;
   return get_track_ptr(track_ID);
}

// ---------------------------------------------------------------------
// Member function associate_track_labelID_and_ID() takes in a track.
// It adds the track's label_ID and ID to STL map
// *track_labels_map_ptr.

void tracks_group::associate_track_labelID_and_ID(track* curr_track_ptr)
{
//   cout << "inside tracks_group::associate_track_labelID_and_ID()"
 //       << endl;

   int track_labelID=curr_track_ptr->get_label_ID();
   int track_ID=curr_track_ptr->get_ID();
   
   TRACK_LABELS_MAP::iterator track_label_iter=track_labels_map_ptr->find(
      track_labelID);
   if (track_label_iter==track_labels_map_ptr->end())
   {
      (*track_labels_map_ptr)[track_labelID]=track_ID;
   }
   else
   {
      track_label_iter->second=track_ID;
   }
}

// ---------------------------------------------------------------------
// Member function destroy_track performs a brute force search for the
// track pointer corresponding to the input ID.  If found, the pointer
// is reset to NULL within member STL vector track_ptrs, the
// dynamically generated track itself is deleted, and a true boolean
// value is returned.

bool tracks_group::destroy_track(int ID)
{
//   cout << "inside tracks_group::destroy_track, input ID = " << ID << endl;
   return destroy_track(get_track_ptr(ID));
}

bool tracks_group::destroy_track(track* track_to_destroy_ptr)
{
//   cout << "inside tracks_group::destroy_track()" << endl;

   bool return_flag=false;
   for (TRACK_PTRS_MAP::iterator iter=track_ptrs_map_ptr->begin();
        iter != track_ptrs_map_ptr->end(); ++iter)
   {
      if (track_to_destroy_ptr==iter->second)
      {
         delete track_to_destroy_ptr;
         track_ptrs_map_ptr->erase(iter->first);
         return_flag=true;
      }
   }

   return return_flag;
}

void tracks_group::destroy_all_tracks()
{
   for (TRACK_PTRS_MAP::iterator iter=track_ptrs_map_ptr->begin();
        iter != track_ptrs_map_ptr->end(); ++iter)
   {
      delete iter->second;
   }
   track_ptrs_map_ptr->clear();
}

// ==========================================================================
// Track manipulation member functions
// ==========================================================================

void tracks_group::rescale_time_values_for_all_tracks(double scale_factor)
{
   for (TRACK_PTRS_MAP::iterator iter=track_ptrs_map_ptr->begin();
        iter != track_ptrs_map_ptr->end(); ++iter)
   {
      track* curr_track_ptr=iter->second;
      curr_track_ptr->rescale_time_values(scale_factor);
   }
}

// ==========================================================================
// Spacetime proximity member functions
// ==========================================================================

// Member function closest_approach_times takes in a point of interest
// (POI).  It returns an STL vector filled with the times of closest
// approach for each track within the current tracks_group object.

vector<double> tracks_group::closest_approach_times(const threevector& POI)
{
//   cout << "inside tracks_group::closest_approach_times()" << endl;
   vector<double> approach_times;

   for (TRACK_PTRS_MAP::iterator iter=track_ptrs_map_ptr->begin();
        iter != track_ptrs_map_ptr->end(); ++iter)
   {
      track* curr_track_ptr=iter->second;
      approach_times.push_back(curr_track_ptr->closest_approach_time(POI));
//      cout << "n = " << n << " closest approach time = "
//           << approach_times.back() << endl;
   } 
   return approach_times;
}

// ---------------------------------------------------------------------
// Member function ground_target_already_exists check whether a
// candidate new ground target already essentially exists within the
// current tracks_group object.  If so, this boolean method returns true.

bool tracks_group::ground_target_already_exists(
   double curr_t,double input_easting,double input_northing,
   double min_separation_distance)
{
   bool target_already_exists_flag=false;

   vector<track*> track_ptrs=get_all_track_ptrs();
   for (unsigned int t=0; t<track_ptrs.size(); t++)
   {
      track* track_ptr=track_ptrs[t];
//      threevector target_posn(track_ptr->get_earliest_posn());

      threevector target_posn;
      if (!track_ptr->get_interpolated_posn(curr_t,target_posn)) continue;
      
//         cout << "target index = " << t 
//              << " target_posn.x = " << ROI_posn.get(0)
//              << " target_posn.y = " << ROI_posn.get(1) << endl;
      double curr_separation=
         sqrt(sqr(target_posn.get(0)-input_easting)+
              sqr(target_posn.get(1)-input_northing));
      if (curr_separation < min_separation_distance)
      {
         target_already_exists_flag=true;
//            cout << "ground target already exists!" << endl;
      }
   } // loop over index t labeling tracks

//         cout << "world origin = " 
//              << EarthRegion_ptr->get_LatLongGrid_ptr()->get_world_origin()
//              << endl;

   return target_already_exists_flag;
}

// ==========================================================================
// Velocity member functions
// ==========================================================================

// Member function compute_speed_dependent_vehicle_track_colors loops
// over each track within *this.  At each DISTINCT track point, it
// computes the ground vehicle's speed in miles-per-hour.  A pure hue
// with maximal intensity is then assigned based upon a specialized
// colormap hardwired into this method.  Slow ground speeds (which
// potentially mark threatening spatial locations) correspond to warm
// colors (red, orange), while rapid ground speeds (which presumably
// cannot correspond to instantaneously threatening ground vehicles)
// correspond to cool colors (cyan,blue).

void tracks_group::compute_speed_dependent_vehicle_track_colors()
{
//   cout << "inside tracks_group::compute_speed_dependent_track_vehicle_colors()"
//        << endl;
   
   //   1 m/s = 2.23693 miles/hour

   const double meters_per_second_to_miles_per_hour=2.236936;
//   const double max_speed_of_interest=50/meters_per_second_to_miles_per_hour;

   double fast_distance=0;
   double total_distance=0;

   for (TRACK_PTRS_MAP::iterator iter=track_ptrs_map_ptr->begin();
        iter != track_ptrs_map_ptr->end(); ++iter)
   {
      track* curr_track_ptr=iter->second;
      if (curr_track_ptr==NULL) 
      {
         cout << "ERROR in tracks_group::compute_speed_dependent_colors()"
              << endl;
         cout << "curr_track_ptr = NULL !!!" << endl;
         outputfunc::enter_continue_char();
         continue;
      }

      vector<threevector> curr_posns=curr_track_ptr->get_posns();
      vector<threevector> curr_velocities=curr_track_ptr->get_velocities();

      threevector prev_posn(
         NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
      for (unsigned int n=0; n<curr_track_ptr->size(); n++)
      {

// Recall that as of 6/28/08, we only use distinct vertex positions to
// generate and display PolyLines representing tracks.  So we must
// assign velocity colors only for PolyLine segments which have
// non-zero length:

         threevector curr_posn=curr_posns[n];
         if (curr_posn.nearly_equal(prev_posn)) continue;

         double curr_distance=0;
         if (n > 0) curr_distance=(curr_posn-prev_posn).magnitude();

         prev_posn=curr_posn;

         threevector curr_velocity=curr_velocities[n];
         double speed_in_mps=curr_velocity.magnitude();
         double speed_in_mph=speed_in_mps*meters_per_second_to_miles_per_hour;
         double hue;

// Follow Delsey's suggestion and implement hue color map based upon
// log of ground vehicle speed:

//          double max_interesting_speed=25;	// mph
//         double speed_frac=basic_math::min(
//           speed_in_mph/max_interesting_speed,1.0);
//         double value=1.0-0.25*speed_frac;
         double value=1.0;
         double saturation=1.0;

/*
//         double hue_min=0;	// red
//         double hue_max=240;	// blue
         double hue_min=60;	// yellow
         double hue_max=0;	// red
         double value_max=1.0;
         double value_min=0.5;
//         double value_min=0.75;
         
         double speed_min=5;	// mph
         double speed_max=25;	// mph

         double A=(hue_max-hue_min)/log(speed_max/speed_min);
         double B=hue_max-A*log(speed_max);
         double C=(value_max-value_min)/(speed_min-speed_max);
         double D=value_max-C*speed_min;

         if (speed_in_mph < speed_min)
         {
            hue=hue_min;   
            value=value_max;
         }
         else if (speed_in_mph > speed_max)
         {
            hue=hue_max; 
            value=value_min;
         }
         else
         {
            hue=A*log(speed_in_mph)+B;
            value=C*speed_in_mph+D;
         }
*/
       
         if (speed_in_mph < 2.0)
         {
            hue=0; // red
            value=1.0;
         }
         if (speed_in_mph < 5.0)
         {
            hue=0; // red
            value=0.9;
         }
         else if (speed_in_mph < 10.0)
         {
            hue=40;	// orange-yellow
            value=0.8;
         }
         else // if (speed_in_mph > 10.0)
         {
            hue=240;	// blue
            value=0.6;
            fast_distance += curr_distance;
         }
         total_distance += curr_distance;

/*
         if (speed_in_mph < 2.0)
         {
            hue=0; // red
            value=1.0;
         }
         if (speed_in_mph >= 2.0 && speed_in_mph < 5.0)
         {
            hue=90; 
            value=1.0;
         }
         else if (speed_in_mph >= 5.0 && speed_in_mph < 10.0)
         {
            hue=180;   
            value=0.8;
         }
         else if (speed_in_mph > 10.0)
         {
            hue=270;	
            value=0.6;            
         }
*/

         double r,g,b,a=1;
         colorfunc::hsv_to_RGB(hue,saturation,value,r,g,b);

//         cout << "t = " << t 
//              << " track pnt = " << n 
//              << " speed_in_mph = " << speed_in_mph
//              << " X = " << curr_posn.get(0)
//              << " Y = " << curr_posn.get(1)
//              << " renorm = " << renormalized_speed
//              << " r = " << r << " g = " << g
//              << " b = " << b << " a = " << a 
//              << endl;

         osg::Vec4 curr_color(r,g,b,a);
         curr_track_ptr->push_back_velocity_color(curr_color);
         
      } // loop over index n labeling track points
   } // loop over index t labeling tracks

//   cout << "At end of tracks_group::compute_vehicle_speed_dependent_track_colors()"
//        << endl;
//   cout << "fast_distance = " << fast_distance
//        << " total_distance = " << total_distance
//        << " ratio = " << fast_distance/total_distance
//        << endl;
}

// ---------------------------------------------------------------------
// Member function read_speeds_from_file

void tracks_group::read_speeds_from_file(string filename)
{
   cout << "inside tracks_group::read_speeds_from_file()" << endl;
   cout << "filename = " << filename << endl;
   outputfunc::enter_continue_char();

   if (!filefunc::ReadInfile(filename))
   {
      cout << "Error in tracks_group::read_speeds_from_file()!" << endl;
      return;
   }
 
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> curr_line_values=
         stringfunc::string_to_numbers(filefunc::text_line[i]);
      int vehicle_label_ID=curr_line_values[1];
      double curr_time=curr_line_values[2];
      double curr_speed=curr_line_values[3];
      double curr_azimuth=curr_line_values[4];
      double curr_phi=(90-curr_azimuth)*PI/180.0;
      threevector curr_velocity(curr_speed*cos(curr_phi),
                                curr_speed*sin(curr_phi));

//      cout << i 
//           << "  vehicle label = " << vehicle_label_ID
//           << "  time = " << curr_time
//           << "  speed = " << curr_speed 
//           << endl;

      int track_ID=get_track_ID(vehicle_label_ID);
      if (track_ID < 0) continue;
      track* curr_track_ptr=get_track_ptr(track_ID);
      int track_point_index=curr_track_ptr->get_track_point_index(curr_time);
      if (track_point_index < 0) continue;

      vector<threevector>* velocity_ptr=curr_track_ptr->get_velocity_ptr();
      if (velocity_ptr->size()==0) 
      {
//         cout << "Filling velocity STL vector with ZeroVectors" << endl;
         for (unsigned int n=0; n<curr_track_ptr->size(); n++)
         {
            velocity_ptr->push_back(Zero_vector);
         }
      }

      if (track_point_index < int(curr_track_ptr->size()))
      {
         (*velocity_ptr)[track_point_index]=curr_velocity;
      }
      else
      {
         cout << "Error in tracks_group:::read_speeds_from_file()!" 
              << endl;
         cout << "track_point_index = " << track_point_index
              << " n_track_points = " << curr_track_ptr->size()
              << endl;
      }
   } // loop over index i labeing lines within input text file
   
}
