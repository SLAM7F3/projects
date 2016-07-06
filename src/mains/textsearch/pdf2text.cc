// ========================================================================
// Program PDF2TEXT runs the built-in unix command pdftotext on a set
// of PDF files within a specified subdirectory.

//				pdf2text

// ========================================================================
// Last updated on 12/9/12; 12/14/12; 12/15/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   string arXiv_subdir="/media/66368D22368CF3F9/visualization/arXiv/";
   string astro_subdir=arXiv_subdir+"astro/";
//   string pdf_subdir=astro_subdir+"pdf/";
   string pdf_subdir=astro_subdir+"new_pdf/";
//   string pdf_subdir="/media/66368D22368CF3F9/visualization/arXiv/HEP/";
//   pdf_subdir += "Nov2012/";
   vector<string> pdf_filenames=filefunc::files_in_subdir(pdf_subdir);

   for (int i=0; i<pdf_filenames.size(); i++)
   {
      string pdf_filename=pdf_filenames[i];
      string unix_cmd="pdftotext "+pdf_filename;
      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
   }
   

}

