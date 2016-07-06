// ========================================================================
// Program VIDEOS_TEMPORAL_LAYOUT reads in a set of node and image IDs as
// well as epoch times from the IMAGERY database for some
// user-specified graph hierarchy.  (As of April 2013, we have stored
// video times relative to their starting frames within the epoch
// column of the IMAGERY database for Boston bombing YouTube clips.
// Each clip was arbitrarily assigned an absolute start time of
// midnight, April 15, 2013.)

// VIDEOS_TEMPORAL_LAYOUT generates a stacked cosines layout for
// (gx2,gy2) graph node coordinates which can serve as a reasonable
// timeline display for multiple video clips.  If boolean
// vertical_layout_flag==true, the sinusoids for multiple video clips
// are oriented vertically rather than horizontally.  The period for
// all cosines is assumed to equal 1 minute.  

// VIDEOS_TEMPORAL_LAYOUT exports gx2,gy2 coordinates to ascii output
// as well as updates gx2,gy2 columns in a SQL script which updates
// the nodes table of the IMAGERY database.  It also exports a SQL
// script which contains time stamp annotations that label the
// sinusoidal graph layout.  Each video clip is also labeled on its
// left-hand side.

// 	videos_temporal_layout --GIS_layer ./packages/imagery_metadata.pkg 

// ========================================================================
// Last updated on 4/19/13; 10/29/13; 10/30/13
// ========================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

//   bool vertical_layout_flag=false;
   bool vertical_layout_flag=true;

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string graphs_subdir=bundler_IO_subdir+"graphs/";
   string transcripts_subdir=bundler_IO_subdir+"transcripts/";
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();


// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   int hierarchy_ID=12;				// Boston bombing clips 1-6
   cout << "Enter ID for graph hierarchy:" << endl;
   cin >> hierarchy_ID;
   if (hierarchy_ID < 0) exit(-1);

   int start_clip_ID=0;
   cout << "Enter starting clip ID:" << endl;
   cin >> start_clip_ID;

// Store extracted clip IDs within STL map:

   typedef map<int,int> CLIP_IDS_MAP;
   CLIP_IDS_MAP clip_ids_map;
// independent int = clip ID
// dependent int = number of frames within video labeled by clip ID   

   unsigned int n_graphs,n_levels,n_connected_components;
   string hierarchy_description;
   graphdbfunc::retrieve_graph_hierarchy_metadata_from_database(
      postgis_db_ptr,hierarchy_ID,hierarchy_description,n_graphs,n_levels,
      n_connected_components);
   cout << "n_connected_components = " << n_connected_components
        << endl;
   
   int campaign_ID,mission_ID;
   imagesdatabasefunc::get_campaign_mission_IDs(
      postgis_db_ptr,hierarchy_ID,campaign_ID,mission_ID);
   cout << "campaign_ID = " << campaign_ID
        << " mission_ID = " << mission_ID << endl;

//  Retrieve UTM_zonenumber from world_regions table in IMAGERY database:

   int UTM_zonenumber=imagesdatabasefunc::retrieve_campaign_UTM_zonenumber(
      postgis_db_ptr,campaign_ID);
   cout << "UTM_zonenumber = " << UTM_zonenumber << endl;

   cout.precision(12);
   int graph_ID=0;
   vector<int> node_IDs;
   vector<double> epoch_times;
   vector<string> image_URLs;
   imagesdatabasefunc::retrieve_image_metadata_from_database(
      postgis_db_ptr,hierarchy_ID,graph_ID,
      node_IDs,epoch_times,image_URLs);

   int n_images=epoch_times.size();
   cout << "n_images = " << n_images << endl;

   templatefunc::Quicksort(epoch_times,node_IDs,image_URLs);

   double min_time=0;
   int i=0;
   while (min_time < 1000)
   {
      min_time=epoch_times[i++];
   }
//   double min_time=epoch_times.front();
   double max_time=epoch_times.back();

   Clock clock;
   clock.compute_UTM_zone_time_offset(UTM_zonenumber);
   clock.convert_elapsed_secs_to_date(min_time);
   string min_date_string=clock.YYYY_MM_DD_H_M_S();
   cout << "min_time = " << min_time << " = " << min_date_string
        << endl;

   clock.convert_elapsed_secs_to_date(max_time);
   string max_date_string=clock.YYYY_MM_DD_H_M_S();
   cout << "max_time = " << max_time << " = " << max_date_string 
        << endl;

   double time_interval=max_time-min_time;
   cout << "max_time - min_time = " << time_interval << " secs = "
        << time_interval/60.0 << " mins  = " 
        << time_interval/3600.0 << " hours = " 
        << time_interval/(24*3600.0) << " days" 
        << endl;

// Force Period to assume canonical time intervals:

   double time_interval_in_days=time_interval/(24*3600);
   double time_interval_in_months=time_interval/(24*3600*30);
   double time_interval_in_years=time_interval/(24*3600*365);
   cout << "time in days = " << time_interval_in_days << endl;
   cout << "time in months = " << time_interval_in_months << endl;
   cout << "time in years = " << time_interval_in_years << endl;

// For Boston Bombing videos, for Period = 1 minute:

   double Period=60;	// secs = 1 minute

   cout << "Period = " << Period << " secs = "
        << Period/60.0 << " mins = " 
        << Period/3600.0 << " hours = " 
        << Period/(24*3600.0) << " days" << endl;
   double omega=2*PI/Period;

   double n_Periods=(max_time-min_time)/Period;
   cout << "n_Periods = " << n_Periods << endl << endl;
   int n_complete_Periods=basic_math::mytruncate(n_Periods);

   double horizontal_stretch_factor=1;
   const double delta_gx_per_connected_component=
      1.5*horizontal_stretch_factor;
//   double gx_extent=delta_gx_per_connected_component*n_connected_components;
   double gx_extent=delta_gx_per_connected_component*(5*n_Periods);

   if (vertical_layout_flag) gx_extent *= 2;
   
   double gy_extent=1;
   double delta_gx_per_Period=gx_extent/n_Periods;
   cout << "gx_extent = " << gx_extent 
        << " gy_extent = " << gy_extent << endl;

   string temporal_filename=graphs_subdir+"temporal_layout.dat";
   ofstream temporal_stream;
   temporal_stream.precision(12);
   filefunc::openfile(temporal_filename,temporal_stream);
   temporal_stream << "# index  NodeID  t_renorm gx'  gy'" << endl;
   temporal_stream << endl;

   string temporal_layout_filename=
      graphs_subdir+"update_temporal_layout.sql";
   ofstream temporal_layout_stream;
   temporal_layout_stream.precision(12);
   filefunc::openfile(temporal_layout_filename,temporal_layout_stream);

   string SQL_timestamp_annotation_filename=
      graphs_subdir+"insert_timestamp_annotations.sql";
   ofstream SQL_timestamp_annotation_stream;
   filefunc::openfile(
      SQL_timestamp_annotation_filename,SQL_timestamp_annotation_stream);

   double sinusoid_separation=1;
   vector<string> update_SQL_commands;
   for (int i=0; i<n_images; i++)
   {
      outputfunc::update_progress_fraction(i,100,n_images);

      if (node_IDs[i] < 0) continue;

// As of 10/29/13, we assume image_basename is of the form
// clip_0025_frame-00030.jpg:

      string image_basename=filefunc::getbasename(image_URLs[i]);
//      cout << "image_basename = " << image_basename << endl;
      string basename_prefix=stringfunc::prefix(image_basename);
      string separator_chars="_-";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         basename_prefix,separator_chars);

//      for (int s=0; s<substrings.size(); s++)
//      {
//         cout << "s = " << s << " substrings[s] = " << substrings[s] << endl;
//      }
      
//      cout << "substrings[0] = " << substrings[0] << endl;
      int clip_ID=stringfunc::string_to_number(substrings[1]);
//      cout << "clip_ID = " << clip_ID << endl;      
//      int frame_ID=stringfunc::string_to_number(substrings[3]);

      CLIP_IDS_MAP::iterator iter=clip_ids_map.find(clip_ID);
      if (iter==clip_ids_map.end())
      {
         clip_ids_map[clip_ID]=1;
      }
      else
      {
         iter->second=iter->second+1;
      }

      double t=epoch_times[i];
      double t_renorm=t-min_time;

      const double A=1;
      double phase=0;
      sinusoid_separation=3*A;
      if (vertical_layout_flag)
      {
         phase=1.5*PI;
         sinusoid_separation=8*A;
      }

      double clip_vertical_offset=
         (clip_ID-start_clip_ID)*sinusoid_separation;

      double new_gx=0+(t-min_time)/(max_time-min_time)*gx_extent;      
      double new_gy=clip_vertical_offset+A*cos(omega*t_renorm+phase);

// Reset temporal graph coordinates for any node whose epoch time
// is less than min_time!

      if (t < min_time) 
      {
         new_gx=new_gy=-1;
      }

      if (vertical_layout_flag)
      {
         std::swap(new_gx,new_gy);
         new_gy *= -1;
      }

      temporal_stream 
         << i << "  "
         << node_IDs[i] << "  "
         << t_renorm << "  "
         << new_gx << "  "
         << new_gy << endl;

      string curr_update_SQL_cmd=
         graphdbfunc::generate_update_node_SQL_command(
            hierarchy_ID,graph_ID,node_IDs[i],new_gx,new_gy);
      update_SQL_commands.push_back(curr_update_SQL_cmd);
      temporal_layout_stream << curr_update_SQL_cmd << ";" << endl;

   } // loop over index i labeling images

   filefunc::closefile(temporal_layout_filename,temporal_layout_stream);

   string banner="Exported temporal layout to SQL script "+
      temporal_layout_filename;
   outputfunc::write_big_banner(banner);

   int n_clips=clip_ids_map.size();
   cout << "n_clips = " << n_clips << endl;

// Add words from video sound tracks into graph display:

   string soundtrack_filename=graphs_subdir+"insert_soundtrack.sql";
   ofstream soundtrack_stream;
   soundtrack_stream.precision(12);
   filefunc::openfile(soundtrack_filename,soundtrack_stream);

//   int stop_clip_ID=start_clip_ID+1;
//   int stop_clip_ID=start_clip_ID+2;
   int stop_clip_ID=start_clip_ID+n_clips;
   for (int clip_ID=start_clip_ID; clip_ID<stop_clip_ID; clip_ID++)
   {
      string transcript_filename=transcripts_subdir+"clip_"+
         stringfunc::integer_to_string(clip_ID,4)+".transcript";
      if (!filefunc::fileexist(transcript_filename)) continue;

      CLIP_IDS_MAP::iterator iter=clip_ids_map.find(clip_ID);
      double clip_duration=iter->second;
      double clip_gx_extent=
         clip_duration/(max_time-min_time)*gx_extent;

// Import transcript words from video sound tracks:

      vector<string> transcript_strings=
         filefunc::ReadInStrings(transcript_filename);
      int n_transcript_words=transcript_strings.size();
      int n_transcript_chars=0;
      for (int w=0; w<n_transcript_words; w++)
      {
         n_transcript_chars += transcript_strings[w].size()+1;
      }

      int char_counter=0;
      int word_counter=0;
      int max_n_words_per_burst=5;
      int min_n_words_per_burst=1;
      int n_words_per_burst=min_n_words_per_burst;
      if (vertical_layout_flag)
      {
         n_words_per_burst=4;
      }
      while (word_counter < n_transcript_words)
      {
         string curr_burst="";
         for (int w=0; w<n_words_per_burst && word_counter<n_transcript_words;
              w++)
         {
            curr_burst += transcript_strings[word_counter++]+" ";
         }
         char_counter += curr_burst.size();

         string cleaned_burst=stringfunc::find_and_replace_char(
            curr_burst,"'","''");
         cleaned_burst=stringfunc::find_and_replace_char(
            cleaned_burst,"\""," ");
//         cleaned_burst=stringfunc::find_and_replace_char(
//            cleaned_burst,"\"","\"\"");

         double curr_burst_frac=double(char_counter)/n_transcript_chars;
         double burst_gx=curr_burst_frac*clip_gx_extent;

//         double clip_vertical_offset=(clip_ID-start_clip_ID)*3;
         double clip_vertical_offset=(clip_ID-start_clip_ID)*
            sinusoid_separation;

         double t_renorm=curr_burst_frac*clip_duration;
         double phi=omega*t_renorm;
         phi=basic_math::phase_to_canonical_interval(phi,0,2*PI);

         const double A=1;
         double gy_lo=A*(-1+0.10);
         double gy_hi=A*(1+0.45);
         double burst_gy=clip_vertical_offset;
         double phi_frac;
         if (phi >= 0 && phi <= PI)
         {

// Falling 
            phi_frac=(phi-0)/(PI-0);
            burst_gy += gy_hi+phi_frac*(gy_lo-gy_hi);
         }
         else
         {

// Rising

            phi_frac=(phi-PI)/(2*PI-PI);
            burst_gy += gy_lo+phi_frac*(gy_hi-gy_lo);
         }

//         double burst_vertical_offset=clip_vertical_offset+0.25*A;
//         double burst_gy=burst_vertical_offset+A*cos(omega*t_renorm);

         double burst_frac=1-2*fabs(phi_frac-0.5);
         n_words_per_burst=basic_math::round(min_n_words_per_burst+burst_frac*(
            max_n_words_per_burst-min_n_words_per_burst));

         if (vertical_layout_flag)
         {
            curr_burst_frac=double(word_counter)/n_transcript_words;
            burst_gx=clip_vertical_offset+3*A;

// We experimented with trying left-justify the transcript text on 10/30/13.
// But left-justification distance appears to depend on text font size
// which is variable.  So we live with center-justified text for now...

//            double prefactor=1.0/3.0;
//            burst_gx += prefactor*0.5*cleaned_burst.size();
            burst_gy=-(curr_burst_frac*clip_gx_extent);
            n_words_per_burst=4;
         }
         
         int level=graph_ID;
         int layout=1;	// timeline graph display
         string color="white";
         double annotation_size=1;
         string SQL_command=
            graphdbfunc::generate_insert_graph_annotation_SQL_command(
               hierarchy_ID,graph_ID,level,layout,
               burst_gx,burst_gy,cleaned_burst,color,annotation_size);
         soundtrack_stream << SQL_command << endl;

      } // loop over index b labeling transcript word bursts
      
   } // loop over clip_ID index

   filefunc::closefile(soundtrack_filename,soundtrack_stream);
   banner="Exported transcripts to SQL script "+soundtrack_filename;
   outputfunc::write_big_banner(banner);

// Periodically add time-stamp label annotations into timeline graph:

   for (int p=0; p<n_complete_Periods+1; p++)
   {
      double gx=p*delta_gx_per_Period;
      double gy=-1;

      if (vertical_layout_flag)
      {
         gx=-2;
         gy=-p*delta_gx_per_Period;
      }
      
      double t=min_time+p*Period;
      clock.convert_elapsed_secs_to_date(t);

      string day_hour_separator_char=" ";
      string time_separator_char=":";
      bool display_UTC_flag=false;
      int n_secs_digits=0;
      string date_string=clock.YYYY_MM_DD_H_M_S(
         day_hour_separator_char,time_separator_char,
         display_UTC_flag,n_secs_digits);

      if (time_interval_in_days > 2)
      {
         date_string=date_string.substr(0,10);	// yyyy-mm-dd
      }
      else
      {
         date_string=date_string.substr(11,date_string.size()-11-8);
      }

/*      
// Display full date and time information within every 4th label.
// Otherwise, display just UTC time:

      if (p%4 != 0)
      {
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(date_string);
         date_string=substrings.back();
      }
*/

//      cout << "p = " << p
//           << " date_string = " << date_string
//           << endl;

      int level=graph_ID;
      int layout=1;	// timeline graph display
      string color="white";
      double annotation_size=300;
      string SQL_command=
         graphdbfunc::generate_insert_graph_annotation_SQL_command(
            hierarchy_ID,graph_ID,level,layout,
            gx,gy,date_string,color,annotation_size);
      SQL_timestamp_annotation_stream << SQL_command << endl;
   } // look over index p labeling complete Periods

// Add video label annotations on LHS of multi-sinusoid display:

   for (int s=0; s<n_clips; s++)
   {
      int gx=-3;
      int gy=s*3;

      if (vertical_layout_flag)
      {
         gx=s*sinusoid_separation;
         gy=1.5;
      }

      string video_label="Video "+stringfunc::number_to_string(s+1);

      int level=graph_ID;
      int layout=1;	// timeline graph display
      string color="white";
      double annotation_size=500;
      string SQL_command=
         graphdbfunc::generate_insert_graph_annotation_SQL_command(
            hierarchy_ID,graph_ID,level,layout,
            gx,gy,video_label,color,annotation_size);
      SQL_timestamp_annotation_stream << SQL_command << endl;
   } // loop over index s labeling sensor_ID

   cout << endl;
   filefunc::closefile(temporal_filename,temporal_stream);
   filefunc::closefile(
      SQL_timestamp_annotation_filename,SQL_timestamp_annotation_stream);

   banner="Exported temporal layout to text file "+temporal_filename;
   outputfunc::write_big_banner(banner);

   banner="Exported timestamp annotations to SQL script "
      +SQL_timestamp_annotation_filename;
   outputfunc::write_big_banner(banner);
}
