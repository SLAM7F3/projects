// ==========================================================================
// SIGNPOSTSGROUP class member function definitions
// ==========================================================================
// Last modified on 9/13/12; 3/26/13; 1/4/14; 4/5/14
// ==========================================================================

#include <iomanip>
#include <map>
#include <osg/Geode>
#include <osgText/Text>

#include "astro_geo/astrofuncs.h"
#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "astro_geo/Ellipsoid_model.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "general/inputfuncs.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "messenger/Messenger.h"
#include "general/outputfuncs.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osgGIS/postgis_database.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

#include "math/rotation.h"
#include "templates/mytemplates.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::setw;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void SignPostsGroup::allocate_member_objects()
{

   signposts_map_ptr=new SIGNPOSTS_MAP;
}		       

void SignPostsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="SignPostsGroup";

   broadcast_NFOV_aimpoint_flag=false;
   aerial_video_frame_flag=false;
   altitude_dependent_size_flag=true;
   raytrace_visible_terrain_flag=false;
   selected_colorfunc_color=colorfunc::red;
   permanent_colorfunc_color=colorfunc::white;

   ColorGeodeVisitor_ptr=NULL;
   PointCloudsGroup_ptr=NULL;
   SKS_worldmodel_database_ptr=NULL;
   MoviesGroup_ptr=NULL;
   photogroup_ptr=NULL;
   SignPostsGroup_3D_ptr=NULL;
   imageplane_SignPostsGroup_ptr=NULL;
   imageplane_SignPostPickHandler_ptr=NULL;

   if (get_ndims()==2) 
   {
      set_common_geometrical_size(0.004);
   }
   else if (get_ndims()==3)
   {
      set_common_geometrical_size(1);
   }

// Note: This next line lines should eventually move to some more
// basic location than SPG (though probably not into GraphicalsGroup
// but rather into a ***single global*** location)

   prev_relative_time_database_accessed=0;

   prev_SignPost_framenumber=prev_photo_ID=-1;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<SignPostsGroup>(
         this, &SignPostsGroup::update_display));
}		       

SignPostsGroup::SignPostsGroup(
   const int p_ndims,Pass* PI_ptr,threevector* GO_ptr):
   GeometricalsGroup(p_ndims,PI_ptr,GO_ptr), AnnotatorsGroup(p_ndims,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

SignPostsGroup::SignPostsGroup(
   Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,clock_ptr,EM_ptr,GO_ptr),
   AnnotatorsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

SignPostsGroup::SignPostsGroup(
   Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,threevector* GO_ptr,
   postgis_database* SKS_db_ptr,PointCloudsGroup* PCG_ptr):
   GeometricalsGroup(3,PI_ptr,clock_ptr,EM_ptr,GO_ptr),
   AnnotatorsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   SKS_worldmodel_database_ptr=SKS_db_ptr;
   PointCloudsGroup_ptr=PCG_ptr;
}		       

SignPostsGroup::SignPostsGroup(
   Pass* PI_ptr,threevector* GO_ptr,postgis_database* SKS_db_ptr,
   PointCloudsGroup* PCG_ptr):
   GeometricalsGroup(3,PI_ptr,GO_ptr),AnnotatorsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   SKS_worldmodel_database_ptr=SKS_db_ptr;
   PointCloudsGroup_ptr=PCG_ptr;
}		       

SignPostsGroup::~SignPostsGroup()
{
   delete signposts_map_ptr;
   delete imageplane_SignPostsGroup_ptr;
   delete imageplane_SignPostPickHandler_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const SignPostsGroup& f)
{
   int node_counter=0;
   cout << "# SignPosts = " << f.get_n_Graphicals() << endl;
   for (unsigned int n=0; n<f.get_n_Graphicals(); n++)
   {
      SignPost* SignPost_ptr=f.get_SignPost_ptr(n);
      outstream << "SignPost node # " << node_counter++ << endl;
      outstream << "SignPost = " << *SignPost_ptr << endl;
   }
   return outstream;
}

// ==========================================================================
// Set & get methods
// ==========================================================================

// Member function set_fixed_label_to_SignPost_ID allows the user to
// specify permanent labels for certain signposts.  ID-fixed label
// information is saved within member STL vector ID_fixed_label_pairs.

void SignPostsGroup::set_fixed_label_to_SignPost_ID(int ID,string fixed_label)
{   
   pair<int,string> p(ID,fixed_label);
   ID_fixed_label_pairs.push_back(p);
}

// --------------------------------------------------------------------------
void SignPostsGroup::set_size(double size)
{
   set_size(size,size);
}

void SignPostsGroup::set_size(double size,double text_size)
{
//   cout << "inside SignPostsGroup::set_size(), size = " << size
//        << " text_size = " << text_size << endl;
   GeometricalsGroup::set_size(size,text_size);
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      SignPost* SignPost_ptr=get_SignPost_ptr(n);
      SignPost_ptr->set_max_text_width(SignPost_ptr->get_label());
   }
}

// --------------------------------------------------------------------------
void SignPostsGroup::set_max_text_width(double width)
{
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      SignPost* SignPost_ptr=get_SignPost_ptr(n);
      SignPost_ptr->get_text_ptr(0)->setMaximumWidth(width);
   }
}

// --------------------------------------------------------------------------
void SignPostsGroup::set_postgis_databases_group_ptr(
   postgis_databases_group* pdg_ptr)
{
   postgis_databases_group_ptr=pdg_ptr;
}

// ==========================================================================
// SignPost creation member functions
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_SignPost from all other graphical insertion
// and manipulation methods...

SignPost* SignPostsGroup::generate_new_SignPost(int ID)
{
//   cout << "inside SignPostsGroup::generate_new_SignPost()" << endl;

   return generate_new_SignPost(1,1,Zero_vector,ID);
}

SignPost* SignPostsGroup::generate_new_SignPost(
   double size,double height_multiplier,int ID,int OSGsubPAT_number)
{
   return generate_new_SignPost(size,height_multiplier,Zero_vector,ID,
                                OSGsubPAT_number);
}

SignPost* SignPostsGroup::generate_new_SignPost(
   double size,double height_multiplier,const threevector& UVW,int ID,
   int OSGsubPAT_number)
{
//   cout << "inside SignPostsGroup::generate_new_SignPost()" << endl;
   
   if (ID==-1) ID=get_next_unused_ID();
   SignPost* curr_SignPost_ptr=new SignPost(ID,get_ndims());
//   cout << "curr_SignPost_ptr = " << curr_SignPost_ptr << endl;

   initialize_new_SignPost(size,height_multiplier,UVW,
                           curr_SignPost_ptr,OSGsubPAT_number);

   return curr_SignPost_ptr;
}

// ---------------------------------------------------------------------
void SignPostsGroup::initialize_new_SignPost(
   double size,double height_multiplier,const threevector& UVW,
   SignPost* curr_SignPost_ptr,int OSGsubPAT_number)
{
//   cout << "inside SignPostsGroup::initialize_new_SignPost" << endl;

   GraphicalsGroup::insert_Graphical_into_list(curr_SignPost_ptr);
   initialize_Graphical(UVW,curr_SignPost_ptr);

   osg::Geode* geode_ptr=curr_SignPost_ptr->generate_drawable_geode(
      size,height_multiplier);
   curr_SignPost_ptr->get_PAT_ptr()->addChild(geode_ptr);

   if (get_ndims()==2) curr_SignPost_ptr->set_quasirandom_color();

// Orient signpost so that it points radially inward wrt earth's
// center if Earth's ellipsoid_model_ptr != NULL:

   rotate_zhat_to_rhat(curr_SignPost_ptr);

   insert_graphical_PAT_into_OSGsubPAT(curr_SignPost_ptr,OSGsubPAT_number);
   reset_colors();
}

// --------------------------------------------------------------------------
// Member function generate_new_SignPost_on_earth instantiates a
// signpost at the specified longitude, latitude and altitude on the
// earth's ellipsoid.  Its size is appropriate for marking an entire
// city-sized region.  

SignPost* SignPostsGroup::generate_new_SignPost_on_earth(
   double longitude,double latitude,double altitude,int ID)
{
   int specified_UTM_zonenumber=-1;
   return generate_new_SignPost_on_earth(
      longitude,latitude,altitude,specified_UTM_zonenumber,ID);
}

SignPost* SignPostsGroup::generate_new_SignPost_on_earth(
   double longitude,double latitude,double altitude,
   int specified_UTM_zonenumber,int ID)
{
//   cout << "inside SignPostsGroup::generate_new_SignPost_on_earth()" << endl;

   SignPost* curr_SignPost_ptr=generate_new_SignPost(
      common_geometrical_size , 1.0 , ID);
//   cout << "common_geom_size = " << common_geometrical_size << endl;
//   cout << "ID = " << ID << endl;

//   cout << "Ellipsoid_model_ptr = " << Ellipsoid_model_ptr << endl;
   if (Ellipsoid_model_ptr != NULL)
   {
      threevector UVW1=Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
         longitude,latitude,altitude);
      threevector UVW2=1.1*UVW1.magnitude()*UVW1.unitvector();
      curr_SignPost_ptr->set_attitude_posn(
         get_curr_t(),get_passnumber(),UVW1,UVW2);
   }
   else
   {
      geopoint curr_geopoint;
      if (specified_UTM_zonenumber >= 0)
      {
         curr_geopoint=geopoint(longitude,latitude,altitude,
                                specified_UTM_zonenumber);
      }
      else
      {
         curr_geopoint=geopoint(longitude,latitude,altitude);
      }
//      cout << "curr_geopoint = " << curr_geopoint << endl;
      
      threevector UVW1(curr_geopoint.get_UTM_easting(),
                       curr_geopoint.get_UTM_northing(),altitude);
      threevector UVW2(curr_geopoint.get_UTM_easting(),
                       curr_geopoint.get_UTM_northing(),
                       altitude+common_geometrical_size);
//      cout << "UVW1 = " << UVW1 << " UVW2 = " << UVW2 << endl;
      curr_SignPost_ptr->set_attitude_posn(
         get_curr_t(),get_passnumber(),UVW1,UVW2);
   }
   return curr_SignPost_ptr;
}

// ==========================================================================
// SignPost manipulation member functions
// ==========================================================================

// Member function edit_SignPost_label allows the user to change the ID
// number associated with a SignPost.  The new ID number must not
// conflict with any other existing SignPost's ID.  It must also be
// non-negative.  The user enters the replacement ID for a selected
// SignPost within the main console window.  (As of 7/10/05, we are
// unfortunately unable to robustly retrieve user input from the
// SignPost text dialog window...)

void SignPostsGroup::edit_SignPost_label()
{   
   int ID=get_selected_Graphical_ID();
   SignPost* curr_SignPost_ptr=get_ID_labeled_SignPost_ptr(ID);
   
   if (curr_SignPost_ptr != NULL)
   {
      if (assign_fixed_label(ID))
      {
      }
      else if (get_selected_Graphical_ID() != -1)
      {
         cout << endl;
         string label_command="Enter new text label for SignPost:";
         string label=inputfunc::enter_string(label_command);
         curr_SignPost_ptr->set_label(label);

//         update_SKS_worldmodel_database_nomination_label(curr_SignPost_ptr);

      } // selected_Graphical_ID != -1 conditional
   } // currnode_SignPost != NULL conditional
}

// --------------------------------------------------------------------------
// Member function assign_fixed_label takes in the ID for some
// SignPost.  It checks whether a fixed label corresponding to that
// signpost ID was predefined within member STL vector
// ID_fixed_label_pairs.  If so, the signpost's label is set and this
// boolean method returns true.  We cooked up this little utility on
// 10/30/06 for the DTED shadowing problem where we want the zeroth
// signpost to always be labeled as "Receiver".

bool SignPostsGroup::assign_fixed_label(int curr_ID)
{   
   bool fixed_label_assigned_flag=false;
   for (unsigned int i=0; i<ID_fixed_label_pairs.size(); i++)
   {
      pair<int,string> p=ID_fixed_label_pairs[i];
      if (p.first==curr_ID)
      {
         SignPost* SignPost_ptr=get_ID_labeled_SignPost_ptr(curr_ID);
         SignPost_ptr->set_label(p.second);
         fixed_label_assigned_flag=true;
      }
   } // loop over pairs within ID_fixed_label_pairs STL vector member
   return fixed_label_assigned_flag;
}

/*
// --------------------------------------------------------------------------
// Member function erase_SignPost sets boolean entries within the
// member map coords_erased to true for the current SignPost.  When
// SignPost crosshairs are drawn within
// SignPostsGroup::reassign_PAT_ptrs(), entries within this STL map are
// first checked and their positions are set to large negative values
// to prevent them from appearing within the OSG data window.  Yet the
// SignPost itself continues to exist.

bool SignPostsGroup::erase_SignPost()
{   
   bool SignPost_erased=false;

   SignPost* curr_SignPost_ptr=get_ID_labeled_SignPost_ptr(
      get_selected_Graphical_ID());
   if (curr_SignPost_ptr != NULL)
   {

// Recall that a SignPost exists for all times.  Yet it generally
// appears in images spanning only a finite time interval.  We
// therefore erase SignPost's (U,V,W) coords for all images greater
// than or equal to the current time:
      
      for (unsigned int n=get_curr_framenumber(); n<=get_last_framenumber(); n++)
      {

// As of 6/5/05, we simply set the time associated with each image in
// pass #0 equal to its imagenumber.  This will eventually need to be
// generalized so that the time field corresponds to a true temporal
// measurement...

         double curr_t=static_cast<double>(n);
         curr_SignPost_ptr->set_mask(curr_t,get_passnumber(),true);
      }

      cout << "Erased SignPost " << get_selected_Graphical_ID() << endl;
      SignPost_erased=true;
   } // currnode_ptr != NULL conditional

   return SignPost_erased;
}

// --------------------------------------------------------------------------
// Member function unerase_SignPost queries the user to enter the ID
// for some erased SignPost.  It then unerases that SignPost within the
// current image.

bool SignPostsGroup::unerase_SignPost()
{   
   bool SignPost_unerased_flag=false;

   string label_command="Enter SignPost number to unerase in current image:";
   int unerased_SignPost_ID=inputfunc::enter_nonnegative_integer(
      label_command);

   SignPost* curr_SignPost_ptr=get_ID_labeled_SignPost_ptr(
      unerased_SignPost_ID);
   if (curr_SignPost_ptr==NULL)
   {
      cout << "Input label does not correspond to any existing SignPost"
           << endl;
   }
   else
   {
      if (!curr_SignPost_ptr->get_mask(get_curr_t(),get_passnumber()))
      {
         cout << "SignPost already exists in current image" << endl;
      }
      else
      {
         curr_SignPost_ptr->set_mask(
            get_curr_t(),get_passnumber(),false);
         set_selected_Graphical_ID(unerased_SignPost_ID);
         cout << "Unerased SignPost " << unerased_SignPost_ID << endl;
         SignPost_unerased_flag=true;
      }
   } // currnode_ptr==NULL conditional

   reset_colors();
   return SignPost_unerased_flag;
}
*/

// --------------------------------------------------------------------------
// Member function destroy_SignPost deletes the selected SignPost and
// purges its entry from the Postgres "entity" table if the database
// is active.

int SignPostsGroup::destroy_SignPost()
{   
//   cout << "inside SignPostsGroup::destroy_SignPost()" << endl;
   int SignPost_ID=get_selected_Graphical_ID();
   if (destroy_SignPost(SignPost_ID))
   {
      set_selected_Graphical_ID(-1);
      return SignPost_ID;
   }
   else
   {
      return -1;
   }
}

bool SignPostsGroup::destroy_SignPost(int SignPost_ID)
{   
//   cout << "inside SignPostsGroup::destroy_SignPost(ID)" << endl;
//   cout << "SignPost_ID = " << SignPost_ID << endl;

   bool destroyed_SignPost_flag=
      GraphicalsGroup::destroy_Graphical(SignPost_ID);

   if (SKS_worldmodel_database_ptr != NULL &&
       SKS_worldmodel_database_ptr->get_connection_status_flag())
   {
      vector<string> commands;
      string delete_command = "DELETE from entity where id="
         +stringfunc::number_to_string(SignPost_ID);
      commands.push_back(delete_command);

      SKS_worldmodel_database_ptr->set_SQL_commands(commands);
      SKS_worldmodel_database_ptr->execute_SQL_commands();
   } // SKS_worldmodel_database_ptr != NULL conditional

   return destroyed_SignPost_flag;
}

void SignPostsGroup::destroy_all_SignPosts()
{
//   cout << "inside SignPostsGroup::destroy_all_SignPosts()" << endl;
   unsigned int n_SignPosts=get_n_Graphicals();
//   cout << "n_SignPosts = " << n_SignPosts << endl;

   vector<int> SignPost_IDs_to_destroy;
   for (unsigned int p=0; p<n_SignPosts; p++)
   {
      SignPost* SignPost_ptr=get_SignPost_ptr(p);
//      cout << "p = " << p << " SignPost_ptr = " << SignPost_ptr << endl;
      SignPost_IDs_to_destroy.push_back(SignPost_ptr->get_ID());
   }

   for (unsigned int p=0; p<n_SignPosts; p++)
   {
      destroy_SignPost(SignPost_IDs_to_destroy[p]);
   }
}

// --------------------------------------------------------------------------
// Member function move_z vertically translates the selected SignPost
// and updates its entry within the Postgres database if the database
// is active.

SignPost* SignPostsGroup::move_z(int sgn)
{   
//   cout << "inside SignPostsGroup::move_z" << endl;
   
   SignPost* curr_SignPost_ptr=dynamic_cast<SignPost*>(
      GraphicalsGroup::move_z(sgn));
   if (curr_SignPost_ptr != NULL)
   {
      threevector SignPost_posn;
      curr_SignPost_ptr->get_UVW_coords(
         get_curr_t(),get_passnumber(),SignPost_posn);
      curr_SignPost_ptr->get_geopoint_ptr()->set_altitude(
         SignPost_posn.get(2));

//      update_SKS_worldmodel_database_entry(curr_SignPost_ptr);
   }
   
   return curr_SignPost_ptr;
}

// --------------------------------------------------------------------------
// Member function set_altitude_dependent_size() retrieves the current
// altitude of the virtual camera.  If CM_3D_ptr != NULL, it resets
// the size for the SignPosts and their text labels within *this as a
// linear function of the altitude.  We wrote this method in Jun 2009
// for automatically resizing ground target signposts within the
// line-of-sight project.

void SignPostsGroup::set_altitude_dependent_size()
{
//   cout << "ladar height data flag = " << get_ladar_height_data_flag() << endl;

   if (get_n_Graphicals()==0) return;

//   cout << "inside SignPostsGroup::set_altitude_dependent_size()"  << endl;

   double zmin,zintermediate,zmax;
   double size_min,size_intermediate,size_max;

   if (get_ladar_height_data_flag())
   {
//      zmin=0.005*1000;		// ALIRT ladar data 
//      zintermediate=10*1000;		// ALIRT ladar data
//      zmax=1000*1000;
//      size_min=0.000015;	// ALIRT ladar data
//      size_intermediate=0.10;	// ALIRT ladar data

// Following parameters are reasonable for TOC11 red actor path problem:

      zmin=0.03*1000;			// ALIRT ladar data 
      zintermediate=5*1000;		// ALIRT ladar data
      zmax=100*1000;

      size_min=0.001;		// ALIRT ladar data
      size_intermediate=1;	// ALIRT ladar data
      size_max=20;
   }
   else
   {
      zmin=0.5*1000;	// Orig LOST value
      zintermediate=50*1000;	// Orig LOST value
      zmax=1000*1000;
      size_min=0.15;	// Orig LOST value
      size_intermediate=1.5;	// Orig LOST value
      size_max=20;
   }

   double prefactor=3;
   double size=prefactor*compute_altitude_dependent_size(
      zmin,zintermediate,zmax,size_min,size_intermediate,size_max);
   if (size > 0)
   {
      double text_size=80*size;
      if (get_ladar_height_data_flag())
      {
         text_size=8*size;
      }

      set_size(size,text_size);
//      cout << "text_size = " << text_size << endl;
   }
//   cout << "size = " << size << endl;
}

// --------------------------------------------------------------------------
// Member function update_display() periodically queries the Postgres
// database where SignPost world information is stored.  If such a
// database exists, it wipes out all current signposts and
// instantiates a new set.  This method also performs a conventional
// display update on existing signposts.  This member function should
// act as a callback within a main program.

void SignPostsGroup::update_display()
{   
//   cout << "inside SignPostsGroup::update_display()" << endl;

   if (!update_display_flag) return;

//   const double max_database_access_interval=1;		// secs
//   const double max_database_access_interval=4;		// secs
   const double max_database_access_interval=10;	// secs
   double curr_relative_time=timefunc::elapsed_timeofday_time();
   if (curr_relative_time-prev_relative_time_database_accessed > 
       max_database_access_interval)
   {
      prev_relative_time_database_accessed=curr_relative_time;
//      retrieve_signposts_from_SKS_worldmodel_database();
   }

   if (photogroup_ptr==NULL)
   {
      project_SignPosts_into_video_plane();
   }
   else
   {
      if (aerial_video_frame_flag)
      {
         backproject_selected_photo_SignPosts_into_3D();
         project_SignPosts_into_selected_aerial_video_frame();
      }
      else
      {
         project_SignPosts_into_selected_photo();
      }
   }

   if (altitude_dependent_size_flag) set_altitude_dependent_size();

   if (raytrace_visible_terrain_flag) raytrace_visible_terrain();

// Call reset_colors to enable SignPost blinking:

   reset_colors();

   GraphicalsGroup::update_display();
}

// ==========================================================================
// Ascii feature file I/O methods
// ==========================================================================

// Member function save_info_to_file loops over all SignPosts within
// *get_SignPostlist_ptr() and prints their times, IDs, pass numbers
// and UVW coordinates to the output ofstream.  This SignPost
// information can later be read back in via member function
// read_SignPost_info_from_file.

string SignPostsGroup::save_info_to_file()
{
   cout << "inside SignPostsGroup::save_info_to_file()" << endl;
   string output_filename="signposts_"+stringfunc::number_to_string(
      get_ndims())+"D.txt";

   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   outstream << "# Time   SignPost_ID   Passnumber   X  Y  Z  Label"
             << endl << endl;

   for (unsigned int imagenumber=get_first_framenumber(); 
        imagenumber <= get_last_framenumber(); imagenumber++)
   {
      double curr_t=static_cast<double>(imagenumber);
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         SignPost* SignPost_ptr=get_SignPost_ptr(n);
         instantaneous_obs* curr_obs_ptr=SignPost_ptr->
            get_particular_time_obs(curr_t,get_passnumber());
         if (curr_obs_ptr != NULL)
         {
            bool erased_point_flag=false;
            threevector p;
            vector<double> column_data;
            if (SignPost_ptr->get_UVW_coords(curr_t,get_passnumber(),p))
            {
               if (SignPost_ptr->get_mask(curr_t,get_passnumber()))
               {
                  erased_point_flag=true;
               }
               else
               {
                  for (unsigned int j=0; j<get_ndims(); j++)
                  {
                     column_data.push_back(p.get(j));
                  }
               }
            }
            if (!erased_point_flag) 
            {

               outstream.setf(ios::showpoint);
               outstream.precision(4);
               outstream << setw(6) << curr_t 
                         << setw(3) << SignPost_ptr->get_ID() 
                         << setw(3) << get_passnumber();
               for (unsigned int j=0; j<get_ndims(); j++)
               {
                  const int column_width=15;
                  outstream.precision(10);
                  outstream << setw(column_width) << column_data[j];
               }
               outstream << setw(18) << SignPost_ptr->get_label();
               outstream << endl;
               
            } // !erased_point_flag conditional
         } // curr_obs_ptr != NULL conditional
      } // loop over nodes in *get_SignPostlist_ptr()
      outstream << endl;
   } // loop over imagenumber index

   filefunc::closefile(output_filename,outstream);

   string banner="Saved signposts to "+output_filename;
   outputfunc::write_banner(banner);

   return output_filename;
}

// --------------------------------------------------------------------------
// Member function read_info_from_file() parses the ascii text file
// generated by member function save_info_to_file().  After purging
// the SignPostslist, this method regenerates the SignPosts within the
// list based upon the ascii text file information.

bool SignPostsGroup::read_info_from_file()
{
//   cout << "inside SignPostsGroup::read_info_from_file() #1" << endl;
   
   string input_filename="signposts_"+stringfunc::number_to_string(
      get_ndims())+"D.txt";
   return read_info_from_file(input_filename);
}

bool SignPostsGroup::read_info_from_file(string input_filename)
{
//   cout << "inside SignPostsGroup::read_info_from_file() #2" << endl;
//   cout << "input_filename = " << input_filename << endl;

   if (!filefunc::ReadInfile(input_filename))
   {
      cout << "Trouble in SignPostsGroup::read_info_from_file()"
           << endl;
      cout << "Couldn't open filename = " << input_filename << endl;
      return false;
   }

   vector<double> curr_time;
   vector<int> SignPost_ID,pass_number;
   vector<threevector> UVW;
   vector<string> label;
   
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> column_values=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[i]);

      curr_time.push_back(stringfunc::string_to_number(column_values[0]));
      SignPost_ID.push_back(stringfunc::string_to_number(column_values[1]));
      pass_number.push_back(stringfunc::string_to_number(column_values[2]));
      double U=stringfunc::string_to_number(column_values[3]);
      double V=stringfunc::string_to_number(column_values[4]);
      double W=0;

      if (get_ndims()==3)
      {
         W=stringfunc::string_to_number(column_values[5]);
      }
      UVW.push_back(threevector(U,V,W));

      string curr_label="";
      for (unsigned int j=6; j<column_values.size(); j++)
      {
         curr_label += column_values[j]+" ";
      }
      curr_label=stringfunc::remove_leading_whitespace(curr_label);
      curr_label=stringfunc::remove_trailing_whitespace(curr_label);
      label.push_back(curr_label);
   } // loop over index i labeling ascii file line number

   cout.precision(10);
   for (unsigned int i=0; i<curr_time.size(); i++)
   {
      cout << "time = " << curr_time[i]
           << " ID = " << SignPost_ID[i]
           << " pass = " << pass_number[i]
           << " UVW = " << UVW[i] << endl;
      cout << "label = " << label[i] << endl;
   }

// On 12/7/11, we ran into precision problems when attempting to write
// out SignPosts for the Pender ALIRT data set to an OSG file (for
// viewing within Eyeglass).  Ross suggested that we capture a large, common 
// UTM offset a MatrixTransform.  So we first compute the
// center-of-mass for all input SignPosts.  We next truncate this COM
// to the closest thousands so that it can be correctly written out to
// an OSG file with limited numerical precision.  We then pass the
// truncated COM to GraphicalsGroup::translate_OSGsubPAT().  We then
// subtract the common offset from the UVW positions when the
// individual SignPosts are instantiated below:

   threevector avg_UVW(Zero_vector);
   for (unsigned int i=0; i<UVW.size(); i++)
   {
      avg_UVW += UVW[i];
   }
   avg_UVW /= UVW.size();

   double avg_U=1000*basic_math::mytruncate(0.001*avg_UVW.get(0));
   double avg_V=1000*basic_math::mytruncate(0.001*avg_UVW.get(1));
   double avg_W=1000*basic_math::mytruncate(0.001*avg_UVW.get(2));
   avg_UVW=threevector(avg_U,avg_V,avg_W);

//   cout.precision(10);
//   cout << "Average SignPost UVW = " << avg_UVW << endl;
//   outputfunc::enter_continue_char();

   translate_OSGsubPAT(0,avg_UVW);

// Destroy all existing SignPosts before creating a new SignPost list
// from the input ascii file:

   destroy_all_Graphicals();

   for (unsigned int i=0; i<SignPost_ID.size(); i++)
   {
      int curr_ID=SignPost_ID[i];
      SignPost* curr_SignPost_ptr=get_ID_labeled_SignPost_ptr(curr_ID);
      if (curr_SignPost_ptr == NULL)
      {
         curr_SignPost_ptr=generate_new_SignPost(
            1,1,UVW[i]-avg_UVW,curr_ID);
         curr_SignPost_ptr->set_label(label[i]);
//         curr_SignPost_ptr->set_permanent_color(colorfunc::cyan);
         curr_SignPost_ptr->set_permanent_color(colorfunc::white);
//         curr_SignPost_ptr->set_permanent_color(colorfunc::red);
      } // curr_SignPost_ptr==NULL conditional
   } // loop over index i labeling entries in SignPost_ID STL vector

   update_display();
   reset_colors();

   return true;
}

/*
// ==========================================================================
// SKS database insertion and retrieval methods:
// ==========================================================================

// Member function insert_into_SKS_worldmodel_database takes in a SignPost
// object and retrieves its spatial and temporal coordinates along
// with its label.  It inserts this information into the "entity" and
// "nomination" tables within the Postgres "world_model" database set
// up by Jim Garlick on Oct 4, 2006.  If the database insertion fails,
// this boolean method returns false.

bool SignPostsGroup::insert_into_SKS_worldmodel_database(
   SignPost* curr_SignPost_ptr,int& entity_id)
{   
//   cout << "inside SPG::insert_into_SKS_worldmodel_database()" << endl;
   
   if (SKS_worldmodel_database_ptr == NULL || 
       !(SKS_worldmodel_database_ptr->get_connection_status_flag())) 
      return false;
      
   Clock_ptr->current_local_time_and_UTC();
   string curr_date_string=Clock_ptr->YYYY_MM_DD_H_M_S();
   double curr_juliandate=Clock_ptr->get_juliandate();
   double next_week_juliandate=curr_juliandate+7;

   int year,month;
   double next_week_day;
   astrofunc::julian_to_calendar_date(
      next_week_juliandate,year,month,next_week_day);
   int day=basic_math::mytruncate(next_week_day);
   double frac_day=next_week_day-day;
   int hours,mins;
   double secs;
   timefunc::frac_day_to_hms(frac_day,hours,mins,secs);
   Clock_ptr->set_UTC_time(year,month,day,hours,mins,secs);
   string next_week_string=Clock_ptr->YYYY_MM_DD_H_M_S();

   double longitude=curr_SignPost_ptr->get_geopoint_ptr()->get_longitude();
   double latitude=curr_SignPost_ptr->get_geopoint_ptr()->get_latitude();
   double altitude=curr_SignPost_ptr->get_geopoint_ptr()->get_altitude();
//   cout << "long = " << longitude << " lat = " << latitude
//        << " alt = " << altitude << endl;

   vector<string> commands;
   commands.push_back("begin");

//  Entity table entries:

//  id             | integer                  | not null
//  location_id    | character varying(1024)  | 
//  entity_type    | character varying(64)    | not null
//  date_created   | timestamp with time zone | default now()
//  date_installed | timestamp with time zone | default now()
//  source_url     | character varying(1024)  | 
//  data_url       | character varying(1024)  | 
//  metadata_url   | character varying(1024)  | 
//  mimetype       | character varying(128)   | 
//  security_label | integer                  | not null
//  altitude       | double precision         | 
//  geometry       | geometry                 | 

   entity_id=SKS_worldmodel_database_ptr->get_next_id("entity_id_seq");
   const string location_id="LL";

// As of 1/9/07, Delsey Sherrill suggests that we no longer attempt to
// insert entries into the comments table.  Instead, we should direct
// input towards the nomination table.

   string entity_type="nomination";
   int security_label=0;	// unclassified/classified label

   string entity_insert_command= 
      "INSERT into entity(id,location_id,entity_type,date_created,date_installed,security_label,geometry,altitude) ";
   entity_insert_command += "values("+
      stringfunc::number_to_string(entity_id)+",'"+
      location_id+"','"+
      entity_type+"','"+
      curr_date_string+"','"+curr_date_string+
      "',"+stringfunc::number_to_string(security_label)+
      ",'SRID=4326;POINT("+
      stringfunc::number_to_string(longitude,12)+" "+
      stringfunc::number_to_string(latitude,12)+")',"+
      stringfunc::number_to_string(altitude,12)+")";

   commands.push_back(entity_insert_command);
//   cout << "entity_insert_command = " << entity_insert_command << endl;

//  Comments table entries:

//  Table "public.comments"
//  Column        |          Type           | Modifiers 
//  ---------------------+-------------------------+-----------
//  id                  | integer                 | not null
//  entity_id           | integer                 | not null
//  comment_type        | character varying(64)   | not null
//  name                | character varying(256)  | 
//  text                | text                    | 
//  labeltextx          | character varying(1024) | 
//  labeltexty          | character varying(1024) | 
//  labeltype           | character varying(128)  | 
//  linecolor           | character varying(128)  | 
//  movieclip           | character varying(1024) | 
//  comment_id          | character varying(1024) | 
//  poi_id              | character varying(1024) | 
//  textbackgroundcolor | character varying(128)  | 
//  textcolor           | character varying(128)  | 
//  url                 | character varying(1024) | 
//  username            | character varying(128)  | 
//  x                   | double precision        | 
//  y                   | double precision        | 
//  xscale              | double precision        | 
//  yscale              | double precision        | 
//  zoom                | character varying(64)   | 
//  security_label      | integer                 | not null

// Note: As of 1/5/07, Delsey Sherrill says that signpost labels should be stored
//within the name rather than the text field in the Comments table.

//   int comment_id=SKS_worldmodel_database_ptr->get_next_id("comment_id_seq"); 
//   string comment_type="com_poi";
//   string comment_type="com_label";
//   string comments_insert_command=
//      "INSERT into comments(id,entity_id,comment_type,name,security_label) ";

// Nomination table entries:

// Table "public.nomination"

//        Column         |            Type             | Modifiers 
//-----------------------+-----------------------------+-----------
// id                    | integer                     | not null
// entity_id             | integer                     | not null
// nomination_type       | character varying(128)      | not null
// nomination_sub_type   | character varying(128)      | 
// nomination_label      | character varying(128)      | not null
// nomination_start_time | timestamp without time zone | not null
// nomination_end_time   | timestamp without time zone | not null
// security_label        | integer                     | not null

   int nomination_id=SKS_worldmodel_database_ptr->get_next_id(
      "nomination_id_seq"); 
   string nomination_type="TARGET_NOMINATION";

   string nomination_insert_command=
      "INSERT into nomination(id,entity_id,nomination_type,nomination_label,nomination_start_time,nomination_end_time,security_label) ";
   nomination_insert_command += "values("+
      stringfunc::number_to_string(nomination_id)+","+
      stringfunc::number_to_string(entity_id)+",'"+
      nomination_type+"','"+
      curr_SignPost_ptr->get_label()+"','"+
      curr_date_string+"','"+
      next_week_string+"',"+
      stringfunc::number_to_string(security_label)+")";
   commands.push_back(nomination_insert_command);
//   cout << "nomination_insert_command = " << nomination_insert_command 
//        << endl;
   commands.push_back("commit");

   SKS_worldmodel_database_ptr->set_SQL_commands(commands);
   SKS_worldmodel_database_ptr->execute_SQL_commands();

   return true;
}

// --------------------------------------------------------------------------
// Member function retrieve_signposts_from_SKS_worldmodel_database reads
// the contents of the "entity" and "nomination" tables within the
// "world_model" ISDS database.  It retrieves the 3D location and
// label for each signpost within the database.  This method next
// purges all existing signposts and instantiates a new set based upon
// the database information.

void SignPostsGroup::retrieve_signposts_from_SKS_worldmodel_database()
{   
//   cout << "inside SignPostsGruop::retrieve_signposts_from_SKS_worldmodel_database()" 
//        << endl;

   if (SKS_worldmodel_database_ptr == NULL || 
       !(SKS_worldmodel_database_ptr->get_connection_status_flag())) return;

   vector<string> commands;
   string select_command = "SELECT entity_id,date_created as timestamp,";
   select_command += "x(geometry) as Longitude,y(geometry) as Latitude,";
   select_command += "altitude as Altitude,nomination_label as label ";
   select_command += 
      "from entity,nomination where entity.id=nomination.entity_id";
//   cout << "select_command = " << select_command << endl;
   commands.push_back(select_command);

   SKS_worldmodel_database_ptr->set_SQL_commands(commands);
   SKS_worldmodel_database_ptr->execute_SQL_commands();

   Genarray<string>* field_array_ptr=SKS_worldmodel_database_ptr->
      get_field_array_ptr();
//   cout << "*field_array_ptr = " << *field_array_ptr << endl;

   vector<double> curr_time;
   vector<int> SignPost_ID;	// Someday replace vector with map
   vector<double> latitude,longitude,altitude;
   vector<threevector> UVW;
   vector<string> label;
   
   for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
   {
      bool valid_database_entry=true;
      bool altitude_specified_flag=true;
      for (unsigned int j=0; j<field_array_ptr->get_ndim(); j++)
      {

// If altitude entry within database is NULL, we'll tentatively assign
// a 0 altitude value.  Later, we'll try to extract a better height
// estimate from PointCloud information if it is available:

         if (field_array_ptr->get(i,j)=="NULL" && j != 4)
         {
            valid_database_entry=false;
         }
         else if (field_array_ptr->get(i,4)=="NULL")
         {
            altitude_specified_flag=false;
         }
      }
      if (valid_database_entry)
      {
         curr_time.push_back(0);
         SignPost_ID.push_back(stringfunc::string_to_integer(
            field_array_ptr->get(i,0)));
         longitude.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,2)));
         latitude.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,3)));

         bool northern_hemisphere_flag;
         int UTM_zonenumber;
         double X,Y;
         latlongfunc::LLtoUTM(
            latitude.back(),longitude.back(),
            UTM_zonenumber,northern_hemisphere_flag,Y,X);
         altitude.push_back(retrieved_SignPost_altitude(
            altitude_specified_flag,field_array_ptr->get(i,4),X,Y));

         if (Ellipsoid_model_ptr != NULL)
         {
            threevector XYZ=Ellipsoid_model_ptr->ConvertLongLatAltToECI(
               longitude.back(),latitude.back(),altitude.back(),*Clock_ptr);
            UVW.push_back(XYZ);
         }
         else
         {
            UVW.push_back(threevector(X,Y,altitude.back()));
         }

         label.push_back(field_array_ptr->get(i,5));

      } // valid_database_entry conditional
   } // loop over index i labeling database rows

//   for (unsigned int i=0; i<curr_time.size(); i++)
//   {
//      cout << "time = " << curr_time[i] << endl;
//      cout << " ID = " << SignPost_ID[i]
//           << " label = " << label[i] << endl;
//      cout << " UVW = " << UVW[i] << endl;
//   }

// Scan through all existing SignPosts and delete any whose IDs do not
// exist within STL container SignPost_ID:

   int n_signposts_destroyed=0;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Graphical* curr_Graphical_ptr=get_Graphical_ptr(n);
      bool delete_Graphical_flag=true;
      for (unsigned int i=0; i<SignPost_ID.size(); i++)
      {
         if (curr_Graphical_ptr->get_ID()==SignPost_ID[i])
         {
            delete_Graphical_flag=false;
         }
         SignPost* curr_SignPost_ptr=
            dynamic_cast<SignPost*>(curr_Graphical_ptr);
         if (!(curr_SignPost_ptr->get_SKS_worldmodel_database_flag()))
         {
            delete_Graphical_flag=false;
         }
      }
      if (delete_Graphical_flag)
      {
         destroy_Graphical(curr_Graphical_ptr);
         n_signposts_destroyed++;
      }
   } // loop over index n labeling existing SignPosts

   for (unsigned int i=0; i<SignPost_ID.size(); i++)
   {
      int curr_ID=SignPost_ID[i];
      SignPost* curr_SignPost_ptr=get_ID_labeled_SignPost_ptr(curr_ID);
      if (curr_SignPost_ptr == NULL)
      {
         curr_SignPost_ptr=generate_new_SignPost(1,1,UVW[i],curr_ID);
         curr_SignPost_ptr->set_SKS_worldmodel_database_flag(true);
      }
      
      curr_SignPost_ptr->set_label(label[i]);
      curr_SignPost_ptr->set_max_text_width(label[i]);
      curr_SignPost_ptr->get_geopoint_ptr()->set_longitude(longitude[i]);
      curr_SignPost_ptr->get_geopoint_ptr()->set_latitude(latitude[i]);
      curr_SignPost_ptr->get_geopoint_ptr()->set_altitude(altitude[i]);
      curr_SignPost_ptr->get_geopoint_ptr()->recompute_UTM_coords();

//      cout << "Signpost geopoint = " 
//           << *(curr_SignPost_ptr->get_geopoint_ptr()) << endl;
   } // loop over index i labeling entries in SignPost_ID STL container

   string banner=stringfunc::number_to_string(SignPost_ID.size())+
      " SignPosts generated from world model database";
//   outputfunc::write_banner(banner);

   reset_colors();
}

// --------------------------------------------------------------------------
// Member function update_SKS_worldmodel_database_entry assumes that the
// input curr_SignPost already exists within the Postgres database.
// It updates the database's information about the SignPosts's
// world-space position and text.

void SignPostsGroup::update_SKS_worldmodel_database_entry(
   SignPost* curr_SignPost_ptr)
{   
//   cout << "inside SPG::update_SKS_worldmodel_database_entry()" << endl;
   if (SKS_worldmodel_database_ptr == NULL || 
       !(SKS_worldmodel_database_ptr->get_connection_status_flag())) return;

   double longitude=curr_SignPost_ptr->get_geopoint_ptr()->get_longitude();
   double latitude=curr_SignPost_ptr->get_geopoint_ptr()->get_latitude();
   double altitude=curr_SignPost_ptr->get_geopoint_ptr()->get_altitude();
//   cout << "longitude = " << longitude << " latitude = " << latitude
//        << " altitude = " << altitude << endl;

   vector<string> commands;
   string geometry_update_command="UPDATE entity SET geometry = ";
   geometry_update_command += 
      "'SRID=4326;POINT("+stringfunc::number_to_string(longitude,12)+" "+
      stringfunc::number_to_string(latitude,12)+")' ";
   geometry_update_command +=
      "where id = "+stringfunc::number_to_string(curr_SignPost_ptr->get_ID());
//   cout << "geometry_update_command = " 
//	<< geometry_update_command << endl;
   commands.push_back(geometry_update_command);

   string altitude_update_command="UPDATE entity SET altitude = ";
   altitude_update_command += stringfunc::number_to_string(altitude,10)+" ";
   altitude_update_command +=
      "where id = "+stringfunc::number_to_string(curr_SignPost_ptr->get_ID());
//   cout << "altitude_update_command = " 
//	<< altitude_update_command << endl;
   commands.push_back(altitude_update_command);

   SKS_worldmodel_database_ptr->set_SQL_commands(commands);
   SKS_worldmodel_database_ptr->execute_SQL_commands();
}

// --------------------------------------------------------------------------
void SignPostsGroup::update_SKS_worldmodel_database_nomination_label(
   SignPost* curr_SignPost_ptr)
{   
   if (SKS_worldmodel_database_ptr == NULL || 
       !(SKS_worldmodel_database_ptr->get_connection_status_flag())) return;

   vector<string> commands;
   string nomination_update_command=
      "UPDATE nomination SET nomination_label = '";
   nomination_update_command += curr_SignPost_ptr->get_label();
   nomination_update_command += "' where entity_id = "
      +stringfunc::number_to_string(curr_SignPost_ptr->get_ID());
   commands.push_back(nomination_update_command);

   SKS_worldmodel_database_ptr->set_SQL_commands(commands);
   SKS_worldmodel_database_ptr->execute_SQL_commands();
}
*/

// ==========================================================================
// PostGIS database methods
// ==========================================================================

// Member function retrieve_all_signposts_from_PostGIS_databases loops
// over every entry within STL map *databases_map_ptr.  For each
// PostGIS database, this method retrieves all tables containing GIS
// point information.  It then converts those GIS points into
// SignPosts.

bool SignPostsGroup::retrieve_all_signposts_from_PostGIS_databases(
   colorfunc::Color signposts_color,double SignPost_size)
{
//   cout << "inside SignPostsGroup::retrieve_all_signposts_from_PostGIS_database()" << endl;

   bool signposts_retrieved_flag=false;

   if (postgis_databases_group_ptr==NULL)
   {
      cout << "Error in SignPostsGroup::retrieve_all_signposts_from_PostGIS_datbases()" << endl;
      cout << "postgis_databases_group_ptr = NULL ! " << endl;
      outputfunc::enter_continue_char();
      return signposts_retrieved_flag;
   }

   vector<postgis_database*> postgis_database_ptrs=
      postgis_databases_group_ptr->get_postgis_database_ptrs();
   for (unsigned int i=0; i<postgis_database_ptrs.size(); i++)
   {
      postgis_database* postgis_database_ptr=postgis_database_ptrs[i];
      
      vector<string> GISpoint_tablenames=postgis_database_ptr->
         get_GISpoint_tablenames();
      for (unsigned int t=0; t<GISpoint_tablenames.size(); t++)
      {
//         cout << "t = " << t
//              << " GISpoint_tablenames = "
//              << GISpoint_tablenames[t] << endl;
         
         if (retrieve_signposts_from_PostGIS_database(
            signposts_color,postgis_database_ptr->get_databasename(),
            GISpoint_tablenames[t],SignPost_size))
         {
            signposts_retrieved_flag=true;
            identify_and_revise_Bluegrass_labels(postgis_database_ptr);
         }
      } // loop over index t labeling GISpoint tablenames
   } // loop over index i labeling postgis database pointers

   return signposts_retrieved_flag;
}

// --------------------------------------------------------------------------
// Member function retrieve_signposts_from_PostGIS_database reads the
// contents of the input table within the input PostGIS database.  It
// retrieves the 3D location and label for each signpost within the
// database.  This method instantiates a new set of signposts based
// upon the database information.

bool SignPostsGroup::retrieve_signposts_from_PostGIS_database(
   colorfunc::Color signposts_color,string DatabaseName,string TableName,
   double SignPost_size)
{   
//   cout << "inside SignPostsGroup::retrieve_signposts_from_PostGIS_database()//" 
//        << endl;
//   cout << "TableName = " << TableName << endl;

   postgis_database* postgis_database_ptr=
      postgis_databases_group_ptr->initialize_database_and_table(
         DatabaseName,TableName);

//   select_command = 
//      "SELECT gid,name,st_x(the_geom) as longitude,st_y(the_geom) as latitude";
//   select_command += " from "+postgis_database_ptr->get_TableName();
//   cout << "select_command = " << select_command << endl;

// As of 12/18/07, we attempt to execute two SQL queries.  The first
// is more complicated than the second.  If the first fails, we
// execute the second:

   vector<string> select_command;

// Note added in Jan 2014: In order to retrieve 2D X,Y spatial
// coordinates under Postgis 2.X, we need to chant st_x(the_geom) and
// st_y(the_geom) rather than x(the_geom) and y(the_geom):
   
   string curr_select_command=
      "SELECT gid,name,st_x(the_geom) as longitude,st_y(the_geom) as latitude,category";
   curr_select_command += " from "+postgis_database_ptr->get_TableName();
   select_command.push_back(curr_select_command);
//   cout << "1st select_command = " << curr_select_command << endl;

   curr_select_command=
      "SELECT gid,name,st_x(the_geom) as longitude,st_y(the_geom) as latitude";
   curr_select_command += " from "+postgis_database_ptr->get_TableName();
   select_command.push_back(curr_select_command);
//   cout << "2nd select_command = " << curr_select_command << endl;

   int postgis_command_successfully_executed=-1;
   for (unsigned int iter=0; iter<select_command.size() && 
           postgis_command_successfully_executed==-1; iter++)
   {
//      cout << "iter = " << iter << endl;

      vector<string> commands;
      commands.push_back(select_command[iter]);

//      cout << "Attempting SQL command = " << endl;
//      cout << commands.back() << endl;
      
      postgis_database_ptr->set_SQL_commands(commands);
      if (postgis_database_ptr->execute_SQL_commands())
      {
         postgis_command_successfully_executed=iter;
         break;
      }
   } // loop over iter index labeling different select commands
   
//   cout << "postgis_command_successfully_executed = "
//        << postgis_command_successfully_executed << endl;
   if (postgis_command_successfully_executed==-1)
   {
      return false;
   }
   else if (postgis_command_successfully_executed==0)
   {
      postgis_database_ptr->set_category_column_flag(true);
   }
   
   Genarray<string>* field_array_ptr=postgis_database_ptr->
      get_field_array_ptr();
//    cout << "*field_array_ptr = " << *field_array_ptr << endl;

   vector<double> curr_time;
   vector<int> SignPost_ID;	// Someday replace vector with map
   vector<double> latitude,longitude,altitude;
   vector<threevector> UVW;
   vector<string> label,category;

   for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
   {
      double curr_longitude=stringfunc::string_to_number(
         field_array_ptr->get(i,2));
      double curr_latitude=stringfunc::string_to_number(
         field_array_ptr->get(i,3));

      bool northern_hemisphere_flag;
      int UTM_zonenumber;
      double X,Y;
      latlongfunc::LLtoUTM(
         curr_latitude,curr_longitude,
         UTM_zonenumber,northern_hemisphere_flag,Y,X);

// Don't instantiate SignPost if it would lie outside PostGIS
// database's current bounding box:

      if (postgis_database_ptr->get_gis_bbox().point_inside(X,Y))
      {
         curr_time.push_back(0);
         SignPost_ID.push_back(stringfunc::string_to_integer(
            field_array_ptr->get(i,0)));
         longitude.push_back(curr_longitude);
         latitude.push_back(curr_latitude);
         altitude.push_back(retrieved_SignPost_altitude(false,"",X,Y));
      
         if (Ellipsoid_model_ptr != NULL)
         {
            threevector XYZ=Ellipsoid_model_ptr->ConvertLongLatAltToECI(
               longitude.back(),latitude.back(),altitude.back(),*Clock_ptr);
            UVW.push_back(XYZ);
         }
         else
         {
            UVW.push_back(threevector(X,Y,altitude.back()));
         }
         label.push_back(field_array_ptr->get(i,1));

         if (postgis_command_successfully_executed==0)
         {
            category.push_back(field_array_ptr->get(i,4));
         }
         else
         {
            category.push_back("");
         }
         
      } // (X,Y) lies inside postgis bbox conditional
   } // loop over index i labeling database rows

//   cout.precision(12);
//   for (unsigned int i=0; i<curr_time.size(); i++)
//   {
//      cout << "time = " << curr_time[i] << endl;
//      cout << " ID = " << SignPost_ID[i]
//           << " label = " << label[i] 
//           << " category = " << category[i] << endl;
//      cout << "long = " << longitude[i] << " lat = " << latitude[i]
//           << " alt = " << altitude[i] << endl;
//      cout << " UVW = " << UVW[i] << endl;
//   }

// Instantiate new set of SignPosts using geolocation and label
// information read in from PostGIS database.  Assign new SignPosts
// read in from some GIS layer to their own distinct OSGsubPAT:

//   int OSGsubPAT_ID=get_n_OSGsubPATs();
   int largest_used_ID=get_largest_used_ID();
   for (unsigned int i=0; i<SignPost_ID.size(); i++)
   {

// To ensure we don't overwrite some already existing signpost, assign
// ID to new signpost which is guaranteed to be greater than any
// existing signpost ID:

      int curr_ID=largest_used_ID+1+SignPost_ID[i];

      SignPost* curr_SignPost_ptr=get_ID_labeled_SignPost_ptr(curr_ID);
      if (curr_SignPost_ptr == NULL)
      {
         curr_SignPost_ptr=generate_new_SignPost(
            SignPost_size,1,UVW[i],curr_ID);

         if (Ellipsoid_model_ptr != NULL)
         {
            double curr_long,curr_lat,curr_alt;
            Ellipsoid_model_ptr->ConvertECIToLongLatAlt(
               UVW[i],*Clock_ptr,curr_long,curr_lat,curr_alt);
         }
      }

      curr_SignPost_ptr->set_label(label[i]);
      curr_SignPost_ptr->set_category(category[i]);
      curr_SignPost_ptr->set_max_text_width(label[i]);
      
      curr_SignPost_ptr->get_geopoint_ptr()->set_longitude(longitude[i]);
      curr_SignPost_ptr->get_geopoint_ptr()->set_latitude(latitude[i]);
      curr_SignPost_ptr->get_geopoint_ptr()->set_altitude(altitude[i]);
      curr_SignPost_ptr->get_geopoint_ptr()->recompute_UTM_coords();
      curr_SignPost_ptr->set_permanent_color(colorfunc::get_OSG_color(
         signposts_color));

      (*signposts_map_ptr)[label[i]]=curr_SignPost_ptr;

//      cout << "Signpost geopoint = " 
//           << *(curr_SignPost_ptr->get_geopoint_ptr()) << endl;
   } // loop over index i labeling entries in SignPost_ID STL container

   string banner=stringfunc::number_to_string(SignPost_ID.size())+
      " signposts generated from database";
//   outputfunc::write_banner(banner);

   update_display();
   reset_colors();

// Record the fact that points from GIS layer corresponding to
// TableName have been read in so that they never have to be read in
// again:

   postgis_databases_group_ptr->record_tablename_as_read(TableName);

   return true;
}

// --------------------------------------------------------------------------
// Member function retrieve_signposts_from_Cambridge_database reads
// the contents of the input table within the input PostGIS database.
// It retrieves the 3D location and label for each signpost within the
// database.  This method instantiates a new set of signposts based
// upon the database information.

bool SignPostsGroup::retrieve_signposts_from_Cambridge_database(
   colorfunc::Color signposts_color,string DatabaseName,string TableName,
   double SignPost_size)
{   
   cout << "inside SignPostsGroup::retrieve_signposts_from_Cambridge_database()" << endl;
   cout << "TableName = " << TableName << endl;

   postgis_database* postgis_database_ptr=
      postgis_databases_group_ptr->initialize_database_and_table(
         DatabaseName,TableName);

//   select_command = 
//      "SELECT gid,name,st_x(the_geom) as longitude,st_y(the_geom) as latitude";
//   select_command += " from "+postgis_database_ptr->get_TableName();
//   cout << "select_command = " << select_command << endl;

// As of 12/18/07, we attempt to execute two SQL queries.  The first
// is more complicated than the second.  If the first fails, we
// execute the second:

   vector<string> select_command;

   string curr_select_command=
      "SELECT gid,full_addr, st_x(the_geom) as longitude,st_y(the_geom) as latitude,roofelev";
   curr_select_command += " from "+postgis_database_ptr->get_TableName();
   select_command.push_back(curr_select_command);
   cout << "1st select_command = " << curr_select_command << endl;

   int postgis_command_successfully_executed=-1;
   for (unsigned int iter=0; iter<select_command.size() && 
           postgis_command_successfully_executed==-1; iter++)
   {
//      cout << "iter = " << iter << endl;

      vector<string> commands;
      commands.push_back(select_command[iter]);

//      cout << "Attempting SQL command = " << endl;
//      cout << commands.back() << endl;
      
      postgis_database_ptr->set_SQL_commands(commands);
      if (postgis_database_ptr->execute_SQL_commands())
      {
         postgis_command_successfully_executed=iter;
         break;
      }
   } // loop over iter index labeling different select commands
   
//   cout << "postgis_command_successfully_executed = "
//        << postgis_command_successfully_executed << endl;
   if (postgis_command_successfully_executed==-1)
   {
      return false;
   }
   
   Genarray<string>* field_array_ptr=postgis_database_ptr->
      get_field_array_ptr();
//   cout << "*field_array_ptr = " << *field_array_ptr << endl;

   vector<double> curr_time;
   vector<int> SignPost_ID;	// Someday replace vector with map
   vector<double> latitude,longitude,altitude;
   vector<threevector> UVW;
   vector<string> label;

   typedef map<twovector,int,lttwovector> QUANTIZED_POINTS_MAP;
   QUANTIZED_POINTS_MAP quantized_points_map;

   for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
   {
      double curr_longitude=stringfunc::string_to_number(
         field_array_ptr->get(i,2));
      double curr_latitude=stringfunc::string_to_number(
         field_array_ptr->get(i,3));

      const double meters_per_feet=0.3048;
      double curr_altitude=stringfunc::string_to_number(
         field_array_ptr->get(i,4))*meters_per_feet;

      bool northern_hemisphere_flag;
      int UTM_zonenumber;
      double X,Y;
      latlongfunc::LLtoUTM(
         curr_latitude,curr_longitude,
         UTM_zonenumber,northern_hemisphere_flag,Y,X);
      twovector curr_quantized_pt(basic_math::round(X),basic_math::round(Y));
      QUANTIZED_POINTS_MAP::iterator iter=quantized_points_map.find(
         curr_quantized_pt);
      if (iter != quantized_points_map.end()) continue;

      quantized_points_map[curr_quantized_pt]=1;

// Don't instantiate SignPost if it would lie outside PostGIS
// database's current bounding box:

      if (postgis_database_ptr->get_gis_bbox().point_inside(X,Y))
      {
         curr_time.push_back(0);
         SignPost_ID.push_back(stringfunc::string_to_integer(
            field_array_ptr->get(i,0)));
         longitude.push_back(curr_longitude);
         latitude.push_back(curr_latitude);

         curr_altitude=basic_math::max(curr_altitude,
                           retrieved_SignPost_altitude(false,"",X,Y));
         altitude.push_back(curr_altitude);
         UVW.push_back(threevector(X,Y,altitude.back()));
         label.push_back(field_array_ptr->get(i,1));

      } // (X,Y) lies inside postgis bbox conditional
   } // loop over index i labeling database rows

   generate_PostGIS_SignPosts(
      curr_time,SignPost_ID,latitude,longitude,altitude,UVW,label,
      SignPost_size,signposts_color);

// Record the fact that points from GIS layer corresponding to
// TableName have been read in so that they never have to be read in
// again:

   postgis_databases_group_ptr->record_tablename_as_read(TableName);

   return true;
}

// --------------------------------------------------------------------------
// Member function retrieved_SignPost_altitude assigns a height value
// to a SignPost retrieved from a PostGIS database based upon its
// altitude entry, its point cloud location or a default zero value.

double SignPostsGroup::retrieved_SignPost_altitude(
   bool altitude_specified_flag,string field_entry,double X,double Y)
{
//   cout << "inside SPG::retrieved_SignPost_alt()" << endl;

   double default_Z=get_grid_world_origin().get(2);
   double Z=default_Z;
   if (altitude_specified_flag)
   {
      Z=stringfunc::string_to_number(field_entry);
   }
   else if (!altitude_specified_flag && PointCloudsGroup_ptr != NULL)
   {
      for (unsigned int c=0; c<PointCloudsGroup_ptr->get_n_Graphicals(); c++)
      {
         PointCloud* curr_cloud_ptr=PointCloudsGroup_ptr->get_Cloud_ptr(c);

// Recall that we vertically position point clouds & surfaces on the
// blue marble such that their minimal heights just touch the
// ellipsoid.  This choice prevents negative height locations from
// being occluded by the marble itself.  But all altitudes are
// effectively increased by -bbox_zmin.  We consequently need to take
// this vertical translation into account when positioning signposts
// on the blue marble:

// On 3/22/07, we added an additional cluge in the form of the
// following extra_delta_z variable to allow the signpost associated
// with the Baghdad Clock Tower to be elevated upwards.  For reasons
// we don't understand, the tip of the Clock Tower's arrow appears
// below the top of the clock on the baghdad surface.  (It may stem
// from an inaccurate clock tower height extraction from a low
// level-of-detail approximation within the multi-resolution Baghdad
// surface.)  At any rate, we elevate all Baghdad signposts by 20
// meters in order to force the Clock Tower's arrow to appear above
// the top of the clock.  We hope to eliminate this ugly cluge
// someday...

         double extra_delta_z=0;

         double zmin_displacement=0;
         if (Ellipsoid_model_ptr != NULL)
         {
            zmin_displacement=curr_cloud_ptr->get_xyz_bbox().zMin();
            extra_delta_z=20;	// meters
         }

         double Z_estimated;
         if (curr_cloud_ptr->find_Z_given_XY(X,Y,Z_estimated))
         {
            Z=Z_estimated-zmin_displacement+extra_delta_z;
//            if (nearly_equal(Z,0))
//            {
//               cout << "Z_estimated = " << Z_estimated
//                    << " extra_delta_z = " << extra_delta_z
//                    << " zmin_displacement = " << zmin_displacement
//                    << endl;
//            }
         }

      } // loop over index c labeling clouds within PointCloudsGroup
   } 

// On 3/11/07, we empirically observed that Baghdad signposts lying
// outside ladar surface are assigned nonsensical altitudes.  So force
// such outlying signposts' altitudes to equal 0:

   const double max_Z=5000;	// meters
   if (Z > max_Z) Z=default_Z;

   if (Z < default_Z)
   {
//      cout << "inside SPG::retrieved_SignPost_alt(), Z = " << Z << endl;
   }

   return Z;
}

// --------------------------------------------------------------------------
// Member function gazetteer() queries the user to enter the label for
// some SignPost.  It then performs a search of *signposts_map_ptr for
// the input label.  This method returns a pointer to the retrieved
// SignPost if it exists and NULL otherwise.

SignPost* SignPostsGroup::gazetteer()
{
//   cout << "inside SignPostsGroup::gazetteer()" << endl;

   string label=inputfunc::enter_string("Enter signpost label:");
   cout << "label = " << label << endl;

   SIGNPOSTS_MAP::iterator signpost_iter=signposts_map_ptr->
      find(label);
   if (signpost_iter==signposts_map_ptr->end())
   {
      cout << "Could not find signpost corresponding to label = "
           << label << endl;
      return NULL;
   }
   else
   {
      SignPost* SignPost_ptr=signpost_iter->second;
      cout << "Signpost = " << *SignPost_ptr << endl;

      threevector UVW;
      if (SignPost_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),UVW))
      {
         cout << "posn = " << UVW << endl;
      }
      return SignPost_ptr;
   }
}

// --------------------------------------------------------------------------
// Member function generate_PostGIS_SignPosts() instantiates a new set
// of SignPosts based upon information retrieved from a PostGIS
// database and stored within input STL vectors.

void SignPostsGroup::generate_PostGIS_SignPosts(
   const vector<double>& curr_time,const vector<int>& SignPost_ID,
   const vector<double>& latitude,const vector<double>& longitude,
   const vector<double>& altitude,const vector<threevector>& UVW,
   const vector<string> label,
   double SignPost_size,colorfunc::Color signposts_color)
{   
   cout << "inside SignPostsGroup::generate_PostGIS_SignPosts()" << endl;

   cout.precision(12);
//   for (unsigned int i=0; i<curr_time.size(); i++)
//   {
//      cout << "time = " << curr_time[i] << endl;
//      cout << " ID = " << SignPost_ID[i]
//           << " label = " << label[i] << endl;
//      cout << "long = " << longitude[i] << " lat = " << latitude[i]
//           << " alt = " << altitude[i] << endl;
//      cout << " UVW = " << UVW[i] << endl;
//   }
//   cout << "SignPost_ID.size() = " << SignPost_ID.size() << endl;

// Instantiate new set of SignPosts using geolocation and label
// information read in from PostGIS database.  Assign new SignPosts
// read in from some GIS layer to their own distinct OSGsubPAT:

   int largest_used_ID=get_largest_used_ID();
   for (unsigned int i=0; i<SignPost_ID.size(); i++)
   {

// To ensure we don't overwrite some already existing signpost, assign
// ID to new signpost which is guaranteed to be greater than any
// existing signpost ID:

      int curr_ID=largest_used_ID+1+SignPost_ID[i];

      SignPost* curr_SignPost_ptr=get_ID_labeled_SignPost_ptr(curr_ID);
      if (curr_SignPost_ptr == NULL)
      {
         curr_SignPost_ptr=generate_new_SignPost(
            SignPost_size,1,UVW[i],curr_ID);
      }

      curr_SignPost_ptr->set_label(label[i]);
      curr_SignPost_ptr->set_max_text_width(label[i]);
      
      curr_SignPost_ptr->get_geopoint_ptr()->set_longitude(longitude[i]);
      curr_SignPost_ptr->get_geopoint_ptr()->set_latitude(latitude[i]);
      curr_SignPost_ptr->get_geopoint_ptr()->set_altitude(altitude[i]);
      curr_SignPost_ptr->get_geopoint_ptr()->recompute_UTM_coords();
      curr_SignPost_ptr->set_permanent_color(colorfunc::get_OSG_color(
         signposts_color));

      (*signposts_map_ptr)[label[i]]=curr_SignPost_ptr;

//      cout << "Signpost geopoint = " 
//           << *(curr_SignPost_ptr->get_geopoint_ptr()) << endl;
   } // loop over index i labeling entries in SignPost_ID STL container

   string banner=stringfunc::number_to_string(SignPost_ID.size())+
      " signposts generated from database";
   outputfunc::write_banner(banner);

   update_display();
   reset_colors();
}

// ==========================================================================
// Message handling member functions
// ==========================================================================

// Member function issue_invocation_message sends the name of a
// selected powerpoint file to a powerpoint messenger queue.

bool SignPostsGroup::issue_invocation_message()
{   
//   cout << "inside SignPostsGroup::issue_invocation_message()" << endl;

   int selected_Graphical_ID=get_selected_Graphical_ID();
//   cout << "selected_Graphical_ID = " << selected_Graphical_ID << endl;
   
   if (selected_Graphical_ID >= 0)
   {
      string label=get_ID_labeled_SignPost_ptr(selected_Graphical_ID)->
         get_label()+extra_textmessage_info+" ";
//      cout << "label = " << label << endl;
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         label);
      
      string text_message;
      for (unsigned int s=0; s<substring.size()-1; s++)
      {
         text_message += substring[s]+"+";
      }
      text_message += substring.back();
//      cout << "text_message = " << text_message << endl;

      string url=
         "http://www.google.com/search?hl=en&btnI=I%27m+Feeling+Lucky&q=";
      url += text_message;
//      url += "+wiki";
//      cout << "url = " << url << endl;

      if (get_Messenger_ptr() != NULL)
         get_Messenger_ptr()->sendTextMessage(url);

      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
// Member function broadcast_NFOV_aimpoint() issues an ActiveMQ message
// with the XY coordinates for the last entered aimpoint SignPost.

void SignPostsGroup::broadcast_NFOV_aimpoint()
{
//   cout << "inside SignPostsGroup::broadcast_NFOV_aimpoint()" << endl;

   if (!broadcast_NFOV_aimpoint_flag) return;

   SignPost* last_SignPost_ptr=dynamic_cast<SignPost*>(
      get_last_Graphical_ptr());
   if (last_SignPost_ptr==NULL) return;

   threevector SignPost_posn;
   if (last_SignPost_ptr->get_UVW_coords(
          get_curr_t(),get_passnumber(),SignPost_posn))
   {         
//      cout << "SignPost_posn = " << SignPost_posn << endl;
   }

   string command="RESET_AIMPOINT";
   string key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   key="ID";
   value=stringfunc::number_to_string(last_SignPost_ptr->get_ID());
   properties.push_back(property(key,value));

   key="X Y";
   value=stringfunc::number_to_string(SignPost_posn.get(0))+" "
      +stringfunc::number_to_string(SignPost_posn.get(1));
   properties.push_back(property(key,value));

   get_Messenger_ptr()->broadcast_subpacket(command,properties);
} 

// ==========================================================================
// Bluegrass specific member functions
// ==========================================================================

// Member function identify_and_revise_Bluegrass_labels tries to
// decompose each SignPosts's label into the form
// string_integer_dot_integer.  If successful, this method replaces
// the initial string with a more human-readable substitute.
// Otherwise, the SignPosts label is left unchanged.

bool SignPostsGroup::identify_and_revise_Bluegrass_labels(
   postgis_database* postgis_database_ptr)
{
//   cout << "inside SignPostsGroup::identify_and_revise_Bluegrass_labels()"
//        << endl;

// First check whether input postgis database contains a category
// column.  If not, it cannot correspond to the Dec 2007 bluegrass
// database:

   if (!postgis_database_ptr->get_category_column_flag()) return false;

   vector<int> SignPost_IDs_to_destroy;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      SignPost* curr_SignPost_ptr=get_SignPost_ptr(n);
      int curr_ID=curr_SignPost_ptr->get_ID();
      string label=curr_SignPost_ptr->get_label();
      string category=curr_SignPost_ptr->get_category();

      bool valid_bluegrass_label=true;      
      if (category.size()==0)
      {
         valid_bluegrass_label=false;
         SignPost_IDs_to_destroy.push_back(curr_ID);
         continue;
      }
      
//      cout << "n = " << n << " label = " << label << endl;
      
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         label,".");
      if (substrings.size() != 2)
      {
//         cout << "n_substrings = " << substrings.size() << endl;
         valid_bluegrass_label=false;
         SignPost_IDs_to_destroy.push_back(curr_ID);
         continue;
      }
   
      string prefix=substrings[0];
      int suffix_size=substrings[1].size();
      string suffix=substrings[1].substr(1,suffix_size-1);

      substrings.clear();
      string number_chars="0123456789";
      substrings=stringfunc::decompose_string_into_substrings(
         prefix,number_chars);
   
      string descriptor=substrings[0];
      string region_str;
      for (unsigned int i=1; i<substrings.size(); i++)
      {
         region_str += substrings[i];
      }
//      cout << "region_str = " << region_str << endl;
      int region_number=stringfunc::string_to_integer(region_str);
//      cout << "Region number = " << region_number << endl;

      if (region_number < 0 || region_number > 12)
      {
//         cout << "Invalid region number!" << endl;
          valid_bluegrass_label=false;
         SignPost_IDs_to_destroy.push_back(curr_ID);
         continue;
      }

      string revised_descriptor=descriptor;
      if (descriptor=="B")
      {
         revised_descriptor="Brush Pass";
      }
      else if (descriptor=="C")
      {
         revised_descriptor="Cache";
      }
      else if (descriptor=="DD")
      {
         revised_descriptor="Dead Drop";
      }
      else if (descriptor=="DM")
      {
         revised_descriptor="Dismount";
      }
      else if (descriptor=="G")
      {
         revised_descriptor="General";
      }
      else if (descriptor=="M")
      {
         revised_descriptor="Meeting";
      }
      else if (descriptor=="S")
      {
         revised_descriptor="Site Monitoring";
      }
      else
      {
//         cout << "Invalid descriptor!" << endl;
          valid_bluegrass_label=false;
         SignPost_IDs_to_destroy.push_back(curr_ID);
         continue;
      }
   
      revised_descriptor += " "+region_str+"."+suffix;
//      cout << "Revised descriptor = " << revised_descriptor << endl;

      if (valid_bluegrass_label)
      {
         curr_SignPost_ptr->set_label(revised_descriptor);
         curr_SignPost_ptr->set_max_text_width("12345678901234567890");
      }
      
   } // loop over index n labeling SignPosts
   
// Destroy invalid Bluegrass SignPosts:

   for (unsigned int i=0; i<SignPost_IDs_to_destroy.size(); i++)
   {
//      SignPost* destroyed_SignPost_ptr=
//         get_ID_labeled_SignPost_ptr(SignPost_IDs_to_destroy[i]);
//      cout << "i = " << i 
//           << " Destroyed ID = " << SignPost_IDs_to_destroy[i] 
//           << " label = " << destroyed_SignPost_ptr->get_label() << endl;
      destroy_SignPost(SignPost_IDs_to_destroy[i]);
   }

   return true;
}

// ==========================================================================
// LOST ground target member functions
// ==========================================================================

// Member function get_ground_target_posns() reads in the positions
// for manually selected ground targets stored in
// GroundTarget_SignPostsGroup_ptr and transfers them to STL vector
// target_posns.  This method returns the targets' 3D COM,

threevector SignPostsGroup::get_ground_target_posns(
   vector<twovector>& target_posns)
{   
//   cout << "inside SignPostsGroup::get_ground_target_posns()" << endl;

   threevector target_posn_COM;
   for (unsigned int t=0; t<get_n_Graphicals(); t++)
   {
      SignPost* GroundTarget_SignPost_ptr=get_SignPost_ptr(t);
      threevector posn;
      if (GroundTarget_SignPost_ptr->get_UVW_coords(
         get_curr_t(),get_passnumber(),posn))
      {
         target_posn_COM += posn;
         target_posns.push_back(twovector(posn));

//         bool northern_hemisphere_flag=get_EarthRegionsGroup_ptr()->
//            get_northern_hemisphere_flag();
//         int UTM_zone=get_EarthRegionsGroup_ptr()->
//            get_specified_UTM_zonenumber();
//         geopoint curr_point(northern_hemisphere_flag,UTM_zone,
//                             posn.get(0),posn.get(1));
//         cout << "geopoint = " << curr_point << endl;
      }
   } // loop over index t labeling ground target SignPosts

   if (target_posns.size() > 0)
   {
      target_posn_COM /= target_posns.size();
   }
//   cout << "target_posn_COM = " << target_posn_COM << endl;
   return target_posn_COM;
}

// --------------------------------------------------------------------------
// Member function generate_ground_target_w_track()

void SignPostsGroup::generate_ground_target_w_track()
{   
//   cout << "inside SignPostsGroup::generate_ground_target_w_track()" << endl;

   if (tracks_group_ptr==NULL) return;

   vector<track*> track_ptrs=tracks_group_ptr->get_all_track_ptrs();
//   cout << "track_ptrs.size() = " << track_ptrs.size() << endl;

// As of 5/17/11, we assume first (and only) track within
// tracks_group_ptr corresponds to a ground mover:

   track* ground_track_ptr=track_ptrs[0];
//   cout << "ground_track_ptr = " << ground_track_ptr << endl;
//   cout << "*ground_track_ptr = " << *ground_track_ptr << endl;

   SignPost* SignPost_ptr=generate_new_SignPost();
   SignPost_ptr->set_stationary_Graphical_flag(false);
   string label="Ground mover";
   SignPost_ptr->set_label(label);

//   cout << "AC.get_nframes() = " << get_AnimationController_ptr()->
//      get_nframes() << endl;
//   cout << "AC.dt_per_frame() = " << get_AnimationController_ptr()->
//      get_delta_world_time_per_frame() << endl;
   
   threevector curr_posn,curr_velocity;
//   double start_true_t=get_AnimationController_ptr()->
//      get_time_corresponding_to_frame(0);
//   cout << "Ground_track.size() = " << ground_track_ptr->size() << endl;
   
   int curr_framenumber=0;
   for (unsigned int n=0; n<ground_track_ptr->size(); n++)
   {
      double curr_track_t=ground_track_ptr->get_time(n);
      ground_track_ptr->get_posn_velocity(
         curr_track_t,curr_posn,curr_velocity);

//      double curr_true_t=start_true_t+curr_track_t;
//      int curr_framenumber=get_AnimationController_ptr()->
//         get_frame_corresponding_to_time(curr_true_t);

      threevector V1=curr_posn;
      threevector V2=V1+1000*z_hat;
      SignPost_ptr->set_attitude_posn(
         curr_framenumber,get_passnumber(),V1,V2);

//      cout << "curr_track_t = " << curr_track_t
//              << " curr_true_t = " << curr_true_t 
//           << " curr_framenumber = " << curr_framenumber << endl;
//      cout << "V1: x = " << V1.get(0) << " y = " << V1.get(1)
//           << " z = " << V1.get(2) << endl;
//      cout << "V2 = " << V2 << endl;
//      cout << "n = " << n
//           << " Vx = " << curr_velocity.get(0)
//           << " Vy = " << curr_velocity.get(1)
//           << " Vz = " << curr_velocity.get(2)
//           << endl;
      curr_framenumber++;      
   } // loop over index n labeling track positions

   set_colors(colorfunc::white,colorfunc::darkpurple);
}

// ==========================================================================
// SignPost video plane projection member functions
// ==========================================================================

// Member function reset_video_SignPost() takes in the UV coordinates
// for a SignPost cone tip as well as the extremal UV values for the
// associated video plane.  This method rotates a SignPost lying
// within the UV image plane by 0, 90, 180 or 270 degrees wrt its
// canonical downward direction.  The rotated orientation is chosen so
// as to minimize the SignPost's overlap on top of the video image.

void SignPostsGroup::reset_video_SignPost(
   SignPost* SignPost_ptr,const twovector& SignPost_tip_UV,
   double Umin,double Umax,double Vmin,double Vmax,
   bool downward_only_arrows_flag)
{
//   cout << "inside SignPostsGroup::reset_video_SignPost()" << endl;

   if (get_ndims() != 2) return;

   threevector UVW(SignPost_tip_UV);
   SignPost_ptr->set_UVW_coords(get_curr_t(),get_passnumber(),UVW);

//    double delta_LHS=UVW.get(0)-Umin;
//   double delta_RHS=Umax-UVW.get(0);
   double delta_BHS=UVW.get(1)-Vmin;
   double delta_THS=Vmax-UVW.get(1);
  
   vector<double> deltas,SignPost_dirs;

// As of 2/12/09, we do not like the appearance of horizontal SignPost
// annotations appearing on the sides of videos.  So we comment out
// the next two lines:

   if (!downward_only_arrows_flag)
   {
//   deltas.push_back(delta_LHS);
//   SignPost_dirs.push_back(-90*PI/180);

//   deltas.push_back(delta_RHS);
//   SignPost_dirs.push_back(90*PI/180);

      deltas.push_back(delta_BHS);
      SignPost_dirs.push_back(180*PI/180);
   }
   
   deltas.push_back(delta_THS);
   SignPost_dirs.push_back(0*PI/180);
      
   templatefunc::Quicksort(deltas,SignPost_dirs);

// Recall 2D video plane lies in worldspace X-Z plane with Uhat = Xhat
// and Vhat = Zhat.  We therefore rotate SignPosts about the Yhat axis
// in order to change their orientations wrt the video image plane.
// Rotation angle theta = 0, 90, 180, and 270 corresponds to a
// SignPost which points downwards, leftwards, upwards and rightwards:
      
//      cout << "Enter rotation angle theta in degrees:" << endl;
//      cin >> theta;
//      theta *= PI/180;

   threevector rotation_origin(0,0,0);
   double theta=SignPost_dirs[0];
   threevector new_xhat=cos(theta)*x_hat-sin(theta)*z_hat;
   threevector new_yhat=y_hat;
   SignPost_ptr->rotate_about_specified_origin(
      get_curr_t(),get_passnumber(),rotation_origin,new_xhat,new_yhat);
}

// --------------------------------------------------------------------------
// Member function project_SignPosts_into_video_plane() instantiates
// member *imageplane_SignPostsGroup_ptr if it does not already exist
// and generates a 2D imageplane SignPost for every 3D SignPost within
// *this.  It then loops over every 3D SignPost and projects it into
// the 2D imageplane for *Movie_ptr.  If the UV coords for the
// projected SignPost's tip lies inside the movie's allowed UV range,
// this method sets the imageplane SignPosts's UV coordinates so that it 
// appears within the movie's viewport.

bool SignPostsGroup::project_SignPosts_into_video_plane()
{
//   cout << "inside SignPostsGroup::project_SignPosts_into_video_plane()"
//        << endl;

   if (MoviesGroup_ptr==NULL || imageplane_SignPostsGroup_ptr==NULL) 
      return false;

   unsigned int n_3D_SignPosts=get_n_Graphicals();
   if (n_3D_SignPosts==0) return false;

   if (int(get_curr_framenumber()) == prev_SignPost_framenumber) return false;

// When a movie loops to its beginning, we do not want to temporally
// pollute position and velocities from movie's end with those at its
// beginning.  So turn off temporal filtering for first few movie
// frames:

   bool temporally_filter_flag=true;
//      bool temporally_filter_flag=false;
   if (get_curr_framenumber() < 3)
   {
      temporally_filter_flag=false;
   }

// Initialize imageplane SignPosts:
   
   if (imageplane_SignPostsGroup_ptr->get_n_Graphicals()==0)
   {
      for (unsigned int s=0; s<n_3D_SignPosts; s++)
      {
         double curr_size=imageplane_SignPostsGroup_ptr->
            get_common_geometrical_size();
         double height_multiplier=1.0;
         double extra_frac_cyl_height=0.5;

         SignPost* imageplane_SignPost_ptr=imageplane_SignPostsGroup_ptr->
            generate_new_SignPost(curr_size,height_multiplier);
         
         imageplane_SignPost_ptr->set_stationary_Graphical_flag(true);
         imageplane_SignPost_ptr->set_label(
            get_SignPost_ptr(s)->get_label(),extra_frac_cyl_height);
         imageplane_SignPost_ptr->set_max_text_width("0123456789");
         imageplane_SignPost_ptr->change_text_size(0,1.7);
      } // loop over index s labeling 3D SignPosts

      colorfunc::Color permanent_color=colorfunc::red;
      colorfunc::Color selected_color=colorfunc::cream;
      imageplane_SignPostsGroup_ptr->set_colors(
         permanent_color,selected_color);
   } // imageplane_SignPostsGroup_ptr==NULL conditional

   cout.precision(10);
//   cout << "n_3D_SignPosts = " << n_3D_SignPosts << endl;

// As of 2/12/09, we assume that *MoviesGroup_ptr contains just a
// single video clip:

   Movie* Movie_ptr=dynamic_cast<Movie*>(
      MoviesGroup_ptr->get_last_Graphical_ptr());
   camera* camera_ptr=Movie_ptr->get_camera_ptr();

// Loop over every 3D SignPost and project it into the 2D imageplane
// for *Movie_ptr.  If the UV coords for the SignPost's tip lies
// inside the movie's allowed UV range, set UV coordinates for
// corresponding imageplane SignPost.

   for (unsigned int s=0; s<n_3D_SignPosts; s++)
   {
      SignPost* SignPost_ptr=get_SignPost_ptr(s);

      threevector XYZ;
      if (SignPost_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),XYZ))
      {
         double frustum_sidelength;
         genmatrix P(3,4);

         if (!import_package_params(
            get_curr_framenumber(),frustum_sidelength,P)) continue;

//         double alpha=0.01;
         double alpha=0.02;
//         double alpha=0.04;
         camera_ptr->set_projection_matrix(P,alpha,temporally_filter_flag);

         threevector rel_XYZ=XYZ-
            MoviesGroup_ptr->get_static_camera_posn_offset();
         threevector UVW=*(camera_ptr->get_P_ptr()) * fourvector(rel_XYZ,1);
         double U=UVW.get(0)/UVW.get(2);
         double V=UVW.get(1)/UVW.get(2);

// This next line is needed in order to prevent the 3D
// SignPostPickHandler from being activated.  Instead, we just want
// the 2D ImagePlaneSignPostPickHandler to be active in program VIDEO
// for the MIT lobby demo...

         SignPost_ptr->set_mask(get_curr_t(),get_passnumber(),true);

         SignPost* imageplane_SignPost_ptr=imageplane_SignPostsGroup_ptr->
            get_SignPost_ptr(s);

         bool mask_flag=true;
         twovector UV(U,V);
         if (U > Movie_ptr->get_minU() && U < Movie_ptr->get_maxU() &&
             V > Movie_ptr->get_minV() && V < Movie_ptr->get_maxV())
         {
//            cout << "s = " << s 
//                 << " label = " << imageplane_SignPost_ptr->get_label() 
//                 << endl;
//            cout << " U = " << U << " V = " << V << endl;

            imageplane_SignPostsGroup_ptr->reset_video_SignPost(
               imageplane_SignPost_ptr,UV,
               Movie_ptr->get_minU(),Movie_ptr->get_maxU(),
               Movie_ptr->get_minV(),Movie_ptr->get_maxV());
            mask_flag=false;
         }

         imageplane_SignPost_ptr->set_mask(
            get_initial_t(),imageplane_SignPostsGroup_ptr->get_passnumber(),
            mask_flag);

      } // SignPost get_UVW_coords conditional
   } // loop over index s labeling SignPosts

   prev_SignPost_framenumber=get_curr_framenumber();
   return true;
} 

// --------------------------------------------------------------------------
// Member function project_SignPosts_into_selected_photo() instantiates
// member *imageplane_SignPostsGroup_ptr if it does not already exist
// and generates a 2D imageplane SignPost for every 3D SignPost within
// *this.  It then loops over every 3D SignPost and projects it into
// the 2D imageplane for the selected photograph.  If the UV coords for the
// projected SignPost's tip lies inside the movie's allowed UV range,
// this method sets the imageplane SignPosts's UV coordinates so that it 
// appears within the movie's viewport.

bool SignPostsGroup::project_SignPosts_into_selected_photo()
{
//   cout << "inside SignPostsGroup::project_SignPosts_into_selected_photo()"
//        << endl;

   if (MoviesGroup_ptr==NULL || imageplane_SignPostsGroup_ptr==NULL ||
       photogroup_ptr==NULL) return false;
   if (MoviesGroup_ptr->get_DTED_ztwoDarray_ptr()==NULL) return false;

   int photo_ID=photogroup_ptr->get_selected_photo_ID();
   if (photo_ID < 0) photo_ID=0;

   if (photo_ID==prev_photo_ID) return false;
   prev_photo_ID=photo_ID;
   photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(photo_ID);

   unsigned int n_3D_SignPosts=get_n_Graphicals();
   if (n_3D_SignPosts==0) return false;

//   cout.precision(10);
//   cout << "n_3D_SignPosts = " << n_3D_SignPosts << endl;

   if (MoviesGroup_ptr->get_reduced_DTED_ztwoDarray_ptr()==NULL) 
   {
      MoviesGroup_ptr->compute_reduced_DTED_ztwoDarray();
   }

// As of 7/3/09, we assume that *MoviesGroup_ptr contains just a
// single video clip:

   Movie* Movie_ptr=dynamic_cast<Movie*>(
      MoviesGroup_ptr->get_last_Graphical_ptr());
   camera* camera_ptr=photograph_ptr->get_camera_ptr();
//   cout << "*camera_ptr = " << *camera_ptr << endl;

//   cout << "Movie_ptr->get_minU() = " << Movie_ptr->get_minU() << endl;
//   cout << "Movie_ptr->get_maxU() = " << Movie_ptr->get_maxU() << endl;
//   cout << "Movie_ptr->get_minV() = " << Movie_ptr->get_minV() << endl;
//   cout << "Movie_ptr->get_maxV() = " << Movie_ptr->get_maxV() << endl;

// Loop over every 3D SignPost and project it into the 2D imageplane
// for *Movie_ptr.  If the UV coords for the SignPost's tip lies
// inside the movie's allowed UV range, set UV coordinates for
// corresponding imageplane SignPost.

   const double min_Z_ground=0;
   const double ds=1;	// meter
   threevector impact_point;

   vector<int> candidate_SignPost_indices;
   vector<double> candidate_Us,candidate_Vs;
   for (unsigned int s=0; s<n_3D_SignPosts; s++)
   {
      SignPost* SignPost_ptr=get_SignPost_ptr(s);
      threevector SignPost_XYZ,r_hat;
      if (SignPost_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),
                                       SignPost_XYZ))
      {
         threevector UVW=*(camera_ptr->get_P_ptr()) * 
            fourvector(SignPost_XYZ,1);
         double U=UVW.get(0)/UVW.get(2);
         double V=UVW.get(1)/UVW.get(2);

         if (U > Movie_ptr->get_minU() && U < Movie_ptr->get_maxU() &&
             V > Movie_ptr->get_minV() && V < Movie_ptr->get_maxV())
         {
            r_hat=camera_ptr->pixel_ray_direction(U,V);
            double max_raytrace_range=(
               SignPost_XYZ-camera_ptr->get_world_posn()).magnitude();
//            max_raytrace_range -= 25;	// meters probably OK for NYC 1K set
            max_raytrace_range -= 35;	// meters probably OK for NYC 1K set

            bool raytrace_flag=camera_ptr->trace_individual_ray(
               r_hat,min_Z_ground,max_raytrace_range,ds,
               MoviesGroup_ptr->get_reduced_DTED_scale_factor(),
               MoviesGroup_ptr->get_DTED_ztwoDarray_ptr(),
               MoviesGroup_ptr->get_reduced_DTED_ztwoDarray_ptr(),
               impact_point);
//         cout << "U = " << U << " V = " << V 
//              << " raytrace_flag = " << raytrace_flag << endl;

            if (!raytrace_flag)
            {
               candidate_Us.push_back(U);
               candidate_Vs.push_back(V);
               candidate_SignPost_indices.push_back(s);
            }
         }
      } // SignPost get_UVW_coords conditional
   } // loop over index s labeling SignPosts
//   cout << "candidate_Vs.size() = " << candidate_Vs.size() << endl;
       
   templatefunc::Quicksort_descending(
      candidate_Vs,candidate_Us,candidate_SignPost_indices);
//   templatefunc::Quicksort(
//      candidate_Vs,candidate_Us,candidate_SignPost_indices);

// On 7/5/09, we realized that there are repeated entries within the
// edited_landmarks table in the nycity database.  So we perform a
// brute force search over the ordered (U,V) pairs and discard any
// which are too close to their predecessors:

   vector<int> cleaned_candidate_SignPost_indices;
   vector<double> cleaned_candidate_Us,cleaned_candidate_Vs;

   unsigned int SignPost_counter=0;
   unsigned int n_tallest_SignPosts=candidate_Vs.size();
   while (cleaned_candidate_Us.size() < n_tallest_SignPosts &&
          SignPost_counter < candidate_Us.size())
   {
      bool repeated_SignPost_entry_found=false;
      for (unsigned int s=0; s<cleaned_candidate_Us.size(); s++)
      {
         if (nearly_equal(
                cleaned_candidate_Us[s],candidate_Us[SignPost_counter]) &&
             nearly_equal(
                cleaned_candidate_Vs[s],candidate_Vs[SignPost_counter]) )
         {
            repeated_SignPost_entry_found=true;
         }
      } // loop over index s labeling previously instantiated
        // imageplane SignPosts

      if (!repeated_SignPost_entry_found)
      {
         cleaned_candidate_Us.push_back(candidate_Us[SignPost_counter]);
         cleaned_candidate_Vs.push_back(candidate_Vs[SignPost_counter]);
         cleaned_candidate_SignPost_indices.push_back(
            candidate_SignPost_indices[SignPost_counter]);
      }      

      SignPost_counter++;
   } // while loop

// Purge and reinitialize imageplane SignPosts:
   
   imageplane_SignPostsGroup_ptr->destroy_all_SignPosts();
   int n_cleaned_candidates=cleaned_candidate_Us.size();

//   double hue_start=360;    // Start with red
//   double hue_stop=120;	    // Cycle thru green going thru blue

//   if (n_cleaned_candidates > 1)
//   {
//      delta_hue=(hue_stop-hue_start)/(n_cleaned_candidates-1);
//   }

// Reasonable hue parameters for 2009 NYC 1K bundler example:

//   double h=100;
//   double delta_hue=150;

   double height_multiplier=1.0;
   double prev_U=NEGATIVEINFINITY;
   for (unsigned int n=n_cleaned_candidates-1; n>=0; n--)
   {
      twovector UV(cleaned_candidate_Us[n],cleaned_candidate_Vs[n]);

      double curr_size=imageplane_SignPostsGroup_ptr->
         get_common_geometrical_size();
      if (fabs(UV.get(0)-prev_U) < 0.5) height_multiplier += 0.35;
      prev_U=UV.get(0);
      double extra_frac_cyl_height=0;

      SignPost* imageplane_SignPost_ptr=imageplane_SignPostsGroup_ptr->
         generate_new_SignPost(curr_size,height_multiplier);
         
      imageplane_SignPost_ptr->set_stationary_Graphical_flag(true);
      string label=
         get_SignPost_ptr(cleaned_candidate_SignPost_indices[n])->get_label();
      imageplane_SignPost_ptr->set_label(label,extra_frac_cyl_height);
      imageplane_SignPost_ptr->set_max_text_width("0123456");
//         imageplane_SignPost_ptr->set_max_text_width("0123456789");
//         imageplane_SignPost_ptr->change_text_size(0,1.0);
//         imageplane_SignPost_ptr->change_text_size(0,1.7);

      bool downward_only_arrows_flag=true;
      imageplane_SignPostsGroup_ptr->reset_video_SignPost(
         imageplane_SignPost_ptr,UV,
         Movie_ptr->get_minU(),Movie_ptr->get_maxU(),
         Movie_ptr->get_minV(),Movie_ptr->get_maxV(),
         downward_only_arrows_flag);
      imageplane_SignPost_ptr->set_quasirandom_color();

//      cout << "n = " << n
//           << " label = " << label
//           << " UV = " << UV << endl;
      
   } // loop over index n labeling tallest SignPosts

   return true;
} 

// --------------------------------------------------------------------------
// Member function
// project_SignPosts_into_selected_aerial_video_frame() generates a 2D
// imageplane SignPost if it doesn't already exists as a counterpart
// for any 3D SignPost within *this.  It then loops over every 3D
// SignPost and projects it into the 2D imageplane for the selected
// aerial video frame.  If the UV coords for the
// projected SignPost's tip lies inside the movie's allowed UV range,
// this method sets the imageplane SignPosts's UV coordinates so that it 
// appears within the movie's viewport.  Otherwise, the imageplane
// SignPost is masked.

bool SignPostsGroup::project_SignPosts_into_selected_aerial_video_frame()
{
//   cout << "inside SignPostsGroup::project_SignPosts_into_selected_aerial_video_frame()"
//        << endl;

   if (MoviesGroup_ptr==NULL || imageplane_SignPostsGroup_ptr==NULL ||
       photogroup_ptr==NULL) return false;

   int photo_ID=photogroup_ptr->get_selected_photo_ID();
   if (photo_ID < 0) photo_ID=0;
//    cout << "photo_ID = " << photo_ID << endl;

   photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(photo_ID);

   unsigned int n_3D_SignPosts=get_n_Graphicals();
   if (n_3D_SignPosts==0) return false;

// As of 7/3/09, we assume that *MoviesGroup_ptr contains just a
// single video clip:

   Movie* Movie_ptr=dynamic_cast<Movie*>(
      MoviesGroup_ptr->get_last_Graphical_ptr());
   camera* camera_ptr=photograph_ptr->get_camera_ptr();
//   cout << "*camera_ptr = " << *camera_ptr << endl;

//   cout << "Movie_ptr->get_minU() = " << Movie_ptr->get_minU() << endl;
//   cout << "Movie_ptr->get_maxU() = " << Movie_ptr->get_maxU() << endl;
//   cout << "Movie_ptr->get_minV() = " << Movie_ptr->get_minV() << endl;
//   cout << "Movie_ptr->get_maxV() = " << Movie_ptr->get_maxV() << endl;

// Loop over every 3D SignPost and project it into the 2D imageplane
// for *Movie_ptr.  If the UV coords for the SignPost's tip lies
// inside the movie's allowed UV range, set UV coordinates for
// corresponding imageplane SignPost.

   for (unsigned int s=0; s<n_3D_SignPosts; s++)
   {
      SignPost* SignPost_ptr=get_SignPost_ptr(s);
      int SignPost_ID=SignPost_ptr->get_ID();

// Make sure a 2D imageplane SignPost with the same ID corresponds to
// every 3D SignPost:

      SignPost* imageplane_SignPost_ptr=imageplane_SignPostsGroup_ptr->
         get_ID_labeled_SignPost_ptr(SignPost_ID);
      if (imageplane_SignPost_ptr==NULL)
      {
         double curr_size=imageplane_SignPostsGroup_ptr->
            get_common_geometrical_size();
         double height_multiplier=1.0;
         imageplane_SignPost_ptr=imageplane_SignPostsGroup_ptr->
            generate_new_SignPost(curr_size,height_multiplier,SignPost_ID);
         imageplane_SignPost_ptr->set_stationary_Graphical_flag(true);

         imageplane_SignPost_ptr->set_label(SignPost_ptr->get_label());
         imageplane_SignPost_ptr->set_max_text_width("0123456");
         imageplane_SignPost_ptr->set_quasirandom_color();

         imageplane_SignPostsGroup_ptr->mask_Graphical_for_all_times(
            imageplane_SignPost_ptr);

      } // imageplane_SignPost_ptr==NULL conditional

      threevector SignPost_XYZ,r_hat;
      if (SignPost_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),
                                       SignPost_XYZ))
      {
         threevector UVW=*(camera_ptr->get_P_ptr()) * 
            fourvector(SignPost_XYZ,1);
         double U=UVW.get(0)/UVW.get(2);
         double V=UVW.get(1)/UVW.get(2);

         double curr_t=imageplane_SignPostsGroup_ptr->get_curr_t();
         int pass_number=imageplane_SignPostsGroup_ptr->get_passnumber();

         if (U > Movie_ptr->get_minU() && U < Movie_ptr->get_maxU() &&
             V > Movie_ptr->get_minV() && V < Movie_ptr->get_maxV())
         {
//            cout << "U = " << U << " V = " << V << endl;

            twovector UV(U,V);
            bool downward_only_arrows_flag=true;
            imageplane_SignPostsGroup_ptr->reset_video_SignPost(
               imageplane_SignPost_ptr,UV,
               Movie_ptr->get_minU(),Movie_ptr->get_maxU(),
               Movie_ptr->get_minV(),Movie_ptr->get_maxV(),
               downward_only_arrows_flag);
            imageplane_SignPost_ptr->set_mask(curr_t,pass_number,false);
         }
         else
         {
            imageplane_SignPost_ptr->set_mask(curr_t,pass_number,true);
         } // Umin < U < Umax && Vmin < V < Vmax conditional

      } // SignPost get_UVW_coords conditional
   } // loop over index s labeling SignPosts

   return true;
} 

// --------------------------------------------------------------------------
// Member function backproject_selected_photo_SignPosts_into_3D() first
// retrieves the projection matrix corresponding to the current video
// frame number.  It then backprojects every 2D SignPost which has been
// manually picked and finds a corresponding 3D worldspace point in a
// ladar cloud.  A new, stationary 3D SignPost within
// *SignPostsGroup_3D_ptr is instantiated.  The 3D SignPost is
// subsequently reprojected back into the 2D UV image plane for all
// dynamic video frames to guarantee a valid relationship between 3D
// and 2D SignPost tiepoints.  Hopefully, the reprojected UV
// coordinates are close to the initially selected UV coordinates for
// the 2D SignPost.  

bool SignPostsGroup::backproject_selected_photo_SignPosts_into_3D()
{
//   cout << "Inside SignPostsGroup::backproject_selected_photo_SignPosts_into_3D()" 
//        << endl;

   if (photogroup_ptr==NULL) return false;
   if (PointFinder_ptr==NULL) return false;
   if (SignPostsGroup_3D_ptr==NULL) return false;
   if (EarthRegionsGroup_ptr==NULL) return false;
   if (get_ndims() != 2) return false;

   unsigned int n_2D_SignPosts=get_n_Graphicals();
//   cout << "n_2D_SignPosts = " << n_2D_SignPosts << endl;
   if (n_2D_SignPosts==0) return false;

   int photo_ID=photogroup_ptr->get_selected_photo_ID();
   if (photo_ID < 0) photo_ID=0;
   photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(photo_ID);
//   cout << "photo_ID = " << photo_ID << endl;

   camera* camera_ptr=photograph_ptr->get_camera_ptr();
   threevector camera_posn=camera_ptr->get_world_posn();
//   cout << "camera_posn = " << camera_posn << endl;

// Loop over every 2D SignPost and backproject it into the 3D world
// space.

   vector<int> IDs_of_SignPosts_to_delete;
   for (unsigned int f=0; f<n_2D_SignPosts; f++)
   {
      bool continue_flag=false;
      SignPost* SignPost_ptr=get_SignPost_ptr(f);
      SignPost_ptr->set_stationary_Graphical_flag(false);

      SignPost* SignPost_3D_ptr=SignPostsGroup_3D_ptr->
         get_ID_labeled_SignPost_ptr(SignPost_ptr->get_ID());

      if (SignPost_3D_ptr != NULL)
      {

// If a stationary 3D SignPost counterpart to a previously picked 2D
// SignPost exists, project 3D SignPost's XYZ coordinates into current
// UV image plane to determine 2D SignPost's updated coordinates:

         threevector XYZ,UVW;
         SignPost_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),UVW);
         SignPost_3D_ptr->get_UVW_coords(
            SignPostsGroup_3D_ptr->get_curr_t(),
            SignPostsGroup_3D_ptr->get_passnumber(),XYZ);

         threevector ray_hat=(camera_posn-XYZ).unitvector();

         bool SignPost_occluded_flag=false;
         camera_ptr->project_XYZ_to_UV_coordinates(XYZ,UVW);
//         cout << "f = " << f 
//              << " curr_t = " << get_curr_t()
//              << " passnumber = " << get_passnumber()
//              << " XYZ = " << XYZ 
//              << " new projected UVW = " << UVW << endl;
//         cout << "SignPost_occluded_flag = " << SignPost_occluded_flag << endl;
         SignPost_ptr->set_UVW_coords(get_curr_t(),get_passnumber(),UVW);

// Mask any 2D SignPost whose coordinates lie outside photo's allowed
// UV extents or which is occluded:

         bool SignPost_outside_viewframe_flag=false;
            
         if (UVW.get(0) < photograph_ptr->get_minU() ||
             UVW.get(0) > photograph_ptr->get_maxU() ||
             UVW.get(1) < photograph_ptr->get_minV() ||
             UVW.get(1) > photograph_ptr->get_maxV())
         {
//            cout << "SignPost lies outside view frame" << endl;
            SignPost_outside_viewframe_flag=true;
         }
         SignPost_ptr->set_mask(get_curr_t(),get_passnumber(),
                               SignPost_outside_viewframe_flag ||
                               SignPost_occluded_flag);

         continue_flag=true;
      } // SignPost_3D_ptr != NULL conditional
    
      threevector UVW;
      if (!SignPost_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),UVW))
      {
         continue_flag=true;
      }

      if (continue_flag) continue;

      threevector ray_hat=camera_ptr->pixel_ray_direction(twovector(UVW));
//      cout << "f = " << f 
//           << " n_2D_SignPosts = " << n_2D_SignPosts 
//           << " UV = " << twovector(UVW) 
//           << " ray = " << ray_hat << endl;
//      cout << "camera_posn = " << camera_posn << endl;

      threevector closest_worldspace_point;
      geopoint closest_geopoint;
      EarthRegion* EarthRegion_ptr=EarthRegionsGroup_ptr->
         get_EarthRegion_ptr(0);

      if (PointFinder_ptr->find_smallest_relative_angle_world_point(
             camera_posn,ray_hat,closest_worldspace_point))
      {
//         cout << "3D counterpart found" << endl;
         closest_geopoint=geopoint(
            EarthRegion_ptr->get_northern_hemisphere_flag(),
            EarthRegion_ptr->get_UTM_zonenumber(),
            closest_worldspace_point.get(0),
            closest_worldspace_point.get(1),
            closest_worldspace_point.get(2));
      }
      else
      {
//         cout << "No 3D counterpart point located!" << endl;
      }


      SignPost_3D_ptr=SignPostsGroup_3D_ptr->generate_new_SignPost(
         SignPostsGroup_3D_ptr->get_common_geometrical_size() , 
         1.0 , SignPost_ptr->get_ID());

      SignPost_3D_ptr->set_UVW_coords(
         SignPostsGroup_3D_ptr->get_curr_t(),
         SignPostsGroup_3D_ptr->get_passnumber(),closest_worldspace_point);

      SignPost_3D_ptr->set_label(SignPost_ptr->get_label());
      SignPostsGroup_3D_ptr->blink_Geometrical(SignPost_3D_ptr->get_ID());
   
   } // loop over index f labeling 2D SignPosts

   SignPostsGroup_3D_ptr->update_colors();

// If no 3D counterpart to a 2D SignPost can be found, delete 2D
// SignPost from *this:

   for (unsigned int f=0; f<IDs_of_SignPosts_to_delete.size(); f++)
   {
      destroy_SignPost(IDs_of_SignPosts_to_delete[f]);
   }

   return true;
}

// ==========================================================================
// Raytracing member functions
// ==========================================================================

void SignPostsGroup::raytrace_visible_terrain()
{
   if (get_n_Graphicals() != 1) return;

//   cout << "inside SignPostsGroup::raytrace_visible_terrain()" << endl;

//   double max_radius=100;	// meters
//   double max_radius=200;	// meters
//   double max_radius=300;	// meters
//   double max_radius=400;	// meters
   double max_radius=500;	// meters
//   double max_radius=600;	// meters
//   double ds=1.0;	// meter
   double ds=0.75;	// meter
//   double ds=0.5;	// meter

   if (ray_tracer_ptr==NULL) 
   {
      cout << "Error in SignPostsGroup::raytrace_visible_terrain()!" << endl;
      cout << "ray_tracer_ptr = NULL !!" << endl;
      return;
   }
   
   SignPost* SignPost_ptr=get_SignPost_ptr(0);
   threevector SignPost_posn;
   SignPost_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),SignPost_posn);
   if ( (SignPost_posn-prev_SignPost_posn).magnitude() < 1)
   {
      return;
   }
   prev_SignPost_posn=SignPost_posn;
   threevector offset_SignPost_posn=SignPost_posn+threevector(0,0,2);

   int n_total_rays=0;
   int n_occluded_rays=0;
   ray_tracer_ptr->raytrace_circle_around_ground_target(
      offset_SignPost_posn,max_radius,ds,n_total_rays,n_occluded_rays);
   cout << "n_occluded_rays/n_total_rays = "
        << double(n_occluded_rays)/double(n_total_rays) << endl;

//   ray_tracer_ptr->ground_target_line_integrals(
//      offset_SignPost_posn,max_radius,ds);

   display_raytracing_results(ray_tracer_ptr->get_DTED_ptwoDarray_ptr());
}

// ---------------------------------------------------------------------
// Member function display_omni_occlusion()

void SignPostsGroup::display_raytracing_results(twoDarray* ptwoDarray_ptr)
{
   cout << "inside SignPostsGroup::display_raytracing_results()" << endl;

// Automatically reset ColorMap to display probabilities and force
// ColorGeodeVisitor to reload latest p values:

   if (ColorGeodeVisitor_ptr == NULL) return;
   
   ColorGeodeVisitor_ptr->clear_ptwoDarray_ptrs();
   ColorGeodeVisitor_ptr->push_back_ptwoDarray_ptr(ptwoDarray_ptr);
      
   if (PointCloudsGroup_ptr != NULL)
   {
      if (PointCloudsGroup_ptr->get_dependent_coloring_var() != 3)
      {
         PointCloudsGroup_ptr->set_dependent_coloring_var(3);
         PointCloudsGroup_ptr->update_dynamic_Grid_color();
      }
      PointCloudsGroup_ptr->reload_all_colors();
   }
}

