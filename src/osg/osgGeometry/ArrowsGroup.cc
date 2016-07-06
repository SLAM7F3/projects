// ==========================================================================
// ARROWSGROUP class member function definitions
// ==========================================================================
// Last modified on 11/16/10; 12/4/10; 1/6/11; 4/5/14
// ==========================================================================

#include <iomanip>
#include <map>
#include <osg/Geode>
#include <osgText/Text>
#include "osg/osgGeometry/ArrowsGroup.h"
#include "math/basic_math.h"
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "general/inputfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/polyhedron.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
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

void ArrowsGroup::allocate_member_objects()
{
}		       

void ArrowsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="ArrowsGroup";

   altitude_dependent_size_flag=true;
   selected_colorfunc_color=colorfunc::red;
   permanent_colorfunc_color=colorfunc::white;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<ArrowsGroup>(
         this, &ArrowsGroup::update_display));
}		       

ArrowsGroup::ArrowsGroup(
   const int p_ndims,Pass* PI_ptr,threevector* GO_ptr):
   GeometricalsGroup(p_ndims,PI_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

ArrowsGroup::~ArrowsGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const ArrowsGroup& f)
{
   int node_counter=0;
   cout << "# Arrows = " << f.get_n_Graphicals() << endl;
   for (unsigned int n=0; n<f.get_n_Graphicals(); n++)
   {
      Arrow* Arrow_ptr=f.get_Arrow_ptr(n);
      outstream << "Arrow node # " << node_counter++ << endl;
      outstream << "Arrow = " << *Arrow_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Set & get methods
// ==========================================================================

// Member function set_fixed_label_to_Arrow_ID allows the user to
// specify permanent labels for certain arrows.  ID-fixed label
// information is saved within member STL vector ID_fixed_label_pairs.

void ArrowsGroup::set_fixed_label_to_Arrow_ID(int ID,string fixed_label)
{   
   pair<int,string> p(ID,fixed_label);
   ID_fixed_label_pairs.push_back(p);
}

// --------------------------------------------------------------------------
void ArrowsGroup::set_size(double size)
{
   set_size(size,size);
}

void ArrowsGroup::set_size(double size,double text_size)
{
   GeometricalsGroup::set_size(size,text_size);
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Arrow* Arrow_ptr=get_Arrow_ptr(n);
      Arrow_ptr->set_max_text_width(Arrow_ptr->get_label());
   }
}

// --------------------------------------------------------------------------
void ArrowsGroup::set_max_text_width(double width)
{
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Arrow* Arrow_ptr=get_Arrow_ptr(n);
      Arrow_ptr->get_text_ptr(0)->setMaximumWidth(width);
   }
}

// --------------------------------------------------------------------------
void ArrowsGroup::set_colors(
   colorfunc::Color permanent_color,colorfunc::Color selected_color)
{
   cout << "inside ArrowsGroup::set_colors()" << endl;
//   cout << "permanent_color = " << permanent_color << endl;
   permanent_colorfunc_color=permanent_color;
   selected_colorfunc_color=selected_color;
   update_colors();
}

// --------------------------------------------------------------------------
void ArrowsGroup::update_colors()
{
   cout << "inside ArrowsGroup::update_colors()" << endl;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Arrow* Arrow_ptr=get_Arrow_ptr(n);
      Arrow_ptr->set_color(
         colorfunc::get_OSG_color(permanent_colorfunc_color));
      Arrow_ptr->set_permanent_color(
         colorfunc::get_OSG_color(permanent_colorfunc_color));
      Arrow_ptr->set_selected_color(
         colorfunc::get_OSG_color(selected_colorfunc_color));
   }
}

// ==========================================================================
// Arrow creation member functions
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Arrow from all other graphical insertion
// and manipulation methods...

Arrow* ArrowsGroup::generate_new_Arrow(int ID,unsigned int OSGsubPAT_number)
{
//   cout << "inside ArrowsGroup::generate_new_Arrow()" << endl;
   
   if (ID==-1) ID=get_next_unused_ID();
//   cout << "ID = " << ID << endl;
   Arrow* curr_Arrow_ptr=new Arrow(
      get_ndims(),get_pass_ptr(),get_grid_world_origin_ptr(),ID);
   curr_Arrow_ptr->set_linewidth(2.0);

//   cout << "grid origin = " << get_grid_world_origin() << endl;
   curr_Arrow_ptr->set_reference_origin(get_grid_world_origin());

   initialize_new_Arrow(curr_Arrow_ptr,OSGsubPAT_number);
   return curr_Arrow_ptr;
}

// ---------------------------------------------------------------------
void ArrowsGroup::initialize_new_Arrow(
   Arrow* curr_Arrow_ptr,unsigned int OSGsubPAT_number)
{
//   cout << "inside ArrowsGroup::initialize_new_Arrow" << endl;

   GraphicalsGroup::insert_Graphical_into_list(curr_Arrow_ptr);
   initialize_Graphical(curr_Arrow_ptr);

// Recall that as of Sep 2009, we store relative vertex information
// with respect to an average reference_origin point to avoid floating
// point problems.  So we need to translate the Arrow by its reference
// origin in order to globally position it:

   curr_Arrow_ptr->set_UVW_coords(
      get_curr_t(),get_passnumber(),
      curr_Arrow_ptr->get_reference_origin());

   osg::Geode* geode_ptr=curr_Arrow_ptr->generate_drawable_geode();

//   AutoTransform_ptr->addChild(geode_ptr);
//   curr_Arrow_ptr->get_PAT_ptr()->addChild(AutoTransform_ptr);
   curr_Arrow_ptr->get_PAT_ptr()->addChild(geode_ptr);
   insert_graphical_PAT_into_OSGsubPAT(curr_Arrow_ptr,OSGsubPAT_number);

// Add ConesGroup indicating arrow pointing directions to OSGsubPAT:
      
   ConesGroup* ConesGroup_ptr=curr_Arrow_ptr->get_ConesGroup_ptr();
   if (ConesGroup_ptr != NULL)
   {
      ConesGroup_ptr->set_CM_3D_ptr(get_CM_3D_ptr());
//      AutoTransform_ptr->addChild(ConesGroup_ptr->get_OSGgroup_ptr());
//      insert_OSGgroup_into_OSGsubPAT(
//         AutoTransform_ptr,OSGsubPAT_number);
      insert_OSGgroup_into_OSGsubPAT(
         ConesGroup_ptr->get_OSGgroup_ptr(),OSGsubPAT_number);
   }
}

// ==========================================================================
// Arrow manipulation member functions
// ==========================================================================

// Member function edit_Arrow_label allows the user to change the ID
// number associated with a Arrow.  The new ID number must not
// conflict with any other existing Arrow's ID.  It must also be
// non-negative.  The user enters the replacement ID for a selected
// Arrow within the main console window.  (As of 7/10/05, we are
// unfortunately unable to robustly retrieve user input from the
// Arrow text dialog window...)

void ArrowsGroup::edit_Arrow_label()
{   
   int ID=get_selected_Graphical_ID();
   Arrow* curr_Arrow_ptr=get_ID_labeled_Arrow_ptr(ID);
   
   if (curr_Arrow_ptr != NULL)
   {
      if (assign_fixed_label(ID))
      {
      }
      else if (get_selected_Graphical_ID() != -1)
      {
         cout << endl;
         string label_command="Enter new text label for Arrow:";
         string label=inputfunc::enter_string(label_command);
         curr_Arrow_ptr->set_label(label);
      } // selected_Graphical_ID != -1 conditional
   } // currnode_Arrow != NULL conditional
}

// --------------------------------------------------------------------------
// Member function assign_fixed_label takes in the ID for some
// Arrow.  It checks whether a fixed label corresponding to that
// arrow ID was predefined within member STL vector
// ID_fixed_label_pairs.  If so, the arrow's label is set and this
// boolean method returns true.  We cooked up this little utility on
// 10/30/06 for the DTED shadowing problem where we want the zeroth
// arrow to always be labeled as "Receiver".

bool ArrowsGroup::assign_fixed_label(int curr_ID)
{   
   bool fixed_label_assigned_flag=false;
   for (unsigned int i=0; i<ID_fixed_label_pairs.size(); i++)
   {
      pair<int,string> p=ID_fixed_label_pairs[i];
      if (p.first==curr_ID)
      {
         Arrow* Arrow_ptr=get_ID_labeled_Arrow_ptr(curr_ID);
         Arrow_ptr->set_label(p.second);
         fixed_label_assigned_flag=true;
      }
   } // loop over pairs within ID_fixed_label_pairs STL vector member
   return fixed_label_assigned_flag;
}

/*
// --------------------------------------------------------------------------
// Member function erase_Arrow sets boolean entries within the
// member map coords_erased to true for the current Arrow.  When
// Arrow crosshairs are drawn within
// ArrowsGroup::reassign_PAT_ptrs(), entries within this STL map are
// first checked and their positions are set to large negative values
// to prevent them from appearing within the OSG data window.  Yet the
// Arrow itself continues to exist.

bool ArrowsGroup::erase_Arrow()
{   
   bool Arrow_erased=false;

   Arrow* curr_Arrow_ptr=get_ID_labeled_Arrow_ptr(
      get_selected_Graphical_ID());
   if (curr_Arrow_ptr != NULL)
   {

// Recall that a Arrow exists for all times.  Yet it generally
// appears in images spanning only a finite time interval.  We
// therefore erase Arrow's (U,V,W) coords for all images greater
// than or equal to the current time:
      
      for (int n=get_curr_framenumber(); n<=get_last_framenumber(); n++)
      {

// As of 6/5/05, we simply set the time associated with each image in
// pass #0 equal to its imagenumber.  This will eventually need to be
// generalized so that the time field corresponds to a true temporal
// measurement...

         double curr_t=static_cast<double>(n);
         curr_Arrow_ptr->set_mask(curr_t,get_passnumber(),true);
      }

      cout << "Erased Arrow " << get_selected_Graphical_ID() << endl;
      Arrow_erased=true;
   } // currnode_ptr != NULL conditional

   return Arrow_erased;
}

// --------------------------------------------------------------------------
// Member function unerase_Arrow queries the user to enter the ID
// for some erased Arrow.  It then unerases that Arrow within the
// current image.

bool ArrowsGroup::unerase_Arrow()
{   
   bool Arrow_unerased_flag=false;

   string label_command="Enter Arrow number to unerase in current image:";
   int unerased_Arrow_ID=inputfunc::enter_nonnegative_integer(
      label_command);

   Arrow* curr_Arrow_ptr=get_ID_labeled_Arrow_ptr(
      unerased_Arrow_ID);
   if (curr_Arrow_ptr==NULL)
   {
      cout << "Input label does not correspond to any existing Arrow"
           << endl;
   }
   else
   {
      if (!curr_Arrow_ptr->get_mask(get_curr_t(),get_passnumber()))
      {
         cout << "Arrow already exists in current image" << endl;
      }
      else
      {
         curr_Arrow_ptr->set_mask(
            get_curr_t(),get_passnumber(),false);
         set_selected_Graphical_ID(unerased_Arrow_ID);
         cout << "Unerased Arrow " << unerased_Arrow_ID << endl;
         Arrow_unerased_flag=true;
      }
   } // currnode_ptr==NULL conditional

   reset_colors();
   return Arrow_unerased_flag;
}
*/

// --------------------------------------------------------------------------
// Member function destroy_Arrow deletes the selected Arrow and
// purges its entry from the Postgres "entity" table if the database
// is active.

int ArrowsGroup::destroy_Arrow()
{   
//   cout << "inside ArrowsGroup::destroy_Arrow()" << endl;
   int Arrow_ID=get_selected_Graphical_ID();
   if (destroy_Arrow(get_ID_labeled_Arrow_ptr(Arrow_ID)))
   {
      set_selected_Graphical_ID(-1);
      return Arrow_ID;
   }
   else
   {
      return -1;
   }
}

bool ArrowsGroup::destroy_Arrow(Arrow* curr_Arrow_ptr)
{   
//   cout << "inside ArrowsGroup::destroy_Arrow()" << endl;
//   cout << "Arrow_ID = " << curr_Arrow_ptr->get_ID() << endl;

// Recall that ConesGroup is added as a child to OSGsubPAT.  So we
// must explicitly remove it from the scenegraph before the Arrow is
// destroyed:

   ConesGroup* ConesGroup_ptr=curr_Arrow_ptr->get_ConesGroup_ptr();
   if (ConesGroup_ptr != NULL)
   {
      remove_OSGgroup_from_OSGsubPAT(ConesGroup_ptr->get_OSGgroup_ptr());
   }

   bool destroyed_Arrow_flag=
      GraphicalsGroup::destroy_Graphical(curr_Arrow_ptr);

   return destroyed_Arrow_flag;
}

void ArrowsGroup::destroy_all_Arrows()
{
//   cout << "inside ArrowsGroup::destroy_all_Arrows()" << endl;
//   cout << "this = " << this << endl;
   unsigned int n_Arrows=get_n_Graphicals();
//   cout << "n_Arrows = " << n_Arrows << endl;

   vector<Arrow*> Arrow_ptrs_to_destroy;
   for (unsigned int p=0; p<n_Arrows; p++)
   {
      Arrow* Arrow_ptr=get_Arrow_ptr(p);
      Arrow_ptrs_to_destroy.push_back(Arrow_ptr);
   }

   for (unsigned int p=0; p<n_Arrows; p++)
   {
      destroy_Arrow(Arrow_ptrs_to_destroy[p]);
   }
}

// --------------------------------------------------------------------------
// Member function move_z vertically translates the selected Arrow
// and updates its entry within the Postgres database if the database
// is active.

Arrow* ArrowsGroup::move_z(int sgn)
{   
//   cout << "inside SPG::move_z" << endl;
   
   Arrow* curr_Arrow_ptr=dynamic_cast<Arrow*>(
      GraphicalsGroup::move_z(sgn));
   if (curr_Arrow_ptr != NULL)
   {
      threevector Arrow_posn;
      curr_Arrow_ptr->get_UVW_coords(
         get_curr_t(),get_passnumber(),Arrow_posn);
   }
   
   return curr_Arrow_ptr;
}

// --------------------------------------------------------------------------
void ArrowsGroup::update_display()
{   
//   cout << "inside ArrowsGroup::update_display()" << endl;

   if (altitude_dependent_size_flag) 
   {

/*
// BASEMENT program params:

      const double prefactor=0.1;
      const double min_size=0.01;
      const double max_size=1.0;
      const double z_min=0.5;
      const double z_max=1000;
*/

// LOST program params:

      const double prefactor=300.0;
//      const double min_size=3;		// ALIRT ladar data
      const double min_size=30;			// Orig LOST value
      const double max_size=3000.0;

//      const double z_min=0.05*1000;	// meters	ALIRT ladar data
      const double z_min=0.5*1000;	// meters	Orig LOST value
      const double z_max=5000*1000;	// meters
      
      for (unsigned int a=0; a<get_n_Graphicals(); a++)
      {
         Arrow* Arrow_ptr=get_Arrow_ptr(a);
         Arrow_ptr->get_ConesGroup_ptr()->set_altitude_dependent_size(
            prefactor,max_size,min_size,z_min,z_max);
      }
   }
   
   GraphicalsGroup::update_display();
}

// ==========================================================================
// Vector field generation member functions
// ==========================================================================

// Phase values within *phase_twoDarray_ptr are assumed to be in degrees.

void ArrowsGroup::generate_flow_field(
   twoDarray* magnitude_twoDarray_ptr,twoDarray* phase_twoDarray_ptr,
   double Z_field,double arrowhead_size_prefactor)
{
//   cout << "inside ArrowsGroup::generate_flow_field()" << endl;

   double max_magnitude=magnitude_twoDarray_ptr->maximum_value();
   double cell_size=basic_math::max(magnitude_twoDarray_ptr->get_deltax(),
                        magnitude_twoDarray_ptr->get_deltay());
//   cout << "max_magnitude = " << max_magnitude
//        << " cell_size = " << cell_size << endl;

   if (nearly_equal(max_magnitude,0))
   {
      max_magnitude=1;
   }

   for (unsigned int px=0; px<magnitude_twoDarray_ptr->get_mdim(); px++)
   {
//      cout << px << " " << flush;
      double x=magnitude_twoDarray_ptr->fast_px_to_x(px);

      for (unsigned int py=0; py<magnitude_twoDarray_ptr->get_ndim(); py++)
      {
         double y=magnitude_twoDarray_ptr->fast_py_to_y(py);
         threevector cell_posn(x,y,Z_field);
         
//         cout << "px = " << px << " py = " << py 
//              << " x = " << x << " y = " << y << endl;
         double curr_magnitude=magnitude_twoDarray_ptr->get(px,py);
//         cout << "curr_magnitude = " << curr_magnitude << endl;
         double renorm_magnitude=0.9*curr_magnitude/max_magnitude*cell_size;
//         cout << "renorm_magnitude = " << renorm_magnitude << endl;

         double curr_phase=phase_twoDarray_ptr->get(px,py)*PI/180;  // rads
//         cout << "curr_phase = " << curr_phase*180/PI << endl;
         threevector e_hat(cos(curr_phase),sin(curr_phase),0);

         Arrow* Arrow_ptr=generate_new_Arrow();
         threevector abs_center=get_grid_world_origin()+cell_posn;
//         cout << "renorm_mag = " << renorm_magnitude
//              << " abs_center = " << abs_center 
//              << endl;

         Arrow_ptr->set_magnitude_direction_and_center(
            renorm_magnitude,e_hat,abs_center,arrowhead_size_prefactor);
//         Arrow_ptr->set_color(colorfunc::get_OSG_color(colorfunc::red));
//         Arrow_ptr->set_color(colorfunc::get_OSG_color(colorfunc::purple));
         Arrow_ptr->set_color(colorfunc::get_OSG_color(colorfunc::white));
      } // loop over py index
   } // loop over px index
}

// ---------------------------------------------------------------------
// Member function display_polyhedron_surface_points() generates an
// Arrow vector field representing the input polyhedron's surface
// points and their normals.
 
void ArrowsGroup::display_polyhedron_surface_points(
   polyhedron* polyhedron_ptr)
{
   cout << "inside ArrowsGroup::display_polyhedron_surface_points()" 
        << endl;

   destroy_all_Arrows();
   vector<pair<threevector,threevector> > pnts_normals=
      polyhedron_ptr->get_surfacepnts_normals();
   for (unsigned int i=0; i<pnts_normals.size(); i++)
   {
      threevector XYZ=pnts_normals[i].first;
      threevector n_hat=pnts_normals[i].second;
//      cout << "XYZ = " << XYZ << " n_hat = " << n_hat << endl;
      
      Arrow* Arrow_ptr=generate_new_Arrow();
      double renorm_magnitude=10;
      Arrow_ptr->set_magnitude_direction_and_base(
         renorm_magnitude,n_hat,XYZ);
      Arrow_ptr->set_color(colorfunc::get_OSG_color(colorfunc::red));
//      cout << "*Arrow_ptr = " << *Arrow_ptr << endl;
   }
}
