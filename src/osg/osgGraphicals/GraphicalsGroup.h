// ==========================================================================
// Header file for pure virtual GRAPHICALSGROUP class
// ==========================================================================
// Last modified on 5/24/13; 3/22/14; 4/5/14; 6/16/14
// ==========================================================================

#ifndef GRAPHICALSGROUP_H
#define GRAPHICALSGROUP_H

#include <deque>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <osg/Group>
#include <osg/PositionAttitudeTransform>
#include "osg/AbstractOSGCallback.h"	// needed by most classes which 
					// inherit from GraphicalsGroup

#include "osg/osgGraphicals/AnimationController.h"
#include "astro_geo/Clock.h"
#include "datastructures/Linkedlist.h"
#include "messenger/message.h"
#include "passes/Pass.h"

class Ellipsoid_model;
class Graphical;
class Messenger;
class PassesGroup;
class threevector;

class GraphicalsGroup 
{

  public:

// Initialization, constructor and destructor functions:

   GraphicalsGroup(int p_ndims,PassesGroup* PG_ptr,threevector* GO_ptr=NULL);
   GraphicalsGroup(int p_ndims,Pass* PI_ptr,threevector* GO_ptr=NULL);
   GraphicalsGroup(int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr,
       threevector* GO_ptr=NULL);
   GraphicalsGroup(
       int p_ndims,Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
       threevector* GO_ptr=NULL);
   virtual ~GraphicalsGroup();

// Set & get methods:

   void set_erase_Graphicals_forward_in_time_flag(bool flag);
   void set_erase_Graphicals_except_at_curr_time_flag(bool flag);
   std::string get_name() const;
   void set_ndims(unsigned int ndims);
   unsigned int get_ndims() const;
   void set_extra_textmessage_info(std::string info);
   void set_AnimationController_ptr(AnimationController* AC_ptr);
   AnimationController* get_AnimationController_ptr() const;
   void set_Clock_ptr(Clock* clock_ptr);
   void set_Ellipsoid_model_ptr(Ellipsoid_model* emodel_ptr);
   Ellipsoid_model* get_Ellipsoid_model_ptr();
   const Ellipsoid_model* get_Ellipsoid_model_ptr() const;
   void set_GraphicalsGroup_bug_ptr(GraphicalsGroup* GG_ptr);

   void set_update_display_flag(bool flag);
   void set_delta_move_z(double dz);

   void set_selected_Graphical_ID(int n);
   int get_selected_Graphical_ID() const;
   int get_prev_selected_Graphical_ID() const;
   int get_selected_Graphical_ID(unsigned int n) const;
   Graphical* get_selected_Graphical_ptr() const;

   int get_selected_OSGsubPAT_ID();
   threevector& get_grid_world_origin() const;
   threevector* get_grid_world_origin_ptr();
   const threevector* get_grid_world_origin_ptr() const;
   void set_MatrixTransform_ptr(osg::MatrixTransform* MT_ptr);
   osg::MatrixTransform* get_MatrixTransform_ptr();
   osg::Group* get_OSGgroup_ptr();
   int get_OSGgroup_refptr_count() const;
   
   unsigned int get_n_Graphicals() const;
   void set_Graphical_counter(int g);
   int get_Graphical_counter() const;
   Graphical* get_Graphical_ptr(unsigned int n) const;
   Graphical* get_ID_labeled_Graphical_ptr(int ID) const;
   Graphical* get_last_Graphical_ptr() const;
   Graphical* get_most_recently_added_Graphical_ptr() const;
   std::vector<Graphical*> get_all_Graphical_ptrs() const;
   double get_initial_t() const; 
   double get_final_t() const;
   double get_curr_t() const; 

// Pass info get member functions:

   int get_max_n_passes(double t) const;
   std::vector<int> get_all_pass_numbers(double t) const;
   Pass* get_pass_ptr();
   const Pass* get_pass_ptr() const;
   void set_PassesGroup_ptr(PassesGroup* PG_ptr);
   PassesGroup* get_PassesGroup_ptr();
   const PassesGroup* get_PassesGroup_ptr() const;
   int get_passnumber() const;

   int get_next_unused_ID() const;
   int get_largest_used_ID() const;
   int get_most_recently_added_ID() const;

   void set_most_recently_selected_ID(int ID);
   int get_most_recently_selected_ID() const;

// Instantaneous observation manipulation member functions:

   std::vector<int> find_nonmatching_IDs(
      double t,GraphicalsGroup* other_GraphicalsGroup_ptr);
   void consolidate_instantaneous_matching_observations(
      double t,GraphicalsGroup* other_GraphicalsGroup_ptr);

// Subgroup member functions:

   void set_OSGgroup_nodemask(int nodemask);
   int get_OSGgroup_nodemask() const;
   void toggle_OSGgroup_nodemask();
   bool is_child_of_OSGgroup(osg::Group* some_osgGroup_ptr);
   bool is_parent_of_Graphical(Graphical* Graphical_ptr);

// OSGsubPAT member functions:

   int OSGsubPAT_parent_of_Graphical(Graphical* Graphical_ptr);
   int n_Graphical_siblings(Graphical* Graphical_ptr);

   unsigned int get_n_OSGsubPATs() const;
   osg::PositionAttitudeTransform* get_OSGsubPAT_ptr(int n);
   int get_OSGsubPAT_number(osg::PositionAttitudeTransform* PAT_ptr);
   void set_OSGsubPAT_nodemask(unsigned int OSGsubPAT_number,int nodemask);
   int get_OSGsubPAT_nodemask(unsigned int OSGsubPAT_number);
   void toggle_OSGsubPAT_nodemask(int OSGsubPAT_number);

   void mask_nonselected_OSGsubPATs();
   void unmask_all_OSGsubPATs();
   void mask_Graphical_for_all_times(Graphical* curr_Graphical_ptr);

   void translate_OSGsubPAT(
      int OSGsubPAT_ID,const threevector& translation);
   void rotate_OSGsubPAT_about_specified_origin(
      int OSGsubPAT_ID,const threevector& rotation_origin,
      const threevector& new_xhat,const threevector& new_yhat);
   void rotate_OSGsubPAT_about_specified_origin_then_translate(
      int OSGsubPAT_ID,const threevector& rotation_origin,
      const threevector& new_xhat,const threevector& new_yhat,
      const threevector& trans);

// ActiveMQ message handling member functions:

   void pushback_Messenger_ptr(Messenger* M_ptr);
   Messenger* get_Messenger_ptr();
   const Messenger* get_Messenger_ptr() const;
   unsigned int get_n_Messenger_ptrs() const;
   Messenger* get_Messenger_ptr(unsigned int i);
   const Messenger* get_Messenger_ptr(unsigned int i) const;

   unsigned int get_n_messages_in_queue() const;
   bool retrieve_messages();

   void issue_selection_message();
   void generate_proximity_messages(double proximity_distance);

// Graphical creation member functions:

   void initialize_Graphical(
      Graphical* curr_Graphical_ptr,osg::Node* node_ptr=NULL);
   void initialize_Graphical(
      const threevector& UVW,Graphical* curr_Graphical_ptr,
      osg::Node* node_ptr=NULL);

// Scenegraph node insertion & removal member functions:

   bool insert_graphical_PAT_into_OSGsubPAT(
      Graphical* Graphical_ptr,unsigned int subPAT_number=0);
   bool insert_OSGgroup_into_OSGsubPAT(
      osg::Group* group_ptr,unsigned int subPAT_number=0);
   bool remove_OSGgroup_from_OSGsubPAT(osg::Group* group_ptr);

// Graphical manipulation methods:

   void change_size(double factor);
   void set_size(double size);
   void set_constant_scale(Graphical* curr_Graphical_ptr,double scale);
   Graphical* rescale(int d,double scale_factor);
   double get_selected_Graphical_z();
   Graphical* move_z(int sgn);
   Graphical* move_z(double delta_z);
   void copy(unsigned int orig_imagenumber,unsigned int start_copy_imagenumber,
             unsigned int stop_copy_imagenumber);

// Graphical erasing methods:

   bool erase_Graphical();
   bool erase_Graphical(int Graphical_ID);
   bool erase_Graphical(double t,int Graphical_ID);
   void erase_all_Graphicals();
   bool unerase_Graphical();
   bool unerase_Graphical(int Graphical_ID);
   void unerase_all_Graphicals();
   int destroy_Graphical();
   bool destroy_Graphical(int Graphical_ID);
   bool destroy_Graphical(Graphical* curr_Graphical_ptr);
   void destroy_all_Graphicals();
   void renumber_all_Graphicals();
   void renumber_Graphical(Graphical* curr_graphical_ptr,int new_ID);

// Ascii file I/O methods:

   std::string get_default_info_filename(std::string Graphicals_prefix) const;
   std::string get_output_filename(std::string Graphicals_prefix) const;
   std::string get_input_filename(std::string Graphicals_prefix) const;
   void read_OSG_file();
   void write_OSG_file();

// Global manipulation member functions:

   void initialize_const_posn(
      const threevector& posn,Graphical* curr_Graphical_ptr);
   void rotate_about_zaxis(
      double curr_t,int pass_number,const threevector& rotation_origin,
      double phi_z);

// Animation methods:

   unsigned int get_Nimages() const;
   unsigned int get_curr_framenumber() const;
   unsigned int get_first_framenumber() const;
   unsigned int get_last_framenumber() const;
   void update_display();

// Earth ellipsoid methods:

   bool convert_XYZ_to_LongLatAlt(
      const threevector& XYZ,double& longitude,double& latitude,
      double& altitude);
   bool convert_LongLatAlt_to_XYZ(
      double longitude, double latitude,double altitude,threevector& UVW);
   bool convert_ECI_to_LongLatAlt(
      const threevector& UVW,double& longitude, double& latitude,
      double& altitude);
   bool convert_LongLatAlt_to_ECI(
      double longitude, double latitude,double altitude,threevector& UVW);
   void compute_local_ellipsoid_directions(double longitude,double latitude);
   void rotate_zhat_to_rhat(Graphical* Graphical_ptr);

  protected:

   bool update_display_flag;
   unsigned int ndims;
   std::string GraphicalsGroup_name;
   std::vector<int> OSGsubPAT_number_given_Graphical;
   double delta_move_z;
   std::string ndims_label,extra_textmessage_info;
   Pass* pass_ptr;
   PassesGroup* PassesGroup_ptr;
   Clock* Clock_ptr;
   AnimationController* AnimationController_ptr;
   Ellipsoid_model* Ellipsoid_model_ptr;
   GraphicalsGroup* GraphicalsGroup_bug_ptr;

   void parse_latest_messages();
   virtual bool parse_next_message_in_queue(message& curr_message);
   void insert_Graphical_into_list(Graphical* curr_Graphical_ptr);

  private:

   bool erase_Graphicals_forward_in_time_flag;
   bool erase_Graphicals_except_at_curr_time_flag;
   int graphical_counter;
   int selected_OSGsubPAT_ID;
   int most_recently_added_ID,most_recently_selected_ID;
   std::deque<int> selected_Graphical_IDs;
   
   threevector* grid_world_origin_ptr;
   osg::ref_ptr<osg::MatrixTransform> MatrixTransform_refptr;
   osg::ref_ptr<osg::Group> OSGgroup_refptr;
   std::vector<osg::ref_ptr<osg::PositionAttitudeTransform> > OSGsubPATs;

   typedef std::map<int,Graphical*> GRAPHICAL_PTRS_MAP; 
	// indep key = Graphical ID
   GRAPHICAL_PTRS_MAP* graphical_ID_ptrs_map_ptr;
   std::vector<Graphical*> graphical_counter_ptrs_vector; 
	// indep var = Graphical counter index

   typedef std::map<int,int> GRAPHICAL_INDEX_ID_MAP;
   GRAPHICAL_INDEX_ID_MAP* Graphical_index_ID_map_ptr;

   typedef std::map<int,int> GRAPHICAL_ID_INDEX_MAP;
   GRAPHICAL_ID_INDEX_MAP* Graphical_ID_index_map_ptr;

   std::vector<Messenger*> Messenger_ptrs;

// FIFO queue in which every message read from ActiveMQ is stored:

   std::deque<message> message_queue; 

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const GraphicalsGroup& g);

   bool retrieve_messages(int i);
   void generate_new_OSGsubPAT();
   bool remove_graphical_PAT_from_OSGsubPAT(Graphical* curr_Graphical_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void GraphicalsGroup::set_erase_Graphicals_forward_in_time_flag(
   bool flag)
{
   erase_Graphicals_forward_in_time_flag = flag;
}

inline void GraphicalsGroup::set_erase_Graphicals_except_at_curr_time_flag(
   bool flag)
{
   erase_Graphicals_except_at_curr_time_flag = flag;
}

inline std::string GraphicalsGroup::get_name() const
{
   return GraphicalsGroup_name;
}

inline void GraphicalsGroup::set_extra_textmessage_info(std::string info)
{
   extra_textmessage_info=info;
}

// nodemask = 0 --> OSGGroup is hidden
// nodemask = 1 --> OSGGroup is visible

inline void GraphicalsGroup::set_OSGgroup_nodemask(int nodemask)
{
   OSGgroup_refptr->setNodeMask(nodemask);
}		       

// Member function get_OSGgroup_nodemask() returns integer 0 or 1 (and
// not -1).

inline int GraphicalsGroup::get_OSGgroup_nodemask() const
{
   int nodemask=static_cast<int>(OSGgroup_refptr->getNodeMask());
   nodemask=basic_math::max(0,nodemask);
   return nodemask;
}		       

inline void GraphicalsGroup::toggle_OSGgroup_nodemask()
{
   int nodemask=get_OSGgroup_nodemask();
   set_OSGgroup_nodemask(modulo(nodemask+1,2));
}		       

inline void GraphicalsGroup::set_AnimationController_ptr(
   AnimationController* AC_ptr)
{
   AnimationController_ptr=AC_ptr;
}

inline AnimationController* GraphicalsGroup::get_AnimationController_ptr() 
   const
{
   return AnimationController_ptr;
}

inline void GraphicalsGroup::set_Clock_ptr(Clock* clock_ptr)
{
   Clock_ptr=clock_ptr;
}

inline void GraphicalsGroup::set_Ellipsoid_model_ptr(
   Ellipsoid_model* emodel_ptr)
{
   Ellipsoid_model_ptr=emodel_ptr;
}

inline Ellipsoid_model* GraphicalsGroup::get_Ellipsoid_model_ptr()
{
   return Ellipsoid_model_ptr;
}

inline const Ellipsoid_model* GraphicalsGroup::get_Ellipsoid_model_ptr() const
{
   return Ellipsoid_model_ptr;
}

inline void GraphicalsGroup::set_GraphicalsGroup_bug_ptr(
   GraphicalsGroup* GG_ptr)
{
   GraphicalsGroup_bug_ptr=GG_ptr;
}

inline void GraphicalsGroup::set_update_display_flag(bool flag)
{
   update_display_flag=flag;
}

inline void GraphicalsGroup::set_delta_move_z(double dz)
{
   delta_move_z=dz;
}

inline void GraphicalsGroup::set_ndims(unsigned int ndims)
{
   this->ndims=ndims;
}

inline unsigned int GraphicalsGroup::get_ndims() const
{
   return ndims;
}

inline Graphical* GraphicalsGroup::get_selected_Graphical_ptr() const
{
   return get_ID_labeled_Graphical_ptr(get_selected_Graphical_ID());
}

inline int GraphicalsGroup::get_selected_OSGsubPAT_ID()
{
   return selected_OSGsubPAT_ID;
}

inline osg::Group* GraphicalsGroup::get_OSGgroup_ptr()
{
   return OSGgroup_refptr.get();
}

inline int GraphicalsGroup::get_OSGgroup_refptr_count() const
{
   return OSGgroup_refptr->referenceCount();
}

inline unsigned int GraphicalsGroup::get_n_OSGsubPATs() const
{
   return OSGsubPATs.size();
}

inline osg::PositionAttitudeTransform* GraphicalsGroup::get_OSGsubPAT_ptr(
   int n)
{
   if (n >= 0 && n < int(OSGsubPATs.size()))
   {
      return OSGsubPATs[n].get();
   }
   else
   {
      return NULL;
   }
}

inline int GraphicalsGroup::get_OSGsubPAT_number(
   osg::PositionAttitudeTransform* PAT_ptr)
{
   for (unsigned int i=0; i<get_n_OSGsubPATs(); i++)
   {
      if (PAT_ptr==get_OSGsubPAT_ptr(i)) return i;
   }
   return -1;
}

inline void GraphicalsGroup::set_OSGsubPAT_nodemask(
   unsigned int OSGsubPAT_number,int nodemask)
{
   if (OSGsubPAT_number < get_n_OSGsubPATs())
   {
      OSGsubPATs[OSGsubPAT_number]->setNodeMask(nodemask);
   }
}

inline int GraphicalsGroup::get_OSGsubPAT_nodemask(
   unsigned int OSGsubPAT_number)
{
   return OSGsubPATs[OSGsubPAT_number]->getNodeMask();
}		       

inline void GraphicalsGroup::toggle_OSGsubPAT_nodemask(int OSGsubPAT_number)
{
   int nodemask=get_OSGsubPAT_nodemask(OSGsubPAT_number);
   set_OSGsubPAT_nodemask(OSGsubPAT_number,(modulo(nodemask+1,2)));
}		       

inline unsigned int GraphicalsGroup::get_n_Graphicals() const
{
//   std::cout << "inside GraphicalsGroup::get_n_Graphicals()" << std::endl;
//   std::cout << "this = " << this << std::endl;
//   std::cout << "graphical_ID_ptrs_map_ptr = "
//             << graphical_ID_ptrs_map_ptr << std::endl;
   return graphical_ID_ptrs_map_ptr->size();
}

// --------------------------------------------------------------------------
// Integer member graphical_counter monotonically increases every time
// a new Graphical is instantiated.  It does NOT decrease whenever a
// Graphical is destroyed.  

inline void GraphicalsGroup::set_Graphical_counter(int g)
{
   graphical_counter=g;
}

inline int GraphicalsGroup::get_Graphical_counter() const
{
//   std::cout << "inside GraphicalsGroup::get_Graphical_counter()" << std::endl;
   return graphical_counter;
}

// --------------------------------------------------------------------------
// Member function get_Graphical_ptr() returns NULL if input index n
// is greater than get_n_Graphicals().  Otherwise, it returns a
// pointer to the Graphical labeled by n.

inline Graphical* GraphicalsGroup::get_Graphical_ptr(unsigned int n) const
{
   if (n >= get_n_Graphicals())
   {
      return NULL;
   }
   else 
   {
      return graphical_counter_ptrs_vector[n];
   }
}

inline Graphical* GraphicalsGroup::get_last_Graphical_ptr() const
{
   return get_Graphical_ptr(get_n_Graphicals()-1);
}

inline Graphical* GraphicalsGroup::get_most_recently_added_Graphical_ptr() 
   const
{
   return get_Graphical_ptr(get_most_recently_added_ID());
}

// --------------------------------------------------------------------------
// As of 6/5/05, we simply set the time associated with each image in
// equal to its imagenumber.  This will eventually need to be
// generalized so that the time field corresponds to a true temporal
// measurement...

inline double GraphicalsGroup::get_initial_t() const
{
   return static_cast<double>(get_first_framenumber());
}

inline double GraphicalsGroup::get_final_t() const
{
   return static_cast<double>(get_last_framenumber());
}

inline double GraphicalsGroup::get_curr_t() const
{
   return static_cast<double>(get_curr_framenumber());
}

inline Pass* GraphicalsGroup::get_pass_ptr() 
{
   return pass_ptr;
}

inline const Pass* GraphicalsGroup::get_pass_ptr() const
{
   return pass_ptr;
}

inline void GraphicalsGroup::set_PassesGroup_ptr(PassesGroup* PG_ptr)
{
   PassesGroup_ptr=PG_ptr;
}

inline PassesGroup* GraphicalsGroup::get_PassesGroup_ptr()
{
  return PassesGroup_ptr;
}

inline const PassesGroup* GraphicalsGroup::get_PassesGroup_ptr() const
{
  return PassesGroup_ptr;
}

inline int GraphicalsGroup::get_passnumber() const
{
   if (pass_ptr != NULL)
   {
      return pass_ptr->get_ID();
   }
   else
   {
      return -1;
//      std::cout << "Error in GraphicalsGroup::get_passnumber()" << std::endl;
//      std::cout << "pass_ptr = NULL!" << std::endl;
//      exit(-1);
   }
}

inline int GraphicalsGroup::get_most_recently_added_ID() const
{
   return most_recently_added_ID;
}

inline void GraphicalsGroup::set_most_recently_selected_ID(int ID)
{
   most_recently_selected_ID=ID;
}

inline int GraphicalsGroup::get_most_recently_selected_ID() const
{
   return most_recently_selected_ID;
}


#endif // GraphicalsGroup.h



