// ==========================================================================
// GRAPHICAL class member function definitions
// ==========================================================================
// Last modified on 6/3/09; 7/8/09; 1/21/13; 4/6/14
// ==========================================================================

#include <iterator>
#include <string>
#include <osg/Geode>
#include "osg/osgGraphicals/AnimationController.h"
#include "math/basic_math.h"
#include "math/constant_vectors.h"
#include "math/fourvector.h"
#include "math/genmatrix.h"
#include "osg/osgGraphicals/Graphical.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"

#include "general/outputfuncs.h"
#include "osg/osgfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Graphical::allocate_member_objects()
{
   coords_map_ptr=new COORDS_MAP;
   PAT_refptr=new osg::PositionAttitudeTransform();
   depth_off_refptr=new osg::Depth(osg::Depth::ALWAYS, 0.0, 0.01);
   depth_on_refptr=new osg::Depth(osg::Depth::LESS, 0.0, 1.0);
}		       

void Graphical::initialize_member_objects()
{
   ndims_label="_"+stringfunc::number_to_string(ndims)+"D";
   Graphical_name="";
   stationary_Graphical_flag=true;
   AnimationController_ptr=NULL;
}		       

// Note added on 10/13/07: Recall Graphical constructor taking no
// arguments is needed for object pooling

Graphical::Graphical()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

Graphical::Graphical(const int p_ndims,int id,AnimationController* AC_ptr):
   ndims(p_ndims)
{	
//    cout << "inside Graphical constructor" << endl;
   initialize_member_objects();
   allocate_member_objects();
   ID=id;
   AnimationController_ptr=AC_ptr;
}		       

Graphical::~Graphical()
{
//   cout << "inside Graphical destructor" << endl;
//   cout << "Graphical_name = " << Graphical_name << endl;
//   cout << "this = " << this << endl;

//   cout << "PAT_refptr.get() = " << PAT_refptr.get() << endl;
//   cout << "PAT_refptr->referenceCount() = "
//        << PAT_refptr->referenceCount() << endl;

   delete coords_map_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Graphical& g)
{
//   outstream << "inside Graphical::operator<<" << endl;
   outstream << "Graphical ID = " << g.ID << endl;
   return outstream;
}

// ---------------------------------------------------------------------
// Member functions set_coords_obs

void Graphical::set_coords_obs(
   double t,int pass_number,const instantaneous_obs& curr_obs)
{
//   cout << "inside Graphical::set_coords_obs(), t = " << t << endl;
//   outputfunc::enter_continue_char();

   twovector key(t,pass_number);
   COORDS_MAP::iterator coords_iter=coords_map_ptr->find(key);
   if (coords_iter != coords_map_ptr->end())
   {
      coords_iter->second.first=curr_obs;
   }
   else
   {
      Triple<instantaneous_obs,bool,bool> t;
      t.first=curr_obs;
      (*coords_map_ptr)[key]=t;
   }

   if (get_stationary_Graphical_flag())
   {
      check_Graphical_stationarity(pass_number,curr_obs);
   }
}

// ---------------------------------------------------------------------
// Member function check_Graphical_stationarity retrieves the
// UVW_coords, Quat and UVW_scales for the zeroth instantaneous
// observation.  If they do not match those of the input current
// observation, this method sets the Graphical's
// stationary_Graphical_flag to false.

void Graphical::check_Graphical_stationarity(
   int pass_number,const instantaneous_obs& curr_obs) 
{
//   cout << "inside Graphical::check_Graphical_stationarity()" << endl;
//   cout << "Graphical name = " << Graphical_name << endl;

   COORDS_MAP::iterator map_iter=coords_map_ptr->begin(); 

   threevector init_UVW_coords=
      map_iter->second.first.retrieve_UVW_coords(pass_number);
   fourvector init_Quat(0,0,0,1);
   osg::Quat quat;
   if (!map_iter->second.first.retrieve_quaternion(pass_number,quat))
   {
      init_Quat=fourvector(quat._v[0],quat._v[1],quat._v[2],quat._v[3]);
   }
//   threevector init_UVW_scale=
//      map_iter->second.first.retrieve_scale(pass_number);

   threevector curr_UVW_coords=curr_obs.retrieve_UVW_coords(pass_number);
   fourvector curr_Quat(0,0,0,1);
   if (curr_obs.retrieve_quaternion(pass_number,quat))
   {
      curr_Quat=fourvector(quat._v[0],quat._v[1],quat._v[2],quat._v[3]);
   }

// If curr_UVW_coords and/or init_UVW_coords contain
// NEGATIVEINFINITY values, we should not use them to asess Graphical
// stationarity:

   double curr_UVW_coords_sum=curr_UVW_coords.get(0)+
     curr_UVW_coords.get(1)+curr_UVW_coords.get(2);
   double init_UVW_coords_sum=init_UVW_coords.get(0)+
     init_UVW_coords.get(1)+init_UVW_coords.get(2);

//   cout << "curr_UVW_coords = " << curr_UVW_coords << endl;
//   cout << "init_UVW_coords = " << init_UVW_coords << endl;
//   cout << "curr_QUAT = " << curr_Quat << endl;
//   cout << "init_QUAT = " << init_Quat << endl;

   if (curr_UVW_coords_sum < NEGATIVEINFINITY &&
       init_UVW_coords_sum < NEGATIVEINFINITY)
   {
      return;
   }

   if (!curr_UVW_coords.nearly_equal(init_UVW_coords) ||
       !curr_Quat.nearly_equal(init_Quat) )
   {
      set_stationary_Graphical_flag(false);
//      cout << "stationary_Graphical_flag = " 
//           << get_stationary_Graphical_flag() << endl;
//      cout << "curr_UVW_coords = " << curr_UVW_coords << endl;
//      cout << "init_UVW_coords = " << init_UVW_coords << endl;
//      cout << "curr_quat = " << curr_Quat << endl;
//      cout << "init_quat = " << init_Quat << endl;
   }
}

// ==========================================================================
// Instantaneous observation manipulation member functions
// ==========================================================================

// Member function get_particular_time_obs checks whether an entry
// corresponding to input time curr_t and pass_number already exists
// within STL member map *coords_map_ptr.  If so, it returns a pointer
// to the entry's instantaneous observation object.  Otherwise, it
// returns NULL.

instantaneous_obs* Graphical::get_particular_time_obs(
   double t,int pass_number) const
{
//   cout << "inside Graphical::get_particular_time_obs()" << endl;
//   cout << "t = " << t << " pass_number = " << pass_number << endl;
   COORDS_MAP::iterator coords_iter=get_coords_map_iterator(t,pass_number);
   if (coords_iter != coords_map_ptr->end())
   {
      return &(coords_iter->second.first);
   }
   else
   {
      return NULL;
   }
}

// ---------------------------------------------------------------------
// Member function get_all_particular_time_observations takes in a
// time t.  It iterates over all entries within *coords_map_ptr and
// extracts those whose times match t.  This method returns a
// consolidated instantaneous observation which corresponds to time t.

instantaneous_obs* Graphical::get_all_particular_time_observations(double t) 
   const
{
//   cout << "inside Graphical::get_all_particular_time_observations()" << endl;
//   cout << "t = " << t << endl;
//   cout << "coords_map_ptr->size() = " << coords_map_ptr->size() << endl;

   vector<instantaneous_obs*> instantaneous_observation_ptrs;

   for (COORDS_MAP::iterator iter=coords_map_ptr->begin();
        iter != coords_map_ptr->end(); iter++)
   {
      double curr_t=iter->first.get(0);
//      int curr_passnumber=iter->first.get(1);
//      cout << "curr_t = " << curr_t
//           << " curr_passnumber = " << curr_passnumber << endl;
      if (nearly_equal(curr_t,t))
      {
         instantaneous_obs* obs_ptr=&(iter->second.first);
         instantaneous_observation_ptrs.push_back(obs_ptr);
      }
   } // iter over all elements of *coords_map_ptr

   if (instantaneous_observation_ptrs.size()==0) return NULL;
   
   instantaneous_obs* obs_ptr=instantaneous_observation_ptrs[0];
   for (unsigned int i=1; i<instantaneous_observation_ptrs.size(); i++)
   {
      obs_ptr->consolidate_image_coords(instantaneous_observation_ptrs[i]);
   }

   return obs_ptr;
}

// ---------------------------------------------------------------------
vector<instantaneous_obs*> Graphical::get_all_observations() const
{
//   cout << "inside Graphical::get_all_observations()" << endl;

   vector<instantaneous_obs*> instantaneous_observations;
   for (COORDS_MAP::iterator iter=coords_map_ptr->begin();
        iter != coords_map_ptr->end(); iter++)
   {
      instantaneous_obs* obs_ptr=&(iter->second.first);
      instantaneous_observations.push_back(obs_ptr);
   } // iter over all elements of *coords_map_ptr
   return instantaneous_observations;
}

// ---------------------------------------------------------------------
// Member function consolidate_instantaneous_observations takes in
// time t along with *other_graphical_ptr.  It constructs the union of
// the instantaneous observations for both graphicals corresponding to
// time t.

void Graphical::consolidate_instantaneous_observations(
   double t,Graphical* other_graphical_ptr)
{
//   cout << "inside Graphical::consolidate_instantaneous_observations()"
//        << endl;

   instantaneous_obs* other_instantaneous_obs_ptr=
      other_graphical_ptr->get_all_particular_time_observations(t);
   if (other_instantaneous_obs_ptr==NULL) return;

   instantaneous_obs* instantaneous_obs_ptr=
      get_all_particular_time_observations(t);
   if (instantaneous_obs_ptr==NULL)
   {
      instantaneous_obs::THREEVECTOR_MAP* other_multiple_image_coords_ptr=
         other_instantaneous_obs_ptr->get_multiple_image_coords_ptr();

      for (instantaneous_obs::THREEVECTOR_MAP::iterator iter=
              other_multiple_image_coords_ptr->begin();
           iter != other_multiple_image_coords_ptr->end(); iter++)
      {
         set_UVW_coords(t,iter->first,iter->second);
      }
   }
   else
   {
      instantaneous_obs_ptr->consolidate_image_coords(
         other_instantaneous_obs_ptr);
   }
}

// ==========================================================================
// Drawing member functions
// ==========================================================================

// Member function adjust_depth_buffering implements Ross Anderson's
// Dec 2006 idea for trumping depth buffering and forcing Graphical
// objects to always be displayed even if they are actually occluded
// by other objects located in front of them.

void Graphical::adjust_depth_buffering(
   bool force_display_flag,osg::Geode* geode_ptr)
{
   osg::StateSet* stateset_ptr=geode_ptr->getOrCreateStateSet();
   stateset_ptr->clear();

   if (force_display_flag)
   {
      stateset_ptr->setAttributeAndModes(
         depth_off_refptr.get(),osg::StateAttribute::ON);
//      stateset_ptr->setAttributeAndModes(
//         depth_on_refptr.get(),osg::StateAttribute::OFF);
   }
   else
   {
      stateset_ptr->setAttributeAndModes(
         depth_on_refptr.get(),osg::StateAttribute::ON);
//      stateset_ptr->setAttributeAndModes(
//         depth_off_refptr.get(),osg::StateAttribute::OFF);
   }

//   cout << "at end of Graphical::adjust_depth_buffering" << endl;
}

// ---------------------------------------------------------------------
void Graphical::dirtyDisplay()
{
   cout << "inside Graphical::dirtyDisplay()" << endl;
}

// ==========================================================================
// Animation member functions
// ==========================================================================

int Graphical::get_curr_framenumber() const
{
//   cout << "inside Graphical::get_curr_framenumber()" << endl;
//   cout << "this = " << this << endl;
   if (AnimationController_ptr != NULL)
   {
      return AnimationController_ptr->get_curr_framenumber();
   }
   else
   {
      return 0;
   }
}

int Graphical::get_first_framenumber() const
{
//   cout << "inside Graphical::get_first_framenumber()" << endl;
   if (AnimationController_ptr != NULL)
   {
      return AnimationController_ptr->get_first_framenumber();
   }
   else
   {
      return 0;
   }
}

int Graphical::get_last_framenumber() const
{
//   cout << "inside Graphical::get_first_framenumber()" << endl;
   if (AnimationController_ptr != NULL)
   {
      return AnimationController_ptr->get_last_framenumber();
   }
   else
   {
      return 0;
   }
}

// --------------------------------------------------------------------------
// As of 6/5/05, we simply set the time associated with each image in
// equal to its imagenumber.  This will eventually need to be
// generalized so that the time field corresponds to a true temporal
// measurement...

double Graphical::get_curr_t() const
{
//   cout << "inside Graphical::get_curr_t(), curr_t = "
//        << get_curr_framenumber() 
//        << " Graphical = " << Graphical_name << endl;
   return static_cast<double>(get_curr_framenumber());
}

double Graphical::get_initial_t() const
{
   return static_cast<double>(get_first_framenumber());
}

// ==========================================================================
// PAT methods
// ==========================================================================

// Member function set_PAT takes in a time t and pass number.  It
// fetches the position, attitude and scale information corresponding
// to these input parameters from the instantaneous_obs COORDS_MAP.
// The current Graphical's PAT is then updated based upon this
// information.

void Graphical::set_PAT(double t,int pass_number)
{
//   if (Graphical_name=="Cylinder")
//   {
 //     cout << endl;
//      cout << "inside Graphical::set_PAT()" << endl;
//      cout << "Graphical name = " << Graphical_name 
//           << " ID = " << get_ID() << endl;
//      cout << "this = " << this << endl;
//      cout << "PAT_ptr = " << PAT_refptr.get() << endl;
//      cout << "get_curr_t() = " << get_curr_t() << endl;
//      cout << "stationary_Graphical_flag = " 
//           << get_stationary_Graphical_flag() << endl;
//      cout << "get_mask() = " << get_mask(t,pass_number) << endl;
//   }
 
   t=get_curr_t();
   
// If Graphical has been declared to be stationary, it should exhibit
// no time dependence.  In this case, reset input time to zero:
   
   if (get_stationary_Graphical_flag())
   {
      t=get_initial_t();
   }

//   cout << "t = " << t << " pass_number = " << pass_number
//        << " get_mask = " << get_mask(t,pass_number) << endl;
   
   if (get_mask(t,pass_number))
   {
      get_PAT_ptr()->setNodeMask(0);	// mask enabled
      return;
   }
   else
   {
      get_PAT_ptr()->setNodeMask(1);
   }
   
   threevector posn(NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
   threevector scale(1,1,1);
   osg::Quat attitude(0,0,0,1);

   get_UVW_coords(t,pass_number,posn);
   get_quaternion(t,pass_number,attitude);
   get_scale(t,pass_number,scale);

   update_PAT_scale(scale);
   update_PAT_attitude(attitude);
   update_PAT_posn(posn);

//   if (Graphical_name=="Cylinder")
//   {
//      cout << "At end of Graphical::set_PAT()" << endl;
//      cout << "Graphical_name = " << Graphical_name << endl;
//      cout << "stationary_flag = " << get_stationary_Graphical_flag()
//           << endl;
//      cout << " t = " << t << " pass = " << pass_number
//           << " get_mask = " << get_mask(pass_number,t) << endl;
//      cout << "posn = " << posn << endl;
//      cout << "quat = " << attitude._v[0] << "," << attitude._v[1]
//           << "," << attitude._v[2] << "," << attitude._v[3] << endl;
//      cout << "Scale = " << scale << endl;
//      outputfunc::enter_continue_char();
//   }

}

void Graphical::set_PAT_pivot(const threevector& p)
{
   if (ndims==2)
   {
      PAT_refptr->setPivotPoint(osg::Vec3d(p.get(0),0,p.get(1)));
   }
   else if (ndims==3)
   {
      PAT_refptr->setPivotPoint(osg::Vec3d(p.get(0),p.get(1),p.get(2)));
   }
}

// ==========================================================================
// Coordinate set & get methods
// ==========================================================================

// Member function set_UVW_scales takes in time curr_t and first
// checks whether a node corresponding to this time exists within
// *coords_map_ptr.  If so, it uses the input values to reset
// the worldspace scale factors U_scale, V_scale and W_scale
// associated with direction vectors Uhat, Vhat and What at time
// curr_t.

void Graphical::set_UVW_scales(
   double curr_t,int pass_number,
   double Uscale,double Vscale,double Wscale)
{
   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);

// If node corresponding to time curr_t does not exist in
// *coords_map_ptr, instantiate a new one and append it to the list:

   if (curr_obs_ptr==NULL)
   {
      cout << "Error in Graphical::set_UVW_scales()" << endl;
      cout << "curr_obs_ptr = NULL" << endl;
      cout << "curr_t = " << curr_t << " pass_number = " << pass_number
           << endl;
      cout << "Uscale = " << Uscale << " Vscale = " << Vscale
           << " Wscale = " << Wscale << endl;
   }
   else
   {
      curr_obs_ptr->set_UVW_scales(Uscale,Vscale,Wscale);
   }
}

// ---------------------------------------------------------------------
// Member function get_UVW_scales takes in time value curr_t.  If no
// node corresponding to this time exists within
// *coords_map_ptr, this boolean method returns false.
// Otherwise, it returns the stored set of UVW scale factors
// corresponding to the input video pass and image numbers.

bool Graphical::get_UVW_scales(
   double curr_t,int pass_number,
   double& Uscale,double& Vscale,double& Wscale) const
{
   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);

   if (curr_obs_ptr==NULL)
   {
      return false;
   }
   else
   {
      curr_obs_ptr->get_UVW_scales(Uscale,Vscale,Wscale);
      return true;
   }
}

// ---------------------------------------------------------------------
// Member function set_UVW_dirs takes in time curr_t and first checks
// whether a node corresponding to this time exists within
// *coords_map_ptr.  If so, it uses the input threevectors to
// reset the worldspace direction vectors Uhat and Vhat associated
// with time curr_t.

void Graphical::set_UVW_dirs(
   double curr_t,int pass_number,
   const threevector& Uhat,const threevector& Vhat)
{
//   cout << "inside Graphical::set_UVW_dirs()" << endl;
   
   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);

// If node corresponding to time curr_t does not exist in
// *coords_map_ptr, instantiate a new one and append it to the list:

   if (curr_obs_ptr==NULL)
   {
      cout << "Error in Graphical::set_UVW_dirs()" << endl;
      cout << "curr_obs_ptr = NULL" << endl;
      cout << "curr_t = " << curr_t << " pass_number = " << pass_number
           << endl;
      cout << "Uhat = " << Uhat << " Vhat = " << Vhat << endl;
   }
   else
   {
      curr_obs_ptr->set_UVW_worldspace_dirs(Uhat,Vhat);
   }
}

// ---------------------------------------------------------------------
// Member function get_UVW_dirs takes in time value curr_t.  If no
// node corresponding to this time exists within
// *coords_map_ptr, this boolean method returns false.
// Otherwise, it returns the stored set of UVW direction vectors
// measured relative to world XYZ space corresponding to the input
// video pass and image numbers.  

bool Graphical::get_UVW_dirs(
   double curr_t,int pass_number,
   threevector& Uhat,threevector& Vhat,threevector& What) const
{
   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);

   if (curr_obs_ptr==NULL)
   {
      return false;
   }
   else
   {
      curr_obs_ptr->get_UVW_worldspace_dirs(Uhat,Vhat,What);
      return true;
   }
}

// ---------------------------------------------------------------------
// Member function set_UVW_coords takes in time curr_t and first
// checks whether a node corresponding to this time exists within
// *coords_map_ptr.  If not, it generates a node.  This method next
// checks whether (U,V,W) coordinates corresponding to input video
// pass and image numbers already exist within curr_coords UVW list.
// If they do not, it appends them to the multiple_image_coords STL
// vector.  Otherwise, it simply changes the threevector value within
// multiple_image_coords to equal input threevector p.

void Graphical::set_UVW_coords(
   double curr_t,int pass_number,const threevector& p3)
{
//   cout << "inside Graphical::set_UVW_coords(), curr_t = " << curr_t
//        <<  endl << " get_curr_t() = "
//        << get_curr_t() << endl;
//   cout << "ID = " << get_ID() << " pass_number = " << pass_number 
//        << " p3 = " << p3 << endl;
//   cout << "Graphical = " << Graphical_name << endl;
   
   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);
//   cout << "curr_obs_ptr = " << curr_obs_ptr << endl;
//   if (curr_obs_ptr != NULL)
//      cout << "*curr_obs_ptr = " << *curr_obs_ptr << endl;

   if (curr_obs_ptr==NULL)
   {
      instantaneous_obs curr_obs(curr_t);
      curr_obs.insert_UVW_coords(pass_number,p3);

// Assign default attitude, scale, score and index values to current
// instantaneous observation:

      curr_obs.insert_quaternion(pass_number,osg::Quat(0,0,0,1));
      curr_obs.insert_scale(pass_number,threevector(1,1,1));
      curr_obs.insert_score(pass_number,NEGATIVEINFINITY);
      curr_obs.insert_index(pass_number,-1);
      set_coords_obs(curr_t,pass_number,curr_obs);
   }
   else
   {
      curr_obs_ptr->change_UVW_coords(pass_number,p3);
   }
}

// ---------------------------------------------------------------------
// Boolean member function get_UVW_coords takes in time value curr_t.
// If no node corresponding to this time exists within
// *coords_map_ptr, this method returns false.  Otherwise, it
// searches for a stored set of UVW image coordinates corresponding to
// the input video pass and image numbers.  If no such coordinates
// exist, this method returns false.  Otherwise, it returns the UVW
// coordinates within threevector p.

bool Graphical::get_UVW_coords(double curr_t,int pass_number,threevector& p) 
   const
{
//   cout << "inside Graphical::get_UVW_coords()" << endl;
   p=threevector(NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);

   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);
//   cout << "curr_obs_ptr = " << curr_obs_ptr << endl;

   if (curr_obs_ptr==NULL)
   {
      return false;
   }
   else
   {
      return curr_obs_ptr->retrieve_UVW_coords(pass_number,p);
   }
}

// ---------------------------------------------------------------------
// Member function set_UVW_velocity() takes in time curr_t along with
// pass_number and first checks whether a node corresponding to this
// time exists in *coords_map_ptr.  If so, it changes the threevector
// value within multiple_image_velocity to equal input threevector v3.

void Graphical::set_UVW_velocity(
   double curr_t,int pass_number,const threevector& v3)
{
//   cout << "inside Graphical::set_UVW_velocity(), curr_t = " << curr_t
//        <<  endl << " get_curr_t() = "
//        << get_curr_t() << endl;
//   cout << "Graphical = " << Graphical_name << endl;
   
   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);
//   cout << "curr_obs_ptr = " << curr_obs_ptr << endl;
//   if (curr_obs_ptr != NULL)
//      cout << "*curr_obs_ptr = " << *curr_obs_ptr << endl;

   if (curr_obs_ptr != NULL)
   {
      curr_obs_ptr->change_velocity(pass_number,v3);
   }
}

// ---------------------------------------------------------------------
// Boolean member function get_UVW_velocity() takes in time value
// curr_t and pass_number.  If no node corresponding to this time
// exists within *coords_map_ptr, this method returns false.
// Otherwise, it searches for a stored set of UVW image velocity
// coordinates corresponding to the input video pass and image
// numbers.  If no such coordinates exist, this method returns false.
// Otherwise, it returns the velocity within threevector v3.

bool Graphical::get_UVW_velocity(
   double curr_t,int pass_number,threevector& v3) const
{
//   cout << "inside Graphical::get_UVW_coords()" << endl;
   v3=threevector(NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);

   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);
//   cout << "curr_obs_ptr = " << curr_obs_ptr << endl;

   if (curr_obs_ptr==NULL)
   {
      return false;
   }
   else
   {
      return curr_obs_ptr->retrieve_velocity(pass_number,v3);
   }
}

// ---------------------------------------------------------------------
// Boolean member function get_velocity first checks whether the
// Graphical's position is defined at input time curr_t.  If so, it
// next checks whether positions are defined at curr_t +/- dt.  If so,
// it returns a difference proportional to position at (curr_t + dt) -
// (curr_t - dt) as its best estimate for the Graphical's velocity.
// If either the next or previous positions are not defined (e.g. at
// the beginning or ends of a Graphical's animation path), it uses a
// lower-order estimate for the velocity.  If not enough information
// exists to compute any velocity estimate at t=curr_t, this boolean
// method returns false.

bool Graphical::get_velocity(
   double curr_t,int pass_number,double dt,threevector& velocity) const
{
//   cout << "inside Graphical::get_velocity()" << endl;
//   cout << "curr_t = " << curr_t << " dt = " << dt << endl;
   
   threevector curr_p,next_p,prev_p;
   if (!get_UVW_coords(curr_t,pass_number,curr_p))
   {
      return false;
   }

//   threevector next_next_p;
//   bool next_next_flag=get_UVW_coords(curr_t+2*dt,pass_number,next_next_p);
   bool next_flag=get_UVW_coords(curr_t+dt,pass_number,next_p);
   bool prev_flag=get_UVW_coords(curr_t-dt,pass_number,prev_p);

//   cout << "next_flag = " << next_flag << endl;
//   cout << "next_p = " << next_p << endl;
//   cout << "curr_p = " << curr_p << endl;

   if (next_flag && prev_flag)
   {
      velocity=0.5*(next_p-prev_p)/dt;
      return true;
   }
   else if (next_flag && !prev_flag)
   {
      velocity=(next_p-curr_p)/dt;
      return true;
   }
   else if (prev_flag && !next_flag)
   {
      velocity=(curr_p-prev_p)/dt;
      return true;
   }
   else
   {
      return false;
   }
}

// ---------------------------------------------------------------------
bool Graphical::get_transformed_UVW_coords(
   double curr_t,int pass_number,threevector& p) const
{
   p=threevector(NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);

   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);

   if (curr_obs_ptr==NULL)
   {
      return false;
   }
   else
   {
      return curr_obs_ptr->retrieve_transformed_UVW_coords(pass_number,p);
   }
}

// ---------------------------------------------------------------------
void Graphical::adjust_UVW_coords(
   double curr_t,int pass_number,const threevector& dp)
{
   threevector p;
   if (get_UVW_coords(curr_t,pass_number,p))
   {
      set_UVW_coords(curr_t,pass_number,p+dp);
   } 
}

// ---------------------------------------------------------------------
// Member function set_vertices takes in time curr_t and first checks
// whether a node corresponding to this time exists within
// *coords_map_ptr.  If not, it generates a node.  This method
// next checks whether an STL of threevector vertices corresponding to
// the input video pass and image numbers already exist the current
// coordinates.  If it does not, it appends the input STL vector to
// the multiple_image_vertices STL vector.  Otherwise, it simply
// changes the STL vector within multiple_image_vertices to equal
// input vector.

void Graphical::set_vertices(
   double curr_t,int pass_number,const vector<threevector>& v)
{
//   cout << "inside Graphical::set_vertices()" << endl;
//   cout << "curr_t = " << curr_t << " pass_number = " << pass_number
//        << endl;
   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);
//   cout << "curr_obs_ptr = " << curr_obs_ptr << endl;

// If node corresponding to time curr_t does not exist in
// *coords_map_ptr, instantiate a new one and append it to the list:

   if (curr_obs_ptr==NULL)
   {
      instantaneous_obs curr_obs(curr_t);
      curr_obs.insert_vertices(pass_number,v);
      set_coords_obs(curr_t,pass_number,curr_obs);
   }
   else
   {
      curr_obs_ptr->change_vertices(pass_number,v);
      set_coords_obs(curr_t,pass_number,*curr_obs_ptr);
   }
//   cout << "at end of Graphical::set_vertices()" << endl;
}

// ---------------------------------------------------------------------
// Boolean member function get_vertices takes in time value curr_t.
// If no node corresponding to this time exists within
// *coords_map_ptr, this method returns false.  Otherwise, it
// searches for a stored STL vector<threevector> corresponding to the
// input video pass and image numbers.  If no such STL vector exists,
// this method returns false.  Otherwise, it returns the vector.

bool Graphical::get_vertices(double curr_t,int pass_number,
                             vector<threevector>& v) const
{
   v.clear();

   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);

   if (curr_obs_ptr==NULL)
   {
      return false;
   }
   else
   {
      if (!curr_obs_ptr->retrieve_vertices(pass_number,v))
      {
         return false;
      }
      else
      {
         return true;
      }
   }
}

// ---------------------------------------------------------------------
// Member function set_quaternion takes in time curr_t and first
// checks whether a node corresponding to this time exists within
// *coords_map_ptr.  If not, it generates a node.  This method next
// checks whether a quaternion corresponding to the input video pass
// and image numbers already exist the current coordinates.  If it
// does not, it appends the input quaternion to the
// multiple_image_attitude STL vector.  Otherwise, it simply changes
// the quaternion within multiple_image_attitude to equal input
// quaternion q.

void Graphical::set_quaternion(
   double curr_t,int pass_number,
   const threevector& u_hat,const threevector& v_hat)
{
   cout << "inside Graphical::set_quaternion()" << endl;

   rotation R;
//   genmatrix R(3,3);
   R.put_column(0,u_hat);
   R.put_column(1,v_hat);
   R.put_column(2,u_hat.cross(v_hat));
//   fourvector q=mathfunc::quaternion_corresponding_to_rotation(R);
   fourvector q=R.quaternion_corresponding_to_rotation();

   osg::Quat quaternion(q.get(0),q.get(1),q.get(2),q.get(3));
   set_quaternion(curr_t,pass_number,quaternion);
}

void Graphical::set_quaternion(
   double curr_t,int pass_number,const osg::Quat& q)
{
   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);

// If node corresponding to time curr_t does not exist in
// *coords_map_ptr, instantiate a new one and append it to the list:

   if (curr_obs_ptr==NULL)
   {
      instantaneous_obs curr_obs(curr_t);
      curr_obs.insert_quaternion(pass_number,q);
      set_coords_obs(curr_t,pass_number,curr_obs);
   }
   else
   {
      curr_obs_ptr->change_quaternion(pass_number,q);
      set_coords_obs(curr_t,pass_number,*curr_obs_ptr);
   }
}

// ---------------------------------------------------------------------
// Boolean member function get_quaternion takes in time value curr_t.
// If no node corresponding to this time exists within
// *coords_map_ptr, this method returns false.  Otherwise, it
// searches for a stored quaternion corresponding to the input video
// pass and image numbers.  If no such quaternion q exists, this
// method returns false.  Otherwise, it returns quaternion q.

bool Graphical::get_quaternion(double curr_t,int pass_number,osg::Quat& q)
{
//   cout << "inside Graphical::get_quaternion, t = " << curr_t
//        << " pass_number = " << pass_number << endl;
   q=osg::Quat(0,0,0,1);

   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);
//   cout << "curr_obs_ptr = " << curr_obs_ptr << endl;

   if (curr_obs_ptr==NULL)
   {
      return false;
   }
   else
   {
      if (!curr_obs_ptr->retrieve_quaternion(pass_number,q))
      {
//         cout << "before returning false" << endl;
//         osgfunc::print_quaternion(q);
         return false;
      }
      else
      {
//         cout << "before returning true" << endl;
//         osgfunc::print_quaternion(q);
         return true;
      }
   }
}

// ---------------------------------------------------------------------
void Graphical::set_scale(
   double curr_t,int pass_number,const threevector& s)
{
//   cout << "inside Graphical::set_scale, s = " << s << endl;
   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);

// If node corresponding to time curr_t does not exist in
// *coords_map_ptr, instantiate a new one and append it to the list:

   if (curr_obs_ptr==NULL)
   {
      instantaneous_obs curr_obs(curr_t);
      curr_obs.insert_scale(pass_number,s);
      set_coords_obs(curr_t,pass_number,curr_obs);
   }
   else
   {
      curr_obs_ptr->change_scale(pass_number,s);
      set_coords_obs(curr_t,pass_number,*curr_obs_ptr);
   }
}

// ---------------------------------------------------------------------
bool Graphical::get_scale(double curr_t,int pass_number,threevector& s) const
{
   s=threevector(1,1,1);

   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);

   if (curr_obs_ptr==NULL)
   {
      return false;
   }
   else
   {
      if (!curr_obs_ptr->retrieve_scale(pass_number,s))
      {
         return false;
      }
      else
      {
         return true;
      }
   }
}

// ---------------------------------------------------------------------
void Graphical::set_score(double curr_t,int pass_number,double score)
{
   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);

// If node corresponding to time curr_t does not exist in
// *coords_map_ptr, instantiate a new one and append it to the list:

   if (curr_obs_ptr==NULL)
   {
      instantaneous_obs curr_obs(curr_t);
      curr_obs.insert_score(pass_number,score);
      set_coords_obs(curr_t,pass_number,curr_obs);
   }
   else
   {
      curr_obs_ptr->change_score(pass_number,score);
      set_coords_obs(curr_t,pass_number,*curr_obs_ptr);
   }
}

// ---------------------------------------------------------------------
bool Graphical::get_score(double curr_t,int pass_number,double& score)
{
   score=NEGATIVEINFINITY;

   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);

   if (curr_obs_ptr==NULL)
   {
      return false;
   }
   else
   {
      if (!curr_obs_ptr->retrieve_score(pass_number,score))
      {
         return false;
      }
      else
      {
         return true;
      }
   }
}

// ---------------------------------------------------------------------
// Member function reset_all_scores() loops over all instantaneous
// observations and over all passes within each observation.  For each
// non-null observation, it sets the score equal to the input value.

void Graphical::reset_all_scores(double score)
{
//   cout << "inside Graphical::reset_all_scores, new_score = " << score
//        << endl;
   vector<instantaneous_obs*> all_observation_ptrs=get_all_observations();
   for (unsigned int i=0; i<all_observation_ptrs.size(); i++)
   {
      instantaneous_obs* curr_obs_ptr=all_observation_ptrs[i];
      if (curr_obs_ptr==NULL) continue;
      vector<int> pass_numbers=curr_obs_ptr->get_pass_numbers();
      for (unsigned int p=0; p<pass_numbers.size(); p++)
      {
//         cout << "i = " << i << " p = " << p 
//              << " pass_numbers[p] = " << pass_numbers[p] << endl;
         curr_obs_ptr->change_score(pass_numbers[p],score);
      } // loop over index p labeling pass numbers
   } // loop over index i labeling instantaneous observations
}

// ---------------------------------------------------------------------
void Graphical::set_index(double curr_t,int pass_number,int index)
{
   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);

// If node corresponding to time curr_t does not exist in
// *coords_map_ptr, instantiate a new one and append it to the list:

   if (curr_obs_ptr==NULL)
   {
      instantaneous_obs curr_obs(curr_t);
      curr_obs.insert_index(pass_number,index);
      set_coords_obs(curr_t,pass_number,curr_obs);
   }
   else
   {
      curr_obs_ptr->change_index(pass_number,index);
      set_coords_obs(curr_t,pass_number,*curr_obs_ptr);
   }
}

// ---------------------------------------------------------------------
bool Graphical::get_index(double curr_t,int pass_number,int& index)
{
   index=-1;

   instantaneous_obs* curr_obs_ptr=get_particular_time_obs(
      curr_t,pass_number);

   if (curr_obs_ptr==NULL)
   {
      return false;
   }
   else
   {
      if (!curr_obs_ptr->retrieve_index(pass_number,index))
      {
         return false;
      }
      else
      {
         return true;
      }
   }
}

// ==========================================================================
// Global data graph manipulation member functions:
// ==========================================================================

void Graphical::absolute_posn(
   double curr_t,int pass_number,const threevector& posn)
{
   threevector UVW;
   get_UVW_coords(curr_t,pass_number,UVW);

   UVW=posn;
   set_UVW_coords(curr_t,pass_number,UVW);
   set_PAT(curr_t,pass_number);
}

void Graphical::translate(
   double curr_t,int pass_number,const threevector& trans)
{
   threevector UVW;
   get_UVW_coords(curr_t,pass_number,UVW);

   UVW += trans;
   set_UVW_coords(curr_t,pass_number,UVW);
   set_PAT(curr_t,pass_number);
}

// ---------------------------------------------------------------------
void Graphical::scale(
   double curr_t,int pass_number,const threevector& scale_origin,
   const threevector& scale)
{
   set_scale(curr_t,pass_number,scale);

   threevector UVW;
   get_UVW_coords(curr_t,pass_number,UVW);
   threevector s(scale_origin);
   s.scale(scale);
   threevector eff_translation=scale_origin-s;
   UVW += eff_translation;
   set_UVW_coords(curr_t,pass_number,UVW);
   set_PAT(curr_t,pass_number);
}

// ---------------------------------------------------------------------
void Graphical::scale_and_translate(
   double curr_t,int pass_number,const threevector& scale_origin,
   const threevector& scale,const threevector& trans)
{
   set_scale(curr_t,pass_number,scale);

   threevector UVW;
   get_UVW_coords(curr_t,pass_number,UVW);
   threevector s(scale_origin);
   s.scale(scale);
   threevector eff_translation=scale_origin-s+trans;
   UVW += eff_translation;
   set_UVW_coords(curr_t,pass_number,UVW);
   set_PAT(curr_t,pass_number);
}

// ---------------------------------------------------------------------
// Member function rotate_about_specified_origin takes in a time and
// pass number along with threevectors rotation_origin, new_xhat and
// new_yhat.  It computes the rotation matrix which maps x_hat=(1,0,0)
// --> new_xhat and y_hat=(0,1,0) --> new_yhat.  It then effectively
// performs this rotation about the axis which pierces the input
// rotation_origin threevector.  The transformation information is
// stored for later callback retrieval.

// On 6/3/07, we learned the painful and hard way that rotation_origin
// should be specified wrt the Graphical's natural coordinate system
// and not wrt a global, world coordinate system.  For example, if we
// want to rotate an ObsFrustum about its camera vertex point,
// rotation_origin should equal Zero_vector and not the camera's
// absolute position in space...

void Graphical::rotate_about_specified_origin(
   double curr_t,int pass_number,const threevector& rotation_origin,
   const threevector& new_xhat,const threevector& new_yhat)
{
//   cout << "inside Graphical::rotate_about_specified_origin()" << endl;

   genmatrix R(3,3);
   R.put_column(0,new_xhat);
   R.put_column(1,new_yhat);
   R.put_column(2,new_xhat.cross(new_yhat));

   double chi;
   threevector nhat;
   mathfunc::decompose_orthogonal_matrix(R,chi,nhat);

   osg::Vec3 axis(nhat.get(0),nhat.get(1),nhat.get(2));
   osg::Quat q;
   q.makeRotate(chi,axis);
   set_quaternion(curr_t,pass_number,q);

   threevector UVW;
   get_UVW_coords(curr_t,pass_number,UVW);
   threevector eff_translation=rotation_origin-R*rotation_origin;
   UVW += eff_translation;
   set_UVW_coords(curr_t,pass_number,UVW);

//   cout << "UVW = " << UVW << endl;
//   cout << "Rotation_origin = " << rotation_origin << endl;
//   cout << "R*rotation_origin = " << R*rotation_origin << endl;
//   cout << "Eff_trans = " << eff_translation << endl;
   set_PAT(curr_t,pass_number);
}

// ---------------------------------------------------------------------
void Graphical::rotate_about_specified_origin_then_translate(
   double curr_t,int pass_number,const threevector& rotation_origin,
   const threevector& new_xhat,const threevector& new_yhat,
   const threevector& trans)
{
   genmatrix R(3,3);
   R.put_column(0,new_xhat);
   R.put_column(1,new_yhat);
   R.put_column(2,new_xhat.cross(new_yhat));
   rotate_about_specified_origin_then_translate(
      curr_t,pass_number,rotation_origin,R,trans);
}

// ---------------------------------------------------------------------
void Graphical::rotate_about_specified_origin_then_translate(
   double curr_t,int pass_number,const threevector& rotation_origin,
   const genmatrix& R,const threevector& trans)
{
//   cout << "inside Graphical::rotate_about_specified_origin_then_translate()"
//        << endl;
   double chi;
   threevector nhat;
   mathfunc::decompose_orthogonal_matrix(R,chi,nhat);

   osg::Vec3 axis(nhat.get(0),nhat.get(1),nhat.get(2));
   osg::Quat q;
   q.makeRotate(chi,axis);

   set_quaternion(curr_t,pass_number,q);

   threevector UVW;
   get_UVW_coords(curr_t,pass_number,UVW);
   threevector eff_translation=rotation_origin-R*rotation_origin+trans;
   UVW += eff_translation;
   set_UVW_coords(curr_t,pass_number,UVW);
   set_PAT(curr_t,pass_number);
}

// ---------------------------------------------------------------------
void Graphical::scale_and_rotate(
   double curr_t,int pass_number,const threevector& origin,
   const genmatrix& R,const threevector& scale)
{
//   cout << "inside Graphical::scale_and_rotate()" << endl;

   scale_rotate_and_then_translate(
      curr_t,pass_number,origin,R,scale,Zero_vector);
}

// ---------------------------------------------------------------------
// Member function scale_rotate_and_then_translate first effectively
// translates the DataGraph's specified origin to (0,0,0).  It next
// scales the DataGraph's X, Y and Z axes by input threevector scale
// and subsequently rotates the scaled DataGraph by input genmatrix R.
// Finally, this method resets the origin to input location trans.

void Graphical::scale_rotate_and_then_translate(
   double curr_t,int pass_number,const threevector& origin,
   const genmatrix& R,const threevector& scale,const threevector& trans)
{
//   cout << "inside Graphical::scale_rotate_and_then_trans()" << endl;
   set_scale(curr_t,pass_number,scale);
//   cout << "Scale = " << scale << endl;

   double chi;
   threevector nhat;
   mathfunc::decompose_orthogonal_matrix(R,chi,nhat);

   osg::Vec3 axis(nhat.get(0),nhat.get(1),nhat.get(2));
   osg::Quat q;
   q.makeRotate(chi,axis);
   set_quaternion(curr_t,pass_number,q);

//   cout << "curr_t = " << curr_t << " pass_number = " << pass_number
//        << endl;
//   cout << "q = " << q._v[0] << " , "
//        << q._v[1] << " , "
//        << q._v[2] << " , "
//        << q._v[3] << endl;

   threevector UVW;
   get_UVW_coords(curr_t,pass_number,UVW);

   threevector column;
   genmatrix RS(3,3);
   R.get_column(0,column);
   RS.put_column(0,column*scale.get(0));
   R.get_column(1,column);
   RS.put_column(1,column*scale.get(1));
   R.get_column(2,column);
   RS.put_column(2,column*scale.get(2));

   threevector eff_translation=origin-RS*origin+trans;
   UVW += eff_translation;
   set_UVW_coords(curr_t,pass_number,UVW);
   set_PAT(curr_t,pass_number);
}

// ---------------------------------------------------------------------
// Member function rotate_about_zaxis takes in a time and pass number
// along with a rotation_origin and azimuthal angle phi.  It computes
// the quaternion corresponding to a z-rotation by angle phi about the
// rotation origin.  We wrote this method primarily for earth
// ellipsoid diurnal spinning purposes.

void Graphical::rotate_about_zaxis(
   double curr_t,int pass_number,const threevector& rotation_origin,
   double phi_z)
{
   genmatrix R(3,3);
   R.put(0,0,cos(phi_z));
   R.put(0,1,-sin(phi_z));
   R.put(1,0,sin(phi_z));
   R.put(1,1,cos(phi_z));
   R.put(2,2,1);
   
   osg::Vec3 axis(0,0,1);
   osg::Quat q0,q;
//   get_quaternion(curr_t,pass_number,q0);

   q.makeRotate(phi_z,axis);

//   cout << "phi_z = " << phi_z << endl;
//   cout << "q.0 = " << q._v[0]
//        << " q.1 = " << q._v[1]
//        << " q.2 = " << q._v[2]
//        << " q.3 = " << q._v[3] << endl;

   set_quaternion(curr_t,pass_number,q);
//   set_quaternion(curr_t,pass_number,q*q0);

   threevector eff_translation=rotation_origin-R*rotation_origin;
   set_UVW_coords(curr_t,pass_number,eff_translation);
   set_PAT(curr_t,pass_number);
}

// ==========================================================================
// Roll-pitch-yaw manipulation member functions
// ==========================================================================

// Member function set_scale_attitude_posn takes in a time and pass
// number along with a corresponding scale, roll, pitch, yaw and XYZ
// position for the model.  This information is stored for later
// callback retrieval.

void Graphical::set_attitude_posn(
   double curr_t,int pass_number,
   const threevector& RPY,const threevector& posn)
{
//   cout << "inside Graphical::set_attitude_posn() #1" << endl;

   set_attitude_posn(curr_t,pass_number,RPY,posn,0);
}

void Graphical::set_attitude_posn(
   double curr_t,int pass_number,
   const threevector& RPY,const threevector& posn,double init_yaw_angle)
{
//   cout << "inside Graphical::set_attitude_posn() #2" << endl;

   osg::Quat quat_canonical,quat_roll,quat_pitch,quat_yaw;
   quat_canonical.makeRotate ( init_yaw_angle ,osg::Vec3f(0,0,1));
   quat_roll.makeRotate(RPY.get(0),osg::Vec3f(0,1,0));
   quat_pitch.makeRotate(RPY.get(1),osg::Vec3f(1,0,0));
   quat_yaw.makeRotate(RPY.get(2),osg::Vec3f(0,0,1));
   osg::Quat total_quat=quat_yaw*quat_pitch*quat_roll*quat_canonical;
   set_quaternion(curr_t,pass_number,total_quat);
   
//   cout << "total quat = " << endl;
//   osgfunc::print_quaternion(total_quat);

   set_UVW_coords(curr_t,pass_number,posn);
}

void Graphical::set_scale_attitude_posn(
   double curr_t,int pass_number,
   double scale_factor,const threevector& RPY,const threevector& posn)
{
   set_scale(curr_t,pass_number,
             threevector(scale_factor,scale_factor,scale_factor));
   set_attitude_posn(curr_t,pass_number,RPY,posn);
}

void Graphical::set_scale_attitude_posn(
   double curr_t,int pass_number,
   const threevector& scale,const threevector& RPY,const threevector& posn)
{
   set_scale(curr_t,pass_number,scale);
   set_attitude_posn(curr_t,pass_number,RPY,posn);
}

// ----------------------------------------------------------------
// This overloaded version of member function set_scale_attitude_posn
// reads in and stores model's position, roll, pitch, yaw (measured in
// radians) and scale for all image times.  We wrote this little
// utility method specifically for the Cessna model.

void Graphical::set_attitude_posn(
   int pass_number,const vector<threevector>& RPY,
   const vector<threevector>& posn,int init_frame)
{
//   cout << "inside Graphical::set_attitude_posn(int,vec<threevec>,vec<threevectro>,int)" << endl;
//   cout << "posn.size() = " << posn.size() << endl;
//   cout << "RPY.size() = " << RPY.size() << endl;
   for (unsigned int i=0; i<posn.size(); i++)
   {
//      cout << i << " " << flush;
      double curr_t=init_frame+i;
      set_attitude_posn(curr_t,pass_number,RPY[i],posn[i]);
   }
//   cout << endl;
}

void Graphical::set_scale_attitude_posn(
   int pass_number,double scale_factor,const vector<threevector>& RPY,
   const vector<threevector>& posn)
{
   for (unsigned int i=0; i<posn.size(); i++)
   {
      double curr_t=static_cast<double>(i);
      set_scale_attitude_posn(
         curr_t,pass_number,scale_factor,RPY[i],posn[i]);
   }
}

void Graphical::set_scale_attitude_posn(
   int pass_number,const threevector& scale,const vector<threevector>& RPY,
   const vector<threevector>& posn)
{
   for (unsigned int i=0; i<posn.size(); i++)
   {
      double curr_t=static_cast<double>(i);
      set_scale_attitude_posn(
         curr_t,pass_number,scale,RPY[i],posn[i]);
   }
}

// ==========================================================================
// Coordinate manipulation methods
// ==========================================================================

// Member functions set_mask and get_mask provide access to a member
// STL map within which we store boolean information about whether the
// user has erased the UVW coordinates for the current feature object
// at time t.  If mask_flag==true, Graphical is "erased" at time t.

void Graphical::set_mask(double t,int pass_number,bool mask_flag)
{
//   if (get_stationary_Graphical_flag()) t=get_initial_t();

   twovector key(t,pass_number);
   COORDS_MAP::iterator coords_iter=coords_map_ptr->find(key);

   if (coords_iter != coords_map_ptr->end())
   {
      coords_iter->second.second=mask_flag;
   }
   else
   {
      Triple<instantaneous_obs,bool,bool> t;
      t.second=mask_flag;
      (*coords_map_ptr)[key]=t;
   }
}

bool Graphical::get_mask(double t,int pass_number) const 
{
//   if (get_stationary_Graphical_flag()) t=get_initial_t();

   COORDS_MAP::iterator coords_iter=get_coords_map_iterator(t,pass_number);
   if (coords_iter != coords_map_ptr->end())
   {
      return coords_iter->second.second;
   }
   else
   {
      return false;
   }
}

// ---------------------------------------------------------------------
// Member function count_image_appearances returns the number of
// images between start_imagenumber and stop_imagenumber in which a
// Graphical appears.

int Graphical::count_image_appearances(
   int pass_number,unsigned int start_imagenumber,
   unsigned int stop_imagenumber)
{
   int n_appearances=0;
   for (unsigned int i=start_imagenumber; i<=stop_imagenumber; i++)
   {
      double curr_t=static_cast<double>(i);
      if (!get_mask(curr_t,pass_number)) n_appearances++;
   } // loop over index i labeling image numbers
   return n_appearances;
}

// ---------------------------------------------------------------------
// Member functions set_coords_manually_manipulated and
// get_coords_manually_manipulated provide access to a member STL map
// within which we store boolean information about whether the user
// has manually altered the UVW coordinates for the current Graphical
// object.

void Graphical::set_coords_manually_manipulated(double t,int pass_number)
{
   twovector key(t,pass_number);
   COORDS_MAP::iterator coords_iter=coords_map_ptr->find(key);

   if (coords_iter != coords_map_ptr->end())
   {
      coords_iter->second.third=true;
   }
   else
   {
      Triple<instantaneous_obs,bool,bool> t;
      t.third=true;
      (*coords_map_ptr)[key]=t;
   }
}

bool Graphical::get_coords_manually_manipulated(double t,int pass_number) 
{
   COORDS_MAP::iterator coords_iter=get_coords_map_iterator(t,pass_number);
   if (coords_iter != coords_map_ptr->end())
   {
      return coords_iter->second.third;
   }
   else
   {
      return false;
   }
}



