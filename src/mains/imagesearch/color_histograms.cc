// ==========================================================================
// Program COLOR_HISTOGRAMS loops over all images corresponding to a
// specified graph hierarchy.  It computes a color histogram for each
// image.  Histogram colors fractions as well as dominant color names
// are saved within the image_color_histograms table of the IMAGERY
// database.
// ==========================================================================
// Last updated on 3/7/12; 3/8/12; 3/9/12
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "video/imagesdatabasefuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   int hierarchy_ID=0;
   cout << "Enter graph hierarchy ID" << endl;
   cin >> hierarchy_ID;

   int graph_ID=0;

   vector<int> campaign_IDs,mission_IDs,image_IDs,datum_IDs;
   vector<string> URLs;
   
   imagesdatabasefunc::get_image_URLs(
      postgis_db_ptr,hierarchy_ID,graph_ID,
      campaign_IDs,mission_IDs,image_IDs,datum_IDs,URLs);

   int image_skip=1;
//   int image_skip=200;
   for (unsigned int i=0; i<URLs.size(); i += image_skip)
   {
      if (i%10==0) cout << i << " " << flush;
      
      string photo_filename=URLs[i];
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         photo_filename,NULL);
      if (texture_rectangle_ptr->get_VideoType()==texture_rectangle::unknown)
      {
         cout << "Could not read in image from file!" << endl;
         continue;
      }

      bool generate_quantized_image_flag=false;
      if (i%2000==0) generate_quantized_image_flag=true;
      string output_filename="colors_"+stringfunc::integer_to_string(i,4)
         +".jpg";

      vector<double> color_histogram=videofunc::generate_color_histogram(
         generate_quantized_image_flag,texture_rectangle_ptr,
         output_filename);

      vector<string> color_label;
      color_label.push_back("red");
      color_label.push_back("orange");
      color_label.push_back("yellow");
      color_label.push_back("green");
      color_label.push_back("blue");
      color_label.push_back("purple");
      color_label.push_back("black");
      color_label.push_back("white");
      color_label.push_back("grey");
      color_label.push_back("brown");

// Sort color fractions and determine primary, secondary and tertiary
// colors for current image:

      vector<double> sorted_color_histogram;
      for (unsigned int c=0; c<color_histogram.size(); c++)
      {
         sorted_color_histogram.push_back(color_histogram[c]);
      }

      templatefunc::Quicksort_descending(sorted_color_histogram,color_label);
      string primary_color=color_label[0];
      string secondary_color=color_label[1];
      string tertiary_color=color_label[2];

      imagesdatabasefunc::insert_image_color_histogram(
         postgis_db_ptr,campaign_IDs[i],mission_IDs[i],image_IDs[i],
         datum_IDs[i],color_histogram[0],color_histogram[1],color_histogram[2],
         color_histogram[3],color_histogram[4],color_histogram[5],
         color_histogram[6],color_histogram[7],color_histogram[8],
         color_histogram[9],primary_color,secondary_color,tertiary_color);

//      cout << "primary = " << primary_color
//           << " secondary = " << secondary_color
//           << " tertiary = " << tertiary_color << endl;

      if (generate_quantized_image_flag)
      {
         cout << endl;
         string banner="Exported histogrammed version of "+photo_filename;
         outputfunc::write_big_banner(banner);
      }

      delete texture_rectangle_ptr;
   } // loop over index i labeling photo filenames
   cout << endl;
   
}

