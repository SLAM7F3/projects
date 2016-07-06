// ========================================================================
// RGB_analyzer 
// ========================================================================
// Last updated on 8/2/13; 11/17/13; 5/7/14; 5/10/14
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "image/extremal_region.h"
#include "general/filefuncs.h"
#include "math/genvector.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "image/recursivefuncs.h"
#include "video/RGB_analyzer.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "coincidence_processing/VolumetricCoincidenceProcessor.h"
#include "graphs/vptree.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::map;
using std::ostream;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ========================================================================
// Initialization, constructor and destructor member functions
// ========================================================================

void RGB_analyzer::allocate_member_objects()
{
//   cout << "inside RGB_analyzer::allocate_member_objects()" << endl;
//   cout << "this = " << this << endl;
//    vptree_ptr=new vptree();
}		       
 
// ----------------------------------------------------------------
void RGB_analyzer::initialize_member_objects()
{
//   cout << "inside RGB_analyzer::initialize_member_objects()" << endl;
//    vptree_ptr->set_sqrd_Euclidean_distance_flag(true);
   colorindex_twoDarray_ptr=NULL;
   quantized_color_lookup_vector_ptr=NULL;

   reset_quantized_color_borders();
   initialize_color_maps();
   initialize_color_metric();
}

// ----------------------------------------------------------------
RGB_analyzer::RGB_analyzer()
{
//    cout << "inside RGB_analyzer constructor#1" << endl;
//    cout << "this = " << this << endl;

   allocate_member_objects();
   initialize_member_objects();
}

// ----------------------------------------------------------------
RGB_analyzer::~RGB_analyzer()
{
//   cout << "inside RGB_analyzer destructor" << endl;
//    delete vptree_ptr;
   delete colorindex_twoDarray_ptr;
   delete M_color_ptr;

// Iterate over all entries within quantized_colors_map and delete
// each corresponding *quantized_color_lookup_vector_ptr:

   for (quantized_colors_iter=quantized_colors_map.begin();
        quantized_colors_iter != quantized_colors_map.end();
        quantized_colors_iter++)
   {
      delete quantized_colors_iter->second;
   }
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const RGB_analyzer& A)
{
   outstream << "inside RGB_analyzer::operator<<" << endl;
   return outstream;
}

// ========================================================================
// Set & get member functions
// ========================================================================

string RGB_analyzer::get_color_name(int color_index) 
{
   string color_name=colorindices_map[color_index];
   return color_name;
}

int RGB_analyzer::get_color_index(string color_name) 
{
//   cout << "inside RGB_analyzer::get_color_index, color_name = "
//        << color_name << endl;
//   colornames_iter=colornames_map.find(color_name);
//   int color_index=colornames_iter->second;
   int color_index=colornames_map[color_name];
   return color_index;
}

// ----------------------------------------------------------------
string RGB_analyzer::get_hue_given_index(int h)
{
   if (h==0)
   {
      return "red";
   }
   else if (h==1)
   {
      return "orange";
   }
   else if (h==2)
   {
      return "yellow";
   }
   else if (h==3)
   {
      return "green";
   }
   else if (h==4)
   {
      return "cyan";
   }
   else if (h==5)
   {
      return "blue";
   }
   else if (h==6)
   {
      return "purple";
   }
   else
   {
      return "unknown hue";
   }
}

// ----------------------------------------------------------------
int RGB_analyzer::get_index_given_hue(string hue_name)
{
   if (hue_name=="red")
   {
      return 0;
   }
   else if (hue_name=="orange")
   {
      return 1;
   }
   else if (hue_name=="yellow")
   {
      return 2;
   }
   else if (hue_name=="green")
   {
      return 3;
   }
   else if (hue_name=="cyan")
   {
      return 4;
   }
   else if (hue_name=="blue")
   {
      return 5;
   }
   else if (hue_name=="purple")
   {
      return 6;
   }
   else
   {
      return -1;
   }
}

// ========================================================================
// Quantized color computation member functions
// ========================================================================

// Member function initialize_color_maps() breaks apart HSV color
// space into distinct regions.  For example if hue=green, we work
// with green, dark green, black, light green, grey-green and grey
// depending upon combinations of saturation and value color
// coordinates.  After much trial and error in Sep 2012, we
// empirically found that 256x256x256=16M RGB triples can be
// reasonably approximated by the O(30) quantized colors below.  But
// smaller numbers of quantized colors do NOT work well.  

void RGB_analyzer::initialize_color_maps()
{
//   cout << "inside RGB_analyzer::initialize_color_maps()" << endl;
   
   colornames_map["red"]=0;
   colornames_map["orange"]=1;
   colornames_map["yellow"]=2;
   colornames_map["green"]=3;
   colornames_map["cyan"]=4;
   colornames_map["blue"]=5;
   colornames_map["purple"]=6;

   colornames_map["lightred"]=7;
   colornames_map["lightorange"]=8;
   colornames_map["lightyellow"]=9;
   colornames_map["lightgreen"]=10;
   colornames_map["lightcyan"]=11;
   colornames_map["lightblue"]=12;
   colornames_map["lightpurple"]=13;

   colornames_map["darkred"]=14;
   colornames_map["darkorange"]=15;
   colornames_map["darkyellow"]=16;
   colornames_map["darkgreen"]=17;
   colornames_map["darkcyan"]=18;
   colornames_map["darkblue"]=19;
   colornames_map["darkpurple"]=20;

   colornames_map["greyred"]=21;
   colornames_map["greyorange"]=22;
   colornames_map["greyyellow"]=23;
   colornames_map["greygreen"]=24;
   colornames_map["greycyan"]=25;
   colornames_map["greyblue"]=26;
   colornames_map["greypurple"]=27;

   colornames_map["white"]=28;
   colornames_map["lightgrey"]=29;
   colornames_map["grey"]=30;
   colornames_map["darkgrey"]=31;
   colornames_map["black"]=32;

   int R,G,B;
   for (colornames_iter=colornames_map.begin();
        colornames_iter != colornames_map.end(); colornames_iter++)
   {
      string curr_colorname=colornames_iter->first;
      int curr_colorindex=colornames_iter->second;
      colorindices_map[curr_colorindex]=curr_colorname;

      compute_quantized_RGB_given_colorname(curr_colorname,R,G,B);
      indexRGB_map[curr_colorindex]=triple(R,G,B);
   }
}

// ----------------------------------------------------------------
// Member function initialize_color_metric() instantiates color
// metric matrix *M_color_ptr which contains manually assigned overlap
// between quantized colors.  Each color is assigned unit overlap with itself.
// The color metric also assigns "alpha" overlap
// between "single-hop" colors in HSV space.  For example,
// M_color("red","lightred") = M_color("lightred","greyred")
// = M_color("greyred","darkred") = M_color("darkred","red") = alpha.
// Similarly, M_color("red","orange") = M_color("orange","yellow") = 
// M_color("white","lightgrey") = alpha.

// Using some judgment, we also assign "sqr(alpha)" overlap between
// most (but not all) "double-hop" colors in HSV space.

// Note: Figure 3 in TOC12 writeup "sign_recog_w_figs.pdf" illustrates
// neighboring quantized color sectors in an SV plane of HSV color
// space.

void RGB_analyzer::initialize_color_metric()
{
   int n_colors=get_n_color_indices();
   M_color_ptr = new genmatrix(n_colors,n_colors);
   M_color_ptr->identity();

   const double alpha = 0.5;

// Single color-sector hops:

   M_color_ptr->put(0,1,alpha);
   M_color_ptr->put(0,6,alpha);
   M_color_ptr->put(0,7,alpha);
   M_color_ptr->put(0,14,alpha);
   M_color_ptr->put(1,2,alpha);
   M_color_ptr->put(1,8,alpha);
   M_color_ptr->put(1,15,alpha);
   M_color_ptr->put(2,3,alpha);
   M_color_ptr->put(2,9,alpha);
   M_color_ptr->put(2,16,alpha);
   M_color_ptr->put(3,4,alpha);
   M_color_ptr->put(3,10,alpha);
   M_color_ptr->put(3,17,alpha);
   M_color_ptr->put(4,5,alpha);
   M_color_ptr->put(4,11,alpha);
   M_color_ptr->put(4,18,alpha);
   M_color_ptr->put(5,6,alpha);
   M_color_ptr->put(5,12,alpha);
   M_color_ptr->put(5,19,alpha);
   M_color_ptr->put(6,13,alpha);
   M_color_ptr->put(6,20,alpha);
   M_color_ptr->put(7,8,alpha);
   M_color_ptr->put(7,13,alpha);
   M_color_ptr->put(7,21,alpha);
   M_color_ptr->put(7,28,alpha);
   M_color_ptr->put(7,29,alpha);
   M_color_ptr->put(7,30,alpha);
   M_color_ptr->put(8,9,alpha);
   M_color_ptr->put(8,22,alpha);
   M_color_ptr->put(8,28,alpha);
   M_color_ptr->put(8,29,alpha);
   M_color_ptr->put(8,30,alpha);
   M_color_ptr->put(9,10,alpha);
   M_color_ptr->put(9,23,alpha);
   M_color_ptr->put(9,28,alpha);
   M_color_ptr->put(9,29,alpha);
   M_color_ptr->put(9,30,alpha);
   M_color_ptr->put(10,11,alpha);
   M_color_ptr->put(10,24,alpha);
   M_color_ptr->put(10,28,alpha);
   M_color_ptr->put(10,29,alpha);
   M_color_ptr->put(10,30,alpha);
   M_color_ptr->put(11,12,alpha);
   M_color_ptr->put(11,25,alpha);
   M_color_ptr->put(11,28,alpha);
   M_color_ptr->put(11,29,alpha);
   M_color_ptr->put(11,30,alpha);
   M_color_ptr->put(12,13,alpha);
   M_color_ptr->put(12,26,alpha);
   M_color_ptr->put(12,28,alpha);
   M_color_ptr->put(12,29,alpha);
   M_color_ptr->put(12,30,alpha);
   M_color_ptr->put(13,27,alpha);
   M_color_ptr->put(13,28,alpha);
   M_color_ptr->put(13,29,alpha);
   M_color_ptr->put(13,30,alpha);
   M_color_ptr->put(14,15,alpha);
   M_color_ptr->put(14,20,alpha);
   M_color_ptr->put(14,21,alpha);
   M_color_ptr->put(14,32,alpha);
   M_color_ptr->put(15,16,alpha);
   M_color_ptr->put(15,22,alpha);
   M_color_ptr->put(15,32,alpha);
   M_color_ptr->put(16,17,alpha);
   M_color_ptr->put(16,23,alpha);
   M_color_ptr->put(16,32,alpha);
   M_color_ptr->put(17,18,alpha);
   M_color_ptr->put(17,24,alpha);
   M_color_ptr->put(17,32,alpha);
   M_color_ptr->put(18,19,alpha);
   M_color_ptr->put(18,25,alpha);
   M_color_ptr->put(18,32,alpha);
   M_color_ptr->put(19,20,alpha);
   M_color_ptr->put(19,26,alpha);
   M_color_ptr->put(19,32,alpha);
   M_color_ptr->put(20,27,alpha);
   M_color_ptr->put(20,32,alpha);
   M_color_ptr->put(21,22,alpha);
   M_color_ptr->put(21,27,alpha);
   M_color_ptr->put(21,30,alpha);
   M_color_ptr->put(21,31,alpha);
   M_color_ptr->put(21,32,alpha);
   M_color_ptr->put(22,23,alpha);
   M_color_ptr->put(22,30,alpha);
   M_color_ptr->put(22,31,alpha);
   M_color_ptr->put(22,32,alpha);
   M_color_ptr->put(23,24,alpha);
   M_color_ptr->put(23,30,alpha);
   M_color_ptr->put(23,31,alpha);
   M_color_ptr->put(23,32,alpha);
   M_color_ptr->put(24,25,alpha);
   M_color_ptr->put(24,30,alpha);
   M_color_ptr->put(24,31,alpha);
   M_color_ptr->put(24,32,alpha);
   M_color_ptr->put(25,26,alpha);
   M_color_ptr->put(25,30,alpha);
   M_color_ptr->put(25,31,alpha);
   M_color_ptr->put(25,32,alpha);
   M_color_ptr->put(26,27,alpha);
   M_color_ptr->put(26,30,alpha);
   M_color_ptr->put(26,31,alpha);
   M_color_ptr->put(26,32,alpha);
   M_color_ptr->put(27,30,alpha);
   M_color_ptr->put(27,31,alpha);
   M_color_ptr->put(27,32,alpha);
   M_color_ptr->put(28,29,alpha);
   M_color_ptr->put(29,30,alpha);
   M_color_ptr->put(30,31,alpha);
   M_color_ptr->put(31,32,alpha);

// Double color-sector hops:

   M_color_ptr->put(0,8,sqr(alpha));
   M_color_ptr->put(0,13,sqr(alpha));
   M_color_ptr->put(0,15,sqr(alpha));
   M_color_ptr->put(0,20,sqr(alpha));
   M_color_ptr->put(0,21,sqr(alpha));
   M_color_ptr->put(1,7,sqr(alpha));
   M_color_ptr->put(1,9,sqr(alpha));
   M_color_ptr->put(1,14,sqr(alpha));
   M_color_ptr->put(1,16,sqr(alpha));
   M_color_ptr->put(1,22,sqr(alpha));
   M_color_ptr->put(2,8,sqr(alpha));
   M_color_ptr->put(2,10,sqr(alpha));
   M_color_ptr->put(2,15,sqr(alpha));
   M_color_ptr->put(2,17,sqr(alpha));
   M_color_ptr->put(2,23,sqr(alpha));
   M_color_ptr->put(3,9,sqr(alpha));
   M_color_ptr->put(3,11,sqr(alpha));
   M_color_ptr->put(3,16,sqr(alpha));
   M_color_ptr->put(3,18,sqr(alpha));
   M_color_ptr->put(3,24,sqr(alpha));
   M_color_ptr->put(4,10,sqr(alpha));
   M_color_ptr->put(4,12,sqr(alpha));
   M_color_ptr->put(4,17,sqr(alpha));
   M_color_ptr->put(4,19,sqr(alpha));
   M_color_ptr->put(4,25,sqr(alpha));
   M_color_ptr->put(5,11,sqr(alpha));
   M_color_ptr->put(5,13,sqr(alpha));
   M_color_ptr->put(5,18,sqr(alpha));
   M_color_ptr->put(5,20,sqr(alpha));
   M_color_ptr->put(5,26,sqr(alpha));
   M_color_ptr->put(6,7,sqr(alpha));
   M_color_ptr->put(6,12,sqr(alpha));
   M_color_ptr->put(6,14,sqr(alpha));
   M_color_ptr->put(6,19,sqr(alpha));
   M_color_ptr->put(6,27,sqr(alpha));
   M_color_ptr->put(7,1,sqr(alpha));
   M_color_ptr->put(7,6,sqr(alpha));
   M_color_ptr->put(7,14,sqr(alpha));
   M_color_ptr->put(7,22,sqr(alpha));
   M_color_ptr->put(7,27,sqr(alpha));
   M_color_ptr->put(8,0,sqr(alpha));
   M_color_ptr->put(8,2,sqr(alpha));
   M_color_ptr->put(8,15,sqr(alpha));
   M_color_ptr->put(8,21,sqr(alpha));
   M_color_ptr->put(8,23,sqr(alpha));
   M_color_ptr->put(9,1,sqr(alpha));
   M_color_ptr->put(9,3,sqr(alpha));
   M_color_ptr->put(9,16,sqr(alpha));
   M_color_ptr->put(9,22,sqr(alpha));
   M_color_ptr->put(9,24,sqr(alpha));
   M_color_ptr->put(10,2,sqr(alpha));
   M_color_ptr->put(10,4,sqr(alpha));
   M_color_ptr->put(10,17,sqr(alpha));
   M_color_ptr->put(10,23,sqr(alpha));
   M_color_ptr->put(10,25,sqr(alpha));
   M_color_ptr->put(11,3,sqr(alpha));
   M_color_ptr->put(11,5,sqr(alpha));
   M_color_ptr->put(11,18,sqr(alpha));
   M_color_ptr->put(11,24,sqr(alpha));
   M_color_ptr->put(11,26,sqr(alpha));
   M_color_ptr->put(12,4,sqr(alpha));
   M_color_ptr->put(12,6,sqr(alpha));
   M_color_ptr->put(12,19,sqr(alpha));
   M_color_ptr->put(12,25,sqr(alpha));
   M_color_ptr->put(12,27,sqr(alpha));
   M_color_ptr->put(13,0,sqr(alpha));
   M_color_ptr->put(13,5,sqr(alpha));
   M_color_ptr->put(13,20,sqr(alpha));
   M_color_ptr->put(13,21,sqr(alpha));
   M_color_ptr->put(13,26,sqr(alpha));
   M_color_ptr->put(14,1,sqr(alpha));
   M_color_ptr->put(14,6,sqr(alpha));
   M_color_ptr->put(14,7,sqr(alpha));
   M_color_ptr->put(14,22,sqr(alpha));
   M_color_ptr->put(14,27,sqr(alpha));
   M_color_ptr->put(15,0,sqr(alpha));
   M_color_ptr->put(15,2,sqr(alpha));
   M_color_ptr->put(15,8,sqr(alpha));
   M_color_ptr->put(15,21,sqr(alpha));
   M_color_ptr->put(15,23,sqr(alpha));
   M_color_ptr->put(16,1,sqr(alpha));
   M_color_ptr->put(16,3,sqr(alpha));
   M_color_ptr->put(16,9,sqr(alpha));
   M_color_ptr->put(16,22,sqr(alpha));
   M_color_ptr->put(16,24,sqr(alpha));
   M_color_ptr->put(17,2,sqr(alpha));
   M_color_ptr->put(17,4,sqr(alpha));
   M_color_ptr->put(17,10,sqr(alpha));
   M_color_ptr->put(17,23,sqr(alpha));
   M_color_ptr->put(17,25,sqr(alpha));
   M_color_ptr->put(18,3,sqr(alpha));
   M_color_ptr->put(18,5,sqr(alpha));
   M_color_ptr->put(18,11,sqr(alpha));
   M_color_ptr->put(18,24,sqr(alpha));
   M_color_ptr->put(18,26,sqr(alpha));
   M_color_ptr->put(19,4,sqr(alpha));
   M_color_ptr->put(19,6,sqr(alpha));
   M_color_ptr->put(19,12,sqr(alpha));
   M_color_ptr->put(19,25,sqr(alpha));
   M_color_ptr->put(19,27,sqr(alpha));
   M_color_ptr->put(20,0,sqr(alpha));
   M_color_ptr->put(20,5,sqr(alpha));
   M_color_ptr->put(20,13,sqr(alpha));
   M_color_ptr->put(20,21,sqr(alpha));
   M_color_ptr->put(20,26,sqr(alpha));
   M_color_ptr->put(21,0,sqr(alpha));
   M_color_ptr->put(21,8,sqr(alpha));
   M_color_ptr->put(21,13,sqr(alpha));
   M_color_ptr->put(21,15,sqr(alpha));
   M_color_ptr->put(21,20,sqr(alpha));
   M_color_ptr->put(22,1,sqr(alpha));
   M_color_ptr->put(22,7,sqr(alpha));
   M_color_ptr->put(22,9,sqr(alpha));
   M_color_ptr->put(22,14,sqr(alpha));
   M_color_ptr->put(22,16,sqr(alpha));
   M_color_ptr->put(23,2,sqr(alpha));
   M_color_ptr->put(23,8,sqr(alpha));
   M_color_ptr->put(23,10,sqr(alpha));
   M_color_ptr->put(23,15,sqr(alpha));
   M_color_ptr->put(23,17,sqr(alpha));
   M_color_ptr->put(24,3,sqr(alpha));
   M_color_ptr->put(24,9,sqr(alpha));
   M_color_ptr->put(24,11,sqr(alpha));
   M_color_ptr->put(24,16,sqr(alpha));
   M_color_ptr->put(24,18,sqr(alpha));
   M_color_ptr->put(25,4,sqr(alpha));
   M_color_ptr->put(25,10,sqr(alpha));
   M_color_ptr->put(25,12,sqr(alpha));
   M_color_ptr->put(25,17,sqr(alpha));
   M_color_ptr->put(25,19,sqr(alpha));
   M_color_ptr->put(26,5,sqr(alpha));
   M_color_ptr->put(26,11,sqr(alpha));
   M_color_ptr->put(26,13,sqr(alpha));
   M_color_ptr->put(26,18,sqr(alpha));
   M_color_ptr->put(26,20,sqr(alpha));
   M_color_ptr->put(27,6,sqr(alpha));
   M_color_ptr->put(27,7,sqr(alpha));
   M_color_ptr->put(27,12,sqr(alpha));
   M_color_ptr->put(27,14,sqr(alpha));
   M_color_ptr->put(27,19,sqr(alpha));

   M_color_ptr->put(7,31,sqr(alpha));
   M_color_ptr->put(8,31,sqr(alpha));
   M_color_ptr->put(9,31,sqr(alpha));
   M_color_ptr->put(10,31,sqr(alpha));
   M_color_ptr->put(11,31,sqr(alpha));
   M_color_ptr->put(12,31,sqr(alpha));
   M_color_ptr->put(13,31,sqr(alpha));
   M_color_ptr->put(21,29,sqr(alpha));
   M_color_ptr->put(22,29,sqr(alpha));
   M_color_ptr->put(23,29,sqr(alpha));
   M_color_ptr->put(24,29,sqr(alpha));
   M_color_ptr->put(25,29,sqr(alpha));
   M_color_ptr->put(26,29,sqr(alpha));
   M_color_ptr->put(27,29,sqr(alpha));

   M_color_ptr->put(28,30,sqr(alpha));
   M_color_ptr->put(29,31,sqr(alpha));
   M_color_ptr->put(30,32,sqr(alpha));

/*

// Single-hops among color sectors with non-zero hue content:

   M_color_ptr->put(0,1,alpha);
   M_color_ptr->put(0,6,alpha);
   M_color_ptr->put(0,7,alpha);
   M_color_ptr->put(0,14,alpha);
   M_color_ptr->put(1,2,alpha);
   M_color_ptr->put(1,8,alpha);
   M_color_ptr->put(1,15,alpha);
   M_color_ptr->put(2,3,alpha);
   M_color_ptr->put(2,9,alpha);
   M_color_ptr->put(2,16,alpha);
   M_color_ptr->put(3,4,alpha);
   M_color_ptr->put(3,10,alpha);
   M_color_ptr->put(3,17,alpha);
   M_color_ptr->put(4,5,alpha);
   M_color_ptr->put(4,11,alpha);
   M_color_ptr->put(4,18,alpha);
   M_color_ptr->put(5,6,alpha);
   M_color_ptr->put(5,12,alpha);
   M_color_ptr->put(5,19,alpha);
   M_color_ptr->put(6,13,alpha);
   M_color_ptr->put(6,20,alpha);
   M_color_ptr->put(7,8,alpha);
   M_color_ptr->put(7,13,alpha);
   M_color_ptr->put(7,21,alpha);
   M_color_ptr->put(8,9,alpha);
   M_color_ptr->put(8,22,alpha);
   M_color_ptr->put(9,10,alpha);
   M_color_ptr->put(9,23,alpha);
   M_color_ptr->put(10,11,alpha);
   M_color_ptr->put(10,24,alpha);
   M_color_ptr->put(11,12,alpha);
   M_color_ptr->put(11,25,alpha);
   M_color_ptr->put(12,13,alpha);
   M_color_ptr->put(12,26,alpha);
   M_color_ptr->put(13,27,alpha);
   M_color_ptr->put(14,15,alpha);
   M_color_ptr->put(14,20,alpha);
   M_color_ptr->put(14,21,alpha);
   M_color_ptr->put(15,16,alpha);
   M_color_ptr->put(15,22,alpha);
   M_color_ptr->put(16,17,alpha);
   M_color_ptr->put(16,23,alpha);
   M_color_ptr->put(17,18,alpha);
   M_color_ptr->put(17,24,alpha);
   M_color_ptr->put(18,19,alpha);
   M_color_ptr->put(18,25,alpha);
   M_color_ptr->put(19,20,alpha);
   M_color_ptr->put(19,26,alpha);
   M_color_ptr->put(20,27,alpha);
   M_color_ptr->put(21,22,alpha);
   M_color_ptr->put(21,27,alpha);
   M_color_ptr->put(22,23,alpha);
   M_color_ptr->put(23,24,alpha);
   M_color_ptr->put(24,25,alpha);
   M_color_ptr->put(25,26,alpha);
   M_color_ptr->put(26,27,alpha);
*/

// Symmetrize entries within *M_color_ptr:

   for (int i=0; i<n_colors; i++)
   {
      for (int j=0; j<n_colors; j++)
      {
         if (M_color_ptr->get(i,j) > 0)
         {
            M_color_ptr->put(j,i,M_color_ptr->get(i,j));
         }
         if (M_color_ptr->get(j,i) > 0)
         {
            M_color_ptr->put(i,j,M_color_ptr->get(j,i));
         }
      }
   }

/*
// On 5/12/14, Martin Byrod suggested that we look at the SVD of our
// hand-crafted M_color.  In order for Vtrans * M * V to equal a
// constant for all color-space descriptors, M must have constant
// singular values!

   genmatrix Usorted(n_colors,n_colors),Wsorted(n_colors,n_colors),
      Vsorted(n_colors,n_colors);

   M_color_ptr->sorted_singular_value_decomposition(
      Usorted,Wsorted,Vsorted);
   for (int i=0; i<n_colors; i++)
   {
      cout << "i = " << i << " Wsorted = " << Wsorted.get(i) << endl;
   }
   
   cout << "det(M_color) = " << M_color_ptr->determinant() << endl;

i = 0 Wsorted = 6.09994966908
i = 1 Wsorted = 0
i = 2 Wsorted = 0
i = 3 Wsorted = 0
i = 4 Wsorted = 0
i = 5 Wsorted = 0
i = 6 Wsorted = 0
i = 7 Wsorted = 0
i = 8 Wsorted = 0
i = 9 Wsorted = 0
i = 10 Wsorted = 0
i = 11 Wsorted = 0
i = 12 Wsorted = 0
i = 13 Wsorted = 0
i = 14 Wsorted = 0
i = 15 Wsorted = 0
i = 16 Wsorted = 0
i = 17 Wsorted = 0
i = 18 Wsorted = 0
i = 19 Wsorted = 0
i = 20 Wsorted = 0
i = 21 Wsorted = 0
i = 22 Wsorted = 0
i = 23 Wsorted = 0
i = 24 Wsorted = 0
i = 25 Wsorted = 0
i = 26 Wsorted = 0
i = 27 Wsorted = 0
i = 28 Wsorted = 0
i = 29 Wsorted = 0
i = 30 Wsorted = 0
i = 31 Wsorted = 0
i = 32 Wsorted = 0
det(M_color) = -1.65482033463e-07

*/

}

// ----------------------------------------------------------------
vector<double> RGB_analyzer::propagate_color_hist(
   const vector<double>& color_hist)
{
   int n_colors=get_n_color_indices();   
   genvector C(n_colors), C_prop(n_colors);
   
   for (int i=0; i<n_colors; i++)
   {
      C.put(i,color_hist[i]);
   }

   C_prop = *M_color_ptr * C;

   vector<double> propagated_color_hist;
   for (int i=0; i<n_colors; i++)
   {
      propagated_color_hist.push_back(C_prop.get(i));
   }
   
   return propagated_color_hist;
}

// ----------------------------------------------------------------
// Member function compute_quantized_color_name() takes in R,G,B values
// ranging from [0,255].  It converts the input color to its HSV
// coordinates.  Using empirically derived HSV relationships, we
// assign one of our pre-determined names to the input color.

string RGB_analyzer::compute_quantized_color_name(int R,int G,int B)
{
//   cout << "inside RGB_analyzer::compute_quantized_color_name()" << endl;

   double r=R/255.0;
   double g=G/255.0;
   double b=B/255.0;
   double h,s,v;
   colorfunc::RGB_to_hsv(r,g,b,h,s,v);
   return compute_quantized_color_name_from_hsv(h,s,v);
}

string RGB_analyzer::compute_quantized_color_name_from_hsv(
   double h,double s,double v)
{
//   cout << "inside RGB_analyzer::compute_quantized_color_name_from_hsv()" << endl;

//   cout << "delta_orange_yellow = " << delta_orange_yellow
//        << " delta_yellow_green = " << delta_yellow_green << endl;

   string curr_hue;
   if (h >=0 && h <20+delta_red_orange)
   {
      curr_hue="red";
   }
   else if (h >=20+delta_red_orange && h <50+delta_orange_yellow)
   {
      curr_hue="orange";
   }
   else if (h >=50+delta_orange_yellow && h <71+delta_yellow_green)	 
   {
      curr_hue="yellow";
   }
   else if (h >=71+delta_yellow_green && h <165+delta_green_cyan)
   {
      curr_hue="green";
   }
   else if (h >=165+delta_green_cyan && h <190+delta_cyan_blue)
   {
      curr_hue="cyan";
   }
   else if (h >=190+delta_cyan_blue && h <255+delta_blue_purple)
   {
      curr_hue="blue";
   }
   else if (h >=255+delta_blue_purple && h <335+delta_purple_red)
   {
      curr_hue="purple";
   }
   else if (h >=335+delta_purple_red && h <361)
   {
      curr_hue="red";
   }

   string curr_value;
   if (v <= 0.10+delta_black_dark)
   {
      curr_value="black";
   }
   else if (v > 0.10+delta_black_dark && v < 0.5+delta_dark_bright)
   {
      curr_value="dark";
   }
   else
   {
      curr_value="bright";
   }

   string curr_saturation;
   if (s >= 0.5+delta_vivid_light)
   {
      curr_saturation="vivid";
   }
   else if (s < 0.15+delta_grey_light - 0.1*v)
   {
      curr_saturation="grey";
   }
   else
   {
      curr_saturation="light";
   }

   string quantized_color_name;
   if (curr_value=="black")
   {
      quantized_color_name="black";
   }
   else if (curr_value=="dark" && curr_saturation=="vivid")
   {
      quantized_color_name="dark"+curr_hue;
   }
   else if (curr_value=="bright" && curr_saturation=="vivid")
   {
      quantized_color_name=curr_hue;
   }
   else if (curr_value=="bright" && curr_saturation=="light")
   {
      quantized_color_name="light"+curr_hue;
   }
   else if (curr_value=="dark" && curr_saturation=="light")
   {
      quantized_color_name="grey"+curr_hue;
   }
   else if (curr_saturation=="grey")
   {
      if (v > 0.8+delta_lightgrey_white)
      {
         quantized_color_name="white";
      }
      else if (v > 0.6+delta_lightgrey_grey)
      {
         quantized_color_name="lightgrey";
      }
      else if (v > 0.4+delta_grey_darkgrey)
      {
         quantized_color_name="grey";
      }
      else if (v > 0.2+delta_darkgrey_black)
      {
         quantized_color_name="darkgrey";
      }
      else 
      {
         quantized_color_name="black";
      }
   }

   return quantized_color_name;
}

// ----------------------------------------------------------------
// Member function compute_quantized_color_index() takes in R,G,B values
// ranging from [0,255].  It first computes the associated quantized
// color name.  This method then searches colornames_map for the
// associated integer color index.

int RGB_analyzer::compute_quantized_color_index(int R,int G,int B)
{
//   cout << "inside RGB_analyzer::compute_quantized_color_index()" << endl;

   string quantized_color_name=compute_quantized_color_name(R,G,B);
   return get_color_index(quantized_color_name);
}

int RGB_analyzer::compute_quantized_color_index_from_hsv(
   double h,double s,double v)
{
//   cout << "inside RGB_analyzer::compute_quantized_color_index_from_hsv()" << endl;

   string quantized_color_name=compute_quantized_color_name_from_hsv(h,s,v);
   return get_color_index(quantized_color_name);
}

// ----------------------------------------------------------------
// Member function compute_quantized_RGB_given_colorname() assigns
// specific HSV values to each quantized color.  

void RGB_analyzer::compute_quantized_RGB_given_colorname(
   string colorname,int& R,int& G,int& B)
{
//   cout << "inside RGB_analyzer::compute_quantized_RGB_given_colorname()"
//   << endl;

   double hue=0;
   if (colorname=="red" || colorname=="lightred" || 
   colorname=="darkred" || colorname=="greyred")
   {
      hue=0;
   }
   else if (colorname=="orange" || colorname=="lightorange" || 
   colorname=="darkorange" || colorname=="greyorange")
   {
      hue=25;
//      hue=30;
   }
   else if (colorname=="yellow" || colorname=="lightyellow" || 
   colorname=="darkyellow" || colorname=="greyyellow")
   {
      hue=60;
   }
   else if (colorname=="green" || colorname=="lightgreen" || 
   colorname=="darkgreen" || colorname=="greygreen")
   {
      hue=120;
   }
   else if (colorname=="cyan" || colorname=="lightcyan" || 
   colorname=="darkcyan" || colorname=="greycyan")
   {
      hue=180;
   }
   else if (colorname=="blue" || colorname=="lightblue" || 
   colorname=="darkblue" || colorname=="greyblue")
   {
      hue=240;
   }
   else if (colorname=="purple" || colorname=="lightpurple" || 
   colorname=="darkpurple" || colorname=="greypurple")
   {
      hue=300;
   }

   double saturation=100;
   double value=100;
   if (colorname=="lightred" || colorname=="lightorange" || 
   colorname=="lightyellow" || colorname=="lightgreen" ||
   colorname=="lightcyan" || colorname=="lightblue" ||
   colorname=="lightpurple")
   {
      saturation=25;
   }
   else if (colorname=="darkred" || colorname=="darkorange" ||
   colorname=="darkyellow" || colorname=="darkgreen" ||
   colorname=="darkcyan" || colorname=="darkblue" ||
   colorname=="darkpurple")
   {
      value=50;
   }
   else if (colorname=="greyred" || colorname=="greyorange" ||
   colorname=="greyyellow" || colorname=="greygreen" ||
   colorname=="greycyan" || colorname=="greyblue" ||
   colorname=="greypurple")
   {
      saturation=30;
      value=40;
   }
   else if (colorname=="white")
   {
      saturation=0;
      value=100;
   }
   else if (colorname=="lightgrey")
   {
      saturation=0;
      value=75;
   }
   else if (colorname=="grey")
   {
      saturation=0;
      value=50;
   }
   else if (colorname=="darkgrey")
   {
      saturation=0;
      value=25;
   }
   else if (colorname=="black")
   {
      saturation=0;
      value=0;
   }

   double s=0.01*saturation;
   double v=0.01*value;

   double r,g,b;
   colorfunc::hsv_to_RGB(hue,s,v,r,g,b);
   R=basic_math::round(255*r);
   G=basic_math::round(255*g);
   B=basic_math::round(255*b);
}

// ----------------------------------------------------------------
// Member function reset_quantized_color_borders()

void RGB_analyzer::reset_quantized_color_borders()
{
//   cout << "inside RGB_analyzer::reset_quantized_color_borders()" << endl;
   
   delta_red_orange=delta_orange_yellow=delta_yellow_green=0;
   delta_green_cyan=delta_cyan_blue=delta_blue_purple=0;
   delta_purple_red=0;
   delta_black_dark=delta_dark_bright=0;
   max_black_saturation=1.0;

   delta_lightgrey_white=delta_lightgrey_grey=delta_grey_darkgrey=0;
   delta_darkgrey_black=delta_vivid_light=delta_grey_light=0;
}

// ----------------------------------------------------------------
// Member function liberalize_color_borders()

void RGB_analyzer::liberalize_color_borders(string liberalized_color)
{
   cout << "inside RGB_analyzer::liberalize_color_borders()" << endl;
   if (liberalized_color=="yellow")	// TOC12 yellow-radiation
   {
//      delta_orange_yellow=-5;
      delta_orange_yellow=-15;	// PointGrey
      delta_yellow_green=5;
   }
   else if (liberalized_color=="orange") // TOC12 biohazard
   {
      delta_red_orange=-18;
      delta_orange_yellow=-20;
      delta_vivid_light=0.05;
//      delta_vivid_light=0.25;
      delta_dark_bright=-0.15;
   }
   else if (liberalized_color=="red") // TOC12 stop
   {
      delta_red_orange=15;
      delta_purple_red=10;
//      delta_vivid_light=0.1;
      delta_vivid_light=0;	// PointGrey
      delta_dark_bright=-0.10;
   }
   else if (liberalized_color=="green") // TOC12 start
   {
      delta_orange_yellow=-8;
//      delta_yellow_green=0;
      delta_yellow_green=-15;	// PointGrey camera
      delta_green_cyan=0;
      delta_vivid_light=-0.1;
      delta_dark_bright=-0.15;
   }
   else if (liberalized_color=="blue")	// TOC12 water, blue-radiation, gas
   {
      delta_cyan_blue=15;
//      delta_blue_purple=25;
      delta_blue_purple=104;	// PointGrey camera
//      delta_vivid_light=-0.3;
      delta_vivid_light=-0.35;	// PointGrey camera
   }
   else if (liberalized_color=="blue2") // TOC12 gas
   {
// Center gas hue = 210
// Center gas sat = 100
// Center gas value = 75
//   else if (h >=190+delta_cyan_blue && h <255+delta_blue_purple)
// Min hue: 200
// Max hue: 249
// Min sat: 22
// Min value: 24

      delta_cyan_blue=5;
      delta_blue_purple=0;
      delta_vivid_light=-0.3;
   }
   else if (liberalized_color=="black") // TOC12 skull and eat
   {
      delta_grey_darkgrey=-0.15;   // make defn of dark grey MORE stringent
      delta_darkgrey_black=-0.15;	
   }
   else if (liberalized_color=="grey") // TOC12 skull and eat
   {
//      delta_vivid_light=-0.1;
//      delta_vivid_light=-0.15;
   }
   
}

// ========================================================================
// Quantized color retrieval member functions
// ========================================================================

// Member function retrieve_quantized_RGB_given_color_index() takes in
// integer color_index and searches indexRGB_map for the associated
// RGB triple.

void RGB_analyzer::retrieve_quantized_RGB_given_color_index(
   int color_index,int& R_quantized,int& G_quantized,int& B_quantized) const
{
//   cout << "inside RGB_analyzer::retrieve_quantized_RGB_given_color_index()"
//   << endl;

   IRGB_MAP::const_iterator irgb_iter=indexRGB_map.find(color_index);
   R_quantized=irgb_iter->second.first;
   G_quantized=irgb_iter->second.second;
   B_quantized=irgb_iter->second.third;
}

// ----------------------------------------------------------------
// Member function print_color_histogram() loops over all entries
// within an input color histogram whose size must equal
// get_n_color_indices().  It sorts the color histogram in
// descending order and ignores any values less than 1%.  The
// remaining entires are printed out alongside their color names.

void RGB_analyzer::print_color_histogram(
   const vector<double>& color_hist) 
{
   if (color_hist.size() != get_n_color_indices())
   {
      cout << "Error in RGB_analyzer::print_color_histogram()" << endl;
      cout << "color_hist.size() = " << color_hist.size() << endl;
      cout << "n_color_indices = " << get_n_color_indices() << endl;
      exit(-1);
   }
   
   vector<double> color_hist_values;
   vector<string> colornames;
   for (unsigned int i=0; i<color_hist.size(); i++)
   {
      color_hist_values.push_back(color_hist[i]);
      colornames.push_back(get_color_name(i));
   }
   
   templatefunc::Quicksort_descending(color_hist_values, colornames);

   cout << endl;
   cout << "Quantized color histogram percentages:" << endl << endl;

   const double SMALL=1E-2;
   for (unsigned int i=0; i<color_hist_values.size(); i++)
   {
      if (color_hist_values[i] < SMALL) break;
      cout << colornames[i] << " : " << color_hist_values[i]*100 
           << " % " << endl;
   }
   cout << endl;
}

// ----------------------------------------------------------------
// Member function color_histogram_inner_product() imports two color
// histograms.  It first recomputes both histograms' norms so that
// they are each L2 normalized.  It then uses color metric
// *M_color_ptr to compute the inner product between the two input
// color histograms.

double RGB_analyzer::color_histogram_inner_product(
   const vector<double>& color_hist1, const vector<double>& color_hist2)
{
   unsigned int n_colors=get_n_color_indices();
   if (color_hist1.size() != n_colors || color_hist2.size() != n_colors)
   {
      cout << "Error in RGB_analyzer::color_histogram_inner_product()!"
           << endl;
      cout << "n_colors = " << n_colors << endl;
      cout << "color_hist1.size() = " << color_hist1.size() << endl;
      cout << "color_hist2.size() = " << color_hist2.size() << endl;
   }

// Renormalize input color histograms so that they have unit L2-norm
// rather than unit L1-norm:
   
   double sqrd_norm1=0;
   double sqrd_norm2=0;
   for (unsigned int c=0; c<n_colors; c++)
   {
      sqrd_norm1 += sqr(color_hist1[c]);
      sqrd_norm2 += sqr(color_hist2[c]);
   }
   double norm1=sqrt(sqrd_norm1);
   double norm2=sqrt(sqrd_norm2);

   genvector hist1(n_colors), hist2(n_colors);
   for (unsigned int i=0; i<n_colors; i++)
   {
      hist1.put(i,color_hist1[i]/norm1);
      hist2.put(i,color_hist2[i]/norm2);
   }

   double hist1_norm = sqrt(hist1.dot(*M_color_ptr * hist1));
//   cout << "hist1_norm = " << hist1_norm << endl;
   double hist2_norm = sqrt(hist2.dot(*M_color_ptr * hist2));
//   cout << "hist2_norm = " << hist2_norm << endl;

   double inner_product = hist1.dot(*M_color_ptr * hist2) / 
      (hist1_norm * hist2_norm);
   return inner_product;
}

// ========================================================================
// Lookup table member functions
// ========================================================================

string RGB_analyzer::get_lookup_filename(string liberalized_color)
{
   string color_subdir=sysfunc::get_projectsrootdir()+"src/color/";
   string lookup_filename=color_subdir+"RGB_lookup";
   if (liberalized_color.length() > 0)
   {
      lookup_filename += "_liberalized_"+liberalized_color;
   }
   lookup_filename += ".table";   
   cout << "lookup_filename = " << lookup_filename << endl;
   return lookup_filename;
}

// ----------------------------------------------------------------
// Member function export_quantized_RGB_lookup_table() loops over all
// RGB triples and computes their associated quantized color indices.
// It exports a binary file containing 256x256x256 color indices as
// single byte values.

void RGB_analyzer::export_quantized_RGB_lookup_table(
   string liberalized_color)
{
   cout << "inside RGB_analyzer::export_quantized_RGB_lookup_table()" << endl;

   liberalize_color_borders(liberalized_color);

   string lookup_filename=get_lookup_filename(liberalized_color);
   ofstream lookup_stream;
   filefunc::open_binaryfile(lookup_filename,lookup_stream);

   timefunc::initialize_timeofday_clock();

   for (int R=0; R<256; R++)
   {
      cout << "R = " << R << " of 256" << endl;
      for (int G=0; G<256; G++)
      {
         for (int B=0; B<256; B++)
         {
            int color_index=compute_quantized_color_index(R,G,B);
            unsigned char I_byte=stringfunc::ascii_integer_to_unsigned_char(
               color_index);
            lookup_stream << I_byte;
//            cout << "R = " << R << " G = " << G << " B = " << B
//                 << " exported index = " << color_index << endl;
         } // loop over B index
      } // loop over G index
   } // loop over R index

   cout << "Elapsed time = " 
        << timefunc::elapsed_timeofday_time()/60 << " mins = " << endl;

   filefunc::closefile(lookup_filename,lookup_stream);

   string banner="Exported quantized RGB lookup table to "+lookup_filename;
   outputfunc::write_big_banner(banner);
}

// ----------------------------------------------------------------
// Member function import_quantized_RGB_lookup_table() parses the
// binary file generated by export_quantized_RGB_lookup_table().

void RGB_analyzer::import_quantized_RGB_lookup_table(
   string liberalized_color)
{
//   cout << "inside RGB_analyzer::import_quantized_RGB_lookup_table()" << endl;
//   cout << "Importing quantized RGB lookup table for liberalized color = "
//        << liberalized_color << endl;

// First check if a quantized RGB lookup table corresponding to the
// input liberalized color already exists within quantized_colors_map:

   quantized_colors_iter=quantized_colors_map.find(liberalized_color);
   if (quantized_colors_iter != quantized_colors_map.end())
   {
      cout << "Quantized lookup table for liberalized_color = "
           << liberalized_color << " has already been imported" << endl;
      return;
   }
   else
   {
      quantized_color_lookup_vector_ptr=new vector<int>;
      quantized_colors_map[liberalized_color]=
         quantized_color_lookup_vector_ptr;
   }

   string lookup_filename=get_lookup_filename(liberalized_color);

   size_t size=256*256*256;
   unsigned char* byte_data=
      filefunc::ReadUnsignedChars(lookup_filename,size);

   int byte_counter=0;
   for (int R=0; R<256; R++)
   {
//      if (R%20==0) cout << R << " " << flush;
      for (int G=0; G<256; G++)
      {
         for (int B=0; B<256; B++)
         {
            int color_index=stringfunc::unsigned_char_to_ascii_integer(
               byte_data[byte_counter++]);
            (*quantized_color_lookup_vector_ptr).push_back(color_index);

//            cout << "R = " << R << " G = " << G << " B = " << B 
//                 << " imported index = " << color_index << endl;
         } // loop over B index
      } // loop over G index
   } // loop over R index
   cout << endl;
}

// ----------------------------------------------------------------
// Member function retrieve_quantized_color_index_from_lookup_map()

int RGB_analyzer::retrieve_quantized_color_index_from_lookup_map(
   string lookup_map_name,int R,int G,int B) 
{
//   cout << "inside RGB_analyzer::retrieve_quantized_color_index_from_lookup_map()" << endl;
   
   quantized_colors_iter=
      quantized_colors_map.find(lookup_map_name);
   if (quantized_colors_iter==quantized_colors_map.end())
   {
      cout << "Error in RGB_analyzer::retrieve_quantized_color_index_from_lookup_map()" << endl;
      cout << "No lookup map found corresponding to " << lookup_map_name
           << endl;
      cout << "lookup_map_name.size() = " << lookup_map_name.size() << endl;
      exit(-1);
   }
   else
   {
      quantized_color_lookup_vector_ptr=quantized_colors_iter->second;
   }
   
   int color_index=quantized_color_lookup_vector_ptr->at(
      get_RGB_voxel_ID(R,G,B));
   return color_index;
}

// ----------------------------------------------------------------
// Member function retrieve_quantized_colorname_from_lookup_map()

string RGB_analyzer::retrieve_quantized_colorname_from_lookup_map(
   string lookup_map_name,int R,int G,int B) 
{
//   cout << "inside RGB_analyzer::retrieve_quantized_colorname_from_lookup_map()" << endl;

   int color_index=retrieve_quantized_color_index_from_lookup_map(
      lookup_map_name,R,G,B);
   string color_name=get_color_name(color_index);
   return color_name;
}

// ----------------------------------------------------------------
// Member function retrieve_quantized_RGB_from_lookup_map()

int RGB_analyzer::retrieve_quantized_RGB_from_lookup_map(
   string lookup_map_name,
   int R,int G,int B,int& R_quantized,int& G_quantized,int& B_quantized) 
{
//   cout << "inside RGB_analyzer::retrieve_quantized_RGB_from_lookup_map()" << endl;

   int color_index=retrieve_quantized_color_index_from_lookup_map(
      lookup_map_name,R,G,B);
   retrieve_quantized_RGB_given_color_index(
      color_index,R_quantized,G_quantized,B_quantized);
   return color_index;
}

// ========================================================================
// Image color quantization member functions
// ========================================================================

// Member function quantize_texture_rectangle_colors() replaces all
// pixels' RGB values with quantized red, orange, yellow, ... , black
// counterparts.  It also reinstantiates *colorindex_twoDarray_ptr and
// fills this twoDarray with quantized color index values for each
// pixel.

void RGB_analyzer::quantize_texture_rectangle_colors(
   texture_rectangle* texture_rectangle_ptr,string lookup_map_name)
{
//   cout << "inside RGB_analyzer::quantize_texture_rectangle_colors()" << endl;
//   cout << "lookup_map_name = " << lookup_map_name << endl;

   int R,G,B,R_quantized,G_quantized,B_quantized;
   int mdim=texture_rectangle_ptr->getWidth();
   int ndim=texture_rectangle_ptr->getHeight();
   
   delete colorindex_twoDarray_ptr;
   colorindex_twoDarray_ptr=new twoDarray(mdim,ndim);

   for (int py=0; py<ndim; py++)
   {
//      cout << "py = " << py << " of " << ndim << endl;
      for (int px=0; px<mdim; px++)
      {
         texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);

         int color_index=retrieve_quantized_RGB_from_lookup_map(
            lookup_map_name,R,G,B,R_quantized,G_quantized,B_quantized);

         colorindex_twoDarray_ptr->put(px,py,color_index);
         texture_rectangle_ptr->set_pixel_RGB_values(
            px,py,R_quantized,G_quantized,B_quantized);
      } // loop over px index
   } // loop over py index
}

// ----------------------------------------------------------------
// Member function smooth_quantized_image()

void RGB_analyzer::smooth_quantized_image(
   const texture_rectangle* texture_rectangle_ptr,
   texture_rectangle* quantized_texture_rectangle_ptr)
{
//   cout << "inside RGB_analyzer::smooth_quantized_image()" << endl;

   twoDarray* filtered_colorindex_twoDarray_ptr=new twoDarray(
      colorindex_twoDarray_ptr);
   colorindex_twoDarray_ptr->copy(filtered_colorindex_twoDarray_ptr);

   int R,G,B;
   int R_neighbor,G_neighbor,B_neighbor;

   int mdim=texture_rectangle_ptr->getWidth();
   int ndim=texture_rectangle_ptr->getHeight();

   int n_filtered_changes=0;
   const double local_threshold=50;
   for (int py=1; py<ndim-1; py++)
   {
//      cout << "py = " << py << " of " << ndim-1 << endl;
      for (int px=1; px<mdim-1; px++)
      {
         texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);

// Fetch quantized color index associated with current pixel:

         int color_index=colorindex_twoDarray_ptr->get(px,py);

         map<int,int> neighbor_color_index_map;
// indep var = neighbor color index
// depend var = count
         map<int,int>::iterator iter;

         int n_color_matches=0;
         for (int i=-1; i<=1; i++)
         {
//            cout << "i = " << i << endl;
            for (int j=-1; j<=1; j++)
            {
               if (i==0 && j==0) continue;

               texture_rectangle_ptr->get_pixel_RGB_values(
                  px+i,py+j,R_neighbor,G_neighbor,B_neighbor);

               if (colorfunc::color_match(
                  R,G,B,R_neighbor,G_neighbor,B_neighbor,local_threshold))
               {
                  n_color_matches++;
                  int neighbor_color_index=colorindex_twoDarray_ptr->
                     get(px+i,py+j);
                  
/*
                  if (neighbor_color_index != color_index)
                  {
                     cout << "R = " << R << " G = " << G << " B = " << B 
                          << " index = " << color_index << endl;

                     cout << "px+i = " << px+i << " py+j = " << py+j 
//                          << " i = " << i << " j = " << j 
                          << " R_neighbor = " << R_neighbor
                          << " G_neighbor = " << G_neighbor
                          << " B_neighbor = " << B_neighbor 
                          << " neighbor index = " << neighbor_color_index
                          << endl;
                     outputfunc::enter_continue_char();
                  }
*/

                  iter=neighbor_color_index_map.find(neighbor_color_index);
                  if (iter==neighbor_color_index_map.end())
                  {
                     neighbor_color_index_map[neighbor_color_index]=1;
                  }
                  else
                  {
                     iter->second=iter->second+1;
                  }
               }
            } // loop over index j 
         } // loop over index i 

//         cout << "n_color_matches = " << n_color_matches << endl;
         if (n_color_matches < 5) continue;

         int max_count=-1;
         int dominant_neighbor_color_index=-1;
         for (iter=neighbor_color_index_map.begin(); iter !=
                 neighbor_color_index_map.end(); iter++)
         {
            if (iter->second > max_count)
            {
               max_count=iter->second;
               dominant_neighbor_color_index=iter->first;
            }
         }

         if (dominant_neighbor_color_index != color_index)
         {
            n_filtered_changes++;
//            colorindex_twoDarray_ptr->put(px,py,dominant_neighbor_color_index);
            filtered_colorindex_twoDarray_ptr->put(
               px,py,dominant_neighbor_color_index);

//         cout << "max_count = " << max_count
//              << " dominant_neighbor_color_index = "
//              << dominant_neighbor_color_index << endl;

            IRGB_MAP::iterator irgb_iter=
               indexRGB_map.find(dominant_neighbor_color_index);
            int Rfiltered_quantized=irgb_iter->second.first;
            int Gfiltered_quantized=irgb_iter->second.second;
            int Bfiltered_quantized=irgb_iter->second.third;
         
//         cout << "Rnew = " << Rfiltered_quantized 
//              << " Gnew = " << Gfiltered_quantized
//              << " Bnew = " << Bfiltered_quantized 
//              << endl;
            quantized_texture_rectangle_ptr->set_pixel_RGB_values(
               px,py,Rfiltered_quantized,Gfiltered_quantized,
               Bfiltered_quantized);
         }

      } // loop over px
   } // loop over py

   filtered_colorindex_twoDarray_ptr->copy(colorindex_twoDarray_ptr);
   delete filtered_colorindex_twoDarray_ptr;

   cout << "n_filtered_changes = " << n_filtered_changes << endl;
}

// ----------------------------------------------------------------
// Member function identify_all_greyish_pixels() loops over all pixels
// within *texture_rectangle_ptr and retrieves their quantized color
// indices.  If a pixel's quantized color is sufficiently close to
// "grey", this method resets the pixel's color to
// R_grey,G_grey,B_grey.

void RGB_analyzer::identify_all_greyish_pixels(
   string lookup_map_name,texture_rectangle* texture_rectangle_ptr,
   int R_grey,int G_grey,int B_grey)
{
//   cout << "inside RGB_analyzer::identify_all_greyish_pixels()" << endl;

   int mdim=texture_rectangle_ptr->getWidth();
   int ndim=texture_rectangle_ptr->getHeight();

   int R_quantized,G_quantized,B_quantized;
   for (int py=0; py<ndim; py++)
   {
//      cout << "py = " << py << " of " << ndim << endl;
      for (int px=0; px<mdim; px++)
      {
         if (identify_greyish_pixel(
            lookup_map_name,px,py,texture_rectangle_ptr,
            R_quantized,G_quantized,B_quantized))
         {
            R_quantized=R_grey;
            G_quantized=G_grey;
            B_quantized=B_grey;
         }

         texture_rectangle_ptr->set_pixel_RGB_values(
            px,py,R_quantized,G_quantized,B_quantized);
      } // loop over px index
   } // loop over py index
}

// ----------------------------------------------------------------
bool RGB_analyzer::identify_greyish_pixel(
   string lookup_map_name,
   int px,int py,texture_rectangle* texture_rectangle_ptr)
{
//   cout << "inside RGB_analyzer::identify_greyish_pixel()" << endl;
   int R_quantized,G_quantized,B_quantized;
   return identify_greyish_pixel(
      lookup_map_name,px,py,texture_rectangle_ptr,
      R_quantized,G_quantized,B_quantized);
}

// ----------------------------------------------------------------
bool RGB_analyzer::identify_greyish_pixel(
   string lookup_map_name,
   int px,int py,texture_rectangle* texture_rectangle_ptr,
   int& R_quantized,int& G_quantized,int& B_quantized) 
{
//   cout << "inside RGB_analyzer::identify_greyish_pixel()" << endl;

   int R,G,B;
   texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
   int color_index=retrieve_quantized_RGB_from_lookup_map(
      lookup_map_name,R,G,B,R_quantized,G_quantized,B_quantized);

   bool grey_flag=false;
//   if (color_index >= 7 && color_index <=13) grey_flag=true;
//   if (color_index >= 21 && color_index <=27) grey_flag=true;
   if (color_index >= 28 && color_index <=30) grey_flag=true;
   return grey_flag;
}

// ----------------------------------------------------------------
// Member function isolate_quantized_colors() takes in an image with
// quantized colors along with a set of desired quantized color names.
// If selected_colors_texture_rectangle_ptr != NULL, it exports into
// *selected_colors_texture_rectangle_ptr a new quantized image
// where all colors not among the desired set are recolored dark
// purple.  This method next performs a few rounds of recursive
// filling in order to eliminate small noise islands.  Finally, it
// exports a binary mask for the desired quantized color regions in
// *binary_texture_rectangle_ptr.

void RGB_analyzer::isolate_quantized_colors(
   const texture_rectangle* quantized_texture_rectangle_ptr,
   vector<string> quantized_colornames,
   texture_rectangle* selected_colors_texture_rectangle_ptr,
   texture_rectangle* binary_texture_rectangle_ptr)
{
//   cout << "inside RGB_analyzer::isolate_quantized_colors()" << endl;

   COLORINDICES_MAP desired_colorindices_map;
   for (unsigned int i=0; i<quantized_colornames.size(); i++)
   {
      int color_index=get_color_index(quantized_colornames[i]);
      colorindices_iter=desired_colorindices_map.find(color_index);
      if (colorindices_iter==desired_colorindices_map.end())
      {
         desired_colorindices_map[color_index]=quantized_colornames[i];
      }
   } // loop over index i labeling quantized colornames to be isolated

   unsigned int mdim=colorindex_twoDarray_ptr->get_mdim();
   unsigned int ndim=colorindex_twoDarray_ptr->get_ndim();
   twoDarray* binary_twoDarray_ptr=new twoDarray(colorindex_twoDarray_ptr);
   binary_twoDarray_ptr->clear_values();

// Use dark purple as background color:

   int R,G,B;
   int background_R=96;
   int background_G=0;
   int background_B=96;

   for (unsigned int py=0; py<ndim; py++)
   {
      for (unsigned int px=0; px<mdim; px++)
      {

         if (selected_colors_texture_rectangle_ptr != NULL)
         {
            selected_colors_texture_rectangle_ptr->set_pixel_RGB_values(
               px,py,background_R,background_G,background_B); 
         }
   
         int curr_color_index=colorindex_twoDarray_ptr->get(px,py);
         colorindices_iter=desired_colorindices_map.find(curr_color_index);
         if (colorindices_iter != desired_colorindices_map.end())
         {
            if (selected_colors_texture_rectangle_ptr != NULL)
            {
               quantized_texture_rectangle_ptr->get_pixel_RGB_values(
                  px,py,R,G,B);
               selected_colors_texture_rectangle_ptr->set_pixel_RGB_values(
                  px,py,R,G,B);
            }
            binary_twoDarray_ptr->put(px,py,255);
         }

      } // loop over px
   } // loop over py
   
// Perform few rounds of recursive filling to eliminate small noise
// islands:

//   int max_recursion_levels=2;
   int max_recursion_levels=3;
   recursivefunc::binary_fill(
      max_recursion_levels,0,mdim,0,ndim,0,255,binary_twoDarray_ptr);

// Fill *binary_texture_rectangle_ptr with black and white values:

   for (unsigned int py=0; py<ndim; py++)
   {
      for (unsigned int px=0; px<mdim; px++)
      {
         if (binary_twoDarray_ptr->get(px,py) > 128)
         {
            binary_texture_rectangle_ptr->set_pixel_RGB_values(
               px,py,255,255,255);
         }
         else
         {
            binary_texture_rectangle_ptr->set_pixel_RGB_values(px,py,0,0,0);
         }

      } // loop over px
   } // loop over py

   delete binary_twoDarray_ptr;
}

// ----------------------------------------------------------------
// Member function analyze_extremal_region_hue_content()

int RGB_analyzer::dominant_extremal_region_hue_content(
   string lookup_map_name,const extremal_region* extremal_region_ptr,
   const texture_rectangle* quantized_texture_rectangle_ptr)
{
//   cout << endl;
//   cout << "inside RGB_analyzer::dominant_extremal_region_hue_content()" 
//        << endl;

   unsigned int xdim=quantized_texture_rectangle_ptr->getWidth();
   unsigned int px_start,px_stop,py;
   int R,G,B;
   vector<int> region_color_indices;
   for (unsigned int i=0; i<extremal_region_ptr->get_RLE_pixel_IDs().size(); 
        i += 2)
   {
      unsigned int start_pixel_ID=extremal_region_ptr->get_RLE_pixel_IDs().
         at(i);
      unsigned int stop_pixel_ID=extremal_region_ptr->get_RLE_pixel_IDs().
         at(i+1);
      graphicsfunc::get_pixel_px_py(start_pixel_ID,xdim,px_start,py);
      graphicsfunc::get_pixel_px_py(stop_pixel_ID,xdim,px_stop,py);

      for (unsigned int px=px_start; px<=px_stop; px++)
      {
         quantized_texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
//         cout << "px = " << px << " py = " << py << " R = " << R
//              << " G = " << G << " B = " << B << endl;
         int color_index=retrieve_quantized_color_index_from_lookup_map(
            lookup_map_name,R,G,B);
         region_color_indices.push_back(color_index);
      }
   } // loop over index i labeling RLE runs

 // Instantiate and fill hue_count histogram:

   vector<double> hue_count;
   for (int h=0; h<7; h++)
   {
      hue_count.push_back(0);
   }
   
   int n_hues=0;
   for (unsigned int c=0; c<region_color_indices.size(); c++)
   {
      int color_index=region_color_indices[c];
      if (color_index >= 28) continue;	// greys
      int hue_index=color_index%7;
      hue_count[hue_index]=hue_count[hue_index]+1;
      n_hues++;
   } // loop over index c labeling extremal region pixel color indices

   if (n_hues <= 1) return -1;

   int max_hue_index=-1;
   double max_hue_count=-1;
   for (int h=0; h<7; h++)
   {
      hue_count[h] /= double(n_hues);
      if (hue_count[h] > max_hue_count)
      {
         max_hue_count=hue_count[h];
         max_hue_index=h;
      }
   } // loop over index h labeling hues

//   cout << "red fraction = " << hue_count[0] << endl;
//   cout << "orange fraction = " << hue_count[1] << endl;
//   cout << "yellow fraction = " << hue_count[2] << endl;
//   cout << "green fraction = " << hue_count[3] << endl;
//   cout << "cyan fraction = " << hue_count[4] << endl;
//   cout << "blue fraction = " << hue_count[5] << endl;
//   cout << "purple fraction = " << hue_count[6] << endl;

   return max_hue_index;
}

// ========================================================================
// Bbox color content member functions
// ========================================================================

vector<double>& RGB_analyzer::compute_color_histogram(
   const texture_rectangle* texture_rectangle_ptr,
   string lookup_map_name)
{
//   cout << "inside RGB_analyzer::compute_color_histogram()" << endl;
//   cout << "lookup_map_name = " << lookup_map_name << endl;

   int left_pu=0;
   int right_pu=texture_rectangle_ptr->getWidth()-1;
   int top_pv=0;
   int bottom_pv=texture_rectangle_ptr->getHeight()-1;

   return compute_bbox_color_content(
      left_pu,top_pv,right_pu,bottom_pv,
      texture_rectangle_ptr,lookup_map_name);
}

// ----------------------------------------------------------------
// Member function compute_bbox_color_content()

vector<double>& RGB_analyzer::compute_bbox_color_content(
   int left_pu,int top_pv,int right_pu,int bottom_pv,
   const texture_rectangle* texture_rectangle_ptr,string lookup_map_name)
{
//   cout << "inside RGB_analyzer::compute_bbox_color_content()" << endl;
//   cout << "lookup_map_name = " << lookup_map_name << endl;

   int n_channels=texture_rectangle_ptr->getNchannels();
//   cout << "n_channels = " << n_channels << endl;

   color_fracs.clear();
   for (unsigned int i=0; i<get_n_color_indices(); i++)
   {
      color_fracs.push_back(0);
   }

   int R,G,B,A;
   int color_counter=0;
   for (int pv=top_pv; pv<=bottom_pv; pv++)
   {
//      cout << "pv = " << pv << endl;
      for (int pu=left_pu; pu<=right_pu; pu++)
      {
//         cout << "pu = " << pu << endl;
         if (n_channels==4)
         {
            texture_rectangle_ptr->get_pixel_RGBA_values(pu,pv,R,G,B,A);
         }
         else if (n_channels==3)
         {
            texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
         }
         else if (n_channels==1)
         {
            R=texture_rectangle_ptr->get_pixel_intensity_value(pu,pv);
            G=R;
            B=R;
         }
         
//         cout << "R = " << R << " G = " << G << " B = " << B << endl;
         int color_index=retrieve_quantized_color_index_from_lookup_map(
            lookup_map_name,R,G,B);

//         cout << "pu = " << pu 
//              << " color_index = " << color_index << endl;
         
         color_fracs[color_index]=color_fracs[color_index]+1;
         color_counter++;
      } // loop over pu index
   } // loop over pv index

//   const double color_frac_threshold=0.02;
//   vector<string> major_color_name;
//   vector<double> major_color_frac;

   for (unsigned int c=0; c<get_n_color_indices(); c++)
   {
      color_fracs[c] /= color_counter;

//      if (color_fracs[c] < color_frac_threshold) continue;
//      major_color_frac.push_back(color_fracs[c]);
//      major_color_name.push_back(get_color_name(c));

//      cout << "c = " << c
//           << " color = " << get_color_name(c)
//           << " color_frac = " << stringfunc::number_to_string(
//              color_fracs[c],3) 
//           << endl;
   }

//   templatefunc::Quicksort_descending(major_color_frac,major_color_name);

//   for (unsigned int m=0; m<major_color_frac.size(); m++)
//   {
//      cout << major_color_name[m] << "   "
//           << major_color_frac[m] << endl;
//   }
   
   return color_fracs;
}

// ----------------------------------------------------------------
// Member function vivid_hue_fraction()

double RGB_analyzer::vivid_hue_fraction()
{
   double hue_frac_integral=0;

/*   
   for (int c=0; c<=1; c++) // red, orange
   {
      hue_frac_integral += color_fracs[c];
   }
   for (int c=3; c<=6; c++) // green,cyan,blue,purple
   {
      hue_frac_integral += color_fracs[c];
   }
*/

   for (int c=0; c<=6; c++) // red,orange,yellow,green,cyan,blue,purple
   {
      hue_frac_integral += color_fracs[c];
   }

   for (int c=14; c<=20; c++) // darkred, darkoange, ..., darkpurple
   {
      hue_frac_integral += color_fracs[c];
   }

/*
   for (int c=21; c<=27; c++) // greyred, greyorange, ... greypurple
   {
      hue_frac_integral += color_fracs[c];
   }
*/

   return hue_frac_integral;
}

// ========================================================================
// RGB color averaging member functions
// ========================================================================

// Member function average_texture_rectangle_colors()

void RGB_analyzer::average_texture_rectangle_colors(
   int nsize,const texture_rectangle* texture_rectangle_ptr,
   texture_rectangle* averaged_texture_rectangle_ptr)
{
//   cout << "inside RGB_analyzer::average_texture_rectangle_colors()" << endl;

   int mdim=texture_rectangle_ptr->getWidth();
   int ndim=texture_rectangle_ptr->getHeight();
   
   int nx_size=nsize;
   int ny_size=nsize;
   int wx=(nx_size-1)/2;
   int wy=(ny_size-1)/2;

   int R,G,B;
   double Ravg,Gavg,Bavg;
   for (int py=wy; py<ndim-wy; py++)
   {
//      cout << "py = " << py << " of " << ndim << endl;
      for (int px=wx; px<mdim-wx; px++)
      {
         Ravg=Gavg=Bavg=0;
         for (int i=0; i<nx_size; i++)
         {
            for (int j=0; j<ny_size; j++)
            {
               texture_rectangle_ptr->get_pixel_RGB_values(
                  px-wx+i,py-wy+j,R,G,B);
               Ravg += R;
               Gavg += G;
               Bavg += B;
            } // loop over index j 
         } // loop over index i 
         Ravg /= (nx_size*ny_size);
         Gavg /= (nx_size*ny_size);
         Bavg /= (nx_size*ny_size);

//         cout << "px = " << px 
//              << " py = " << py
//              << " Ravg = " << Ravg
//              << " Gavg = " << Gavg
//              << " Bavg = " << Bavg
//              << endl;

         averaged_texture_rectangle_ptr->set_pixel_RGB_values(
            px,py,Ravg,Gavg,Bavg);
      } // loop over px index
   } // loop over py index
}

// ----------------------------------------------------------------
// Member function median_texture_rectangle_colors()

void RGB_analyzer::median_texture_rectangle_colors(
   int nsize,const texture_rectangle* texture_rectangle_ptr,
   texture_rectangle* median_texture_rectangle_ptr)
{
   cout << "inside RGB_analyzer::median_texture_rectangle_colors()" << endl;

   int mdim=texture_rectangle_ptr->getWidth();
   int ndim=texture_rectangle_ptr->getHeight();
   
   int nx_size=nsize;
   int ny_size=nsize;
   int wx=(nx_size-1)/2;
   int wy=(ny_size-1)/2;

   int R,G,B;
   vector<double> Rvalues,Gvalues,Bvalues;
   for (int py=wy; py<ndim-wy; py++)
   {
//      cout << "py = " << py << " of " << ndim << endl;
      for (int px=wx; px<mdim-wx; px++)
      {
         Rvalues.clear();
         Gvalues.clear();
         Bvalues.clear();
         for (int i=0; i<nx_size; i++)
         {
            for (int j=0; j<ny_size; j++)
            {
               texture_rectangle_ptr->get_pixel_RGB_values(
                  px-wx+i,py-wy+j,R,G,B);
               Rvalues.push_back(R);
               Gvalues.push_back(G);
               Bvalues.push_back(B);
            } // loop over index j 
         } // loop over index i 

         double Rmedian=mathfunc::median_value(Rvalues);
         double Gmedian=mathfunc::median_value(Gvalues);
         double Bmedian=mathfunc::median_value(Bvalues);

         median_texture_rectangle_ptr->set_pixel_RGB_values(
            px,py,Rmedian,Gmedian,Bmedian);
      } // loop over px index
   } // loop over py index
}

