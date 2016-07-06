// ==========================================================================
// Program RF_DIRFINDER reads in RF direction finding position,
// pointing and signal strength information from input files generated
// by Tamara Yu.  It computes time-averaged median values for the
// sensor's geolocation.  RF_DIRFINDER also effectively computes
// signal strength as a function of yaw.  After gaussian filtering the
// raw readings, this program finds the peak as well as the yaw values
// where the signal strength falls by 3 dB from the peak.  It outputs
// a text file with averaged sensor geoposition, yaw pointing, RF lobe
// yaw size and peak signal strength.

//			rf_dirfinder

// Note: Need to clean Tamara's raw RF data files.  Eliminate any
// lines with --- entries.  Also change all '+' to ' '.

// ==========================================================================
// Last updated on 8/20/11; 8/24/11; 9/7/11
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "plot/metafile.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   sysfunc::clearscreen();
   cout << endl;

   string banner="Input RF direction finder files must end with '.dat' suffix.";
   outputfunc::write_banner(banner);
   banner="Enter full path for subdirectory containing all RF direction finder input files:";
   outputfunc::write_banner(banner);

   string subdir;
//   cin >> subdir;
//   subdir="/home/cho/programs/c++/svn/projects/src/mains/hmi/dir_finder/LL-Pavilion-2011-08-03/";
//   subdir="/home/cho/programs/c++/svn/projects/src/mains/hmi/FtDevens/rf/Ft-Devens-2011-08-19/";
//   subdir="/home/cho/programs/c++/svn/projects/src/mains/hmi/dir_finder/LL-Pavilion-2011-08-23/";
   subdir="/home/cho/programs/c++/svn/projects/src/mains/hmi/dir_finder/LL-Pavilion-2011-09-06/";
   
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("dat");
   allowed_suffixes.push_back("csv");
   vector<string> input_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,subdir);
   cout << "input_filenames.size() = " << input_filenames.size() << endl;

   string output_dir=subdir+"/metafiles/";
   filefunc::dircreate(output_dir);

   string rf_filename=subdir+"rf_results.txt";
   ofstream rf_stream;
   filefunc::openfile(rf_filename,rf_stream);
   rf_stream.precision(12);
   rf_stream << "# Set  Easting        Northing      Yaw  Lobe size  Peak strength" << endl;
   rf_stream << "#       (m)              (m)        (deg)  (deg)       (dB)"
             << endl << endl;

   int iter_start=0;
//   int iter_start=4;
   for (unsigned int iter=iter_start; iter<input_filenames.size(); iter++)
   {
      string curr_filename=input_filenames[iter];
      string base_filename=filefunc::getbasename(curr_filename);

      cout << "iter = " << iter << " *******************************" << endl;
      cout << "curr_filename = " << curr_filename << endl;
      cout << "base_filename = " << base_filename << endl;

      filefunc::ReadInfile(curr_filename);

      double t0=0;
      double min_yaw=POSITIVEINFINITY;
      double max_yaw=NEGATIVEINFINITY;
      double min_signal_strength=POSITIVEINFINITY;
      double max_signal_strength=NEGATIVEINFINITY;
      double prev_roll=0,prev_pitch=0,prev_yaw=0;
      vector<double> time_values,latitude_values,longitude_values,
         altitude_values,roll_values,pitch_values,yaw_values,signal_strengths;

      for (unsigned int i=0; i<filefunc::text_line.size()-1; i++)
      {
//      cout << filefunc::text_line[i] << endl;

         string separator_chars=",:";
         vector<string> columns=stringfunc::decompose_string_into_substrings(
            filefunc::text_line[i],separator_chars);

         vector<double> column_values;
         for (unsigned int c=0; c<columns.size(); c++)
         {
            column_values.push_back(stringfunc::string_to_number(columns[c]));
//            cout << "c = " << c << " curr_value = " << column_values.back()
//                 << endl;
         }
         cout << endl;         
      
         double curr_time=0.001*column_values[0];     // relative time in secs
         time_values.push_back(curr_time);

         latitude_values.push_back(column_values[1]);
         longitude_values.push_back(column_values[2]);
         altitude_values.push_back(column_values[3]);

         double curr_roll=column_values[4];
         double curr_pitch=column_values[5];
         double curr_yaw=column_values[6];
      
         if (i==0)
         {
            prev_roll=curr_roll;
            prev_pitch=curr_pitch;
            prev_yaw=curr_yaw;
         }
         curr_roll=basic_math::phase_to_canonical_interval(
            curr_roll,prev_roll-180,prev_roll+180);
         curr_pitch=basic_math::phase_to_canonical_interval(
            curr_pitch,prev_pitch-180,prev_pitch+180);
         curr_yaw=basic_math::phase_to_canonical_interval(
            curr_yaw,prev_yaw-180,prev_yaw+180);

//         cout << "i = " << i
//              << " curr_yaw = " << curr_yaw
//              << " prev_yaw = " << prev_yaw << endl;

         prev_roll=curr_roll;
         prev_pitch=curr_pitch;
         prev_yaw=curr_yaw;

         roll_values.push_back(curr_roll);
         pitch_values.push_back(curr_pitch);
         yaw_values.push_back(curr_yaw);
         min_yaw=basic_math::min(min_yaw,curr_yaw);
         max_yaw=basic_math::max(max_yaw,curr_yaw);

         signal_strengths.push_back(column_values[7]);
         min_signal_strength=basic_math::min(
            min_signal_strength,signal_strengths.back());
         max_signal_strength=basic_math::max(
            max_signal_strength,signal_strengths.back());
      } // loop over index i labeling lines within input file

//      outputfunc::enter_continue_char();

// Renormalize times relative to first time:

      cout << "min_signal_strength = " << min_signal_strength << endl;
      t0=time_values[0];
      for (unsigned int s=0; s<signal_strengths.size(); s++)
      {
         time_values[s]=time_values[s]-t0;
//         signal_strengths[s]= signal_strengths[s]-min_signal_strength;
//      cout << "s = " << s << " time = " << time_values[s] << endl;
      }
//      max_signal_strength -= min_signal_strength;

      double median_latitude=mathfunc::median_value(latitude_values);
      double sigma_latitude=mathfunc::std_dev(latitude_values);
   
      double median_longitude=mathfunc::median_value(longitude_values);
      double sigma_longitude=mathfunc::std_dev(longitude_values);

      double median_altitude=mathfunc::median_value(altitude_values);
      double sigma_altitude=mathfunc::std_dev(altitude_values);

      cout.precision(12);
      cout << "Latitude = " << median_latitude << " +/- " << sigma_latitude 
           << endl;
      cout << "Longitude = " << median_longitude << " +/- " 
           << sigma_longitude << endl;
      cout << "Altitude = " << median_altitude << " +/- " << sigma_altitude 
           << endl;

      geopoint sensor_location(
         median_longitude,median_latitude,median_altitude);
      
// Generate signal strength histogram as function of yaw bins:

      double yaw_start=0;
      double yaw_stop=360;
//      double delta_yaw=5;
//      double delta_yaw=7.5;
      double delta_yaw=10;
      int n_yaw_bins=(yaw_stop-yaw_start)/delta_yaw;

      vector<int> n_histogram_entries;
      vector<double> yaw_histogram_values,signal_strength_histogram;
      for (int y=0; y<n_yaw_bins; y++)
      {
         yaw_histogram_values.push_back(0);
         signal_strength_histogram.push_back(0);
         n_histogram_entries.push_back(0);
      }

      cout << "yaw_values.size() = " << yaw_values.size() << endl;
      cout << "n_yaw_bins = " << n_yaw_bins << endl;
      for (unsigned int n=0; n<yaw_values.size(); n++)
      {
         double curr_yaw=basic_math::phase_to_canonical_interval(
            yaw_values[n],yaw_start,yaw_stop);
         int y_index=curr_yaw/delta_yaw;
         yaw_histogram_values[y_index]=yaw_histogram_values[y_index]+curr_yaw;
         double curr_signal_strength=signal_strengths[n];
         signal_strength_histogram[y_index]=
            signal_strength_histogram[y_index]+curr_signal_strength;
         n_histogram_entries[y_index]=n_histogram_entries[y_index]+1;
      } // loop over index n labeling raw yaw values

      for (unsigned int y=0; y<yaw_histogram_values.size(); y++)
      {
         int n_hist_entries=n_histogram_entries[y];
         if (n_hist_entries == 0) continue;
         
         yaw_histogram_values[y]=yaw_histogram_values[y]/n_hist_entries;
         signal_strength_histogram[y]=signal_strength_histogram[y]/
            n_hist_entries;
         cout << "y = " << y
              << " n_hist = " << n_hist_entries 
              << " yaw_hist = " << yaw_histogram_values[y]
              << " signal hist = " << signal_strength_histogram[y]
              << endl;
      }

//      outputfunc::enter_continue_char();
      
// Yaw values may not necessarily span an entire 360 range.  If not,
// sentinel 0 values may remain within yaw_histogram_value and
// signal_strength_histogram STL vectors.  Look for and eliminate any
// such sentinel values:

      bool erased_flag=false;
      do
      {
         erased_flag=false;
         for (vector<double>::iterator v_iterator=
                 signal_strength_histogram.begin(); v_iterator != 
                 signal_strength_histogram.end(); v_iterator++)
         {
            double curr_signal_strength=*v_iterator;
            if (nearly_equal(curr_signal_strength,0))
            {
               signal_strength_histogram.erase(v_iterator);
               erased_flag=true;
               break;
            }
         }
      }
      while (erased_flag);

      do
      {
         erased_flag=false;
         for (vector<double>::iterator v_iterator=
                 yaw_histogram_values.begin(); v_iterator != 
                 yaw_histogram_values.end(); v_iterator++)
         {
            double curr_yaw_value=*v_iterator;
            if (nearly_equal(curr_yaw_value,0))
            {
               yaw_histogram_values.erase(v_iterator);
               erased_flag=true;
               break;
            }
         }
      }
      while (erased_flag);

// Low-pass filter signal strength histogram with a gaussian filter:

      vector<double> gauss_filter;
      gauss_filter.reserve(n_yaw_bins);
      
      double sigma=5;	// degrees
      filterfunc::gaussian_filter(delta_yaw,sigma,gauss_filter);

      bool wrap_around_input_values=true;
      vector<double> filtered_signal_strength_histogram;
      filterfunc::brute_force_filter(
         signal_strength_histogram,gauss_filter,
         filtered_signal_strength_histogram,wrap_around_input_values);

      cout << "filtered_strength.size() = "
           << filtered_signal_strength_histogram.size() << endl;

// Search for maximum filtered signal strength value:

      int y_peak=-1;
      double peak_strength=min_signal_strength;
      double peak_yaw=-1;
      for (unsigned int y=0; y<filtered_signal_strength_histogram.size(); y++)
      {
         double curr_strength=filtered_signal_strength_histogram[y];
//         cout << "y = " << y
//              << " filtered strength = " << curr_strength
//              << " peak_strength = " << peak_strength << endl;
         if (curr_strength > peak_strength)
         {
            peak_strength=curr_strength;
            y_peak=y;
            peak_yaw=yaw_histogram_values[y_peak];
         }
      } // loop over index y 

      cout << "y_peak = " << y_peak
           << " peak_yaw = " << peak_yaw
           << " peak_strength = " << peak_strength << endl;
      
// Next search for yaw interval over which signal strength decreases
// by 3 dB from peak:

      unsigned int counter=0;
      unsigned int y=y_peak;
      double yaw_hi=peak_yaw;
      while (counter < yaw_histogram_values.size())
      {
         double curr_strength=filtered_signal_strength_histogram[y];
         if (curr_strength < peak_strength - 3)
         {
            yaw_hi=yaw_histogram_values[y];
            break;
         }
         y++;
         counter++;
         if (y >= yaw_histogram_values.size()) y=0;
      }
      cout << "yaw_hi = " << yaw_hi << endl;

      counter=0;
      y=y_peak;
      double yaw_lo=peak_yaw;
      while (counter < yaw_histogram_values.size())
      {
         double curr_strength=filtered_signal_strength_histogram[y];
         if (curr_strength < peak_strength - 3)
         {
            yaw_lo=yaw_histogram_values[y];
            break;
         }
         y--;
         counter++;
         if (y <0) y=yaw_histogram_values.size()-1;
      }
      cout << "yaw_lo = " << yaw_lo << endl;

      yaw_lo=basic_math::phase_to_canonical_interval(
         yaw_lo,yaw_hi-180,yaw_hi+180);
      double yaw_lobe_size=yaw_hi-yaw_lo;
      cout << "yaw_lobe size = " << yaw_lobe_size << endl;
      double yaw_direction=0.5*(yaw_lo+yaw_hi);
      yaw_direction=basic_math::phase_to_canonical_interval(
         yaw_direction,0,360);

// Write direction finding results to text file output:

/*
      rf_stream << "Data set " << iter << endl;
      rf_stream << "Sensor easting = " << sensor_location.get_UTM_easting()
                << endl;
      rf_stream << "Sensor northing = " << sensor_location.get_UTM_northing()
                << endl;
      rf_stream << "RF yaw direction = " << yaw_direction << endl;
      rf_stream << "RF yaw lobe size = " << yaw_lobe_size << endl;
      rf_stream << "Peak strength = " << peak_strength << endl;
      rf_stream << endl;
*/

      rf_stream << iter << "   "
                << sensor_location.get_UTM_easting() << "   "
                << sensor_location.get_UTM_northing() << "   "
                << yaw_direction << "   "
                << yaw_lobe_size << "   "
                << peak_strength << endl;
      
// Write out metafiles:

      metafile M;
      string iterator_str=stringfunc::number_to_string(iter+1);

      string meta_filename=output_dir+"yaw_vs_time_"+iterator_str;
      string title="Yaw vs time";
      string x_label="Relative time (secs)";
      string y_label="Yaw angle (degs)";
      double x_min=0;
      double x_max=time_values.back();
      double y_min=min_yaw;
      double y_max=max_yaw;
      M.set_parameters(
         meta_filename,title,x_label,y_label,x_min,x_max,y_min,y_max);
      M.openmetafile();
      M.write_header();
      M.write_curve(time_values,yaw_values);
      M.closemetafile();
      string unix_cmd="meta_to_jpeg "+meta_filename;
      sysfunc::unix_command(unix_cmd);

      meta_filename=output_dir+"signal_strength_vs_time_"+iterator_str;
      title="Signal strength vs time";
      x_label="Relative time (secs)";
      y_label="Relative signal strength (dB)";
      x_min=0;
      x_max=time_values.back();
      y_min=min_signal_strength;
      y_max=max_signal_strength;
      M.set_parameters(
         meta_filename,title,x_label,y_label,x_min,x_max,y_min,y_max);
      M.openmetafile();
      M.write_header();
      M.write_curve(time_values,signal_strengths);
      M.closemetafile();
      unix_cmd="meta_to_jpeg "+meta_filename;
      sysfunc::unix_command(unix_cmd);

      meta_filename=output_dir+"signal_strength_vs_yaw_"+iterator_str;
      title="Signal strength vs yaw";
      x_label="Yaw angle (degs)";
      y_label="Relative signal strength (dB)";
      x_min=min_yaw;
      x_max=max_yaw;
      y_min=min_signal_strength;
      y_max=max_signal_strength;
      M.set_parameters(
         meta_filename,title,x_label,y_label,x_min,x_max,y_min,y_max);
      M.openmetafile();
      M.write_header();
      M.write_curve(yaw_values,signal_strengths);
      M.closemetafile();
      unix_cmd="meta_to_jpeg "+meta_filename;
      sysfunc::unix_command(unix_cmd);

      meta_filename=output_dir+
         "signal_strength_vs_yaw_histogram"+iterator_str;
      title="Signal strength vs yaw";
      x_label="Yaw angle (degs)";
      y_label="Relative signal strength (dB)";
      x_min=0;
      x_max=360;
      y_min=min_signal_strength;
      y_max=max_signal_strength;
      M.set_parameters(
         meta_filename,title,x_label,y_label,x_min,x_max,y_min,y_max);
      M.openmetafile();
      M.write_header();
      M.write_curve(yaw_histogram_values,signal_strength_histogram);
      M.closemetafile();
      unix_cmd="meta_to_jpeg "+meta_filename;
      sysfunc::unix_command(unix_cmd);

      meta_filename=output_dir+
         "filtered_signal_strength_vs_yaw_histogram"+iterator_str;
      title="Filtered signal strength vs yaw";
      x_label="Yaw angle (degs)";
      y_label="Relative signal strength (dB)";
      x_min=0;
      x_max=360;
      y_min=min_signal_strength;
      y_max=max_signal_strength;
      M.set_parameters(
         meta_filename,title,x_label,y_label,x_min,x_max,y_min,y_max);
      M.openmetafile();
      M.write_header();

      vector<string> extra_lines;
      extra_lines.push_back("curve color blue");
      yaw_lo=basic_math::phase_to_canonical_interval(yaw_lo,0,360);
      extra_lines.push_back(stringfunc::number_to_string(yaw_lo)+" -1000");
      extra_lines.push_back(stringfunc::number_to_string(yaw_lo)+" 1000");
      extra_lines.push_back("curve color green");
      yaw_hi=basic_math::phase_to_canonical_interval(yaw_hi,0,360);
      extra_lines.push_back(stringfunc::number_to_string(yaw_hi)+" -1000");
      extra_lines.push_back(stringfunc::number_to_string(yaw_hi)+" 1000");

      for (unsigned int e=0; e<extra_lines.size(); e++)
      {
         M.add_extraline(extra_lines[e]);
      }
      M.add_extralines();

      M.write_curve(yaw_histogram_values,filtered_signal_strength_histogram);

      M.closemetafile();
      unix_cmd="meta_to_jpeg "+meta_filename;
      sysfunc::unix_command(unix_cmd);

   } // loop over iter index labeling different input files

   cout << "Done exporting metafiles" << endl;

   filefunc::closefile(rf_filename,rf_stream);

   banner="Processed RF finder geoposition and orientation data written to"+
      rf_filename;
   outputfunc::write_big_banner(banner);

}
