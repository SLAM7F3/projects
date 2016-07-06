// ========================================================================
// Program TEXT2HTML reads in a set of text files (e.g. scraped from
// Reuters' new feed) from a specified subdirectory.  It generates
// simple html versions of the input text files with rudimentary
// formatting.  TEXT2HTML then converts the html into postscript and
// jpg formats.  The html and jpg files are archived while the
// postscript versions are deleted.  White borders are cropped and
// image quality is reduced as much as possible to minimize disk space
// usage by the output jpg files.

//				./text2html

// ========================================================================
// Last updated on 12/22/12; 12/23/12; 5/28/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   string reuters_subdir="/home/cho/bombing/reuters/2013/";
//   string reuters_subdir=
//      "/media/66368D22368CF3F9/visualization/arXiv/reuters/export/";
   string text_subdir=reuters_subdir+"more_text/";
//   string text_subdir=reuters_subdir+"text/";
   string html_subdir=reuters_subdir+"html/";
   string jpg_subdir=reuters_subdir+"jpg/";

   filefunc::dircreate(html_subdir);
   filefunc::dircreate(jpg_subdir);
   
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("txt");
   vector<string> txt_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,text_subdir);

   timefunc::initialize_timeofday_clock();

   int file_counter=0;
   int i_start=0;
//   int i_start=299;
   int i_stop=txt_filenames.size();
//   int i_stop=5;
   for (int i=i_start; i<i_stop; i++)
   {
      cout << "Processing file " << i << " of " << i_stop << endl;
      
      string txt_filename=txt_filenames[i];
//      cout << "txt_filename = " << txt_filename << endl;

      bool strip_comments_flag=false;
      filefunc::ReadInfile(txt_filename,strip_comments_flag);

      string html_filename=filefunc::replace_suffix(
         txt_filename,"html");
//      cout << "html_filename = " << html_filename << endl;
      ofstream html_stream;
      filefunc::openfile(html_filename,html_stream);
      
      html_stream << "<!DOCTYPE html>" << endl << endl;
      html_stream << "<html>" << endl;
      html_stream << "<body>" << endl << endl;
      html_stream << "<h1>" << filefunc::text_line[0] << "</h1>" << endl
                  << endl;

      for (unsigned int f=1; f<filefunc::text_line.size(); f++)
      {
         string text_line=filefunc::text_line[f];
         if (text_line.size()==0) continue;
         html_stream << "<p>" << text_line << "</p>" << endl;
         html_stream << endl;
      }
      html_stream << "</body>" << endl;
      html_stream << "</html>" << endl;

      filefunc::closefile(html_filename,html_stream);

// Convert html to postscript:

//      cout << "Converting html to postscript" << endl;
      string postscript_filename=filefunc::replace_suffix(html_filename,"ps");
      string unix_cmd="html2ps "+html_filename+" > "+postscript_filename;
      sysfunc::unix_command(unix_cmd);

// Convert postscript to jpg.  Crop unnecessary white borders and
// reduce quality as much as possible to minimize jpg file size:

//      cout << "Converting postscript to jpg" << endl;
      string jpg_filename=filefunc::replace_suffix(postscript_filename,"jpg");
      unix_cmd="convert -density 250 -quality 100 "+postscript_filename
         +"\\[0] "+jpg_filename;
      sysfunc::unix_command(unix_cmd);
      unix_cmd="convert -quality 100 -resize 50% "+jpg_filename+" "+
         jpg_filename;
      sysfunc::unix_command(unix_cmd);
      unix_cmd="convert -quality 50 -shave 10% "+jpg_filename+" "+
         jpg_filename;
      sysfunc::unix_command(unix_cmd);

      filefunc::deletefile(postscript_filename);
      unix_cmd="mv "+html_filename+" "+html_subdir;
      sysfunc::unix_command(unix_cmd);
      unix_cmd="mv "+jpg_filename+" "+jpg_subdir;
      sysfunc::unix_command(unix_cmd);

      file_counter++;
      if (file_counter > 1 && file_counter%5==0)
      {
         outputfunc::print_elapsed_time();

         double avg_time_per_file=
            timefunc::elapsed_timeofday_time()/file_counter;
         cout << "Average time per text file = " << avg_time_per_file 
              << " secs" << endl;
         double remaining_time=(i_stop-i_start-file_counter)*avg_time_per_file;
         cout << "Remaining processing time = "
              << remaining_time << " secs = "
              << remaining_time/60.0 << " mins = "
              << remaining_time/3600.0 << " hrs " << endl;
         cout << endl;
      }

   } // loop over index i labeling input text files

}

