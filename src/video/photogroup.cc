// ==========================================================================
// Photogroup class member function definitions
// ==========================================================================
// Last modified on 3/28/14; 4/6/14; 6/7/14; 11/28/15
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "osg/osgGraphicals/AnimationController.h"

#include "video/camerafuncs.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "graphs/graph_edge.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "video/photodbfuncs.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::map;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void photogroup::allocate_member_objects()
{
   image_planes_map_ptr=new IMAGE_PLANES_MAP;
}		       

void photogroup::initialize_member_objects()
{
   selected_photo_ID=-1;
//   UTM_zonenumber=18;	// NYC
   UTM_zonenumber=19;	// Boston
   northern_hemisphere_flag=true;
   n_params_per_photo=4;
//   n_params_per_photo=6;

   common_planes_map_ptr=NULL;
   Messenger_ptr=NULL;
   cluster_photogroup_ptr=NULL;
}

photogroup::photogroup(int ID,int level) :
   graph(ID,level)
{
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

photogroup::photogroup(const photogroup& pg) :
   graph(pg)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(pg);
}

photogroup::~photogroup()
{
//   cout << "inside photogroup destructor" << endl;
   for (IMAGE_PLANES_MAP::const_iterator iter=image_planes_map_ptr->begin();
        iter != image_planes_map_ptr->end(); iter++)
   {
      delete iter->second;
   }
   delete image_planes_map_ptr;

   if (common_planes_map_ptr != NULL)
   {
      for (COMMON_PLANES_MAP::const_iterator iter=
              common_planes_map_ptr->begin(); 
           iter != common_planes_map_ptr->end(); iter++)
      {
         delete iter->second;
      }

      delete common_planes_map_ptr;
   }
   delete cluster_photogroup_ptr;
}

// ---------------------------------------------------------------------
void photogroup::docopy(const photogroup& pg)
{
//   cout << "inside photogroup::docopy()" << endl;
   
   n_params_per_photo=pg.n_params_per_photo;
   bundler_to_world_scalefactor=pg.bundler_to_world_scalefactor;
   bundler_to_world_az=pg.bundler_to_world_az;
   bundler_to_world_el=pg.bundler_to_world_el;
   bundler_to_world_roll=pg.bundler_to_world_roll;
   bundler_to_world_translation=pg.bundler_to_world_translation;

   for (NODES_MAP::const_iterator iter=pg.nodes_map.begin();
        iter != pg.nodes_map.end(); iter++)
   {
      photograph* photo_ptr=static_cast<photograph*>(iter->second);
      photograph* photograph_ptr=new photograph(*photo_ptr);
      add_node(photograph_ptr);
   } // loop over photo iterator

   photo_order.clear();
   for (unsigned int i=0; i<pg.photo_order.size(); i++)
   {
      photo_order.push_back(pg.photo_order[i]);
   }
}

// Overload = operator:

photogroup& photogroup::operator= (const photogroup& pg)
{
   if (this==&pg) return *this;
   graph::operator=(pg);
   docopy(pg);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const photogroup& pg)
{
   outstream << "inside photogroup::operator<<" << endl;
   outstream << (graph&)pg << endl;
   outstream << "n_params_per_photo = " << pg.n_params_per_photo << endl;
   outstream << endl;
   return outstream;
}


// =====================================================================
// Set & get member functions
// =====================================================================

// In the following set and get member functions, integer key p
// corresponds to a photograph's ordering within a mosaic rather than
// its ID = initial index value.

// ---------------------------------------------------------------------
// Member function set_photo_order() reads in a file which is assumed
// to contain integer pairs of photo order vs photo ID.  It fills STL
// vector member photo_order with this information.  We wrote this
// little utility to avoid having to import and process expensive SIFT
// feature information in program LADARPAN.

void photogroup::set_photo_order(string filename)
{
//   cout << "inside photogroup::set_photo_order()" << endl;
   if (!filefunc::ReadInfile(filename))
   {
      cout << "Error in photogroup::set_photo_order()" << endl;
      cout << "file " << filename << " not read in !" << endl;
      return;
   }

   photo_order.clear();
   for (unsigned int n=0; n<filefunc::text_line.size(); n++)
   {
      int order,photo_ID;
      stringfunc::string_to_two_numbers(
         filefunc::text_line[n],order,photo_ID);
      photo_order.push_back(photo_ID);
//      cout << "n = " << n << " photo_order = " << photo_order.back() << endl;
   } // loop over index n 
}

// ---------------------------------------------------------------------
// Member function set_photo_order_equal_to_ID()

void photogroup::set_photo_order_equal_to_ID()
{
//   cout << "inside photogroup::set_photo_order_equal_to_ID()" << endl;

   for (unsigned int p=0; p<get_n_photos(); p++)
   {
      photo_order.push_back(get_photograph_ptr(p)->get_ID());
//      cout << "p = " << p << " photo ID = " << get_photograph_ptr(p)->get_ID()
//           << " photo_order = " << photo_order.back() << endl;
   }
}

// ---------------------------------------------------------------------
// Member function push_back_photo_ID_onto_photo_order() takes in the
// ID for some photo and pushes it back onto member STL vector
// photo_order.  We wrote this little utility when
// mosaicing/compositing order is *NOT* important.  Instead, we
// sometimes need to know a photo's ID given its order (e.g. when
// some naturally ordered/IDed photos are not discarded from a sequence).

void photogroup::push_back_photo_ID_onto_photo_order(int ID)
{
//   cout << "inside photogroup::push_back_photo_ID_onto_photo_order()" << endl;
   photo_order.push_back(ID);
}

// ---------------------------------------------------------------------
void photogroup::set_base_URL(string URL)
{
//   cout << "inside photogroup::set_base_URL()" << endl;
   base_URL=URL;

// Loop over all photographs.  Assign their individual URLs to equal
// base_URL + basename(photo filename):

   for (unsigned int n=0; n<get_n_photos(); n++)
   {
      photograph* photograph_ptr=get_photograph_ptr(n);
//      cout << "photo_order.size() = " << photo_order.size() << endl;
      if (photo_order.size() > 0)
      {
         photograph_ptr=get_ordered_photograph_ptr(n);
      }
      string photo_URL=base_URL+filefunc::getbasename(
         photograph_ptr->get_filename());
      photograph_ptr->set_URL(photo_URL);
//      cout << "n = " << n << " photo_URL = " << photo_URL << endl;
   } // loop over index n labeling photographs
}

string photogroup::get_base_URL() const
{
   return base_URL;
}

// --------------------------------------------------------------------------
// Member function set_bbox() computes the bounding box which
// encompasses all photographs' camera positions.

void photogroup::set_bbox()
{
//   cout << "inside photogroup::set_bbox()" << endl;

   vector<threevector> photo_posns;
   for (unsigned int n=0; n<get_n_photos(); n++)
   {
      photo_posns.push_back(
         get_photograph_ptr(n)->get_camera_ptr()->get_world_posn());
//      cout << "n = " << n
//           << " photo_posn = " << photo_posns.back() << endl;
   }
   photo_bbox.reset_bounds(photo_posns);
//   cout << "photo_bbox = " << photo_bbox << endl;
}

// ==========================================================================
// Photogroup construction & manipulation member functions
// ==========================================================================

// Member function generate_single_photograph() takes in the filename
// for some image.  It instantiates a photograph for the input image filename
// and returns its pointer.

photograph* photogroup::generate_single_photograph(string photo_filename)
{
//   cout << "inside photogroup::generate_single_photograph()" << endl;

   int photo_ID=get_n_photos();

   photograph* photograph_ptr=new photograph(photo_filename,photo_ID);
//   cout << "photograph_ptr = " << photograph_ptr << endl;
   add_node(photograph_ptr);
   set_photo_order_equal_to_ID();

   return photograph_ptr;
}

// ---------------------------------------------------------------------
void photogroup::destroy_single_photograph(photograph* photo_ptr)
{
//   cout << "inside photogroup::destroy_single_photograph()" << endl;

   erase_node(photo_ptr);
   delete photo_ptr;
}

// ---------------------------------------------------------------------
// This overloaded version of generate_single_photograph() takes in
// the filename for some photo along with its pixel dimensions xdim &
// ydim.  It also takes in calibrated camera parameters including az,
// el, roll measured in degrees.  This method instantiates a
// photograph with the specified input parameters and returns its
// pointer.

photograph* photogroup::generate_single_photograph(
   string photo_filename,int xdim,int ydim,
   double fu,double fv,double U0,double V0,double az,double el,double roll,
   const threevector& camera_posn,double frustum_sidelength)
{
//   cout << "inside photogroup::generate_single_photograph()" << endl;

   int photo_ID=get_n_photos();

   photograph* photograph_ptr=new photograph(
      photo_filename,xdim,ydim,photo_ID);
   add_node(photograph_ptr);
   set_photo_order_equal_to_ID();

   az *= PI/180;
   el *= PI/180;
   roll *= PI/180;

   photograph_ptr->reset_camera_parameters(
      fu,fv,U0,V0,az,el,roll,camera_posn);

   if (frustum_sidelength > 0)
   {
      photograph_ptr->set_frustum_sidelength(frustum_sidelength);
   }
   return photograph_ptr;
}

// ---------------------------------------------------------------------
// Member function read_photographs() takes in a PassesGroup as its
// argument and extracts out the names for all input photos.  It
// dynamically generates photograph objects for each photo filename
// and adds them to the photogroup.  This method also parses internal
// and external camera parameters passed as arguments in package
// files.  It initializes each photo's camera with these input
// parameters.

void photogroup::read_photographs(PassesGroup& passes_group)
{
   string image_sizes_filename="";
   read_photographs(passes_group,image_sizes_filename);
}

void photogroup::read_photographs(PassesGroup& passes_group,
                                  string image_sizes_filename)
{
//   cout << "inside photogroup::read_photographs()" << endl;
//   cout << "image_sizes_filename = " << image_sizes_filename << endl;
//   cout << "passes_group.get_n_passes() = "
//        << passes_group.get_n_passes() << endl;

   vector<int> xdim,ydim;
   import_image_sizes(image_sizes_filename,xdim,ydim);
//   cout << "xdim.size() = " << xdim.size() << endl;

// On 11/14/09, we realized the painful and hard way that image size
// info contained within image_sizes_filename must NOT be used if the
// number of input photos is smaller than the number of images inside
// image_sizes_filename.  Otherwise, image centers are generally off
// and image planes do not properly fit within their view frusta!

   if (xdim.size() > 2*passes_group.get_n_passes())
   {
      xdim.clear();
      ydim.clear();
   }

   int photo_counter=0;
   for (unsigned int n=0; n<passes_group.get_n_passes(); n++)
   {
//      cout << "n = " << n << endl;
      Pass* curr_pass_ptr=passes_group.get_pass_ptr(n);
//      cout << "curr_pass_ptr = " << curr_pass_ptr << endl;
//      cout << "passtype = " << curr_pass_ptr->get_passtype() << endl;

      if (curr_pass_ptr->get_passtype() != Pass::video) continue;
      Pass::InputFileType input_file_type=curr_pass_ptr->get_input_filetype();

      if (input_file_type==Pass::png || input_file_type==Pass::jpg ||
          input_file_type==Pass::tif || input_file_type==Pass::rgb)
      {
         string photo_filename=curr_pass_ptr->get_first_filename();
         PassInfo* PassInfo_ptr=curr_pass_ptr->get_PassInfo_ptr();

//         cout << "PassInfo_ptr = " << PassInfo_ptr << endl;
//         cout << "*PassInfo_ptr = " << *PassInfo_ptr << endl;
         
         int photo_ID=photo_counter;
//         cout << "photo_counter = " << photo_counter << endl;

         push_back_photo_ID_onto_photo_order(photo_ID);

//         cout << "n = " << n 
//              << " photo_ID = " << photo_ID 
//              << " photo_filename = " << photo_filename 
//              << " passtype = " << curr_pass_ptr->get_passtype() 
//              << endl;

         photograph* photograph_ptr;
         if (xdim.size() > 0)
         {
            photograph_ptr=new photograph(
               photo_filename,xdim[photo_counter],ydim[photo_counter],
               photo_ID);
         }
         else
         {
            photograph_ptr=new photograph(photo_filename,photo_ID);
//            cout << "xdim.size() = " << xdim.size() << endl;
//            cout << "photograph_ptr->get_xdim() = "
//                 << photograph_ptr->get_xdim() << endl;
/*
            if (xdim[photo_ID] != photograph_ptr->get_xdim() ||
                ydim[photo_ID] != photograph_ptr->get_ydim())
            {
               cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" 
                    << endl;
               cout << "photo_ID = " << photo_ID 
                    << " xdim[photo_ID] = " << xdim[photo_ID]
                    << " ydim[photo_ID] = " << ydim[photo_ID] << endl;
               cout << "photo_xdim = " << photograph_ptr->get_xdim()
                    << " photo_ydim = " << photograph_ptr->get_ydim()
                    << endl;
               outputfunc::enter_continue_char();
            }
*/
         }
         add_node(photograph_ptr);
         photo_counter++;

         double fu=PassInfo_ptr->get_Uaxis_focal_length();
//         cout << "fu = " << fu  << endl;

// If sentinel fu != NEGATIVEINFINITY, set internal and external
// parameters for camera corresponding to *photograph_ptr:

         if (fabs(fu) < 0.5*POSITIVEINFINITY)
         {
            double fv=PassInfo_ptr->get_Vaxis_focal_length();
            double U0=PassInfo_ptr->get_U0();
            double V0=PassInfo_ptr->get_V0();
      
            double az=PassInfo_ptr->get_relative_az()*PI/180;
            double el=PassInfo_ptr->get_relative_el()*PI/180;
            double roll=PassInfo_ptr->get_relative_roll()*PI/180;
            threevector camera_posn(PassInfo_ptr->get_camera_posn());

//            cout.precision(10);
//            cout << " fu = " << fu << " fv = " << fv << endl;
//            cout << "U0 = " << U0 << " V0 = " << V0 << endl;
//            cout << "az = " << az*180/PI << " el = " << el*180/PI
//                 << " roll = " << roll*180/PI << endl;
//            cout << "camera_posn = " << camera_posn << endl;

            photograph_ptr->reset_camera_parameters(
               fu,fv,U0,V0,az,el,roll,camera_posn);

// Check whether image plane info has been pre-calculated.  If so,
// instantiate a new entry within STL map member
// *image_planes_map_ptr.  Also initialize *camera_ptr's imageplane to
// imported plane:

            fourvector pi=PassInfo_ptr->get_imageplane_pi();
            if (pi.get(0) > 0.5*NEGATIVEINFINITY)
            {
               plane* curr_imageplane_ptr=new plane(pi);
               (*image_planes_map_ptr)[photograph_ptr->get_ID()]=
                  curr_imageplane_ptr;
               photograph_ptr->get_camera_ptr()->set_imageplane(pi);
//               cout << "imageplane pi = " << pi << endl;
            }

            double frustum_sidelength=PassInfo_ptr->get_frustum_sidelength();
            double movie_downrange_distance=
               PassInfo_ptr->get_downrange_distance();
            if (frustum_sidelength > 0)
            {
               photograph_ptr->set_frustum_sidelength(frustum_sidelength);
            }
            if (movie_downrange_distance > 0)
            {
               photograph_ptr->set_movie_downrange_distance(
                  movie_downrange_distance);
            }
         } // |fu| < 0.5*POSITIVEINFINITY conditional
      } // curr_pass==video conditional
   } // loop over index n labeling passes

   set_bbox();
}

// ---------------------------------------------------------------------
// This overloaded version of read_photographs() simply instantiates
// and fills a photogroup based upon a set of image files within
// the specified input subdirectory.

void photogroup::read_photographs(string images_subdir)
{
//   cout << "inside photogroup::read_photographs() #3" << endl;

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      images_subdir);
   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
//      cout << "i = " << i << " image_filename = " << image_filenames[i]
//           << endl;
      push_back_photo_ID_onto_photo_order(i);

      photograph* photograph_ptr=new photograph(image_filenames[i],i);
      add_node(photograph_ptr);
   }
}


// ---------------------------------------------------------------------
// Member function average_camera_elevation()

double photogroup::average_camera_elevation() 
{
//   cout << "inside photogroup::average_camera_elevation()" << endl;
   
   vector<double> camera_elevation_angles;
   for (unsigned int n=0; n<get_n_photos(); n++)
   {
      photograph* photograph_ptr=get_photograph_ptr(n);
      camera* camera_ptr=photograph_ptr->get_camera_ptr();
//      double az=camera_ptr->get_rel_az();
      double el=camera_ptr->get_rel_el();
//      double roll=camera_ptr->get_rel_roll();
      camera_elevation_angles.push_back(el);
   }

   double mu_elevation=mathfunc::mean(camera_elevation_angles);
   double sigma_elevation=mathfunc::std_dev(camera_elevation_angles);
   
   cout << "camera elevation angle = " 
        << mu_elevation*180/PI << " +/- " 
        << sigma_elevation*180/PI << endl;
   return mu_elevation;
}

// ---------------------------------------------------------------------
// Member function import_image_filenames() fills and returns an STL
// vector with input photograph names.

vector<string> photogroup::import_image_filenames(
   string images_subdir,string image_list_filename,
   int& n_photos_to_reconstruct)
{
//   cout << "inside photogroup::import_image_filenames()" << endl;
//   cout << "image_list_filename = " << image_list_filename << endl;
   filefunc::ReadInfile(image_list_filename);
   
   if (n_photos_to_reconstruct < 0) 
      n_photos_to_reconstruct=int(filefunc::text_line.size());
//   cout << "n_photos_to_reconstruct = " << n_photos_to_reconstruct << endl;

   vector<string> image_filenames;
   for (int n=0; n<n_photos_to_reconstruct; n++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[n]);
      image_filenames.push_back(images_subdir+substrings[0]);
//      cout << "n = " << n << " image_filename = " << image_filenames.back()
//           << endl;
   }

   return image_filenames;
}

// ---------------------------------------------------------------------
// Member function import_image_sizes() reads in precalculated image
// sizes in order to speed up photograph instantiation by avoiding
// having to perform expensive OSG file input.

void photogroup::import_image_sizes(string image_sizes_filename,
                                    vector<int>& xdim,vector<int>& ydim)
{
//   cout << "inside photogroup::import_image_sizes()" << endl;
//   cout << "image_sizes_filename = " << image_sizes_filename << endl;

   if (image_sizes_filename.size() <= 0) return;

   filefunc::ReadInfile(image_sizes_filename);
   for (unsigned int n=0; n<filefunc::text_line.size(); n++)
   {
      vector<string> substrings=
         stringfunc::decompose_string_into_substrings(
            filefunc::text_line[n]);
      xdim.push_back(stringfunc::string_to_number(substrings[1]));
      ydim.push_back(stringfunc::string_to_number(substrings[2]));
//      cout << "n = " << n << " xdim = " << xdim.back()
//           << " ydim = " << ydim.back() << endl;
   } 
}

// ---------------------------------------------------------------------
// Member function export_image_sizes() loops over all images within
// the current photogroup and writes their xdim,ydim sizes to an
// output text file.  In subsequent runs, programs can read this
// output file rather than calling the slow
// photograph::set_image_dimensions() method.

void photogroup::export_image_sizes(string output_filename)
{
//   cout << "inside photogroup::export_image_sizes(), output_filename = "
//        << output_filename << endl;
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   
   for (unsigned int n=0; n<get_n_photos(); n++)
   {
      photograph* photograph_ptr=get_photograph_ptr(n);
      int xdim=photograph_ptr->get_xdim();
      int ydim=photograph_ptr->get_ydim();
      outstream << n << "  " << xdim << "  " << ydim << endl;
   }
   filefunc::closefile(output_filename,outstream);
}

// ---------------------------------------------------------------------
void photogroup::estimate_internal_camera_params(double FOV_u)
{
   for (unsigned int n=0; n<get_n_photos(); n++)
   {
      photograph* curr_photo_ptr=get_photograph_ptr(n);
      curr_photo_ptr->estimate_internal_camera_params(FOV_u);
   } // loop over index n labeling photographs
}

// ---------------------------------------------------------------------
// Member function get_photograph_ptr() returns the photograph pointer
// corresponding to input ID.

photograph* photogroup::get_photograph_ptr(int ID)
{
//   cout << "inside photogroup::get_photograph_ptr#1, ID = " << ID << endl;
//   cout << "get_node_ptr(ID) = " << get_node_ptr(ID) << endl;
//   cout << "static_cast<photograph*>(get_node_ptr(ID)) = " 
//        << static_cast<photograph*>(get_node_ptr(ID)) << endl;
   return static_cast<photograph*>(get_node_ptr(ID));
}

const photograph* photogroup::get_photograph_ptr(int ID) const
{
//   cout << "inside photogroup::get_photograph_ptr#2, ID = " << ID << endl;
//   cout << "get_node_ptr(ID) = " << get_node_ptr(ID) << endl;
//   cout << "static_cast<const photograph*>(get_node_ptr(ID)) = " 
//        << static_cast<const photograph*>(get_node_ptr(ID)) << endl;

   return static_cast<const photograph*>(get_node_ptr(ID));
}

// ---------------------------------------------------------------------
photograph* photogroup::get_ordered_photograph_ptr(int p) 
{
//   cout << "inside photogroup::get_ordered_photograph_ptr() #1" << endl;
//   cout << "p = " << p << " photo_order[p] = " << photo_order[p]
//        << endl;
   return get_photograph_ptr(photo_order[p]);
}

const photograph* photogroup::get_ordered_photograph_ptr(int p) const
{
//   cout << "inside photogroup::get_ordered_photograph_ptr() #2" << endl;
//   cout << "p = " << p << " photo_order[p] = " << photo_order[p]
//        << endl;
   return get_photograph_ptr(photo_order[p]);
}

// ---------------------------------------------------------------------
photograph* photogroup::get_selected_photograph_ptr()
{
   return get_photograph_ptr(get_selected_photo_ID());
}

const photograph* photogroup::get_selected_photograph_ptr() const
{
   return get_photograph_ptr(get_selected_photo_ID());
}

// ---------------------------------------------------------------------
void photogroup::destroy_all_photos()
{
//   cout << "inside photogroup::destroy_all_photos()" << endl;
//   cout << "get_n_photos() = " << get_n_photos() << endl;

   for (unsigned int n=0; n<get_n_photos(); n++)
   {
      photograph* curr_photo_ptr=get_photograph_ptr(n);
      destroy_single_photograph(curr_photo_ptr);
   } // loop over index n labeling photographs
}

// ==========================================================================
// Subgroup member functions
// ==========================================================================

// Member function generate_subgroup() dynamically instantiates a new
// photogroup.  It copies photographs labeled by IDs ranging from
// ID_start to ID_stop into the new subgroup.  This method returns a
// pointer to the new subgroup.

photogroup* photogroup::generate_subgroup(
   unsigned int ID_start,unsigned int ID_stop)
{
//   cout << "inside photogroup::generate_subgroup()" << endl;
//   cout << "ID_start = " << ID_start << " ID_stop = " << ID_stop << endl;

   photogroup* sub_photogroup_ptr=new photogroup;
   for (unsigned int ID=ID_start; ID <= ID_stop; ID++)
   {
      photograph* photo_ptr=get_photograph_ptr(ID);
      sub_photogroup_ptr->add_node(photo_ptr);
      sub_photogroup_ptr->push_back_photo_ID_onto_photo_order(ID);
   }
//   cout << "sub_photogroup_ptr->get_n_photos() = "
//        << sub_photogroup_ptr->get_n_photos() << endl;

   return sub_photogroup_ptr;
}

// ==========================================================================
// Photograph parameter manipulation member functions
// ==========================================================================

void photogroup::save_initial_camera_f_az_el_roll_params()
{
//   cout << "inside photogroup::save_initial_camera_f_az_el_roll_params()" 
//        << endl;
   
   for (unsigned int p=0; p<get_n_photos(); p++)
   {
      photograph* photograph_ptr=get_photograph_ptr(p);
      camera* camera_ptr=photograph_ptr->get_camera_ptr();
      camera_ptr->save_initial_f_az_el_roll_params();
   } // loop over index p labeling photos
}

void photogroup::restore_initial_camera_f_az_el_roll_params()
{
//   cout << "inside photogroup::restore_initial_camera_f_az_el_roll_params()" 
//        << endl;
   
   for (unsigned int p=0; p<get_n_photos(); p++)
   {
      photograph* photograph_ptr=get_photograph_ptr(p);
      camera* camera_ptr=photograph_ptr->get_camera_ptr();
      camera_ptr->restore_initial_f_az_el_roll_params();
   } // loop over index p labeling photos
}

void photogroup::globally_reset_camera_world_posn(const threevector& posn)
{
//   cout << "inside photogroup::globally_reset_camera_world_posn()" << endl;
   
   for (unsigned int p=0; p<get_n_photos(); p++)
   {
      photograph* photograph_ptr=get_photograph_ptr(p);
      camera* camera_ptr=photograph_ptr->get_camera_ptr();
      camera_ptr->set_world_posn(posn);
   } // loop over index p labeling photos
}

// ---------------------------------------------------------------------
// Member function rescale_focal_lengths() multiplies the focal
// parameters for each calibrated photo in *photogroup_ptr by the
// input scale_factor.  It then computes in closed-form the
// compensating relative rotation between each photo pair which keeps
// tiepoint pairs aligned.  The focal and rotation angle parameters
// are adjusted for each photo's camera.

void photogroup::rescale_focal_lengths(double scale_factor)
{
//   cout << "inside photogroup::rescale_focal_lengths()" << endl;
   
   for (unsigned int p=0; p<get_n_photos(); p++)
   {
      photograph* photograph_ptr=get_photograph_ptr(p);
      camera* camera_ptr=photograph_ptr->get_camera_ptr();

      double f_init=camera_ptr->get_fu();
      double az_init,el_init,roll_init;
      camera_ptr->get_az_el_roll_from_Rcamera(az_init,el_init,roll_init);

      camera_ptr->rescale_focal_length(
         photograph_ptr->get_ydim(),f_init,scale_factor,
         az_init,el_init,roll_init);
   } // loop over index p labeling photos
}

// ---------------------------------------------------------------------
// Member function globally_rotate

void photogroup::globally_rotate(const rotation& R_global)
{
//   cout << "inside photogroup::globally_rotate()" << endl;
   for (unsigned int n=0; n<get_n_photos(); n++)
   {
      photograph* photograph_ptr=get_photograph_ptr(n);
      camera* camera_ptr=photograph_ptr->get_camera_ptr();

      double init_az,init_el,init_roll;
      camera_ptr->get_az_el_roll_from_Rcamera(init_az,init_el,init_roll);

//      cout << "n = " << n 
//           << " init_az = " << init_az*180/PI
//           << " init_el = " << init_el*180/PI
//           << " init_roll = " << init_roll*180/PI << endl;

      rotation R_init;
      R_init=R_init.rotation_from_az_el_roll(init_az,init_el,init_roll);
//      cout << "R_init = " << R_init << endl;
      rotation R_final=R_global*R_init;
//      cout << "R_global = " << R_global << endl;
//      cout << "R_final = " << R_final << endl;
      double final_az,final_el,final_roll;
      R_final.az_el_roll_from_rotation(final_az,final_el,final_roll);

//      cout << "n = " << n 
//           << " final_az = " << final_az*180/PI
//           << " final_el = " << final_el*180/PI
//           << " final_roll = " << final_roll*180/PI << endl;

      camera_ptr->set_Rcamera(final_az,final_el,final_roll);
      camera_ptr->construct_projection_matrix();

//      cout << "*camera_ptr = " << *camera_ptr << endl;
   } // loop over index n labeling photographs
}

// ---------------------------------------------------------------------
// Member function export_photo_parameters generates package files for
// all photos within the current photogroup object.  These package
// files can be read by program NEW_FOV to display 2D images as 3D
// OBSERVATIONFRUSTA.

void photogroup::export_photo_parameters(
   string packages_subdir,bool photos_ordered_flag,double frustum_sidelength)
{
//   cout << "inside photogroup::export_photo_parameters()" << endl;
   string banner="Exporting photograph parameters to package files:";
   outputfunc::write_banner(banner);

   for (unsigned int n=0; n<get_n_photos(); n++)
   {
      photograph* photograph_ptr=get_photograph_ptr(n);
//      cout << "photo_order.size() = " << photo_order.size() << endl;
      if (photo_order.size() > 0 && photos_ordered_flag)
      {
         photograph_ptr=get_ordered_photograph_ptr(n);
      }

//      cout << "n = " << n
//           << " photo filename = " << photograph_ptr->get_filename()
//           << endl;
//      cout << "photograph_ptr = " << photograph_ptr << endl;
      
      string filename_descriptor="";
      photograph_ptr->export_camera_parameters(
         get_photograph_ptr(n)->get_camera_ptr(),filename_descriptor,
         packages_subdir,frustum_sidelength);
   } // loop over index n labeling photos within photogroup
}

// ==========================================================================
// Photo resizing member functions
// ==========================================================================

// Member function generate_thumbnails() loops over all photographs
// within the current photogroup.  It temporarily instantiates a
// texture_rectangle and calls this class' generate_thumbnail() member
// function.  The output thumbnail file is written to a thumbnails/
// subdirectory of the photographs' directory.  And each thumbnail
// file has a thumbnail_ prepended to its name.

void photogroup::generate_thumbnails()
{
   cout << "inside photogroup::generate_thumbnails()" << endl;
//   cout << "n_photos = " << get_n_photos() << endl;

   unsigned int n_start=0;
   int n_photos = get_n_photos();
   int counter = 0;
   for (int n=n_start; n<n_photos; n++)
   {
      outputfunc::update_progress_and_remaining_time(
         counter, double(n), 0.05 * n_photos, double(n_photos));
      counter++;

      photograph* photo_ptr=get_photograph_ptr(n);
//      cout << "Resizing photo " << n << endl;
//      cout << "Resizing photo " << n << " of " << n_photos
//           << " : Orig xdim = " << photo_ptr->get_xdim()
//           << " Orig ydim = " << photo_ptr->get_ydim() << endl;

      unsigned int thumbnail_xdim,thumbnail_ydim;
      videofunc::get_thumbnail_dims(
         photo_ptr->get_xdim(),photo_ptr->get_ydim(),
         thumbnail_xdim,thumbnail_ydim);
//      cout << "thumbnail_xdim = " << thumbnail_xdim
//           << " thumbnail_ydim = " << thumbnail_ydim << endl;
      
      string thumbnail_filename=videofunc::generate_thumbnail(
         photo_ptr->get_filename(),photo_ptr->get_xdim(),photo_ptr->get_ydim(),
         thumbnail_xdim,thumbnail_ydim);

   } // loop over index n labeling photos
}

// ---------------------------------------------------------------------
// Member function downsize_photos() takes in maximum allowed x and y
// dimensions measured in pixels.  It loops through each photo within
// and downsizes any whose x or y dimensions exceed the maximum input
// bounds.  It original oversized image is moved into an
// oversized_original_images subdir and replaced by the downsized
// image with the same name.

void photogroup::downsize_photos(unsigned int max_xdim,unsigned int max_ydim)
{
//   cout << "inside photogroup::downsize_photos()" << endl;
//   cout << "n_images = " << get_n_photos() << endl;

   unsigned int n_start=0;
   for (unsigned int n=n_start; n<get_n_photos(); n++)
   {
      photograph* photo_ptr=get_photograph_ptr(n);

      unsigned int xdim=photo_ptr->get_xdim();
      unsigned int ydim=photo_ptr->get_ydim();
      if (xdim <= max_xdim && ydim <= max_ydim) continue;

      double aspect_ratio=double(xdim)/double(ydim);
      double xratio=double(xdim)/double(max_xdim);
      double yratio=double(ydim)/double(max_ydim);
      int new_xdim,new_ydim;
      if (xratio > yratio)
      {
         new_xdim=max_xdim;
         new_ydim=new_xdim/aspect_ratio;
      }
      else
      {
         new_ydim=max_ydim;
         new_xdim=aspect_ratio*new_ydim;
      }

      cout << "Downsizing image " << n << " of " << get_n_photos()-n_start
           << endl;
      cout << "   Orig xdim = " << xdim
           << " Orig ydim = " << ydim 
           << " New xdim = " << new_xdim
           << " new ydim = " << new_ydim 
           << endl;

// Move oversized original image into subdirectory and replace it with
// downsized version:

      string oversized_original_images_subdir=get_bundler_IO_subdir()+
         "images/oversized_original_images/";
//      cout << "oversized_original_images_subdir = " 
//           << oversized_original_images_subdir << endl;
      filefunc::dircreate(oversized_original_images_subdir);

      string image_filename=photo_ptr->get_filename();
      string unix_cmd="mv "+image_filename+" "+
         oversized_original_images_subdir;
      sysfunc::unix_command(unix_cmd);

      string oversized_image_filename=oversized_original_images_subdir+
         filefunc::getbasename(image_filename);
      
      string downsized_image_filename=image_filename;
      videofunc::resize_image(
         oversized_image_filename,xdim,ydim,new_xdim,new_ydim,
         downsized_image_filename);
      
// On 3/23/12, we empirically found that the actual downsized image
// may have x or y dimensions which slightly differ from new_xdim and
// new_ydim.  So we need to remeasure the downsized image's pixel
// width and height:

      imagefunc::get_image_width_height(downsized_image_filename,xdim,ydim);
      photo_ptr->set_xdim(xdim);
      photo_ptr->set_ydim(ydim);

   } // loop over index n labeling photos
}

// ---------------------------------------------------------------------
// Member function standard_size_photos() loops over all photos within
// *this.  For each input image, it generates a new version whose
// pixel dimensions exactly correspond to the specified input width
// and height.  The standardized images are exported to the specified
// output subdirectory.

void photogroup::standard_size_photos(
   unsigned int output_xdim,unsigned int output_ydim,
   string standard_sized_images_subdir)
{
   cout << "inside photogroup::standand_size_photos()" << endl;
   cout << "n_images = " << get_n_photos() << endl;

   unsigned int n_start=0;
   for (unsigned int n=n_start; n<get_n_photos(); n++)
   {
      photograph* photo_ptr=get_photograph_ptr(n);
      string image_filename=photo_ptr->get_filename();
      string basename=filefunc::getbasename(image_filename);
      string standard_sized_image_filename=standard_sized_images_subdir+
         basename;

      unsigned int xdim=photo_ptr->get_xdim();
      unsigned int ydim=photo_ptr->get_ydim();

      cout << "Resizing image " << n << " of " << get_n_photos()-n_start
           << endl;
      cout << "   Orig xdim = " << xdim
           << " Orig ydim = " << ydim 
           << " New xdim = " << output_xdim
           << " new ydim = " << output_ydim 
           << endl;

      videofunc::force_size_image(
         image_filename,output_xdim,output_ydim,
         standard_sized_image_filename);
   } // loop over index n labeling photos
}

// ==========================================================================
// Bundler member functions
// ==========================================================================

// Member function generate_bundler_photographs() reads in the
// filenames for Noah's reconstructed photos.  It instantiates a
// photograph for each bundler image.

void photogroup::generate_bundler_photographs(
   string bundler_IO_subdir,string image_list_filename,
   int n_photos_to_reconstruct,
   bool parse_exif_metadata_flag,bool check_for_corrupted_images_flag)
{
   string image_sizes_filename;
   generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,image_sizes_filename,
      n_photos_to_reconstruct,parse_exif_metadata_flag,
      check_for_corrupted_images_flag);
}

void photogroup::generate_bundler_photographs(
   string bundler_IO_subdir,string image_list_filename,
   string image_sizes_filename,int n_photos_to_reconstruct,
   bool parse_exif_metadata_flag,bool check_for_corrupted_images_flag)
{
//   cout << "inside photogroup::generate_bundler_photographs() #2" << endl;
//   cout << "n_photos_to_reconstruct = " << n_photos_to_reconstruct << endl;
//   cout << "image_list_filename = " << image_list_filename << endl;

   vector<string> image_filenames=import_image_filenames(
      bundler_IO_subdir,image_list_filename,n_photos_to_reconstruct);
   vector<int> xdim,ydim;
   import_image_sizes(image_sizes_filename,xdim,ydim);
   generate_bundler_photographs(
      image_filenames,xdim,ydim,
      parse_exif_metadata_flag,check_for_corrupted_images_flag);
}

void photogroup::generate_bundler_photographs(
   const vector<string>& image_filenames,
   const vector<int>& xdim,const vector<int>& ydim,
   bool parse_exif_metadata_flag,bool check_for_corrupted_images_flag)
{
//   cout << "inside photogroup::generate_bundler_photographs() #3" << endl;
//   cout << "image_filenames.size() = " << image_filenames.size() << endl;
//   cout << "parse_exif_metadata_flag = " << parse_exif_metadata_flag << endl;
//   cout << "check_for_corrupted_images_flag = "
//        << check_for_corrupted_images_flag << endl;

   int n_start=0;
   for (unsigned int n=n_start; n<image_filenames.size(); n++)
   {
//      outputfunc::write_banner("Generating bundler photograph "+
//                               stringfunc::number_to_string(n));
      if (n%500==0)
         cout << "n = " << n 
              << " image_filename = " << image_filenames[n] << endl;

// On 5/7/12, we discovered the hard way that input JPG files
// (e.g. from flickr) can be corrupted.  So we need to explicitly
// check for bad input images.  If found, we replace them with blank
// dummy images:

      if (check_for_corrupted_images_flag)
      {
         if (!imagefunc::valid_image_file(image_filenames[n]))
         {
            int mdim=600;
            int ndim=400;
            videofunc::generate_blank_jpgfile(mdim,ndim);
            cout << "Generated blank image to replace corrupted one in "
                 << image_filenames[n] << endl;
         }
      } // check_for_corrupted_images_flag conditional
      
      photograph* photograph_ptr=NULL;
      if (xdim.size() > 0)
      {
//         cout 
//             << "n = " << n 
//              << " filename = " << image_filenames[n] 
//              << " xdim = " << xdim[n] << " ydim = " << ydim[n]
//              << endl;

         photograph_ptr=new photograph(
            image_filenames[n],xdim[n],ydim[n],n,parse_exif_metadata_flag); 
      }
      else
      {
         photograph_ptr=new photograph(
            image_filenames[n],n,parse_exif_metadata_flag); 
      }
      add_node(photograph_ptr);
   } // loop over index n labeling input photos
}

// ---------------------------------------------------------------------
// Member function reconstruct_bundler_cameras() extracts
// reconstructed cameras from the bundle.out file generated by Noah
// Snavely's BUNDLER program.  For each photo within input file
// image_list_filename, this method recovers the intrinsic and
// extrinsic camera parameters generated by BUNDLER.
// It dynamically instantiates a new photograph for each
// camera and sets its world rotation, position and focal parameter
// based upon the parameters within bundle.out.

void photogroup::reconstruct_bundler_cameras(
   string bundler_IO_subdir,string image_list_filename,
   string image_sizes_filename,
   string bundle_filename,int n_photos_to_reconstruct)
{
//   cout << "inside photogroup::reconstruct_bundler_cameras()" << endl;
//   cout << "image_list_filename = " << image_list_filename << endl;
//   cout << "image_sizes_filename = " << image_sizes_filename << endl;
//   cout << "bundle_filename = " << bundle_filename
//        << endl;

   generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,image_sizes_filename,
      n_photos_to_reconstruct);
   reconstruct_bundler_cameras(bundle_filename,n_photos_to_reconstruct);
}

// ---------------------------------------------------------------------
void photogroup::reconstruct_bundler_cameras(
   string bundle_filename,int n_photos_to_reconstruct)
{
//   cout << "inside photogroup::reconstruct_bundler_cameras()" << endl;
//   cout << "bundle_filename = " << bundle_filename
//        << endl;

   filefunc::ReadInfile(bundle_filename);
   string first_line=filefunc::text_line[0];
//   cout <<  "first_line = " << first_line << endl;
   vector<double> bundler_params=stringfunc::string_to_numbers(first_line);
   unsigned int n_cameras=bundler_params[0];
   unsigned int n_reconstructed_3D_points=bundler_params[1];
   cout << "n_cameras = " << n_cameras 
        << " n_reconstructed_3D_pnts = " << n_reconstructed_3D_points 
        << endl;

   int curr_linenumber=1;
   if (n_photos_to_reconstruct > 0) n_cameras=n_photos_to_reconstruct;

   for (unsigned int n=0; n<n_cameras; n++)
   {
//      cout << "Camera = " << n << endl;
      photograph* photograph_ptr=get_photograph_ptr(n);
      if (photograph_ptr==NULL) continue;
      
      camera* camera_ptr=photograph_ptr->get_camera_ptr();

// Parse Noah's camera parameters file:

      vector<string> line;
      for (unsigned int j=0; j<5; j++)
      {
         line.push_back(filefunc::text_line[curr_linenumber]);
         curr_linenumber++;
//         cout << line.back() << endl;
      }

      vector<double> fkk=stringfunc::string_to_numbers(line[0]);
      vector<double> R0=stringfunc::string_to_numbers(line[1]);
      vector<double> R1=stringfunc::string_to_numbers(line[2]);
      vector<double> R2=stringfunc::string_to_numbers(line[3]);
      vector<double> t=stringfunc::string_to_numbers(line[4]);

      double f=-fkk[0]/photograph_ptr->get_ydim();
      double kappa2=fkk[1];
      double kappa4=fkk[2];
      double U0=0.5*photograph_ptr->get_xdim()/photograph_ptr->get_ydim();
      double V0=0.5;

      camera_ptr->set_internal_params(f,f,U0,V0);
      camera_ptr->set_kappas(kappa2,kappa4);

      threevector Uhat(R0[0],R0[1],R0[2]);
      threevector Vhat(R1[0],R1[1],R1[2]);
      camera_ptr->set_Rcamera(Uhat,Vhat);

      threevector t_noah(t[0],t[1],t[2]);
      threevector world_posn=-(camera_ptr->get_Rcamera_ptr()->transpose())
         *t_noah;
      camera_ptr->set_world_posn(world_posn);      
   } // loop over index n labeling cameras
}

// ---------------------------------------------------------------------
// Member function compute_cameras_COM_and_plane() calculates the
// center-of-mass of the cameras for all photos within the current
// photogroup.  It also computes the plane which best fits the
// camera's world positions.

void photogroup::compute_cameras_COM_and_plane()
{
   cout << "inside photogroup::compute_cameras_COM_and_plane()" << endl;

   threevector camera_COM;
   vector<threevector> camera_posns;
   for (unsigned int n=0; n<get_n_photos(); n++)
   {
      photograph* photograph_ptr=get_photograph_ptr(n);
      camera* camera_ptr=photograph_ptr->get_camera_ptr();

      camera_posns.push_back(camera_ptr->get_world_posn());
      camera_COM += camera_posns.back();

      cout << "Camera = " << n 
           << " X = " << camera_posns.back().get(0)
           << " Y = " << camera_posns.back().get(1)
           << " Z = " << camera_posns.back().get(2) << endl;

   } // loop over index n labeling cameras
   camera_COM /= get_n_photos();
   
   cout << "camera_COM = " << camera_COM << endl;
   plane camera_plane(camera_posns);
   cout << "camera_plane = " << camera_plane << endl;
   
   rotation R;
   R=R.rotation_taking_pqr_to_uvw(
      camera_plane.get_ahat(),camera_plane.get_bhat(),camera_plane.get_nhat(),
      x_hat,y_hat,z_hat);
   cout << "R = " << R << endl;

   double az,el,roll;
   R.az_el_roll_from_rotation(az,el,roll);
   cout << "az = " << az*180/PI << endl;
   cout << "el = " << el*180/PI << endl;
   cout << "roll = " << roll*180/PI << endl;

   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function print_bundler_to_world_params()

void photogroup::print_bundler_to_world_params()
{
   cout << "inside photogroup::print_bundler_to_world_params()" << endl;

   cout.precision(10);

   cout << "bundler_to_world_translation = "
        << bundler_to_world_translation << endl;
   cout << "bundler_to_world_az = " 
        << bundler_to_world_az*180/PI << endl;
   cout << "bundler_to_world_el = " 
        << bundler_to_world_el*180/PI << endl;
   cout << "bundler_to_world_roll = " 
        << bundler_to_world_roll*180/PI << endl;
   cout << "bundler_to_world_scalefactor = " 
        << bundler_to_world_scalefactor << endl;
}

// ---------------------------------------------------------------------
// Member function read_photograph_covariance_traces() parses Noah's
// text file containing covariance matrix and trace information for
// each photo reconstructed by BUNDLER.

void photogroup::read_photograph_covariance_traces(
   string covariances_filename)
{
   cout << "inside photogroup::read_photograph_covariance_traces()"
        << endl;
   
   filefunc::ReadInfile(covariances_filename);
   vector<int> photo_IDs;
   vector<double> covar_traces;
   for (unsigned int n=0; n<get_n_photos(); n++)
   {
      int curr_ID=stringfunc::string_to_number(
         filefunc::text_line[3*n+0]);
      double curr_trace=stringfunc::string_to_number(
         filefunc::text_line[3*n+2]);
//      cout << "n = " << n << " curr_trace = " << curr_trace << endl;
      get_photograph_ptr(n)->set_bundler_covariance_trace(curr_trace);
//      cout << "n = " << n << " Covariance trace = "
//           << get_photograph_ptr(n)->get_bundler_covariance_trace() << endl;

      photo_IDs.push_back(curr_ID);
      covar_traces.push_back(curr_trace);
   }

// For ladar-bundler 3D point matching, output first few photos with
// lowest reconstructed covariance matrix traces:

   templatefunc::Quicksort(covar_traces,photo_IDs);
   for (unsigned int i=0; i<250; i++)
   {
      cout << "i = " << i << " sorted covar = " << covar_traces[i]
           << " photo ID = " << photo_IDs[i] << endl;
   }

}

// ---------------------------------------------------------------------
// Member function compute_node_color() recovers the covariance trace
// calculated by Noah Snavely's BUNDLER program for the input
// *photograph_ptr.  If the trace exceeds 0, this method sets a hue
// based upon the logarithm (base 10) of the trace.  It returns an RGB
// value containing the hue (and maximal saturation plus value) color
// information.

colorfunc::RGB photogroup::compute_node_color(photograph* photograph_ptr)
{
   cout << "inside photogroup::compute_node_color()" << endl;

   colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(colorfunc::grey);

   double trace=photograph_ptr->get_bundler_covariance_trace();
   cout << "trace = " << trace << endl;
   if (trace < 0) return curr_RGB;

   double hue=0;
   double log10trace=log10(trace);
   const double log10_trace_start=-6;
   const double log10_trace_stop=-2;

   const double hue_start=300;	// purple
   const double hue_stop=0;	// red
   hue=hue_start;
   if (log10trace < log10_trace_start)
   {
      hue=hue_start;
   }
   else if (log10trace > log10_trace_stop)
   {
      hue=hue_stop;
   }
   else
   {
      hue=hue_start+(log10trace-log10_trace_start)/
         (log10_trace_stop-log10_trace_start)*(hue_stop-hue_start);
   }

//   cout << "hue = " << hue << endl;

   colorfunc::HSV curr_hsv;
   curr_hsv.first=hue;
   curr_hsv.second=1.0;
   curr_hsv.third=1.0;
   curr_RGB=colorfunc::hsv_to_RGB(curr_hsv);

   return curr_RGB;
}

// ==========================================================================
// Image plane member functions
// ==========================================================================

// Member function get_image_plane() takes in integer i labeling a
// pre-calculated image plane for some reconstructed camera.  It
// searches STL member *image_planes_map_ptr for a pre-defined
// imageplane.  If one exists, this method returns a pointer to the
// requested plane.  Otherwise, it returns NULL.

plane* photogroup::get_image_plane(int i)
{
//   cout << "inside photogroup::get_image_plane()" << endl;
   
   IMAGE_PLANES_MAP::const_iterator iter=image_planes_map_ptr->find(i); 
   if (iter==image_planes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return iter->second;
   }
}

// ---------------------------------------------------------------------
// Member function import_common_photo_planes() stores pre-calculated
// common image planes for pairs of photos within STL map
// common_planes_map.

void photogroup::import_common_photo_planes(string common_planes_filename)
{
//   cout << "inside photogroup::import_common_photo_planes()" << endl;
   
   filefunc::ReadInfile(common_planes_filename);

   common_planes_map_ptr=new COMMON_PLANES_MAP;
   for (unsigned int n=0; n<filefunc::text_line.size(); n++)
   {
      vector<double> fields=
         stringfunc::string_to_numbers(filefunc::text_line[n]);
      int photo_id1=fields[0];
      int photo_id2=fields[1];
      fourvector pi(fields[2],fields[3],fields[4],fields[5]);
      plane* common_plane_ptr=new plane(pi);
      (*common_planes_map_ptr)[pair<int,int>(photo_id1,photo_id2)]=
         common_plane_ptr;
   } // loop over index n labeling lines within common_planes_filename

//   cout << "common_planes_map.size() = " << common_planes_map_ptr->size() 
//        << endl;
}

// ---------------------------------------------------------------------
// Member function get_common_image_plane() takes in integers i and j
// labeling some pair of reconstructed cameras.  It searches STL
// member *common_planes_map_ptr for a pre-defined common imageplane
// for these two cameras.  If one exists, this method returns a
// pointer to the requested plane.  Otherwise, it returns NULL.

plane* photogroup::get_common_image_plane(int i,int j)
{
//   cout << "inside photogroup::get_common_image_plane()" << endl;
//   cout << "i = " << i << " j = " << j << endl;

   pair<int,int> p;
   if (i < j)
   {
      p=pair<int,int>(i,j);
   }
   else
   {
      p=pair<int,int>(j,i);
   }
   COMMON_PLANES_MAP::const_iterator iter=common_planes_map_ptr->find(p); 
   if (iter==common_planes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return iter->second;
   }
}

// ---------------------------------------------------------------------
// Member function get_overlapping_image_planes() takes in integer
// photo_ID labeling some reconstructed photo.  It performs a brute
// force search over all other photos and returns the image planes for
// those which share SIFT content with the input photo.

vector<plane*> photogroup::get_overlapping_image_planes(int photo_ID)
{
   cout << "inside photogroup::get_overlapping_image_planes()" << endl;
   cout << "photo_ID = " << photo_ID << endl;
   vector<plane*> overlapping_image_planes;

   for (NODES_MAP::const_iterator iter=nodes_map.begin(); iter !=
           nodes_map.end(); iter++)
   {
      node* node_ptr=iter->second;
      int other_photo_ID=node_ptr->get_ID();
      if (photo_ID==other_photo_ID) continue;
      plane* curr_plane_ptr=get_common_image_plane(photo_ID,other_photo_ID);
      if (curr_plane_ptr != NULL) 
      {
         overlapping_image_planes.push_back(
            get_image_plane(other_photo_ID));
         cout << "other_photo_ID = " << other_photo_ID << endl;
      }
   }
   return overlapping_image_planes;
}

// Member function get_overlapping_imageplane_orders() takes in the ID
// for some photo.  It iterates over all entries within the nodes_map
// and pulls out those photographs which share a common image plane
// with the input photo.  This method returns an STL vector containing
// the orders (and not the IDs!) of the overlapping photos.  Recall
// that OBSFRUSTA and MOVIES are sequentially ordered.  So their IDs
// equal their orders.  On the other hand, photographs' IDs generally
// do NOT equal their orders.  So the photo orders returned by this
// method essentially identify corresponding OBSFRUSTA and MOVIES.

vector<int> photogroup::get_overlapping_imageplane_orders(int photo_ID)
{
   cout << "inside photogroup::get_overlapping_imageplane_orders()" << endl;
   cout << "photo_ID = " << photo_ID << endl;
   vector<int> overlapping_imageplane_orders;

   for (NODES_MAP::const_iterator iter=nodes_map.begin(); iter !=
           nodes_map.end(); iter++)
   {
      node* node_ptr=iter->second;
      int other_photo_ID=node_ptr->get_ID();
      if (photo_ID==other_photo_ID) continue;
      
      plane* curr_plane_ptr=get_common_image_plane(photo_ID,other_photo_ID);
      if (curr_plane_ptr != NULL) 
      {
         int other_photo_order=get_photo_order_given_index(other_photo_ID);
         overlapping_imageplane_orders.push_back(other_photo_order);
         cout << "other_photo_ID = " << other_photo_ID 
              << " other_photo_order = " << other_photo_order << endl;
      }
   }
   return overlapping_imageplane_orders;
}

// ==========================================================================
// Message handling member functions
// ==========================================================================

// Member function issue_add_vertex_message takes in *mover_ptr and
// generates an ActiveMQ "ADD_VERTEX" message.  The mover's type, ID
// and RGB color are encoded as key-value string pair properties.

void photogroup::issue_add_vertex_message(photograph* photograph_ptr)
{
//   cout << "inside photogroup::issue_add_vertex_message()" << endl;
//   cout << "*photograph_ptr = " << *photograph_ptr << endl;
//   cout << "Messenger_ptr = " << Messenger_ptr << endl;
//   cout << "Messenger_ptr->connected_to_broker_flag() = "
//        << Messenger_ptr->connected_to_broker_flag() << endl;

   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;
   
// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

// Adding/selecting vertex example:

   command="ADD_VERTEX";
 
   key="TYPE";
   value="PHOTO";		
   properties.push_back(property(key,value));

   key="ID";
   value=stringfunc::number_to_string(photograph_ptr->get_ID());
   properties.push_back(property(key,value));

/*
   string annotation_label=mover_ptr->get_annotation_label();
   if (annotation_label.size() > 0)
   {
      key="ANNOTATION_LABEL";
      value=annotation_label;
      properties.push_back(property(key,value));
   }
*/
 
   key="THUMBNAIL_URL";
//   value="http://127.0.0.1:8080/photosynth/Manhattan/images/thumbnails/";
   value="http://127.0.0.1:8080/photosynth/nyc_1000/images/thumbnails/";
   value += filefunc::getbasename(
      videofunc::get_thumbnail_filename(photograph_ptr->get_filename()));
   properties.push_back(property(key,value));

/*
// Scale size of photo nodes by factors proportional to log of their
// total number of SIFT feature matches:

   int n_matching_SIFT_features=
      photograph_ptr->get_n_matching_SIFT_features();
   double ln_n_features=log(n_matching_SIFT_features);
   const double ln_start=log(20.0);
   const double ln_stop=log(2000.0);

   double weight_factor=0.5;
   double relative_size_start=weight_factor*log10(2.0);
   double relative_size_stop=weight_factor*log10(2000.0);
   double relative_size=relative_size_start;
   if (n_matching_SIFT_features > 2)
   {
      relative_size=relative_size_start+
         (ln_n_features-ln_start)/(ln_stop-ln_start)*(
            relative_size_stop-relative_size_start);
   }
   key="RELATIVE_SIZE";
   value=stringfunc::number_to_string(relative_size);
   properties.push_back(property(key,value));
*/

   key="RGB COLOR";
   colorfunc::RGB curr_RGB=compute_node_color(photograph_ptr);
   value=stringfunc::number_to_string(curr_RGB.first)+" ";
   value += stringfunc::number_to_string(curr_RGB.second)+" ";
   value += stringfunc::number_to_string(curr_RGB.third);
   properties.push_back(property(key,value));

//   for (unsigned int p=0; p<properties.size(); p++)
//   {
//      cout << "p = " << p
//           << " key = " << properties[p].first
//           << " value = " << properties[p].second << endl;
//   }

//   cout << "Messenger_ptr->get_topicName() = "
//        << Messenger_ptr->get_topicName() << endl;

   Messenger_ptr->sendTextMessage(command,properties);
}

// ---------------------------------------------------------------------
// Member function issue_add_edge_message() takes in *photograph1_ptr
// and *photograph_ptr and generates an ActiveMQ "ADD_EDGE" message.
// The two movers types and IDs are encoded as key-value string pair
// properties.

void photogroup::issue_add_edge_message(
   photograph* photograph1_ptr,photograph* photograph2_ptr,
   int n_SIFT_matches)
{
//   cout << "inside photogroup::issue_add_edge_message()" << endl;
//   cout << "Messenger_ptr = " << Messenger_ptr << endl;
//   cout << "n_SIFT_matches = " << n_SIFT_matches << endl;

   if (photograph1_ptr==NULL || photograph2_ptr==NULL)
   {
      cout << "Error in photogroup::issue_add_edge_message()!" << endl;
      cout << "photograph1_ptr = " << photograph1_ptr 
           << " photograph2_ptr = " << photograph2_ptr << endl;
      outputfunc::enter_continue_char();
      return;
   }

   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) 
   {
      cout << "inside photogroup::issue_add_edge_message()" << endl;
      cout << "Messenger_ptr = " << Messenger_ptr << endl;
      cout << "Messenger_ptr->connected_to_broker_flag() = "
           << Messenger_ptr->connected_to_broker_flag() << endl;
      return;
   }

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   command="ADD_EDGE";

   key="TYPE1";
   value="PHOTO";
   properties.push_back(property(key,value));

   key="ID1";
   value=stringfunc::number_to_string(photograph1_ptr->get_ID());
   properties.push_back(property(key,value));

   key="TYPE2";
   value="PHOTO";	       
   properties.push_back(property(key,value));

   key="ID2";
   value=stringfunc::number_to_string(photograph2_ptr->get_ID());
   properties.push_back(property(key,value));

// Set color of edges linking one photo to another based upon number of 
// SIFT features which they share in common:


// NYC 318 set:

//   threshold.push_back(16);
//   threshold.push_back(26);
//   threshold.push_back(46);
//   threshold.push_back(76);
//   threshold.push_back(116);
//   threshold.push_back(166);

// NYC 1012 set:

   edge_weights_threshold.push_back(5);
   edge_weights_threshold.push_back(10);
   edge_weights_threshold.push_back(16);
   edge_weights_threshold.push_back(23);
   edge_weights_threshold.push_back(30);
   edge_weights_threshold.push_back(38);

   vector<double> hue;
   hue.push_back(300);
   hue.push_back(240);
   hue.push_back(180);
   hue.push_back(120);
   hue.push_back(60);
   hue.push_back(0);

   double curr_hue=0;
   if (n_SIFT_matches < edge_weights_threshold[0])
   {
      curr_hue=hue[0];
   }
   else if (n_SIFT_matches >= edge_weights_threshold[0] &&
            n_SIFT_matches < edge_weights_threshold[1])
   {
      curr_hue=hue[0]+(n_SIFT_matches-edge_weights_threshold[0])/
         (edge_weights_threshold[1]-edge_weights_threshold[0])*(hue[1]-hue[0]);
   }
   else if (n_SIFT_matches >= edge_weights_threshold[1] &&
            n_SIFT_matches < edge_weights_threshold[2])
   {
      curr_hue=hue[1]+(n_SIFT_matches-edge_weights_threshold[1])/
         (edge_weights_threshold[2]-edge_weights_threshold[1])*(hue[2]-hue[1]);
   }
   else if (n_SIFT_matches >= edge_weights_threshold[2] &&
            n_SIFT_matches < edge_weights_threshold[3])
   {
      curr_hue=hue[2]+(n_SIFT_matches-edge_weights_threshold[2])/
         (edge_weights_threshold[3]-edge_weights_threshold[2])*(hue[3]-hue[2]);
   }
   else if (n_SIFT_matches >= edge_weights_threshold[3] &&
            n_SIFT_matches < edge_weights_threshold[4])
   {
      curr_hue=hue[3]+(n_SIFT_matches-edge_weights_threshold[3])/
         (edge_weights_threshold[4]-edge_weights_threshold[3])*(hue[4]-hue[3]);
   }
   else if (n_SIFT_matches >= edge_weights_threshold[4] &&
            n_SIFT_matches < edge_weights_threshold[5])
   {
      curr_hue=hue[4]+(n_SIFT_matches-edge_weights_threshold[4])/
         (edge_weights_threshold[5]-edge_weights_threshold[4])*(hue[5]-hue[4]);
   }
   else if (n_SIFT_matches > edge_weights_threshold[5])
   {
      curr_hue=hue[5];
   }

/*
   double ln_n_features=log(n_SIFT_matches);
   const double ln_start=log(16.0);
   const double ln_stop=log(76.0);

   const double hue_start=300;		// purple
   const double hue_stop=0;		// red
   double hue=hue_start;
   if (ln_n_features < ln_start)
   {
      hue=hue_start;
   }
   else if (ln_n_features > ln_stop)
   {
      hue=hue_stop;
   }
   else
   {
      hue=hue_start+(ln_n_features-ln_start)/
         (ln_stop-ln_start)*(hue_stop-hue_start);
   }
*/

   colorfunc::HSV curr_hsv;
   curr_hsv.first=curr_hue;
   curr_hsv.second=1.0;
   curr_hsv.third=1.0;
   colorfunc::RGB curr_RGB=colorfunc::hsv_to_RGB(curr_hsv);

//   curr_RGB.first=1;
//   curr_RGB.second=1;
//   curr_RGB.third=1;

   key="RGB COLOR";
   value=stringfunc::number_to_string(curr_RGB.first)+" ";
   value += stringfunc::number_to_string(curr_RGB.second)+" ";
   value += stringfunc::number_to_string(curr_RGB.third);
   properties.push_back(property(key,value));

//   cout << "Before call to sendTextMessage()" << endl;
//   cout << "command = " << command << endl;
//   for (unsigned int p=0; p<properties.size(); p++)
//   {
//      cout << "p = " << p 
//           << " properties[p].first = " << properties[p].first
//           << " properties[p].second = " << properties[p].second
//           << endl;
//   }
   Messenger_ptr->sendTextMessage(command,properties);

//   cout << "at end of photogroup::issue_add_edge_message()" << endl;
}

// ---------------------------------------------------------------------
// Member function issue_show_banner_message() 

void photogroup::issue_show_banner_message()
{
//   cout << "inside photogroup::issue_show_banner_message()" << endl;
//   cout << "Messenger_ptr = " << Messenger_ptr << endl;

   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   command="SHOW_BANNER";

   Messenger_ptr->sendTextMessage(command,properties);
}

// ---------------------------------------------------------------------
void photogroup::issue_hide_banner_message()
{
//   cout << "inside photogroup::issue_hide_banner_message()" << endl;
//   cout << "Messenger_ptr = " << Messenger_ptr << endl;
//   cout << "Messenger_ptr->connected_to_broker_flag() = "
//        << Messenger_ptr->connected_to_broker_flag() << endl;

   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   command="HIDE_BANNER";

   Messenger_ptr->sendTextMessage(command,properties);
}

// ---------------------------------------------------------------------
// Member function issue_set_banner_message() 

void photogroup::issue_set_banner_message(string banner)
{
//   cout << "inside photogroup::issue_set_banner_message()" << endl;
//   cout << "Messenger_ptr = " << Messenger_ptr << endl;

   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   command="SET_BANNER";

   key="MESSAGE";
   value=banner;
   properties.push_back(property(key,value));

   Messenger_ptr->sendTextMessage(command,properties);
}

// ==========================================================================
// Graph building via GraphML JSON commands
// ==========================================================================

// Member function issue_build_graph_message()

void photogroup::issue_build_graph_message(string JSON_URL)
{
   cout << "inside photogroup::issue_build_graph()" << endl;
//   cout << "Messenger_ptr = " << Messenger_ptr << endl;
//   cout << "Messenger_ptr->connected_to_broker_flag() = "
//        << Messenger_ptr->connected_to_broker_flag() << endl;

   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;
   
// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   command="SET_GRAPH";
 
   key="JSON_URL";
   value=JSON_URL;

   properties.push_back(property(key,value));

//   for (unsigned int p=0; p<properties.size(); p++)
//   {
//      cout << "p = " << p
//           << " key = " << properties[p].first
//           << " value = " << properties[p].second << endl;
//   }

//   cout << "Messenger_ptr->get_topicName() = "
//        << Messenger_ptr->get_topicName() << endl;

   bool print_msg_flag=true;
   Messenger_ptr->sendTextMessage(command,properties,print_msg_flag);
}

// ==========================================================================
// Photo ordering member functions
// ==========================================================================

// Member function order_photos_by_their_scores()

void photogroup::order_photos_by_their_scores()
{
   cout << "inside photogroup::order_photos_by_their_scores()" << endl;

   vector<photograph*> photo_ptrs;
   vector<double> scores;
   for (unsigned int n=0; n<get_n_photos(); n++)
   {
      photograph* photo_ptr=get_photograph_ptr(n);
      photo_ptrs.push_back(photo_ptr);
      scores.push_back(photo_ptr->get_score());
   }
   templatefunc::Quicksort_descending(scores,photo_ptrs);

   score_ordered_photo_ptrs.clear();
   for (unsigned int n=0; n<get_n_photos(); n++)
   {
      score_ordered_photo_ptrs.push_back(photo_ptrs[n]);
   }
   
// Write out photo filenames so that best scored appears last,
// next-to-best scored appears next-to-last, etc...

   for (unsigned int order=get_n_photos()-1; order>=0; order--)
   {
      photograph* curr_photo_ptr=get_score_ordered_photo(order);
      cout << "order = " << order
           << " score = " << curr_photo_ptr->get_score()
           << " ID = " << curr_photo_ptr->get_ID() 
           << " photo = " << curr_photo_ptr->get_filename() << endl;
      cout << "geolocation = " << curr_photo_ptr->get_geolocation()
           << endl;
   }
}

// ---------------------------------------------------------------------
// Member function get_score_ordered_photo() returns the ith-best
// photo ordered according to polyhedra bbox projection scores.

photograph* photogroup::get_score_ordered_photo(int i)
{
//   cout << "inside photogroup::get_score_ordered_photo()" << endl;

   if (i < 0 || i >= int(get_n_photos())) return NULL;

   return score_ordered_photo_ptrs[i];
}

// -------------------------------------------------------------------------
// Member function print_compositing_order()

void photogroup::print_compositing_order()
{
   string banner="Computing photograph compositing order:";
   outputfunc::write_banner(banner);

   unsigned int n_photos=get_n_photos();
   
   cout << endl;
   cout << "Pass compositing order:" << endl;
   cout << "n_photos = " << n_photos << endl;
   for (unsigned int p=0; p<n_photos; p++)
   {
      int photo_ID=get_photo_order()[p];
      cout << "Order = " << p 
           << " photo ID = " << photo_ID
           << " filename = " << get_ordered_photograph_ptr(p)->get_filename() 
           << endl;
   }
//   outputfunc::enter_continue_char();
}

// ==========================================================================
// Photo clustering member functions
// ==========================================================================

// Member function generate_cluster_photogroup() instantiates member
// *cluster_photogroup_ptr and copies metadata from *this into the new
// photogroup.  It generates the cluster photogroup's super nodes
// using precalculated information stored in clusters_map.  This
// method next forms super edges by integrating weights for all edges
// leaving each super node.  The pointer cluster_photogroup_ptr is
// returned.  

photogroup* photogroup::generate_cluster_photogroup()
{
   cout << "inside photogroup::generate_cluster_photogroup()" << endl;

   cluster_photogroup_ptr=new photogroup(get_ID()+1,get_level()+1);
   cluster_photogroup_ptr->set_parent_identity(-1);
   cluster_photogroup_ptr->set_base_URL(get_base_URL());
   cluster_photogroup_ptr->set_UTM_zonenumber(get_UTM_zonenumber());
   cluster_photogroup_ptr->set_northern_hemisphere_flag(
      get_northern_hemisphere_flag());

// First generate cluster photogroup's super nodes using precalculated
// information stored within clusters_map:

   unsigned int n_clusters=clusters_map.size();
   cout << "n_clusters = " << n_clusters << endl;

   for (unsigned int cluster_ID=0; cluster_ID<n_clusters; cluster_ID++)
   {
      vector<int> clustered_node_IDs=clusters_map[cluster_ID];

      double max_degree=NEGATIVEINFINITY;
      vector<double> U,V;
      photograph* representative_photo_ptr=NULL;
      for (unsigned int i=0; i<clustered_node_IDs.size(); i++)
      {
         photograph* curr_photo_ptr=get_photograph_ptr(clustered_node_IDs[i]);
         U.push_back(curr_photo_ptr->get_Uposn());
         V.push_back(curr_photo_ptr->get_Vposn());
         double curr_degree=curr_photo_ptr->get_degree();
         if (curr_degree > max_degree)
         {
            max_degree=curr_degree;
            representative_photo_ptr=curr_photo_ptr;
         }
      } // loop over index i labeling nodes within current cluster
      double Uavg=mathfunc::mean(U);
      double Vavg=mathfunc::mean(V);

      photograph* cluster_photo_ptr=new photograph(
         representative_photo_ptr->get_filename(),
         representative_photo_ptr->get_xdim(),
         representative_photo_ptr->get_ydim(),cluster_ID);

      cluster_photo_ptr->set_parent_ID(-1);
//      cluster_photo_ptr->set_parent_ID(cluster_ID);
      cluster_photo_ptr->set_Uposn(Uavg);
      cluster_photo_ptr->set_Vposn(Vavg);

      cluster_photogroup_ptr->add_node(cluster_photo_ptr);
      cluster_photogroup_ptr->compute_node_cluster_color(
         cluster_photo_ptr,cluster_ID);
      
   } // loop over cluster_ID index labeling clusters

// Next form cluster photogroup's super edges by integrating weights
// for all edges leaving all nodes within each supernode:

   cout << "Before forming cluster photogroup's super edges:" << endl;

   for (unsigned int cluster_ID=0; cluster_ID<n_clusters; cluster_ID++)
   {
      cout << "Second cluster_ID = " << cluster_ID << endl;
      vector<int> clustered_node_IDs=clusters_map[cluster_ID];

      vector<double> cluster_edge_weights;
      for (unsigned int neighbor_cluster_ID=0; neighbor_cluster_ID<n_clusters; 
           neighbor_cluster_ID++)
      {
         cluster_edge_weights.push_back(0);
      }

      for (unsigned int i=0; i<clustered_node_IDs.size(); i++)
      {
         node* curr_node_ptr=get_node_ptr(clustered_node_IDs[i]);
         vector<graph_edge*> graph_edge_ptrs=
            curr_node_ptr->get_graph_edge_ptrs();
         for (unsigned int e=0; e<graph_edge_ptrs.size(); e++)
         {
            graph_edge* curr_edge_ptr=graph_edge_ptrs[e];
            double curr_weight=curr_edge_ptr->get_weight();
            int neighbor_node_ID=curr_node_ptr->get_neighbor_node_ID(
               curr_edge_ptr);
            node* neighbor_node_ptr=get_ordered_node_ptr(neighbor_node_ID);
            unsigned int neighbor_cluster_ID=
               neighbor_node_ptr->get_parent_ID();
            if (neighbor_cluster_ID==cluster_ID) continue;
            cluster_edge_weights[neighbor_cluster_ID]=
               cluster_edge_weights[neighbor_cluster_ID]+curr_weight;
         } // loop over e index labeling edges of *curr_node_ptr
      } // loop over index i labeling nodes within current cluster
      
      cout << "Before loop over neighbor_cluster_ID" << endl;
      cout << "n_clusters = " << n_clusters << endl;
      for (unsigned int neighbor_cluster_ID=0; neighbor_cluster_ID<n_clusters; 
           neighbor_cluster_ID++)
      {
         double curr_cluster_edge_weight=cluster_edge_weights[
            neighbor_cluster_ID];
         if (nearly_equal(curr_cluster_edge_weight,0)) continue;

         cluster_photogroup_ptr->add_graph_edge(
            cluster_ID,neighbor_cluster_ID,curr_cluster_edge_weight);
      } // loop over neighbor_cluster_ID index
      
      cout << "At end of loop over cluster_ID" << endl;

   } // loop over cluster_ID index labeling clusters

   cout << "Before writing out n_nodes and n_edges" << endl;
   cout << "cluster_photogroup_ptr = " << cluster_photogroup_ptr << endl;
   cout << "cluster_photogroup_ptr->get_n_nodes() = "
        << cluster_photogroup_ptr->get_n_nodes() << endl;
   cout << "cluster_photogroup_ptr->get_n_edges() = " 
        << cluster_photogroup_ptr->get_n_graph_edges() << endl;

   return cluster_photogroup_ptr;
}

// -------------------------------------------------------------------------
// Member function generate_grandparent_photogroup()

photogroup* photogroup::generate_grandparent_photogroup()
{
   cout << "inside photogroup::generate_grandparent_photogroup()" << endl;

   cluster_photogroup_ptr=new photogroup(get_ID()+1,get_level()+1);
   cluster_photogroup_ptr->set_parent_identity(-1);
   cluster_photogroup_ptr->set_base_URL(get_base_URL());
   cluster_photogroup_ptr->set_UTM_zonenumber(get_UTM_zonenumber());
   cluster_photogroup_ptr->set_northern_hemisphere_flag(
      get_northern_hemisphere_flag());

// First generate grandparent photogroup's nodes using precalculated
// information stored within clusters_map:

   unsigned int n_clusters=clusters_map.size();
   cout << "n_clusters = " << n_clusters << endl;

   for (unsigned int n=0; n<n_clusters; n++)
   {
      int grandparent_ID=n;
      twovector cluster_COM=graphfunc::compute_cluster_COM(
         this,this,grandparent_ID);
      photograph* representative_photo_ptr=
         static_cast<photograph*>(max_relative_size_node_in_cluster(
            grandparent_ID));

      photograph* cluster_photo_ptr=new photograph(
         representative_photo_ptr->get_filename(),
         representative_photo_ptr->get_xdim(),
         representative_photo_ptr->get_ydim(),grandparent_ID);

      cluster_photo_ptr->set_parent_ID(-1);
      cluster_photo_ptr->set_posn(cluster_COM);
      cluster_photogroup_ptr->add_node(cluster_photo_ptr);
   } // loop over index n labeling grandparent clusters

// Next form grandparent super edges by integrating weights for all
// edges leaving all parent nodes:

   for (unsigned int cluster_ID=0; cluster_ID<n_clusters; cluster_ID++)
   {
      vector<int> parent_node_IDs=clusters_map[cluster_ID];

      vector<double> cluster_edge_weights;
      for (unsigned int neighbor_cluster_ID=0; neighbor_cluster_ID<n_clusters; 
           neighbor_cluster_ID++)
      {
         cluster_edge_weights.push_back(0);
      }

      for (unsigned int i=0; i<parent_node_IDs.size(); i++)
      {
         node* curr_node_ptr=get_node_ptr(parent_node_IDs[i]);

         vector<graph_edge*> graph_edge_ptrs=
            curr_node_ptr->get_graph_edge_ptrs();
         for (unsigned int e=0; e<graph_edge_ptrs.size(); e++)
         {
            graph_edge* curr_edge_ptr=graph_edge_ptrs[e];
            double curr_weight=curr_edge_ptr->get_weight();
            int neighbor_node_ID=curr_node_ptr->get_neighbor_node_ID(
               curr_edge_ptr);
            node* neighbor_node_ptr=get_ordered_node_ptr(neighbor_node_ID);
            unsigned int neighbor_cluster_ID=
               neighbor_node_ptr->get_parent_ID();
            if (neighbor_cluster_ID==cluster_ID) continue;
            cluster_edge_weights[neighbor_cluster_ID]=
               cluster_edge_weights[neighbor_cluster_ID]+curr_weight;
         } // loop over e index labeling edges of *curr_node_ptr
      } // loop over index i labeling nodes within current cluster
      
      for (unsigned int neighbor_cluster_ID=0; neighbor_cluster_ID<n_clusters; 
           neighbor_cluster_ID++)
      {
         double curr_cluster_edge_weight=cluster_edge_weights[
            neighbor_cluster_ID];
         if (nearly_equal(curr_cluster_edge_weight,0)) continue;

         cluster_photogroup_ptr->add_graph_edge(
            cluster_ID,neighbor_cluster_ID,curr_cluster_edge_weight);
      } // loop over neighbor_cluster_ID index
      
   } // loop over cluster_ID index labeling clusters

   cout << "cluster_photogroup_ptr->get_n_nodes() = "
        << cluster_photogroup_ptr->get_n_nodes() << endl;
   cout << "cluster_photogroup_ptr->get_n_edges() = " 
        << cluster_photogroup_ptr->get_n_graph_edges() << endl;

   return cluster_photogroup_ptr;
}

// ==========================================================================
// SQL database population member functions
// ==========================================================================

// Member function write_SQL_insert_photo_commands() loops over all
// photos within the current photogroup.  It writes SQL insert
// commands to the specified SQL_filename which are needed to populate
// the photo table of the data_network postgis database.

void photogroup::write_SQL_insert_photo_commands(string SQL_photo_filename)
{
   cout << "inside photogroup::write_SQL_insert_photo_commands()" << endl;
   cout << "SQL_photo_filename = " << SQL_photo_filename << endl;

   ofstream SQL_photo_stream;
   filefunc::openfile(SQL_photo_filename,SQL_photo_stream);

// Generate SQL insert commands for photos:

//   int n_photos=get_n_photos();
//   cout << "n_photos = " << n_photos << endl;
   for (NODES_MAP::const_iterator iter=nodes_map.begin(); iter !=
           nodes_map.end(); iter++)
   {
      photograph* photograph_ptr=static_cast<photograph*>(iter->second);
//      cout << "photo_ptr = " << photograph_ptr << endl;
      SQL_photo_stream << output_photo_to_SQL(photograph_ptr) << endl;
   }
   filefunc::closefile(SQL_photo_filename,SQL_photo_stream);
}

// ---------------------------------------------------------------------
// Member function output_photo_to_SQL() takes in a particular
// *photograph_ptr.  It extracts the metadata from *photograph_ptr
// needed to populate the columns of the photo table within the
// data_network postgis database.

string photogroup::output_photo_to_SQL(photograph* photograph_ptr)
{
//   cout << "inside photogroup::output_photo_to_SQL()" << endl;

   int photo_ID=photograph_ptr->get_ID();
   int time_stamp=
      photograph_ptr->get_clock().secs_elapsed_since_reference_date();
   string photo_URL="";
   string thumbnail_URL="";
   string photo_filename=photograph_ptr->get_filename();
//   cout << "photo_filename = " << photo_filename << endl;
   if (photo_filename.size() > 0)
   {
      photo_URL = base_URL+
         filefunc::getbasename(photograph_ptr->get_filename());
      thumbnail_URL = base_URL+"thumbnails/"+
         filefunc::getbasename(videofunc::get_thumbnail_filename(
            photograph_ptr->get_filename()));
//      cout << "thumbnail_URL = " << thumbnail_URL << endl;
   }

   camera* camera_ptr=photograph_ptr->get_camera_ptr();
   geopoint camera_geolocation(*(camera_ptr->get_geolocation_ptr()));
//   cout << "Camera_geolocation = " << camera_geolocation << endl;
   double longitude=camera_geolocation.get_longitude();
   double latitude=camera_geolocation.get_latitude();
   double altitude=camera_geolocation.get_altitude();
//   cout << "lon = " << longitude
//        << " lat = " << latitude
//        << " alt = " << altitude << endl;

   double az = camera_ptr->get_rel_az()*180/PI;
   double el = camera_ptr->get_rel_el()*180/PI;
   double roll = camera_ptr->get_rel_roll()*180/PI;
   double focal_param=camera_ptr->get_fu();

   unsigned int thumbnail_xdim,thumbnail_ydim;
   videofunc::get_thumbnail_dims(
      photograph_ptr->get_xdim(),photograph_ptr->get_ydim(),
      thumbnail_xdim,thumbnail_ydim);

   string insert_command = photodbfunc::generate_insert_photo_SQL_command(
      photo_ID,time_stamp,photo_URL,
      photograph_ptr->get_xdim(),photograph_ptr->get_ydim(),
      thumbnail_URL,thumbnail_xdim,thumbnail_ydim,
      longitude,latitude,altitude,
      az,el,roll,focal_param);

//   cout << "insert_command = " << insert_command << endl;
   return insert_command;
}

// ==========================================================================
// Virtual camera member functions
// ==========================================================================

// Member function generate_blank_photograph() instantiates a blank
// image for a virtual camera.  It takes in the camera's horizontal
// and vertical fields-of-view as well as its az, el and roll angles
// measured in degrees.

photograph* photogroup::generate_blank_photograph(
   double horiz_FOV,double vert_FOV,
   double az, double el,double roll,const threevector& camera_posn,
   double frustum_sidelength,double blank_grey_level)
{
//   cout << "inside photogroup::generate_blank_photograph()" << endl;

   double FOV_u=horiz_FOV*PI/180;
   double FOV_v=vert_FOV*PI/180;

   double f,aspect_ratio;
   camerafunc::f_and_aspect_ratio_from_horiz_vert_FOVs(
      FOV_u,FOV_v,f,aspect_ratio);
//   cout << "f = " << f << " aspect ratio = " << aspect_ratio << endl;
   double fu=f;
   double fv=fu;

   int n_horiz_pixels,n_vertical_pixels;
   if (aspect_ratio > 1)
   {
      n_vertical_pixels=1000;
      n_horiz_pixels=aspect_ratio*n_vertical_pixels;
   }
   else
   {
      n_horiz_pixels=1000;
      n_vertical_pixels=n_horiz_pixels/aspect_ratio;
   }
//   cout << "n_horiz_pixels = " << n_horiz_pixels << endl;
//   cout << "n_vertical_pixels = " << n_vertical_pixels << endl;

   double U0=0.5*aspect_ratio;
   double V0=0.5;

// Note added on Monday, 3/7/11 at 10:32 am.  For reasons we don't
// understand, the following call to generate_blank_imagefile() yields a
// seg fault.  We'll worry about this later...

//   string blank_filename=videofunc::generate_blank_imagefile(
//      n_horiz_pixels,n_vertical_pixels);

   string blank_filename="/tmp/blank.png";
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle_ptr->generate_blank_image_file(
      n_horiz_pixels,n_vertical_pixels,blank_filename,blank_grey_level);
   delete texture_rectangle_ptr;

   photograph* photo_ptr=generate_single_photograph(
      blank_filename,n_horiz_pixels,n_vertical_pixels,
      fu,fv,U0,V0,az,el,roll,camera_posn,frustum_sidelength);

   return photo_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_blank_imagefile() instantiates a blank
// image for a virtual camera.  

string photogroup::generate_blank_imagefile(
   int n_horiz_pixels,int n_vertical_pixels)
{
   cout << "inside photogroup::generate_blank_image()" << endl;

   cout << "n_horiz_pixels = " << n_horiz_pixels << endl;
   cout << "n_vertical_pixels = " << n_vertical_pixels << endl;

   double grey_level=0.5;
   string blank_filename="/tmp/blank.png";
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle_ptr->generate_blank_image_file(
      n_horiz_pixels,n_vertical_pixels,blank_filename,grey_level);
   delete texture_rectangle_ptr;

   return blank_filename;
}

/*
// ---------------------------------------------------------------------
// Member function export_bundle_dot_out()

void photogroup::export_bundle_dot_out()
{
   cout << "inside photogroup::export_bundle_dot_out()" << endl;

   string bundle_filename="bundle.out";
   ofstream bundlestream;
   filefunc::openfile(bundle_filename,bundlestream);
   bundlestream << "# Bundle file v0.3" << endl;
   bundlestream << get_n_photos() << " " << n_features << endl;

   for (unsigned int n=0; n<get_n_photos(); n++)
   {
      photograph* photo_ptr=get_photograph_ptr(n);
      camera* camera_ptr=camera_ptrs[n];

      int xdim=photo_ptr->get_xdim();
      int ydim=photo_ptr->get_ydim();
      double aspect_ratio=double(xdim)/double(ydim);
      
      double f=camera_ptr->get_fu();
      double f_noah=fabs(f)*ydim;
      double k1=0;
      double k2=0;
      bundlestream << stringfunc::scinumber_to_string(f_noah)
                   << " " << k1 << " " << k2 << endl;

//      double FOV_u,FOV_v;
//      camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
//         f,aspect_ratio,FOV_u,FOV_v);
//      cout << "FOV_u = " << FOV_u*180/PI << " FOV_v = " << FOV_v*180/PI
//           << endl;

      rotation R_noah;
      threevector t_noah;
      camera_ptr->convert_world_to_bundler_coords(R_noah,t_noah);

      for (unsigned int r=0; r<3; r++)
      {
         for (unsigned int c=0; c<3; c++)
         {
            bundlestream << stringfunc::scinumber_to_string(R_noah.get(r,c),9)
                         << " ";
         }
         bundlestream << endl;
      }
      bundlestream << stringfunc::scinumber_to_string(t_noah.get(0)) << " "
                   << stringfunc::scinumber_to_string(t_noah.get(1)) << " "
                   << stringfunc::scinumber_to_string(t_noah.get(2)) << endl;

      cout <<  "R_noah = " << R_noah << endl;
      cout << "t_noah = " << t_noah << endl;

   } // loop over index n labeling cameras

   filefunc::closefile(bundle_filename,bundlestream);
   
   banner="Exported "+bundle_filename;
   outputfunc::write_big_banner(banner);

}
*/
