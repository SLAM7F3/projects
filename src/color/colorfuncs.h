// Brown:  R = 0.3607  G = 0.2 ; B = 0.09019

// =========================================================================
// Header file for stand-alone color functions.
// =========================================================================
// Last modified on 2/1/16; 3/7/16; 3/9/16; 3/25/16
// =========================================================================

#ifndef COLORFUNCS_H
#define COLORFUNCS_H

#include <osg/Vec4>
#include <osg/Vec4ub>
#include <string>
#include <vector>
#include "datastructures/Quadruple.h"
#include "datastructures/Triple.h"

namespace colorfunc
{
   enum Color
   {
      blue,red,green,orange,pink,
      gold,cyan,yegr,purple,brick,
      blgr,yellow,grey,magenta,ltgr,
      cream,white,brightyellow,brightpurple,brightcyan,
      darkpurple,brown,black,null
   };

   typedef Triple<double,double,double> RGB;
   typedef Triple<double,double,double> HSV;
   typedef Quadruple<double,double,double,double> RGBA;
   typedef Triple<unsigned char,unsigned char,unsigned char> RGB_bytes;
   typedef Quadruple<unsigned char,unsigned char,unsigned char,unsigned char>
      RGBA_bytes;

   unsigned int get_n_colors();
   Color get_color(int i);
   int get_color_index(std::string color_str);
   std::string getcolor(int i);
   Color get_rainbowcolor(int i);
   std::string get_colorstr(const Color color);
   Color string_to_color(std::string colorstr);
   double color_to_value(const Color color);

// RGB methods:

   void print_RGB(const RGB& curr_rgb);
   osg::Vec4 get_OSG_color(const Color color,double alpha=1.0);
   osg::Vec4 get_OSG_color(const RGB& rgb,double alpha=1.0);
   Color get_colorfunc_color(osg::Vec4 osg_color);
   RGB get_RGB_values(const Color color);
   Color get_color(const RGB& curr_rgb);
   void rgb_to_RGB(double r,double g,double b,int& R,int& G,int& B);
   void RGB_to_rgb(int R,int G,int B,double& r,double& g,double& b);
   std::string RGB_to_CSV(const RGB& curr_rgb);
   RGB CSV_to_RGB(std::string csv);

   RGB scalar_value_to_RGB(double min_value,double max_value,double value,
                           bool greyscale_flag=false);
   RGB meta_value_to_RGB(double value);
   RGB meta_value_to_greyscale(double value);
   RGB_bytes RGB_to_bytes(RGB curr_rgb,bool normalized_input_RGB_values=true);
   RGBA_bytes RGBA_to_bytes(
      RGBA curr_rgba,bool normalized_input_RGBA_values=true);
   
   RGB_bytes RGB_to_bytes(double r, double g, double b,
                          bool normalized_input_RGB_values=true);
   RGB bytes_to_RGB(char* RGB_byte);
   RGBA bytes_to_RGBA(const osg::Vec4ub& rgba_bytes);

   double exponential_distributed_S(double lambda);
   double gaussian_distributed_V(double mu, double sigma);
   HSV generate_random_HSV();
   HSV generate_random_HSV(
      double h_min, double h_max, double s_min, double s_max, 
      double v_min, double v_max);
   RGB generate_random_RGB(bool normalized_RGB_values);
   RGB generate_random_RGB(
      bool normalized_RGB_values, 
      double h_min, double h_max, double s_min, double s_max,
      double v_min, double v_max);
   RGB generate_random_RGB(
      bool normalized_RGB_values, double lambda_s,
      double mu_v, double sigma_v);
   int fluctuate_value(int R);
   int fluctuate_value(int R, double value);

// RGB - HSV color coordinate transformation methods

   void RGB_to_hs(double r,double g,double b,double& h,double& s);
   void RGB_to_hsv(double r,double g,double b,double& h,double& s,double& v);
   void hsv_to_RGB(double h,double s,double v,double& r,double& g,double& b);
   void hsv_to_RGBA(double h,double s,double v,double& r,double& g,double& b,
                    double& a);
   HSV RGB_to_hsv(const RGB& curr_rgb,bool normalized_RGB_values=true);
   RGB hsv_to_RGB(const HSV& curr_hsv,bool normalized_RGB_values=true);
   RGBA hsv_to_RGBA(const HSV& curr_hsv,bool normalized_RGB_values=true);

// 16-bit "565" methods

   std::string RGB_to_565_string(int R,int G,int B);
   unsigned short RGB_to_565_short(int R,int G,int B);

// Luminosity methods

   double RGB_to_luminosity(double R,double G,double B);

// RGB colormap methods

   double rgb_colormap_value(const RGB& curr_rgb);
   double rgb_colormap_value(const Color color);
   double rgb_colormap_value(double r,double g,double b);
   void query_rgb_colormap_value();

// Hue-value colormap methods

   void initialize_hue_value_colormap();
   void initialize_big_hue_value_colormap();
   void delete_dataviewer_colormap();
   RGB dataviewer_colormap_to_RGB(double f);
   std::vector<RGB> generate_distinct_colors(
      int n_output_colors,int n_preceding_colors=0);

// Hexadecimal output methods

   std::string RGB_to_RRGGBB_hex(double r,double g,double b);
   std::string RGB_to_RRGGBB_hex(const RGB& curr_rgb);
   std::string RGBA_to_RRGGBBAA_hex(double r,double g,double b,double a);
   std::string RGBA_to_AABBGGRR_hex(double r,double g,double b,double a);
   void RRGGBB_hex_to_rgb(std::string hex_str,double& r,double& g,double& b);
   void RRGGBB_hex_to_RGB(std::string hex_str,RGB& curr_rgb);
   void RRGGBBAA_hex_to_rgba(
      std::string hex_str,double& r,double& g,double& b,double& a);

// Color quantization methods:

   std::vector<std::string> get_quantized_color_labels();
   unsigned int get_n_quantized_colors();
   int assign_hsv_to_color_histogram_bin(
      double h,double s,double v);
   int assign_hsv_to_color_histogram_bin(
      double h,double s,double v,
      double& h_quantized,double& s_quantized,double& v_quantized);

   bool color_match(
      int R1,int G1,int B1,int R2,int G2,int B2,double threshold);
   double color_distance(
      colorfunc::RGB& RGB_1,colorfunc::RGB& RBB_2);

// YCbCr methods:

   void RGB_to_YCbCr(double R, double G, double B, 
                     double& Y, double& Cb, double& Cr);
   void YCbCr_to_RGB(double Y, double Cb, double Cr, 
                     double& R, double& G, double& B);
   
// TOC specific methods

   Color get_platform_color(int platform_ID);
      
}

#endif // colorfuncs.h



