// ========================================================================
// Program DISPLAY_DOC_TEXT queries the user to enter a document ID.
// It then displays the document's beginning text.  We wrote this
// little utility program in order to debug MALLET clustering.

//		       	./display_doc_text

// ========================================================================
// Last updated on 5/27/13; 5/29/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "math/basic_math.h"
#include "general/filefuncs.h"
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

   string reuters_subdir=
      "/data_third_disk/text_docs/reuters/export/";
   string text_subdir=reuters_subdir+"text/50K_docs/";
//   string reuters_subdir=
//      "/media/66368D22368CF3F9/visualization/arXiv/reuters/export/";
//   string text_subdir=reuters_subdir+"text/";
//   string html_subdir=reuters_subdir+"html/";
//   string jpg_subdir=reuters_subdir+"jpg/";
//   string thumbnails_subdir=jpg_subdir+"new_thumbnails/";
//   filefunc::dircreate(thumbnails_subdir);
   
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("txt");
   vector<string> txt_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,text_subdir);
   cout << "txt_filenames.size() = " << txt_filenames.size() << endl;

   while (true)
   {
      int doc_ID;
      cout << "Enter document ID:" << endl;
      cin >> doc_ID;
      
      string txt_filename=txt_filenames[doc_ID];
      cout << "txt_filename = " << txt_filename << endl << endl;
      filefunc::ReadInfile(txt_filename);

      int n_doc_lines=basic_math::min(10,int(filefunc::text_line.size()));
      for (int i=0; i<n_doc_lines; i++)
      {
         cout << filefunc::text_line[i] << endl;
      }
      cout << endl;

   } // infinite while loop
   
   

}

