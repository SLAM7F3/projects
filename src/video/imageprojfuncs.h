// ==========================================================================
// Header file for imageprojfunc namespace
// ==========================================================================
// Last modified on 10/14/08
// ==========================================================================

#ifndef IMAGEPROJFUNCS_H
#define IMAGEPROJFUNCS_H

namespace imageprojfunc
{
   void calcImgProj(
      double a[5], double v[3], double t[3], double M[3], double n[2]);
   void calcImgProjJacKRTS(
      double a[5], double v[3], double t[3], double M[3], 
      double jacmKRT[2][11], double jacmS[2][3]);
   void calcImgProjJacKRT(
      double a[5], double v[3], double t[3], double M[3], 
      double jacmKRT[2][11]);
   void calcImgProjJacS(
      double a[5], double v[3], double t[3], double M[3], double jacmS[2][3]);
   void calcImgProjJacRTS(
      double a[5], double v[3], double t[3], double M[3], 
      double jacmRT[2][6], double jacmS[2][3]);
   void calcImgProjJacRT(
      double a[5], double v[3], double t[3], double M[3], 
      double jacmRT[2][6]);

} // imageprojfunc namespace

#endif // imageprojfuncs.h

