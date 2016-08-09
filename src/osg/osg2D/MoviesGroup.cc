// ==========================================================================
// MOVIESGROUP class member function definitions
// ==========================================================================
// Last modified on 4/24/12; 8/6/13; 12/4/13; 8/9/16
// ==========================================================================

#include "osg/osgGraphicals/AnimationController.h"
#include "image/binaryimagefuncs.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "image/compositefuncs.h"
#include "image/drawfuncs.h"
#include "general/filefuncs.h"
#include "geometry/homography.h"
#include "osg/osg2D/MoviesGroup.h"
#include "passes/PassesGroup.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"

#include "osg/osg3D/tdpfuncs.h"
#include "image/TwoDarray.h"

#include "templates/mytemplates.h"
 
using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void MoviesGroup::allocate_member_objects()
{
}		       

void MoviesGroup::initialize_member_objects()
{
   GraphicalsGroup_name="MoviesGroup";
   
   hide_backfaces_flag=false;
   extract_frames_flag=false;
   play_photos_as_video_flag=false;
   aerial_video_frame_flag=false;
   prev_extracted_framenumber=first_framenumber_to_extract=
      last_framenumber_to_extract=-1;
   framenumber_skip=1;
   prev_diag = -1;

   photo_filenames_map_ptr=NULL;
   PointsGroup_ptr=NULL;
   PolygonsGroup_ptr=NULL;
   PolyLinesGroup_ptr=NULL;
   photogroup_ptr=NULL;
   DTED_ztwoDarray_ptr=NULL;
   ray_tracer_ptr=NULL;
   reduced_DTED_ztwoDarray_ptr=NULL;
   reduced_DTED_scale_factor=10;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<MoviesGroup>(
         this, &MoviesGroup::update_display));
}		       

MoviesGroup::MoviesGroup(
   const int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr,
   osgGA::Custom3DManipulator* CM_ptr):
   GraphicalsGroup(p_ndims,PI_ptr,AC_ptr)
{      
   initialize_member_objects();
   allocate_member_objects();
   CM_3D_refptr=CM_ptr;
}		       

MoviesGroup::MoviesGroup(
   const int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr,
   photogroup* pg_ptr):
   GraphicalsGroup(p_ndims,PI_ptr,AC_ptr)
{      
   initialize_member_objects();
   allocate_member_objects();
   photogroup_ptr=pg_ptr;
}		       

MoviesGroup::MoviesGroup(
   const int p_ndims,Pass* PI_ptr,
   osgGeometry::PointsGroup* PG_ptr,osgGeometry::PolygonsGroup* PolyGrp_ptr,
   AnimationController* AC_ptr):
   GraphicalsGroup(p_ndims,PI_ptr,AC_ptr)
{      
   initialize_member_objects();
   allocate_member_objects();
   PointsGroup_ptr=PG_ptr;
   PolygonsGroup_ptr=PolyGrp_ptr;
}		       

MoviesGroup::~MoviesGroup()
{
//   cout << "inside MoviesGroup destructor" << endl;
//   cout << "this = " << this << endl;
   delete photo_filenames_map_ptr;
}

// Next method is an ugly hack added on 1/16/12.  It is only called from 
// LadarServer::insert_simulated_camera().

void MoviesGroup::clear_all_camera_ptrs()
{
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      get_Movie_ptr(n)->clear_camera_ptrs();
   }
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const MoviesGroup& M)
{
   int node_counter=0;
   for (unsigned int n=0; n<M.get_n_Graphicals(); n++)
   {
      Movie* Movie_ptr=M.get_Movie_ptr(n);
      outstream << "Movie node # " << node_counter++ << endl;
      outstream << "Movie = " << *Movie_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Movie creation and destruction methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Movie from all other graphical insertion
// and manipulation methods...

// This first version of generate_new_Movie assumes that only one
// video pass exists within the input PassesGroup.  It instantiates a
// Movie corresponding to just this single video pass...

Movie* MoviesGroup::generate_new_Movie(
   const PassesGroup& passes_group,double alpha,int ID)
{
//   cout << "inside MoviesGroup::generate_new_Movie() #1" << endl;
   string movie_filename=
      passes_group.get_videopass_ptr()->get_first_filename();
//   cout << "movie_filename = " << movie_filename << endl;
   return generate_new_Movie(movie_filename,alpha,ID);
}

Movie* MoviesGroup::generate_new_Movie(
   string movie_filename,double alpha,int ID,
   bool force_empty_movie_construction_flag)
{
//   cout << "inside MoviesGroup::generate_new_Movie() #2" << endl;
//   cout << "movie_filename = " << movie_filename << endl;
//   cout << "this = " << this << endl;
//   cout << "Initially, n_Graphicals() = " << get_n_Graphicals() << endl;

// First check whether video file corresponding to input filename
// actually exists:

   if (!filefunc::fileexist(movie_filename) &&
       !force_empty_movie_construction_flag)
   {
      cout << "No video file found corresponding to movie_filename = "
           << movie_filename << endl;
      return NULL;
   }

   if (ID==-1) ID=get_next_unused_ID();

   Movie* curr_Movie_ptr=NULL;
   if (CM_3D_refptr.valid())
   {
      curr_Movie_ptr=new Movie(
         get_ndims(),movie_filename,ID,alpha,AnimationController_ptr,
         hide_backfaces_flag,CM_3D_refptr.get());
   }
   else
   {
      curr_Movie_ptr=new Movie(
         get_ndims(),movie_filename,ID,alpha,AnimationController_ptr,
         hide_backfaces_flag);
   }
   GraphicalsGroup::insert_Graphical_into_list(curr_Movie_ptr);

   if (force_empty_movie_construction_flag) return curr_Movie_ptr;

// If current movie corresponds to an aerial nadir data set which is
// georegistered, we want to copy the UTM coordinates of its 4 corners
// into the movie's video_corner_UTM_coords member STL vector:

   if (get_pass_ptr() != NULL)
   {
      curr_Movie_ptr->set_video_corner_UTM_coords(
         get_pass_ptr()->get_PassInfo_ptr()->
         get_video_corner_vertices());

//      cout << "curr_Movie_ptr->get_video_corner_UTM_coords() = " << endl;
//      templatefunc::printVector(curr_Movie_ptr->get_video_corner_UTM_coords());
   }
   
// Initialize every video image's PAT to default origin and scaling
// settings:

   threevector origin(0,0,0);
   for (int n=curr_Movie_ptr->get_first_framenumber(); 
        n<=curr_Movie_ptr->get_last_framenumber(); n++)
   {
      double curr_t=static_cast<double>(n);
      curr_Movie_ptr->set_UVW_coords(curr_t,get_passnumber(),origin);
      curr_Movie_ptr->set_UVW_scales(
         curr_t,get_passnumber(),curr_Movie_ptr->get_maxU(),
         curr_Movie_ptr->get_maxV());
   }

// Rotation axes:  x = horizontal, y = into screen, z = vertical

//   osg::Quat q;
//   q.makeRotate(30*PI/180,0,0,2);
//   curr_Movie_ptr->set_quaternion(curr_t,get_passnumber(),q);

// Scale axes:  x = horizontal, y = vertical, z = out of screen

//      curr_t,get_passnumber(),threevector(10,1,1));

// Add current movie's PositionAttitudeTransform pointer to current
// Graphical Group's OSGgroup:

   curr_Movie_ptr->get_PAT_ptr()->addChild(
      reinterpret_cast<osg::Node*>(curr_Movie_ptr->getGeode()));
   insert_graphical_PAT_into_OSGsubPAT(curr_Movie_ptr,0);

   return curr_Movie_ptr;
}

// ---------------------------------------------------------------------
texture_rectangle* MoviesGroup::generate_new_texture_rectangle(
   string movie_filename)
{
//   cout << "inside MoviesGroup::generate_new_texture_rectangle()" << endl;

// First check whether video file corresponding to input filename
// actually exists:

   if (!filefunc::fileexist(movie_filename)) 
   {
      cout << "No video file found corresponding to movie_filename = "
           << movie_filename << endl;
      return NULL;
   }

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
      movie_filename,AnimationController_ptr);

//   cout << "*texture_rectangle_ptr = " << *texture_rectangle_ptr << endl;
   return texture_rectangle_ptr;
}

// ---------------------------------------------------------------------
Movie* MoviesGroup::generate_new_Movie(
   texture_rectangle* texture_rectangle_ptr,double alpha,int ID)
{
//   cout << "inside MoviesGroup::generate_new_Movie(texture_rectangle_ptr)" 
//        << endl;
//   cout << "get_ndims() = " << get_ndims() << endl;

   if (ID==-1) ID=get_next_unused_ID();

   Movie* curr_Movie_ptr;
   if (CM_3D_refptr.valid())
   {
      curr_Movie_ptr=new Movie(
         get_ndims(),texture_rectangle_ptr,ID,alpha,CM_3D_refptr.get());
   }
   else
   {
      curr_Movie_ptr=new Movie(
         get_ndims(),texture_rectangle_ptr,ID,alpha);
   }
   GraphicalsGroup::insert_Graphical_into_list(curr_Movie_ptr);

// If current movie corresponds to an aerial nadir data set which is
// georegistered, we want to copy the UTM coordinates of its 4 corners
// into the movie's video_corner_UTM_coords member STL vector:

   if (get_pass_ptr() != NULL)
   {
      curr_Movie_ptr->set_video_corner_UTM_coords(
         get_pass_ptr()->get_PassInfo_ptr()->
         get_video_corner_vertices());
//      cout << "curr_Movie_ptr->get_video_corner_UTM_coords() = " << endl;
//      templatefunc::printVector(
//         curr_Movie_ptr->get_video_corner_UTM_coords());
   }
   
// Initialize every video image's PAT to default origin and scaling
// settings:

   threevector origin(0,0,0);
   for (int n=curr_Movie_ptr->get_first_framenumber(); 
        n<=curr_Movie_ptr->get_last_framenumber(); n++)
   {
      double curr_t=static_cast<double>(n);
      curr_Movie_ptr->set_UVW_coords(curr_t,get_passnumber(),origin);
      curr_Movie_ptr->set_UVW_scales(
         curr_t,get_passnumber(),curr_Movie_ptr->get_maxU(),
         curr_Movie_ptr->get_maxV());
   }

// Rotation axes:  x = horizontal, y = into screen, z = vertical

//   osg::Quat q;
//   q.makeRotate(30*PI/180,0,0,2);
//   curr_Movie_ptr->set_quaternion(curr_t,get_passnumber(),q);

// Scale axes:  x = horizontal, y = vertical, z = out of screen

//      curr_t,get_passnumber(),threevector(10,1,1));

// Add current movie's PositionAttitudeTransform pointer to current
// Graphical Group's OSGgroup:

   curr_Movie_ptr->get_PAT_ptr()->addChild(
      reinterpret_cast<osg::Node*>(curr_Movie_ptr->getGeode()));
   insert_graphical_PAT_into_OSGsubPAT(curr_Movie_ptr,0);

   return curr_Movie_ptr;
}

// ---------------------------------------------------------------------
// Member function convert_photos_to_movies() loops over all
// photographs within *photogroup_ptr.  It instantiates a Movie object
// for each photo.

void MoviesGroup::convert_photos_to_movies()
{
//   cout << "inside MoviesGroup::convert_photos_to_movies()" << endl;
   
   if (photogroup_ptr==NULL) return;
   for (unsigned int n=0; n<photogroup_ptr->get_n_photos(); n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      generate_new_Movie(photo_ptr->get_filename());
   }
   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
bool MoviesGroup::destroy_Movie(Movie*& Movie_ptr)
{
//   cout << "inside MoviesGroup::destroy_Movie(), Movie_ptr = " << Movie_ptr
//        << endl;
   if (Movie_ptr==NULL) return false;
   bool flag=destroy_Graphical(Movie_ptr);
   Movie_ptr=NULL;

   return flag;
}

// ---------------------------------------------------------------------
void MoviesGroup::destroy_all_Movies()
{
//   cout << "inside MoviesGroup::destroy_all_Movies()" << endl;
//   cout << "this = " << this << endl;
   unsigned int n_Movies=get_n_Graphicals();
//   cout << "n_Movies = " << n_Movies << endl;

   vector<Movie*> Movie_ptrs_to_destroy;
   for (unsigned int p=0; p<n_Movies; p++)
   {
      Movie* Movie_ptr=get_Movie_ptr(p);
      Movie_ptrs_to_destroy.push_back(Movie_ptr);
   }

   for (unsigned int p=0; p<n_Movies; p++)
   {
      destroy_Movie(Movie_ptrs_to_destroy[p]);
   }
}

// ==========================================================================
// Movie manipulation methods
// ==========================================================================

void MoviesGroup::reset_Uscale()
{
   threevector scale;
   display_scale_factors(scale);

   double Uscale;
   cout << "Enter new U-axis scale value:" << endl;
   cin >> Uscale;
   scale.put(0,Uscale);

   reset_scale_factors(scale);
}

void MoviesGroup::reset_Vscale()
{
   threevector scale;
   display_scale_factors(scale);

   double Vscale;
   cout << "Enter new V-axis scale value:" << endl;
   cin >> Vscale;
   scale.put(2,Vscale);

   reset_scale_factors(scale);
}

// ---------------------------------------------------------------------
// Member function display_scale_factors

void MoviesGroup::display_scale_factors(threevector& scale)
{
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Movie* curr_Movie_ptr=get_Movie_ptr(n);
      int ID=curr_Movie_ptr->get_ID();
      for (int i=curr_Movie_ptr->get_first_framenumber(); 
           i<=curr_Movie_ptr->get_last_framenumber(); i++)
      {
         double curr_t=static_cast<double>(i);
         curr_Movie_ptr->get_scale(curr_t,get_passnumber(),scale);
         cout << "Movie ID = " << ID 
              << "    image = " << i 
              << "    U scale = " << scale.get(0)
              << "    V scale = " << scale.get(2) << endl;
      } // loop over index i labeling images within current Movie
   } // loop over index n labeling movies within current MoviesGroup object
}

// ---------------------------------------------------------------------
// Member function reset_scale_factors

void MoviesGroup::reset_scale_factors(const threevector& scale)
{
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Movie* curr_Movie_ptr=get_Movie_ptr(n);
      for (int i=curr_Movie_ptr->get_first_framenumber(); 
           i<=curr_Movie_ptr->get_last_framenumber(); i++)
      {
         double curr_t=static_cast<double>(i);
         curr_Movie_ptr->set_scale(curr_t,get_passnumber(),scale);
      } // loop over index i labeling images within current Movie
   } // loop over index n labeling movies within current MoviesGroup object
}

// ---------------------------------------------------------------------
void MoviesGroup::update_display()
{   
//   cout << "**********************************************" << endl;
//   cout << "inside MoviesGroup::update_display()" << endl;
//   cout << "this  = " << this << endl;
//   cout << "get_n_Graphicals() = " << get_n_Graphicals() << endl;

//   cout << "get_OSGgroup_nodemask() = " << get_OSGgroup_nodemask() << endl;
//   cout << "get_n_OSGsubPATs() = " << get_n_OSGsubPATs() << endl;
//   for (int n=0; n<get_n_OSGsubPATs(); n++)
//   {
//      cout << "n = " << n 
//           << " OSGsubPAT_ptr = " << get_OSGsubPAT_ptr(n) 
//           << " get_OSGsubPAT_nodemask(n) = "
//           << get_OSGsubPAT_nodemask(n) << endl;
//   }
   
//   if (get_OSGgroup_nodemask() < 0) return;

   parse_latest_messages();

   if (extract_frames_flag)
   {
      Movie* Movie_ptr=get_Movie_ptr(0);
      int curr_framenumber=Movie_ptr->get_curr_framenumber();
      if (curr_framenumber > prev_extracted_framenumber+framenumber_skip-1 &&
          curr_framenumber >= first_framenumber_to_extract &&
          curr_framenumber <= last_framenumber_to_extract)
      {
         string output_subdir="./subframes/";
         filefunc::dircreate(output_subdir);

         string suffix=".png";
         string output_filename=output_subdir+"frame_"
            +stringfunc::number_to_string(curr_framenumber)+suffix;
         cout << "output_filename = " << output_filename << endl;
         Movie_ptr->get_texture_rectangle_ptr()->write_curr_frame(
            output_filename);
         prev_extracted_framenumber=curr_framenumber;
      }
   }

//   cout << "get_n_Movies() = " << get_n_Graphicals() << endl;
//   cout << "ndim = " << get_ndims() << endl;

   if (play_photos_as_video_flag)
   {
      reset_photo(AnimationController_ptr->get_curr_framenumber());
   }
   else
   {
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         Movie* curr_Movie_ptr=get_Movie_ptr(n);
         curr_Movie_ptr->get_AnimationController_ptr()->sync_to_master();
         if (curr_Movie_ptr->get_watch_subdirectory().size() > 0)
         {
            curr_Movie_ptr->import_next_to_latest_photo();
         }

         curr_Movie_ptr->display_current_frame();
         double curr_diag = curr_Movie_ptr->get_texture_rectangle_ptr()->
            getDiag();
         if(!nearly_equal(curr_diag, prev_diag))
         {
            cout << "curr_diag = " << curr_diag
                 << " prev_diag = " << prev_diag << endl;
            prev_diag = curr_diag;
         }
      }
   }
   
   if (aerial_video_frame_flag)
   {
      project_PolyLines_into_selected_aerial_video_frame();
   }

   if (photo_filenames_map_ptr != NULL)
   {
      load_current_photos();
   }

   GraphicalsGroup::update_display();
}

// ==========================================================================
// Movie output methods:
// ==========================================================================

// Member function save_to_file dumps the video char* information
// currently stored within memory to a .vid file whose name is
// specified by the user.  We wrote this method to allow for video
// information altered from its input progenitor to be saved into .vid
// files which can be used for later processing or display.

void MoviesGroup::save_to_file()
{
   for (unsigned int g=0; g<get_n_Graphicals(); g++)
   {
      Movie* curr_Movie_ptr=get_Movie_ptr(g);

      string filename="Movie.vid";
      cout << "Enter name of output .vid file for current Movie:" << endl;
      cin >> filename;

      vector<unsigned char*> charstar_vector;
      for (int n=curr_Movie_ptr->get_first_framenumber(); 
           n<=curr_Movie_ptr->get_last_framenumber(); n++)
      {
         unsigned char* curr_data_ptr=
            new unsigned char[curr_Movie_ptr->get_VidFile_ptr()->
                             image_size_in_bytes()];
         curr_Movie_ptr->get_VidFile_ptr()->read_image( n, curr_data_ptr );
         charstar_vector.push_back(curr_data_ptr);
      } // loop over index n labeling current movie's image number
      curr_Movie_ptr->get_texture_rectangle_ptr()->
         write_dotVidfile(filename,charstar_vector);

      for (unsigned int n=0; n<charstar_vector.size(); n++)
      {
         delete [] charstar_vector[n];
      }
      
   } // loop over index g labeling movies
}

// ==========================================================================
// 2D region selection methods
// ==========================================================================

osgGeometry::Polygon* MoviesGroup::generate_convexhull_poly()
{
   Movie* curr_Movie_ptr=get_selected_Movie_ptr();
   polygon* poly_ptr=PointsGroup_ptr->generate_convexhull_poly(
      curr_Movie_ptr->getWidth(),curr_Movie_ptr->getHeight());
//   cout << "*poly_ptr = " << *poly_ptr << endl;   

   osgGeometry::Polygon* Polygon_ptr=
      PolygonsGroup_ptr->generate_new_Polygon(
         poly_ptr->get_vertex(0),*poly_ptr);
   delete poly_ptr;
   return Polygon_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_poly forms a polygon out of user
// instantiated points for the current image number.  The polygon's
// vertex information is appended to a text file for long term
// storage.  The polygon and point vertices are erased for all image
// numbers other than the current one.

osgGeometry::Polygon* MoviesGroup::generate_poly()
{
   polygon* poly_ptr=PointsGroup_ptr->generate_poly();
//   cout << "*poly_ptr = " << *poly_ptr << endl;

   string polygon_filename="polygon_regions.txt";
   ofstream textstream;
   filefunc::appendfile(polygon_filename,textstream);
   textstream << get_curr_framenumber() << " # frame number " << endl;
   poly_ptr->write_to_textstream(textstream);
   filefunc::closefile(polygon_filename,textstream);

   osgGeometry::Polygon* Polygon_ptr=PolygonsGroup_ptr->
      generate_new_Polygon(poly_ptr->get_vertex(0),*poly_ptr);
   delete poly_ptr;

// Erase *Polygon_ptr as well as all vertex Points for all image
// numbers greater (and less) than the current one:

   for (unsigned int n=get_curr_framenumber()+1; 
        n<=get_last_framenumber(); n++)
   {
      double curr_t=static_cast<double>(n);
      Polygon_ptr->set_mask(curr_t,get_passnumber(),true);

      for (unsigned int p=0; p<PointsGroup_ptr->get_n_Graphicals(); p++)
      {
         PointsGroup_ptr->get_Point_ptr(p)->set_mask(
            curr_t,get_passnumber(),true);
      }
   } // loop over index n labeling image numbers

   return Polygon_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_Copley_photo_intersection_polys is a
// specialized method for demo purposes.  We hardwire within this file
// the projections of the ground footprint for the intersection volume
// between the 2 Copley ObsFrusta within the 2 image planes.  Using
// program VIDEO, we can display these two 2D projections as
// alpha-blended polygons superposed on top of the original Copley
// images.  The alpha-blended regions show the 3D overlap between the
// two 2D photos.

osgGeometry::Polygon* MoviesGroup::generate_Copley_photo_intersection_polys()
{
//   cout << "inside MoviesGroup::generate_intersection_polys()" << endl;

   vector<threevector> vertices;

// Copley image #0:

//   vertices.push_back(threevector(0.000500742604626,0.32261826297));
//   vertices.push_back(threevector(0.0783111294501,0.325218584774));
//   vertices.push_back(threevector(0.381049838892,0.509051684992));
//   vertices.push_back(threevector(0.000262843054475,0.687195062834));

// Panoramic image #1:

   vertices.push_back(threevector(0.00101517097076,0.0961978450542));
   vertices.push_back(threevector(-0.00184842125281,-0.00145758194951));
   vertices.push_back(threevector(1.35755600234,-0.00145593488248));
   vertices.push_back(threevector(1.34150903316,0.599097701289));

   polygon poly(vertices);

   osgGeometry::Polygon* Polygon_ptr=
      PolygonsGroup_ptr->generate_new_Polygon(vertices[0],poly);
   return Polygon_ptr;
}

// ---------------------------------------------------------------------
void MoviesGroup::null_region_outside_poly()
{
   cout << "inside MG::null_region_outside_poly()" << endl;
   
   Movie* curr_Movie_ptr=get_selected_Movie_ptr();
   twoDarray* ztwoDarray_ptr=new twoDarray(
      curr_Movie_ptr->getWidth(),curr_Movie_ptr->getHeight());
   ztwoDarray_ptr->init_coord_system(
      curr_Movie_ptr->get_minU(),curr_Movie_ptr->get_maxU(),
      curr_Movie_ptr->get_minV(),curr_Movie_ptr->get_maxV());
   ztwoDarray_ptr->clear_values();

   polygon* poly_ptr=PolygonsGroup_ptr->get_Polygon_ptr(0)->
      get_relative_poly_ptr();

   cout << "Coloring polygon interior" << endl;
   const double intensity_value=1.0;
   drawfunc::color_polygon_interior(
      *poly_ptr,intensity_value,ztwoDarray_ptr);

   cout << "Resetting pixel RGB values" << endl;
   int counter=0;

   int R=100;
   int G=100;
   int B=0;
   for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
      {
         if (nearly_equal(ztwoDarray_ptr->get(px,py),0))
         {
            curr_Movie_ptr->set_pixel_RGB_values(px,py,R,G,B);
            counter++;
         }
      }
   }
   cout << "Number of nulled pixels = " << counter << endl;

   curr_Movie_ptr->set_image();

   delete ztwoDarray_ptr;
   update_display();
}

// ---------------------------------------------------------------------
void MoviesGroup::move_z(double delta_z,int Movie_ID)
{
//   cout << "inside MoviesGroup::move_z()" << endl;

   set_selected_Graphical_ID(Movie_ID);

   int curr_framenumber=get_curr_framenumber();
   AnimationController_ptr->set_curr_framenumber(0);
   GraphicalsGroup::move_z(delta_z);
   AnimationController_ptr->set_curr_framenumber(curr_framenumber);

//   cout << "get_selected_Graphical_z() = "
//        << get_selected_Graphical_z() << endl;
}

// ---------------------------------------------------------------------
// Member function get_total_altitude first retrives the input Movie's
// z value relative to its starting position.  It next computes the
// average of the Movie's video corner vertices (which we assume were
// specified in some input package file).  The sum of the global and
// relative z values is returned.

double MoviesGroup::get_total_altitude(int Movie_ID)
{
//   cout << "inside MoviesGroup::get_total_altitude()" << endl;
   
   if (get_n_Graphicals()==0) return NEGATIVEINFINITY;

   set_selected_Graphical_ID(Movie_ID);
   double z_relative=get_selected_Graphical_z();

   double z_global=0;
   vector<threevector> video_corner_vertices=
      get_pass_ptr()->get_PassInfo_ptr()->get_video_corner_vertices();
   for (unsigned int v=0; v<video_corner_vertices.size(); v++)
   {
      z_global += video_corner_vertices[v].get(2);
   }
   z_global /= video_corner_vertices.size();

//   cout << "z_global = " << z_global 
//        << " z_relative = " << z_relative << endl;
   return z_global+z_relative;
}

// ==========================================================================
// Message handling member functions
// ==========================================================================

bool MoviesGroup::parse_next_message_in_queue(message& curr_message)
{
//   cout << "inside MoviesGroup::parse_next_message_in_queue()" << endl;
//   cout << "curr_message.get_text_message() = "
//        << curr_message.get_text_message() << endl;

   bool message_handled_flag=false;
   if (curr_message.get_text_message()=="SELECT_VERTEX")
   {
//      cout << "Received SELECT_VERTEX message from ActiveMQ" << endl;
      curr_message.extract_and_store_property_keys_and_values();
      string type=curr_message.get_property_value("TYPE");
//      cout << "type = " << type << endl;

      if (type=="PHOTO")
      {
         string ID_str=curr_message.get_property_value("ID");  
         int ID=stringfunc::string_to_number(ID_str);
//         cout << "ID = " << ID << endl;
         reset_photo(ID);
         message_handled_flag=true;
      }

   } // curr_message.get_text_message() conditional

   return message_handled_flag;
}

// ==========================================================================
// Photo handling member functions
// ==========================================================================

void MoviesGroup::reset_photo(int ID)
{
//   cout << "inside MoviesGroup::reset_photo(), ID = " << ID << endl;

   if (photogroup_ptr == NULL) return;
   if (photogroup_ptr->get_selected_photo_ID()==ID) return;
   
   photogroup_ptr->set_selected_photo_ID(ID);
   photograph* selected_photo_ptr=photogroup_ptr->get_photograph_ptr(ID);
   if (selected_photo_ptr==NULL) return;

   string photo_filename=selected_photo_ptr->get_filename();
//   cout << "photo_filename = " << photo_filename << endl;

   Movie* Movie_ptr=get_Movie_ptr(0);

// As of 7/5/09, we assume that MoviesGroup corresponds to 2D photos
// rather than 3D OBSFRUSTA...

   bool twoD_flag=true;
   Movie_ptr->reset_displayed_photograph(photo_filename,twoD_flag);
}

// ---------------------------------------------------------------------
bool MoviesGroup::read_future_photo(bool prev_number_flag)
{
//   cout << "inside MoviesGroup::read_future_photo()" << endl;
   Movie* Movie_ptr=get_Movie_ptr(0);
   string curr_photo_filename=Movie_ptr->get_video_filename();

   string future_photo_filename=get_future_photo_filename(
      curr_photo_filename,prev_number_flag);
   if (!filefunc::fileexist(future_photo_filename)) return false;

   bool twoD_flag=true;
   Movie_ptr->reset_displayed_photograph(future_photo_filename,twoD_flag);
   return true;
}

// ---------------------------------------------------------------------
// Member function get_future_photo_filename() takes in the current
// photo filename which is assumed to be of the form XXXX-NNNN.suffix . 
// If input bool prev_number_flag==true [false], this method returns a
// new filename where the index number is decreased [increased] by
// one.

string MoviesGroup::get_future_photo_filename(
   string photo_filename,bool prev_number_flag)
{
//   cout << "inside MoviesGroup::get_futurephoto_filename()" << endl;
   vector<string> substrings=stringfunc::decompose_string_into_substrings(
      photo_filename,"-.");
   unsigned int n_digits=substrings[1].size();
//   cout << "n_digits = " << n_digits << endl;
   int photo_number=stringfunc::string_to_number(substrings[1]);
//   cout << "photo_number = " << photo_number << endl;

   int future_photo_number;
   if (prev_number_flag)
   {
      future_photo_number=photo_number-1;
   }
   else
   {
      future_photo_number=photo_number+1;
   }
//   cout << "future_photo_number = " << future_photo_number << endl;
   string future_photo_number_str=stringfunc::integer_to_string(
      future_photo_number,n_digits);
//   cout << "future_photo_number_str = " << future_photo_number_str << endl;
   string future_photo_filename=substrings[0]+"-"+future_photo_number_str+"."+
      substrings[2];
//   cout << "future_photo_filename = " << future_photo_filename << endl;
   return future_photo_filename;
}

// ---------------------------------------------------------------------
// Member function insert_photo_filename_into_map() takes in a Movie
// ID, frame number and photo filename.  It inserts these correlated
// data within STL map member *photo_filenames_map_ptr.

void MoviesGroup::insert_photo_filename_into_map(
   int Movie_ID,int frame_number,string photo_filename)
{
   if (photo_filenames_map_ptr==NULL)
   {
      photo_filenames_map_ptr=new PHOTO_FILENAMES_MAP;
   }
   
   twovector MovieID_framenumber(Movie_ID,frame_number);
   (*photo_filenames_map_ptr)[MovieID_framenumber]=photo_filename;
}

// ---------------------------------------------------------------------
// Member function load_current_photos() first checks if the current
// framenumber differs from the previously stored frame number.  If
// not, it loops over all Movies and retrieves their photos
// corresponding to the current frame number from member STL map
// *photo_filenames_map_ptr.  It then resets the Movies' displayed
// photos to their current set.

void MoviesGroup::load_current_photos()
{
   if (AnimationController_ptr->curr_framenumber_equals_prev_framenumber())
      return;

   cout << "inside MoviesGroup::load_current_photos()" << endl;

   int curr_framenumber=AnimationController_ptr->get_curr_framenumber();
   AnimationController_ptr->set_prev_framenumber(curr_framenumber);

   for (unsigned int i=0; i<get_n_Graphicals(); i++)
   {
      Movie* Movie_ptr=get_Movie_ptr(i);
      int Movie_ID=Movie_ptr->get_ID();

      string curr_photo_filename="";

// Given curr_framenumber & Movie_ID, lookup in an STL map filename
// for associated photo:

      twovector MovieID_framenumber(Movie_ID,curr_framenumber);
      PHOTO_FILENAMES_MAP::iterator iter=photo_filenames_map_ptr->find(
         twovector(Movie_ID,curr_framenumber));

      if (iter != photo_filenames_map_ptr->end())
      {
         curr_photo_filename=iter->second;
      }

//      cout << "Movie ID = " << Movie_ID
//           << " framenumber = " << curr_framenumber
//           << " photo filename = " << curr_photo_filename << endl;

// If no photo exists for current Movie and frame number, don't
// attempt to update it:

      if (curr_photo_filename.size()==0) continue;
      
      bool twoD_flag=false;
      Movie_ptr->reset_displayed_photograph(curr_photo_filename,twoD_flag);
   } // loop over index i labeling Movies
}

// ==========================================================================
// Raytracing member functions
// ==========================================================================

// Member function identify_sky_pixels() requires that *photogroup_ptr
// and *DTED_ztwoDarray_ptr have been predefined and computes
// *reduced_DTED_ztwoDarray_ptr if it's not already predefined.  This
// method retrieves the Movie corresponding to the selected photo and
// its texture rectangle.  It then loops over every pixel in the
// select photo and raytraces the pixels into the 3D scene.  If the
// pixel is not occluded by any 3D scene object, we assume the pixel
// corresponds to sky and maximally reset its red color coordinate.  

// In Aug 2009, we empirically found that this raytracing approach to
// sky detection work remarkably well for the ~500 georegistered NYC
// skyline photos reconstructed by Noah Snavely's photosynth algorithms.

void MoviesGroup::identify_sky_pixels()
{
   cout << "inside MoviesGroup::identify_sky_pixels()" << endl;

   if (photogroup_ptr==NULL) return;
   if (DTED_ztwoDarray_ptr==NULL) return;
   if (reduced_DTED_ztwoDarray_ptr==NULL) 
   {
      compute_reduced_DTED_ztwoDarray();
   }

   const double min_Z_ground=0;
   const double max_raytrace_range=20*1000;	// meters
   const double ds=1;	// meter

   int photo_ID=photogroup_ptr->get_selected_photo_ID();
   if (photo_ID < 0) photo_ID=0;
   cout << "photo_ID = " << photo_ID << endl;
   photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(photo_ID);

// As of 2/16/09, we assume that *MoviesGroup_ptr contains just a
// single video clip:

   Movie* Movie_ptr=dynamic_cast<Movie*>(get_last_Graphical_ptr());

   camera* camera_ptr=photograph_ptr->get_camera_ptr();
   threevector camera_posn=camera_ptr->get_world_posn();
//   cout << "*camera_ptr = " << *camera_ptr << endl;

   double U,V;
   threevector r_hat,impact_point;
   texture_rectangle* texture_rectangle_ptr=Movie_ptr->
      get_texture_rectangle_ptr();

   twoDarray* binary_twoDarray_ptr=
      new twoDarray(Movie_ptr->getWidth(),Movie_ptr->getHeight());

   cout << "Width = " << Movie_ptr->getWidth() << endl;
   int n_orig_sky_pixels=0;
   for (unsigned int px=0; px<Movie_ptr->getWidth(); px++)
   {
      cout << px << " " << flush;
      for (unsigned int py=0; py<Movie_ptr->getHeight(); py++)
      {
         texture_rectangle_ptr->get_uv_coords(px,py,U,V);
         r_hat=camera_ptr->pixel_ray_direction(U,V);

         bool ray_occluded_flag=camera_ptr->trace_individual_ray(
            r_hat,min_Z_ground,max_raytrace_range,ds,
            reduced_DTED_scale_factor,DTED_ztwoDarray_ptr,
            reduced_DTED_ztwoDarray_ptr,impact_point);
//         cout << "U = " << U << " V = " << V 
//              << " ray_occluded_flag = " << ray_occluded_flag << endl;

         if (!ray_occluded_flag)
         {
            n_orig_sky_pixels++;
            binary_twoDarray_ptr->put(px,py,1);
         }
         else
         {
            binary_twoDarray_ptr->put(px,py,0);
         }
      } // loop over py index
   } // loop over px index

// In Aug 2009, we empirically discovered some noise voxels within the
// NYC ladar map.  These voxels generate erroneous noise columns
// within the nyc geotif height map.  Raytracing occlusion by such
// noise columns leads to false non-sky columns appearing within 2D
// photo sky classifications.  

// We really should eliminate the noise voxels within the NYC ladar
// map.  But as a quick work-around, we perform simple binary
// filtering to search for non-sky pixels surrounded by many sky
// pixels in the horizontal directions.  This method reclassifies such
// non-sky pixels as sky:

   unsigned int nx_size=21;
   unsigned int ny_size=1;
   double intensity_frac=0.85;
   twoDarray* filtered_binary_twoDarray_ptr=new twoDarray(
      binary_twoDarray_ptr);
   binary_twoDarray_ptr->copy(filtered_binary_twoDarray_ptr);

   binaryimagefunc::binary_filter(
      nx_size,ny_size,binary_twoDarray_ptr,filtered_binary_twoDarray_ptr,
      intensity_frac);

   int n_changes=0;
   for (unsigned int px=0; px<Movie_ptr->getWidth(); px++)
   {
      cout << px << " " << flush;
      for (unsigned int py=0; py<Movie_ptr->getHeight(); py++)
      {

         if (!nearly_equal(filtered_binary_twoDarray_ptr->get(px,py),
                           binary_twoDarray_ptr->get(px,py)))
         {
            n_changes++;
         }
         
         if (nearly_equal(filtered_binary_twoDarray_ptr->get(px,py),1))
         {
            int R,G,B;
            texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
//            cout << "orig R = " << R << " G = " << G << " B = " << B << endl;
            double r=R/255.0;
            double g=G/255.0;
            double b=B/255.0;
            double h,s,v;
            colorfunc::RGB_to_hsv(r,g,b,h,s,v);
            h=0;	// change hue to red
            v *= 0.75;
            colorfunc::hsv_to_RGB(h,s,v,r,g,b);
            R=255;
//            R=255*r;
            G=255*g;
            B=255*b;
//            cout << "new R = " << R << " G = " << G << " B = " << B << endl;
            texture_rectangle_ptr->set_pixel_RGB_values(px,py,R,G,B);
         }
      }
   }

   int n_total_pixels=Movie_ptr->getWidth()*Movie_ptr->getHeight();
   
   cout << "Number of original sky pixels = " << n_orig_sky_pixels << endl;
   cout << "Total number of Movie pixels = " << n_total_pixels << endl;
   cout << "Orig sky pixel frac = " << double(n_orig_sky_pixels)/
      double(n_total_pixels) << endl;
   cout << "Number of filter changes = " << n_changes << endl;
   cout << "frac change = " << double(n_changes)/double(n_orig_sky_pixels)
        << endl;

   delete binary_twoDarray_ptr;
   delete filtered_binary_twoDarray_ptr;

   cout << endl;
}

// ---------------------------------------------------------------------
// Member function identify_ocean_pixels() requires that
// *photogroup_ptr and *DTED_ztwoDarray_ptr have been predefined and
// computes *reduced_DTED_ztwoDarray_ptr if it's not already
// predefined.  This method retrieves the Movie corresponding to the
// selected photo and its texture rectangle.  It then loops over every
// pixel in the select photo and raytraces the pixels into the 3D
// scene.  If the pixel is occluded and the occlusion point lies at 0
// meters above sea level, we assume the pixel corresponds to ocean
// and maximally reset its blue color coordinate.

// In Aug 2009, we empirically found that this raytracing approach to
// ocean detection work remarkably well for the ~500 georegistered NYC
// skyline photos reconstructed by Noah Snavely's photosynth
// algorithms.

void MoviesGroup::identify_ocean_pixels()
{
//   cout << "inside MoviesGroup::identify_ocean_pixels()" << endl;

   if (photogroup_ptr==NULL) return;
   if (DTED_ztwoDarray_ptr==NULL) return;
   if (reduced_DTED_ztwoDarray_ptr==NULL) 
   {
      compute_reduced_DTED_ztwoDarray();
   }

   const double Z_ocean=1;	// meter
   const double max_raytrace_range=20*1000;	// meters
   const double ds=1;	// meter

   int photo_ID=photogroup_ptr->get_selected_photo_ID();
   if (photo_ID < 0) photo_ID=0;
   cout << "photo_ID = " << photo_ID << endl;
   photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(photo_ID);

// As of 2/16/09, we assume that *MoviesGroup_ptr contains just a
// single video clip:

   Movie* Movie_ptr=dynamic_cast<Movie*>(get_last_Graphical_ptr());

   camera* camera_ptr=photograph_ptr->get_camera_ptr();
   threevector camera_posn=camera_ptr->get_world_posn();
//   cout << "*camera_ptr = " << *camera_ptr << endl;

   double U,V;
   threevector r_hat,impact_point;
   texture_rectangle* texture_rectangle_ptr=Movie_ptr->
      get_texture_rectangle_ptr();

   cout << "Width = " << Movie_ptr->getWidth() << endl;
   for (unsigned int px=0; px<Movie_ptr->getWidth(); px++)
   {
      cout << px << " " << flush;
      for (unsigned int py=0; py<Movie_ptr->getHeight(); py++)
      {
         texture_rectangle_ptr->get_uv_coords(px,py,U,V);
         r_hat=camera_ptr->pixel_ray_direction(U,V);

         bool ray_occluded_flag=camera_ptr->trace_individual_ray(
            r_hat,Z_ocean,max_raytrace_range,ds,reduced_DTED_scale_factor,
            DTED_ztwoDarray_ptr,reduced_DTED_ztwoDarray_ptr,impact_point);
//         cout << "U = " << U << " V = " << V 
//              << " ray_occluded_flag = " << ray_occluded_flag << endl;

         if (ray_occluded_flag && impact_point.get(2) <= Z_ocean+0.5)
         {
            int R,G,B;
            texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
            B=255;
            texture_rectangle_ptr->set_pixel_RGB_values(px,py,R,G,B);
         }
      } // loop over py index
   } // loop over px index
   cout << endl;
}

// ---------------------------------------------------------------------
// Member function compute_reduced_DTED_ztwoDarray() generates a
// coarse-grain version of *DTED_ztwoDarray_ptr within member
// *reduced_DTED_ztwoDarray_ptr.  The coarse height map is used to
// speed up raytracing computations.

void MoviesGroup::compute_reduced_DTED_ztwoDarray()
{
//   cout << "inside MoviesGroup::compute_reduced_DTED_ztwoDarray()" << endl;
   
   cout << "Instantiating reduced DTED ztwoDarray:" << endl;

   reduced_DTED_scale_factor=10;
   reduced_DTED_ztwoDarray_ptr=new twoDarray(
      DTED_ztwoDarray_ptr->get_mdim()/reduced_DTED_scale_factor,
      DTED_ztwoDarray_ptr->get_ndim()/reduced_DTED_scale_factor);
   
   int extremal_sentinel=2;
   compositefunc::extremal_subsample_twoDarray(
      DTED_ztwoDarray_ptr,reduced_DTED_ztwoDarray_ptr,extremal_sentinel);
}

// ---------------------------------------------------------------------
// Member function
// project_PolyLines_into_selected_aerial_video_frame() 

bool MoviesGroup::project_PolyLines_into_selected_aerial_video_frame()
{
//   cout << "inside MoviesGroup::project_PolyLines_into_selected_aerial_video_frame()"
//        << endl;

   if (PolyLinesGroup_ptr==NULL) return false;

   Movie* Movie_ptr=dynamic_cast<Movie*>(get_last_Graphical_ptr());
   double minU=Movie_ptr->get_minU();
   double maxU=Movie_ptr->get_maxU();
   double minV=Movie_ptr->get_minV();
   double maxV=Movie_ptr->get_maxV();

//   cout << "Movie_ptr->get_minU() = " << Movie_ptr->get_minU() << endl;
//   cout << "Movie_ptr->get_maxU() = " << Movie_ptr->get_maxU() << endl;
//   cout << "Movie_ptr->get_minV() = " << Movie_ptr->get_minV() << endl;
//   cout << "Movie_ptr->get_maxV() = " << Movie_ptr->get_maxV() << endl;

   return PolyLinesGroup_ptr->
      project_PolyLines_into_selected_aerial_video_frame(minU,maxU,minV,maxV);
} 

// ==========================================================================
// 3D decal display member functions
// ==========================================================================

// Member function display_decals() 

void MoviesGroup::display_decals()
{
   cout << "inside MoviesGroup::display_decals()" << endl;
   
   string decals_subdir="./rectified_views/colorblend/";
   string substring="blend.jpg";
   vector<string> decal_filenames=filefunc::files_in_subdir_matching_substring(
      decals_subdir,substring);

   substring="geom.metadata";
   vector<string> geom_filenames=filefunc::files_in_subdir_matching_substring(
      decals_subdir,substring);

   for (unsigned int i=0; i<decal_filenames.size(); i++)
   {
//      cout << "i = " << i 
//           << " decal_filename = " << decal_filenames[i]
//           << " geom metadata = " << geom_filenames[i]
//           << endl;

      filefunc::ReadInfile(geom_filenames[i]);
      vector<twovector> AB;

      for (int l=0; l<4; l++)
      {
         vector<double> column_values=stringfunc::string_to_numbers(
            filefunc::text_line[l]);
         twovector curr_AB(column_values[0],column_values[1]);
         AB.push_back(curr_AB);
//         cout << "l = " << l << " AB = " << AB.back() << endl;
      }
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[4]);
      threevector rectangle_COM(
         column_values[0],column_values[1],column_values[2]);
//      cout << "rectangle_COM = " << rectangle_COM << endl;

      vector<threevector> relative_vertices;
      for (int l=5; l<9; l++)
      {
         vector<double> column_values=stringfunc::string_to_numbers(
            filefunc::text_line[l]);
         threevector relative_corner(
            column_values[0],column_values[1],column_values[2]);
         relative_vertices.push_back(relative_corner);
      }

// Instantiate new texture rectangle to display decal images within 3D
// map:

      texture_rectangle* AB_texture_rectangle_ptr=
         generate_new_texture_rectangle(decal_filenames[i]);
      Movie* Movie_ptr=generate_new_Movie(AB_texture_rectangle_ptr);
      Movie_ptr->set_texture_fracs(AB[0],AB[1],AB[2],AB[3]);

      Movie_ptr->reset_geom_vertices(
         relative_vertices[0],relative_vertices[1],
         relative_vertices[2],relative_vertices[3]);
//         relative_vertices[1],relative_vertices[0],
//         relative_vertices[3],relative_vertices[2]);

      Movie_ptr->set_UVW_coords(
         get_curr_t(),get_passnumber(),rectangle_COM);

   } // loop over index i labeling decal filenames


}
