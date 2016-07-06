// ==========================================================================
// GEOMETRICALSGROUP class member function definitions
// ==========================================================================
// Last modified on 5/17/11; 10/9/11; 10/12/11
// ==========================================================================

#include <iostream>
#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "astro_geo/Clock.h"
#include "math/constant_vectors.h"
#include "astro_geo/Ellipsoid_model.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "general/stringfuncs.h"

#include "osg/osgfuncs.h"
#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void GeometricalsGroup::allocate_member_objects()
{
//   cout << "inside GeometricalsGroup::allocate_member_objs()" << endl;
//   cout << "this = " << this << endl;
   geode_refptr = new osg::Geode();
   PAT_refptr=new osg::PositionAttitudeTransform();

   font_refptr=osgText::readFontFile("fonts/times.ttf");
   if (!font_refptr.valid())
   {
      cout << "Error in GeometricalsGroup::allocate_member_objects()" << endl;
      cout << "font pointer = NULL !" << endl;
   }
}		       

void GeometricalsGroup::initialize_member_objects()
{
   Geometricals_updated_flag=false;
   ladar_height_data_flag=false;
   GraphicalsGroup_name="GeometricalsGroup";
   n_text_messages=0;
   common_geometrical_size=1.0;
   tracks_group_ptr=NULL;
}		       

GeometricalsGroup::GeometricalsGroup(const int p_ndims,Pass* PI_ptr,
                                     threevector* GO_ptr):
   GraphicalsGroup(p_ndims,PI_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

GeometricalsGroup::GeometricalsGroup(
   const int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr,
   threevector* GO_ptr):
   GraphicalsGroup(p_ndims,PI_ptr,AC_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

GeometricalsGroup::GeometricalsGroup(
   const int p_ndims,Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
   threevector* GO_ptr):
   GraphicalsGroup(p_ndims,PI_ptr,clock_ptr,EM_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

GeometricalsGroup::~GeometricalsGroup()
{
}

// ==========================================================================
// CustomManipulator set & get methods
// ==========================================================================

osgGA::CustomManipulator* GeometricalsGroup::get_CM_ptr()
{
   return CM_refptr.get();
}

const osgGA::CustomManipulator* GeometricalsGroup::get_CM_ptr() const
{
   return CM_refptr.get();
}

osg::ref_ptr<osgGA::CustomManipulator>& GeometricalsGroup::get_CM_refptr()
{
   return CM_refptr;
}

const osg::ref_ptr<osgGA::CustomManipulator>& GeometricalsGroup::get_CM_refptr() const
{
   return CM_refptr;
}

void GeometricalsGroup::set_CM_2D_ptr(osgGA::Custom2DManipulator* CM_ptr)
{
   CM_refptr=CM_ptr;
}

osgGA::Custom2DManipulator* GeometricalsGroup::get_CM_2D_ptr()
{
   return dynamic_cast<osgGA::Custom2DManipulator*>(CM_refptr.get());
}

const osgGA::Custom2DManipulator* GeometricalsGroup::get_CM_2D_ptr() const
{
   return dynamic_cast<osgGA::Custom2DManipulator*>(CM_refptr.get());
}

void GeometricalsGroup::set_CM_3D_ptr(osgGA::Custom3DManipulator* CM_ptr)
{
//   cout << "inside GeometricalsGroup::set_CM_3D_ptr()" << endl;
   CM_refptr=CM_ptr;
}

osgGA::Custom3DManipulator* GeometricalsGroup::get_CM_3D_ptr()
{
   return dynamic_cast<osgGA::Custom3DManipulator*>(CM_refptr.get());
}

const osgGA::Custom3DManipulator* GeometricalsGroup::get_CM_3D_ptr() const
{
   return dynamic_cast<osgGA::Custom3DManipulator*>(CM_refptr.get());
}

// ==========================================================================
// Scenegraph node insertion & removal member functions
// ==========================================================================

// Member function attach_bunched_geometries_to_OSGsubPAT works with
// GeometricalsGroup's single geode which is assumed to contain a
// bunch of geometries (e.g. PolyLines representing national borders
// or city streets).  It attaches the single geode to
// GeometricalGroup's single PAT.  It then adds the single PAT to
// GeometricalsGroup's OSGsubPAT labeled by input parameter
// subPAT_number.

void GeometricalsGroup::attach_bunched_geometries_to_OSGsubPAT(
   const threevector& reference_vertex,int subPAT_number)
{
//   cout << "inside GeometricalsGroup::attach_bunched_geoms_to_OSGsubPAT()" 
//        << endl;
//   cout << "geode_ptr = " << get_geode_ptr() << endl;
//   cout << "get_geode_ptr()->getNumDrawables() = "
//        << get_geode_ptr()->getNumDrawables() << endl;

   get_PAT_ptr()->addChild(get_geode_ptr());
   get_PAT_ptr()->setPosition(osg::Vec3d(
      reference_vertex.get(0),
      reference_vertex.get(1),
      reference_vertex.get(2)));

   insert_OSGgroup_into_OSGsubPAT(get_PAT_ptr(),subPAT_number);
}

bool GeometricalsGroup::remove_bunched_geometries_from_OSGsubPAT()
{
   return remove_OSGgroup_from_OSGsubPAT(get_PAT_ptr());
}

// ==========================================================================
// Geometrical appearance alteration methods
// ==========================================================================

// Member function change_size multiplies the size parameter for
// SignPost objects corresponding to the current dimension by input
// parameter factor.

void GeometricalsGroup::change_size(double factor)
{
   change_size(factor,factor);
}

void GeometricalsGroup::change_size(
   double geometrical_factor,double text_scale_factor)
{
   change_size(geometrical_factor,geometrical_factor,geometrical_factor,
               text_scale_factor);
}

void GeometricalsGroup::change_size(
   double geom_X_factor,double geom_Y_factor,double geom_Z_factor,
   double text_scale_factor)
{   
//   cout << "inside GeometricalsGroup::change_size()" << endl;
   
   double geometrical_factor=pow(
      geom_X_factor*geom_Y_factor*geom_Z_factor,0.33333);
   common_geometrical_size *= geometrical_factor;
//   GraphicalsGroup::change_size(geometrical_factor);

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
//      cout << "n = " << n << endl;
      threevector scale;
      if (get_Geometrical_ptr(n)->get_scale(
         get_curr_t(),get_passnumber(),scale))
      {
         scale = threevector(scale.get(0)*geom_X_factor,
                             scale.get(1)*geom_Y_factor,
                             scale.get(2)*geom_Z_factor);
         get_Geometrical_ptr(n)->set_scale(
            get_curr_t(),get_passnumber(),scale);
         get_Geometrical_ptr(n)->set_size(
            geometrical_factor*get_Geometrical_ptr(n)->get_size());

         for (unsigned int m=0; m<get_Geometrical_ptr(n)->
                 get_n_text_messages(); m++)
         {
            get_Geometrical_ptr(n)->change_text_size(
               get_Geometrical_ptr(n)->get_text_ptr(m),text_scale_factor);
         }
      }
   } // loop over index n labeling Geometricals
}

// --------------------------------------------------------------------------
void GeometricalsGroup::set_size(double size)
{
   set_size(size,size);
}

void GeometricalsGroup::set_size(double geometrical_size,double text_size)
{
   set_size(geometrical_size,geometrical_size,geometrical_size,text_size);
}

void GeometricalsGroup::set_size(
   double geom_X_size,double geom_Y_size,double geom_Z_size,double text_size,
   int n_repeats)
{   
//   cout << "inside GeometricalsGroup::set_size()" << endl;
//   cout << "n_repeats = " << n_repeats << endl;
//   cout << "geom_X_size = " << geom_X_size
//        << " geom_Y_size = " << geom_Y_size
//        << " geom_Z_size = " << geom_Z_size << endl;

   double geometrical_size=pow(geom_X_size*geom_Y_size*geom_Z_size,0.33333);

   common_geometrical_size=geometrical_size;
//   cout << "common_geometrical_size = " << common_geometrical_size << endl;

   threevector scale(geom_X_size,geom_Y_size,geom_Z_size);
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      for (int r=0; r<n_repeats; r++)
      {
         get_Geometrical_ptr(n)->set_scale(
            get_curr_t()+r,get_passnumber(),scale);
      }
      
      get_Geometrical_ptr(n)->set_size(geometrical_size);
//      cout << "n = " << n 
//           << " geom_X_size = " << geom_X_size 
//           << " geometrical_size = " << geometrical_size
//           << endl;

      const double default_character_height=15;
//      const double default_character_height=1;
      for (unsigned int m=0; m<get_Geometrical_ptr(n)->get_n_text_messages(); 
           m++)
      {
         get_Geometrical_ptr(n)->set_text_size(
            m,0.2*default_character_height*text_size);
      }
   } // loop over index n labeling Geometricals
}

// --------------------------------------------------------------------------
// Member function compute_altitude_dependent_size() retrieves the
// current altitude of the virtual camera.  If CM_3D_ptr != NULL, it
// computes the size for Geometricals as a linear function of the
// altitude.

double GeometricalsGroup::compute_altitude_dependent_size(
   double zmin,double zmax,double size_min,double size_max)
{
//   cout << "inside GeometricalsGroup::compute_altitude_dependent_size()" 
//        << endl;

   if (!CM_refptr.valid()) return -1;
   double curr_Z=get_CM_3D_ptr()->get_eye_world_posn().get(2);
   double size=size_min;
   if (curr_Z > zmax)
   {
      size=size_max;
   }
   else if (curr_Z < zmin)
   {
      size=size_min;
   }
   else
   {
      size=size_min+(curr_Z-zmin)/(zmax-zmin)*(size_max-size_min);
   }

//   cout << "curr_Z/km  = " << curr_Z/1000 
//        << " size = " << size 
//        << " common_geom_size = " << get_common_geometrical_size() << endl;
   
   return size;
}

// --------------------------------------------------------------------------
// We wrote this overloaded version of
// compute_altitude_dependent_size() in Oct 2009 to meet the LOST
// sponsor's demand that SignPosts become quite small as the user
// zooms in towards them.  So this method takes in 3 altitudes and
// linearly interpolates Geometrical size between the first-second and
// second-third pairs.  It also incorporates a zenith-angle
// amplification factor which grows as the virtual camera's view
// deviates more from nadir.  So SignPost size becomes big enough to
// easily see when the virtual camera is zoomed into the aircraft's
// OBSFRUSTUM.

double GeometricalsGroup::compute_altitude_dependent_size(
   double zmin,double zintermediate,double zmax,
   double size_min,double size_intermediate,double size_max)
{
//   cout << "inside GeometricalsGroup::compute_altitude_dependent_size()" 
//        << endl;

   if (!CM_refptr.valid()) return -1;
   double curr_Z=get_CM_3D_ptr()->get_eye_world_posn().get(2);
//   cout << "curr_Z (km) = " << 0.001*curr_Z << endl;

   threevector point_hat=-get_CM_3D_ptr()->get_camera_Zhat();
//   cout << "point_hat = " << point_hat << endl;
   double dotproduct=-point_hat.get(2);
//   double theta=acos(dotproduct);
   double amp_factor=basic_math::min(1.0/fabs(dotproduct),10.0);
//   cout << "dotproduct = " << dotproduct 
//        << " theta = " << theta*180/PI 
//        << " amp_factor = " << amp_factor << endl;

   double size=size_min;
   if (curr_Z < zmin)
   {
      size=size_min;
   }
   else if (curr_Z >= zmin && curr_Z < zintermediate)
   {
      size=size_min+(curr_Z-zmin)/(zintermediate-zmin)*
         (size_intermediate-size_min);
   }
   else if (curr_Z >= zintermediate && curr_Z < zmax)
   {
      size=size_intermediate+(curr_Z-zintermediate)/(zmax-zintermediate)*
         (size_max-size_intermediate);
   }
   else if (curr_Z >= zmax)
   {
      size=size_max;
   }

   size *= amp_factor;

//   cout << "curr_Z/m  = " << curr_Z
//        << " zmin = " << zmin << " zintermediate = " << zintermediate
//        << " size = " << size 
//        << " common_geom_size = " << get_common_geometrical_size() 
//        << endl;

   return size;
}

// ---------------------------------------------------------------------
double GeometricalsGroup::compute_altitude_dependent_alpha(
   double zmin,double zmax,double alpha_max)
{
//   cout << "Inside GeometricalsGroup::compute_altitude_dependent_alpha()"
//        << endl;
//   cout << "this = " << this 
//        << " GeometricalsGroup_name = " << get_name() << endl;

   if (!CM_refptr.valid())
   {
      cout << "Error in GeometricalsGroup::compute_altitude_dependent_alpha()"
           << endl;
      cout << "CM_refptr.valid() = " << CM_refptr.valid() << endl;
      return 1;
   }
   
   double alpha;
   double curr_Z=get_CM_3D_ptr()->get_eye_world_posn().get(2);
//   cout << "curr_Z = " << curr_Z << " zmin = " << zmin 
//        << " zmax = " << zmax << endl;
   if (curr_Z < zmin)
   {
      alpha=0;
   }
   else if (curr_Z > zmax)
   {
      alpha=alpha_max;
   }
   else
   {
      alpha=alpha_max*(curr_Z-zmin)/(zmax-zmin);
   }
//   cout << "curr_Z = " << curr_Z << " zmin = " << zmin 
//        << " zmax = " << zmax << "alpha = " << alpha << endl;
   return alpha;
}

// ==========================================================================
// Geometrical coloring member functions
// ==========================================================================

// Member function reset_colors loops over all Geometricals.  It
// specially colors the currently selected Graphical.  All others are
// colored according to their permanent colorings.

// Note added on 5/21/08 at 2:30 pm: We have instrumented Geometrical
// objects to have separate permanent_text_color and
// selected_text_color which could be different from the non-text colors
// permanent_color and selected_color.  But we currently do not need
// to take advantage of this greater flexibility...

void GeometricalsGroup::reset_colors()
{   
//   cout << "inside GeometricalGroup::reset_colors()" << endl;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Geometrical* Geometrical_ptr=get_Geometrical_ptr(n);

// FAKE FAKE:  Thurs Aug 11, 2011 at 4:19 pm

//      if (Geometrical_ptr->get_name()=="PolyLine")
//      {
//         cout << "n = " << n 
//              << " Geometrical ID = " << Geometrical_ptr->get_ID() 
//              << " multicolor_flag = " 
//              << Geometrical_ptr->get_multicolor_flag()
//              << endl;
//         set_selected_Graphical_ID(100);
//         cout << "selected_Graphical_ID = " << get_selected_Graphical_ID() 
//              << endl;
//      }
    

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

      if (Geometrical_ptr==NULL) 
      {
         continue;
      }
      else if (Geometrical_ptr->get_blinking_flag())
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
      else if (get_selected_Graphical_ID()==Geometrical_ptr->get_ID())
      {
         osg::Vec4 selected_color=Geometrical_ptr->get_selected_color();
//         cout << "Geometrical name = " << Geometrical_ptr->get_name()
//              << endl;
//         cout << "selected_color = " 
//              << selected_color.r() << "  "
//              << selected_color.g() << "  "
//              << selected_color.b() << endl;
         
         Geometrical_ptr->set_color(selected_color);
         selected_color=osg::Vec4(selected_color.r(),selected_color.g(),
                                  selected_color.b(),1);
         Geometrical_ptr->set_text_color(selected_color);
      }
      else if (!Geometrical_ptr->get_multicolor_flag())
      {
         osg::Vec4 perm_color=Geometrical_ptr->get_permanent_color();
         Geometrical_ptr->set_color(perm_color);
         perm_color=osg::Vec4(perm_color.r(),perm_color.g(),
                              perm_color.b(),1);
         Geometrical_ptr->set_text_color(perm_color);
      }
      else
      {
         if (Geometrical_ptr->get_multicolor_flag())
         {
            Geometrical_ptr->set_colors(
               Geometrical_ptr->get_local_colors());
         }
         else
         {
            Geometrical_ptr->set_color(
               Geometrical_ptr->get_permanent_color());
         }
      }
   } // loop over Geometricals 
}

// --------------------------------------------------------------------------
// Member function reset_text_color

void GeometricalsGroup::reset_text_color(int i,const osg::Vec4& color)
{   
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      get_Geometrical_ptr(n)->set_text_color(i,color);
   } // loop over Geometricals 
}

// --------------------------------------------------------------------------
// Member function blink_Geometrical takes in a Geometrical ID.  It
// then sets the blinking flag and start time for the selected
// Geometrical.

bool GeometricalsGroup::blink_Geometrical(
   int Geometrical_ID,double max_blink_period)
{
//   cout << "inside GeometricalsGroup::blink_Geometrical(), Geometrical_ID = " << Geometrical_ID << endl;
//   cout << "max_blink_period = " << max_blink_period << endl;

   if (Geometrical_ID < 0) return false;

// Initialize all Geometricals' blinking flags to false:
      
   for (unsigned int p=0; p<get_n_Graphicals(); p++)
   {
      get_Geometrical_ptr(p)->set_blinking_flag(false);
   }

   Geometrical* Geometrical_ptr=get_ID_labeled_Geometrical_ptr(
	Geometrical_ID);
   if (Geometrical_ptr==NULL) return false;
   
   Geometrical_ptr->set_blinking_flag(true);
   Geometrical_ptr->set_blinking_start_time(
      timefunc::elapsed_timeofday_time());
   Geometrical_ptr->set_max_blink_period(max_blink_period);	// secs
   return true;
}

bool GeometricalsGroup::blink_Geometricals(
   const vector<int>& Geometrical_IDs,double max_blink_period)
{
//   cout << "inside blink_Geometricals(vector)" << endl;

// Initialize all Geometricals' blinking flags to false:
      
   for (unsigned int p=0; p<get_n_Graphicals(); p++)
   {
      get_Geometrical_ptr(p)->set_blinking_flag(false);
   }

   bool blinking_flag=false;
   for (unsigned int p=0; p<Geometrical_IDs.size(); p++)
   {
      int curr_Geometrical_ID=Geometrical_IDs[p];
      Geometrical* Geometrical_ptr=get_ID_labeled_Geometrical_ptr(
         curr_Geometrical_ID);
      if (Geometrical_ptr != NULL)
      {
         Geometrical_ptr->set_blinking_flag(true);
         Geometrical_ptr->set_blinking_start_time(
            timefunc::elapsed_timeofday_time());
         Geometrical_ptr->set_max_blink_period(max_blink_period);	// secs
         blinking_flag=true;
      }
   }
   return blinking_flag;
}

// --------------------------------------------------------------------------
void GeometricalsGroup::set_colors(
   colorfunc::Color permanent_color,colorfunc::Color selected_color)
{
//   cout << "inside GeometricalsGroup::set_colors()" << endl;
//   cout << "permanent_color = " << permanent_color << endl;
   permanent_colorfunc_color=permanent_color;
   selected_colorfunc_color=selected_color;
   update_colors();
}

// --------------------------------------------------------------------------
void GeometricalsGroup::update_colors()
{
//   cout << "inside GeometricalsGroup::update_colors()" << endl;
   if (get_ndims()==2) return;
   
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Geometrical* Geometrical_ptr=get_Geometrical_ptr(n);
      Geometrical_ptr->set_color(
         colorfunc::get_OSG_color(permanent_colorfunc_color));
      Geometrical_ptr->set_permanent_color(
         colorfunc::get_OSG_color(permanent_colorfunc_color));
      Geometrical_ptr->set_selected_color(
         colorfunc::get_OSG_color(selected_colorfunc_color));
   }
}

// ==========================================================================

// Member function import_package_params() searches the subdirectory
// specified within member string package_subdir for a package whose
// filename ends with the input framenumber.  If it finds such a file,
// this method attempts to extract both frustum_sidelength and 3x4
// projection matrix entries and returns true.  Otherwise, this
// boolean method returns false.

bool GeometricalsGroup::import_package_params(
   int framenumber,double& frustum_sidelength,genmatrix& P)
{
//   cout << "inside GeometricalsGroup::import_package_params()" << endl;

   const int ndigits=4;
   string package_filename=package_subdir+package_filename_prefix+
      stringfunc::integer_to_string(framenumber,ndigits)+".pkg";
//   cout << "package_filename = " << package_filename << endl;
   if (!filefunc::fileexist(package_filename)) return false;

   filefunc::ReadInfile(package_filename);
   unsigned int linenumber=0;

   frustum_sidelength=100;	// meters
   while (filefunc::text_line[linenumber] != "--projection_matrix" &&
          linenumber < filefunc::text_line.size())
   {
      string curr_line=filefunc::text_line[linenumber];
      vector<string> substrings=
         stringfunc::decompose_string_into_substrings(curr_line);
      if (substrings[0]=="--frustum_sidelength")
      {
         frustum_sidelength=stringfunc::string_to_number(
            substrings[1]);
      }
      linenumber++;
   }

   if (linenumber==filefunc::text_line.size())
   {
      return false;
   }

   string proj_matrix_str=filefunc::text_line[linenumber+1]+" ";
   proj_matrix_str += filefunc::text_line[linenumber+2]+" ";
   proj_matrix_str += filefunc::text_line[linenumber+3];
   vector<double> matrix_entries=
      stringfunc::string_to_numbers(proj_matrix_str);

   int entry=0;
   for (int r=0; r<3; r++)
   {
      for (int c=0; c<4; c++)
      {
         P.put(r,c,matrix_entries[entry++]);
      }
   }
//   cout << "P = " << P << endl;

   return true;
}

// ==========================================================================
// Geometrical tracking member functions
// ==========================================================================

// Member function follow_selected_Geometrical() locks the
// TerrainManipulator virtual camera's lateral position to the
// instantaneous X & Y coordinates of the currently selected geometrical.
// The virtual camera's height above the geometrical can be
// dynamically adjusted.

void GeometricalsGroup::follow_selected_Geometrical(
   double min_height_above_Geometrical)
{
//   cout << "inside GeometricalsGroup::follow_selected_Geometrical()" << endl;

// Test lab for centering a particular geometrical and moving map
// underneath it:

   int ID=get_selected_Graphical_ID();
   if (ID < 0) return;
//   cout << "ID = " << ID << endl;
   Geometrical* curr_Geometrical_ptr=get_ID_labeled_Geometrical_ptr(ID);
//   cout << "curr_Geometrical_ptr = " << curr_Geometrical_ptr << endl;

   track* track_ptr=curr_Geometrical_ptr->get_track_ptr();
//   cout << "track_ptr = " << track_ptr << endl;
   if (track_ptr==NULL) return;
   
   threevector geometrical_posn;
   curr_Geometrical_ptr->get_UVW_coords(
      get_curr_t(),get_passnumber(),geometrical_posn);
//   cout << "geometrical_posn = " << geometrical_posn << endl;
//   cout << "CM_3D_ptr = " << get_CM_3D_ptr() << endl;
   
   if (get_CM_3D_ptr() != NULL && !geometrical_posn.nearly_equal(Zero_vector))
   {
      osg::Matrixd M=get_CM_3D_ptr()->getMatrix();
//      osgfunc::print_matrix(M);

      double min_eye_posn_altitude=geometrical_posn.get(2)+
         min_height_above_Geometrical;
      double eye_posn_altitude=basic_math::max(min_eye_posn_altitude,M(3,2));

      osg::Matrixd new_M;
      new_M.set(
         M(0,0) , M(0,1) , M(0,2) , M(0,3) , 
         M(1,0) , M(1,1) , M(1,2) , M(1,3) ,
         M(2,0) , M(2,1) , M(2,2) , M(2,3) ,
         geometrical_posn.get(0) , geometrical_posn.get(1) , 
         eye_posn_altitude, 1 );
      get_CM_3D_ptr()->setMatrices(new_M);
   } // CM_3D_ptr != NULL conditional
}
