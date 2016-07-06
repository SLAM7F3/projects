// ==========================================================================
// Quantumarray class member function definitions
// ==========================================================================
// Last modified on 5/22/05
// ==========================================================================

#include "math/constants.h"
#include "quantum/quantumarray.h"
#include "image/TwoDarray.h"

using std::endl;
using std::ios;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

quantumarray::quantumarray(void)
{
}

// Copy constructor

quantumarray::quantumarray(const quantumarray& q):
   dataarray(q)
{
   docopy(q);
}

quantumarray::quantumarray(double minx,double delta,twoDarray& Tdata):
   dataarray(minx,delta,Tdata)
{
}

quantumarray::~quantumarray()
{
}

// ---------------------------------------------------------------------
void quantumarray::docopy(const quantumarray& q)
{
}	

// Overload = operator:

quantumarray& quantumarray::operator= (const quantumarray& q)
{
   if (this==&q) return *this;
   dataarray::operator=(q);
   docopy(q);
   return *this;
}

// ==========================================================================
// Quantumarray manipulation member functions
// ==========================================================================

// Member function write_singlet_data writes to meta file output
// potential energy, wavefunction probability density, energy
// eigenvalues, and/or energy eigenfunctions in singlet plot format
// depending upon the setting of the complex_plot_type flag.

void quantumarray::write_singlet_data(
   int n_energystates,double xshift,
   quantum::Complex_plot_type complex_plot_type)
{
   write_singlet_data(n_energystates,xshift,complex_plot_type,"");
}

void quantumarray::write_singlet_data(
   int n_energystates,double xshift,
   quantum::Complex_plot_type complex_plot_type,string potential_type)
{
   datastream.setf(ios::fixed);
   datastream.setf(ios::showpoint);  
//   datastream.precision(NPRECISION);
   datastream.precision(10);

// Allow user to add extra lines into datastream by hand before the
// data stored within quantumarray is written out to file:

   int i=0;
   while(i < N_EXTRAINFO_LINES && extraline[i] != "")
   {
      datastream << extraline[i] << endl;
      i++;
   }
   datastream << endl;

   if (complex_plot_type==quantum::sqd_amp)
   {
      
// Plot wavefunction probability density:

      datastream << "curve color red" << endl;
      if (thickness != 1) datastream << "thick " << thickness << endl;
      for (int j=0; j<npoints; j++)
      {

// Skip over any points whose absolute value >= POSITIVEINFINITY

         if (T_ptr->get(0,j) < POSITIVEINFINITY)
         {
            datastream << X[j]+xshift << "\t\t" 
                       << xnorm*T_ptr->get(0,j) << endl;
         }
      }
      datastream << endl;

// Plot non-trivial potentials:

      if (potential_type != "freeparticle")
      {
         datastream << "curve style 1 color blue" << endl << endl;
         if (thickness != 1) datastream << "thick " << thickness << endl;
         for (int j=0; j<npoints; j++)
         {

// Skip over any points where potential's absolute value >=
// POSITIVEINFINITY

            if (T_ptr->get(2,j) < POSITIVEINFINITY)
            {
               datastream << X[j]+xshift << "\t\t" 
                          << xnorm*T_ptr->get(2,j) << endl;
            }
         } // loop over index j
      }
   }
   else if (complex_plot_type==quantum::energy_spectrum)
   {

// Plot potential:

      datastream << "curve style 1 color black" << endl << endl;
      if (thickness != 1) datastream << "thick " << thickness << endl;
      for (int j=0; j<npoints; j++)
      {

// Skip over any points where potential's absolute value >=
// POSITIVEINFINITY

         if (fabs(T_ptr->get(0,j)) < POSITIVEINFINITY)
         {
            datastream << X[j]+xshift << "\t\t" 
                       << T_ptr->get(0,j) << endl;
         }
      } // loop over index j

// Plot energy eigenvalues as horizontal bars through potential:

      for (int m=0; m<n_energystates; m++)
      {
         datastream << endl;
         datastream << "curve thick 3 color " << colorfunc::getcolor(m) 
                    << endl;
         for (int j=0; j<npoints; j++)
         {
            datastream << X[j]+xshift << "\t\t" 
                       << T_ptr->get(m+1,j) << endl;
         }
      } // loop over energy eigenstates
   }
   else if (complex_plot_type==quantum::real_imag)
   {

// Plot energy eigenfunctions:

      for (int m=0; m<n_energystates; m++)
      {
         datastream << endl;
         datastream << "curve thick 2 color " << colorfunc::getcolor(m) 
                    << endl;
         for (int j=0; j<npoints; j++)
         {
            datastream << X[j]+xshift << "\t\t" 
                       << T_ptr->get(m,j) << endl;
         }
      } // loop over energy eigenstates
   } // complex_plot_type conditional
}

// ---------------------------------------------------------------------
// Member function write_doublet_data writes to meta file output the
// first two curves within the current quantumarray.  This subroutine
// is specifically called by member function
// quantum_1Dwavefunction::plot_wavefunction().

void quantumarray::write_doublet_data(
   int doublet_pair_member,double xshift,
   quantum::Domain_name domain_name,
   quantum::Complex_plot_type complex_plot_type)
{
   datastream.setf(ios::fixed);
   datastream.setf(ios::showpoint);  
   datastream.precision(NPRECISION);

// Allow user to add extra lines into datastream by hand before the
// data stored within quantumarray is written out to file:

   int i=0;
   while(i < N_EXTRAINFO_LINES && extraline[i] != "")
   {
      datastream << extraline[i] << endl;
      i++;
   }
   datastream << endl;

   datastream << "curve" << endl;
   if (thickness != 1) datastream << "thick " << thickness << endl;
   datastream << "color "+colorfunc::getcolor(doublet_pair_member);
   datastream << endl;
      
   for (int j=0; j<npoints; j++)
   {

// Skip over any points whose absolute value >= POSITIVEINFINITY

      double currvalue=T_ptr->get(doublet_pair_member,j);
      if (fabs(currvalue) < POSITIVEINFINITY)
      {
         datastream << X[j]+xshift << "\t\t" 
                    << xnorm*currvalue << endl;
      }
   } // loop over index j

// Plot system energy or potential energy:

   if (domain_name==quantum::position_space)
   {
      if ((complex_plot_type==quantum::energy_prob || 
           complex_plot_type==quantum::energy_real) 
          && doublet_pair_member==0)
      {
         datastream << endl;
         datastream << "curve color cyan thick 3" << endl;
         datastream << "-1000 " << xnorm*T_ptr->get(2,0) << endl;
         datastream << "1000 " << xnorm*T_ptr->get(2,0) << endl;
      }
      else if ((complex_plot_type==quantum::mag_phase && 
                doublet_pair_member==0) ||
               complex_plot_type==quantum::real_imag ||
               (complex_plot_type==quantum::posn_momentum && 
                doublet_pair_member==0))
      {
         datastream << endl;
         datastream << "curve style 1 color blgr" << endl;
         if (thickness != 1) datastream << "thick " << thickness << endl;
      
         for (int j=0; j<npoints; j++)
         {
            if (fabs(T_ptr->get(2,j)) < POSITIVEINFINITY)
            {
               datastream << X[j]+xshift << "\t\t" 
                          << xnorm*T_ptr->get(2,j) << endl;
            }
         } // loop over index j
      }
   } // domain_name==quantum::position_space conditional
}

// ---------------------------------------------------------------------
// This overloaded version of member function write_doublet_data
// writes to meta file output energy eigenvalues and eigenfunctions
// contained within the current quantumarray.  This subroutine is
// specifically called by member function
// quantum_1Dwavefunction::plot_spectrum_and_efuncs().

void quantumarray::write_doublet_data(
   int doublet_pair_member,int n_energystates,double xshift)
{
   int i,j,m;

   datastream.setf(ios::fixed);
   datastream.setf(ios::showpoint);  
   datastream.precision(NPRECISION);

// Allow user to add extra lines into datastream by hand before the
// data stored within quantumarray is written out to file:

   i=0;
   while(i < N_EXTRAINFO_LINES && extraline[i] != "")
   {
      datastream << extraline[i] << endl;
      i++;
   }
   datastream << endl;

   if (doublet_pair_member==0)
   {
      
// Plot potential:

      datastream << "curve style 1 color black" << endl << endl;
      if (thickness != 1) datastream << "thick " << thickness << endl;
      for (j=0; j<npoints; j++)
      {

// Skip over any points where potential's absolute value >=
// POSITIVEINFINITY

         if (fabs(T_ptr->get(0,j)) < POSITIVEINFINITY)
         {
            datastream << X[j]+xshift << "\t\t" 
                       << T_ptr->get(0,j) << endl;
         }
      } // loop over index j

// Plot energy eigenvalues as horizontal bars through potential:

      for (m=0; m<n_energystates; m++)
      {
         datastream << endl;
         datastream << "curve thick 3 color " << colorfunc::getcolor(m) 
                    << endl;
         for (j=0; j<npoints; j++)
         {
            datastream << X[j]+xshift << "\t\t" 
                       << T_ptr->get(m+1,j) << endl;
         }
      } // loop over energy eigenstates
   }
   else if (doublet_pair_member==1)
   {

// Plot energy eigenfunctions:

      for (m=0; m<n_energystates; m++)
      {
         datastream << endl;
         datastream << "curve thick 2 color " << colorfunc::getcolor(m) 
                    << endl;
         for (j=0; j<npoints; j++)
         {
            datastream << X[j]+xshift << "\t\t" 
                       << T_ptr->get(n_energystates+1+m,j) << endl;
         }
      } // loop over energy eigenstates
   }
}
