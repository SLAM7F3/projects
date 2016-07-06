// ==========================================================================
// GRAPHICALSGROUP class member function definitions
// ==========================================================================
// Last modified on 12/8/11; 1/27/12; 5/24/13; 4/5/14; 6/16/14
// ==========================================================================

#include <algorithm>
#include <iomanip>
#include <vector>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include "osg/osgGraphicals/AnimationController.h"      
#include "math/constant_vectors.h"
#include "astro_geo/Ellipsoid_model.h"
#include "general/filefuncs.h"
#include "osg/osgGraphicals/Graphical.h"
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "general/inputfuncs.h"
#include "messenger/Messenger.h"
#include "general/stringfuncs.h"
#include "math/threevector.h"

#include "general/outputfuncs.h"
#include "templates/mytemplates.h"

using std::cin;
using std::cout;
using std::deque;
using std::endl;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::ostream;
using std::setw;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void GraphicalsGroup::allocate_member_objects()
{
//   cout << "inside GraphicalsGroup::allocate_member_objects()" << endl;

   graphical_ID_ptrs_map_ptr=new GRAPHICAL_PTRS_MAP;
   Graphical_index_ID_map_ptr=new GRAPHICAL_INDEX_ID_MAP;
   Graphical_ID_index_map_ptr=new GRAPHICAL_ID_INDEX_MAP;

   OSGgroup_refptr=new osg::Group;
   OSGgroup_refptr->setName("OSGgroup");
   generate_new_OSGsubPAT();
}		       

void GraphicalsGroup::generate_new_OSGsubPAT()
{
//   cout << "inside GraphicalsGroup::generate_new_OSGsubPAT()" << endl;
   
   osg::ref_ptr<osg::PositionAttitudeTransform> OSGsubPAT_ptr=
      new osg::PositionAttitudeTransform;
   string subPAT_name="OSGsubPAT "+stringfunc::number_to_string(
      OSGsubPATs.size());
   OSGsubPAT_ptr->setName(subPAT_name);

// Default OSGsubPAT's mask to OFF:

   OSGsubPAT_ptr->setNodeMask(0);  // mask enabled

   OSGsubPATs.push_back(OSGsubPAT_ptr);
   OSGgroup_refptr->addChild(OSGsubPAT_ptr.get());
}		       

void GraphicalsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="GraphicalsGroup";

   erase_Graphicals_forward_in_time_flag = false;
   erase_Graphicals_except_at_curr_time_flag = false;
   update_display_flag=true;
   graphical_counter=0;
   selected_OSGsubPAT_ID=-1;
   most_recently_added_ID=most_recently_selected_ID=-1;
   pass_ptr=NULL;
   PassesGroup_ptr=NULL;
   grid_world_origin_ptr=NULL;
   ndims_label="_"+stringfunc::number_to_string(ndims)+"D";
   Clock_ptr=NULL;
   AnimationController_ptr=NULL;
   Ellipsoid_model_ptr=NULL;
   GraphicalsGroup_bug_ptr=NULL;

// delta_move_z=0.0015;	// meter (appropriate for SPASE data)
   delta_move_z=0.1;	// meter (appropriate for ALIRT data)
}		       

GraphicalsGroup::GraphicalsGroup(
   int p_ndims,PassesGroup* PG_ptr,threevector* GO_ptr):
   ndims(p_ndims)
{	
   allocate_member_objects();
   initialize_member_objects();
   PassesGroup_ptr=PG_ptr;
   grid_world_origin_ptr=GO_ptr;
}		       

GraphicalsGroup::GraphicalsGroup(
   int p_ndims,Pass* PI_ptr,threevector* GO_ptr):
   ndims(p_ndims)
{	
   allocate_member_objects();
   initialize_member_objects();
   pass_ptr=PI_ptr;
   grid_world_origin_ptr=GO_ptr;
}		       

GraphicalsGroup::GraphicalsGroup(
   int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr,threevector* GO_ptr):
   ndims(p_ndims)
{	
   allocate_member_objects();
   initialize_member_objects();
   pass_ptr=PI_ptr;
   grid_world_origin_ptr=GO_ptr;

   AnimationController_ptr=AC_ptr;
   if (AnimationController_ptr != NULL)
   {
      AnimationController_ptr->register_GraphicalsGroup(this);
   }
}		       

GraphicalsGroup::GraphicalsGroup(
   int p_ndims,Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
   threevector* GO_ptr):
   ndims(p_ndims)
{	
   allocate_member_objects();
   initialize_member_objects();
   pass_ptr=PI_ptr;
   Clock_ptr=clock_ptr;
   Ellipsoid_model_ptr=EM_ptr;
   grid_world_origin_ptr=GO_ptr;
}		       

GraphicalsGroup::~GraphicalsGroup()
{
//   cout << "inside GraphicalsGroup destructor" << endl;
//   cout << "GraphicalsGroup_name = " << GraphicalsGroup_name << endl;
//   cout << "this = " << this << endl;

   destroy_all_Graphicals();
   delete graphical_ID_ptrs_map_ptr;
   delete Graphical_index_ID_map_ptr;
   delete Graphical_ID_index_map_ptr;

   for (unsigned int i=0; i<OSGsubPATs.size(); i++)
   {
      OSGgroup_refptr->removeChild(OSGsubPATs[i].get());
   }

   if (AnimationController_ptr != NULL)
   {
      AnimationController_ptr->unregister_GraphicalsGroup(this);
   }
}

// ==========================================================================
// Set & get methods
// ==========================================================================

// Member functions set_MatrixTransform_ptr and
// get_MatrixTransform_ptr allow a MatrixTransform to be placed at the
// very top of the scenegraph associated with the current
// GraphicalsGroup.  It can be used to rotate, for example, from
// canonical to lat/long coordinates.

// Note added on 8/25/07: If we ever need to remove OSGgroup_refptr
// from the scenegraph and if it has been attached to
// MatrixTransform_refptr via this method, then we must be sure to
// remove it from the MatrixTransform via a removeChild call...

void GraphicalsGroup::set_MatrixTransform_ptr(osg::MatrixTransform* MT_ptr)
{
//   cout << "inside GraphicalsGroup::set_MatrixTransform_ptr()" << endl;
   MatrixTransform_refptr=MT_ptr;
   MatrixTransform_refptr->addChild(OSGgroup_refptr.get());
}

osg::MatrixTransform* GraphicalsGroup::get_MatrixTransform_ptr()
{
   return MatrixTransform_refptr.get();
}

// ---------------------------------------------------------------------
threevector& GraphicalsGroup::get_grid_world_origin() const
{
//   cout << "inside GraphicalsGroup::get_grid_world_origin()" << endl;
   if (grid_world_origin_ptr != NULL)
   {
      return *grid_world_origin_ptr;
   }
   else
   {
      cout << "Error in GraphicalsGroup::get_grid_world_origin()" << endl;
      cout << "grid_world_origin_ptr = NULL" << endl;
      cout << "Exiting now..." << endl;
      exit(-1);
   }
}

threevector* GraphicalsGroup::get_grid_world_origin_ptr() 
{
   return grid_world_origin_ptr;
}

const threevector* GraphicalsGroup::get_grid_world_origin_ptr() const
{
   return grid_world_origin_ptr;
}

// --------------------------------------------------------------------------
Graphical* GraphicalsGroup::get_ID_labeled_Graphical_ptr(int ID) const
{
//   cout << "inside Graphicalsgroup::get_ID_labeled_Graphical_ptr(ID)"
//        << endl;
//   cout << "ID = " << ID << endl;

   GRAPHICAL_PTRS_MAP::iterator graphical_ptrs_iter=
      graphical_ID_ptrs_map_ptr->find(ID);

   if (graphical_ptrs_iter != graphical_ID_ptrs_map_ptr->end())
   {
      return graphical_ptrs_iter->second;
   } 
   else
   {
      return NULL;
   }
}

// --------------------------------------------------------------------------
vector<Graphical*> GraphicalsGroup::get_all_Graphical_ptrs() const
{
//   cout << "inside Graphicalsgroup::get_all_Graphical_ptrs()" << endl;
//   cout << "GraphicalsGroup_name = " << GraphicalsGroup_name << endl;
   vector<Graphical*> Graphical_ptrs;

//   cout << "graphical_ID_ptrs_map_ptr->size() = "
//        << graphical_ID_ptrs_map_ptr->size() << endl;
   for (GRAPHICAL_PTRS_MAP::iterator iter=graphical_ID_ptrs_map_ptr->begin();
        iter != graphical_ID_ptrs_map_ptr->end(); iter++)
   {
      Graphical_ptrs.push_back(iter->second);
   }
   return Graphical_ptrs;
}

// --------------------------------------------------------------------------
// Member function get_next_unused_ID loops over all nodes within the
// Graphicals map.  It returns the smallest integer which does not
// correspond to any Graphical ID within the map.

int GraphicalsGroup::get_next_unused_ID() const
{
//   cout << "inside GraphicalsGroup::get_next_unused_ID()" << endl;

   bool ID_already_exists;
   int next_ID=0;

   do
   {
      ID_already_exists=false;

      for (GRAPHICAL_PTRS_MAP::iterator iter=graphical_ID_ptrs_map_ptr->
              begin(); iter != graphical_ID_ptrs_map_ptr->end(); ++iter)
      {
         Graphical* Graphical_ptr=iter->second;
         if (Graphical_ptr->get_ID()==next_ID)
         {
            next_ID++;
            ID_already_exists=true;
         }
      }
   }
   while (ID_already_exists);

//   cout << "next_ID = " << next_ID << endl;
   return next_ID;
}

// --------------------------------------------------------------------------
// Member function get_largest_used_ID loops over all nodes within the
// *graphical_ID_ptrs_map_ptr.  It returns the largest ID for all the
// Graphicals within the map.

int GraphicalsGroup::get_largest_used_ID() const
{
   int largest_ID=-1;

   for (GRAPHICAL_PTRS_MAP::iterator iter=graphical_ID_ptrs_map_ptr->begin();
        iter != graphical_ID_ptrs_map_ptr->end(); iter++)
   {
      Graphical* Graphical_ptr=iter->second;
      largest_ID=basic_math::max(largest_ID,Graphical_ptr->get_ID());
   }
   return largest_ID;
}

// --------------------------------------------------------------------------
void GraphicalsGroup::set_selected_Graphical_ID(int n)
{
//   cout << "inside GraphicalsGroup::set_selected_Graphical_ID(), n = "
//        << n << endl;

   if (n == get_selected_Graphical_ID() && selected_OSGsubPAT_ID > -1) 
      return;

// Push back input n onto STL deque member selected_Graphical_IDs.  If
// deque's size exceeds 10, pop off earliest selected Graphical ID
// from deque' front:

   selected_Graphical_IDs.push_back(n);
   if (selected_Graphical_IDs.size() > 10)
   {
      selected_Graphical_IDs.pop_front();
   }
   
   int selected_Graphical_ID=n;
   if (selected_Graphical_ID==-1)
   {
      selected_OSGsubPAT_ID=-1;
   }
   else
   {
      selected_OSGsubPAT_ID=OSGsubPAT_parent_of_Graphical(
         get_ID_labeled_Graphical_ptr(selected_Graphical_ID));
   }
//   cout << "selected_OSGsubPAT_ID = " << selected_OSGsubPAT_ID << endl;
//   cout << "selected_Graphical_ID = " << selected_Graphical_ID << endl;
//   outputfunc::enter_continue_char();
}

// --------------------------------------------------------------------------
int GraphicalsGroup::get_selected_Graphical_ID() const
{
   return get_selected_Graphical_ID(0);
}

int GraphicalsGroup::get_prev_selected_Graphical_ID() const
{
   return get_selected_Graphical_ID(1);
}

// This overloaded version of get_selected_Graphical_ID() takes in
// input integer n.  If n==0, it returns the most recently selected
// Graphical ID.  If n==1, it returns the previously selected
// Graphical ID.  If n==2, it returns the second previously selected
// Graphical ID, etc.  If n exceeds the size of deque member
// selected_Graphical_IDs.size(), this method returns -1.

int GraphicalsGroup::get_selected_Graphical_ID(unsigned int n) const
{
   int selected_Graphical_ID=-1;
   if (selected_Graphical_IDs.size() >= 1+n)
   {
      selected_Graphical_ID=selected_Graphical_IDs[
         selected_Graphical_IDs.size()-1-n];
   }
   return selected_Graphical_ID;
}

// ==========================================================================
// Pass info get member functions
// ==========================================================================

// Member function get_max_n_passes() performs a brute force search
// over all Graphicals' instantaneous observations for input time t.
// It returns the maximum number of different observations = maximum
// number of passes for time t.

int GraphicalsGroup::get_max_n_passes(double t) const
{
//   cout << "inside GraphicalsGroup::get_max_n_passes()" << endl;
   
   int max_n_passes=0;
   for (unsigned int f=0; f<get_n_Graphicals(); f++)
   {
      Graphical* Graphical_ptr=get_Graphical_ptr(f);
      instantaneous_obs* obs_ptr=
         Graphical_ptr->get_all_particular_time_observations(t);
      max_n_passes=basic_math::max(max_n_passes,obs_ptr->get_npasses());
   } // loop over index f labeling Graphicals
   return max_n_passes;
}

// -------------------------------------------------------------------------
// Member function get_all_pass_numbers() loops over every Graphical
// and extracts each instanteous observation.  It returns a sorted STL
// vector containing the pass numbers for every pass represented
// within the current GraphicalsGroup.

vector<int> GraphicalsGroup::get_all_pass_numbers(double t) const
{
//   cout << "inside GraphicalsGroup::get_pass_numbers()" << endl;
   
   map<int,int> passnumbers_map;

   for (unsigned int f=0; f<get_n_Graphicals(); f++)
   {
      Graphical* Graphical_ptr=get_Graphical_ptr(f);
      instantaneous_obs* obs_ptr=
         Graphical_ptr->get_all_particular_time_observations(t);
      vector<int> curr_pass_numbers=obs_ptr->get_pass_numbers();
      for (unsigned int p=0; p<curr_pass_numbers.size(); p++)
      {
         if (passnumbers_map.find(curr_pass_numbers[p])==
             passnumbers_map.end())
         {
            passnumbers_map[curr_pass_numbers[p]]=1;
         }
      } // loop over index p labeling pass numbers for *obs_ptr
   } // loop over index f labeling Graphicals

   vector<int> all_pass_numbers;
   for (map<int,int>::iterator iter=passnumbers_map.begin();
        iter != passnumbers_map.end(); iter++)
   {
      all_pass_numbers.push_back(iter->first);
   }

   std::sort(all_pass_numbers.begin(),all_pass_numbers.end());

   return all_pass_numbers;
}

// ==========================================================================
// Instantaneous observation manipulation member functions
// ==========================================================================

// Member function find_nonmatching_IDs performs a brute force search
// over all Graphicals within *other_GraphicalsGroup_ptr and *this.
// It returns an STL vector containing the IDs of those Graphicals
// within the former which do not exist within the latter.

vector<int> GraphicalsGroup::find_nonmatching_IDs(
   double t,GraphicalsGroup* other_GraphicalsGroup_ptr)
{
//   cout << "inside GraphicalsGroup::find_nonmatching_IDs()" << endl;

   vector<int> nonmatching_IDs;
   for (unsigned int j=0; j<other_GraphicalsGroup_ptr->get_n_Graphicals(); j++)
   {
      Graphical* other_Graphical_ptr=other_GraphicalsGroup_ptr->
         get_Graphical_ptr(j);
      int other_Graphical_ID=other_Graphical_ptr->get_ID();

      bool ID_match_found=false;
      for (unsigned int i=0; i<get_n_Graphicals(); i++)
      {
         Graphical* Graphical_ptr=get_Graphical_ptr(i);
         if (other_Graphical_ID==Graphical_ptr->get_ID())
         {
            ID_match_found=true;
         }
      }

      if (!ID_match_found)
      {
         nonmatching_IDs.push_back(other_Graphical_ID);
      }
   } // loop oer index j labeling Graphicals in *other_GraphicalsGroup_ptr

   return nonmatching_IDs;
}

// ---------------------------------------------------------------------
// Member function consolidate_instantaneous_matching_observations
// consolidates instantaneous observations for all Graphicals with
// matching IDs.

void GraphicalsGroup::consolidate_instantaneous_matching_observations(
   double t,GraphicalsGroup* other_GraphicalsGroup_ptr)
{
//   cout << "inside GraphicalsGroup::consolidate_instantaneous_matching_observations()"
//        << endl;
   
   for (unsigned int i=0; i<get_n_Graphicals(); i++)
   {
      Graphical* Graphical_ptr=get_Graphical_ptr(i);
      for (unsigned int j=0; j<other_GraphicalsGroup_ptr->get_n_Graphicals(); j++)
      {
         Graphical* other_Graphical_ptr=other_GraphicalsGroup_ptr->
            get_Graphical_ptr(j);

         if (Graphical_ptr->get_ID()==other_Graphical_ptr->get_ID())
         {
            Graphical_ptr->consolidate_instantaneous_observations(
               t,other_Graphical_ptr);
         }
      } // loop over index j labeling Graphicals in *other_GraphicalsGroup_ptr
   } // loop over index i labeling Graphicals in *this
}

// ==========================================================================
// Subgroup member functions
// ==========================================================================

// Member function is_child_of_OSGgroup takes in pointer
// some_osgGroup_ptr to some parent osg::Group.  It checks whether
// *osgGroup_ptr has any of the current GraphicalsGroup's OSGsubPATs
// or OSGgroup as its children.  If so, this boolean method returns
// true.

bool GraphicalsGroup::is_child_of_OSGgroup(osg::Group* some_osgGroup_ptr)
{
   for (unsigned int i=0; i<OSGsubPATs.size(); i++)
   {
      if (some_osgGroup_ptr->containsNode(OSGsubPATs[i].get()))
         return true;
   }
   if (some_osgGroup_ptr->containsNode(OSGgroup_refptr.get()))
      return true;

   return false;
}

// ---------------------------------------------------------------------
// Boolean member function is_parent_of_Graphical returns true if the
// current GraphicalsGroup's OSGgroup contains the input Graphical's
// PAT.

bool GraphicalsGroup::is_parent_of_Graphical(Graphical* Graphical_ptr)
{
   return OSGgroup_refptr->containsNode(Graphical_ptr->get_PAT_ptr());
}

// ==========================================================================
// OSGsubPAT member functions
// ==========================================================================

// Member function OSGsubPAT_parent_of_Graphical loops over every
// OSGsubPAT member of the current GraphicalsGroup.  It returns the
// number of the OSGsubPAT containing the input Graphical's PAT.  If
// no such OSGsubPAT parent is found, this method returns -1.

int GraphicalsGroup::OSGsubPAT_parent_of_Graphical(Graphical* Graphical_ptr)
{
//   cout << "inside GraphicalsGroup::OSGsubPAT_parent_of_Graphical()" << endl;
//   cout << "Graphical_ptr = " << Graphical_ptr << endl;
   if (Graphical_ptr==NULL) return -1;

//   cout << "n_OSGsubPATs = " << get_n_OSGsubPATs() << endl;
   for (unsigned int i=0; i<get_n_OSGsubPATs(); i++)
   {
      if (get_OSGsubPAT_ptr(i)->containsNode(Graphical_ptr->get_PAT_ptr()))
      {
//         cout << "OSGsubPAT_parent = " << i << endl;
         return i;
      }
   }
   return -1;
}

// ---------------------------------------------------------------------
// Member function n_Graphical_siblings returns the number of siblings
// belonging to the same parent OSGsubPAT as input Graphical
// *Graphical_ptr:

int GraphicalsGroup::n_Graphical_siblings(Graphical* Graphical_ptr)
{
//   cout << "inside GraphicalsGroup::n_graphical_siblings" << endl;
   int parent_ID=OSGsubPAT_parent_of_Graphical(Graphical_ptr);
//   cout << "parent_ID = " << parent_ID << endl;

   int n_siblings=-1;
   for (unsigned int i=0; i<get_n_Graphicals(); i++)
   {
      Graphical* Graphical_ptr=get_Graphical_ptr(i);
      if (OSGsubPAT_parent_of_Graphical(Graphical_ptr)==parent_ID)
      {
         n_siblings++;
      }
   }
   
//   cout << "n_siblings = " << n_siblings << endl;
   return n_siblings;
}

// ---------------------------------------------------------------------
// Member function mask_nonselected_OSGsubPATs unmasks every OSGsubPAT
// if selected_OSGsubPAT_ID==-1.  Otherwise, it masks every OSGsubPAT
// except for the one whose ID = selected_OSGsubPAT_ID.

void GraphicalsGroup::mask_nonselected_OSGsubPATs()
{
//   cout << "inside GraphicalsGroup::mask_nonselected_OSGsubPATs()" << endl;

   if (get_selected_OSGsubPAT_ID()==-1)
   {
      unmask_all_OSGsubPATs();
   }
   else
   {
      for (unsigned int i=0; i<get_n_OSGsubPATs(); i++)
      {
         set_OSGsubPAT_nodemask(i,0);
      } 
      set_OSGsubPAT_nodemask(get_selected_OSGsubPAT_ID(),1);
   }
   update_display();
}

void GraphicalsGroup::unmask_all_OSGsubPATs()
{
//   cout << "inside GraphicalsGroup::unmask_all_OSGsubPATs()" << endl;

   for (unsigned int i=0; i<get_n_OSGsubPATs(); i++)
   {
      set_OSGsubPAT_nodemask(i,1);
   } 
}

// --------------------------------------------------------------------------
void GraphicalsGroup::mask_Graphical_for_all_times(
   Graphical* curr_Graphical_ptr)
{   
//   cout << "inside GraphicalsGroup::mask_Graphical_for_all_times()" << endl;
//   cout << "first framenumber = " << get_first_framenumber()
//        << " last frame = " << get_last_framenumber() << endl;

   for (unsigned int n=get_first_framenumber(); n<=get_last_framenumber(); n++)
   {
      double curr_t=static_cast<double>(n);
      curr_Graphical_ptr->set_mask(curr_t,get_passnumber(),true);
   }
}

// --------------------------------------------------------------------------
// Member function translate_OSGsubPAT() resets the position
// for the OSGsubPAT corresponding to the input ID. 

void GraphicalsGroup::translate_OSGsubPAT(
   int OSGsubPAT_ID,const threevector& translation)
{
   osg::PositionAttitudeTransform* PAT_ptr=get_OSGsubPAT_ptr(OSGsubPAT_ID);
  
   osg::Vec3 posn=PAT_ptr->getPosition();
   threevector UVW(posn.x(),posn.y(),posn.z());
   UVW += translation;
   
   PAT_ptr->setPosition(osg::Vec3d(UVW.get(0),UVW.get(1),UVW.get(2)));
}

// --------------------------------------------------------------------------
// Member function rotate_OSGsubPAT_about_specified_origin resets the
// attitude for the OSGsubPAT corresponding to the input ID.  We wrote
// this method in order to rotate coarse OSG satellite models for
// refinement purposes.

void GraphicalsGroup::rotate_OSGsubPAT_about_specified_origin(
      int OSGsubPAT_ID,const threevector& rotation_origin,
      const threevector& new_xhat,const threevector& new_yhat)
{   
   rotate_OSGsubPAT_about_specified_origin_then_translate(
      OSGsubPAT_ID,rotation_origin,new_xhat,new_yhat,Zero_vector);
}

void GraphicalsGroup::rotate_OSGsubPAT_about_specified_origin_then_translate(
      int OSGsubPAT_ID,const threevector& rotation_origin,
      const threevector& new_xhat,const threevector& new_yhat,
      const threevector& trans)
{   
   osg::PositionAttitudeTransform* PAT_ptr=get_OSGsubPAT_ptr(OSGsubPAT_ID);
   
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
   PAT_ptr->setAttitude(q);
   
//   cout << "Quaternion q = " << endl;
//   osgfunc::print_quaternion(q);
  
   osg::Vec3 posn=PAT_ptr->getPosition();
   threevector UVW(posn.x(),posn.y(),posn.z());
   threevector eff_translation=rotation_origin-R*rotation_origin+trans;
   UVW += eff_translation;
   
//   cout << "effective translation = " << eff_translation << endl;
   
   PAT_ptr->setPosition(osg::Vec3d(UVW.get(0),UVW.get(1),UVW.get(2)));
}

// ==========================================================================
// ActiveMQ message handling member functions
// ==========================================================================

void GraphicalsGroup::pushback_Messenger_ptr(Messenger* M_ptr)
{
   Messenger_ptrs.push_back(M_ptr);
}

Messenger* GraphicalsGroup::get_Messenger_ptr()
{
   return get_Messenger_ptr(Messenger_ptrs.size()-1);
}

const Messenger* GraphicalsGroup::get_Messenger_ptr() const
{
   return get_Messenger_ptr(Messenger_ptrs.size()-1);
}

unsigned int GraphicalsGroup::get_n_Messenger_ptrs() const
{
   return Messenger_ptrs.size();
}

Messenger* GraphicalsGroup::get_Messenger_ptr(unsigned int i)
{
   if (i >= 0 && i < Messenger_ptrs.size())
   {
      return Messenger_ptrs[i];
   }
   else
   {
      return NULL;
   }
}

const Messenger* GraphicalsGroup::get_Messenger_ptr(unsigned int i) const
{
   if (i >= 0 && i < Messenger_ptrs.size())
   {
      return Messenger_ptrs[i];
   }
   else
   {
      return NULL;
   }
}

// ---------------------------------------------------------------------
unsigned int GraphicalsGroup::get_n_messages_in_queue() const
{
   return message_queue.size();
}

// ---------------------------------------------------------------------
// Member function parse_latest_messages loops first retrieves all
// messages from ActiveMQ.  It then loops over every message in the
// FIFO queue and calls virtual method parse_next_message_in_queue.
// Finally, this method clears the message_queue.

void GraphicalsGroup::parse_latest_messages()
{
   if (!retrieve_messages()) return;
//   cout << "inside GraphicalsGroup::parse_latest_messages()" << endl;
//   cout << "GraphicalsGroup_name = " << GraphicalsGroup_name << endl;

   int n_messages_handled=0;
   for (unsigned int n=0; n<get_n_messages_in_queue(); n++)
   {
      message curr_message=message_queue[n];
//      cout << "Inside GraphicalsGroup::parse_latest_messages(), n = " << n
//           << " get_n_messages_in_queue() = " << get_n_messages_in_queue()
//           << endl;
//      cout << "curr_message.get_text_message() = " 
//           << curr_message.get_text_message() << endl;
//      cout << "curr_message = " << curr_message << endl;
      if (this->parse_next_message_in_queue(curr_message))
      {
         n_messages_handled++;
      }
   }
//   cout << "GraphicalsGroup_name = " << GraphicalsGroup_name << endl;
//   cout << "n_messages_handled = " << n_messages_handled << endl;

// Recall that we must instantiate a separate Messenger for every
// different GraphicalsGroup which needs a copy of the messages
// coming from some external sender.  So the current GraphicalsGroup
// handles some number of these messages and simply ignores the
// others.  Without loss, we clear out the message_queue for the
// current GraphicalsGroup which has no impact upon other
// GraphicalsGroups' message_queues...

   message_queue.clear();
}

// ---------------------------------------------------------------------
bool GraphicalsGroup::parse_next_message_in_queue(message& curr_message)
{
   cout << "inside GraphicalsGroup::parse_next_message_in_queue() dummy method" 
        << endl;
   return true;
}

// --------------------------------------------------------------------------
// Member function retrieve_messages loops over all Messengers and
// extracts their messages into the GraphicalsGroup's message_queue.

bool GraphicalsGroup::retrieve_messages()
{
   bool some_messages_retrieved_flag=false;
   for (unsigned int i=0; i<Messenger_ptrs.size(); i++)
   {
      if (retrieve_messages(i)) some_messages_retrieved_flag=true;
   }
   return some_messages_retrieved_flag;
}

// This overloaded version of retrieve_messages() extracts messages
// from the ith Messenger into the GraphicalGroup's message_queue.

bool GraphicalsGroup::retrieve_messages(int i)
{
//   cout << "inside GraphicalsGroup::retrieve_messages(int i), i = " << i
//        << endl;
   Messenger* curr_Messenger_ptr=get_Messenger_ptr(i);
   if (curr_Messenger_ptr == NULL) return false;
   
   unsigned int n_mailbox_messages=
      curr_Messenger_ptr->get_n_messages_in_mailbox();
   if (n_mailbox_messages==0) 
   {
      return false;
   }
   else
   {
//      cout << "inside GraphicalsGroup::retrieve_messages()" << endl;
//      cout << "curr_Messenger_ptr = " << curr_Messenger_ptr << endl;
//      cout << "n_mailbox_messages = " << n_mailbox_messages << endl;
//      outputfunc::enter_continue_char();
   }
   
   bool messages_copied_flag=false;
   int counter=0;
   while (!messages_copied_flag && counter < 100)
   {
      messages_copied_flag=curr_Messenger_ptr->
         copy_messages_and_purge_mailbox();
      counter++;
   }
   if (!messages_copied_flag) return false;

   for (unsigned int i=0; i<curr_Messenger_ptr->get_n_curr_messages(); i++)
   {
      const message* curr_message_ptr=curr_Messenger_ptr->get_message_ptr(i);
      if (curr_message_ptr != NULL)
      {
         message_queue.push_back(*curr_message_ptr);
         message_queue.back().extract_and_store_property_keys_and_values();

//         cout << "Message: " << message_queue.back().get_text_message() << endl;
//         cout << "Message i = " << i << " " << message_queue.back() << endl;
      }
   }
//   cout << "curr_Messenger_ptr->n_received_messages = " 
//        << curr_Messenger_ptr->get_n_received_messages() 
//        << endl;

   if (message_queue.size()==0) return false;

   bool flag=(message_queue.front().get_text_message().size() > 0);
//   cout << "flag = " << flag << endl;
   return flag;
}

// --------------------------------------------------------------------------
// Method issue_message sends a SELECT message to the Message Queue
// containing the currently selected Graphical integer ID.

void GraphicalsGroup::issue_selection_message()
{   
//   cout << "inside GG::issue_selection_message()" << endl;
   
   if (get_selected_Graphical_ID() >= 0)
   {
      string text_message="SELECT "+stringfunc::number_to_string(
         get_selected_Graphical_ID());
//      cout << "text_message = " << text_message << endl;
      if (get_Messenger_ptr() != NULL)
         get_Messenger_ptr()->sendTextMessage(text_message);
   }
}

// --------------------------------------------------------------------------
// Member function generate_proximity_messages()

void GraphicalsGroup::generate_proximity_messages(double proximity_distance)
{
//   cout << "inside GraphicalsGroup::generate_proximity_messages()" << endl;

   for (unsigned int c1=0; c1<get_n_Graphicals(); c1++)
   {
      Graphical* Graphical1_ptr=get_Graphical_ptr(c1);

      threevector UVW1;
      if ( Graphical1_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),UVW1))
      {
         for (unsigned int c2=c1+1; c2<get_n_Graphicals(); c2++)
         {
            Graphical* Graphical2_ptr=get_Graphical_ptr(c2);

            threevector UVW2;
            if ( Graphical2_ptr->get_UVW_coords(
               get_curr_t(),get_passnumber(),UVW2))
            {
               double separation_distance=(UVW2-UVW1).magnitude();
               if (separation_distance < proximity_distance)
               {
//                  cout << "Graphical ID = " << Graphical1_ptr->get_ID()
//                       << " is close to Graphical ID = " 
//                       << Graphical2_ptr->get_ID() << endl;
                  string text_message="LINK "+stringfunc::number_to_string(
                     Graphical1_ptr->get_ID())+ " "+
                     stringfunc::number_to_string(Graphical2_ptr->get_ID());
//                  cout << "text message = " << text_message << endl;
                  if (get_Messenger_ptr() != NULL)
                     get_Messenger_ptr()->sendTextMessage(text_message);
               }
            } // Graphical2 get UVW coords conditional
         } // loop over index c2 labeling Graphicals
      } // Graphical1 get UVW coords conditional
   } // loop over index c1 labeling Graphicals
}

// ==========================================================================
// Graphical creation methods
// ==========================================================================

void GraphicalsGroup::initialize_Graphical(
   Graphical* curr_Graphical_ptr,osg::Node* node_ptr)
{   
   threevector UVW(0,0,0);
   initialize_Graphical(UVW,curr_Graphical_ptr,node_ptr);
}

void GraphicalsGroup::initialize_Graphical(
   const threevector& UVW,Graphical* curr_Graphical_ptr,osg::Node* node_ptr)
{   
//   cout << "inside GraphicalsGroup::initialize_Graphical()" << endl;
//   cout << "curr_Graphical name = " << curr_Graphical_ptr->get_name()
//        << endl;
//   cout << "curr_Graphical_ptr = " << curr_Graphical_ptr << endl;
//   cout << "UVW = " << UVW << endl;
//   cout << "AC_ptr = " << AnimationController_ptr << endl;
//   outputfunc::enter_continue_char();

   curr_Graphical_ptr->set_AnimationController_ptr(AnimationController_ptr);

// On 3/4/13, we commented out the following section in order to
// NOT have 2D UV feature coordinates automatically erased for frame
// numbers less than the frame in which a feature is manipulated:

// Recall that some Graphicals exist for all times (e.g. features).
// Yet they generally appear in images spanning only finite time
// intervals.  So we initially erase such Graphicals for all images
// with numbers less than the current image number.


   if (erase_Graphicals_forward_in_time_flag)
   {
      for (unsigned int n=get_first_framenumber(); n<get_curr_framenumber(); 
           n++)
      {
         double curr_t=static_cast<double>(n);
         curr_Graphical_ptr->set_mask(curr_t,get_passnumber(),true);

// Treat any Graphical which is masked for some times and unmasked for
// others as non-stationary:

         curr_Graphical_ptr->set_stationary_Graphical_flag(false);
      }
   }
   else if (erase_Graphicals_except_at_curr_time_flag)
   {
      for (unsigned int n=get_first_framenumber(); n<=get_last_framenumber(); 
           n++)
      {
         if (n == get_curr_framenumber()) continue;
         
         double curr_t=static_cast<double>(n);
         curr_Graphical_ptr->set_mask(curr_t,get_passnumber(),true);

// Treat any Graphical which is masked for some times and unmasked for
// others as non-stationary:

         curr_Graphical_ptr->set_stationary_Graphical_flag(false);
      }
   }
   

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Note added on 3/6/11: We learned the hard and painful way that the
// following section needs to be commented out in order for program
// mains/sift/LADARPAN to work correctly.  If the following section is
// left in, ladar features which have no image tiepoint counterpart
// are assigned XYZ=(0,0,0).  This completely fouls up focal parameter
// and rotation angle extraction in LADARPAN.

// We initialize the new Graphical's coords for all image times.  But
// the Graphical is erased for all images less than the current image
// number:

   const osg::Quat trivial_q(0,0,0,1);
   const threevector trivial_scale(1,1,1);

   for (unsigned int n=get_first_framenumber(); n<=get_last_framenumber(); n++)
   {

// As of 6/5/05, we simply set the time associated with each image
// equal to its imagenumber.  This will eventually need to be
// generalized so that the time field corresponds to a true temporal
// measurement...

      double curr_t=static_cast<double>(n);
      curr_Graphical_ptr->set_UVW_coords(curr_t,get_passnumber(),UVW);
      curr_Graphical_ptr->set_quaternion(
         curr_t,get_passnumber(),trivial_q);
      curr_Graphical_ptr->set_scale(curr_t,get_passnumber(),trivial_scale);
   } // loop over index n labeling image numbers

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Make a note that we have manually manipulated the Graphical's UVW
// coordinates for one particular time in one particular pass:

   curr_Graphical_ptr->set_coords_manually_manipulated(
      get_curr_t(),get_passnumber());

// Add node_ptr to current Graphical's PositionAttitudeTransform:

   if (node_ptr != NULL)
   {
      curr_Graphical_ptr->get_PAT_ptr()->addChild(node_ptr);
   }
}

// --------------------------------------------------------------------------
// Note added on 9/3/08: This method should someday become private.
// It should then be renamed as insert_Graphical_into_map().

// Member function insert_Graphical_into_list

void GraphicalsGroup::insert_Graphical_into_list(
   Graphical* curr_Graphical_ptr)
{
//   cout << "inside GraphicalsGroup::insert_Graphical_into_list()" << endl;
//   cout << "this = " << this << endl;
//   cout << "curr_Graphical_ptr = " << curr_Graphical_ptr << endl;
//   cout << "curr_Graphical_ptr->get_ID() = "
//        << curr_Graphical_ptr->get_ID() << endl;
//   cout << "curr_Graphical_name = " << curr_Graphical_ptr->get_name()
//        << endl;
   
   int curr_ID=curr_Graphical_ptr->get_ID();
   if (graphical_ID_ptrs_map_ptr->find(curr_ID)==
       graphical_ID_ptrs_map_ptr->end())
   {
      int curr_index=get_n_Graphicals();

      (*graphical_ID_ptrs_map_ptr)[curr_ID]=curr_Graphical_ptr;
      graphical_counter_ptrs_vector.push_back(curr_Graphical_ptr);
      (*Graphical_index_ID_map_ptr)[curr_index]=curr_ID;
      (*Graphical_ID_index_map_ptr)[curr_ID]=curr_index;

      graphical_counter++;
   }

// IDs assigned to new Graphicals do not necessarily increment in
// numerical order.  Instead, the next unused ID may correspond to the
// ID for some previous Graphical which was subsequently destroyed.
// In order to retrieve the most recently added Graphical within this
// GraphicalsGroup, we record its ID within member integer
// most_recently_added_ID:

   most_recently_added_ID=curr_Graphical_ptr->get_ID();
//   cout << "most_recently_added_ID = " << most_recently_added_ID
//        << endl;
//   cout << "graphical_counter_ptrs_vector.size() = "
//        << graphical_counter_ptrs_vector.size() << endl;
//   cout << "grapahical_ID_ptrs_map_ptr = "
//        << graphical_ID_ptrs_map_ptr << endl;
//   cout << "graphical_ID_ptrs_map_ptr->size() = "
//        << graphical_ID_ptrs_map_ptr->size() << endl;
}

// ==========================================================================
// Scenegraph node insertion & removal member functions
// ==========================================================================

// Member function insert_graphical_PAT_into_OSGsubPAT attaches the
// input Graphicals' PAT to the OSGsubPAT labeled by input index
// subPAT_number.  If input subPAT_number equals OSGsubPATs.size()
// (rather than lying in the interval [0,OSGsubPATs.size()-1]), a new
// OSGsubPAT is instantiated, and the input Graphical's PAT is
// attached to it.  Otherwise, the insertion is not successful, and
// this boolean method returns false.

bool GraphicalsGroup::insert_graphical_PAT_into_OSGsubPAT(
   Graphical* Graphical_ptr,unsigned int subPAT_number)
{   
//   cout << "inside GraphicalsGroup::insert_graphical_PAT_into_OSGsubPAT" << endl;
//   cout << "this GraphicalsGroup = " << this << endl;
//   cout << "subPAT_number = " << subPAT_number << endl;
//   cout << "OSGsubPATs.size() = " << OSGsubPATs.size() << endl;

//   cout << "Graphical_ptr = " << Graphical_ptr << endl;
//   cout << "Graphical_ptr->get_name() = "
//        << Graphical_ptr->get_name() 
//        << " ID = " << Graphical_ptr->get_ID() << endl;

   if (subPAT_number >= 0 &&  subPAT_number < OSGsubPATs.size())
   {
      OSGsubPATs[subPAT_number]->addChild(Graphical_ptr->get_PAT_ptr());
      OSGsubPATs[subPAT_number]->setNodeMask(1);
      return true;
   }
   else if (subPAT_number==OSGsubPATs.size())
   {
      generate_new_OSGsubPAT();
      return insert_graphical_PAT_into_OSGsubPAT(Graphical_ptr,subPAT_number);
   }
   else
   {
      return false;
   }
}

// -------------------------------------------------------------------------
// Private member function remove_graphical_PAT_from_OSGsubPAT
// searches over all OSG subgroups to find the one containing
// *curr_Graphical_ptr->get_PAT_ptr().  If it's found, this method
// removes the PAT child from the subgroup.  Otherwise, this boolean
// method returns false.

// Note: Not all graphicals' PATs are necessarily inserted as children
// into OSGsubPATs.  For example, LineSegment geodes are directly
// attached to an OSG group member of GraphNode rather than to an
// OSGsubPAT of LineSegmentsGroup.

// Note added on 5/24/13: We empirically found that removing the PAT
// associated with *curr_Graphical_ptr from OSGsubPATs is
// computationally expensive!

bool GraphicalsGroup::remove_graphical_PAT_from_OSGsubPAT(
   Graphical* curr_Graphical_ptr)
{   
//   cout << "inside GraphicalsGroup::remove_graphical_PAT_from_OSGsubPAT()" 
//        << endl;
//   cout << "GraphicalsGroup_name = " << get_name() << endl;
//   cout << "curr_Graphical_ptr->get_name() = "
//        << curr_Graphical_ptr->get_name() << endl;
   bool PAT_removed_flag=false;
   for (unsigned int i=0; i<OSGsubPATs.size(); i++)
   {
      if (OSGsubPATs[i]->removeChild(curr_Graphical_ptr->get_PAT_ptr()))
      {
         PAT_removed_flag=true;
      }
   }

//   if (!PAT_removed_flag)
//   {
//      cout << "OSGsubPAT containing PAT not found" << endl;
//      cout << "curr_Graphical_ptr = " << curr_Graphical_ptr << endl;
//      cout << "curr_Graphical_ptr->get_name() = "
//           << curr_Graphical_ptr->get_name() << endl;
//      cout << "OSGsubPATs.size() = " << OSGsubPATs.size() << endl;
//   }

   return PAT_removed_flag;
}

// --------------------------------------------------------------------------
// Member function insert_OSGgroup_into_OSGsubPAT attaches the input
// group_ptr to the OSGsubPAT labeled by input index subPAT_number.
// If input subPAT_number equals OSGsubPATs.size() (rather than
// lying in the interval [0,OSGsubPATs.size()-1]), a new OSGsubPAT is
// instantiated, and the input group_ptr is attached to it.
// Otherwise, the insertion is not successful, and this boolean method
// returns false.

bool GraphicalsGroup::insert_OSGgroup_into_OSGsubPAT(
   osg::Group* group_ptr,unsigned int subPAT_number)
{   
//   cout << "inside GraphicalsGroup::insert_OSGgroup_into_OSGsubPAT()" << endl;
//   cout << "GraphicalsGroup_name = " << GraphicalsGroup_name << endl;
//   cout << "subPAT_number = " << subPAT_number << endl;

   if (subPAT_number >= 0 && subPAT_number < OSGsubPATs.size())
   {

// First check whether group_ptr already exists as a child node within
// OSGsubPAT:
      
      if (OSGsubPATs[subPAT_number]->containsNode(group_ptr))
      {
//         cout << "group_ptr = " << group_ptr << " already exists in subPAT!"
//              << endl;
//         outputfunc::enter_continue_char();
      }
      else
      {
         OSGsubPATs[subPAT_number]->addChild(group_ptr);
         OSGsubPATs[subPAT_number]->setNodeMask(1);
      }
      return true;
   }
   else if (subPAT_number==OSGsubPATs.size())
   {
      generate_new_OSGsubPAT();
      return insert_OSGgroup_into_OSGsubPAT(group_ptr,subPAT_number);
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
// Member function remove_OSGgroup_from_OSGsubPAT

bool GraphicalsGroup::remove_OSGgroup_from_OSGsubPAT(osg::Group* group_ptr)
{   
//   cout << "inside GraphicalsGroup::remove_OSGgroup_from_OSGsubPAT()" 
//        << endl;
//   cout << "GraphicalsGroup_name = " << GraphicalsGroup_name << endl;
//   cout << "group_ptr = " << group_ptr << endl;
   
   bool OSGgroup_removed_flag=false;
   if (group_ptr != NULL)
   {

      for (unsigned int i=0; i<OSGsubPATs.size(); i++)
      {
         if (OSGsubPATs[i]->removeChild(group_ptr))
         {
            OSGgroup_removed_flag=true;
//         break;
         }
      }
   }
   
   if (!OSGgroup_removed_flag)
   {
//      cout << "OSGsubPAT containing OSGgroup not found" << endl;
//      cout << "group_ptr = " << group_ptr << endl;
//      cout << "OSGsubPATs.size() = " << OSGsubPATs.size() << endl;
   }

   return OSGgroup_removed_flag;
}

// ==========================================================================
// Graphical manipulation methods
// ==========================================================================

// Member function change_size multiplies the size parameter for
// objects corresponding to the current dimension by input parameter factor.

void GraphicalsGroup::change_size(double factor)
{   
//   cout << "inside GraphicalsGroup::change_size()" << endl;
//   cout << "factor = " << factor << endl;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Graphical* Graphical_ptr=get_Graphical_ptr(n);
      Graphical_ptr->set_size(Graphical_ptr->get_size()*factor);
//      cout << "n = " << n << " Graphical_ptr->get_size() = "
//           << Graphical_ptr->get_size() << endl;
   }
}

void GraphicalsGroup::set_size(double size)
{   
//   cout << "inside GraphicalsGroup::set_size()" << endl;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Graphical* Graphical_ptr=get_Graphical_ptr(n);
//      cout << "n = " << n << " initially, Graphical_ptr->get_size() = "
//           << Graphical_ptr->get_size() << endl;
      Graphical_ptr->set_size(size);
//      cout << "n = " << n << " Graphical_ptr->get_size() = "
//           << Graphical_ptr->get_size() << endl;
   }
}

// --------------------------------------------------------------------------
// Member function set_constant_scale rescales all 3-dimensions of the
// graphical labeled by input integer ID for all image times.

void GraphicalsGroup::set_constant_scale(
   Graphical* curr_Graphical_ptr,double scale)
{   
//   cout << "inside GraphicalsGroup::set_constant_scale(), scale = " 
//        << scale << endl;
//   cout << "name = " << get_name() << endl;
   if (curr_Graphical_ptr != NULL)
   {
      threevector Graphical_scale(scale,scale,scale);
      for (unsigned int n=get_first_framenumber(); n<=get_last_framenumber(); n++)
      {
         double curr_t=static_cast<double>(n);
         curr_Graphical_ptr->set_scale(
            curr_t,get_passnumber(),Graphical_scale);
      } // loop over index n labeling image numbers
   } // curr_Graphical_ptr != NULL conditional
}

// --------------------------------------------------------------------------
// Member function rescale takes in integer d labeling the X, Y or Z
// axes.  It also takes in scale_factor by which the current Graphical

Graphical* GraphicalsGroup::rescale(int d,double scale_factor)
{   
   Graphical* curr_Graphical_ptr=get_ID_labeled_Graphical_ptr(
      get_selected_Graphical_ID());
   if (curr_Graphical_ptr != NULL)
   {
      threevector Graphical_scale(1,1,1);
      curr_Graphical_ptr->get_scale(
         get_curr_t(),get_passnumber(),Graphical_scale);
      Graphical_scale.put(d,Graphical_scale.get(d)*scale_factor);
      curr_Graphical_ptr->set_scale(
         get_curr_t(),get_passnumber(),Graphical_scale);
   } // curr_Graphical_ptr != NULL conditional
   return curr_Graphical_ptr;
}

// --------------------------------------------------------------------------
// Member function get_selected_Graphical_z returns the z-coordinate
// for the current selected Graphical or NEGATIVEINFINTITY if no
// Graphical is selected.

double GraphicalsGroup::get_selected_Graphical_z() 
{
//   cout << "inside GraphicalsGroup::get_selected_Graphical_z()" << endl;
   
   if (get_ndims()==3 && get_selected_Graphical_ID() >= 0)
   {
      Graphical* curr_Graphical_ptr=get_ID_labeled_Graphical_ptr(
         get_selected_Graphical_ID());

      threevector Graphical_posn;
      if (curr_Graphical_ptr->get_UVW_coords(
         get_curr_t(),get_passnumber(),Graphical_posn))
      {
         return Graphical_posn.get(2);
      }
   } // ndims==3 conditional
   return NEGATIVEINFINITY;
}

// --------------------------------------------------------------------------
// Member function move_z moves 3D Graphicals up or down in
// world-space z.  This method was constructed to facilitate movement
// of 3D objects (crosshairs, boxes, maps, etc) in the z direction.
// It returns a pointer to the Graphical object which was moved.

Graphical* GraphicalsGroup::move_z(int sgn)
{
//   cout << "inside GG::move_z(sgn)" << endl;
   return move_z(sgn*delta_move_z);
}

// --------------------------------------------------------------------------
Graphical* GraphicalsGroup::move_z(double delta_z)
{
//   cout << "inside GraphicalsGroup::move_z()" << endl;
   
   Graphical* curr_Graphical_ptr=NULL;
   if (get_ndims()==3 && get_selected_Graphical_ID() >= 0)
   {
//      cout << "get_selected_Graphical_ID() = " << get_selected_Graphical_ID()
//           << endl;

      curr_Graphical_ptr=get_ID_labeled_Graphical_ptr(
         get_selected_Graphical_ID());

//      cout << "curr_Graphical_ptr = " << curr_Graphical_ptr << endl;

      threevector Graphical_posn;
      if (curr_Graphical_ptr->get_UVW_coords(
         get_curr_t(),get_passnumber(),Graphical_posn))
      {
//            cout.precision(12);
//            cout << "Graphical_posn = " << Graphical_posn << endl;
//            cout << "curr_t = " << get_curr_t() << endl;

         if (Ellipsoid_model_ptr != NULL)
         {

// If Ellipsoid_model_ptr is defined, we assume that the Graphical to
// be moved is located near the blue marble surface.  We then alter
// its altitude rather than its z coordinate:

            double longitude,latitude,altitude;
            convert_XYZ_to_LongLatAlt(
               Graphical_posn,longitude,latitude,altitude);

//               cout << "altitude = " << altitude << endl;
               
            altitude += delta_z;
            convert_LongLatAlt_to_XYZ(longitude,latitude,altitude,
                                      Graphical_posn);
         }
         else
         {
            Graphical_posn.put(2,Graphical_posn.get(2)+delta_z);
         }
         curr_Graphical_ptr->set_UVW_coords(
            get_curr_t(),get_passnumber(),Graphical_posn);
            
//            threevector curr_posn;
//            curr_Graphical_ptr->get_UVW_coords(
//               get_curr_t(),get_passnumber(),curr_posn);
//            cout << "curr Graphical posn = " << curr_posn << endl;
//            cout << "curr Graphical z = " << curr_posn.get(2) << endl;
      }
   } // ndims==3 conditional
   return curr_Graphical_ptr;
}

// --------------------------------------------------------------------------
// Member function copy takes in the image number for some Graphical
// which is to be copied to the current image number.  It then
// transfers the position, attitude and scale information from the
// former time to the current one.

void GraphicalsGroup::copy(
   unsigned int orig_imagenumber,unsigned int start_copy_imagenumber,
   unsigned int stop_copy_imagenumber)
{   
   double orig_t=static_cast<double>(orig_imagenumber);
   Graphical* curr_Graphical_ptr=get_ID_labeled_Graphical_ptr(
      get_selected_Graphical_ID());
   if (curr_Graphical_ptr != NULL)
   {
      threevector UVW;
      curr_Graphical_ptr->get_UVW_coords(orig_t,get_passnumber(),UVW);

      osg::Quat q;
      curr_Graphical_ptr->get_quaternion(orig_t,get_passnumber(),q);

      threevector Graphical_scale;
      curr_Graphical_ptr->get_scale(orig_t,get_passnumber(),Graphical_scale);

      for (unsigned int n=start_copy_imagenumber; n<=stop_copy_imagenumber; 
           n++)
      {
         double t=static_cast<double>(n);
         curr_Graphical_ptr->set_UVW_coords(t,get_passnumber(),UVW);
         curr_Graphical_ptr->set_quaternion(t,get_passnumber(),q);
         curr_Graphical_ptr->set_scale(t,get_passnumber(),Graphical_scale);
      } // loop over index n labeling images with cloned Rectangles 
   } // curr_Graphical_ptr != NULL conditional
}

// ==========================================================================
// Graphical erasing methods
// ==========================================================================

// Member function erase_Graphical sets boolean entries within the
// member map coords_erased to true for the current Graphical.  When
// Graphicals are drawn in GraphicalsGroup::reassign_PAT_ptrs(),
// entries within this STL map are first checked and they are masked
// to prevent them from appearing within the OSG data window.  Yet the
// Graphical itself continues to exist.

bool GraphicalsGroup::erase_Graphical()
{   
   return erase_Graphical(get_selected_Graphical_ID());
}

bool GraphicalsGroup::erase_Graphical(int Graphical_ID)
{   
//   cout << "inside GG::erase_Graphical()" << endl;
//   cout << "Graphical_ID = " << Graphical_ID << endl;
   bool erased_Graphical_flag=false;
   Graphical* Graphical_ptr=get_ID_labeled_Graphical_ptr(Graphical_ID);
   if (Graphical_ptr != NULL)
   {
      for (unsigned int n=get_first_framenumber(); n<=get_last_framenumber(); n++)
      {

// As of 6/5/05, we simply set the time associated with each image in
// the current pass equal to its imagenumber.  This will eventually
// need to be generalized so that the time field corresponds to a true
// temporal measurement...

         double curr_t=static_cast<double>(n);
         Graphical_ptr->set_mask(curr_t,get_passnumber(),true);
      }
//      cout << "Masked Graphical " << Graphical_ID << " for all times" 
//           << endl;
      erased_Graphical_flag=true;
   } // Graphical_ptr != NULL conditional
   return erased_Graphical_flag;
}

bool GraphicalsGroup::erase_Graphical(double t,int Graphical_ID)
{   
   bool erased_Graphical_flag=false;
   Graphical* Graphical_ptr=get_ID_labeled_Graphical_ptr(Graphical_ID);
   if (Graphical_ptr != NULL)
   {
      Graphical_ptr->set_mask(t,get_passnumber(),true);
      erased_Graphical_flag=true;
   } // currnode_ptr != NULL conditional
   return erased_Graphical_flag;
}

void GraphicalsGroup::erase_all_Graphicals()
{   
   for (unsigned int i=0; i<get_n_Graphicals(); i++)
   {
      erase_Graphical(get_Graphical_ptr(i)->get_ID());
   }
}

// --------------------------------------------------------------------------
// Member function unerase_Graphical queries the user to enter the ID
// for some erased Graphical.  It then unerases that Graphical within
// the current image.

bool GraphicalsGroup::unerase_Graphical()
{   
   bool Graphical_unerased_flag=false;

   string label_command="Enter Graphical number to unerase in current image:";
   int unerased_Graphical_ID=inputfunc::enter_nonnegative_integer(
      label_command);

   Graphical* curr_Graphical_ptr=get_ID_labeled_Graphical_ptr(
      unerased_Graphical_ID);
   if (curr_Graphical_ptr==NULL)
   {
      cout << "Input label does not correspond to any existing Graphical"
           << endl;
   }
   else
   {
      if (!curr_Graphical_ptr->get_mask(get_curr_t(),get_passnumber()))
      {
         cout << "Graphical already exists in current image" << endl;
      }
      else
      {
         curr_Graphical_ptr->set_mask(
            get_curr_t(),get_passnumber(),false);
         set_selected_Graphical_ID(unerased_Graphical_ID);
//         cout << "Unerased Graphical " << unerased_Graphical_ID << endl;
         Graphical_unerased_flag=true;
      }
   } // currnode_ptr==NULL conditional

   return Graphical_unerased_flag;
}

bool GraphicalsGroup::unerase_Graphical(int Graphical_ID)
{   
   bool Graphical_unerased_flag=false;

   Graphical* Graphical_ptr=get_ID_labeled_Graphical_ptr(Graphical_ID);
   if (Graphical_ptr != NULL)
   {
      for (unsigned int n=get_first_framenumber(); n<=get_last_framenumber(); n++)
      {
         double curr_t=static_cast<double>(n);
         Graphical_ptr->set_mask(curr_t,get_passnumber(),false);
         Graphical_unerased_flag=true;
      }
   } // Graphical_ptr != NULL conditional
   return Graphical_unerased_flag;
}

void GraphicalsGroup::unerase_all_Graphicals()
{   
   for (unsigned int i=0; i<get_n_Graphicals(); i++)
   {
      unerase_Graphical(get_Graphical_ptr(i)->get_ID());
   }
}

// --------------------------------------------------------------------------
// Member function destroy_Graphical removes the selected Graphical
// from *graphical_ID_ptrs_map_ptr, graphical_counter_ptrs_vector and the OSG
// Graphicals group.  If the Graphical is successfully destroyed, its
// number is returned by this method.  Otherwise, -1 is returned.

int GraphicalsGroup::destroy_Graphical()
{   
//   cout << "inside GraphicalsGroup::destroy_Graphical()" << endl;
   int destroyed_Graphical_number=-1;
   if (destroy_Graphical(get_selected_Graphical_ID()))
   {
      destroyed_Graphical_number=get_selected_Graphical_ID();
//      cout << "Destroyed Graphical " << destroyed_Graphical_number << endl;
      set_selected_Graphical_ID(-1);
   }
//   cout << "destroyed_Graphical_number = " << destroyed_Graphical_number
//        << endl;
   return destroyed_Graphical_number;
}

bool GraphicalsGroup::destroy_Graphical(int graphical_ID)
{   
//   cout << "inside GraphicalsGroup::destroy_Graphical, ID = " 
//        << graphical_ID << endl;
   Graphical* curr_Graphical_ptr=get_ID_labeled_Graphical_ptr(graphical_ID);
   return destroy_Graphical(curr_Graphical_ptr);
}

// In order to correctly remove a Graphical from the scenegraph, we
// should NEVER call just a bare C++ delete command.  Instead,
// destroy_Graphical should be called so that it removes child nodes
// from the scene graph and reference pointers are correctly
// decremented.  As of Aug 2007, we believe that OSGgroup_refptr
// should have a reference count of +1 at the end of this
// destroy_Graphical method.  

// We also realized the hard and painful way in Aug 2007 that special
// Graphicals (e.g. PolyLine) which contain other GraphicalGroups
// (e.g. PointsGroup) inside of them must have their own specialized
// destroy methods (e.g. destroy_PolyLine).  Those methods must
// explicitly unattach the other GraphicalsGroups OSGgroups from the
// scenegraph and before calling the following destroy_Graphical()
// method...

bool GraphicalsGroup::destroy_Graphical(Graphical* curr_Graphical_ptr)
{   
//   cout << "inside GraphicalsGroup::destroy_Graphical()" << endl;
//   cout << "GraphicalsGroup_name = " << GraphicalsGroup_name << endl;
//   cout << "curr_Graphical_ptr = " << curr_Graphical_ptr << endl;

   if (curr_Graphical_ptr != NULL)
   {
      remove_graphical_PAT_from_OSGsubPAT(curr_Graphical_ptr);
//      cout << "curr_Graphical_ptr->get_ID() = "
//           << curr_Graphical_ptr->get_ID() << endl;

// Perform brute-force search over all entries within STL member
// vector graphical_counter_ptr_vector for Graphical whose ID matches
// that of *curr_Graphical_ptr.  Once it's found, erase this
// Graphical's pointer from graphical_counter_ptrs_vector:

      for (vector<Graphical*>::iterator iter=
              graphical_counter_ptrs_vector.begin(); 
           iter != graphical_counter_ptrs_vector.end(); iter++)
      {
         if ( (*iter)->get_ID()==curr_Graphical_ptr->get_ID())
         {
            graphical_counter_ptrs_vector.erase(iter);
            break;
         }
      }      

      int curr_ID=curr_Graphical_ptr->get_ID();
      GRAPHICAL_ID_INDEX_MAP::iterator index_iter=
         Graphical_ID_index_map_ptr->find(curr_ID);
      int curr_index=index_iter->second;

      graphical_ID_ptrs_map_ptr->erase(curr_ID);
      Graphical_ID_index_map_ptr->erase(curr_ID);
      Graphical_index_ID_map_ptr->erase(curr_index);

// Reset selected_Graphical_ID to -1 if it currently equals
// curr_Graphical_ptr->get_ID():

      if (get_selected_Graphical_ID()==curr_Graphical_ptr->get_ID())
      {
         set_selected_Graphical_ID(-1);
      }

      delete curr_Graphical_ptr;
      return true;
   }


   return false;
}

// -------------------------------------------------------------------------
// Member function destroy_all_Graphicals deletes all children of the
// OSGgroup member and purges all entries within
// *graphical_ID_ptrs_map_ptr & graphical_counter_ptrs_vector.

void GraphicalsGroup::destroy_all_Graphicals()
{   
//   cout << "inside GraphicalsGroup::destroy_all_Graphicals()" << endl;
//   cout << "GraphicalsGroup_name = " << GraphicalsGroup_name << endl;
//   cout << "this = " << this << endl;
   unsigned int n_Graphicals=get_n_Graphicals();
//   cout << "n_graphicals = " << n_Graphicals << endl;
   for (unsigned int n=0; n<n_Graphicals; n++)
   {
      Graphical* Graphical_ptr=get_Graphical_ptr(n);
      remove_graphical_PAT_from_OSGsubPAT(Graphical_ptr);
      delete Graphical_ptr;
   }

   graphical_ID_ptrs_map_ptr->clear();
   graphical_counter_ptrs_vector.clear();
   Graphical_index_ID_map_ptr->clear();
   Graphical_ID_index_map_ptr->clear();
}

// -------------------------------------------------------------------------
// Member function renumber_all_Graphicals loops over all graphicals
// and assigns their IDs equal to integers ranging from 0 to
// n_Graphicals-1.  We wrote this little utility method to facilitate
// 3D tie-point picking for down-selected KLT tracked 2D video
// features.

void GraphicalsGroup::renumber_all_Graphicals()
{   
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Graphical* graphical_ptr=get_Graphical_ptr(n);
      renumber_Graphical(graphical_ptr,n);
      graphical_ptr->set_ID(n);
   }
}

void GraphicalsGroup::renumber_Graphical(
   Graphical* curr_Graphical_ptr,int new_ID)
{   
   graphical_ID_ptrs_map_ptr->erase(curr_Graphical_ptr->get_ID());
   (*graphical_ID_ptrs_map_ptr)[new_ID]=curr_Graphical_ptr;
}

// ==========================================================================
// Ascii file I/O methods
// ==========================================================================

// Member function get_default_info_filename retrieves the name of the
// file containing the 2D or 3D data.  It strips off this file's
// suffix and replaces it with one that indicates the data set's
// dimensionality.  This new filename is used to store Graphical
// information.

string GraphicalsGroup::get_default_info_filename(
   string Graphicals_prefix) const
{
   return Graphicals_prefix+ndims_label+"_"+pass_ptr->get_passname_prefix()
      +".txt";
}

// --------------------------------------------------------------------------
string GraphicalsGroup::get_output_filename(string Graphicals_prefix) const
{
   string output_filename;
   cout << "Enter output file name in which "+Graphicals_prefix
      +" info will be stored:" << endl;
   cout << "Default output filename = " << get_default_info_filename(
      Graphicals_prefix) << endl;
   cout << "(Enter 'd' to indicate default filename should be used)" << endl;
   cin >> output_filename;
   if (output_filename=="d") output_filename=get_default_info_filename(
      Graphicals_prefix);
   cout << "Saving "+Graphicals_prefix+" information to output text file ";
   cout << output_filename << endl << endl;
   return output_filename;
}

// --------------------------------------------------------------------------
string GraphicalsGroup::get_input_filename(string Graphicals_prefix) const
{
   string input_filename;
   cout << "Enter name of input text file containing "+
      Graphicals_prefix+" information:" << endl;
   cout << "Default input filename = " 
        << get_default_info_filename(Graphicals_prefix) << endl;
   cout << "(Enter 'd' to indicate default filename should be used)" << endl;
   cin >> input_filename;
   if (input_filename=="d") 
      input_filename=get_default_info_filename(Graphicals_prefix);
   cout << "Reading "+Graphicals_prefix+" information from input text file ";
   cout << input_filename << endl << endl;
   return input_filename;
}

// --------------------------------------------------------------------------
// Member function read_OSG_file() reads the contents of an input .osg
// file into a new node and appends it to *get_OSGgroup_ptr().

void GraphicalsGroup::read_OSG_file()
{
   string osg_filename;
   cout << "Enter name of input .osg file:" << endl;
   cin >> osg_filename;
   
   osg::Node* Node_ptr=osgDB::readNodeFile(osg_filename);
   if (Node_ptr != NULL)
   {
      osg::PositionAttitudeTransform* OSGsubPAT_ptr=get_OSGsubPAT_ptr(0);
      OSGsubPAT_ptr->addChild(Node_ptr);
      toggle_OSGsubPAT_nodemask(0);
   }
}

// --------------------------------------------------------------------------
// Member function write_OSG_file() writes the contents of
// *get_OSGgroup_ptr() to an output .osg file. 

void GraphicalsGroup::write_OSG_file()
{
   outputfunc::write_banner("Writing OSGgroup to output OSG file:");
   
   ofstream binary_outstream;
   string output_filename="Graphicals.osg";
   filefunc::deletefile(output_filename);
   
   if (osgDB::writeNodeFile(
      *(get_OSGgroup_ptr()) , output_filename ) )
   {
      cout << "Wrote .osg file " << output_filename << endl;
   }
   else
   {
      cout << "Could not write output .osg file" << endl;
   }
}

// ==========================================================================
// Global manipulation member functions
// ==========================================================================

void GraphicalsGroup::initialize_const_posn(
   const threevector& posn,Graphical* curr_Graphical_ptr)
{
   for (unsigned int n=get_first_framenumber(); n<=get_last_framenumber(); n++)
   {
      double curr_t=static_cast<double>(n);
      curr_Graphical_ptr->set_UVW_coords(curr_t,get_passnumber(),posn);
   }
}

// ---------------------------------------------------------------------
void GraphicalsGroup::rotate_about_zaxis(
   double curr_t,int pass_number,const threevector& rotation_origin,
   double phi_z)
{
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      get_Graphical_ptr(n)->rotate_about_zaxis(
         curr_t,pass_number,rotation_origin,phi_z);
   } // loop over index n labeling images
}

// ==========================================================================
// Animation methods
// ==========================================================================

unsigned int GraphicalsGroup::get_Nimages() const
{
   if (AnimationController_ptr != NULL)
   {
      return AnimationController_ptr->get_nframes();
   }
   else
   {
      return 1;
   }
}

// --------------------------------------------------------------------------
unsigned int GraphicalsGroup::get_curr_framenumber() const
{
   if (AnimationController_ptr != NULL)
   {
      return AnimationController_ptr->get_curr_framenumber();
   }
   else
   {
      return 0;
   }
}

// --------------------------------------------------------------------------
unsigned int GraphicalsGroup::get_first_framenumber() const
{
   if (AnimationController_ptr != NULL)
   {
      return AnimationController_ptr->get_first_framenumber();
   }
   else
   {
      return 0;
   }
}

// --------------------------------------------------------------------------
unsigned int GraphicalsGroup::get_last_framenumber() const
{
//   cout << "inside GraphicalsGroup::get_last_framenumber() " << endl;
   if (AnimationController_ptr != NULL)
   {
      return AnimationController_ptr->get_last_framenumber();
   }
   else
   {
      return get_Nimages()-1;
   }
}

// --------------------------------------------------------------------------
// Member function update_display implements Ross Anderson's
// suggestion to instantiate Graphicals centered about some canonical
// origin but then to move them to their actual location each time a
// callback from the infinite viewer loop is executed.  Current
// graphical position and attitude information are stored within
// PositionAttitudeTransform pointers located inside *OSGgroup_refptr.
// This method updates the PAT positions for each Graphical whose
// masked flag currently equals false.

void GraphicalsGroup::update_display()
{   
//   cout << "=============================================" << endl;
//   cout << "inside GraphicalsGroup::update_display()" << endl;
//   cout << "GraphicalsGroup_name = " << get_name() << endl;
//   cout << "this = " << this << endl;
//   cout << "OSGgroup_refptr.get() = " << OSGgroup_refptr.get() << endl;
//   cout << "OSGgroup_refptr->NumChildren = " 
//        << OSGgroup_refptr->getNumChildren() << endl;
//   cout << "OSGsubPATs.size() = " << OSGsubPATs.size() << endl;
//   cout << "OSGsubPATs.back().get() = " << OSGsubPATs.back().get() << endl;
//   cout << "OSGsubPATs.back().get()->getNumChildren() = "
//        << OSGsubPATs.back().get()->getNumChildren() << endl;
//   cout << "graphical_ID_ptrs_map_ptr->size() = "
//        << graphical_ID_ptrs_map_ptr->size() << endl;
//   cout << "n_Graphicals = " << get_n_Graphicals() << endl;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
//      Graphical* Graphical_ptr=get_Graphical_ptr(n);
//      cout << "n = " << n << " Graphical_ptr = " << Graphical_ptr << endl;
//      cout << "Graphical_name = " << Graphical_ptr->get_name() << endl;
      
      get_Graphical_ptr(n)->set_PAT(get_curr_t(),get_passnumber());

//      cout << "get_curr_t() = " << get_curr_t() 
//           << " passnumber = " << get_passnumber() << endl;
//      cout << "Graphical_ptr = " << Graphical_ptr << endl;
//      cout << "Graphical_ptr->get_PAT_ptr() = " 
//           << Graphical_ptr->get_PAT_ptr() << endl;

/*
      if (graphical_ID_ptrs_map_ptr->size()==1)
      {
         threevector p;
         if (Graphical_ptr->get_UVW_coords(
            get_curr_t(),get_passnumber(),p))
         {
            cout << "Graphical_ptr = " << Graphical_ptr << endl;
            cout << "Graphical_ptr->get_PAT_ptr() = "
                 << Graphical_ptr->get_PAT_ptr() << endl;
            cout << "Graphical_ptr->get_PAT_ptr()->getNumChildren() = "
                 << Graphical_ptr->get_PAT_ptr()->getNumChildren() << endl;
            cout << "t = " << get_curr_t()
                 << " passnumber = " << get_passnumber() << endl;
            cout << "posn = " << threevector(Graphical_ptr->get_PAT_ptr()->
                                             getPosition()) << endl;
            cout << "PivotPoint = " 
                 << threevector(Graphical_ptr->get_PAT_ptr()->
                                getPivotPoint()) << endl;
            cout << "scale = " 
                 << threevector(Graphical_ptr->get_PAT_ptr()->getScale())
                 << endl;
            osg::Quat q=Graphical_ptr->get_PAT_ptr()->getAttitude();
            cout << "Quat = " 
                 << q._v[0] << " , " 
                 << q._v[1] << " , " 
                 << q._v[2] << " , " 
                 << q._v[3] << endl;
            cout << "x = " << p.get(0) 
                 << " y = " << p.get(1)
                 << " z = " << p.get(2) << endl;
         }
      } // graphical_ID_ptrs_map_ptr->size() >= 1 conditional
*/

   } // loop over index n labeling Graphicals within GraphicalsGroup

//   cout << "at end of GraphicalsGroup::update_display()" << endl;
}

// ==========================================================================
// Earth ellipsoid methods
// ==========================================================================

bool GraphicalsGroup::convert_XYZ_to_LongLatAlt(
   const threevector& XYZ,double& longitude,double& latitude,double& altitude)
{   
//   cout << "inside GraphicalsGroup::convert_XYZ_to_LongLatAlt()" << endl;
   
   bool successful_conversion_flag=false;
   if (Ellipsoid_model_ptr != NULL)
   {
      Ellipsoid_model_ptr->ConvertXYZToLongLatAlt(
         XYZ,longitude,latitude,altitude);
//      cout << "Longitude = " << longitude
//           << " latitude = " << latitude
//           << " altitude = " << altitude << endl;
      successful_conversion_flag=true;
   }
   return successful_conversion_flag;
}

// --------------------------------------------------------------------------
bool GraphicalsGroup::convert_LongLatAlt_to_XYZ(
   double longitude,double latitude,double altitude,threevector& UVW)
{   
   bool successful_conversion_flag=false;
   if (Ellipsoid_model_ptr != NULL && Clock_ptr != NULL)
   {
      UVW=Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
         longitude,latitude,altitude);
      successful_conversion_flag=true;
   }
   return successful_conversion_flag;
}

// --------------------------------------------------------------------------
bool GraphicalsGroup::convert_ECI_to_LongLatAlt(
   const threevector& UVW,double& longitude,double& latitude,double& altitude)
{   
//   cout << "inside GraphicalsGroup::convert_ECI_to_LongLatAlt()" << endl;
   bool successful_conversion_flag=false;
   if (Ellipsoid_model_ptr != NULL && Clock_ptr != NULL)
   {
      Ellipsoid_model_ptr->ConvertECIToLongLatAlt(
         UVW,*Clock_ptr,longitude,latitude,altitude);
//      cout << "longitude = " << longitude
//           << " latitude = " << latitude
//           << " altitude = " << altitude << endl;
      successful_conversion_flag=true;
   }
   return successful_conversion_flag;
}

// --------------------------------------------------------------------------
bool GraphicalsGroup::convert_LongLatAlt_to_ECI(
   double longitude,double latitude,double altitude,threevector& UVW)
{   
   bool successful_conversion_flag=false;
   if (Ellipsoid_model_ptr != NULL && Clock_ptr != NULL)
   {
      UVW=Ellipsoid_model_ptr->ConvertLongLatAltToECI(
         longitude,latitude,altitude,*Clock_ptr);
      successful_conversion_flag=true;
   }
   return successful_conversion_flag;
}

// --------------------------------------------------------------------------
// Member function compute_local_ellipsoid_directions calculates local
// east_hat and north_hat direction vectors on the earth ellipsoid at
// the input longitude and latitude coordinates.

void GraphicalsGroup::compute_local_ellipsoid_directions(
   double longitude,double latitude)
{   
   Ellipsoid_model_ptr->east_north_radial_to_ECI_rotation(
      latitude,longitude,*Clock_ptr);
}

// --------------------------------------------------------------------------
// Member function rotate_zhat_to_rhat reorient's the input
// Geometrical's primary symmetry axis from pointing along +z_hat to
// the current radial direction along the earth's surface.

void GraphicalsGroup::rotate_zhat_to_rhat(Graphical* Graphical_ptr)
{
//   cout << "inside GG::rotate_zhat_to_rhat()" << endl;
   if (Ellipsoid_model_ptr != NULL)
   {
      threevector XYZ;
      Graphical_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),XYZ);

      double longitude,latitude,altitude;
      Ellipsoid_model_ptr->ConvertXYZToLongLatAlt(
         XYZ,longitude,latitude,altitude);
      osg::Quat q=Ellipsoid_model_ptr->rotate_zhat_to_rhat(
         longitude,latitude);
      Graphical_ptr->set_quaternion(get_curr_t(),get_passnumber(),q);
   }
}
