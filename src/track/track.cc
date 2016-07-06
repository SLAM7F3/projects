// ==========================================================================
// Track class member function definitions
// ==========================================================================
// Last modified on 10/9/11; 1/30/12; 5/19/13; 4/5/14
// ==========================================================================

#include "math/adv_mathfuncs.h"
#include "geometry/bounding_box.h"
#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "math/mathfuncs.h"
#include "track/mover_funcs.h"
#include "messenger/Messenger.h"
#include "templates/mytemplates.h"
#include "numrec/nrfuncs.h"
#include "geometry/polyline.h"
#include "math/statevector.h"
#include "math/threevector.h"
#include "track/track.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void track::allocate_member_objects()
{
//   cout << "inside track::allocate_member_objs()" << endl;
   time_map_ptr=new TIME_MAP;
}		       

void track::initialize_member_objects()
{
   broadcast_contents_flag=spatially_fixed_flag=false;
   label_ID=Cylinder_ID=PolyLine_ID=-1;
   description="Unknown";
   track_closed_flag=false;
   RGB_color.first=RGB_color.second=RGB_color.third=-1;
   t.clear();
}		       

track::track(int id)
{
   allocate_member_objects();
   initialize_member_objects();
   ID=id;
}

// Copy constructor

track::track(const track& T)
{
//   cout << "inside track copy constructor" << endl;
   allocate_member_objects();
   docopy(T);
}

track::~track()
{
//   cout << "inside track destructor" << endl;
   delete time_map_ptr;
}

// ---------------------------------------------------------------------
void track::docopy(const track& T)
{
//   cout << "inside track::docopy()" << endl;
   track_closed_flag=T.track_closed_flag;
   ID=T.ID;
   label_ID=T.label_ID;
   Cylinder_ID=T.Cylinder_ID;
   PolyLine_ID=T.PolyLine_ID;
   length=T.length;
   RGB_color=T.RGB_color;
   entityID=T.entityID;
   entityType=T.entityType;
   label=T.label;

   t=T.t;
   (*time_map_ptr)=*(T.time_map_ptr);

   posn=T.posn;
   velocity=T.velocity;
   segment=T.segment;
   velocity_colors=T.velocity_colors;
   segment_colors=T.segment_colors;
}

// Overload = operator:

track& track::operator= (const track& T)
{
   if (this==&T) return *this;
   docopy(T);
   return *this;
}

// Overload << operator

ostream& operator<< (ostream& outstream,const track& T)
{
//   cout << "inside track::operator<<()" << endl;
   outstream << "Track ID = " << T.ID << endl;
   outstream << "spatially_fixed_flag = " 
             << T.spatially_fixed_flag << endl;
   outstream << "n_track_points = " << T.size() << endl;
   for (unsigned int i=0; i<T.size(); i++)
   {
      outstream << "i=" << i 
                << " t=" << T.get_time(i)
                << " X=" << T.posn[i].get(0) 
                << " Y=" << T.posn[i].get(1)
                << " Z=" << T.posn[i].get(2);
      if (T.velocity.size() > 0)
      {
         outstream << " Vx=" << T.velocity[i].get(0)
                   << " Vy=" << T.velocity[i].get(1)
                   << " Vz=" << T.velocity[i].get(2)
                   << " XY speed = " << sqrt(sqr(T.velocity[i].get(0))+
                                             sqr(T.velocity[i].get(1)));
      }
      outstream << endl;
   }
   return outstream;
}

// ==========================================================================
// Set and get methods
// ==========================================================================

// Member function pushback_time() takes in candidate time curr_t.  If
// curr_t does not already exist within *time_map_ptr, this method
// pushes it back onto STL vector t and adds it to *time_map_ptr.
// Otherwise, this boolean member function returns false.

bool track::pushback_time(double curr_t) 
{
//   cout << "inside track::pushback_time, curr_t = " << curr_t << endl;
   
   TIME_MAP::iterator time_iter=time_map_ptr->find(curr_t);

   if (time_iter != time_map_ptr->end())
   {
      return false;
   }
   else
   {
      t.push_back(curr_t);
      (*time_map_ptr)[curr_t]=time_map_ptr->size();
      return true;
   }
}

void track::set_time(int i,double curr_t) 
{
   t[i]=curr_t;
}

double track::get_time(int i) const
{
   return t[i];
}

void track::set_XYZ_coords(double curr_t,const threevector& r,
   bool temporally_sort_flag)
{
//   cout << "inside track::set_XYZ_coords()" << endl;

   if (!pushback_time(curr_t)) return;

   posn.push_back(r);
   if (temporally_sort_flag) templatefunc::Quicksort(t,posn);
}

bool track::get_XYZ_coords(double curr_t,threevector& r) const
{ 
   int index=get_track_point_index(curr_t);
   if (index >= 0 && index <= int(size()-1))
   {
      r=posn[index];
      return true;
   }
   else
   {
      return false;
   }
}

void track::set_posn_velocity(
   double curr_t,const threevector& r,const threevector& v,
   bool temporally_sort_flag)
{
   if (!pushback_time(curr_t)) return;

//   cout << "inside track::set_posn_velocity()" << endl;
//   cout << "t.size() = " << t.size() 
//        << " time_map_ptr->size() = " << time_map_ptr->size() << endl;
//   cout << "curr_t = " << curr_t << " r = " << r << " v = " << v << endl;

   if (t.size() != time_map_ptr->size())
   {
      cout << "inside track::set_posn_velocity()" << endl;
      cout << "t.size != time_map_ptr->size !!!" << endl;
      outputfunc::enter_continue_char();
   }
   
   posn.push_back(r);
   velocity.push_back(v);
   if (temporally_sort_flag) temporally_sort_posns_and_velocities();
}

// ---------------------------------------------------------------------
// Member function set_posn_velocity_GPSquality() is intended to
// accept raw data from an input GPS sensor.  In particular, it takes in 
// threevector q which contains the current GPS fix quality, number of
// visible GPS satellites and horizontal dilution o fprecision:

void track::set_posn_velocity_GPSquality(
   double curr_t,const threevector& r,const threevector& v,
   const threevector& q)
{
   set_posn_velocity(curr_t,r,v);

   fix_quality.push_back(q.get(0));
   n_satellites.push_back(q.get(1));
   horiz_dilution.push_back(q.get(2));
}

// ---------------------------------------------------------------------
void track::set_posn_velocity_GPSquality_rpy(
   double curr_t,const threevector& r,const threevector& v,
   const threevector& q,const rpy& r_p_y)
{
   set_posn_velocity_GPSquality(curr_t,r,v,q);
   roll_pitch_yaw.push_back(r_p_y);
}

// ---------------------------------------------------------------------
void track::set_posn_rpy(double curr_t,const threevector& r,const rpy& r_p_y)
{
   set_XYZ_coords(curr_t,r);
   roll_pitch_yaw.push_back(r_p_y);
}

// ---------------------------------------------------------------------
void track::set_posn_rpy_sensor_aer_fov(
   double curr_t,const threevector& r,const rpy& r_p_y,
   const threevector& sensor_aer,const twovector& sensor_hfov_vfov)
{
//   cout << "inside track::set_posn_rpy_sensor_aer_fov()" << endl;
//   cout << "curr_t = " << curr_t << endl;
//   cout << "r = " << r << endl;
   set_XYZ_coords(curr_t,r);
   roll_pitch_yaw.push_back(r_p_y);
   sensor_az_el_roll.push_back(sensor_aer);
   sensor_FOV.push_back(sensor_hfov_vfov);
}

// ---------------------------------------------------------------------
void track::temporally_sort_posns_and_velocities()
{
   templatefunc::Quicksort(t,posn,velocity);
}

bool track::get_posn_velocity(double curr_t,threevector& r,threevector& v) 
   const
{ 
//   cout << "inside track::get_posn_velocity()" << endl;

   int index=get_track_point_index(curr_t);
   if (index >= 0 && index <= int(size()-1))
   {
      r=posn[index];
      v=velocity[index];
      return true;
   }
   else
   {
      return false;
   }
}

// ---------------------------------------------------------------------
// Member function set_bbox() assumes that a track point corresponding
// to input time curr_t has already been instantiated.  If so, this
// method pushes the input bounding_box curr_bbox onto the STL vector
// member bbox.  

void track::set_bbox(double curr_t,const bounding_box& curr_bbox)
{
//   cout << "inside track::set_bbox()" << endl;
//   cout << "spatially_fixed_flag = " << spatially_fixed_flag << endl;

   if (spatially_fixed_flag)
   {
      bbox.clear();
      bbox.push_back(curr_bbox);
   }
   else
   {
      int index=get_track_point_index(curr_t);
      if (index >= 0 && index <= int(size()-1))
      {
         bbox.push_back(curr_bbox);
      }
   }
}

bool track::get_bbox(double curr_t,bounding_box& curr_bbox) const
{ 
   if (spatially_fixed_flag)
   {
      if (bbox.size() > 0)
      {
         curr_bbox=bbox.back();
         return true;
      }
      else
      {
         return false;
      }
   }

   int index=get_track_point_index(curr_t);
   if (index >= 0 && index <= int(size()-1))
   {
      curr_bbox=bbox[index];
      return true;
   }
   else
   {
      return false;
   }
}

// ---------------------------------------------------------------------
void track::purge_all_values()
{
   t.clear();
   time_map_ptr->clear();
   posn.clear();
   distinct_posn.clear();
   posns_with_distinct_velocities.clear();
   velocity.clear();
   segment.clear();
   velocity_colors.clear();
   segment_colors.clear();
   bbox.clear();
}


// ==========================================================================
// Track coloring member functions
// ==========================================================================

// Member function compute_track_color_fading loops over all track
// points within *this.  At each point, it multiplies the value V for
// the track's HSV triple by a piecewise-linear decreasing function of
// cumulative track length.  We wrote this specialized method in Feb
// 2009 in order to fade down dynamic UAV tracks computed by Luca
// Bertucelli's Consensus Based Bundle Algorithm which are more
// trustworthy near the track's beginning than at its end.

// Note added on 2/4/09: This method is now deprecated.  Use
// PolyLine::compute_color_fading() instead.

void track::compute_track_color_fading()
{
//   cout << "inside track::compute_track_color_fading()" << endl;

   double track_length=total_length();
   vector<double> cumulative_lengths=get_cumulative_lengths();
   vector<threevector> curr_posns=get_posns();

   threevector prev_posn(
      NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
   for (unsigned int n=0; n<size(); n++)
   {

// Recall that as of 6/28/08, we only use distinct vertex positions to
// generate and display PolyLines representing tracks.  So we must
// assign velocity colors only for PolyLine segments which have
// non-zero length:

      threevector curr_posn=curr_posns[n];
      if (curr_posn.nearly_equal(prev_posn)) continue;
      prev_posn=curr_posn;

      double f=cumulative_lengths[n]/track_length;
      const double f_start=0.25;
      const double f_stop=0.55;
      const double v_start=1.0;
      const double v_stop=0.5;
      double v_frac;
      if (f < f_start)
      {
         v_frac=v_start;
      }
      else if (f > f_stop)
      {
         v_frac=v_stop;
      }
      else
      {
         v_frac=v_start+(f-f_start)/(f_stop-f_start)*(v_stop-v_start);
      }
         
      colorfunc::RGB track_RGB=get_RGB_color();
      colorfunc::HSV orig_track_HSV=colorfunc::RGB_to_hsv(track_RGB);
      colorfunc::HSV track_HSV=orig_track_HSV;
      track_HSV.third *= v_frac;
      track_RGB=colorfunc::hsv_to_RGB(track_HSV);

//      cout << "n = " << n 
//           << " cum l = " << cumulative_lengths[n]
//           << " f = " << f
//           << " v_frac = " << v_frac << endl;
//           << " h = " << track_HSV.first
//           << " s = " << track_HSV.second
//           << " vorig = " << orig_track_HSV.third
//           << " v = " << track_HSV.third 
//           << " r = " << track_RGB.first
//           << " g = " << track_RGB.second 
//           << " b = " << track_RGB.third
//           << endl;

      double alpha=1;
      osg::Vec4 curr_color(
         track_RGB.first,track_RGB.second,track_RGB.third,alpha);
      push_back_segment_color(curr_color);
   } // loop over index n labeling track points
}

// ==========================================================================
// Track property member functions
// ==========================================================================

double track::get_earliest_time() const
{
   if (t.size() > 0)
   {
      return t.front();
   }
   else
   {
      return NEGATIVEINFINITY;
   }
}

double track::get_latest_time() const
{
   if (t.size() > 0)
   {
      return t.back();
   }
   else
   {
      return POSITIVEINFINITY;
   }
}

threevector track::get_earliest_posn() const
{
   return posn.front();
}

threevector track::get_latest_posn() const
{
   return posn.back();
}

const vector<double>& track::get_cumulative_lengths() const
{
   return cumulative_lengths;
}

vector<threevector>& track::get_posns() 
{
   return posn;
}

const vector<threevector>& track::get_posns() const
{
   return posn;
}

vector<threevector>& track::get_distinct_posns() 
{
   if (distinct_posn.size()==0) compute_distinct_posns();
   return distinct_posn;
}

vector<threevector>& track::get_velocities() 
{
   return velocity;
}

const vector<threevector>& track::get_velocities() const
{
   return velocity;
}

vector<threevector>* track::get_velocity_ptr()
{
   return &velocity;
}

vector<threevector>& track::get_posns_with_distinct_velocities()
{
   if (posns_with_distinct_velocities.size()==0) 
      compute_posns_with_distinct_velocities();
   return posns_with_distinct_velocities;
}

vector<rpy>& track::get_rpys()
{
   return roll_pitch_yaw;
}

const vector<rpy>& track::get_rpys() const
{
   return roll_pitch_yaw;
}

// ---------------------------------------------------------------------
// Member function get_latest_heading returns within output threevector
// u_hat the unit vector pointing in the direction between the track's
// last two positions.  If the track contains less than 2 position
// values, this boolean method returns false.

bool track::get_latest_heading(threevector& u_hat)
{
   if (size() <= 1)
   {
      return false;
   }
   else
   {
      threevector u(posn[size()-1]-
                    posn[size()-2]);
      u_hat=u.unitvector();
      return true;
   }
}

// ---------------------------------------------------------------------
// Member function max_temporal_gap returns the largest gap between
// track time measurements.

double track::max_temporal_gap()
{
   double dt_max=0;
   for (unsigned int i=1; i<size(); i++)
   {
      dt_max=basic_math::max(dt_max,get_time(i)-get_time(i-1));
   }
   return dt_max;
}

// ---------------------------------------------------------------------
// Member function temporal_duration returns the difference between
// the last and first track time measurements.

double track::temporal_duration()
{
   return (t.back()-t.front());
}

// ---------------------------------------------------------------------
// Member function close_track_test computes the difference between
// the input current time and the track's latest temporal entry.  If
// this difference exceeds the input delta_t, the track is considered
// to be closed.  No subsequent detections should then be added to the
// track even if they lie sufficiently near to the latest
// position. Instead, a brand new track should be formed.

void track::close_track_test(double curr_t,double delta_t)
{
   if (curr_t-get_latest_time() > delta_t)
   {
      track_closed_flag=true;
   }
}

// ---------------------------------------------------------------------
// Member function total_length computes and stores the integral of
// each linesegment comprising the entire track within member variable
// length.  It also saves the cumulative lengths within member STL
// vector cumulative_lengths.

// Old note: As of 1/1/06, we assume the track is 2-dimensional (so
// that detection ID info can be stored in the 3rd entry in each of
// the posn array member)...

double track::total_length() 
{
   length=0;
   cumulative_lengths.push_back(length);
   for (unsigned int i=0; i<size()-1; i++)
   {
      threevector separation(posn[i+1]-posn[i]);
      length += separation.magnitude();
      cumulative_lengths.push_back(length);
   }
   return length;
}

// ---------------------------------------------------------------------
// Member function avg_speed returns the total integrated track length
// divided by the difference between its stopping and starting times
// in miles/hour

double track::avg_speed() 
{
   const double meters_per_sec_to_miles_per_hour=2.227;
   return total_length()/(t.back()-t.front())*
      meters_per_sec_to_miles_per_hour;
}

// ---------------------------------------------------------------------
// Member function median_altitude() returns the median value of all
// waypoints' altitudes.

double track::median_altitude() const
{
   vector<double> altitudes;
   for (unsigned int i=0; i<size(); i++)
   {
      altitudes.push_back(posn[i].get(2));
   }
   return mathfunc::median_value(altitudes);
}

// ---------------------------------------------------------------------
// Member function compute_distinct_posns() loops over all vertex
// positions within the current track.  It stores within member STL
// vector distinct_posn those which do not lie on top of each other.

void track::compute_distinct_posns()
{
   distinct_posn.push_back(posn[0]);

   for (unsigned int i=1; i<posn.size(); i++)
   {
      if (!posn[i].nearly_equal(distinct_posn.back()))
      {
         distinct_posn.push_back(posn[i]);
      }
   }
}

// ---------------------------------------------------------------------
// Member function compute_posns_with_distinct_directions() first
// recomputes all track line segments.  It next pushes the first track
// position onto local STL vector posns_with_distinct_directions.
// This method subsequently loops over all intermediate line segments
// and compares their direction vectors with the last one stored in
// distinct_directions.  If they are different, it pushes the
// corresponding track posn onto posns_with_distinct_directions.
// Finally, this method pushes the last track position onto
// posns_with_distinct_directions and returns the STL vector.

vector<threevector> track::compute_posns_with_distinct_directions()
{
//   cout << "inside track::compute_posns_with_distinct_directions()" << endl;

   vector<threevector> posns_with_distinct_directions,distinct_directions;

   compute_segments();
   posns_with_distinct_directions.push_back(posn.front());
   distinct_directions.push_back(segment.front().get_ehat());

   const double SMALL=0.001;
   for (unsigned int i=1; i<segment.size()-1; i++)
   {
      threevector curr_dir=segment[i].get_ehat();
//      cout << "i = " << i 
//           << " curr_dir.x = " << curr_dir.get(0)
//           << " curr_dir.y = " << curr_dir.get(1)
//           << " dd.x = " << distinct_directions.back().get(0)
//           << " dd.y = " << distinct_directions.back().get(1) << endl;
      
      if (!curr_dir.nearly_equal(distinct_directions.back(),SMALL))
      {
         posns_with_distinct_directions.push_back(posn[i]);
         distinct_directions.push_back(curr_dir);
      }
   }

   posns_with_distinct_directions.push_back(posn.back());
   return posns_with_distinct_directions;
}

// ---------------------------------------------------------------------
// Member function compute_posns_with_distinct_velocities() fills
// member STL vectors posns_with_distinct_velocities &
// distinct_velocities with the first and last posn and velocity
// threevector values.  It also includes intermediate track positions
// and velocities if they are not nearly equal to their immediate
// predecessors.

void track::compute_posns_with_distinct_velocities()
{
//   cout << "inside track::compute_posns_with_distinct_velocities()" << endl;

   vector<threevector> distinct_velocities;

   posns_with_distinct_velocities.push_back(posn[0]);
   distinct_velocities.push_back(velocity[0]);

   const double SMALL=0.001;
   for (unsigned int i=1; i<velocity.size()-1; i++)
   {
      if (!velocity[i].nearly_equal(distinct_velocities.back(),SMALL))
      {
         posns_with_distinct_velocities.push_back(posn[i]);
//         cout << "i = " << i << " posn[i] = " << posn[i] << endl;
         distinct_velocities.push_back(velocity[i]);
      }
   }

   posns_with_distinct_velocities.push_back(posn[posn.size()-1]);
   distinct_velocities.push_back(velocity[posn.size()-1]);
}

// ---------------------------------------------------------------------
// Member function compute_average_velocities() loops over all vertex
// positions within the current track.  It uses track positions to
// derive averaged track velocities.

void track::compute_average_velocities()
{
   vector<double> X,Y,Z;
   for (unsigned int i=0; i<posn.size(); i++)
   {
      X.push_back(posn[i].get(0));
      Y.push_back(posn[i].get(1));
      Z.push_back(posn[i].get(2));
   }

   velocity.clear();
   velocity.push_back(Zero_vector);

   const int deriv_order=1;
   for (unsigned int i=1; i<posn.size()-1; i++)
   {
      double Vx=advmath::interpolate_function_or_deriv(
         X,get_earliest_time(),get_latest_time(),get_time(i),deriv_order);
      double Vy=advmath::interpolate_function_or_deriv(
         Y,get_earliest_time(),get_latest_time(),get_time(i),deriv_order);
      double Vz=advmath::interpolate_function_or_deriv(
         Z,get_earliest_time(),get_latest_time(),get_time(i),deriv_order);
      velocity.push_back(threevector(Vx,Vy,Vz));
   }

   velocity.push_back(velocity.back());
   velocity[0]=velocity[1];
}

// ==========================================================================
// Track manipulation member functions
// ==========================================================================

// Member function rescale_time_values() stretches all time entries in
// member STL vector t and STL map *time_map_ptr by input scale_factor
// relative to the track's earliest time.

void track::rescale_time_values(double scale_factor)
{
//   cout << "inside track::rescale_time_values(), scale_factor = "
//        << scale_factor << endl;
   time_map_ptr->clear();

   double t_init=get_earliest_time();
   for (unsigned int i=0; i<t.size(); i++)
   {
      double curr_t=get_time(i);
      double rescaled_t=t_init+scale_factor*(curr_t-t_init);
      set_time(i,rescaled_t);
      (*time_map_ptr)[rescaled_t]=i;
//      cout << "i = " << i << " orig_t = " << curr_t
//           << " new_t = " << get_time(i) << endl;
   }
}

// ---------------------------------------------------------------------
// Member function offset_time_values() adds a constant offset to all
// time entries in member STL vector t and STL map *time_map_ptr.

void track::offset_time_values(double delta_t)
{
//   cout << "inside track::offset_time_values(), delta_t = "
//        << delta_t << endl;
   time_map_ptr->clear();

   for (unsigned int i=0; i<t.size(); i++)
   {
      double orig_t=get_time(i);
      double new_t=orig_t+delta_t;
      set_time(i,new_t);
      (*time_map_ptr)[new_t]=i;
//      cout << "i = " << i << " orig_t = " << orig_t
//           << " new_t = " << get_time(i) << endl;
   }
}

// ==========================================================================
// Interpolation member functions
// ==========================================================================

// Member function get_track_point_index takes in time t and searches
// STL map member *time_map_ptr for a matching time entry.  If one is
// found, the integer track point index corresponding to the input
// time is returned.  (This index may subsequently be used to obtain
// the position and velocity corresponding to the track point labeled
// by input time t.)  Otherwise, this method returns -1.

// Important note added on 7/6/08: STL map's find operation can only
// work for input values of curr_t which actually match independent
// value entries within *time_map_ptr.  This method performs NO
// interpolation of input time values!

int track::get_track_point_index(double curr_t) const
{
//   cout << "inside track::get_track_point_index(), curr_t = " << curr_t 
//        << endl;

   int track_point_index=-1;
   TIME_MAP::iterator time_iter=time_map_ptr->find(curr_t);
   if (time_iter != time_map_ptr->end())
   {
      track_point_index=time_iter->second;
   }

   return track_point_index;
}

// ---------------------------------------------------------------------
// Member function get_interpolated_posn takes an input time.  If the
// input time lies outside the track's temporal interval, this boolean
// method returns false.  Otherwise, it performs a binary search for
// the two bracketing track samples and returns an interpolated
// average of their positions.

bool track::get_interpolated_posn(
   double curr_t,threevector& interpolated_posn) const
{
//   cout << "inside track::get_interpolated_posn(), curr_t = " 
//        << curr_t << endl;

   if (spatially_fixed_flag)
   {
      if (posn.size() > 0)
      {
         interpolated_posn=get_earliest_posn();
         return true;
      }
      else
      {
         return false;
      }
   }

   bool interpolation_successful_flag=true;
   unsigned int index=mathfunc::mylocate(t,curr_t);
   if (index < 0)
   {
      interpolated_posn=posn[0];
      interpolation_successful_flag=false;
   }
   else if (index >= size()-1)
   {
      interpolated_posn=posn[size()-1];
      interpolation_successful_flag=false;
   }
   else
   {
      interpolated_posn=posn[index]+(curr_t-get_time(index))/
         (get_time(index+1)-get_time(index))*
         (posn[index+1]-posn[index]);
   }
   return interpolation_successful_flag;
}

// ---------------------------------------------------------------------
bool track::get_interpolated_statevector(
   double curr_t,statevector& interpolated_statevector) const
{
//   cout << "inside track::get_interpolated_statevector(), this = " 
//        << this << endl;
//   cout << "track_ID = " << get_ID() << endl;
//   cout << "curr_t = " << curr_t << endl;
//   cout << "t.front = " << t.front() << " t.back = " << t.back() << endl;
//   cout << "t.size() = " << t.size() << endl;
//   cout << "time_map_ptr->size() = " << time_map_ptr->size() << endl;
//   cout << "posn.size() = " << posn.size() << endl;
//   cout << "velocity.size() = " << velocity.size() << endl;

   bool interpolation_successful_flag=true;
   interpolated_statevector.set_time(curr_t);
   
   if (spatially_fixed_flag)
   {
      interpolated_statevector.set_position(get_earliest_posn());
      interpolated_statevector.set_velocity(Zero_vector);
   }
   else
   {
      unsigned int index=mathfunc::mylocate(t,curr_t);
//      cout << "index = " << index 
//           << " n_track_pnts = " << size() << endl;

      if (index < 0)
      {
         interpolated_statevector.set_position(get_earliest_posn());
         interpolated_statevector.set_velocity(velocity[0]);

//         cout << "index < 0 : interp failed" << endl;
         interpolation_successful_flag=false;
      }
      else if (index >= size()-1)
      {
         interpolated_statevector.set_position(get_latest_posn());
         interpolated_statevector.set_velocity(
            velocity[size()-1]);
         interpolation_successful_flag=false;
//         cout << "index >= size-1 : interp failed" << endl;
      }
      else
      {
         interpolated_statevector.set_position(
            posn[index]+(curr_t-get_time(index))/
            (get_time(index+1)-get_time(index))*
            (posn[index+1]-posn[index]));
         interpolated_statevector.set_velocity(
            velocity[index]+(curr_t-get_time(index))/
            (get_time(index+1)-get_time(index))*
            (velocity[index+1]-velocity[index]));
      }
   } // spatially_fixed_flag conditional
   
   return interpolation_successful_flag;
}

// ---------------------------------------------------------------------
bool track::get_interpolated_posn_rpy_sensor_aer_FOV(
   double curr_t,threevector& interpolated_posn,
   rpy& interpolated_rpy,threevector& interpolated_sensor_aer,
   twovector& interpolated_sensor_FOV) const
{
//   cout << "inside track::get_interpolated_posn_rpy_sensor_aer_FOV()"
//        << endl;
//   cout << "track_ID = " << get_ID() << endl;
//   cout << "curr_t = " << curr_t << endl;
//   cout << "t.front = " << t.front() << " t.back = " << t.back() << endl;
//   cout << "t.size() = " << t.size() << endl;
//   cout << "time_map_ptr->size() = " << time_map_ptr->size() << endl;
//   cout << "posn.size() = " << posn.size() << endl;

   bool interpolation_successful_flag=true;
   
   unsigned int index=mathfunc::mylocate(t,curr_t);
//      cout << "index = " << index 
//           << " n_track_pnts = " << size() << endl;

   if (index < 0)
   {
      interpolated_posn=get_earliest_posn();
      interpolated_rpy=roll_pitch_yaw.front();
      if (sensor_az_el_roll.size() > 0)
      {
         interpolated_sensor_aer=sensor_az_el_roll.front();
      }
      if (sensor_FOV.size() > 0)
      {
         interpolated_sensor_FOV=sensor_FOV.front();
      }
//         cout << "index < 0 : interp failed" << endl;
      interpolation_successful_flag=false;
   }
   else if (index >= size()-1)
   {
      interpolated_posn=get_latest_posn();
      interpolated_rpy=roll_pitch_yaw.back();
      if (sensor_az_el_roll.size() > 0)
      {
         interpolated_sensor_aer=sensor_az_el_roll.back();
      }
      if (sensor_FOV.size() > 0)
      {
         interpolated_sensor_FOV=sensor_FOV.back();
      }
      interpolation_successful_flag=false;
//         cout << "index >= size-1 : interp failed" << endl;
   }
   else
   {
      interpolated_posn=
         posn[index]+(curr_t-get_time(index))/
         (get_time(index+1)-get_time(index))*
         (posn[index+1]-posn[index]);
      interpolated_rpy=
         roll_pitch_yaw[index]+(curr_t-get_time(index))/
         (get_time(index+1)-get_time(index))*
         (roll_pitch_yaw[index+1]-roll_pitch_yaw[index]);
      if (sensor_az_el_roll.size() > 0)
      {
         interpolated_sensor_aer=
            sensor_az_el_roll[index]+(curr_t-get_time(index))/
            (get_time(index+1)-get_time(index))*
            (sensor_az_el_roll[index+1]-sensor_az_el_roll[index]);
      }
      if (sensor_FOV.size() > 0)
      {
         interpolated_sensor_FOV=
            sensor_FOV[index]+(curr_t-get_time(index))/
            (get_time(index+1)-get_time(index))*
            (sensor_FOV[index+1]-sensor_FOV[index]);
      }
   }
   
   return interpolation_successful_flag;
}

// ==========================================================================
// Track segment member functions
// ==========================================================================

// Member function compute_segments fills member STL vector segment
// with linesegments connecting each of the positions along the track.
// If input boolean twoD_flag==true, all data in the 3rd entries of
// the position member variables is suppressed.

void track::compute_segments(bool twoD_flag)
{
   segment.clear();
   for (unsigned int i=0; i<size()-1; i++) 
   {
      if (twoD_flag)
      {
         segment.push_back(linesegment (threevector(twovector(posn[i])),
                                        threevector(twovector(posn[i+1]))) );
      }
      else
      {
         segment.push_back(linesegment(posn[i],posn[i+1]));
      }
   }
   total_length();
}

// ---------------------------------------------------------------------
// Member function segment_number takes in fraction 0 <= frac <= 1 and
// returns the number of the track segment on which the point whose
// length from the starting position equals frac*total_length():

unsigned int track::segment_number(double frac)
{
   bool segment_found=false;
   unsigned int segment_number=0;
   double running_length=0;
   do
   {
      if (frac*length >= running_length &&
          frac*length < running_length + segment[segment_number].get_length())
      {
         segment_found=true;
      }
      else
      {
         running_length += segment[segment_number].get_length();
         segment_number++;
      }
   }
   while(!segment_found && segment_number < size());
   return segment_number;
}

// ---------------------------------------------------------------------
// Member function segment_point takes in fraction 0 <= frac <= 1 and
// returns the corresponding point along the track whose length from
// the starting point equals frac*total_length():

void track::segment_point(double frac,threevector& curr_point) 
{
   unsigned int n=segment_number(frac);
   double running_length=0;
   for (unsigned int i=0; i<n; i++)
   {
      running_length += segment[i].get_length();
   }

// Next compute residual fractional length of desired point on its
// segment:

   double curr_segment_frac=(frac*length-running_length)/segment[n].
      get_length();
   curr_point=threevector(
      segment[n].get_v1()+curr_segment_frac*segment[n].get_length()*
      segment[n].get_ehat());
}

// ---------------------------------------------------------------------
// Member function interpolated_posns takes in spacing increment ds.
// It starts at the track's initial point and returns every subsequent
// point along the track spaced apart by ds within an output STL vector.

void track::interpolated_posns(double ds,vector<threevector>& interp_posn) 
{
   interp_posn.clear();
   unsigned int nsteps=basic_math::round(length/ds);

   threevector curr_point;
   for (unsigned int i=0; i<nsteps; i++)
   {
      double frac=double(i)/double(nsteps);
      segment_point(frac,curr_point);
      interp_posn.push_back(curr_point);
   }
}

// ---------------------------------------------------------------------
// Member function closest_approach_time takes in some point of
// interest (POI).  After performing a brute-force loop over the
// track's positions, it returns the time corresponding to the
// position closest to the POI.

double track::closest_approach_time(const threevector& POI)
{
   double closest_approach_distance=POSITIVEINFINITY;
   double closest_time=NEGATIVEINFINITY;
   for (unsigned int i=0; i<posn.size(); i++)
   {
      double curr_approach_distance=(posn[i]-POI).magnitude();
      if (curr_approach_distance < closest_approach_distance)
      {
         closest_approach_distance=curr_approach_distance;
         closest_time=get_time(i);
      }
   }
   return closest_time;
}

// ==========================================================================
// Track intersection member functions
// ==========================================================================

// Member function intercepts_bbox() assumes that member STL vector
// distinct_posn has been filled via a prior call to
// compute_distinct_posns().  It performs a brute force search for any
// distinct posns along the current track object which lie inside the
// bounding box passed as an input parameter.  If so, this boolean
// method returns true.

bool track::intercepts_bbox(const bounding_box& bbox)
{
   for (unsigned int i=0; i<distinct_posn.size(); i++)
   {
      if (bbox.point_inside(distinct_posn[i].get(0),distinct_posn[i].get(1)))
      {
         return true;
      }
   }
   return false;
}

bool track::intercepts_bbox_polyline(const polyline* polyline_ptr)
{
   bounding_box curr_bbox(polyline_ptr);
   return intercepts_bbox(curr_bbox);
}

// ==========================================================================
// ActiveMQ broadcast member functions
// ==========================================================================

// Member function broadcast_statevector() constructs key-value
// string pairs containing track statevector information.  We wrote
// this specialized utility to communicate with Luca Bertucelli's
// MATLAB Multi-UAV C2 simulator.

bool track::broadcast_statevector(
   double secs_elapsed_since_epoch,Messenger* Messenger_ptr,
   colorfunc::Color curr_color,string ID_label) const
{
//   cout << "inside track::broadcast_statevector() #1" << endl;
   threevector grid_origin=Zero_vector;
   return broadcast_statevector(
      secs_elapsed_since_epoch,&grid_origin,Messenger_ptr,curr_color,ID_label);
}

bool track::broadcast_statevector(
   double secs_elapsed_since_epoch,const threevector* grid_origin_ptr,
   Messenger* Messenger_ptr,colorfunc::Color curr_color,string ID_label)
           const
{
//   cout << "inside track::broadcast_statevector() #2" << endl;

   statevector curr_statevector;
   bool interp_flag=get_interpolated_statevector(
      secs_elapsed_since_epoch,curr_statevector);
   
   if (interp_flag)
   {
      broadcast_statevector(
         curr_statevector,grid_origin_ptr,Messenger_ptr,curr_color,ID_label);
   }
   else
   {
//      cout << "inside track::broadcast_statevector(), track_ID = " 
//           << get_ID() << endl;
//      cout << "secs_elapsed = " << secs_elapsed_since_epoch << endl;
//      cout << "get_interpolated_statevector() returned false!" << endl;
//      outputfunc::enter_continue_char();
   }
   return interp_flag;
}

// ---------------------------------------------------------------------
// This overloaded version of member function
// broadcast_statevector() generates an ActiveMQ message for the
// current input statevector which may correspond to a ground target,
// KOZ bounding box or aerial mover.

void track::broadcast_statevector(
   const statevector& curr_statevector,const threevector* grid_origin_ptr,
   Messenger* Messenger_ptr,colorfunc::Color curr_color,string ID_label) const
{
//   cout << "inside track::broadcast_statevector() #3" << endl;

// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   vector<Messenger::Property> properties;

   command="SEND_STATEVECTOR";	

   key="ID";
//   cout << "get_label_ID() = "
//        << get_label_ID() << endl;
//   cout << "get_ID() = "
//        << get_ID() << endl;

   if (ID_label=="")
   {
      int ID=get_label_ID();
      if (ID==-1) ID=get_ID();
      value=stringfunc::number_to_string(ID);
   }
   else
   {
      value=ID_label;
   }
   properties.push_back(Messenger::Property(key,value));

   string track_label=get_label();
   if (track_label.size() > 0)
   {
      key="Label";
      value=track_label;
      properties.push_back(Messenger::Property(key,value));
   }
   
   key="Secondary_ID";
   value=stringfunc::number_to_string(get_ID());
   properties.push_back(Messenger::Property(key,value));

   key="Time";
   value=stringfunc::number_to_string(curr_statevector.get_time());
   properties.push_back(Messenger::Property(key,value));
 
   key="Type";
   value=get_description();
   properties.push_back(Messenger::Property(key,value));
//   cout << "value = " << value << endl;

   if (value=="Keep Out Zone")
   {
      command="SEND_KOZ_BBOX";	

      bounding_box KOZ_bbox;
//      bool flag=
         get_bbox(0,KOZ_bbox);
//      cout << "KOZ bbox = " << KOZ_bbox << endl;

      key="Xlo";
      value=stringfunc::number_to_string(
         KOZ_bbox.get_xmin()-grid_origin_ptr->get(0),10);
      properties.push_back(Messenger::Property(key,value));

      key="Ylo";
      value=stringfunc::number_to_string(
         KOZ_bbox.get_ymin()-grid_origin_ptr->get(1),10);
      properties.push_back(Messenger::Property(key,value));
   
      key="Zlo";
      value=stringfunc::number_to_string(
         KOZ_bbox.get_zmin()-grid_origin_ptr->get(2),10);
      properties.push_back(Messenger::Property(key,value));

      key="Xhi";
      value=stringfunc::number_to_string(
         KOZ_bbox.get_xmax()-grid_origin_ptr->get(0),10);
      properties.push_back(Messenger::Property(key,value));

      key="Yhi";
      value=stringfunc::number_to_string(
         KOZ_bbox.get_ymax()-grid_origin_ptr->get(1),10);
      properties.push_back(Messenger::Property(key,value));

      key="Zhi";
      value=stringfunc::number_to_string(
         KOZ_bbox.get_zmax()-grid_origin_ptr->get(2),10);
      properties.push_back(Messenger::Property(key,value));
   }
   else
   {
      key="X";
      value=stringfunc::number_to_string(
         curr_statevector.get_position().get(0)-grid_origin_ptr->get(0),10);
      properties.push_back(Messenger::Property(key,value));

      key="Y";
      value=stringfunc::number_to_string(
         curr_statevector.get_position().get(1)-grid_origin_ptr->get(1),10);
      properties.push_back(Messenger::Property(key,value));

      key="Z";
      value=stringfunc::number_to_string(
         curr_statevector.get_position().get(2)-grid_origin_ptr->get(2),10);
      properties.push_back(Messenger::Property(key,value));
   } // value=="Keep Out Zone" conditional
   
   key="Vx";
   value=stringfunc::number_to_string(
      curr_statevector.get_velocity().get(0),10);
   properties.push_back(Messenger::Property(key,value));

   key="Vy";
   value=stringfunc::number_to_string(
      curr_statevector.get_velocity().get(1),10);
   properties.push_back(Messenger::Property(key,value));

   key="Vz";
   value=stringfunc::number_to_string(
      curr_statevector.get_velocity().get(2),10);
   properties.push_back(Messenger::Property(key,value));

   string RRGGBB_hex=colorfunc::RGB_to_RRGGBB_hex(
      colorfunc::get_RGB_values(curr_color));
   key="Color";
   value="#"+RRGGBB_hex;
   properties.push_back(Messenger::Property(key,value));

   key="Priority";
   value=stringfunc::number_to_string(1.0);
   properties.push_back(Messenger::Property(key,value));

//   for (unsigned int p=0; p<properties.size(); p++)
//   {
//      Messenger::Property curr_property(properties[p]);
//      cout << p << "  "
//           << "key = " << curr_property.first
//           << " value = " << curr_property.second
//           << endl;
//   }

   Messenger_ptr->broadcast_subpacket(command,properties);
}

// ==========================================================================
// SQL input/output member functions
// ==========================================================================

// Member function write_SQL_insert_track_commands() loops over all
// points within the current track.  We assume that the position STL
// vector has previously been filled with longitude, latitude and
// altitude (rather than UTM easting, northing and altitude)
// coordinate values.  This method writes SQL insert commands to the specified
// SQL_filename which are needed to populate the photo table of the
// data_network postgis database.

void track::write_SQL_insert_track_commands(
   int fieldtest_ID,int mission_ID,int platform_ID,int sensor_ID,
   string SQL_track_points_filename)
{
//   cout << "inside track::write_SQL_insert_track_commands()" << endl;
//   cout << "SQL_track_points_filename = " << SQL_track_points_filename << endl;

   vector<string> insert_commands=generate_SQL_insert_track_commands(
      fieldtest_ID,mission_ID,platform_ID,sensor_ID);

   ofstream SQL_track_stream;
   filefunc::openfile(SQL_track_points_filename,SQL_track_stream);
   for (unsigned int i=0; i<insert_commands.size(); i++)
   {
      SQL_track_stream << insert_commands[i] << endl;
   }
   filefunc::closefile(SQL_track_points_filename,SQL_track_stream);

   string banner="Track points written to SQL file "+SQL_track_points_filename;
   outputfunc::write_big_banner(banner);
}

// ---------------------------------------------------------------------
// Member function generate_SQL_insert_track_commands() loops over all
// points within the current track.  We assume that the position STL
// vector has previously been filled with longitude, latitude and
// altitude (rather than UTM easting, northing and altitude)
// coordinate values.  This method writes SQL insert commands to an
// STL vector which is returned.

vector<string> track::generate_SQL_insert_track_commands(
   int fieldtest_ID,int mission_ID,int platform_ID,int sensor_ID)
{
   cout << "inside track::generate_SQL_insert_track_commands()" << endl;

   vector<string> insert_commands;

// Generate SQL insert commands for track points:

   for (unsigned int point_order=0; point_order<t.size(); point_order++)
   {
      int curr_fix_quality=fix_quality[point_order];
      int curr_n_satellites=n_satellites[point_order];
      double horizontal_dilution=horiz_dilution[point_order];

      double X=posn[point_order].get(0);
      double Y=posn[point_order].get(1);

      double longitude=X;
      double latitude=Y;

// In mid-Aug 2010, program GPSDEVICE mistakenly started writing UTM
// easting and northing values rathern than lon-lat geocoordinates.
// So as a simple cluge, we'll assume any Y value exceeding 90 degrees
// in absolute value indicates that an (easting,northing) pair in UTM
// zone 19 was recorded and needs to be converted to a (lon,lat) pair:

      if (fabs(Y) > 90)
      {
         bool northern_hemisphere_flag=true;
         int UTM_zone=19;	// Boston
         geopoint curr_point(northern_hemisphere_flag,UTM_zone,X,Y);
         longitude=curr_point.get_longitude();
         latitude=curr_point.get_latitude();
      }

      double altitude=posn[point_order].get(2);

      double roll=0;
      double pitch=0;
      double yaw=0;
      if (roll_pitch_yaw.size() > 0)
      {
         rpy curr_rpy(roll_pitch_yaw[point_order]);
         roll=curr_rpy.get_roll();
         pitch=curr_rpy.get_pitch();
         yaw=curr_rpy.get_yaw();
      }

      string curr_insert_cmd=
         mover_func::generate_insert_track_point_SQL_command(
            fieldtest_ID,mission_ID,platform_ID,sensor_ID,t[point_order],
            curr_fix_quality,curr_n_satellites,horizontal_dilution,
            longitude,latitude,altitude,roll,pitch,yaw);
      insert_commands.push_back(curr_insert_cmd);
   }
   return insert_commands;
}


