// ==========================================================================
// Program FIND_AND_REPLACE was created to help rapidly search for
// particular strings within multiple files.  It takes in a file
// containing a list of .cc and .h files (generated via the unix
// "find" command").  This program scans through each file within the
// list and searches for a particular string which is entered by the
// user (e.g. "nrfuncs.h").  It then prepends some user specified
// string (e.g. "numrec/").  We used this program to move all .h files
// within the /include file into specialized subdirectories of
// /include.  We then had to make corresponding modifications to
// #include statments within many .h and .cc files.  This program
// saved us from having to perform all these tedious modifications
// completely by hand.

// ==========================================================================
// Last updated on 11/21/04
// ==========================================================================

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "image/recursivefuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   
   int recursion_level=0;
   recursivefunc::dummy_func(recursion_level);
   
  
}
