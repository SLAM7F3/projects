// ==========================================================================
// Window header file
// ==========================================================================
// Last updated on 4/30/03
// ==========================================================================

#ifndef WINDOW_H
#define WINDOW_H

#include "math/myvector.h"
#include "datastructures/waveform.h"

namespace window
{
   void alpha_filter(double currx,double filtered_x[],double alpha);
   void alpha_filter(myvector currx,myvector filtered_x[],double alpha);
   void alphabeta_filter(double currx,double filtered_x[],
                         double filtered_xdot[],
                         double alpha,double beta,double dt);
   void alphabeta_filter(myvector currx,myvector filtered_x[],
                         myvector filtered_xdot[],
                         double alpha,double beta,double dt);
   void alphabetagamma_filter(double currx,double filtered_x[],
                              double filtered_xdot[],
                              double filtered_xdotdot[],
                              double alpha,double beta,double gamma,
                              double dt);
   void alphabetagamma_filter(myvector currx,myvector filtered_x[],
                              myvector filtered_xdot[],
                              myvector filtered_xdotdot[],
                              double alpha,double beta,double gamma,
                              double dt);
   void binomialfilter(int n,double tau,double dt,double y_unfiltered,
                       double y[]);
   void binomialfilter(int n,double tau,double dt,
                       myvector y_unfiltered,myvector y[]);
   waveform butterworth4(double delta,double bandwidth,double nucarrier,
                         double nusquint,fftw_plan theforward);
   void butterworth(int norder,double delta,double bandwidth,double nucarrier,
                    double nusquint,waveform& filter,fftw_plan theforward);
   double dolphfreq(int i,int M,double r);
   double dolphfreq(double currtheta,double rdB,double theta_s);
   double dolphwindowfunc(double M);
   waveform generate_bandlimited_dolphwindow(
      double delta,double bandwidth,
      double rdb,fftw_plan thebackward);
   waveform generate_bandlimited_rationalwindow(
      double delta,double deltafreq,
      fftw_plan thebackward);
   waveform generatedolphwindow(int mbin,double delta,double rdb,
                                fftw_plan thebackward);
   waveform generate_rational_window(double delta,double deltafreq,int order,
                                     double a[], double b[],
                                     fftw_plan thebackward);
   double leadlag_filter(double currx,double prevx,double prevy,
                         double alpha,double beta,double gamma,double dt);
   complex leadlag_filter(complex currx,complex prevx,complex prevy,
                          double alpha,double beta,double gamma,double dt);
   myvector leadlag_filter(myvector currx,myvector prevx,myvector prevy,
                           double alpha,double beta,double gamma,double dt);
   waveform shiftwindow(double delta,double deltafreq,double freqshift,
                        waveform& w,fftw_plan thebackward);
   waveform timedomainfilter(int filterorder,double delta,waveform& X,
                             complex a[],complex b[],fftw_plan theforward);
   double Tn(double x,int n);
   waveform windowmultiply(double delta,waveform& signal,waveform& window,
                           fftw_plan theforward);
   double wn(int n,int M,double r);
} // window namespace
 
#endif // filter/window.h
