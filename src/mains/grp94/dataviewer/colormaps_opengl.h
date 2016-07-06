// ==========================================================================
// Header file for colormaps_func namespace
// ==========================================================================
// Last updated on 1/13/05
// ==========================================================================

/********************************************************************
 * Name: colormaps_opengl.h
 *
 * Author: Joseph Adams
 *
 * Description:
 * creates predefined opengl colormaps
 *
 **********************************************************************/

#ifndef COLORMAPS_OPENGL_H
#define COLORMAPS_OPENGL_H

#include <cstdio>          // for the printf warnings on bad allocs 
#include <cstdlib>         // for the mallocs  

enum colormap_t 
{
   jet,
   jetwhite,
//   gray,
   newhv,
   bighv,
   rgb,
   hvs_zero,
   hvs_pointone,
   hvs_pointtwo,
   hvs_pointthree,
   hvs_pointfour,
   hvs_pointfive,
   hvs_pointsix,
   hvs_pointseven,
   hvs_pointeight,
   hvs_pointnine,
   hvs_one,
//    dynamic_hsv,
//   newhs,
//   bighs
};

namespace colormaps_func
{
   extern const int number_of_colormaps;

   colormap_t operator+ (colormap_t _cm, colormap_t n);
   double* colormap(colormap_t cm,int *N_colors);
   void RGB_to_hsv(double r,double g,double b,double& h,double& s,double& v);
   void hsv_to_RGB(double h,double s,double v,double& r,double& g,double& b);
   int generate_dynamic_colormap(double s,double map[]);
   int generate_rgb_colormap(double map[]);

   template<class T>
      T* colormap_interpolate(colormap_t cm,int ncolors, T min, T max)
      {
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
               map[i*3+1]=(T)(map_original[(n_colors_original-1)*3+1]
                              *range+min);
               map[i*3+2]=(T)(map_original[(n_colors_original-1)*3+2]
                              *range+min);
            }
            else{
               map[i*3]=(T)(
                  ((map_original[(ipos+1)*3]-
                    (minorig=map_original[ipos*3]))*difpos+minorig)*range+min);
               map[i*3+1]=(T)(
                  ((map_original[(ipos+1)*3+1]-
                    (minorig=map_original[ipos*3+1]))*difpos+minorig)
                  *range+min);
               map[i*3+2]=(T)(
                  ((map_original[(ipos+1)*3+2]-
                    (minorig=map_original[ipos*3+2]))*difpos+minorig)
                  *range+min);
            }
         }
         free(map_original);
         return map;
      }
} // colormaps_func namespace

#endif //  COLORMAPS_OPENGL_H
