// ==========================================================================
// POLYHEDRAGROUP class member function definitions
// ==========================================================================
// Last modified on 1/24/12; 3/13/12; 4/21/12
// ==========================================================================

#include <iomanip>
#include <set>
#include <string>
#include "osg/osgGraphicals/AnimationController.h"
#include "models/BuildingsGroup.h"
#include "geometry/bounding_box.h"
#include "osg/osgGeometry/Cylinder.h"
#include "general/filefuncs.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "messenger/Messenger.h"
#include "models/ParkingLotsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "geometry/polyhedron.h"
#include "models/RoadsGroup.h"

#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PolyhedraGroup::allocate_member_objects()
{
   Polyhedron_map_ptr=new POLYHEDRON_MAP;
}		       

void PolyhedraGroup::initialize_member_objects()
{
//   cout << "inside PolyhedraGroup::initialize_member_objects()" << endl;

   GraphicalsGroup_name="PolyhedraGroup";
   bbox_sidelength=20;
   bbox_height=5;
   bbox_color_str="white";
   bbox_label_color_str="red";
   altitude_dependent_volume_alphas_flag=false;
   min_alpha_altitude=250*1000;		// meters
   max_alpha_altitude=1500*1000;	// meters
   OFF_subdir="";

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<PolyhedraGroup>(
         this, &PolyhedraGroup::update_display));
}		       

PolyhedraGroup::PolyhedraGroup(Pass* PI_ptr,threevector* GO_ptr,
                               AnimationController* AC_ptr):
   GeometricalsGroup(3,PI_ptr,AC_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

PolyhedraGroup::~PolyhedraGroup()
{
   delete Polyhedron_map_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const PolyhedraGroup& P)
{
   int node_counter=0;
   for (unsigned int n=0; n<P.get_n_Graphicals(); n++)
   {
      Polyhedron* Polyhedron_ptr=P.get_Polyhedron_ptr(n);
      outstream << "Polyhedron node # " << node_counter++ << endl;
      outstream << "Polyhedron = " << *Polyhedron_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Polyhedron creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Polyhedron from all other graphical insertion
// and manipulation methods...

Polyhedron* PolyhedraGroup::generate_new_Polyhedron(
   int ID,int OSGsubPAT_number)
{
//   cout << "inside PolyhedraGroup::generate_new_Polyhedron(#1)" << endl;
   if (ID==-1) ID=get_next_unused_ID();
   Polyhedron* curr_Polyhedron_ptr=new Polyhedron(
      pass_ptr,get_grid_world_origin(),font_refptr.get(),ID);

   initialize_new_Polyhedron(curr_Polyhedron_ptr,OSGsubPAT_number);
   return curr_Polyhedron_ptr;
}

Polyhedron* PolyhedraGroup::generate_new_Polyhedron(
   polyhedron* p_ptr,int ID,int OSGsubPAT_number)
{
//   cout << "inside PolyhedraGroup::generate_new_Polyhedron(#2)" << endl;
//   cout << "polyhedron *p_ptr = " << *p_ptr << endl;
   if (ID==-1) ID=get_next_unused_ID();

   Polyhedron* curr_Polyhedron_ptr=new Polyhedron(
      pass_ptr,get_grid_world_origin(),p_ptr,font_refptr.get(),ID);
   initialize_new_Polyhedron(curr_Polyhedron_ptr,OSGsubPAT_number);

   osg::Geode* geode_ptr=curr_Polyhedron_ptr->generate_drawable_geode();
   curr_Polyhedron_ptr->get_PAT_ptr()->addChild(geode_ptr);

   curr_Polyhedron_ptr->build_current_polyhedron(
      get_curr_t(),get_passnumber());

// Add entry into *Polyhedron_map_ptr linking polyhedron and
// Polyhedron pointers:

   (*Polyhedron_map_ptr)[p_ptr]=curr_Polyhedron_ptr;

   return curr_Polyhedron_ptr;
}

// ---------------------------------------------------------------------
void PolyhedraGroup::initialize_new_Polyhedron(
   Polyhedron* Polyhedron_ptr,int OSGsubPAT_number)
{
//   cout << "inside PolyhedraGroup::initialize_new_Polyhedron()" << endl;
   
   GraphicalsGroup::insert_Graphical_into_list(Polyhedron_ptr);

// Note added on 1/17/07: We should someday write a
// destroy_Polyhedron() method which should explicitly remove the
// following LineSegmentsGroup->OSGGroup_ptr from curr_Polyhedron
// before deleting curr_Polyhedron...

   initialize_Graphical(Polyhedron_ptr);
   Polyhedron_ptr->get_PAT_ptr()->addChild(
      Polyhedron_ptr->get_LineSegmentsGroup_ptr()->get_OSGgroup_ptr());
   Polyhedron_ptr->get_LineSegmentsGroup_ptr()->
      set_AnimationController_ptr(AnimationController_ptr);
   Polyhedron_ptr->get_PAT_ptr()->addChild(
      Polyhedron_ptr->get_PointsGroup_ptr()->get_OSGgroup_ptr());

   insert_graphical_PAT_into_OSGsubPAT(Polyhedron_ptr,OSGsubPAT_number);
}

// --------------------------------------------------------------------------
// Member function generate_bbox() instantiates a 3D bounding box
// based upon the top_left and bottom_right geopoint eastings and
// northings passed in as arguments.  The max [min] height for the
// bbox is assumed to be specified by the altitude of the top_left
// [bottom_right] geopoint.  This method instantiates an OSG
// Polyhedron and attaches it to *PolyhedraGroup_ptr. 

Polyhedron* PolyhedraGroup::generate_bbox(int Polyhedra_subgroup,double alpha)
{
   colorfunc::Color bbox_color=colorfunc::string_to_color(bbox_color_str);
   return generate_bbox(Polyhedra_subgroup,bbox_color,alpha);
}

Polyhedron* PolyhedraGroup::generate_bbox(
   int Polyhedra_subgroup,colorfunc::Color& bbox_color,double alpha)
{
//   cout << "inside PolyhedraGroup::generate_bbox()" << endl;
//   cout << "Polyhedra_subgroup = " << Polyhedra_subgroup << endl;

   double min_X=-0.5*bbox_sidelength;
   double max_X=0.5*bbox_sidelength;
   double min_Y=-0.5*bbox_sidelength;
   double max_Y=0.5*bbox_sidelength;
   return generate_bbox(min_X,max_X,min_Y,max_Y,Polyhedra_subgroup,
                        bbox_color,alpha);
}

Polyhedron* PolyhedraGroup::generate_bbox(
   double min_X,double max_X,double min_Y,double max_Y,
   int Polyhedra_subgroup,colorfunc::Color& bbox_color,double alpha)
{
//   cout << "inside PolyhedraGroup::generate_bbox()" << endl;
//   cout << "Polyhedra_subgroup = " << Polyhedra_subgroup << endl;

   double min_Z=get_grid_world_origin().get(2);
   double max_Z=min_Z+bbox_height;

//   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
//   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
//   cout << "min_Z = " << min_Z << " max_Z = " << max_Z << endl;

   polyhedron P;
   P.generate_box(min_X,max_X,min_Y,max_Y,min_Z,max_Z);
   threevector origin(0.5*(min_X+max_X) , 0.5*(min_Y+max_Y) , min_Z);
   P.set_origin(origin);
//   cout << "polyhedron P = " << P << endl;

   Polyhedron* Polyhedron_ptr=
      generate_new_Polyhedron(&P,-1,Polyhedra_subgroup);
   Polyhedron_ptr->set_color(
      colorfunc::get_OSG_color(bbox_color),
      colorfunc::get_OSG_color(bbox_color,alpha));

// Specify text's position relative to local 3D bounding box origin
// (lower right corner) rather than in global world space:

   threevector text_posn(0,0,1.05*(max_Z-min_Z));
//   cout << "text_posn = " << text_posn << endl;
   bool text_screen_axis_alignment_flag=false;

   colorfunc::Color bbox_label_color=
      colorfunc::string_to_color(bbox_label_color_str);
   Polyhedron_ptr->generate_text(0,text_posn,bbox_label_color,
                                 text_screen_axis_alignment_flag);

   string bbox_label="ROI "+stringfunc::number_to_string(get_n_Graphicals());
   Polyhedron_ptr->set_text_label(0,bbox_label);

   double text_size=0.15*(max_X-min_X);
//   cout << "text_size = " << text_size << endl;
   Polyhedron_ptr->set_text_size(0,text_size);

   return Polyhedron_ptr;
}

// ==========================================================================
// Polyhedron selection member functions
// ==========================================================================

// Member function display_selected_Polyhedron_vertex() erases all
// Points representing the currently selected Polyhedron's vertices.
// It then unerases the currently selected vertex within the currently
// selected Polyhedron.

void PolyhedraGroup::display_selected_Polyhedron_vertex()
{
//   cout << "inside PolyhedraGroup::display_selected_Polyhedron_vertex()" 
//        << endl;

   Polyhedron* selected_Polyhedron_ptr=get_selected_Polyhedron_ptr();
   if (selected_Polyhedron_ptr==NULL) return;

   osgGeometry::PointsGroup* PointsGroup_ptr=
      selected_Polyhedron_ptr->get_PointsGroup_ptr();
   PointsGroup_ptr->set_OSGgroup_nodemask(1);

   PointsGroup_ptr->erase_all_Graphicals();
   int selected_Point_ID=PointsGroup_ptr->get_selected_Graphical_ID();  
   PointsGroup_ptr->unerase_Graphical(selected_Point_ID);

   threevector selected_vertex_posn=
      selected_Polyhedron_ptr->get_polyhedron_ptr()->
      get_vertex(selected_Point_ID).get_posn();

   cout << "selected vertex ID = " << selected_Point_ID 
        << " position = " 
        << selected_vertex_posn.get(0) << "   "
        << selected_vertex_posn.get(1) << "   "
        << selected_vertex_posn.get(2) << endl << endl;

   PointsGroup_ptr->reset_colors();
}

// --------------------------------------------------------------------------
// Member function unselect_Polyhedra_vertices() unselects the currently
// selected Polyhedron's selected vertex.  

void PolyhedraGroup::unselect_Polyhedra_vertices()
{
//   cout << "inside PolyhedraGroup::unselect_Polyhedra_vertices()" << endl;

   Polyhedron* selected_Polyhedron_ptr=get_selected_Polyhedron_ptr();
   if (selected_Polyhedron_ptr==NULL) return;
   
   osgGeometry::PointsGroup* PointsGroup_ptr=
      selected_Polyhedron_ptr->get_PointsGroup_ptr();
   PointsGroup_ptr->set_selected_Graphical_ID(-1);
   PointsGroup_ptr->erase_all_Graphicals();
}

// --------------------------------------------------------------------------
// Member function unselect_Polyhedra_edges() unselects the currently
// selected Polyhedron's selected edge.  

void PolyhedraGroup::unselect_Polyhedra_edges()
{
//   cout << "inside PolyhedraGroup::unselect_Polyhedra_edges()" << endl;

   Polyhedron* selected_Polyhedron_ptr=get_selected_Polyhedron_ptr();
   if (selected_Polyhedron_ptr==NULL) return;
   
   LineSegmentsGroup* LineSegmentsGroup_ptr=
      selected_Polyhedron_ptr->get_LineSegmentsGroup_ptr();
   LineSegmentsGroup_ptr->set_selected_Graphical_ID(-1);
   LineSegmentsGroup_ptr->reset_colors();
}

// --------------------------------------------------------------------------
// Member function increment_selected_Polyhedron() returns the ID of
// the newly selected Polyhedron.

int PolyhedraGroup::increment_selected_Polyhedron()
{
//   cout << "inside PolyhedraGroup::increment_selected_Polyhedron()" << endl;
   
   unsigned int n_Polyhedra=get_n_Graphicals();
   if (n_Polyhedra==0) 
   {
      return -1;
   }

   int selected_Polyhedron_ID=get_selected_Graphical_ID();
   selected_Polyhedron_ID=modulo(
      selected_Polyhedron_ID+1,n_Polyhedra);
   set_selected_Graphical_ID(selected_Polyhedron_ID);
   cout << "Selected Polyhedron ID = " 
        << get_selected_Graphical_ID() << endl;
   reset_colors();

   return selected_Polyhedron_ID;
}

// --------------------------------------------------------------------------
// Member function decrement_selected_Polyhedron() returns the ID of
// the newly selected Polyhedron.

int PolyhedraGroup::decrement_selected_Polyhedron()
{
//   cout << "inside PolyhedraGroup::decrement_selected_Polyhedron()" << endl;
   
   unsigned int n_Polyhedra=get_n_Graphicals();
   if (n_Polyhedra==0) 
   {
      return -1;
   }

   int selected_Polyhedron_ID=get_selected_Graphical_ID();
   selected_Polyhedron_ID=modulo(
      selected_Polyhedron_ID-1,n_Polyhedra);
   set_selected_Graphical_ID(selected_Polyhedron_ID);
   cout << "Selected Polyhedron ID = " 
        << get_selected_Graphical_ID() << endl;
   reset_colors();

   return selected_Polyhedron_ID;
}

// ==========================================================================
// Polyhedron importing member functions
// ==========================================================================

// Member function import_new_Polyhedra() reads a set of ascii .off
// files (Object File Format) which are each assumed to contain
// information for one polyhedron.  It instantiates a new Polyhedron
// based upon the input information.

void PolyhedraGroup::import_new_Polyhedra()
{
   if (OFF_subdir.size()==0)
   {
      cout << "Enter subdirectory containing Object File Format (.off) files:"
           << endl;
      cin >> OFF_subdir;
   }

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("off");

   vector<string> off_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,OFF_subdir);
   unsigned int n_polyhedra=off_filenames.size();
   for (unsigned int f=0; f<n_polyhedra; f++)
   {
      cout << "Importing Polyhedron " << f 
           << " of " << n_polyhedra << endl;
      import_new_Polyhedron(off_filenames[f]);
   }
}

// --------------------------------------------------------------------------
Polyhedron* PolyhedraGroup::import_new_Polyhedron(string OFF_filename)
{
//   cout << "inside PolyhedraGroup::import_new_Polyhedron()" << endl;
   
   fourvector polyhedron_color;
   polyhedron* polyhedron_ptr=new polyhedron();
   polyhedron_ptr->read_OFF_file(OFF_filename,polyhedron_color);
   
   Polyhedron* Polyhedron_ptr=generate_new_Polyhedron(polyhedron_ptr);

   Polyhedron_ptr->reset_edges_width(5);

   osg::Vec4 edge_color,volume_color;
   if (polyhedron_color.get(0) < 0)
   {
      edge_color=colorfunc::get_OSG_color(colorfunc::white);
      volume_color=colorfunc::get_OSG_color(colorfunc::grey);
   }
   else
   {
      edge_color=colorfunc::get_OSG_color(colorfunc::black);
      volume_color=osg::Vec4(
         polyhedron_color.get(0),polyhedron_color.get(1),
         polyhedron_color.get(2),polyhedron_color.get(3));
   }

   Polyhedron_ptr->set_color(edge_color,volume_color);
//   Polyhedron_ptr->reset_volume_alpha(0.33);
   Polyhedron_ptr->reset_volume_alpha(0.5);

   return Polyhedron_ptr;
}

// --------------------------------------------------------------------------
// Member function generate_Building_Polyhedra() loops over all
// building polyhedra within input *BuildingsGroup_ptr.  It
// instantiates Polyhedra corresponding to each Building's polyhedra.

void PolyhedraGroup::generate_Building_Polyhedra(
   BuildingsGroup* BuildingsGroup_ptr)
{
   cout << "inside PolyhedraGroup::generate_Building_Polyhedra()" << endl;
   
   BuildingsGroup::BUILDING_POLYHEDRON_MAP* Building_polyhedron_map_ptr=
      BuildingsGroup_ptr->get_Building_polyhedron_map_ptr();

   cout << "bldg_polyhdron_map_ptr->size() = "
        << Building_polyhedron_map_ptr->size() << endl;

   for (BuildingsGroup::BUILDING_POLYHEDRON_MAP::iterator iter=
           Building_polyhedron_map_ptr->begin(); 
        iter != Building_polyhedron_map_ptr->end(); iter++)
   {
      polyhedron* polyhedron_ptr=iter->second;

      cout << "*polyhedron_ptr = " << *polyhedron_ptr << endl;
      
      Polyhedron* Polyhedron_ptr=generate_new_Polyhedron(polyhedron_ptr);
      Polyhedron_ptr->reset_edges_width(5);

      fourvector polyhedron_color(-1,-1,-1,-1);
      
      osg::Vec4 edge_color,volume_color;
      if (polyhedron_color.get(0) < 0)
      {

         edge_color=colorfunc::get_OSG_color(colorfunc::white);

// FAKE FAKE:  Thurs Jun 7, 2012 at 9:22 am
// For viewgraphs only change edges to black:

//         edge_color=colorfunc::get_OSG_color(colorfunc::black);
         volume_color=colorfunc::get_OSG_color(colorfunc::grey);
      }
      else
      {
         edge_color=colorfunc::get_OSG_color(colorfunc::black);
         volume_color=osg::Vec4(
            polyhedron_color.get(0),polyhedron_color.get(1),
            polyhedron_color.get(2),polyhedron_color.get(3));
      }

      Polyhedron_ptr->set_color(edge_color,volume_color);
      Polyhedron_ptr->reset_volume_alpha(0.5);
      
   } // loop over iterator iter
}

// --------------------------------------------------------------------------
// Member function fit_constant_z_ground() loops over all Polyhedra
// within *this.  After reading each Polyhedron's vertices, it stores
// the minimal vertex z value (provided that it lies below a reasonable
// max_ground_z) within an STL vector.  This method returns the
// average of the STL vector's min_z values.

double PolyhedraGroup::fit_constant_z_ground()
{
   cout << "inside PolyhedraGroup::fit_constant_z_ground()" << endl;

   unsigned int n_Polyhedra=get_n_Graphicals();
//   cout << "n_Polyhedra = " << n_Polyhedra << endl;

   vector<double> zmin_values;
   for (unsigned int p=0; p<n_Polyhedra; p++)
   {
      Polyhedron* Polyhedron_ptr=get_Polyhedron_ptr(p);
      polyhedron* polyhedron_ptr=Polyhedron_ptr->get_polyhedron_ptr();
      int n_vertices=polyhedron_ptr->get_n_vertices();

      double zmin=POSITIVEINFINITY;
      for (int v=0; v<n_vertices; v++)
      {
         threevector vertex_posn=polyhedron_ptr->get_vertex(v).get_posn();
         zmin=basic_math::min(zmin,vertex_posn.get(2));
      } // loop over index v labeling polyhedron vertices

      const double max_ground_z=4;	// meters
      if (zmin < max_ground_z)
      {
         zmin_values.push_back(zmin);
      }
   } // loop over index p labeling Polyhedra
   
   std::sort(zmin_values.begin(),zmin_values.end());
   
   for (unsigned int p=0; p<zmin_values.size(); p++)
   {
      cout << "p = " << p << " zmin = " << zmin_values[p] << endl;
   }
   double mu_zmin=mathfunc::mean(zmin_values);
   double sigma_zmin=mathfunc::std_dev(zmin_values);
   double median_zmin=mathfunc::median_value(zmin_values);
   
   cout << "zmin = " << mu_zmin << " +/- " << sigma_zmin << endl;
   cout << "median zmin = " << median_zmin << endl;

   return mu_zmin;
}

// ==========================================================================
// Polyhedron destruction member functions
// ==========================================================================

void PolyhedraGroup::destroy_all_Polyhedra()
{
//   cout << "inside PolyhedraGroup::destroy_all_Polyhedra()" << endl;
   unsigned int n_Polyhedra=get_n_Graphicals();
//   cout << "n_Polyhedra = " << n_Polyhedra << endl;

   vector<Polyhedron*> Polyhedra_to_destroy;
   for (unsigned int p=0; p<n_Polyhedra; p++)
   {
      Polyhedron* Polyhedron_ptr=get_Polyhedron_ptr(p);
//      cout << "p = " << p << " Polyhedron_ptr = " << Polyhedron_ptr << endl;
      Polyhedra_to_destroy.push_back(Polyhedron_ptr);
   }

   for (unsigned int p=0; p<n_Polyhedra; p++)
   {
      destroy_Polyhedron(Polyhedra_to_destroy[p]);
   }
}

bool PolyhedraGroup::destroy_Polyhedron()
{   
//   cout << "inside PolyhedraGruop::destroy_Polyhedron()" << endl;
   return destroy_Polyhedron(get_selected_Graphical_ID());
}

bool PolyhedraGroup::destroy_Polyhedron(int ID)
{
   if (ID >= 0)
   {
      return destroy_Polyhedron(get_ID_labeled_Polyhedron_ptr(ID));
   }
   else
   {
      return false;
   }
}

bool PolyhedraGroup::destroy_Polyhedron(Polyhedron* curr_Polyhedron_ptr)
{
   Cylinder* Cylinder_ptr=curr_Polyhedron_ptr->get_Cylinder_ptr();
   if (Cylinder_ptr != NULL)
   {
      Cylinder_ptr->set_Polyhedron_ptr(NULL);
   }
   bool flag=destroy_Graphical(curr_Polyhedron_ptr);
   return flag;
}

// --------------------------------------------------------------------------
// Member function update_display()

void PolyhedraGroup::update_display()
{
//   cout << "******************************************************" << endl;
//   parse_latest_messages();

   if (altitude_dependent_volume_alphas_flag) adjust_Polyhedra_alphas();

// Note added on 7/11/11: We should be able to reset_colors() at this
// point in order to enable Polyhedra blinking.  But doing so
// massively messes up coloring for non-blinking Polyhedra.  So we
// instead live with the ugly hack of copying the blinking part of
// GeometricalsGroup::reset_colors() here:

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Geometrical* Geometrical_ptr=get_Geometrical_ptr(n);

//      if (Geometrical_ptr->get_name()=="Polyhedron")
//      {         
//         cout << "n = " << n 
//              << " n_Graphicals = " << get_n_Graphicals() 
//              << " Geometrical_ptr = " << Geometrical_ptr
//              << " Graphical name = " << Geometrical_ptr->get_name()
//              << endl;
//         cout << "GeometricalsGroup this = " << this << endl;
//         cout << "GeometricalsGroup_name = " << get_name() << endl;
//         cout << "Geometrical_ptr->get_blinking_flag() = "
//              << Geometrical_ptr->get_blinking_flag() << endl;
//         cout << "Geometrical_ptr->get_permanent_color() = " << endl;
//         osgfunc::print_Vec4(Geometrical_ptr->get_permanent_color());
//      }

      if (Geometrical_ptr->get_blinking_flag())
      {
         if (Geometrical_ptr->get_multicolor_flag())
         {
            if (Geometrical_ptr->
                time_to_switch_multicolors_to_blinking_color())
            {
               Geometrical_ptr->set_color(
                  Geometrical_ptr->get_blinking_color());
            }
            else
            {
               Geometrical_ptr->set_colors(
                  Geometrical_ptr->get_local_colors());
            }
         }
         else
         {
            Geometrical_ptr->set_color(
               Geometrical_ptr->get_curr_blinking_color());
         }
      }
   } // loop over index n labeling Polyhedra Graphicals
   
   GraphicalsGroup::update_display();
}

// ==========================================================================
// Polyhedra display member functions
// ==========================================================================

// Member function adjust_Polyhedra_alphas() varies the alpha-blending
// of Polyhedra as a function of virtual camera altitude.

void PolyhedraGroup::adjust_Polyhedra_alphas()
{   
//   cout << "inside PolyhedraGroup::adjust_Polyhedra_alphas()" << endl;
   if (get_CM_refptr().valid())
   {
      for (unsigned int r=0; r<get_n_Graphicals(); r++)
      {
         Polyhedron* Polyhedron_ptr=get_Polyhedron_ptr(r);
         set_altitude_dependent_Polyhedron_alpha(Polyhedron_ptr);
      }
   } // CM_refptr.valid() conditional
}

// --------------------------------------------------------------------------
void PolyhedraGroup::set_altitude_dependent_Polyhedron_alpha(
   Polyhedron* Polyhedron_ptr)
{
//   cout << "inside PolyhedraGroup::set_altitude_dependent_Polyhedron_alpha()" 
//        << endl;

   const double alpha_max=0.5;

   double alpha=compute_altitude_dependent_alpha(
      min_alpha_altitude,max_alpha_altitude,alpha_max);
   Polyhedron_ptr->reset_volume_alpha(alpha);
   Polyhedron_ptr->reset_edges_width(3.0*(1-alpha));
}

// ==========================================================================
// Message handling member functions
// ==========================================================================

bool PolyhedraGroup::parse_next_message_in_queue(message& curr_message)
{
   cout << "inside PolyhedraGroup::parse_next_message_in_queue()" << endl;
//   cout << "curr_message.get_text_message() = "
//        << curr_message.get_text_message() << endl;

   bool message_handled_flag=false;
   return message_handled_flag;
}

// --------------------------------------------------------------------------
// Member function broadcast_bbox_corners() retrieves the currently
// selected Polyhedron which we assume is a bounding box.  It extracts
// the bbox's central position as well as the relative positions of
// its 4 (upper) corners.  This method broadcasts the selected bbox's
// ID as well as the XY coordinates of its 4 corners.

void PolyhedraGroup::broadcast_bbox_corners()
{
   cout << "inside PolyhedraGroup::broadcast_bbox_corners()" << endl;
   cout << "Get_Messenger_ptr() = " << get_Messenger_ptr() << endl;
   if (get_Messenger_ptr()==NULL) return;

   Polyhedron* selected_Polyhedron_ptr=dynamic_cast<Polyhedron*>(
      get_selected_Graphical_ptr());
   if (selected_Polyhedron_ptr==NULL) return;

   polyhedron* polyhedron_ptr=selected_Polyhedron_ptr->get_polyhedron_ptr();
   
//   cout << "selected_Polyhedron_ptr->get_ID() = "
//        << selected_Polyhedron_ptr->get_ID() << endl;
//   cout << "*selected_Polyhedron_ptr = " << *selected_Polyhedron_ptr
//        << endl;

   threevector Polyhedron_posn;
   if (selected_Polyhedron_ptr->get_UVW_coords(
      get_curr_t(),get_passnumber(),Polyhedron_posn))
   {         
//      cout << "Polyhedron_posn = " << Polyhedron_posn << endl;
   }

   string command="UPDATE_ROI";
   string key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   key="ID";
   value=stringfunc::number_to_string(selected_Polyhedron_ptr->get_ID());
   properties.push_back(property(key,value));

   for (int c=0; c<4; c++)
   {
      threevector curr_vertex_posn=polyhedron_ptr->get_vertex(c).get_posn();
      threevector corner_posn=Polyhedron_posn+curr_vertex_posn;
//      cout << "c = " << c 
//           << " vertex posn = " << curr_vertex_posn 
//           << " corner posn = " << corner_posn 
//           << endl;

      key="ROI corner "+stringfunc::number_to_string(c);
      value=stringfunc::number_to_string(corner_posn.get(0))+" "
         +stringfunc::number_to_string(corner_posn.get(1));
      properties.push_back(property(key,value));
   }

   get_Messenger_ptr()->broadcast_subpacket(command,properties);
} 

// ==========================================================================
// NYC demo member functions
// ==========================================================================

void PolyhedraGroup::generate_skyscraper_bbox(
   polyhedron& bbox_3D,colorfunc::Color box_color,double alpha)
{
//   cout << "inside PolyhedraGroup::generate_skyscraper_bbox()" << endl;
   Polyhedron* Polyhedron_ptr=generate_new_Polyhedron(&bbox_3D);
   Polyhedron_ptr->reset_edges_width(5);
                  
   osg::Vec4 edge_color=colorfunc::get_OSG_color(box_color);
   osg::Vec4 volume_color=colorfunc::get_OSG_color(box_color,alpha);
   Polyhedron_ptr->set_color(edge_color,volume_color);
}

// --------------------------------------------------------------------------
// Member function generate_skyscraper_bboxes is a specialized method
// which wraps translucent green and red bounding boxes around the
// Bear Stearns and Tower 49 skyscrapers within the RTV NYC point
// cloud.  The bounding box coordinates are hardwired into this
// method.

void PolyhedraGroup::generate_skyscraper_bboxes()
{
//   cout << "inside PolyhedraGroup::generate_skyscraper_bboxes()" << endl;
   
// Polyhedron P1 is a bounding box around the "Bear Stearns" NYC
// skyscraper nearby Rockefeller Center:

   vector<threevector> vertices;

   vertices.push_back(threevector(4373,7307,25));
   vertices.push_back(threevector(4434.98640649,7274.97368998,25));
   vertices.push_back(threevector(4465.94095691,7327.89904952,25));
   vertices.push_back(threevector(4404.0726366,7360.1272605,25));

   vertices.push_back(threevector(4373,7307,260));
   vertices.push_back(threevector(4434.98640649,7274.97368998,260));
   vertices.push_back(threevector(4465.94095691,7327.89904952,260));
   vertices.push_back(threevector(4404.0726366,7360.1272605,260));

// On 11/5/07, we found that the Bear Stearns and Tower 49 bboxes were
// displaced from their locations surrounding these skyscrapers for
// reasons we don't understand.  So we have to introduce the following
// fudge translation in order to return them to their correct
// positions:

   threevector trans(-632.2,-718.7);

// On 11/15/07, we started working with the new & improved median
// filled NYC point cloud which has a different zmin than the RTV
// cloud from April 2007.  In order to avoid injecting variable grid
// world origin dependence into the skyscraper bboxes locations, we
// hardwire here a reasonable value for the world-origin:

   threevector world_origin(582591.629687,4505517.6125,-7.80999898911);

   polyhedron P1;
   P1.generate_box(vertices);
   P1.absolute_position(world_origin);
   P1.translate(trans);
//   generate_skyscraper_bbox(P1,colorfunc::red);

// "Tower 49" NYC skyscraper nearby Rockefeller center:

   vertices.clear();

   vertices.push_back(threevector(4351,7500,35));
   vertices.push_back(threevector(4399.85948382,7474.57312577,35));
   vertices.push_back(threevector(4431.00280557,7529.50494857,35));
   vertices.push_back(threevector(4382.13771061,7554.92192566,35));
   vertices.push_back(threevector(4351,7500,220));
   vertices.push_back(threevector(4399.85948382,7474.57312577,220));
   vertices.push_back(threevector(4431.00280557,7529.50494857,220));
   vertices.push_back(threevector(4382.13771061,7554.92192566,220));

   polyhedron P2;
   P2.generate_box(vertices);
   P2.absolute_position(world_origin);
   P2.translate(trans);

//   generate_skyscraper_bbox(P2,colorfunc::green);

// Empire State Building:

   vertices.clear();
   vertices.push_back(threevector(585703,4511331,400));
   vertices.push_back(threevector(585668,4511261,400));
   vertices.push_back(threevector(585562,4511320,400));
   vertices.push_back(threevector(585595,4511391,400));
   vertices.push_back(threevector(585703,4511331,17));
   vertices.push_back(threevector(585668,4511261,17));
   vertices.push_back(threevector(585562,4511320,17));
   vertices.push_back(threevector(585595,4511391,17));

//   polyhedron P3;
//   P3.generate_box(vertices);
//   generate_skyscraper_bbox(P3,colorfunc::yellow);


// 1 New York Plaza:

   vertices.clear();
   vertices.push_back(threevector(583417,4506130,2));
   vertices.push_back(threevector(583406,4506179,2));
   vertices.push_back(threevector(583520,4506203,2));
   vertices.push_back(threevector(583526,4506143,2));

   vertices.push_back(threevector(583417,4506130,203));
   vertices.push_back(threevector(583406,4506179,203));
   vertices.push_back(threevector(583520,4506203,203));
   vertices.push_back(threevector(583526,4506143,203));

   polyhedron P4;
   P4.generate_box(vertices);
   generate_skyscraper_bbox(P4,colorfunc::yellow);

// When demo starts up , we initially mask the two skyscrapers'
// bounding boxes.  We can later toggle them on by pressing the
// '3/page down' key on the RHS keypad:

   set_OSGgroup_nodemask(0);
}

// --------------------------------------------------------------------------
// Member function generate_ParkingLot_Polyhedra() loops over all
// parking lot polyhedra within input *ParkingLotsGroup_ptr.  It
// instantiates Polyhedra corresponding to each ParkingLot's polyhedra.

void PolyhedraGroup::generate_ParkingLot_Polyhedra(
   ParkingLotsGroup* ParkingLotsGroup_ptr)
{
//   cout << "inside PolyhedraGroup::generate_ParkingLot_Polyhedra()" << endl;
   
   ParkingLotsGroup::PARKINGLOT_POLYHEDRON_MAP* ParkingLot_polyhedron_map_ptr=
      ParkingLotsGroup_ptr->get_ParkingLot_polyhedron_map_ptr();

   for (ParkingLotsGroup::PARKINGLOT_POLYHEDRON_MAP::iterator iter=
           ParkingLot_polyhedron_map_ptr->begin(); 
        iter != ParkingLot_polyhedron_map_ptr->end(); iter++)
   {
      polyhedron* polyhedron_ptr=iter->second;
      Polyhedron* Polyhedron_ptr=generate_new_Polyhedron(polyhedron_ptr);
      Polyhedron_ptr->reset_edges_width(5);

      fourvector polyhedron_color(-1,-1,-1,-1);
      
      osg::Vec4 edge_color,volume_color;
      if (polyhedron_color.get(0) < 0)
      {
         edge_color=colorfunc::get_OSG_color(colorfunc::pink);
         volume_color=colorfunc::get_OSG_color(colorfunc::purple);
      }
      else
      {
         edge_color=colorfunc::get_OSG_color(colorfunc::black);
         volume_color=osg::Vec4(
            polyhedron_color.get(0),polyhedron_color.get(1),
            polyhedron_color.get(2),polyhedron_color.get(3));
      }

      Polyhedron_ptr->set_color(edge_color,volume_color);
      Polyhedron_ptr->reset_volume_alpha(0.35);
      
   } // loop over iterator iter
}

// --------------------------------------------------------------------------
// Member function generate_Road_Polyhedra() loops over all
// road polyhedra within input *RoadsGroup_ptr.  It
// instantiates Polyhedra corresponding to each Road's polyhedra.

void PolyhedraGroup::generate_Road_Polyhedra(
   RoadsGroup* RoadsGroup_ptr)
{
//   cout << "inside PolyhedraGroup::generate_Road_Polyhedra()" << endl;
   
   RoadsGroup::ROAD_POLYHEDRON_MAP* Road_polyhedron_map_ptr=
      RoadsGroup_ptr->get_Road_polyhedron_map_ptr();

   for (RoadsGroup::ROAD_POLYHEDRON_MAP::iterator iter=
           Road_polyhedron_map_ptr->begin(); 
        iter != Road_polyhedron_map_ptr->end(); iter++)
   {
      polyhedron* polyhedron_ptr=iter->second;
      Polyhedron* Polyhedron_ptr=generate_new_Polyhedron(polyhedron_ptr);
      Polyhedron_ptr->reset_edges_width(5);

      fourvector polyhedron_color(-1,-1,-1,-1);
      
      osg::Vec4 edge_color,volume_color;
      if (polyhedron_color.get(0) < 0)
      {
         edge_color=colorfunc::get_OSG_color(colorfunc::blue);
         volume_color=colorfunc::get_OSG_color(colorfunc::cyan);
      }
      else
      {
         edge_color=colorfunc::get_OSG_color(colorfunc::black);
         volume_color=osg::Vec4(
            polyhedron_color.get(0),polyhedron_color.get(1),
            polyhedron_color.get(2),polyhedron_color.get(3));
      }

      Polyhedron_ptr->set_color(edge_color,volume_color);
      Polyhedron_ptr->reset_volume_alpha(0.35);
      
   } // loop over iterator iter
}
