// ==========================================================================
// COLORFUNCS stand-alone methods
// ==========================================================================
// Last modified on 2/11/16; 3/7/16; 3/9/16; 3/25/16
// ==========================================================================

#include <bitset>
#include <iostream>
#include "math/basic_math.h"
#include "math/constants.h"
#include "color/colorfuncs.h"
#include "color/colormapfuncs.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"

using std::bitset;
using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

namespace colorfunc
{

   int L;
   double* dataviewer_colormap=NULL;

// ---------------------------------------------------------------------
// Method get_n_colors() returns the number of colorfunc Colors:

   unsigned int get_n_colors()
      {
         const int NCOLORS=22;
         return NCOLORS;
      }

// ---------------------------------------------------------------------
// Method get_color() returns a metafile color corresponding to input
// integer i.

   Color get_color(int i)
      {
         if (i==0)
         {
            return blue;
         }
         else if (i==1)
         {
            return red;
         }
         else if (i==2)
         {
            return green;
         }
         else if (i==3)
         {
            return orange;
         }
         else if (i==4)
         {
            return pink;
         }
         else if (i==5)
         {
            return gold;
         }
         else if (i==6)
         {
            return cyan;
         }
         else if (i==7)
         {
            return yegr;
         }
         else if (i==8)
         {
            return purple;
         }
         else if (i==9)
         {
            return brick;
         }
         else if (i==10)
         {
            return blgr;
         }
         else if (i==11)
         {
            return yellow;
         }
         else if (i==12)
         {
            return grey;
         }
         else if (i==13)
         {
            return magenta;
         }
         else if (i==14)
         {
            return ltgr;
         }
         else if (i==15)
         {
            return cream;
         }
         else if (i==16)
         {
            return white;
         }
         else if (i==17)
         {
            return brightyellow;
         }
         else if (i==18)
         {
            return brightpurple;
         }
         else if (i==19)
         {
            return brightcyan;
         }
         else if (i==20)
         {
            return darkpurple;
         }
         else if (i==21)
         {
            return brown;
         }
         else // i==22
         {
            return black;
         }
      }

// ---------------------------------------------------------------------
// Method get_color_index() takes in a color's string and returns its
// corresponding integer index:

   int get_color_index(string color_str)
      {
         if (color_str=="blue")
         {
            return 0;
         }
         else if (color_str=="red")
         {
            return 1;
         }
         else if (color_str=="green")
         {
            return 2;
         }
         else if (color_str=="orange")
         {
            return 3;
         }
         else if (color_str=="pink")
         {
            return 4;
         }
         else if (color_str=="gold")
         {
            return 5;
         }
         else if (color_str=="cyan")
         {
            return 6;
         }
         else if (color_str=="yegr")
         {
            return 7;
         }
         else if (color_str=="purple")
         {
            return 8;
         }
         else if (color_str=="brick")
         {
            return 9;
         }
         else if (color_str=="blgr")
         {
            return 10;
         }
         else if (color_str=="yellow")
         {
            return 11;
         }
         else if (color_str=="grey")
         {
            return 12;
         }
         else if (color_str=="magenta")
         {
            return 13;
         }
         else if (color_str=="ltgr")
         {
            return 14;
         }
         else if (color_str=="cream")
         {
            return 15;
         }
         else if (color_str=="white")
         {
            return 16;
         }
         else if (color_str=="brightyellow")
         {
            return 17;
         }
         else if (color_str=="brightpurple")
         {
            return 18;
         }
         else if (color_str=="brightcyan")
         {
            return 19;
         }
         else if (color_str=="darkpurple")
         {
            return 20;
         }
         else if (color_str=="brown")
         {
            return 21;
         }
         else if (color_str=="black")
         {
            return 22;
         }
         else if (color_str=="null")
         {
            return 23;
         }
         else
         {
            return -1;
         }
      }

// ---------------------------------------------------------------------
// Method getcolor returns colors for metafile output which are based
// upon an input integer mod NCOLORS

   string getcolor(int i)
      {
         const int NCOLORS=23;
         if (isfinite(i))
         {
            return get_colorstr(get_color(i%NCOLORS));
         }
         else
         {
            cout << "Error inside colorfunc::getcolor()!" << endl;
            cout << "Input integer = " << i << endl;
            return get_colorstr(black);
         }
      }

   Color get_rainbowcolor(int i)
      {
         const int NCOLORS=10;
   
         if (isfinite(i))
         {
            Color color[NCOLORS];
            color[0]=purple;
            color[1]=blue;  
            color[2]=blgr;
            color[3]=green;
            color[4]=yegr;
            color[5]=yellow;
            color[6]=gold;
            color[7]=orange; 	
            color[8]=red;     
            color[9]=brick;
            return color[i%NCOLORS];
         }
         else
         {
            return black;
         }
      }

   string get_colorstr(const Color color)
      {
         string colorstr;
         if (color==blue)
         {
            colorstr="blue";
         }
         else if (color==red)
         {
            colorstr="red";
         }
         else if (color==green)
         {
            colorstr="green";
         }
         else if (color==orange)
         {
            colorstr="orange";
         }
         else if (color==pink)
         {
            colorstr="pink";
         }
         else if (color==gold)
         {
            colorstr="gold";
         }
         else if (color==cyan)
         {
            colorstr="cyan";
         }
         else if (color==yegr)
         {
            colorstr="yegr";
         }
         else if (color==purple)
         {
            colorstr="purple";
         }
         else if (color==brick)
         {
            colorstr="brick";
         }
         else if (color==blgr)
         {
            colorstr="blgr";
         }
         else if (color==yellow)
         {
            colorstr="yellow";
         }
         else if (color==grey)
         {
            colorstr="grey";
         }
         else if (color==magenta)
         {
            colorstr="magenta";
         }
         else if (color==ltgr)
         {
            colorstr="ltgr";
         }
         else if (color==cream)
         {
            colorstr="cream";
         }
         else if (color==white)
         {
            colorstr="white";
         }
         else if (color==brown)
         {
            colorstr="brown";
         }
         else
         {
            colorstr="black";
         }
         return colorstr;
      }

// ---------------------------------------------------------------------
   Color string_to_color(string colorstr)
      {
         if (colorstr=="blue")
         {
            return blue;
         }
         else if (colorstr=="red")
         {
            return red;
         }
         else if (colorstr=="green")
         {
            return green;
         }
         else if (colorstr=="orange")
         {
            return orange;
         }
         else if (colorstr=="pink")
         {
            return pink;
         }
         else if (colorstr=="gold")
         {
            return gold;
         }
         else if (colorstr=="cyan")
         {
            return cyan;
         }
         else if (colorstr=="yegr")
         {
            return yegr;
         }
         else if (colorstr=="purple")
         {
            return purple;
         }
         else if (colorstr=="brick")
         {
            return brick;
         }
         else if (colorstr=="blgr")
         {
            return blgr;
         }
         else if (colorstr=="yellow")
         {
            return yellow;
         }
         else if (colorstr=="grey")
         {
            return grey;
         }
         else if (colorstr=="magenta")
         {
            return magenta;
         }
         else if (colorstr=="ltgr")
         {
            return ltgr;
         }
         else if (colorstr=="cream")
         {
            return cream;
         }
         else if (colorstr=="white")
         {
            return white;
         }
         else if (colorstr=="brightyellow")
         {
            return brightyellow;
         }
         else if (colorstr=="brightpurple")
         {
            return brightpurple;
         }
         else if (colorstr=="brightcyan")
         {
            return brightcyan;
         }
         else if (colorstr=="darkpurple")
         {
            return darkpurple;
         }
         else if (colorstr=="brown")
         {
            return brown;
         }
         else if (colorstr=="black")
         {
            return black;
         }
         else if (colorstr=="null")
         {
            return null;
         }
         else
         {
            return black;
         }
      }

// ---------------------------------------------------------------------
// Method color_to_value returns relative intensity values which
// generally range between 0 to 60 dB.  This intensity range coincides
// with our default colortable which we heavily use for drawing
// two-dimensional images using Iva's metafile routines.

   double color_to_value(const Color color)
      {
         double value=0;
         if (color==black)
         {
            value=0;
         }
         else if (color==purple)
         {
            value=10;
         }
         else if (color==blue)
         {
            value=20;
         }
         else if (color==cyan)
         {
            value=30;
         }
         else if (color==green)
         {
            value=35;
         }
         else if (color==yegr)
         {
            value=40;
         }
         else if (color==yellow)
         {
            value=47;
         }
         else if (color==orange)
         {
            value=51;
         }
         else if (color==red)
         {
            value=60;
         }
         else if (color==brightyellow)
         {
            value=400;
         }
         else if (color==brightcyan)
         {
            value=501;
         }
         else if (color==brightpurple)
         {
            value=601;
         }
         else if (color==grey)
         {
            value=901;
         }
         else if (color==white)
         {
            value=1000;
         }
         return value;
      }

// ==========================================================================
// RGB methods
// ==========================================================================

   void print_RGB(const RGB& curr_rgb)
      {
         cout << "R = " << curr_rgb.first << " G = " << curr_rgb.second
              << " B = " << curr_rgb.third << endl;
      }

// ---------------------------------------------------------------------
   osg::Vec4 get_OSG_color(const Color color,double alpha)
      {
//         cout << "inside colorfunc::get_OSG_color(), alpha = " << alpha
//              << endl;

         RGB curr_RGB=get_RGB_values(color);
         return osg::Vec4(curr_RGB.first,curr_RGB.second,curr_RGB.third,
                          alpha);
      }

// ---------------------------------------------------------------------
   osg::Vec4 get_OSG_color(const RGB& rgb,double alpha)
      {
         return osg::Vec4(rgb.first,rgb.second,rgb.third,alpha);
      }

// ---------------------------------------------------------------------
   Color get_colorfunc_color(osg::Vec4 osg_color)
      {
         return get_color(RGB(osg_color.r(),osg_color.g(),osg_color.b()));
      }
   
// ---------------------------------------------------------------------
// Method get_RGB_values takes in a colorfunc::Color and returns a
// Triple of doubles containing RGB fraction values ranging from 0 to
// 1.  We used the GIMP's "color selection" tool to read off several
// of the RGB values listed below.

   RGB get_RGB_values(const Color color)
      {
         if (color==blue)
         {
            return RGB(0,0,1);
         }
         else if (color==red)
         {
            return RGB(1,0,0);
         }
         else if (color==green)
         {
            return RGB(0,1,0);
         }
         else if (color==orange)
         {
            return RGB(255.0/255.0 , 123.0/255.0, 0);
         }
         else if (color==pink)
         {
            return RGB(244.0/255.0 , 141.0/255.0, 239.0/255.0);
         }
         else if (color==gold)
         {
            return RGB(226.0/255.0 , 196.0/255.0, 104.0/255.0);
         }
         else if (color==cyan || color==brightcyan)
         {
            return RGB(0,1,1);
         }
         else if (color==yegr)
         {
            return RGB(185.0/255.0 , 201.0/255.0, 62.0/255.0);
         }
         else if (color==purple)
         {
            return RGB(1,0,1);
         }
         else if (color==brick)
         {
            return RGB(165.0/255.0 , 31.0/255.0, 8.0/255.0);
         }
         else if (color==blgr)
         {
            return RGB(10.0/255.0 , 173.0/255.0, 143.0/255.0);
         }
         else if (color==yellow || color==brightyellow)
         {
            return RGB(1,1,0);
         }
         else if (color==grey)
         {
            return RGB(0.5,0.5,0.5);
//            return RGB(0.8,0.8,0.8);
         }
         else if (color==magenta)
         {
            return RGB(204.0/255.0 , 12.0/255.0, 194.0/255.0);
         }
         else if (color==ltgr)
         {
            return RGB(158.0/255.0 , 224.0/255.0, 132.0/255.0);
         }
         else if (color==cream)
         {
            return RGB(239.0/255.0 , 239.0/255.0, 194.0/255.0);
         }
         else if (color==white)
         {
            return RGB(1,1,1);
         }
         else if (color==brightpurple)
         {
            return RGB(0.7833,0.0,0.9329);
         }
         else if (color==darkpurple)
         {
            return RGB(0.75,0.0,0.5);
         }
         else if (color==brown)
         {
            return RGB(0.3607,0.2,0.09019);
         }
         else if (color==black)
         {
            return RGB(0,0,0);
         }
	 return RGB(-1,-1,-1);
      }
   
// ---------------------------------------------------------------------
// Method get_color takes in an RGB triple and returns the
// corresponding colorfunc::Color value.  This method performs the
// inverse operation to get_RGB_values():

   Color get_color(const RGB& curr_rgb)
      {
         if (curr_rgb.nearly_equal(RGB(0,0,1)))
         {
            return blue;
         }
         else if (curr_rgb.nearly_equal(RGB(1,0,0)))
         {
            return red;
         }
         else if (curr_rgb.nearly_equal(RGB(0,1,0)))
         {
            return green;
         }
         else if (curr_rgb.nearly_equal(RGB(255.0/255.0 , 123.0/255.0, 0)))
         {
            return orange;
         }
         else if (curr_rgb.nearly_equal(RGB(244.0/255.0 , 141.0/255.0, 
                                            239.0/255.0)))
         {
            return pink;
         }
         else if (curr_rgb.nearly_equal(
            RGB(226.0/255.0 , 196.0/255.0, 104.0/255.0)))
         {
            return gold;
         }
         else if (curr_rgb.nearly_equal(RGB(0,1,1)))
         {
            return cyan;
         }
         else if (curr_rgb.nearly_equal(
            RGB(185.0/255.0 , 201.0/255.0, 62.0/255.0)))
         {
            return yegr;
         }

         else if (curr_rgb.nearly_equal(RGB(1,0,1)))
         {
            return purple;
         }
         else if (curr_rgb.nearly_equal(
            RGB(165.0/255.0 , 31.0/255.0, 8.0/255.0)))
         {
            return brick;
         }
         else if (curr_rgb.nearly_equal(
            RGB(10.0/255.0 , 173.0/255.0, 143.0/255.0)))
         {
            return blgr;
         }
         else if (curr_rgb.nearly_equal(RGB(1,1,0)))
         {
            return yellow;
         }
         else if (curr_rgb.nearly_equal(RGB(0.5 , 0.5 , 0.5)))
         {
            return grey;
         }
         else if (curr_rgb.nearly_equal(
            RGB(204.0/255.0 , 12.0/255.0, 194.0/255.0)))
         {
            return magenta;
         }
         else if (curr_rgb.nearly_equal(
            RGB(158.0/255.0 , 224.0/255.0, 132.0/255.0)))
         {
            return ltgr;
         }
         else if (curr_rgb.nearly_equal(
            RGB(239.0/255.0 , 238.0/255.0, 194.0/255.0)))
         {
            return cream;
         }
         else if (curr_rgb.nearly_equal(RGB(1,1,1)))
         {
            return white;
         }
         else if (curr_rgb.nearly_equal(RGB(0.7833, 0.0 , 0.9329)))
         {
            return brightpurple;
         }
         else if (curr_rgb.nearly_equal(RGB(0.3607,0.2,0.09019)))
         {
            return brown;
         }
         else if (curr_rgb.nearly_equal(RGB(0,0,0)))
         {
            return black;
         }
	 return null;
      }

// ---------------------------------------------------------------------
// Method rgb_to_RGB converts floating point rgb values ranging from
// [0,1] to integer RGB values ranging from [0,255]

   void rgb_to_RGB(double r,double g,double b,int& R,int& G,int& B)
      {
//         cout << "inside rgb_to_RGB()" << endl;
//         cout << "r = " << r << " g = " << g << " b = " << b << endl;
         R=static_cast<int>(r*255);
         G=static_cast<int>(g*255);
         B=static_cast<int>(b*255);
//         cout << "R = " << R << " G = " << G << " B = " << B << endl;
      }

// ---------------------------------------------------------------------
// Method RGB_to_rgb converts integer RGB values ranging from [0,255]
// to floating point rgb values ranging from [0,1]

   void RGB_to_rgb(int R,int G,int B,double& r,double& g,double& b)
      {
//         cout << "inside RGB_to_rgb()" << endl;
//         cout << "R = " << R << " G = " << G << " B = " << B << endl;
         r=double(R)/255.0;
         g=double(G)/255.0;
         b=double(B)/255.0;
//         cout << "r = " << r << " g = " << g << " b = " << b << endl;
      }

// ---------------------------------------------------------------------
// Method RGB_to_CSV() takes in an RGB triple and returns a
// comma-separated value string containing the red, green and blue
// components as integers.  We wrote this utility method in April 2010
// for postgres database insertion purposes.

   string RGB_to_CSV(const RGB& curr_rgb)
      {   
//         cout << "inside colorfunc::RGB_to_CSV()" << endl;
         int R,G,B;
         rgb_to_RGB(curr_rgb.first,curr_rgb.second,curr_rgb.third,R,G,B);
//         cout << "curr_rgb = " << curr_rgb << endl;
//         cout << "R = " << R << " G = " << G << " B = " << B << endl;

         string CSV=stringfunc::number_to_string(R)+",";
         CSV += stringfunc::number_to_string(G)+",";
         CSV += stringfunc::number_to_string(B);

//         cout << "CSV = " << CSV << endl;
         return CSV;
      }

// ---------------------------------------------------------------------
// Method CSV_to_RGB() takes in a comma-separated value string
// containing red, green and blue integer values ranging from 0 to
// 255.  It returns an RGB triple where 0 <= r,g,b <= 1.

   RGB CSV_to_RGB(string CSV)
      {   
//         cout << "inside colorfunc::CSV_to_RGB()" << endl;
         vector<double> RGB_vals=stringfunc::string_to_numbers(CSV,",");
         double r,g,b;
         RGB_to_rgb(RGB_vals[0],RGB_vals[1],RGB_vals[2],r,g,b);
         RGB curr_RGB(r,g,b);
         return curr_RGB;
      }
   
// ---------------------------------------------------------------------
// Method meta_value_to_RGB takes in a metafile colormap value and
// returns the interpolated RGB information.  This method works with
// the following "rainbow" metafile colormap:

// 0 		0	0	0	20  '0'
// 10		255	0	255	20  '10'
// 20		0	0	255	20  '20'
// 30      	0	200	200	20  '30'
// 40		150	250	0	20  '40'
// 50      	200     150     0       20  '50'
// 60		255 	0	1	20  '60'

   RGB scalar_value_to_RGB(
      double min_value,double max_value,double value,bool greyscale_flag)
      {
         double meta_value=60*(value-min_value)/(max_value-min_value);
         if (greyscale_flag)
         {
            return meta_value_to_greyscale(meta_value);
         }
         else
         {
            return meta_value_to_RGB(meta_value);
         }
      }

   RGB meta_value_to_RGB(double value)
      {
         RGB meta_RGB[7];
         meta_RGB[0]=RGB(128,0,128);
         meta_RGB[1]=RGB(255,0,255);
         meta_RGB[2]=RGB(0,0,255);
         meta_RGB[3]=RGB(0,200,200);
         meta_RGB[4]=RGB(150,250,0);
         meta_RGB[5]=RGB(200,150,0);
         meta_RGB[6]=RGB(255,0,0);

         int floor=basic_math::mytruncate(0.1*value);
         double remainder=value-10*floor;
         double frac=0.1*remainder;
//         cout << "floor = " << floor << " remainder = " << remainder
//              << " frac = " << frac << endl;

         if (floor < 0)
         {
            return RGB(0.5,0,0.5);
         }
         else if (floor >= 6)
         {
            return RGB(1,0,0);
         }
         else 
         {
            RGB meta_RGB_interp=meta_RGB[floor]+
               frac*(meta_RGB[floor+1]-meta_RGB[floor]);
            return meta_RGB_interp/255.0;
         }
      }

// ---------------------------------------------------------------------
// Method meta_value_to_greyscale returns an RGB with a grey coloring
// based upon the input scalar value.

   RGB meta_value_to_greyscale(double value)
      {
         const double min_grey=0.2;
         const double max_grey=1.0;

         vector<RGB> greyscale;
         greyscale.push_back(RGB(min_grey,min_grey,min_grey));
         greyscale.push_back(RGB(max_grey,max_grey,max_grey));

         int floor=basic_math::mytruncate(0.1*value);
         double remainder=value-10*floor;
         double frac=0.1*remainder;
         
         if (floor < 0)
         {
            return greyscale[0];
         }
         else if (floor >= 6)
         {
            return greyscale[1];
         }
         else 
         {
            return greyscale[0]+frac*(greyscale[1]-greyscale[0]);
         }
      }
   
// ---------------------------------------------------------------------
// Method RGB_to_bytes converts RGB double values into byte values.
// Boolean parameter normalized_input_RGB_values should equal true if
// the entries in curr_rgb range from 0 to 1.

   RGB_bytes RGB_to_bytes(RGB curr_rgb,bool normalized_input_RGB_values)
      {
//         cout << "inside colorfunc::RGB_to_bytes()" << endl;
         if (normalized_input_RGB_values)
         {
            curr_rgb.first *= 255;
            curr_rgb.second *= 255;
            curr_rgb.third *= 255;
         }

//         cout << "curr_rgb.first = " << basic_math::round(curr_rgb.first)
//              << " curr_rgb.second = " << basic_math::round(curr_rgb.second)
//              << " curr_rgb.third = " << basic_math::round(curr_rgb.third)
//              << endl;
         
         unsigned char red_byte=
            stringfunc::ascii_integer_to_unsigned_char(
               basic_math::round(curr_rgb.first));
         unsigned char green_byte=
            stringfunc::ascii_integer_to_unsigned_char(
               basic_math::round(curr_rgb.second));
         unsigned char blue_byte=
            stringfunc::ascii_integer_to_unsigned_char(
               basic_math::round(curr_rgb.third));
         return RGB_bytes(red_byte,green_byte,blue_byte);
      }

   RGBA_bytes RGBA_to_bytes(RGBA curr_rgba,bool normalized_input_RGBA_values)
      {
//         cout << "inside colorfunc::RGBA_to_bytes()" << endl;
//         cout << "normalized_input_RGBA_values = "
//              << normalized_input_RGBA_values << endl;
         
         if (normalized_input_RGBA_values)
         {
            curr_rgba.first *= 255;
            curr_rgba.second *= 255;
            curr_rgba.third *= 255;
            curr_rgba.fourth *= 255;
         }

//         cout << "curr_rgba = " 
//              << curr_rgba.first << " "
//              << curr_rgba.second << " "
//              << curr_rgba.third << " "
//              << curr_rgba.fourth << endl;

         unsigned char red_byte=
            stringfunc::ascii_integer_to_unsigned_char(
               basic_math::round(curr_rgba.first));
         unsigned char green_byte=
            stringfunc::ascii_integer_to_unsigned_char(
               basic_math::round(curr_rgba.second));
         unsigned char blue_byte=
            stringfunc::ascii_integer_to_unsigned_char(
               basic_math::round(curr_rgba.third));
         unsigned char alpha_byte=
            stringfunc::ascii_integer_to_unsigned_char(
               basic_math::round(curr_rgba.fourth));
         return RGBA_bytes(red_byte,green_byte,blue_byte,alpha_byte);
      }

   RGB_bytes RGB_to_bytes(double r,double g,double b,
                          bool normalized_input_RGB_values)
      {
//         cout << "inside colorfunc::RGB_to_bytes" << endl;
         
         if (normalized_input_RGB_values)
         {
            r *= 255;
            g *= 255;
            b *= 255;
         }

//         cout << "r = " << r << " g = " << g << " b = " << b << endl;
         unsigned char red_byte=
            stringfunc::ascii_integer_to_unsigned_char(basic_math::round(r));
         unsigned char green_byte=
            stringfunc::ascii_integer_to_unsigned_char(basic_math::round(g));
         unsigned char blue_byte=
            stringfunc::ascii_integer_to_unsigned_char(basic_math::round(b));
         return RGB_bytes(red_byte,green_byte,blue_byte);
      }
   
// ---------------------------------------------------------------------
// Method bytes_to_RGB takes in a pointer to a char* array which is
// assumed to contain a string of RGB bytes.  This method returns an
// RGB structure containing unrenormalized red, green and blue values
// corresponding to the RGB byte triple at the current char* location.

   RGB bytes_to_RGB(char* RGB_byte)
      {
         RGB curr_RGB;
         curr_RGB.first=stringfunc::unsigned_char_to_ascii_integer(
            *RGB_byte);
         curr_RGB.second=stringfunc::unsigned_char_to_ascii_integer(
            *(RGB_byte+1));  
         curr_RGB.third=stringfunc::unsigned_char_to_ascii_integer(
            *(RGB_byte+2));
         return curr_RGB;
      }

// ---------------------------------------------------------------------
// Method bytes_to_RGBA takes in an osg::vec4ub which contains color
// information as bytes.  This method returns an RGBA structure
// containing renormalized red, green, blue and alpha values
// corresponding to the RGBA byte quadruple.

   RGBA bytes_to_RGBA(const osg::Vec4ub& rgba_bytes)
      {
//         cout << "inside colorfunc::bytes_to_RGBA()" << endl;

//         cout << "unsigned_char_to_ascii_int(rgba_bytes.r()) = "
//              << stringfunc::unsigned_char_to_ascii_integer(
//                 rgba_bytes.r()) << endl;
//         cout << "unsigned_char_to_ascii_int(rgba_bytes.g()) = "
//              << stringfunc::unsigned_char_to_ascii_integer(
//                 rgba_bytes.g()) << endl;
//         cout << "unsigned_char_to_ascii_int(rgba_bytes.b()) = "
//              << stringfunc::unsigned_char_to_ascii_integer(
//                 rgba_bytes.b()) << endl;

         RGBA curr_RGBA;

         curr_RGBA.first=stringfunc::unsigned_char_to_ascii_integer(
            rgba_bytes.r())/255.0;
         curr_RGBA.second=stringfunc::unsigned_char_to_ascii_integer(
            rgba_bytes.g())/255.0;
         curr_RGBA.third=stringfunc::unsigned_char_to_ascii_integer(
            rgba_bytes.b())/255.0;
         curr_RGBA.fourth=stringfunc::unsigned_char_to_ascii_integer(
            rgba_bytes.a())/255.0;
         return curr_RGBA;
      }

// ---------------------------------------------------------------------
// Method exponential_distributed_S() returns a saturation distributed
// according to an exponential with decay parameter -|lambda|.  Note
// that lambda passed into this method should be positive.

   double exponential_distributed_S(double lambda)
   {
      double S = nrfunc::expdev(lambda);  
      S = basic_math::max(0.0, S);
      S = basic_math::min(1.0, S);
      return S;
   }

// ---------------------------------------------------------------------
// Method gaussian_distributed_V() returns a value distributed
// according to a gaussian with mean mu and standard deviation sigma.

   double gaussian_distributed_V(double mu, double sigma)
   {
      double V = mu + sigma * nrfunc::gasdev();  
      V = basic_math::max(0.0, V);
      V = basic_math::min(1.0, V);
      return V;
   }

// ---------------------------------------------------------------------
// Method generate_random_HSV()

   HSV generate_random_HSV()
   {
      return generate_random_HSV(0, 360, 0, 1, 0, 1);
   }

   HSV generate_random_HSV(
      double h_min, double h_max, double s_min, double s_max, 
      double v_min, double v_max)
   {
      double h = h_min + (h_max - h_min) * nrfunc::ran1();
      double s = s_min + (s_max - s_min) * nrfunc::ran1();
      double v = v_min + (v_max - v_min) * nrfunc::ran1();

      HSV curr_HSV;
      curr_HSV.first = h;
      curr_HSV.second = s;
      curr_HSV.third = v;
      return curr_HSV;
   }
   
// ---------------------------------------------------------------------
// Method generate_random_RGB()

   RGB generate_random_RGB(bool normalized_RGB_values)   
   {
      return generate_random_RGB(normalized_RGB_values, 0, 360, 0, 1, 0, 1);
   }
   
   RGB generate_random_RGB(
      bool normalized_RGB_values, 
      double h_min, double h_max, double s_min, double s_max,
      double v_min, double v_max)
   {
      HSV curr_HSV = generate_random_HSV(
         h_min, h_max, s_min, s_max, v_min, v_max);

      double r, g, b;
      hsv_to_RGB(curr_HSV.first, curr_HSV.second, curr_HSV.third, 
                            r, g, b);
      if(!normalized_RGB_values)
      {
         r *= 255;
         g *= 255;
         b *= 255;
      }

      RGB curr_RGB;
      curr_RGB.first = r;
      curr_RGB.second= g;
      curr_RGB.third = b;

//      curr_RGB.first=255*nrfunc::ran1();
//      curr_RGB.second=255*nrfunc::ran1();
//      curr_RGB.third=255*nrfunc::ran1();
      return curr_RGB;
   }

// ---------------------------------------------------------------------
// This overloaded version of generate_random_RGB() selects a hue
// uniformly across interval [0,360], a saturation distributed
// according to an exponential distribution and a value distributed
// according to a gaussian:
   
   RGB generate_random_RGB(
      bool normalized_RGB_values, double lambda_s,
      double mu_v, double sigma_v)
   {
      double h = 360 * nrfunc::ran1();
      double s = exponential_distributed_S(lambda_s);
      double v = gaussian_distributed_V(mu_v, sigma_v);
   
      double r, g, b;
      hsv_to_RGB(h, s, v, r, g, b);
      if(!normalized_RGB_values)
      {
         r *= 255;
         g *= 255;
         b *= 255;
      }

      RGB curr_RGB;
      curr_RGB.first = r;
      curr_RGB.second= g;
      curr_RGB.third = b;
      return curr_RGB;
   }

// ---------------------------------------------------------------------
   int fluctuate_value(int R)
   {
      const double sigma = 0.02 * 256;
      return fluctuate_value(R, sigma);
   }
   
   int fluctuate_value(int R, double sigma)
   {
      int R_fluctuation=sigma*nrfunc::gasdev();
      R += R_fluctuation;

      R = basic_math::max(0,R);
      R = basic_math::min(255,R);
      return R;
   }

// ==========================================================================
// RGB - HSV color coordinate transformation methods
// ==========================================================================

// Method RGB_to_hs can take in RGB values which need not lie within
// the interval [0,1].  Since hue and saturation depend only upon RGB
// value ratios, their absolute normalizations do not matter.
   
   void RGB_to_hs(double r,double g,double b,double& h,double& s)
      {

// First make sure R, G and B lie within allowed intervals:

         r=basic_math::max(0.0,r);
         g=basic_math::max(0.0,g);
         b=basic_math::max(0.0,b);
         
         double max_value=basic_math::max(r,g,b);
         double min_value=basic_math::min(r,g,b);
         double delta=max_value-min_value;
         
         if (nearly_equal(max_value,0))
         {
            s=0;
         }
         else
         {
            s=delta/max_value;
         }

         if (s==0)
         {
            h=NEGATIVEINFINITY;
         }
         else
         {
            if (r==max_value)
            {
               h=(g-b)/delta;
            }
            else if (g==max_value)
            {
               h=2+(b-r)/delta;
            }
            else if (b==max_value)
            {
               h=4+(r-g)/delta;
            }
            h *= 60;
            if (h < 0) h += 360;
         }
      }

// ---------------------------------------------------------------------
// Methods RGB_to_hsv and hsv_to_RGB convert back and forth between
// the RGB and HSV color coordinate systems.  R, G and B values are
// taken to be doubles in the range [0,1].  Hue H is an angle
// ranging over [0 degs,360 degs).  Pure red, yellow, green, cyan,
// blue and magneta correspond to hue=0, 60, 120, 180, 240 and 300
// degrees.  Saturation S is a radial coordinate ranging over [0,1].
// S=1 corresponds to pure hues, while S=0 corresponds to pure grey.
// Value V ranges over [0,1].  It is partially correlated with
// brightness.  V=0 corresponds to pure black, while V=1 yields the
// brightest hues possible.
   
   void RGB_to_hsv(double r,double g,double b,double& h,double& s,double& v)
      {

// First make sure R, G and B lie within allowed intervals:

         if (r > 1)
         {
//            cout << "Error in colorfunc::RGB_to_hsv" << endl;
//            cout << "r = " << r << endl;
            r=1;
         }
         else if (r < 0)
         {
//            cout << "Error in colorfunc::RGB_to_hsv" << endl;
//            cout << "r = " << r << endl;
            r=0;
         }

         if (g > 1)
         {
//            cout << "Error in colorfunc::RGB_to_hsv" << endl;
//            cout << "g = " << g << endl;
            g=1;
         }
         else if (g < 0)
         {
//            cout << "Error in colorfunc::RGB_to_hsv" << endl;
//            cout << "g = " << g << endl;
            g=0;
         }

         if (b > 1)
         {
//            cout << "Error in colorfunc::RGB_to_hsv" << endl;
//            cout << "b = " << b << endl;
            b=1;
         }
         else if (b < 0)
         {
//            cout << "Error in colorfunc::RGB_to_hsv" << endl;
//            cout << "b = " << b << endl;
            b=0;
         }
         
         double max_value=basic_math::max(r,g,b);
         double min_value=basic_math::min(r,g,b);
         v=max_value;
         if (max_value != 0)
         {
            s=(max_value-min_value)/max_value;
         }
         else
         {
            s=0;
         }

         h = -1;
         if (s==0)
         {
            h=NEGATIVEINFINITY;
         }
         else
         {
            double delta=max_value-min_value;
            if (r==max_value)
            {
               h=(g-b)/delta;
            }
            else if (g==max_value)
            {
               h=2+(b-r)/delta;
            }
            else if (b==max_value)
            {
               h=4+(r-g)/delta;
            }
            h *= 60;
            if (h < 0) h += 360;
         }
      }

   void hsv_to_RGB(double h,double s,double v,double& r,double& g,double& b)
   {

// First make sure h, s and v lie within allowed intervals:

         h=basic_math::phase_to_canonical_interval(h,0,360);

         if (nearly_equal(s,1)) s=1;
         
         if (s > 1)
         {
            cout << "Error in colorfunc::hsv_to_RGB" << endl;
            cout << "s = " << s << endl;
            s=1;
         }
         else if (s < 0)
         {
            cout << "Error in colorfunc::hsv_to_RGB" << endl;
            cout << "s = " << s << endl;
            s=0;
         }

         if (v > 1)
         {
            cout << "Error in colorfunc::hsv_to_RGB" << endl;
            cout << "v = " << v << endl;
            v=1;
         }
         else if (v < 0)
         {
            cout << "Error in colorfunc::hsv_to_RGB" << endl;
            cout << "v = " << v << endl;
            v=0;
         }

         if (s==0)
         {
            r=g=b=v;
         }
         else
         {
            if (h==360) h=0;
            h /= 60.0;
            int i=basic_math::mytruncate(h);
            double frac=h-i;
            double p=v*(1-s);
            double q=v*(1-s*frac);
            double t=v*(1-s*(1-frac));
            switch (i)
            {
               case 0:
                  r=v;
                  g=t;
                  b=p;
                  break;
               case 1:
                  r=q;
                  g=v;
                  b=p;
                  break;
               case 2:
                  r=p;
                  g=v;
                  b=t;
                  break;
               case 3:
                  r=p;
                  g=q;
                  b=v;
                  break;
               case 4:
                  r=t;
                  g=p;
                  b=v;
                  break;
               case 5:
                  r=v;
                  g=p;
                  b=q;
                  break;
            } // switch statement
         }
      }

   void hsv_to_RGBA(double h,double s,double v,double& r,double& g,double& b,
                    double& a)
   {
      hsv_to_RGB(h,s,v,r,g,b);
      a = 1;
   }

   HSV RGB_to_hsv(const RGB& curr_rgb,bool normalized_RGB_values)
      {
         double r=curr_rgb.first;
         double g=curr_rgb.second;
         double b=curr_rgb.third;
         if (!normalized_RGB_values)
         {
            r /= 255.0;
            g /= 255.0;
            b /= 255.0;
         }

         double h,s,v;
         RGB_to_hsv(r,g,b,h,s,v);
         return HSV(h,s,v);
      }
   
   RGB hsv_to_RGB(const HSV& curr_hsv,bool normalized_RGB_values)
      {
         double h=curr_hsv.first;
         double s=curr_hsv.second;
         double v=curr_hsv.third;

         double r,g,b;
         hsv_to_RGB(h,s,v,r,g,b);

         if (!normalized_RGB_values)
         {
            r *= 255;
            g *= 255;
            b *= 255;
         }
         return RGB(r,g,b);
      }

   RGBA hsv_to_RGBA(const HSV& curr_hsv,bool normalized_RGB_values)
   {
      RGB curr_RGB = hsv_to_RGB(curr_hsv, normalized_RGB_values);
      double a = 1;
      if(!normalized_RGB_values)
      {
         a = 255;
      }
      
      return RGBA(curr_RGB.first, curr_RGB.second, curr_RGB.third, a);
   }

// ==========================================================================
// 16-bit "565" methods
// ==========================================================================

// Method RGB_to_565_string() takes in R,G,B values ranging from [0,255].  It
// returns a corresponding 16-bit binary string which corresponds as
// closely as possible to the input 24-bit RGB color.

   string RGB_to_565_string(int R, int G, int B)
   {
//      cout << "colorfunc::inside rgb_to_565() " << endl;
//      cout << "R = " <<  R << " G = " << G << " B = " <<  B << endl;

      double r=R/255.0;
      double g=G/255.0;
      double b=B/255.0;
      
      int r5=basic_math::round(r*31);
      int g6=basic_math::round(g*63);
      int b5=basic_math::round(b*31);
//      cout << "r5 = " << r5 << " g6 = " << g6 << " b5 = " << b5 << endl;
      
      unsigned char rchar = stringfunc::ascii_integer_to_char(r5);
      unsigned char gchar = stringfunc::ascii_integer_to_char(g6);
      unsigned char bchar = stringfunc::ascii_integer_to_char(b5);

      string r_bits=stringfunc::byte_bits_rep(rchar,5);
//      cout << "R bits:" << endl;
//      cout << r_bits << endl;

      string g_bits=stringfunc::byte_bits_rep(gchar,6);
//      cout << "G bits:" << endl;
//      cout << g_bits << endl;

      string b_bits=stringfunc::byte_bits_rep(bchar,5);
//      cout << "B bits:" << endl;
//      cout << b_bits << endl;
      
      string binary_str = r_bits + g_bits + b_bits;
//      cout << "binary_str = " << binary_str << endl;
      return binary_str;
   }

   unsigned short RGB_to_565_short(int R,int G,int B)
   {
      string binary_str=RGB_to_565_string(R,G,B);
      unsigned short short_color=bitset<64>(binary_str).to_ulong();
      return short_color;
   }

// ==========================================================================
// Luminosity methods
// ==========================================================================

// Method RBB_to_luminosity() returns a particular linear combination
// of the input (integer or real) R,G,B values which corresponds to
// "luminosity":

   double RGB_to_luminosity(double R,double G,double B)
   {
      return 0.2126*R+0.7152*G+0.0722*B;
   }

// ==========================================================================
// RGB colormap methods
// ==========================================================================

// Method rgb_colormap_value takes in RGB fractions ranging from 0 to
// 1.  It converts these 3 fractions into a single fraction ranging
// from 0 to 1 which is correlated with the RGB colormap that we
// implemented within the Group 94/106 dataviewer in Jan 2005.  If the
// p-value of an XYZP point is set equal to the value returned by this
// method and if it is viewed using the RGB colormap, then its
// appearance should reasonably coincide with the rgb values which are
// input into this method...

   double rgb_colormap_value(const RGB& curr_rgb)
      {
         return rgb_colormap_value(curr_rgb.first,curr_rgb.second,
                                   curr_rgb.third);
      }

   double rgb_colormap_value(const Color color)
      {
         return rgb_colormap_value(get_RGB_values(color));
      }

   double rgb_colormap_value(double r,double g,double b)
      {
         const int nbins=50;
         const double delta=1.0/double(nbins-1);

// For reasons which we do not understand as of Jan 05, the RGB
// colormap within the dataviewer has some problems near some of the
// boundary faces of the unit color cube.  We therefore limit the
// input rgb fractions so that they do not get too near any boundary
// face:

         const double min_color_value=0.001;
         const double max_color_value=0.999;
//         const double min_color_value=0.05;
//         const double max_color_value=0.925;
         r=basic_math::max(min_color_value,r);
         g=basic_math::max(min_color_value,g);
         b=basic_math::max(min_color_value,b);
         r=basic_math::min(max_color_value,r);
         g=basic_math::min(max_color_value,g);
         b=basic_math::min(max_color_value,b);

         int Nr=basic_math::round(r/delta);
         int Ng=basic_math::round(g/delta);
         int Nb=basic_math::round(b/delta);
         double color_value=double(Nr*sqr(nbins)+Ng*nbins+Nb)/
            double(nbins*nbins*nbins-1);
         return color_value;
      }
   
// ---------------------------------------------------------------------
// Query_rgb_colormap_value 

   void query_rgb_colormap_value()
      {
         double r,g,b;
         cout << "Enter r fraction:" << endl;
         cin >> r;
         cout << "Enter g fraction:" << endl;
         cin >> g;
         cout << "Enter b fraction:" << endl;
         cin >> b;
         cout << "RGB colormap value = " << rgb_colormap_value(r,g,b)
              << endl;
      }

// ==========================================================================
// Hue-value colormap methods
// ==========================================================================

// Method hue_value_to_RGB takes in fraction f ranging from 0 to 1.
// It returns the RGB values corresponding to our "Hue plus value"
// colormap within the group 94/106 dataviewer.  This method was
// written in order to fuse ALIRT feature and height information with
// actual color photographs in a single XYZP file using our RGB
// dataviewer colormap.

   void initialize_hue_value_colormap()
      {
         if (dataviewer_colormap==NULL) 
            dataviewer_colormap=colormapfunc::generate_hue_value_colormap(L);
      }

   void initialize_big_hue_value_colormap()
      {
//         if (dataviewer_colormap==NULL) 
//            dataviewer_colormap=
//               colormapfunc::generate_big_hue_value_colormap(L);
      }

   void delete_dataviewer_colormap()
      {
         delete dataviewer_colormap;
         dataviewer_colormap=NULL;
      }

   RGB dataviewer_colormap_to_RGB(double f)
      {
         int bin_lo=basic_math::mytruncate(f*(L/3));
         double bin_frac=f*(L/3);
         if (bin_lo >= L/3-1)
         {
            bin_lo=L/3-2;
         }
         int bin_hi=bin_lo+1;

         double r_lo=dataviewer_colormap[3*bin_lo+0];
         double g_lo=dataviewer_colormap[3*bin_lo+1];
         double b_lo=dataviewer_colormap[3*bin_lo+2];
         double r_hi=dataviewer_colormap[3*bin_hi+0];
         double g_hi=dataviewer_colormap[3*bin_hi+1];
         double b_hi=dataviewer_colormap[3*bin_hi+2];
         
         double r=r_lo+(r_hi-r_lo)*(bin_frac-bin_lo)/(bin_hi-bin_lo);
         double g=g_lo+(g_hi-g_lo)*(bin_frac-bin_lo)/(bin_hi-bin_lo);
         double b=b_lo+(b_hi-b_lo)*(bin_frac-bin_lo)/(bin_hi-bin_lo);

//         cout << "r_lo = " << r_lo << " r_hi = " << r_hi << endl;
//         cout << "g_lo = " << g_lo << " g_hi = " << g_hi << endl;
//         cout << "b_lo = " << b_lo << " b_hi = " << b_hi << endl;
//         cout << "r = " << r << " g = " << g << " b = " << b << endl;
         
         return RGB(r,g,b);
      }

// ---------------------------------------------------------------------
// Method generate_distinct_colors takes outputs an STL vector
// containing n_output_colors evenly sampled from our hue_value
// colormap.  n_output_colors should be larger than 10 but less than 150.

   vector<RGB> generate_distinct_colors(
      int n_output_colors,int n_preceding_colors)
      {
         vector<RGB> output_colors;

         if (n_output_colors+n_preceding_colors < 10)
         {
            for (int n=n_preceding_colors; 
                 n<n_output_colors+n_preceding_colors; n++)
            {
               output_colors.push_back(
                  get_RGB_values(get_color(n)));
            }
         }
         else
         {
            initialize_hue_value_colormap();

            double f_start=0;
            if (n_preceding_colors > 0)
            {
               f_start=0.1*nrfunc::ran1();
            }

            double f_stop=1.0;
            double df=(f_stop-f_start)/double(n_output_colors);


            for (int n=0; n<n_output_colors; n++)
            {
               double f=f_start+n*df;
               output_colors.push_back(dataviewer_colormap_to_RGB(f));
            }
         }
         
         return output_colors;
      }

// ==========================================================================
// Hexadecimal output methods
// ==========================================================================

// Method RGB_to_RRGGBB_hex takes in r,g,b color values ranging
// between 0 and 1.  It first converts each color double into the
// integer interval [0,255].  It then converts each integer to its
// hexadecimal representation.  The concatenated hexadecimal version
// is returned.

   string RGB_to_RRGGBB_hex(double r,double g,double b)
      {
         int R=r*255;
         R=basic_math::max(0,R);
         R=basic_math::min(255,R);
         string R_hex=stringfunc::integer_to_hexadecimal(R,2);

         int G=g*255;
         G=basic_math::max(0,G);
         G=basic_math::min(255,G);
         string G_hex=stringfunc::integer_to_hexadecimal(G,2);

         int B=b*255;
         B=basic_math::max(0,B);
         B=basic_math::min(255,B);
         string B_hex=stringfunc::integer_to_hexadecimal(B,2);

//         cout << "R_hex = " << R_hex 
//              << " G_hex = " << G_hex
//              << " B_hex = " << B_hex << endl;

         string RRGGBB_hex=R_hex+G_hex+B_hex;
//         cout << "RRGGBB_hex = " << RRGGBB_hex << endl;
         return RRGGBB_hex;
      }

   string RGB_to_RRGGBB_hex(const RGB& curr_rgb)
   {
      return RGB_to_RRGGBB_hex(curr_rgb.first,curr_rgb.second,curr_rgb.third);
   }

   string RGBA_to_RRGGBBAA_hex(double r,double g,double b,double a)
   {
      string RGB_hex = RGB_to_RRGGBB_hex(r, g, b);

      int A=a*255;
      A=basic_math::max(0,A);
      A=basic_math::min(255,A);
      string A_hex=stringfunc::integer_to_hexadecimal(A,2);
      string RRGGBBAA_hex = RGB_hex + A_hex;
      return RRGGBBAA_hex;
   }
   
// ---------------------------------------------------------------------
// Method RGBA_to_AABBGGRR_hex takes in r,g,b,a color values ranging
// between 0 and 1.  It first converts each color double into the
// integer interval [0,255].  It then converts each integer to its
// hexadecimal representation.  The concatenated version is returned
// in a format which is suitable for KML file generation.

   string RGBA_to_AABBGGRR_hex(double r,double g,double b,double a)
   {
      int R=r*255;
      R=basic_math::max(0,R);
      R=basic_math::min(255,R);
      string R_hex=stringfunc::integer_to_hexadecimal(R,2);

      int G=g*255;
      G=basic_math::max(0,G);
      G=basic_math::min(255,G);
      string G_hex=stringfunc::integer_to_hexadecimal(G,2);

      int B=b*255;
      B=basic_math::max(0,B);
      B=basic_math::min(255,B);
      string B_hex=stringfunc::integer_to_hexadecimal(B,2);

      int A=a*255;
      A=basic_math::max(0,A);
      A=basic_math::min(255,A);
      string A_hex=stringfunc::integer_to_hexadecimal(A,2);

//         cout << "R_hex = " << R_hex 
//              << " G_hex = " << G_hex
//              << " B_hex = " << B_hex
//              << " A_hex = " << A_hex << endl;

      string AABBGGRR_hex=A_hex+B_hex+G_hex+R_hex;
//         cout << "AABBGGRR_hex = " << AABBGGRR_hex << endl;
      return AABBGGRR_hex;
   }

// ---------------------------------------------------------------------
// Method RRGGBB_hex_to_rgb() takes in a hexadecimal string in the
// form RRGGBB.  It returns the corresponding red, green and blue
// values as fractions ranging from 0 to 1.

   void RRGGBB_hex_to_rgb(string hex_str,double& r,double& g,double& b)
      {
         string RR=hex_str.substr(0,2);
         string GG=hex_str.substr(2,2);
         string BB=hex_str.substr(4,2);
//         cout << "RR = " << RR << " GG = " << GG << " BB = " << BB << endl;
         int Rint=stringfunc::hexadecimal_to_integer(RR);
         int Gint=stringfunc::hexadecimal_to_integer(GG);
         int Bint=stringfunc::hexadecimal_to_integer(BB);
//         cout << "Rint = " << Rint << " Gint = " << Gint
//              << " Bint = " << Bint << endl;
         r=double(Rint)/255.0;
         g=double(Gint)/255.0;
         b=double(Bint)/255.0;
//         cout << "r = " << r << " g = " << g << " b = " << b << endl;
      }

   void RRGGBB_hex_to_RGB(string hex_str,RGB& curr_rgb)
      {
         double r,g,b;
         RRGGBB_hex_to_rgb(hex_str,r,g,b);
         curr_rgb.first=r;
         curr_rgb.second=g;
         curr_rgb.third=b;
      }

// Method RRGGBBAA_hex_to_rgba() takes in hex string RRGGBBAA where
// RR, GG, BB and AA range from 00 to FF=255.  It returns decimal
// fractions r, g, b and a corresponding to the hex values.

   void RRGGBBAA_hex_to_rgba(string hex_str,double& r,double& g,double& b,
      double& a)
      {
         string RR=hex_str.substr(0,2);
         string GG=hex_str.substr(2,2);
         string BB=hex_str.substr(4,2);
         string AA=hex_str.substr(6,2);
//         cout << "RR = " << RR << " GG = " << GG << " BB = " << BB 
//              << " AA = " << AA << endl;
         int Rint=stringfunc::hexadecimal_to_integer(RR);
         int Gint=stringfunc::hexadecimal_to_integer(GG);
         int Bint=stringfunc::hexadecimal_to_integer(BB);
         int Aint=stringfunc::hexadecimal_to_integer(AA);
//         cout << "Rint = " << Rint << " Gint = " << Gint
//              << " Bint = " << Bint << " Aint = " << Aint << endl;
         r=double(Rint)/255.0;
         g=double(Gint)/255.0;
         b=double(Bint)/255.0;
         a=double(Aint)/255.0;
//         cout << "r = " << r << " g = " << g << " b = " << b 
//              << " a = " << a <<  endl;
      }

// ==========================================================================
// Color quantization methods
// ==========================================================================

   vector<string> get_quantized_color_labels()
   {
      vector<string> color_labels;
      color_labels.push_back("red");		// 0
      color_labels.push_back("orange");		// 1
      color_labels.push_back("yellow");		// 2
      color_labels.push_back("green");		// 3
      color_labels.push_back("blue");		// 4
      color_labels.push_back("purple");		// 5
      color_labels.push_back("black");		// 6
      color_labels.push_back("white");		// 7
      color_labels.push_back("grey");		// 8
      color_labels.push_back("brown");		// 9
      return color_labels;
   }

// ---------------------------------------------------------------------      
   unsigned int get_n_quantized_colors()
   {
      vector<string> color_labels=get_quantized_color_labels();
      return color_labels.size();
   }
   
// ---------------------------------------------------------------------      
// Method assign_hsv_to_color_histogram_bin() takes in hue, saturation
// and value color coordinates.  On 3/7/12 and 3/8/12, we empirically
// and subjectively assigned color names to particular color bins.
// This method returns the color histogram bin number as well as
// quantized hue, saturation and value coordinates.

      int assign_hsv_to_color_histogram_bin(double h,double s,double v)
      {
         double h_quantized,s_quantized,v_quantized;
         return assign_hsv_to_color_histogram_bin(
            h,s,v,h_quantized,s_quantized,v_quantized);
      }

// ---------------------------------------------------------------------      
      int assign_hsv_to_color_histogram_bin(
         double h,double s,double v,
         double& h_quantized,double& s_quantized,double& v_quantized)
      {
//         cout << "inside videofunc::assign_hsv_to_histogram_bin()" << endl;
	int color_histogram_bin_number=-1;

         if (v < 0.1)
         {
            color_histogram_bin_number=6;	// black
            v_quantized=0;
         }
         else if ( 
            ( (h >= 285 || h < 15) && s < 0.08) || // red
            (h >= 15 && h < 30 && s < 0.12) || // orange
            (h >= 180 && h < 285 && s < 0.13) || // blue
            (h >= 30 && h <= 180 && s < 0.16) ) // yellow-cyan
         {
            if (v > 0.85)
            {
               color_histogram_bin_number=7;	// white
               v_quantized=1;
            }
            else
            {
               color_histogram_bin_number=8;	// grey
               v_quantized=0.5;
            }
            s_quantized=0;
         }
         else if (h >=15 && h < 45 && v < 0.66)
         {
            h_quantized=30;
            s_quantized=1;
            v_quantized=0.5;
            color_histogram_bin_number=9;	// brown
         }
         else
         {
            if (h < 15)
            {
               h_quantized=0;
               color_histogram_bin_number=0;	// red
            }
            else if (h >=15 && h < 45 && v >= 0.66)
            {
               h_quantized=30;
               color_histogram_bin_number=1;	// orange
            }
            else if (h >=45 && h < 65)
            {
               h_quantized=60;
               color_histogram_bin_number=2;	// yellow
            }
            else if (h >=65 && h < 170)
            {
               h_quantized=120;
               color_histogram_bin_number=3;	// green
            }
            else if (h >=170 && h < 260)
            {
               h_quantized=240;
               color_histogram_bin_number=4;	// blue
            }
            else if (h >=260 && h < 315)
            {
               h_quantized=300;
               color_histogram_bin_number=5;	// purple
            }
            else if (h >= 315)
            {
               h_quantized=0;
               color_histogram_bin_number=0;	// red
            }
            v_quantized=1;
            s_quantized=1;
         }
         return color_histogram_bin_number;
      }

// ---------------------------------------------------------------------
// Boolean method color_match() returns true if the Euclidean
// distance between (R1,G1,B1) and (R2,G2,B2) is less than threshold.

   bool color_match(
      int R1,int G1,int B1,int R2,int G2,int B2,double threshold)
   {
      double sqrd_delta=sqr(R1-R2)+sqr(G1-G2)+sqr(B1-B2);
      return (sqrd_delta < sqr(threshold));
   }

   double color_distance(
      colorfunc::RGB& RGB_1,colorfunc::RGB& RGB_2)
   {
      double sqrd_delta=sqr(RGB_1.first-RGB_2.first)+
         sqr(RGB_1.second-RGB_2.second)+
         sqr(RGB_1.third-RGB_2.third);
      return sqrt(sqrd_delta);
   }

// ==========================================================================
// YCbCr methods
// ==========================================================================

// Method RGB_to_YCbCr()

   void RGB_to_YCbCr(double R, double G, double B, 
                     double& Y, double& Cb, double& Cr)
   {
      Y = 0 + 0.299 * R +  0.587 * G + 0.114 * B;
      Cb = 128 - 0.168736 * R - 0.331264 * G + 0.5 * B;
      Cr = 128 + 0.5 * R - 0.418688 * G - 0.081312 * B;
   }
   
   void YCbCr_to_RGB(double Y, double Cb, double Cr, 
                     double& R, double& G, double& B)
   {
      R = Y + 1.402 * (Cr - 128);
      G = Y - 0.34414 * (Cb - 128) - 0.71414 * (Cr - 128);
      B = Y + 1.772 * (Cb - 128);
   }
   
// ==========================================================================
// TOC specific methods
// ==========================================================================

// Method get_platform_color() hardwires particular colors to
// particular platforms in the 2010 Tech Challenge.

   Color get_platform_color(int platform_ID)
   {
      Color platform_color=grey;

      if (platform_ID==1)	// car
      {
         platform_color=colorfunc::pink;
      }
      else if (platform_ID==2)	// bike
      {
         platform_color=colorfunc::orange;
      }
      else if (platform_ID==3)	// helicopter
      {
         platform_color=colorfunc::yellow;
      }
      else if (platform_ID==8)	// sail plane
      {
         platform_color=colorfunc::white;
      }
      else if (platform_ID==9)	// human
      {
         platform_color=colorfunc::green;
      }
      else if (platform_ID==10) // quad rotor
      {
         platform_color=colorfunc::cyan;
      }
      else if (platform_ID==11) // mast
      {
         platform_color=colorfunc::purple;
      }
      else if (platform_ID==12) // blue plane
      {
         platform_color=colorfunc::blue;
      }
      else if (platform_ID==13) // red robot
      {
         platform_color=colorfunc::red;
      }
      else if (platform_ID==14) // RC car
      {
         platform_color=colorfunc::brick;
      }
      else if (platform_ID==15) // dog
      {
         platform_color=colorfunc::cream;
      }
      return platform_color;
   }
   
} // colorfunc namespace


