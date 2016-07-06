// ========================================================================
// Program NULL_POLY_EXTERIORS reads in a text file containing user
// selected polygons which segment each image in a video into
// "interior" and "exterior" regions.  It then nulls all pixels lying
// within the polygons' exterior regions.  Finally, this program
// writes the modified imagery to a new, output video.
// ========================================================================
// Last updated on 12/7/06; 12/19/06; 2/4/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "general/filefuncs.h"
#include "osg/osg2D/MoviesGroup.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "video/VidFile.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ========================================================================

int main(int argc, char* argv[])
{

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Initialize constants and parameters read in from command line as
// well as ascii text file:

   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();

// Instantiate animation controller & key handler:

   AnimationController* AnimationController_ptr=new AnimationController();

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group);

// Read in polygons from previously stored text file which segment
// each image into interior regions over which histogram equalization
// should be performed and exterior regions which should be nulled:

//   string polygons_filename="polys.txt";
   string polygons_filename="mini_grey_lowell_polys.txt";
   if (!filefunc::ReadInfile(polygons_filename))
   {
      cout << "No polygon region text file found!" << endl;
   }

   vector<polygon> poly;
   unsigned int line_counter=0;

   while(line_counter < filefunc::text_line.size())
   {
//      int imagenumber=stringfunc::string_to_integer(
//         filefunc::text_line[line_counter++]);
      int n_vertices=stringfunc::string_to_integer(
         filefunc::text_line[line_counter++]);
//      cout << "imagenumber = " << imagenumber << " n_vertices = "
//           << n_vertices << endl;
      vector<threevector> Vertices;
      for (int n=0; n<n_vertices; n++)
      {
         vector<double> V=stringfunc::string_to_numbers(
            filefunc::text_line[line_counter++]);
         Vertices.push_back(threevector(V[0],V[1],V[2]));
      }
      poly.push_back(polygon(Vertices));
//      cout <<  "poly = " << poly.back() << endl;
   }

//   int n_final_images=3;
   int n_final_images=movie_ptr->get_Nimages();
   const int bytes_per_pixel=movie_ptr->getNchannels();

   string vid_filename="nulled_exterior_regions.vid";
   VidFile vid_out;
   vid_out.New_8U(vid_filename.c_str(),movie_ptr->getWidth(),
                  movie_ptr->getHeight(), n_final_images,
                  bytes_per_pixel);

   movie_ptr->get_texture_rectangle_ptr()->set_first_frame_to_display(0);
   for (int i=0; i<n_final_images; i++)
   {
      cout << i << " " << flush;
      movie_ptr->displayFrame(i);
      movie_ptr->null_region_outside_poly(poly[i]);

      vid_out.WriteFrame(
         movie_ptr->get_m_image_ptr(),movie_ptr->getWidth()*bytes_per_pixel);
   }
   outputfunc::newline();

}
