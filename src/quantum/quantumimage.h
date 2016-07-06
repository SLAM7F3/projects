// ==========================================================================
// Header file for QUANTUMIMAGE class
// ==========================================================================
// Last modified on 5/22/05
// ==========================================================================

#ifndef QUANTUMIMAGE_H
#define QUANTUMIMAGE_H

#include "math/complex.h"
#include "quantum/quantum_wavefunction.h"

class quantumimage: public quantum_wavefunction
{
  private:

   void docopy(const quantumimage& q);

  public:

   int nxbins,nybins;
   int nxbins_to_display,nybins_to_display;
   double xhi,xlo,deltax;
   double yhi,ylo,deltay;
   double kx_hi,kx_lo,delta_kx;
   double ky_hi,ky_lo,delta_ky;
   double xperiod,yperiod;	// Potential period in x/y directions

// ---------------------------------------------------------------------
// Constructor and destructor functions:
// ---------------------------------------------------------------------

   quantumimage(void);
   quantumimage(const quantumimage& q);
   virtual ~quantumimage();
   quantumimage& operator= (const quantumimage& q);

// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------

// FFT member functions:

   void fouriertransform(
      const complex value_in[nxbins_max][nybins_max],
      complex tilde_out[nxbins_max][nybins_max]);
   void inversefouriertransform(
      const complex tilde_in[nxbins_max][nybins_max],
      complex value_out[nxbins_max][nybins_max]);

// Meta file member functions:

   void label_axes(double& xmin,double& xmax,double& ymin,double& ymax);
   void header_end(double xmin,double xmax,double ymin,double ymax);
   void append_dynamic_colortable(
      double height,double width,double legloc_x,double legloc_y);
   void append_colortable(
      double height,double width,double legloc_x,double legloc_y);
   void insert_colortable_head();
   void insert_colortable_foot();
   void insert_nowhite_dynamic_colortable(
      double height,double width,double legloc_x,double legloc_y);
   void insert_nowhite_colortable(
      double height,double width,double legloc_x,double legloc_y);
   void potential_header(std::string plot_type);
   void singletfile_header(int imagenumber);
   void doubletfile_header(int doublet_pair_member);
   void tripletfile_header(int triplet_pair_member);
   void potential_footer();
   void singletfile_footer(double E);
   void doubletfile_footer(int imagenumber,double E);
   void tripletfile_footer(int imagenumber,double E);
};

#endif // quantumimage.h






