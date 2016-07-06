/********************************************************************
 *
 *
 * Name: colormaps_opengl.h
 *
 *
 * Author: Joseph Adams
 *
 * Description:
 * creates predefined opengl colormaps
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/
#ifndef _jsa_colormaps_opengl_
#define _jsa_colormaps_opengl_

#include <cstdio>                                /* for the printf warnings on bad allocs */

#include <cstdlib>                               /* for the mallocs  */
enum colormap_t {
   none,
   jet,
   jetwhite,
   gray,
   newhv,
   bighv,
   newhs,
   bighs,
   hsv,
   hot,
   bone,
   copper,
   pink,
   white,
   flag,
   lines,
   colorcube,
   vga,
   prism,
   cool,
   autumn,
   spring,
   winter,
   summer,
   idl_16_level,
   idl_b_w_linear,
   idl_beach,
   idl_blue_green_red_yellow,
   idl_blue_pastel_red,
   idl_blue_red,
   idl_blue_waves,
   idl_blue_white,
   idl_eos_a,
   idl_eos_b,
   idl_green_pink,
   idl_green_white_linear,
   idl_grn_red_blue_wht,
   grn_wht_exponential,
   idl_hardcandy,
   idl_haze,
   idl_hue_sat_lightness_1,
   idl_hue_sat_lightness_2,
   idl_hue_sat_value_1,
   idl_hue_sat_value_2,
   idl_nature,
   idl_ocean,
   idl_pastels,
   idl_peppermint,
   idl_plasma,
   idl_prism,
   idl_purple_red_stripes,
   idl_rainbow,
   idl_raindbow18,
   idl_rainbow_black,
   idl_rainbow_white,
   idl_red_purple,
   idl_red_temperature,
   idl_std_gamma_ii,
   idl_steps,
   idl_stern_special,
   idl_volcano,
   idl_waves};
#define _jsa_colormap_use_all_colormaps_ 
double * colormap(colormap_t cm,int *N_colors);
//double * colormap_interpolate(colormap_t cm,int ncolors);
template<class T>
T * colormap_interpolate(colormap_t cm,int ncolors, T min, T max){
   //printf("interpolate: cm=%d\n",cm);
   int n_colors_original=0;
   T *map=NULL;
   int L=ncolors*3;
   map=(T *)calloc(L,sizeof(T));
   if (map==NULL)
   {
      printf("colormap_jet :Allocation failed");
      exit(1);
   }
   double* map_original=colormap(cm,&n_colors_original);
   double newtoorig=(double)n_colors_original/(double)ncolors;
   double origtonew=1/newtoorig;
   int ipos;
   double dpos;
   double difpos;
   double minorig;
   T range=max-min;
   for(int i=0;i<ncolors;i++){
      ipos=(int)(dpos=(i*newtoorig));
      difpos=dpos-(double)ipos;
      if(ipos>=n_colors_original-1){
         map[i*3]=(T)(map_original[(n_colors_original-1)*3]*range+min);
         map[i*3+1]=(T)(map_original[(n_colors_original-1)*3+1]*range+min);
         map[i*3+2]=(T)(map_original[(n_colors_original-1)*3+2]*range+min);
      }
      else{
         map[i*3]=(T)(((map_original[(ipos+1)*3]-(minorig=map_original[ipos*3]))*difpos+minorig)*range+min);
         map[i*3+1]=(T)(((map_original[(ipos+1)*3+1]-(minorig=map_original[ipos*3+1]))*difpos+minorig)*range+min);
         map[i*3+2]=(T)(((map_original[(ipos+1)*3+2]-(minorig=map_original[ipos*3+2]))*difpos+minorig)*range+min);
      }
   }
   free(map_original);
   return map;
}
colormap_t operator + (colormap_t _cm, colormap_t n);
//double * colormap_jet(int *N_colors);
//double * colormap_gray(int *N_colors);

//#ifdef _jsa_colormap_use_all_colormaps_

/*double * colormap_hsv(int *N_colors);
double * colormap_hot(int *N_colors);
double * colormap_bone(int *N_colors);
double * colormap_copper(int *N_colors);
double * colormap_pink(int *N_colors);
double * colormap_white(int *N_colors);
double * colormap_flag (int *N_colors);
double * colormap_lines(int *N_colors);
double * colormap_colorcube(int *N_colors);
double * colormap_vga(int *N_colors);
double * colormap_prism(int *N_colors);
double * colormap_cool(int *N_colors);
double * colormap_autumn(int *N_colors);
double * colormap_spring(int *N_colors);
double * colormap_winter(int *N_colors);
double * colormap_summer(int *N_colors);
double * colormap_idl_16_level(int *N_colors);
double * colormap_idl_b_w_linear(int *N_colors);
double * colormap_idl_beach(int *N_colors);
double * colormap_idl_blue_green_red_yellow(int *N_colors);
double * colormap_idl_blue_pastel_red(int *N_colors);
double * colormap_idl_blue_red(int *N_colors);
double * colormap_idl_blue_waves(int *N_colors);
double * colormap_idl_blue_white(int *N_colors);
double * colormap_idl_eos_a(int *N_colors);
double * colormap_idl_eos_b(int *N_colors);
double * colormap_idl_green_pink(int *N_colors);
double * colormap_idl_green_white_linear(int *N_colors);
double * colormap_idl_grn_red_blu_wht(int *N_colors);
double * colormap_idl_grn_wht_exponential(int *N_colors);
double * colormap_idl_hardcandy(int *N_colors);
double * colormap_idl_haze(int *N_colors);
double * colormap_idl_hue_sat_lightness_1(int *N_colors);
double * colormap_idl_hue_sat_lightness_2(int *N_colors);
double * colormap_idl_hue_sat_value_1(int *N_colors);
double * colormap_idl_hue_sat_value_2(int *N_colors);
double * colormap_idl_mac_style(int *N_colors);
double * colormap_idl_nature(int *N_colors);
double * colormap_idl_ocean(int *N_colors);
double * colormap_idl_pastels(int *N_colors);
double * colormap_idl_peppermint(int *N_colors);
double * colormap_idl_plasma(int *N_colors);
double * colormap_idl_prism(int *N_colors);
double * colormap_idl_purple_red_stripes(int *N_colors);
double * colormap_idl_rainbow(int *N_colors);
double * colormap_idl_rainbow18(int *N_colors);
double * colormap_idl_rainbow_black(int *N_colors);
double * colormap_idl_rainbow_white(int *N_colors);
double * colormap_idl_red_purple(int *N_colors);
double * colormap_idl_red_temperature(int *N_colors);
double * colormap_idl_std_gamma_ii(int *N_colors);
double * colormap_idl_steps(int *N_colors);
double * colormap_idl_stern_special(int *N_colors);
double * colormap_idl_volcano(int *N_colors);
double * colormap_idl_waves(int *N_colors);
#endif*/
#endif
