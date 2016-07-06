// ==========================================================================
// Webimage class member function definitions which allow one to
// create HTML links using C++.
// ==========================================================================
// Last modified on 3/23/04
// ==========================================================================

#include "general/stringfuncs.h"
#include "general/filefuncs.h"
#include "webimage.h"

using std::string;
using std::istream;
using std::ifstream;
using std::ostream;
using std::endl;

const int webimage::NMAXCOLORS=800;

// ---------------------------------------------------------------------
// Constructor functions:
// ---------------------------------------------------------------------

webimage::webimage(void)
{
   R=new int[NMAXCOLORS];
   G=new int[NMAXCOLORS];
   B=new int[NMAXCOLORS];
   colorname=new string[NMAXCOLORS];

   border=height=width=0;
   horizposn="center";
}

// When an object is initialized with an object of the same type, the
// following function is called.  This next constructor is apparently
// called whenever a function is passed an object as an argument:

webimage::webimage(const webimage& currimage)
{
   R=new int[NMAXCOLORS];
   G=new int[NMAXCOLORS];
   B=new int[NMAXCOLORS];
   colorname=new string[NMAXCOLORS];

   docopy(currimage);
}

webimage::~webimage()
{
   delete [] R;
   delete [] G;
   delete [] B;
   delete [] colorname;
}

// ---------------------------------------------------------------------
// As Ed Broach has pointed out, the default C++ =operator for objects
// may simply equate pointers to arrays within objects when one object
// is equated with another.  Individual elements within the arrays
// apparently are not equated to one another by the default C++
// =operator.  This can lead to segmentation errors if the arrays are
// dynamically rather than statically allocated, for the pointer to
// the original array may be destroyed before the elements within the second
// array are copied over.  So we need to write an explicit copy function 
// which transfers all of the subfields within an object to another object
// whenever the object in question has dynamically allocated arrays rather 
// than relying upon C++'s default =operator:

void webimage::docopy(const webimage& currimage)
{
   int i;
   
   border=currimage.border;
   height=currimage.height;
   width=currimage.width;
   horizposn=currimage.horizposn;
   src=currimage.src;

   for (i=0; i<NMAXCOLORS; i++)
   {
      R[i]=currimage.R[i];
      G[i]=currimage.G[i];
      B[i]=currimage.B[i];
      colorname[i]=currimage.colorname[i];
   }
}	

// Overload = operator:

webimage webimage::operator= (const webimage currimage)
{
   docopy(currimage);
   return *this;
}

// ---------------------------------------------------------------------
// Subroutine intialize_RGBtable reads in the contents of the RGB
// table stored within /usr/X11R6/lib/X11/rgb.txt and attempts to
// correlate the RGB values with the common string color names.  This
// routine is successful for single word color names.  However, it
// fails for color names which have more than one word.  As of
// 6/16/00, we can live with this problem.  But we should go back and
// fix this up someday in the future when we have more time.

void webimage::initialize_RGBtable()
{
   const string lowerletters="abcdfghijklmnopqrstuvwxyz";
   const string upperletters="ABCDFGHIJKLMNOPQRSTUVWXYZ";
   const string letters=lowerletters+upperletters;

   int i,nlines,length;
   double X[3];
   string rgbfilename,currline,line[1000];
   ifstream rgbstream;

   rgbfilename="/usr/X11R6/lib/X11/rgb.txt";
   filefunc::openfile(rgbfilename,rgbstream);
   filefunc::ReadInfile(rgbfilename,line,nlines);

   for (i=0; i<nlines-1; i++)
   {
      currline=line[i+1];
      stringfunc::string_to_n_numbers(3,currline,X);
      R[i]=int(X[0]);
      G[i]=int(X[1]);
      B[i]=int(X[2]);
      string::size_type letterpos=currline.find_first_of(letters);
      length=currline.size();
      colorname[i]=currline.substr(letterpos,length);
   }
   ncolors=nlines-1;
}

// ---------------------------------------------------------------------
bool webimage::find_RGB(string colorstring,int& r,int& g,int& b)
{
   bool foundcolor=false;
   int i;

   i=0;
   while (!foundcolor && i<NMAXCOLORS)
   {
      if (colorstring==colorname[i])
      {
         foundcolor=true;
         r=R[i];
         g=G[i];
         b=B[i];
      }
      i++;
   }
   return foundcolor;
}

