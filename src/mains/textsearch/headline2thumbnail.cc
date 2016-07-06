// ========================================================================
// Program HEADLINE2THUMBNAIL is a specialized utility which we wrote
// for Reuters news articles.  It extracts the first headline from
// each input Reuters text file and generates an HTML file containing
// just the headline in large font.  The HTML output is converted to
// postscript and then to jpg via ImageMagick.  Final thumbnails are
// deposited within the thumbnails_subdir specified below.

//			    headline2thumbnail

// ========================================================================
// Last updated on 12/25/12; 5/28/13
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
//   string text_subdir=reuters_subdir+"text/";
   string text_subdir=reuters_subdir+"more_text/";
   string html_subdir=reuters_subdir+"html/";
   string jpg_subdir=reuters_subdir+"jpg/";
   string thumbnails_subdir=jpg_subdir+"new_thumbnails/";

   filefunc::dircreate(thumbnails_subdir);
   
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("txt");
   vector<string> txt_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,text_subdir);
   cout << "txt_filenames.size() = " << txt_filenames.size() << endl;

   timefunc::initialize_timeofday_clock();

   int file_counter=0;
   int i_start=0;
//   int i_stop=5;
   int i_stop=txt_filenames.size();

   bool strip_comments_flag=false;
   string html_filename="./headline.html";
   string postscript_filename="./headline.ps";
   ofstream html_stream;
   for (int i=i_start; i<i_stop; i++)
   {
      cout << "Processing file " << i << " of " << i_stop << endl;
      
      string txt_filename=txt_filenames[i];
//      cout << "txt_filename = " << txt_filename << endl;
      filefunc::ReadInfile(txt_filename,strip_comments_flag);

      filefunc::openfile(html_filename,html_stream);
      
      html_stream << "<!DOCTYPE html>" << endl << endl;
      html_stream << "<html>" << endl;
      html_stream << "<body>" << endl << endl;
      html_stream << "<h1><font size=\"11\">" << filefunc::text_line[0] 
                  << "</font></h1>" << endl << endl;
      html_stream << "</body>" << endl;
      html_stream << "</html>" << endl;

      filefunc::closefile(html_filename,html_stream);

// Convert html to postscript:

//      cout << "Converting html to postscript" << endl;
      string unix_cmd="html2ps "+html_filename+" > "+postscript_filename;
      sysfunc::unix_command(unix_cmd);

// Convert postscript to jpg.  Crop unnecessary white borders and
// reduce quality as much as possible to minimize jpg file size:

//      cout << "Converting postscript to jpg" << endl;

      string basename=filefunc::getbasename(txt_filename);
      string prefix=stringfunc::prefix(basename);
      string thumbnail_filename=thumbnails_subdir+"thumbnail_"+prefix+".jpg";

      unix_cmd="convert -density 250 -shave 10% "+postscript_filename
         +"\\[0] "+thumbnail_filename;
      sysfunc::unix_command(unix_cmd);
      unix_cmd="convert -resize 150x200 -quality 75 "+thumbnail_filename+" "+
         thumbnail_filename;
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

