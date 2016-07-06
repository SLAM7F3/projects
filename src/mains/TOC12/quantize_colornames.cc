// ==========================================================================
// Program QUANTIZE_COLORNAMES loops over all HSV coordinates.  For
// each color, it exports a quantized color name.  As of 9/10/12, we
// do NOT need to use this program in order to generate a lookup table
// for 16M RGB triples.
// ==========================================================================
// Last updated on 9/8/12; 9/9/12; 9/10/12
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "video/RGB_analyzer.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   string color_subdir="/home/cho/programs/c++/svn/projects/src/color/";
   string quantized_colors_filename=color_subdir+"quantized_color_names.dat";
   ofstream colorstream;
   filefunc::openfile(quantized_colors_filename,colorstream);
   colorstream << "# color name	R	G	B" << endl << endl;

   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();

   int n_quantized_colors=0;
   double r,g,b;
   for (int hue=0; hue<360; hue += 5)
   {
      for (int saturation=0; saturation <=100; saturation += 5)
      {
         double s=0.01*saturation;
         for (int value=0; value <= 100; value += 5)
         {
            double v=0.01*value;
            colorfunc::hsv_to_RGB(hue,s,v,r,g,b);
            int R=basic_math::round(255*r);
            int G=basic_math::round(255*g);
            int B=basic_math::round(255*b);
            string quantized_color_name=RGB_analyzer_ptr->
               compute_quantized_color_name(R,G,B);
            colorstream << quantized_color_name << "\t\t"
                        << R << "\t"
                        << G << "\t"
                        << B << "\t // " 
                        << " h=" << hue << ", s=" << saturation
                        << ", v=" << value
                        << endl;
            n_quantized_colors++;
         }
      } // loop over saturation
   } // loop over hue 

   filefunc::closefile(quantized_colors_filename,colorstream);

   delete RGB_analyzer_ptr;

   string banner="Exported "+stringfunc::number_to_string(n_quantized_colors)
      +" quantized colors to "+quantized_colors_filename;
   outputfunc::write_big_banner(banner);
}

