// ==========================================================================
// WINDOW.CC contains various subroutines for computing and
// manipulating filter windows within the time and frequency domains.
// ==========================================================================
// Last updated on 7/15/03
// ==========================================================================

#include "math/mathfuncs.h"
#include "filter/window.h"

using std::ostream;
using std::cout;
using std::endl;
using std::string;

namespace window
{
   
// ==========================================================================
// Global variables
// ==========================================================================

// Unfortunately in order to conform to the Numerical Recipes
// conventions that functions whose roots are to be found must take
// only one argument, we seem to need to define the r and theta
// variables as global.  We copy below the contents of r and theta into
// RGLOBAL and THETAGLOBAL and use these global variables only within the
// generate_bandlimited_dolphwindow function.

   double RGLOBAL;
   double THETAGLOBAL;

// ---------------------------------------------------------------------
// First order alpha filter:

   void alpha_filter(double currx,double filtered_x[],double alpha)
      {
// First move previous time's filtered x value from 0th to 1st
// position in filtered array:

         filtered_x[1]=filtered_x[0];

// Filtered estimate for position at present time:
         filtered_x[0]=alpha*currx+(1-alpha)*(filtered_x[1]);
      }

// ---------------------------------------------------------------------
// First order alpha filter:

   void alpha_filter(myvector currx,myvector filtered_x[],double alpha)
      {
// First move previous time's filtered x value from 0th to 1st
// position in filtered array:

         filtered_x[1]=filtered_x[0];

// Filtered estimate for position at present time:
         filtered_x[0]=alpha*currx+(1-alpha)*(filtered_x[1]);
      }

// ---------------------------------------------------------------------
// Second order alpha-beta filter:

   void alphabeta_filter(double currx,double filtered_x[],
                         double filtered_xdot[],
                         double alpha,double beta,double dt)
      {
// First move previous time's filtered x and xdot values from 0th to
// 1st positions in filtered arrays:

         filtered_x[1]=filtered_x[0];
         filtered_xdot[1]=filtered_xdot[0];

// Filtered estimate for velocity at present time:   
         filtered_xdot[0]=beta*(currx-filtered_x[1])/dt+
            (1-beta)*filtered_xdot[1];

// Filtered estimate for position at present time:
         filtered_x[0]=alpha*currx+(1-alpha)*(filtered_x[1]+
                                              dt*filtered_xdot[1]);
      }

// ---------------------------------------------------------------------
// Overloaded version of second order alpha-beta filter with myvector
// input/output:

   void alphabeta_filter(myvector currx,myvector filtered_x[],
                         myvector filtered_xdot[],
                         double alpha,double beta,double dt)
      {
// First move previous time's filtered x and xdot values from 0th to
// 1st positions in filtered arrays:

         filtered_x[1]=filtered_x[0];
         filtered_xdot[1]=filtered_xdot[0];

// Filtered estimate for velocity at present time:   
         filtered_xdot[0]=beta*(currx-filtered_x[1])/dt+
            (1-beta)*filtered_xdot[1];

// Filtered estimate for position at present time:
         filtered_x[0]=alpha*currx+(1-alpha)*(filtered_x[1]+
                                              dt*filtered_xdot[1]);
      }

// ---------------------------------------------------------------------
// Third order alpha-beta-gamma filter:

   void alphabetagamma_filter(double currx,double filtered_x[],
                              double filtered_xdot[],double filtered_xdotdot[],
                              double alpha,double beta,double gamma,double dt)
      {
// First move previous time's filtered x, xdot and xdotdot values from
// 0th to 1st positions in filtered arrays:

         filtered_x[1]=filtered_x[0];
         filtered_xdot[1]=filtered_xdot[0];
         filtered_xdotdot[1]=filtered_xdotdot[0];

// Filtered estimate for acceleration at present time:   
         filtered_xdotdot[0]=(1-gamma/2)*filtered_xdotdot[1]
            -gamma*filtered_xdot[1]/dt
            +gamma*(currx-filtered_x[1])/sqr(dt);

// Filtered estimate for velocity at present time:   
         filtered_xdot[0]=beta*(currx-filtered_x[1])/dt+
            (1-beta)*filtered_xdot[1]
            +(1-beta/2)*filtered_xdotdot[1]*dt;

// Filtered estimate for position at present time:
         filtered_x[0]=alpha*currx+(1-alpha)*(
            filtered_x[1]+filtered_xdot[1]*dt
            +0.5*filtered_xdotdot[1]*sqr(dt));
      }


// ---------------------------------------------------------------------
// Overloaded version of third order alpha-beta-gamma filter with
// myvector input/output:

   void alphabetagamma_filter(myvector currx,myvector filtered_x[],
                              myvector filtered_xdot[],
                              myvector filtered_xdotdot[],
                              double alpha,double beta,double gamma,double dt)
      {
// First move previous time's filtered x, xdot and xdotdot values from
// 0th to 1st positions in filtered arrays:

         filtered_x[1]=filtered_x[0];
         filtered_xdot[1]=filtered_xdot[0];
         filtered_xdotdot[1]=filtered_xdotdot[0];

// Filtered estimate for acceleration at present time:   
         filtered_xdotdot[0]=(1-gamma/2)*filtered_xdotdot[1]
            -gamma*filtered_xdot[1]/dt
            +gamma*(currx-filtered_x[1])/sqr(dt);

// Filtered estimate for velocity at present time:   
         filtered_xdot[0]=beta*(currx-filtered_x[1])/dt+
            (1-beta)*filtered_xdot[1]
            +(1-beta/2)*filtered_xdotdot[1]*dt;

// Filtered estimate for position at present time:
         filtered_x[0]=alpha*currx+(1-alpha)*(
            filtered_x[1]+filtered_xdot[1]*dt
            +0.5*filtered_xdotdot[1]*sqr(dt));
      }

// ---------------------------------------------------------------------
// Subroutine binomialfilter takes in a filter order parameter n,a raw
// measured value y_unfiltered as well as an array y[] of previously
// filtered measured values (angles, accelerations, etc).  It returns
// the new filtered value which combines the old and new values within
// y[n].

   void binomialfilter(
      int n,double tau,double dt,double y_unfiltered,double y[])
      {
         int i;
         double beta,term[20];
   
// Shift previously filtered measurements for past n timesteps one bin
// down in the filtered array:

         for (i=0; i<n; i++)
         {
            y[i]=y[i+1];
         }

// Pass raw and old measurements through filter:

         beta=(n*dt+tau)/(dt*tau);
   
         term[0]=pow(n/(beta*tau),n)*y_unfiltered;
         for (i=1; i<=n; i++)
         {
            term[i]=mathfunc::choose(n,i)/pow(beta*dt,i)*y[n-i];
            if (is_even(i))
            {
               term[i]=-term[i];
            }
         }
   
         y[n]=0;
         for (i=0; i<=n; i++)
         {
            y[n] += term[i];
         }
      }

// ---------------------------------------------------------------------
// This overloaded version of subroutine binomialfilter takes in a
// filter order parameter n, a raw measured vector y_unfiltered as
// well as an array y[] of previously filtered measured vectors.  It
// returns the new filtered vector which combines the old and new
// values within y[n].

   void binomialfilter(int n,double tau,double dt,
                       myvector y_unfiltered,myvector y[])
      {
         int i;
         double beta;
         myvector term[20];
   
// Shift previously filtered measurements for past n timesteps one bin
// down in the filtered array:

         for (i=0; i<n; i++)
         {
            y[i]=y[i+1];
         }

// Pass raw and old measurements through filter:

         beta=(n*dt+tau)/(dt*tau);
   
         term[0]=pow(n/(beta*tau),n)*y_unfiltered;
         for (i=1; i<=n; i++)
         {
            term[i]=mathfunc::choose(n,i)/pow(beta*dt,i)*y[n-i];
            if (is_even(i))
            {
               term[i]=-term[i];
            }
         }
   
         y[n].vectclear();
         for (i=0; i<=n; i++)
         {
            y[n] += term[i];
         }
      }

// ---------------------------------------------------------------------
// Subroutine butterworth4 generates a fourth order Butterworthfilter
// in the time domain with a specified bandwidth, carrier frequency and
// frequency squint.  The result is returned as a waveform.

   waveform butterworth4(double delta,double bandwidth,double nucarrier,
                         double nusquint,fftw_plan theforward)
      {
         const int N=2048;   // should agree with N defined in waveform class
         const double PI=3.14159265358979323846;

         int i;
         double BWang,wcutoff,wcarrier,wsquint;
         double theta0,theta1,theta2;
         double cos1,cos2,sin1,sin2;
         double t,prefactor,term1,term2,term3,term4;
         double envelope;
         complex carrierterm,phasewrap;
         complex *h=new complex[N];

         BWang=2*PI*bandwidth;
         wcutoff=BWang/2;
         wcarrier=2*PI*nucarrier;
         wsquint=2*PI*nusquint;

         theta0=3*PI/8;
         theta1=5*PI/8;
         theta2=7*PI/8;
   
         cos1=cos(theta1);
         sin1=sin(theta1);
         cos2=cos(theta2);
         sin2=sin(theta2);

         for (i=0; i<N; i++)
         {
            t=i*delta;
            term1=exp(cos1*wcutoff*t);
            term2=cos(sin1*wcutoff*t+theta2);
            prefactor=1/(sqrt(2)-1);
            term3=exp(cos2*wcutoff*t);
            term4=cos(sin2*wcutoff*t-theta0);

            envelope=wcutoff*(term1*term2+prefactor*term3*term4);
//      carrierterm=2*cos(wcarrier*t);
            carrierterm=1;

// Squinting filter off carrier frequency adds phase wrapping to real
// envelope function:

            phasewrap=complex(cos(wsquint*t),sin(wsquint*t));

            h[i]=envelope*carrierterm*phasewrap;
         }
         waveform filter(delta,h);
         filter.fouriertransform(delta,theforward);

         delete [] h;
         return filter;
      }

// ---------------------------------------------------------------------
// Subroutine butterworth generates an n-th order Butterworth filter
// in the time domain with a specified bandwidth BW, carrier frequency
// and frequency squint.  n is limited to integer values less than or
// equal to 10 values.  The filter's impulse response function was
// determined using Mathematica program "butterworth.m".

// Note added on 6/4/99: We have attempted to optimize the norder=4 and 7
// cases since they are repeatedly called by discrimtime and the flyout code.

   void butterworth(int norder,double delta,double BW,double nucarrier,
                    double nusquint,waveform& filter,fftw_plan theforward)
      {
         const int N=2048;  // should agree with N defined in waveform class
         const double PI=3.14159265358979323846;

         int i;
         double t,BWt,wcarrier,wsquint;
         double e1,e2,e3,c1,s1;
         complex carrierterm,envelope,phasewrap;

         wcarrier=2*PI*nucarrier;
         wsquint=2*PI*nusquint;

         for (i=0; i<N; i++)
         {
            t=i*delta;
   
            BWt=BW*t;
            if (BWt > 50)
            {
               envelope=0;
            }
            else
            {
               if (norder==1)
               {
                  envelope=(-3.141592653589793*BW)/
                     exp(3.141592653589793*BWt);
               }
               else if (norder==2)
               {
                  envelope = (4.442882938158366*BW*sin(2.221441469079183*BWt))/
                     exp(2.221441469079183*BWt);
               }
               else if (norder==3)
               {
                  envelope = (3.141592653589792*
                              (-1.*exp(1.570796326794896*BWt)*BW + 
                               1.*exp(3.141592653589792*BWt)*BW*
                               cos(2.720699046351326*BWt) - 
                               0.5773502691896256*
                               exp(3.141592653589792*BWt)*BW*
                               sin(2.720699046351326*BWt)))/
                     exp(4.712388980384689*BWt);
               }
               else if (norder==4)
               {
                  e1 = exp(1.202235459768692*BWt)*BW;
                  e2 =  exp(2.90245315213943*BWt)*BW;

                  envelope = (-2.902453152139431*
                              (-e1*cos(1.202235459768692*BWt) + 
                               e2*cos(2.90245315213943*BWt) - 
                               2.414213562373094*e1*
                               sin(1.202235459768692*BWt) + 
                               0.4142135623730948*e2*
                               sin(2.90245315213943*BWt)))/
                     exp(4.104688611908123*BWt);
               }
               else if (norder==5)
               {
                  envelope = (0.868314853690823*
                              (-6.85410196624969*
                               exp(3.512407365520362*BWt)*BW + 
                               5.854101966249691*
                               exp(4.112398172952526*BWt)*BW*
                               cos(1.846581830490456*BWt) + 
                               1.*exp(5.683194499747422*BWt)*BW*
                               cos(2.987832164741556*BWt) - 
                               8.05748010694082*exp(4.112398172952526*BWt)*BW*
                               sin(1.846581830490456*BWt) + 
                               3.077683537175255*exp(5.683194499747422*BWt)*BW*
                               sin(2.987832164741556*BWt)))/
                     exp(6.654000019110156*BWt);
               }
               else if (norder==6)
               {
                  envelope = (1.282549830161864*
                              (6.464101615137749*exp(3.034545479782387*BWt)*BW*
                               cos(0.813104010703205*BWt) - 
                               7.464101615137748*exp(3.847649490485593*BWt)*BW*
                               cos(2.221441469079183*BWt) + 
                               exp(5.25598694886157*BWt)*BW*
                               cos(3.034545479782388*BWt) + 
                               11.19615242270661*exp(3.034545479782387*BWt)*BW*
                               sin(0.813104010703205*BWt) - 
                               1.732050807568874*exp(5.25598694886157*BWt)*BW*
                               sin(3.034545479782388*BWt)))/
                     exp(6.069090959564775*BWt);
               }
               else if (norder==7)
               {
                  e1=exp(8.45723487857909*BWt)*BW;
                  e2=exp(9.32896107398689*BWt)*BW;
                  e3=exp(10.58864192370734*BWt)*BW;
                  c1=cos(1.36308596732379*BWt);
                  s1=sin(1.36308596732379*BWt);

                  envelope=(complex(-1.157639553604772,-0.2642236738062165)*
                            (complex(11.12229334880575,-2.538590877117886)*
                             exp(8.14611940150465*BWt)*BW - 
                             complex( 7.69687, -1.75676)*e1*c1 - 
                             complex(5.326396405565987,-1.215715221585583)
                             *e2*
                             cos(2.456196041666779*BWt) + 
                             complex(1.900968867902418,-0.4338837391175578)
                             *e3*
                             cos(3.062826366690051*BWt) + 
                             complex(15.9827,-3.64795)*e1*s1 - 
                             complex(6.679089835248941,-1.524458669761153)*e2*
                             sin(2.456196041666779*BWt) - 
                             complex(0.4338837391175578,-0.0990311320975812)
                             *e3*
                             sin(3.062826366690051*BWt)))/
                     exp(11.28771205509444*BWt);
               }
               else if (norder==8)
               {
                  envelope= (1.847061077048066*
                             (12.13707118454407*exp(4.970408510732717*BWt)*BW*
                              cos(0.6128943224323367*BWt) - 
                              14.31665161164734*exp(5.43949751670368*BWt)*BW*
                              cos(1.745375362607551*BWt) + 
                              1.179580427103263*exp(6.306260979788959*BWt)*BW*
                              cos(2.61213882569283*BWt) + 
                              exp(7.438742019964175*BWt)*BW*
                              cos(3.081227831663793*BWt) + 
                              18.16441067666988*exp(4.970408510732717*BWt)*BW*
                              sin(0.6128943224323367*BWt) + 
                              2.847759065022562*exp(5.43949751670368*BWt)*BW*
                              sin(1.745375362607551*BWt) - 
                              5.930151265314955*exp(6.306260979788959*BWt)*BW*
                              sin(2.61213882569283*BWt) + 
                              0.6681786379192979*exp(7.438742019964175*BWt)*BW*
                              sin(3.081227831663793*BWt)))/
                     exp(8.05163634239651*BWt);
               }
               else if (norder==9)
               {
                  envelope=(-0.3636878928451229*
                            (92.6109271910071*exp(7.475059194981993*BWt)*BW - 
                             complex(26.57831517204964,-73.02332078235446)*
                             exp(7.664520414478237*BWt)*BW*
                             cos(1.074487969651648*BWt) - 
                             complex(26.57831517204952,73.02332078235451)*
                             exp(7.664520414478237*BWt)*BW*
                             cos(1.074487969651649*BWt) - 
                             68.73834908286241*exp(8.21005225374592*BWt)*BW*
                             cos(2.019376832409775*BWt) + 
                             28.2840522359545*exp(9.04585552177689*BWt)*BW*
                             cos(2.720699046351326*BWt) + 
                             exp(10.0711200093041*BWt)*BW*
                             cos(3.093864802061424*BWt) + 
                             complex(73.02332078235446,26.57831517204964)*
                             exp(7.664520414478237*BWt)*BW*
                             sin(1.074487969651648*BWt) + 
                             complex(73.02332078235451,-26.57831517204952)*
                             exp(7.664520414478237*BWt)*BW*
                             sin(1.074487969651649*BWt) - 
                             57.67832336316494*exp(8.21005225374592*BWt)*BW*
                             sin(2.019376832409775*BWt) - 
                             16.32980517220174*exp(9.04585552177689*BWt)*BW*
                             sin(2.720699046351326*BWt) + 
                             5.671281819617701*exp(10.0711200093041*BWt)*BW*
                             sin(3.093864802061424*BWt)))/
                     exp(10.61665184857178*BWt);
               }
               else if (norder==10)
               {
                  envelope=(-1.167880894046564*
                            (-52.49096121841128*exp(6.938327604689893*BWt)*BW*
                             cos(0.4914533661386389*BWt) + 
                             61.70682576567666*exp(7.242062488849118*BWt)*BW*
                             cos(1.426253218781319*BWt) - 
                             10.21586454726532*exp(8.61498882075855*BWt)*BW*
                             cos(2.799179550690753*BWt) + 
                             exp(9.54978867340123*BWt)*BW*
                             cos(3.102914434849977*BWt) - 
                             72.24761000917483*exp(6.938327604689893*BWt)*BW*
                             sin(0.4914533661386389*BWt) - 
                             20.04976308328054*exp(7.242062488849118*BWt)*BW*
                             sin(1.426253218781319*BWt) + 
                             33.05923212388069*exp(7.819800570460689*BWt)*BW*
                             sin(2.221441469079183*BWt) - 
                             3.319335605453961*exp(8.61498882075855*BWt)*BW*
                             sin(2.799179550690753*BWt) - 
                             1.376381920471171*exp(9.54978867340123*BWt)*BW*
                             sin(3.102914434849977*BWt)))/
                     exp(10.04124203953987*BWt);
               }
            }
      
//    carrierterm=2*cos(wcarrier*t);
//      carrierterm=1;
            carrierterm=complex(cos(wcarrier*t),sin(wcarrier*t));
   
// Squinting filter off carrier frequency adds phase wrapping to real
// envelope function:

            phasewrap=complex(cos(wsquint*t),sin(wsquint*t));
            filter.val[i]=envelope*carrierterm*phasewrap;
         }
         filter.fouriertransform(delta,theforward);
      }

// --------------------------------------------------------------------------
// Function dolphfreq computes the Dolph-Chebyshev window coefficients
// in the frequency domain.  This routine is based upon eqn (1)
// contained within the paper "The Dolph-Chebyshev window: A simple
// optimal filter", Peter Lynch, Monthly weather review, April 1997, 
// p 655.

   double dolphfreq(int i,int M,double r)
      {
         const int N=2048;    // should agree with N defined in waveform class
         const double PI=3.14159265358979323846;

         int twoM;
         double x0,currtheta,term1,term2;

         twoM=2*M;
         x0=cosh(mathfunc::coshinv(1/r)/twoM);

         currtheta=-PI+2*PI*i/N;
         term1=x0*cos(currtheta/2.);
         term2=Tn(term1,twoM);

// Note:  If we remove the (-1)**i term from below, the mainlobe appears
// split across the beginning and end of the *time* domain array rather
// than being centered about the middle bin.  Of course, the magnitude
// of the filter in the frequency domain remains unchanged whether we include
// the phase prefactor or not.

//   return(negonepower(i)*r*term2);   
         return(r*term2);   
      }

// --------------------------------------------------------------------------
// Function dolphfreq computes the Dolph-Chebyshev window coefficients
// in the frequency domain.  This routine is based upon eqn (1)
// contained within the paper "The Dolph-Chebyshev window: A simple
// optimal filter", Peter Lynch, Monthly weather review, April 1997, 
// p 655.

// theta_s = stop band edge in rads
// rdB = ripple ratio in dB = sidelobe/mainlobe power suppression factor

   double dolphfreq(double currtheta,double rdB,double theta_s)
      {
         int M,twoM;
         double r,x0,numer,denom,term1,term2;

         r=pow(10,-fabs(rdB)/10.);
         x0=1.0/cos(theta_s/2.0);

// N = 2M+1 = filter order 

         numer=mathfunc::coshinv(1.0/r);
         denom=mathfunc::coshinv(1.0/cos(theta_s/2.0));
         M=mathfunc::round(numer/(denom*2));
         twoM=2*M;

         term1=x0*cos(currtheta/2.);
         term2=Tn(term1,twoM);

         return(r*term2);   
      }

// ---------------------------------------------------------------------
   double dolphwindowfunc(double M)
      {
         double x0,term1,term2,term3,f;
   
         x0=cosh(mathfunc::coshinv(1/RGLOBAL)/(2*M));
         term1=x0*cos(THETAGLOBAL/2.0);
         term2=Tn(term1,int(2*M));
         term3=1/(2.0*RGLOBAL);
         f=term2-term3;
         return f;
      }

// ---------------------------------------------------------------------
// Function generate_bandlimited_dolphwindow creates and returns a
// Dolph-Chebyshev waveform centered about zero frequency with a
// specified FWHM bandwidth.  The width of the returned window only
// *approximately* agrees with the requested input bandwidth due to
// errors arising from truncating fractional parts of the Chebyshev
// polynomial order M.  (The true window width also varies with N.)
// The maximum time domain window coefficient is normalized to have
// unit magnitude. 

// Dolph-Chebyshev window input parameters:

// bandwidth: FWHM interval in frequency space over which window is
// within 3dB of its maximum value.  

// rdb = Dolph-Chebyshev window sidelobe/mainlobe suppression factor in dB

   waveform generate_bandlimited_dolphwindow(double delta,double bandwidth, 
                                             double rdb,fftw_plan thebackward)
      {
         const int N=2048;   // should agree with N defined in waveform class
         const double PI=3.14159265358979323846;

         int i,mbin;
         double r,theta;
         double dB_lo,dB_hi,freq_lo,freq_hi,truebandwidth;
         double dolphmag1,dolphmag2;
         complex *Wtilde=new complex[N];

// Compute the Chebyshev polynomial order M for which the
// Dolph-Chebyshev window has a FWHM value equal to bandwidth in the
// frequency domain:

//   cout << "Inside generate_bandlimited_dolphwindow, bandwidth = " <<
//      bandwidth;
//   cout << "rdb = " << rdb << endl;

         r=pow(10,-fabs(rdb)/10.);
         theta=2*PI*(bandwidth/2.0)*delta;
         RGLOBAL=r;
         THETAGLOBAL=theta;
         mbin=mathfunc::round(numrec::zbrent(&dolphwindowfunc,1,500,0.1));
         cout << "mbin = " << mbin << endl;
   
         for (i=0; i < N; i++)
         {
            Wtilde[i]=complex(dolphfreq(i,mbin,r),0);
         }

         waveform dolphwindow(delta,Wtilde);
         dolphwindow.inversefouriertransform(delta,thebackward);
         complex normalization=1.0/dolphwindow.valmagmax;
         dolphwindow.rescale_val(normalization);
//   dolphwindow.inspect_contents();

//   Check window's true bandwidth

         i=0;
         while (i<N-1)
         {
            dolphmag1=dolphwindow.tilde[i].getmod();
            dolphmag2=dolphwindow.tilde[i+1].getmod();
      
            dB_lo=mathfunc::dB(dolphwindow.freqnorm*dolphmag1);
            dB_hi=mathfunc::dB(dolphwindow.freqnorm*dolphmag2);
            freq_lo=dolphwindow.Freq[i];
            freq_hi=dolphwindow.Freq[i+1];

            if (dB_lo > -3 && dB_hi < -3)
            {
               truebandwidth=2*(freq_lo+(freq_hi-freq_lo)*
                                (-3-dB_lo)/(dB_hi-dB_lo));

/*
  cout << "dolphwindow.freqnorm = " << dolphwindow.freqnorm << endl;
  cout << "i = " << i << endl;
  cout << "mathfunc::dB(dolphwindow.tilde[i].r) = " << dB_lo << endl;
  cout << "mathfunc::dB(dolphwindow.tilde[i+1].r) = " << dB_hi << endl;
  cout << "freq_lo = " << freq_lo << endl;
  cout << "freq_hi = " << freq_hi << endl;
         
  cout << "True Dolph-Chebyshev window bandwidth = " 
  << truebandwidth << " KHz " << endl;
*/

            }
            i++;
         }

         delete [] Wtilde;
         return dolphwindow;
      }
 
// ---------------------------------------------------------------------
   waveform generate_bandlimited_rationalwindow(double delta,double deltafreq,
                                                fftw_plan thebackward)
      {
         double *a=new double[5];
         double *b=new double[5];

// Elliptic filter coefficients

         a[0]=1;
         a[1]=10.31052760307442;
         a[2]=130.2190706301566;
         a[3]=686.118333046533;
         a[4]=2362.785501879845;
   
         b[0]=0.001777449555329686;
         b[1]=-8.121403611986001e-19;
         b[2]=5.496695852433233;
         b[3]=-1.700683487858573e-15;
         b[4]=2230.613351380357;

// Butterworth filter coefficients

// Matlab generated coefficients
/*
  a[0]=1;
  a[1]=24.62813167145;
  a[2]=303.27243481311;
  a[3]=2187.62925061625;
  a[4]=7890.13637375420;

  b[0]=0;
  b[1]=0;
  b[2]=0;
  b[3]=0;
  b[4]=7890.13637375420;
*/

         waveform filter=generate_rational_window(delta,deltafreq,4,a,b,
                                                  thebackward);

         delete [] a;
         delete [] b;
         return filter;
      }

// ---------------------------------------------------------------------
// Function generatedolphwindow creates and returns a Dolph-Chebyshev
// waveform centered about zero frequency.  The maximum time domain
// window coefficient is normalized to have unit magnitude.

// Dolph-Chebyshev window input parameters:

// mbin: Bin index number on time domain weights runs from -mbin to
// +mbin .  We convert these limits to 0 to +2*mbin.  

// rdb = Dolph-Chebyshev window sidelob/mainlobe suppression factor in dB

   waveform generatedolphwindow(int mbin,double delta,double rdb,
                                fftw_plan thebackward)
      {
         const int N=2048;   // should agree with N defined in waveform class

         int i;
         double r;
         complex *Wtilde=new complex[N];
   
         r=pow(10,-fabs(rdb)/10.);

         for (i=0; i < N; i++)
         {
            Wtilde[i]=complex(dolphfreq(i,mbin,r),0);
         }

         waveform dolphwindow(delta,Wtilde);
         dolphwindow.inversefouriertransform(delta,thebackward);
         complex normalization=1.0/dolphwindow.valmagmax;
         dolphwindow.rescale_val(normalization);

         delete [] Wtilde;
         return dolphwindow;
      }

// ---------------------------------------------------------------------
// Function generate_rational_window creates and returns a filter of
// the form H(w)=B(w)/A(w) where A and B are polynomials of some
// specified order.  The filter is returned in a waveform which is
// centered about zero frequency.  

   waveform generate_rational_window(double delta,double deltafreq,
                                     int filterorder,
                                     double a[], double b[],
                                     fftw_plan thebackward)
      {
         const int N=2048;   // should agree with N defined in waveform class
         const double PI=3.14159265358979323846;

         int i,j;
         double nu,omega;
         complex s,numer,denom;
         complex *Wtilde=new complex[N];
   
         for (i=0; i<N; i++)
         {
            nu=(-N/2+i)*deltafreq;
            omega=2*PI*nu;
            s=complex(0,omega);
            numer=denom=0;
            for (j=0; j<=filterorder; j++)
            {
               numer += b[j]*s.power(s,filterorder-j);
               denom += a[j]*s.power(s,filterorder-j);
            }
            Wtilde[i]=numer/denom;
         }

         waveform rational_filter(delta,Wtilde);
         rational_filter.inversefouriertransform(delta,thebackward);

         delete [] Wtilde;
         return rational_filter;
      }

// ---------------------------------------------------------------------
// The following overloaded versions of subroutine leadlag_filter
// recursively implement the convolution of a lead lag filter with an
// input signal x(t) within the time domain.  The filter's impulse
// response function within the complex frequency domain is given by
// H(s) = (gamma+alpha s)/(1+beta s).  The routines return the
// convolved value y(t) = h(t) * x(t):

   double leadlag_filter(double currx,double prevx,double prevy,
                         double alpha,double beta,double gamma,double dt)
      {
         double curry;
   
         curry=(beta*prevy+(alpha+gamma*dt)*currx-alpha*prevx)/(dt+beta);
         return curry;
      }

   complex leadlag_filter(complex currx,complex prevx,complex prevy,
                          double alpha,double beta,double gamma,double dt)
      {
         complex curry;
   
         curry=(beta*prevy+(alpha+gamma*dt)*currx-alpha*prevx)/(dt+beta);
         return curry;
      }

   myvector leadlag_filter(myvector currx,myvector prevx,myvector prevy,
                           double alpha,double beta,double gamma,double dt)
      {
         myvector curry;
   
         curry=(beta*prevy+(alpha+gamma*dt)*currx-alpha*prevx)/(dt+beta);
         return curry;
      }

// ---------------------------------------------------------------------
// Routine shiftwindow takes in a waveform w and shifts all elements
// within its frequency domain array by some specified frequency
// interval.  This routine effectively convolves the frequency
// spectrum with a delta function centered about nu=freqshift.  After
// taking the inverse fourier transform to obtain the resulting
// modified signal within the time domain, shiftwindow normalizes the
// new (complex) signal's time domain value with maximum modulus to
// unity and then returns the altered waveform.

   waveform shiftwindow(double delta,double deltafreq,double freqshift,
                        waveform& w,fftw_plan thebackward)
      {
         const int N=2048;   // should agree with N defined in waveform class

         int i,j,nshift;
         complex *Wshifttilde=new complex[N];
   
         nshift=mathfunc::round(freqshift/deltafreq);
  
//   cout << "Inside shiftwindow, freqshift = " << freqshift 
//        << "  nshift = " << nshift << endl;
 
         for (i=0; i<N; i++)
         {
            j=i+nshift;
            if (j < 0) 
            {
               j += N;
            }
            else if (j >= N)
            {
               j -= N;
            }
            Wshifttilde[j]=w.tilde[i];
         }
   
         waveform wshift(delta,Wshifttilde);
         wshift.inversefouriertransform(delta,thebackward);
//   complex normalization=1.0/wshift.valmagmax;
//   wshift.rescale_val(normalization);

         delete [] Wshifttilde;
         return wshift;
      }

// ---------------------------------------------------------------------

   waveform timedomainfilter(int filterorder,double delta,waveform& X,
                             complex a[],complex b[],fftw_plan theforward)
      {
         const int N=2048;   // should agree with N defined in waveform class

         int i,n,k;
         complex sum1,sum2;
         complex *Y=new complex[N];
   
         for (i=0; i<N; i++)
         {
            Y[i]=0;
         }

         for (n=0; n<N; n++)
         {
            sum1=sum2=0;
            for (k=0; k<=filterorder; k++)
            {
               if (n >= k)
               {
                  sum1 += b[k]*X.val[n-k];
               }
            }
            for (k=1; k<=filterorder; k++)
            {
               if (n >= k)
               {
                  sum2 += a[k]*Y[n-k];
               }
            }
            Y[n]=(sum1-sum2)/a[0];
         }

         waveform filteredsignal(delta,Y);
         filteredsignal.fouriertransform(delta,theforward);

         delete [] Y;
         return filteredsignal;
      }

// --------------------------------------------------------------------------
// Function Tn returns the nth order Chebyshev polynomial value 

   double Tn(double x,int n)
      {
         int sign=1;

// T_n(-x) = T_n(x) for n even
// T_n(-x) = - T_n(x) for n odd
   
         if (x < 0)
         {
            x=-x;
            if (is_even(n))
            {
               sign=1;
            }
            else
            {
               sign=-1;
            }
         }
   
         if (x > 1)
         {
            return (sign*cosh(n*mathfunc::coshinv(x)));
         }
         else
         {
            return(sign*cos(n*mathfunc::cosinv(x)));
         }
      }

// ---------------------------------------------------------------------
// Function windowmultiply multiplies together the time domain values from
// an input signal and window waveform and returns the result as an output
// waveform.

   waveform windowmultiply(double delta,waveform& signal,waveform& window,
                           fftw_plan theforward)
      {
         const int N=2048;   // should agree with N defined in waveform class

         int i;
         complex *Z=new complex[N];

         for (i=0; i<N; i++)
         {
            Z[i]=signal.val[i]*window.val[i];
         }
         waveform windowsignal(delta,Z);
         windowsignal.fouriertransform(delta,theforward);

         delete [] Z;
         return windowsignal;
      }

// --------------------------------------------------------------------------
// Function wn directly computes the Dolph-Chebyshev window coefficients
// in the time domain without first transforming frequency domain results.
// This routine is based upon eqn (3) contained within the paper "The 
// Dolph-Chebyshev window: A simple optimal filter", Peter Lynch, 
// Monthly weather review, April 1997, p 655.

   double wn(int n,int M,double r)
      {
         const double PI=3.14159265358979323846;

         int i,twoM,Nbig;
         double x0,theta1,theta2;
         double term1,term2,term3,sum;

// Note: w_{-n} = w_n

         Nbig=2*M+1;
         twoM=2*M;

         x0=cosh((mathfunc::coshinv(1/r))/(Nbig-1));

         sum=0;
         for (i=1; i <= M; i++)
         {
            theta1=i*PI/Nbig;
            term1=x0*cos(theta1);
            term2=Tn(term1,twoM);
            theta2=2*n*PI*i/Nbig;
            sum += term2*cos(theta2);
         }
         term3=1/r+2*sum;
         return (term3/Nbig);
      }
} // window namespace

