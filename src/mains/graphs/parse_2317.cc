// ==========================================================================
// Program PARSE_2317 imports MIT2317 node IDs versus their
// reconstructed relative easting,northing,altitude camera
// geolocations.  It assigns each node to one of n_itervals x
// n_intervals "sectors" based upon their easting,northing
// geocoordinates.  PARSE_2317 exports sector assignments versus node
// ID to an output text file.  It also generates a metafile plot where
// each node is colored according to its sector assignment.

// We wrote this utility program for graph label propagation testing
// purposes.

// ==========================================================================
// Last updated on 5/3/13; 5/4/13
// ==========================================================================

#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>

#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "plot/metafile.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   string node_xyz_filename="NodeID_vs_XYZ.txt";
   filefunc::ReadInfile(node_xyz_filename);
   
   vector<int> node_ID;
   vector<double> rel_easting,rel_northing,rel_alt;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_line=filefunc::text_line[i];
      if (curr_line.size() < 10) continue;
      vector<double> column_values=
         stringfunc::string_to_numbers(curr_line);
      if (column_values.size() < 4) continue;

      node_ID.push_back(column_values[0]);
      rel_easting.push_back(column_values[1]);
      rel_northing.push_back(column_values[2]);
      rel_alt.push_back(column_values[3]);
//      cout << "i = " << i << " nodeID = " << node_ID.back() << endl;
   }

// Find extremal easting & northing values:
   
   double max_rel_easting=mathfunc::maximal_value(rel_easting);
   double min_rel_easting=mathfunc::minimal_value(rel_easting);

   double max_rel_northing=mathfunc::maximal_value(rel_northing);
   double min_rel_northing=mathfunc::minimal_value(rel_northing);
   
   double max_rel_alt=mathfunc::maximal_value(rel_alt);
   double min_rel_alt=mathfunc::minimal_value(rel_alt);

   cout << "min_rel_easting = " << min_rel_easting
        << " max_rel_easting = " << max_rel_easting << endl;
   cout << "min_rel_northing = " << min_rel_northing
        << " max_rel_northing = " << max_rel_northing << endl;
   cout << "min_rel_alt = " << min_rel_alt
        << " max_rel_alt = " << max_rel_alt << endl;

// Divide all camera geolocations into n_intervals x n_intervals
// sectors:

   int n_intervals=3;
   double delta_easting=(max_rel_easting-min_rel_easting)/n_intervals;
   double delta_northing=(max_rel_northing-min_rel_northing)/n_intervals;
   double delta_alt=(max_rel_alt-min_rel_alt)/n_intervals;

   vector<int> class_freq;
   for (int i=0; i<n_intervals; i++)
   {
      for (int j=0; j<n_intervals; j++)
      {
         class_freq.push_back(0);
      }
   }

   string output_filename="NodeID_vs_classID.txt";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   
// Assign camera geolocations to distinct sectors:

   typedef map<int,vector<twovector>* > SECTOR_GEOLOCATION_MAP;
   SECTOR_GEOLOCATION_MAP sector_geolocation_map;
   SECTOR_GEOLOCATION_MAP::iterator iter;

   outstream << "# NodeID  ClassID" << endl << endl;
   for (int i=0; i<node_ID.size(); i++)
   {
      double curr_easting=rel_easting[i];
      double curr_northing=rel_northing[i];
      double curr_alt=rel_alt[i];

      int easting_index=-1;
      int northing_index=-1;
      for (int n=0; n<n_intervals; n++)
      {
         if (curr_easting >= min_rel_easting+n*delta_easting &&
         curr_easting <= min_rel_easting+(n+1)*delta_easting)
         {
            easting_index=n;
         }
         if (curr_northing >= min_rel_northing+n*delta_northing &&
         curr_northing <= min_rel_northing+(n+1)*delta_northing)
         {
            northing_index=n;
         }
      } // loop over index n labeling east-north-alt intervals

      int class_ID=northing_index*n_intervals+easting_index;
      cout << "node_ID = " << node_ID[i]
           << " easting_index = " << easting_index
           << " northing_index = " << northing_index
           << " class_ID = " << class_ID
           << endl;
      class_freq[class_ID]=class_freq[class_ID]+1;

      vector<twovector>* V_ptr=NULL;
      iter=sector_geolocation_map.find(class_ID);
      if (iter==sector_geolocation_map.end())
      {
         V_ptr=new vector<twovector>;
         sector_geolocation_map[class_ID]=V_ptr;
      }
      else
      {
         V_ptr=iter->second;
      }
      V_ptr->push_back(twovector(curr_easting,curr_northing));      

      outstream << node_ID[i] << "   " << class_ID << endl;
   } // loop over index i labeling nodes
   filefunc::closefile(output_filename,outstream);

// Generate metafile which colors camera geolocations by their
// sector assignments:

   metafile xyclass_metafile;
   string meta_filename="xyclass";
   xyclass_metafile.set_parameters(
      meta_filename,"Camera geolocations",
      "Relative Easting","Relative Northing",
      min_rel_easting,max_rel_easting,
      min_rel_northing,max_rel_northing);
   xyclass_metafile.openmetafile();
   xyclass_metafile.write_header();

   for (iter=sector_geolocation_map.begin(); iter !=
           sector_geolocation_map.end(); iter++)
   {
      int class_ID=iter->first;
      cout << "class_ID = " << class_ID << endl;
      cout << "color = " << colorfunc::getcolor(class_ID) << endl;
      string marker_color=colorfunc::getcolor(class_ID);

      vector<twovector>* V_ptr=iter->second;
      vector<double> eastings,northings;

      for (int k=0; k<V_ptr->size(); k++)
      {
         eastings.push_back(V_ptr->at(k).get(0));
         northings.push_back(V_ptr->at(k).get(1));
      }
      xyclass_metafile.write_markers(marker_color,eastings,northings);
   }
   xyclass_metafile.closemetafile();

// Convert exported metafile into PDF output:

   string unix_cmd="meta_to_pdf "+meta_filename;
   sysfunc::unix_command(unix_cmd);

   string banner="Exported PDF "+meta_filename+".pdf";
   outputfunc::write_big_banner(banner);

   int class_freq_sum=0;
   for (int k=0; k<class_freq.size(); k++)
   {
      cout << "k = " << k << " class_freq[k] = " << class_freq[k]
           << endl;
      class_freq_sum += class_freq[k];
   }
   cout << "class_freq_sum = " << class_freq_sum << endl;

}

