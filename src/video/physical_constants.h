/********************************************************************
 *
 *
 * Name: mks.h
 *
 *
 * Author: Joseph Adams
 *
 * Description:
 *   Support for units and constants
 * --------------------------------------------------------------
 *    $Revision: 1.1 $
 * ---------------------------------------------------------------
 *    $Log: physical_constants.h,v $
 *    Revision 1.1  2004/06/23 21:26:18  jadams
 *    *** empty log message ***
 *
 *    Revision 1.1.1.1  2003/06/18 18:02:45  jadams
 *    Starting revamped dll, basically working in the plane
 *
 *
 *
 * 1     6/11/03 4:25p Jadams
 * Rewrite of alirt dll
 * with thread manager and roadrunner class
 *
 * 3     6/03/03 5:29p Jadams
 * Version 74 didn't check in ok try agin (this will be 75)
 *
 * 2     6/02/03 10:36a Jadams
 * Data file based simulator added
 *
 * 1     6/02/03 10:33a Jadams
 * Ali file support added
 *
 * 1     5/19/03 6:13p Jadams
 *    Revision 1.1.1.1  2003/04/22 15:16:58  jadams
 *    Alirt Dll with fligh management support
 *
 *
 *
 * 1     4/22/03 10:08a Jadams
 *    Revision 1.1.1.1  2003/03/27 16:54:03  jadams
 *
 *
 *
 *    Revision 1.1.1.1  2003/02/27 21:59:23  jadams
 *
 *
 *
 *
 *
 **********************************************************************/
#ifndef _JSA_MKS_
#define _JSA_MKS_

#include <math.h>

//define base mksa

namespace SI
{

    #ifndef M_PI
    #define M_PI 3.1415926535897932384626433832795
    #endif

    const double pi=M_PI;
    const double two_pi=2*M_PI;

    const double  m_ =1;
    const double  kg_ = 1;
    const double  s_   = 1;
    const double  A_   = 1;
    const double  K_   =1 ;
    const double  mol_ =1 ;
    const double  cd_  = 1 ;
    const double  R_  =  1;

    //coloumb is derived in MKSA

    //define time
    const double min_ =60*s_;
    const double h_   =3600*s_;
    const double d_   =86400*s_;
    const double yr_  =3.1558e7*s_;               //sidereal year

    const double ms_= 1e-3*s_;
    const double us_= 1e-6*s_;
    const double ns_= 1e-9*s_;
    const double ps_= 1e-12*s_;

    //lengths
    const double km_=1e3*m_;
    const double cm_=1e-2*m_;
    const double mm_=1e-3*m_;
    const double um_=1e-6*m_;
    const double nm_=1e-9*m_;
    const double pm_=1e-12*m_;
    const double fm_=1e-15*m_;

    const double in_=2.54e-2*m_;
    const double mil_=2.54e-5*m_;
    const double ft_=3.0480e-1*m_;
    const double yd_=9.144e-1*m_;
    const double mi_=1.6093e3*m_;

    //masses
    const double g_ = 1e-3 * kg_;

    //volumes
    const double cc_ = pow(cm_,3);

    //temperatures
    const double mK_=1e-3*K_;
    const double uK_=1e-6*K_;
    const double nK_=1e-9*K_;

    //angle

    const double mR_=1e-3*R_;
    const double uR_=1e-6*R_;
    const double deg_=((2*pi)/360)*R_;
    const double mdeg_=1e-3*deg_;
    const double udeg_=1e-6*deg_;

    //%%we should add in min_, sec_ and arcsec_

    //derived mksa
    const double  Hz_=1/s_;
    const double   N_=m_*kg_/(s_*s_);
    const double  Pa_=N_/(m_*m_);
    const double   J_=N_*m_;
    const double   W_=J_/s_;
    const double   C_=A_*s_;
    const double   V_=W_/A_;
    const double   F_=C_/V_;
    const double ohm_=V_/A_;
    const double   S_=A_/V_;
    const double  Wb_=V_*s_;
    const double   T_=Wb_/(m_*m_);
    const double   H_=Wb_/A_;
    const double  Bq_=1/s_;
    const double poise_=.1*kg_/(m_*s_);
    const double upoise_=1e-6*poise_;

    //mksa outside SI
    const double   l_=1e-3*(pow(m_,3));
    const double  eV_=1.60218e-19*J_;
    const double   u_=1.66054e-27*kg_;
    const double  au_=1.49598e11*m_;
    const double  AA_=1e-10*m_;
    const double  Ci_=3.7e10*Bq_;
    const double bar_=1e5*Pa_;
    const double erg_=1e-7*J_;

    const double meV_=1e-3*eV_;
    const double keV_=1e3*eV_;
    const double MeV_=1e6*eV_;
    const double GeV_=1e9*eV_;

    const double mV_=1e-3*V_;
    const double uV_=1e-6*V_;
    const double nV_=1e-9*V_;
    const double pV_=1e-12*V_;

    const double mohm_=1e-3*ohm_;
    const double uohm_=1e-6*ohm_;

    const double mA_=1e-3*A_;
    const double uA_=1e-6*A_;
    const double nA_=1e-9*A_;

    const double mW_=1e-3*W_;
    const double uW_=1e-6*W_;
    const double nW_=1e-9*W_;

    const double mJ_=1e-3*J_;
    const double uJ_=1e-6*J_;
    const double nJ_=1e-9*J_;

    const double mH_=1e-3*H_;
    const double uH_=1e-6*H_;
    const double nH_=1e-9*H_;

    const double mHz_=1e-3*Hz_;
    const double kHz_=1e3*Hz_;
    const double MHz_=1e6*Hz_;
    const double GHz_=1e9*Hz_;

    //'acronym mksa'
    const double kph_= km_/h_;
    const double mph_= mi_/h_;
    const double cfm_= pow(ft_,3)/min_;
    const double lps_= l_/s_;

    const double mCi_=1e-3*Ci_;
    const double uCi_=1e-6*Ci_;
    const double nCi_=1e-9*Ci_;

    //%%%pressure units (yech)
    //%%% please note that beatiful american units such as inHg vacuum and
    //%%% psig are not implemented in this library
    const double atm_=101325*Pa_;
    //	 const double bar_=100000*Pa_;
    const double psi_=6894.757*Pa_;
    const double torr_=133.3223*Pa_;
    const double mmHg_=torr_;
    const double inHg_=25.4*mmHg_;
    const double inH20=248.84*Pa_;

    //#include <gsl/gsl_const_mks.h>
    //#include <gsl/gsl_math.h>

    //%%%%%% these are physical constanst
    //%%%%%% i.e. the table of galatic uneccesaries
    namespace TGU
    {

        const double c=299792458*m_/s_;
        const double kb=1.38e-23*J_/K_;
        const double planck=6.63e-34*J_*s_;
        const double hbar=1.05e-34*J_*s_;
        const double e=1.6e-19*C_;
        const double me=9.11e-31*kg_;
        const double mu_o=1.26e-6*H_/m_;
        const double phi_o=2.06e-15*T_*pow(m_,m_);// flux quantum
        const double uphi_o=1e-6*phi_o;           // flux quantum*1e-6
        const double Na=6.01e23*1/mol_;
                                                  // stefan_boltzmann constant
        const double sigma=5.67e-8*W_/(pow(m_,2)*pow(K_,4));
        const double m3=5.007e-24*g_;             //mass of 3He atom
        const double m4=6.646e-24*g_;             //mass of 3He atom
        const double R=8.314e7*erg_/(K_*mol_);    //Gas constant
        const double eo=8.85e-12*F_/m_;
        const double rho4=0.145*g_/cc_;           // mass density of 4He
        const double n4=rho4/m4;
        const double ao=0.529177*AA_;             //bohr radius
        const double re=2.8179*fm_;               //classical radius of electron
        const double compton=1.4263*pm_;          //compton wavelnegth
        const double rydberg=1.097373*1/m_;       //Rydberg constant for infinite mass nucleus
        const double mu_b=0.5788e-10*MeV_/T_;     //Bohr magneton
        const double mu_e=1.001159*mu_b;          //electron magnetic moment
        const double mu_n=3.15245e-14*MeV_/T_;    //Nuclear magneton
        const double mu_p=2.792*mu_n;             //proton magnetic moment
        const double fine_structure=1/137.03604;  // fine structure constant (alpha)
                                                  //graviational constant
        const double G = 6.6720e-11*pow(m_,3)/(kg_*pow(s_,2));

    }                                             // end of namespace TGU

}                                                 // end of namespace SI
#endif
