// ==========================================================================
// POTENTIALFUNCS stand-alone methods
// ==========================================================================
// Last modified on 5/22/05
// ==========================================================================

#include <iostream>
#include <iomanip>
#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "plot/plotfuncs.h"
#include "quantum/potentialfuncs.h"
#include "quantum/quantumarray.h"
#include "image/twoDarray.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;

namespace potentialfunc
{
   string get_potential_str(const Potential_type potential_type)
      {
         string potentialstr;
         
         if (potential_type==box)
         {
            potentialstr="box";
         }
         else if (potential_type==harmonic_osc)
         {
            potentialstr="harmonic_osc";
         }
         else if (potential_type==lambda_phi_4)
         {
            potentialstr="lambda_phi_4";
         }
         else if (potential_type==doublewell)
         {
            potentialstr="doublewell";
         }
         else if (potential_type==mathieu)
         {
            potentialstr="mathieu";
         }
         else if (potential_type==freeparticle)
         {
            potentialstr="freeparticle";
         }
         else if (potential_type==ramp)
         {
            potentialstr="ramp";
         }
         else if (potential_type==inverted_parabola)
         {
            potentialstr="inverted_parabola";
         }
         else if (potential_type==aperiodic_cosine)
         {
            potentialstr="aperiodic_cosine";
         }
         else if (potential_type==smooth_step)
         {
            potentialstr="smooth_step";
         }
         else if (potential_type==smooth_box)
         {
            potentialstr="smooth_box";
         }
         else if (potential_type==squid)
         {
            potentialstr="squid";
         }
         else if (potential_type==QFP)
         {
            potentialstr="QFP";
         }

         return potentialstr;
      }

// ---------------------------------------------------------------------
// Method evaluate_current_potential_params computes potential
// parameters as a function of the current time value.  This method is
// used to introduce continuous time dependence into the potential.
// Generally, we hold the potential parameters equal to some initial
// set of values for 0 < t < t1.  Then over a short time on the order
// of sigma, some subset of parameters are changed.  The new parameter
// values are held fixed for t1 < t < t2.  Finally, we typically reset
// the potential values back to their original values over a short
// interval on the order of sigma.

   void evaluate_current_potential_params(
      const Potential_type potential_type,
      double t1,double t2,double t,double potential_param[])
      {
         if (potential_type==potentialfunc::harmonic_osc)	// 1D
         {
            double timescale=1;
            double coeff_lo=1;            
            double coeff_hi=8;

            double sigma=0.75*timescale;
//            double sigma=1E-6*timescale;

            double arg1=(t-t1)/(sqrt(2.0)*sigma);
            double arg2=(t-t2)/(sqrt(2.0)*sigma);
            potential_param[0]=coeff_lo+ 
               (coeff_hi-coeff_lo)*0.5*(
                  mathfunc::error_function(arg1)-
                  mathfunc::error_function(arg2));
         }
         else if (potential_type==potentialfunc::smooth_box)		// 1D
         {
            double timescale=1;
            double sigma=0.000001*timescale;
            double coeff_lo=8;            
            double coeff_hi=5;            
            double arg1=(t-t1)/(sqrt(2.0)*sigma);
            double arg2=(t-t2)/(sqrt(2.0)*sigma);
            potential_param[1]=coeff_lo+ 
               (coeff_hi-coeff_lo)*0.5*(
                  mathfunc::error_function(arg1)-
                  mathfunc::error_function(arg2));
            
/*
            double sigma=0.000001*timescale;
            double coeff_lo=11;            
            double coeff_hi=5;            
            double arg1=(t-t1)/(sqrt(2.0)*sigma);
            double arg2=(t-t2)/(sqrt(2.0)*sigma);
            potential_param[1]=coeff_lo+ 
               (coeff_hi-coeff_lo)*0.5*(
                  mathfunc::error_function(arg1)-
                  mathfunc::error_function(arg2));
*/
         }
         else if (potential_type==potentialfunc::squid)		// 1D
         {

// Notional squid parameters:

            double timescale=1;

            double sigma1=0.8*timescale;
            double sigma2=1.6*timescale;
//            double sigma=1E-6*timescale;

            double Ejlo=0.4;
            double Ejhi=10;
            double arg1=(t-t1)/(sqrt(2.0)*sigma1);
            double arg2=(t-t2)/(sqrt(2.0)*sigma2);

            potential_param[0]=Ejhi+(Ejlo-Ejhi)*0.5*(
               mathfunc::error_function(arg1)-mathfunc::error_function(arg2));
            potential_param[1]=1*3.14159;
            potential_param[2]=PI;
         }
         else if (potential_type==potentialfunc::QFP)		// 2D
         {

// Notional QFP parameters:

            double EjoverEc=33;
            double phiQ=0;
            double betaQ=48.5;
            double betaE=3.03;

            potential_param[0]=EjoverEc;
            potential_param[1]=phiQ;
            potential_param[2]=betaQ;
            potential_param[4]=betaE;
   
            double alpha=potential_param[0];
            double timescale=1/sqrt(alpha);
            double sigma=0.25*timescale;
            double t1=1*timescale;		
            double phiE_init=0;
            double phiE_final=-PI;
            double arg1=(t-t1)/(sqrt(2.0)*sigma);
            potential_param[3]=phiE_init+
               (phiE_final-phiE_init)*0.5*(1+mathfunc::error_function(arg1));
         }
      }

// ---------------------------------------------------------------------
// Method fluctuate_potential_params adds random gaussian noise to one
// or more of the values within input array potential_param[].

   void fluctuate_potential_params(
      const Potential_type potential_type,long& seed,double potential_param[])
      {
         if (potential_type==potentialfunc::smooth_box)	// 1D
         {
            double fluctuation=0.5*numrec::gasdev(&seed);
            potential_param[1] += fluctuation;
         }
      }

// ---------------------------------------------------------------------
// Method plot_potential_param_time_dependence generates a metafile
// plot of the time dependence of some potential parameter.

   void plot_potential_param_time_dependence(
      const Potential_type potential_type,double tmin,double tmax,
      double t1,double t2,string imagedir,double potential_param[])
      {
         const int NBINS=513;
//   const double TINY_POSITIVE=-0.00001;
//   const double TINY_NEGATIVE=-TINY_POSITIVE;

         cout << "Plotting potential parameter time dependence:" << endl;
         outputfunc::newline();

         double dt=(tmax-tmin)/NBINS;
         twoDarray param_twoDarray(1,NBINS);
         for (int i=0; i<NBINS; i++)
         {
            double t=tmin+i*dt;
            evaluate_current_potential_params(
               potential_type,t1,t2,t,potential_param);
            if (potential_type==harmonic_osc || 
                potential_type==squid)
            {
               param_twoDarray.put(0,i,potential_param[0]);
            }
            else if (potential_type==QFP)
            {
               param_twoDarray.put(0,i,potential_param[3]);
            }
         }
         quantumarray param(tmin,dt,param_twoDarray);

         param.datafilenamestr=imagedir+"pot_param_time_dep";
         if (potential_type==harmonic_osc)
         {
            param.title=
               "Time dependence of harmonic oscillator potential coefficient";
            param.ylabel="alpha";
//      param.yplotminval=TINY_NEGATIVE;
         }
         else if (potential_type==squid)
         {
            param.title="Time dependence of SQUID potential parameter E^-j^n";
            param.ylabel="alpha";
//      param.yplotminval=TINY_NEGATIVE;
         }
         else if (potential_type==QFP)
         {
            param.title="Time dependence of QFP potential parameter phiE";
            param.ylabel="^g\152^u^-E^n";
//      param.yplotmaxval=TINY_POSITIVE;
         }
   
         param.xlabel="Time";
         param.xmin=tmin;
         param.xmax=tmax;

         filefunc::openfile(param.datafilenamestr+".meta",param.datastream);
         param.datafileheader();
         param.writedataarray();
         filefunc::closefile(param.datafilenamestr+".meta",param.datastream);
         filefunc::meta_to_jpeg(param.datafilenamestr);
         filefunc::gzip_file(param.datafilenamestr+".meta");
      }

// ---------------------------------------------------------------------
// Method set_xshift sets the shift in the metafile output's dependent
// variable for 1D potentials:

   double set_xshift(const Potential_type potential_type)
      {
         double xshift;
         if (potential_type==squid)
         {
            xshift=-PI;
         }         
         else
         {
            xshift=0;
         }
         return xshift;
      }
   
// ---------------------------------------------------------------------
// Method potential computes the potential along with its first and
// second derivatives for several different models.  We store the
// dimensionless coupling contant ( = ratio of characteristic
// potential to kinetic energies) for each system in
// potential_param[0].  It is important to recall that a system's
// characteristic time and length scales are set by fractional powers
// of this dimensionless coupling!
   
   void potential(
      bool time_dependent_potential,const Potential_type potential_type,
      long& seed,double t1,double t2,double t,double potential_param[],
      double x,double& V,double& dV,double& d2V)
      {
         double d3V,d4V,Kconst;
         potential(
            time_dependent_potential,potential_type,seed,t1,t2,t,
            potential_param,x,V,dV,d2V,d3V,d4V,Kconst,false);
      }

// ---------------------------------------------------------------------
// This overloaded version of method potential computes the 1D
// potential along with its 1st,2nd,3rd and 4th derivatives for
// several different models if boolean input parameter
// compute_high_derivs==true:

   void potential(
      bool time_dependent_potential,const Potential_type potential_type,
      long& seed,double t1,double t2,double t,double potential_param[],
      double x,double& V,double& dV,double& d2V,double& d3V,double& d4V,
      double& Kconst,bool compute_high_derivs)
      {
         if (potential_type==box)
         {
            double L=1;
            if (fabs(x) < L)
            {
               V=dV=d2V=0;
               if (compute_high_derivs) d3V=d4V=Kconst=0;
            }
            else
            {
               V=POSITIVEINFINITY;
               dV=d2V=0;
               if (compute_high_derivs) d3V=d4V=Kconst=0;
            }
            potential_param[0]=1;
         }
         else if (potential_type==harmonic_osc)
         {
            double coeff;
            if (time_dependent_potential)
            {
               evaluate_current_potential_params(
                  harmonic_osc,t1,t2,t,potential_param);
               coeff=potential_param[0];
            }
            else
            {
               coeff=1;      
//         coeff=10;      
//         coeff=100;      
//         coeff=1E4;      
//         coeff=1E5;      
//         coeff=1E6;      
            } // time_dependent_potential conditional

            V=coeff*sqr(x);
            dV=coeff*2*x;
            d2V=coeff*2;
            if (compute_high_derivs)
            {
               d3V=d4V=0;
               Kconst=coeff*2;
            }
            potential_param[0]=coeff;
         }
         else if (potential_type==doublewell)
         {
            double alpha=3.5;
            double constant=sqr(2*alpha-1)/(4*alpha);
            V=1-cos(x)-alpha*sqr(sin(x))+constant;
            dV=sin(x)-alpha*sin(2*x);
            d2V=cos(x)-2*alpha*cos(2*x);
            if (compute_high_derivs)
            {
               d3V=-sin(x)+4*alpha*sin(2*x);
               d4V=-cos(x)+8*alpha*cos(2*x);
            }
            potential_param[0]=alpha;
         }
         else if (potential_type==lambda_phi_4)
         {
            double alpha,beta,lambda;
//      alpha=3;
//      alpha=sqrt(3);
//      alpha=sqrt(1);

            beta=0;
//      beta=alpha;
            alpha=sqrt(1);
            lambda=100;

            V=lambda*sqr(sqr(x)-sqr(alpha))-sqr(beta*cos(PI*x/(2*alpha)));
            dV=lambda*4*x*(sqr(x)-sqr(alpha))+
               PI*sqr(beta)/(2*alpha)*sin(PI*x/alpha);
            d2V=lambda*(12*sqr(x)-4*sqr(alpha))+
               sqr(PI*beta/alpha)/2*cos(PI*x/alpha);
            if (compute_high_derivs)
            {
               d3V=lambda*24*x+sqr(PI*beta)/(2*sqr(alpha))*cos(PI*x/alpha);
               d4V=lambda*24-sqr(beta)/2*pow(PI/alpha,3)*sin(PI*x/alpha);
               Kconst=lambda*8*sqr(alpha);
            }
            potential_param[0]=lambda;
         }
         else if (potential_type==mathieu)
         {
//      Kcosine=1;
//      Kcosine=15;
            double Kcosine=100;
            V=Kcosine*(2+2*cos(2*x));
            dV=Kcosine*(-4*sin(2*x));
            d2V=Kcosine*-8*cos(2*x);
            if (compute_high_derivs)
            {
               d3V=Kcosine*16*sin(2*x);
               d4V=Kcosine*32*cos(2*x);
               Kconst=Kcosine*-8;
            }
            potential_param[0]=Kcosine;
         }
         else if (potential_type==freeparticle)
         {
            V=dV=d2V=0;
            if (compute_high_derivs) d3V=d4V=Kconst=0;
            potential_param[0]=1;
         }
         else if (potential_type==ramp)
         {
            V=-x;
            dV=-1;
            d2V=0;
            if (compute_high_derivs) d3V=d4V=Kconst=0;
            potential_param[0]=1;
         }
         else if (potential_type==inverted_parabola)
         {
            double coeff=1E4;
            V=-coeff*sqr(x);
            dV=-2*coeff*x;
            d2V=-2*coeff;
            if (compute_high_derivs)
            {
               d3V=d4V=0;
               Kconst=-2*coeff;
            }
            potential_param[0]=coeff;
         }
         else if (potential_type==aperiodic_cosine)
         {
//      Kcosine=1;
//      Kcosine=15;
            double Kcosine=1E3;
//    double kfreq=0.5;
            double kfreq=1.0;
            double Kparabola=0.02;
            double x0=1;
            V=Kcosine*((cos(kfreq*x)-1)+Kparabola*sqr(x-x0));
            dV=Kcosine*(-kfreq*sin(kfreq*x)+2*Kparabola*(x-x0));
            d2V=Kcosine*(-sqr(kfreq)*cos(kfreq*x)+2*Kparabola);
            if (compute_high_derivs)
            {
               d3V=Kcosine*pow(kfreq,3)*sin(kfreq*x);
               d4V=Kcosine*pow(kfreq,4)*cos(kfreq*x);
               Kconst=-Kcosine*sqr(kfreq);
            }
            potential_param[0]=Kcosine;
         }
         else if (potential_type==smooth_step)
         {
            double V0=1000;
            double mu=10;
            double sigma=0.1;
            double arg=(x-mu)/sigma;
            double normal=1/(sqrt(2*PI)*sigma)*exp(-sqr(arg)/2);
            V=V0/2*(1+mathfunc::error_function(arg/sqrt(2)));
            dV=V0*normal;
            d2V=-V0*(arg/sigma)*normal;
            if (compute_high_derivs)
            {
               d3V=-V0/sqr(sigma)*(1-sqr(arg))*normal;
               d4V=V0/pow(sigma,3)*arg*(3-sqr(arg))*normal;
               Kconst=0;
            }
            potential_param[0]=1;
         }
         else if (potential_type==smooth_box)
         {
//            double V0=1000;
//            double mu1=-8;
//            double mu2=8;
//            double sigma=0.1;
            double V0=10;
            double mu1,mu2;
            if (time_dependent_potential)
            {
//               evaluate_current_potential_params(
//			smooth_box,t1,t2,t,potential_param);
               potential_param[1]=8;
               fluctuate_potential_params(smooth_box,seed,potential_param);
               mu2=potential_param[1];
               fluctuate_potential_params(smooth_box,seed,potential_param);
               mu1=-potential_param[1];
            }
            else
            {
               mu2=8;
               mu1=-mu2;            
            }

            double sigma=0.5;
            double arg1=(x-mu1)/sigma;
            double arg2=(x-mu2)/sigma;
            double normal1=1/(sqrt(2*PI)*sigma)*exp(-sqr(arg1)/2);
            double normal2=1/(sqrt(2*PI)*sigma)*exp(-sqr(arg2)/2);
            V=V0/2*(2-mathfunc::error_function(arg1/sqrt(2))+
                    mathfunc::error_function(arg2/sqrt(2)));
            dV=V0*(-normal1+normal2);
            d2V=V0/sigma*(arg1*normal1-arg2*normal2);
            if (compute_high_derivs)
            {
               d3V=V0/sqr(sigma)*((1-sqr(arg1))*normal1-(1-sqr(arg2))
                                  *normal2);
               d4V=V0/pow(sigma,3)*(
                  -arg1*(3-sqr(arg1))*normal1+arg2*(3-sqr(arg2))*normal2);
               Kconst=0;
            }
            potential_param[0]=1;
         }
         else if (potential_type==squid)
         {
            double Ej,BetaL,x0;
            if (time_dependent_potential)
            {
               evaluate_current_potential_params(
                  squid,t1,t2,t,potential_param);
               Ej=potential_param[0];
               BetaL=potential_param[1];
               x0=potential_param[2];
            }
            else
            {
//      Ej=0.2;
//      Ej=0.3;
//               Ej=0.4;	// intermediate notional 1D squid value
//         Ej=1;
//      Ej=3;
//         Ej=5;
               Ej=10;	// initial and final notional 1D squid
//         Ej=4000;	// realistic 1D squid
//      BetaL=0.3*3.14159;
//      BetaL=0.5*3.14159;      
//      BetaL=0.8*3.14159;      
               BetaL=1*3.14159;      // notional 1D squid
//      BetaL=3*3.14159;
//      BetaL=5*3.14159;
//         BetaL=1.84;	// realistic 1D squid
               x0=PI;
      
               potential_param[0]=Ej;
               potential_param[1]=BetaL;
               potential_param[2]=x0;
            } // time_dependent_potential conditional

//      cout << "Ej = " << Ej << " BetaL = " << BetaL << " x0 = " << x0
//           << endl;

            V=Ej*(1-cos(x)+sqr(x-x0)/(2*BetaL));
            dV=Ej*(sin(x)+(x-x0)/BetaL);
            d2V=Ej*(cos(x)+1/BetaL);
            if (compute_high_derivs)
            {
               d3V=-Ej*sin(x);
               d4V=-Ej*cos(x);
               Kconst=0;	// fix this later
            }
         } // potential_type conditional
      }

// ---------------------------------------------------------------------
// This overloaded version of method potential computes the 2D
// potential along with its 1st derivative, laplacian and double
// laplacian for several different models:

   void potential(
      bool time_dependent_potential,const Potential_type potential_type,
      long& seed,double t1,double t2,double t,double potential_param[],
      double x,double y,double xshift,double yshift,
      double& V,double& dVdx,double& dVdy,double& laplacianV,
      double& doublelaplacianV,double& Kconst,double& K12)
      {
         double Cx,Cy;

// QFP parameter set #2 as of April 2002:
      
         double EjoverEc=33;
//   double EjoverEc=3.3E6;
         double phiQ=0;
         double betaQ=48.5;
//   double phiE=0;
         double phiE=potential_param[3];
         double betaE=3.03;
   
         if (potential_type==potentialfunc::harmonic_osc)
         {
            Cx=1;
            Cy=1;
//      Cx=1E6;
//      Cy=1E6;
//      Cx=2*EjoverEc*(1/(betaE+2*betaQ)+1);
//      Cy=2*EjoverEc*(1/betaE+1);

            V=Cx*sqr(x)+Cy*sqr(y);
            dVdx=2*Cx*x;
            dVdy=2*Cy*y;
            laplacianV=2*Cx+2*Cy;
            doublelaplacianV=0;
            Kconst=2*Cx+2*Cy;
            K12=0;
         }
         else if (potential_type==potentialfunc::doublewell)
         {
// double well trap parameters:

//   const double alpha=0;
//   const double alpha=0.5;
            const double alpha=1;
//   const double alpha=1.5;

//   const double f=0.25;
            const double f=0.5;
//   const double f=0.75;

            double cosx=cos(x);
            double sinx=sin(x);
            double cosy=cos(y);
            double siny=sin(y);
            double costerm=cos(2*PI*f+2*y);
            V=2*(1-cosx*cosy)+alpha*(1-costerm);
            dVdx=2*sinx*cosy;
            dVdy=2*cosx*siny+2*alpha*sin(2*PI*f+2*y);
            laplacianV=4*(cosx*cosy+alpha*costerm);
            doublelaplacianV=-8*(cosx*cosy+2*alpha*costerm);
            Kconst=4*(1+alpha*cos(2*PI*f));
            K12=0;
            potential_param[0]=2;
         }
         else if (potential_type==potentialfunc::mathieu)
         {
            double cos2x=cos(2*x);
            double sin2x=sin(2*x);
            double cos2y=cos(2*y);
            double sin2y=sin(2*y);
            V=4*(1+cos2x)*(1+cos2y);
            dVdx=-8*sin2x*(1+cos2y);
            dVdy=-8*(1+cos2x)*sin2y;
            laplacianV=-16*(cos2x+cos2y+2*cos2x*cos2y);
            doublelaplacianV=64*(cos2x+cos2y+4*cos2x*cos2y);
            Kconst=-64;
            K12=0;

//      V=4+2*cos2x+2*cos2y;
//      dVdx=-8*cos2x;
//      dVdy=-8*cos2y;
//      laplacianV=-8*(cos2x+cos2y);
//      doublelaplacianV=32*(cos2x+cos2y);
//      Kconst=-16;
//      K12=0;
         }
         else if (potential_type==potentialfunc::QFP)
         {
            if (time_dependent_potential)
            {
               potentialfunc::evaluate_current_potential_params(
                  QFP,t1,t2,t,potential_param);
               EjoverEc=potential_param[0];
               phiQ=potential_param[1];
               betaQ=potential_param[2];
               phiE=potential_param[3];
               betaE=potential_param[4];
            }
            else
            {

// Store potential parameters into potential_param variables for
// metafile output purposes:

               potential_param[0]=EjoverEc;
               potential_param[1]=phiQ;
               potential_param[2]=betaQ;
               potential_param[3]=phiE;
               potential_param[4]=betaE;
            }

// Add constant to potential in order to ensure min(V) = 0 :
      
            V=2*EjoverEc*(sqr(x-xshift-phiQ)/(betaE+2*betaQ)+
                          sqr(y-yshift-phiE)/betaE
                          -2*cos(x-xshift)*cos(y-yshift)+2);
            dVdx=4*EjoverEc*((x-xshift-phiQ)/(betaE+2*betaQ)+
                             sin(x-xshift)*cos(y-yshift));
            dVdy=4*EjoverEc*((y-yshift-phiE)/betaE+
                             cos(x-xshift)*sin(y-yshift));
            laplacianV=4*EjoverEc*(1/betaE+1/(betaE+2*betaQ)+
                                   2*cos(x-xshift)*cos(y-yshift));
            doublelaplacianV=-16*EjoverEc*cos(x-xshift)*cos(y-yshift);
            Kconst=4*EjoverEc*(1/betaE+1/(betaE+2*betaQ)+2);
            K12=0;
         } // potential_type conditional
      }

} // potentialfunc namespace





