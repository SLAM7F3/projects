// ========================================================================
// Program PDF2JPG reads in pdf files from a specified subdirectory.
// It generates JPG files of the PDF documents' first pages using
// ImageMagick's convert utility.

//				pdf2jpg

// ========================================================================
// Last updated on 12/12/12; 12/14/12; 12/15/12
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main(int argc, char** argv)
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::map;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   const int ascii_a=97;
   const int ascii_z=122;
   const int ascii_A=65;
   const int ascii_Z=90;
   const int ascii_hyphen=45;

   typedef map<string,int> WORD_MAP;
   WORD_MAP::iterator word_iter;
   WORD_MAP multi_doc_word_map;

// Import PDF files:

//   string pdf_subdir="/media/66368D22368CF3F9/visualization/arXiv/HEP/pdf/";
   string arXiv_subdir="/media/66368D22368CF3F9/visualization/arXiv/";
   string astro_subdir=arXiv_subdir+"astro/";
//   string pdf_subdir=astro_subdir+"pdf/";
   string pdf_subdir=astro_subdir+"new_pdf/";

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("pdf");
   vector<string> pdf_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,pdf_subdir);
   int n_pdf_files=pdf_filenames.size();

   for (int n=0; n<n_pdf_files; n++)
   {
      string pdf_filename=pdf_filenames[n];
      string basename=filefunc::getbasename(pdf_filename);
      string prefix=stringfunc::prefix(basename);
      string image_filename=pdf_subdir+prefix+".jpg";
      string unix_cmd="convert -density 300 -quality 100 "
         +pdf_filename+"\\[0] "+image_filename;
      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
   }
   


}

