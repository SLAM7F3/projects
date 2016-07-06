// ==========================================================================
// Quantum_2Dwavefunction class member function definitions
// ==========================================================================
// Last modified on 5/22/05
// ==========================================================================

#include "math/complex.h"
#include "general/filefuncs.h"
#include "general/genfuncs_complex.h"
#include "math/mathfuncs.h"
#include "math/mypolynomial.h"
#include "general/outputfuncs.h"
#include "plot/plotfuncs.h"
#include "quantum/quantum_2Dwavefunction.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::ios;
using std::string;
using std::ostream;
using std::ofstream;
using std::ifstream;
using std::cout;
using std::endl;

// const int quantum_2Dwavefunction::Nmax_energystates=3;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions
// ---------------------------------------------------------------------

void quantum_2Dwavefunction::allocate_member_objects()
{
   prev_arg=new double[nxbins_max][nybins_max];
   curr_arg=new double[nxbins_max][nybins_max];
   value=new complex[nxbins_max][nybins_max];
   tilde=new complex[nxbins_max][nybins_max];
   energy_eigenstate=new 
      complex[Nmax_energystates][Nmax_energystates][nxbins_max][nybins_max];
}

void quantum_2Dwavefunction::initialize_member_objects()
{
   ndims=2;
   xtic=ytic=0;
}

quantum_2Dwavefunction::quantum_2Dwavefunction()
{
   allocate_member_objects();
   initialize_member_objects();
   clear_wavefunction();
}

// When an object is initialized with an object of the same type, the
// following function is called.  This next constructor is apparently
// called whenever a function is passed an object as an argument.  As
// James Wanken reminded us on 10/19/00, any time we have dynamic
// member variables within a class, we must first be sure to allocate
// memory space for them first before we call the docopy command below!

quantum_2Dwavefunction::quantum_2Dwavefunction(
   const quantum_2Dwavefunction& q):
   quantumimage(q)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(q);
}

quantum_2Dwavefunction::~quantum_2Dwavefunction()
{
   delete [] prev_arg;
   delete [] curr_arg;
   delete [] value;
   delete [] tilde;
   delete [] energy_eigenstate;
}

// ---------------------------------------------------------------------
void quantum_2Dwavefunction::docopy(const quantum_2Dwavefunction& q)
{
   x=q.x;
   y=q.y;
   
   for (int i=0; i<nxbins_max; i++)
   {
      for (int j=0; j<nybins_max; j++)
      {
         curr_arg[i][j]=q.curr_arg[i][j];
         prev_arg[i][j]=q.prev_arg[i][j];
         value[i][j]=q.value[i][j];
         tilde[i][j]=q.tilde[i][j];
         for (int k=0; k<Nmax_energystates; k++)
         {
            for (int l=0; l<Nmax_energystates; l++)
            {
               energy_eigenstate[k][l][i][j]=q.energy_eigenstate[k][l][i][j];
            }
         }
      } // loop over index j
   } // loop over index i
}	

// Overload = operator:

quantum_2Dwavefunction& quantum_2Dwavefunction::operator= 
(const quantum_2Dwavefunction& q)
{
   if (this==&q) return *this;
   quantumimage::operator=(q);
   docopy(q);
   return *this;
}

// =====================================================================
// Meta file member functions:
// =====================================================================

// Member function writeimagedata writes to meta file output the
// contents of the 2D intensity array.  Columns of pixels are either
// added or subtracted so that the horizontal extent of the image
// approximately equals its vertical extent:

void quantum_2Dwavefunction::writeimagedata()
{
   bool pixel_inside_inner_display_frac_region=true;
   
   imagestream.precision(2);
   imagestream.setf(ios::scientific);
   imagestream.setf(ios::showpoint);  

// Recall ylo corresponds to index 0, while yhi corresponds to index
// nybins-1.  In order for the image to appear right-side up, we need
// to decrement j from nybins-1 down to 0:

   double curr_value=0;
   for (int j=nybins-1; j>=0; j--)  
   {
      for (int i=0; i<nxbins; i++)
      {
         if ((abs(i-nxbins/2) <= (nxbins_to_display-1)/2) &&
             (abs(j-nybins/2) <= (nybins_to_display-1)/2))
         {
            pixel_inside_inner_display_frac_region=true;
         }
         else
         {
            pixel_inside_inner_display_frac_region=false;
         }

         if (pixel_inside_inner_display_frac_region)
         {
            if (domain_name==quantum::position_space)
            {
               if (complex_plot_type==quantum::sqd_amp)
               {
                  curr_value=sqr(value[i][j].getmod());
               }
            }
            else if (domain_name==quantum::momentum_space)
            {
               if (complex_plot_type==quantum::sqd_amp)
               {
                  curr_value=sqr(tilde[i][j].getmod());
               }
            }
         
// Set any points whose value >= POSITIVEINFINITY equal to 1000 and
// any points whose values <= NEGATIVEINFINITY equal to -1000:

            if (fabs(curr_value) < POSITIVEINFINITY)
            {
               imagestream << curr_value << " ";
            }
            else if (curr_value >= POSITIVEINFINITY)
            {
               imagestream << " +1000 ";
            }
            else if (curr_value <= NEGATIVEINFINITY)
            {
               imagestream << " -1000 ";
            }
         
// According to Iva, each line of output in an metafile image must
// contain no more than 2000 characters.  However, the metaplot parser
// ignores all carriage returns.  So we simply need to insert enough
// carriage returns in the output so that there are never more than
// 2000 characters on any given line:

            if ((i+1)%10==0) imagestream << endl;
         
         } // check on whether (i,j) pixel lies within inner display_frac
         //  fraction of image
         
      } // loop over index i

      if (pixel_inside_inner_display_frac_region)
      {
         imagestream << endl;
         imagestream << endl;
      }
   }  // loop over index j
}

// ---------------------------------------------------------------------
// Member function writeimagedata writes out to a meta file the
// contents of the 2D intensity array.  Columns of pixels are either
// added or subtracted so that the cross range extent of the image
// approximately equals its range extent:

void quantum_2Dwavefunction::writeimagedata(int doublet_pair_member)
{
   bool pixel_inside_inner_display_frac_region=true;
   double curr_value=0;
   double x,y;

   imagestream.precision(2);
   imagestream.setf(ios::scientific);
   imagestream.setf(ios::showpoint);  

// Recall ylo corresponds to index 0, while yhi corresponds to index
// nybins-1.  In order for the image to appear right-side up, we need
// to decrement j from nybins-1 down to 0:

   for (int j=nybins-1; j>=0; j--)  
   {
      for (int i=0; i<nxbins; i++)
      {
         if ((abs(i-nxbins/2) <= (nxbins_to_display-1)/2) &&
             (abs(j-nybins/2) <= (nybins_to_display-1)/2))
         {
            pixel_inside_inner_display_frac_region=true;
         }
         else
         {
            pixel_inside_inner_display_frac_region=false;
         }

         if (pixel_inside_inner_display_frac_region)
         {
            if (doublet_pair_member==0)
            {
               if (domain_name==quantum::position_space)
               {
                  if (complex_plot_type==quantum::mag_phase)
                  {
                     curr_value=value[i][j].getmod();
                  }
                  else if (complex_plot_type==quantum::real_imag)
                  {
                     curr_value=value[i][j].getreal();
                  }
                  else if (complex_plot_type==quantum::energy_prob)
                  {
                     x=xlo+i*deltax;
                     y=ylo+j*deltay;
                     curr_value=V_displayfactor*potential(x,y);
                  }
               }
               else if (domain_name==quantum::momentum_space)
               {
                  if (complex_plot_type==quantum::mag_phase)
                  {
                     curr_value=tilde[i][j].getmod();
                  }
                  else if (complex_plot_type==quantum::real_imag)
                  {
                     curr_value=tilde[i][j].getreal();
                  }
               }
            }
            else if (doublet_pair_member==1)
            {
               if (domain_name==quantum::position_space)
               {
                  if (complex_plot_type==quantum::mag_phase)
                  {
                     curr_value=curr_arg[i][j];
                  }
                  else if (complex_plot_type==quantum::real_imag)
                  {
                     curr_value=value[i][j].getimag();
                  }
                  else if (complex_plot_type==quantum::energy_prob)
                  {
                     curr_value=sqr(value[i][j].getmod());
                  }
               }
               else if (domain_name==quantum::momentum_space)
               {
                  if (complex_plot_type==quantum::mag_phase)
                  {
                     curr_value=tilde[i][j].getarg();
                  }
                  else if (complex_plot_type==quantum::real_imag)
                  {
                     curr_value=tilde[i][j].getimag();
                  }
               }
            }
         
// Set any points whose value >= POSITIVEINFINITY equal to 1000 and
// any points whose values <= NEGATIVEINFINITY equal to -1000:

            if (fabs(curr_value) < POSITIVEINFINITY)
            {
               imagestream << curr_value << " ";
            }
            else if (curr_value >= POSITIVEINFINITY)
            {
               imagestream << " +1000 ";
            }
            else if (curr_value <= NEGATIVEINFINITY)
            {
               imagestream << " -1000 ";
            }
         
// According to Iva, each line of output in an metafile image must
// contain no more than 2000 characters.  However, the metaplot parser
// ignores all carriage returns.  So we simply need to insert enough
// carriage returns in the output so that there are never more than
// 2000 characters on any given line:

            if ((i+1)%10==0) imagestream << endl;
            
         } // check on whether (i,j) pixel lies within inner display_frac
         //  fraction of image
         
      } // loop over index i

      if (pixel_inside_inner_display_frac_region)
      {
         imagestream << endl;
         imagestream << endl;
      }
   }  // loop over index j
}

// ---------------------------------------------------------------------
// Member function writeimagedata writes out to a meta file the
// contents of the 2D intensity array.  Columns of pixels are either
// added or subtracted so that the cross range extent of the image
// approximately equals its range extent:

void quantum_2Dwavefunction::writeimagedata(
   int doublet_pair_member,const complex cvalue[nxbins_max][nybins_max])
{
   bool pixel_inside_inner_display_frac_region=true;
   double curr_value;
   
   imagestream.precision(2);
   imagestream.setf(ios::scientific);
   imagestream.setf(ios::showpoint);  

// Recall ylo corresponds to index 0, while yhi corresponds to index
// nybins-1.  In order for the image to appear right-side up, we need
// to decrement j from nybins-1 down to 0:

   for (int j=nybins-1; j>=0; j--)  
   {
      for (int i=0; i<nxbins; i++)
      {
         if ((abs(i-nxbins/2) <= (nxbins_to_display-1)/2) &&
             (abs(j-nybins/2) <= (nybins_to_display-1)/2))
         {
            pixel_inside_inner_display_frac_region=true;
         }
         else
         {
            pixel_inside_inner_display_frac_region=false;
         }

         if (pixel_inside_inner_display_frac_region)
         {
            if (doublet_pair_member==0)
            {
               curr_value=cvalue[i][j].getmod();
            }
            else
            {
               curr_value=cvalue[i][j].getarg();
            }
         
// Set any points whose value >= POSITIVEINFINITY equal to 1000 and
// any points whose values <= NEGATIVEINFINITY equal to -1000:

            if (fabs(curr_value) < POSITIVEINFINITY)
            {
               imagestream << curr_value << " ";
            }
            else if (curr_value >= POSITIVEINFINITY)
            {
               imagestream << " +1000 ";
            }
            else if (curr_value <= NEGATIVEINFINITY)
            {
               imagestream << " -1000 ";
            }
         
// According to Iva, each line of output in an metafile image must
// contain no more than 2000 characters.  However, the metaplot parser
// ignores all carriage returns.  So we simply need to insert enough
// carriage returns in the output so that there are never more than
// 2000 characters on any given line:

            if ((i+1)%10==0) imagestream << endl;
         
         } // check on whether (i,j) pixel lies within inner display_frac
         //  fraction of image
         
      } // lop over index i

      if (pixel_inside_inner_display_frac_region)
      {
         imagestream << endl;
         imagestream << endl;
      }
   }  // loop over index j
}

// ---------------------------------------------------------------------
// Member function writeimage was added on 4/25/02 primarily for
// debugging and algorithm development purposes

void quantum_2Dwavefunction::writeimage(
   string base_imagefilename,string currtitle,
   const complex carray[nxbins_max][nybins_max])
{
   imagefilenamestr=imagedir+base_imagefilename;
   string metafilename=imagefilenamestr+".meta";
   title=currtitle;
   filefunc::openfile(metafilename,imagestream);
   potential_header("triplet");
   write_potential_data();
   for (int j=0; j<2; j++)
   {
      tripletfile_header(j);
      writeimagedata(j,carray);
   }
   filefunc::closefile(metafilename,imagestream);
   filefunc::meta_to_jpeg(imagefilenamestr);
   filefunc::gzip_file(metafilename);
}

// ---------------------------------------------------------------------
void quantum_2Dwavefunction::plot_potential(int imagenumber)
{
   string plotfilename="potential"
      +stringfunc::number_to_string(imagenumber);
//      +integer_to_string(imagenumber,ndigits_max);
   plot_potential(plotfilename);
}

// ---------------------------------------------------------------------
// Member function plot_potential generates a metafile containing
// potential values plotted as functions of horizontal and vertical
// coordinates:

void quantum_2Dwavefunction::plot_potential(string plotfilename)
{
   outputfunc::write_banner("Plotting potential:");
   imagefilenamestr=imagedir+plotfilename+".meta";
   filefunc::openfile(imagefilenamestr,imagestream);
   potential_header("singlet");
   write_potential_data();
   potential_footer();
   filefunc::closefile(imagefilenamestr,imagestream);
   filefunc::meta_to_jpeg(imagefilenamestr);
   filefunc::gzip_file(imagefilenamestr);
   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Member function write_potential_data writes an image array's worth
// of potential energy values to the output metafile:

void quantum_2Dwavefunction::write_potential_data()
{
   bool pixel_inside_inner_display_frac_region=true;

   imagestream.precision(2);
   imagestream.setf(ios::scientific);
   imagestream.setf(ios::showpoint);  
   
// Recall ylo corresponds to index 0, while yhi corresponds to index
// nybins-1.  In order for the image to appear right-side up, we need
// to decrement j from nybins-1 down to 0:

   for (int j=nybins-1; j>=0; j--)  
   {
      y=ylo+j*deltay;
      for (int i=0; i<nxbins; i++)
      {
         x=xlo+i*deltax;
         
         if ((abs(i-nxbins/2) <= (nxbins_to_display-1)/2) &&
             (abs(j-nybins/2) <= (nybins_to_display-1)/2))
         {
            pixel_inside_inner_display_frac_region=true;
         }
         else
         {
            pixel_inside_inner_display_frac_region=false;
         }

         if (pixel_inside_inner_display_frac_region)
         {
            double V=potential(x,y);

// Set any points whose value >= POSITIVEINFINITY equal to 1000 and
// any points whose values <= NEGATIVEINFINITY equal to -1000:

            if (fabs(V_displayfactor*V) < POSITIVEINFINITY)
            {
               imagestream << V_displayfactor*V << " ";
            }
            else if (V_displayfactor*V >= POSITIVEINFINITY)
            {
               imagestream << " +1000 ";
            }
            else if (V_displayfactor*V <= NEGATIVEINFINITY)
            {
               imagestream << " -1000 ";
            }
         
// According to Iva, each line of output in an metafile image must
// contain no more than 2000 characters.  However, the metaplot parser
// ignores all carriage returns.  So we simply need to insert enough
// carriage returns in the output so that there are never more than
// 2000 characters on any given line:

            if ((i+1)%10==0) imagestream << endl;
         
         } // check on whether (i,j) pixel lies within inner display_frac
         //  fraction of image
         
      } // index i over horizontal direction x loop

      if (pixel_inside_inner_display_frac_region)
      {
         imagestream << endl;
         imagestream << endl;
      }
   }  // index j over vertical direction y loop
}

// =====================================================================
// Wavefunction manipulation member functions:
// =====================================================================

void quantum_2Dwavefunction::clear_wavefunction()
{
   for (int i=0; i<nxbins_max; i++)
   {
      for (int j=0; j<nybins_max; j++)
      {
         value[i][j]=tilde[i][j]=0;
      }
   }
}

// ---------------------------------------------------------------------
void quantum_2Dwavefunction::copy_wavefunction(
   const complex value_orig[nxbins_max][nybins_max],
   complex value_copy[nxbins_max][nybins_max])
{
   for (int i=0; i<nxbins_max; i++)
   {
      for (int j=0; j<nybins_max; j++)
      {
         value_copy[i][j]=value_orig[i][j];
      }
   }
}

// ---------------------------------------------------------------------
// Member function initialize_spatial_and_momentum_parameters sets the
// size of the domain over which the wavefunction is to be calculated.
// We exploit the periodicity of FFT's for problems in which the
// potential is periodic.  In particular, we restrict the domains on
// the x and y axes to just a single period for periodic potentials.
// We also take great care to ensure that x_{nxbins-1} does NOT
// correspond to x_{0} (and similarly for the discretized y values).
// For periodic problems, x_{nxbins-1} should instead correspond to
// "x_{-1}".  The maximum/minimum limits of discretized momentum space
// variables are similarly arranged so that kx_{nxbins-1} corresponds
// to kx_{-1} for problems with period nxbins...

void quantum_2Dwavefunction::initialize_spatial_and_momentum_parameters()
{
   nxbins=nxbins_max;  // odd
   nybins=nybins_max;  // odd

// First set wavefunction normalization value:

   norm=1;

// Introduce shifts xshift and yshift for numerical simulation
// purposes only.  These constant offsets do not affect the QFP's
// dynamics.  Instead, they simply shift the independent variables'
// coordinate system.

   if (potential_type==potentialfunc::QFP)
   {
//      yshift=PI/2;
   }

// Next set spatial and momentum step sizes.  Recall that the system's
// dimensionless coupling constant alpha ( = ratio of potential to
// kinetic energies) is saved within the potential_param[0] member
// variable.  And an aperiodic system's characteristic length scale
// goes like 1/sqrt(sqrt(alpha)).

   double alpha=potential_param[0];
   double lengthscale;
   if (xperiod >= sqrt(double(nxbins)))	 // potential aperiodic in x direction
   {
      lengthscale=1.0/sqrt(sqrt(alpha));
      xhi=sqrt(double(nxbins))/2*lengthscale;
      deltax=2*xhi/(nxbins-1);
   }
   else					// potential periodic in x direction
   {
      deltax=xperiod/double(nxbins);
      xhi=(nxbins-1)/2.0*deltax;
   }
   xlo=-xhi;
   
   if (yperiod >= sqrt(nybins))		// potential aperiodic in y direction
   {
      lengthscale=1.0/sqrt(sqrt(alpha));
      yhi=sqrt(double(nybins))/2*lengthscale;
      deltay=2*yhi/(nybins-1);
   }
   else					// potential periodic in y direction
   {
      deltay=yperiod/double(nybins);
      yhi=(nybins-1)/2.0*deltay;
   }
   ylo=-yhi;

   delta_kx=1.0/(nxbins*deltax);
   if (xperiod < POSITIVEINFINITY)	// periodic potential
   {
      kx_hi=(nxbins-1)/2.0*delta_kx;
      kx_lo=-kx_hi;
   }
   else					// aperiodic potential
   {
      kx_hi=(nxbins-1)*delta_kx/2.0;
      kx_lo=-kx_hi;
   }

   delta_ky=1.0/(nybins*deltay);
   if (yperiod < POSITIVEINFINITY)	// periodic potential
   {
      ky_hi=(nybins-1)/2.0*delta_ky;
      ky_lo=-ky_hi;
   }
   else					// aperiodic potential
   {
      ky_hi=(nybins-1)*delta_ky/2.0;
      ky_lo=-ky_hi;
   }

   cout << "Coupling constant alpha = " << alpha << endl;
   cout << "xhi = " << xhi << " xlo = " << xlo << endl;
   cout << "yhi = " << yhi << " ylo = " << ylo << endl;
   cout << "nxbins = " << nxbins << endl;
   cout << "nybins = " << nybins << endl;
   cout << "x bin size = " << deltax << endl;
   cout << "y bin size = " << deltay << endl;
   cout << "kx bin size = " << delta_kx << endl;
   cout << "ky bin size = " << delta_ky << endl;
}

// ---------------------------------------------------------------------
// Member function trial_wavefunction returns within complex array
// curr_value a trial wavefunction.  If the potential V(x,y) is
// periodic function of x and y, we take the ground state trial
// wavefunction to be proportional to 1/V_displayfactor*V(x,y).  [We
// include the scaling factor V_displayfactor into the trial
// wavefunction so that the initial guess is neither enormously large
// nor small.]  On the other hand, if the potential has infinite
// period, we take the ground state trial wavefunction to be gaussian.
// In both cases, we multiply the trial wavefunctions by x [y] if the
// input boolean flag even_x_wavefunction [even_y_wavefunction] is
// false.

void quantum_2Dwavefunction::trial_wavefunction(
   int m,int n,complex curr_value[nxbins_max][nybins_max])
{
   const double minV=1E-5;
   double xterm,yterm;
   double currterm;
   mypolynomial poly_x,poly_y;

// Recall that the system's dimensionless coupling constant alpha ( =
// ratio of potential to kinetic energies) is saved within the
// potential_param[0] member variable.  And an aperiodic system's
// characteristic length scale goes like 1/sqrt(sqrt(alpha)).

   double alpha=potential_param[0];
   double lengthscale=1.0/sqrt(sqrt(alpha));

   if (xperiod < POSITIVEINFINITY)
   {
      for (int i=0; i<nxbins; i++)
      {
         x=xlo+i*deltax;
         for (int j=0; j<nybins; j++)
         {
            y=ylo+j*deltay;
            double V=potential(x,y);
            if (fabs(V_displayfactor*V) > minV)
            {
               currterm=1.0/(V_displayfactor*V);
            }
            else
            {
               currterm=1.0/minV;
            }
            currterm *= poly_x.basis_chebyshev(x/lengthscale,m)*
               poly_y.basis_chebyshev(y/lengthscale,n);
            curr_value[i][j]=currterm;
         } // loop over index j
      } // loop over index i
   }
   else 
   {
      for (int i=0; i<nxbins; i++)
      {
         x=xlo+i*deltax;
         xterm=exp(-sqr((x-xshift)/lengthscale))
            *poly_x.basis_chebyshev((x-xshift)/lengthscale,m);
         for (int j=0; j<nybins; j++)
         {
            y=ylo+j*deltay;
            yterm=exp(-sqr((y-yshift)/lengthscale))
               *poly_y.basis_chebyshev((y-yshift)/lengthscale,n);
            curr_value[i][j]=xterm*yterm;
         }  // loop over index j
      } // loop over index i
   } // potential_type conditional

   renormalize_wavefunction(curr_value);
//   writeimage("trial","trial",curr_value);

// FAKE FAKE:  For debugging purposes only:

//   cout << "Fourier transform of trial wavefunction:" << endl;
//   complex curr_tilde[nxbins_max][nybins_max];
//   quantumimage::fouriertransform(curr_value,curr_tilde);
//   writeimage("ft_trial","ft_trial",curr_tilde);
}

// ---------------------------------------------------------------------
// Member function initialize_wavefunction fills up the position space
// value array with either the projected ground state, first excited
// state or a superposition of the two depending upon the state_type
// flag set within quantum_wavefunction::select_initial_state:

void quantum_2Dwavefunction::initialize_wavefunction()
{
   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins; j++)
      {
         if (state_type=="(0,0)")
         {
            value[i][j]=energy_eigenstate[0][0][i][j];
         }
         else if (state_type=="(1,0)")
         {
            value[i][j]=energy_eigenstate[1][0][i][j];
         }
         else if (state_type=="(0,1)")
         {
            value[i][j]=energy_eigenstate[0][1][i][j];
         }
         else if (state_type=="(0,0)+(1,0)")
         {
            value[i][j]=(energy_eigenstate[0][0][i][j]+
                         energy_eigenstate[1][0][i][j])/sqrt(2.0);
         }
         else if (state_type=="(0,0)+(0,1)")
         {
            value[i][j]=(energy_eigenstate[0][0][i][j]+
                         energy_eigenstate[0][1][i][j])/sqrt(2.0);
         }
         else if (state_type=="(0,0)+(1,0)+(0,1)")
         {
            value[i][j]=(energy_eigenstate[0][0][i][j]+
                         energy_eigenstate[1][0][i][j]+
                         energy_eigenstate[0][1][i][j])/sqrt(3.0);
         }
         else if (state_type=="(2,0)")
         {
            value[i][j]=energy_eigenstate[2][0][i][j];
         }
         else if (state_type=="(1,1)")
         {
            value[i][j]=energy_eigenstate[1][1][i][j];
         }
         else if (state_type=="(0,2)")
         {
            value[i][j]=energy_eigenstate[0][2][i][j];
         }
         prev_arg[i][j]=curr_arg[i][j]=value[i][j].getarg();
      }  // loop over index j
   }  // loop over index i
}

// ---------------------------------------------------------------------
void quantum_2Dwavefunction::evolve_phase()
{
   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins; j++)
      {
         if (domain_name==quantum::position_space)
         {
            if (value[i][j].getmod() < TINY)
            {
               curr_arg[i][j]=0;
            }
            else
            {
               curr_arg[i][j]=value[i][j].getarg(prev_arg[i][j]);
            }
         }
         else if (domain_name==quantum::momentum_space)
         {
            if (tilde[i][j].getmod() < TINY)
            {
               curr_arg[i][j]=0;
            }
            else
            {
               curr_arg[i][j]=tilde[i][j].getarg(prev_arg[i][j]);
            }
         }

// For display purposes only, we restrict the range of curr_arg to lie
// within the interval (-PI,PI):

         curr_arg[i][j]=basic_math::phase_to_canonical_interval(
            curr_arg[i][j],-PI,PI);
         prev_arg[i][j]=curr_arg[i][j];
      }	// j loop
   } // i loop
}

// ---------------------------------------------------------------------
// Member function potential computes the potential along with its
// first and second derivatives for several different models.  We
// store the dimensionless coupling contant ( = ratio of
// characteristic potential to kinetic energies) for each system in
// potential_param[0] for resonant frequency and time stepsize
// computation purposes.  It is important to recall that a system's
// characteristic time and length scales are set by fractional powers
// of this dimensionless coupling!

double quantum_2Dwavefunction::potential(double x,double y)
{
   double V,dVdx,dVdy,laplacianV;
   potential(x,y,V,dVdx,dVdy,laplacianV);
   return V;
}

void quantum_2Dwavefunction::potential(
   double x,double y,double& V,double& dVdx,double& dVdy,double& laplacianV)
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
   
   if (potential_type==potentialfunc::box)
   {

// box potential parameters:

      const double Lx=1;
      const double Ly=1;

      if (fabs(x) < Lx && fabs(y) < Ly)
      {
         V=0;
      }
      else
      {
         V=POSITIVEINFINITY;
      }
      dVdx=dVdy=laplacianV=0;
      potential_param[0]=1;
   }
   else if (potential_type==potentialfunc::harmonic_osc)
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
      potential_param[0]=sqrt(sqr(Cx)+sqr(Cy));
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

//      V=4+2*cos2x+2*cos2y;
//      dVdx=-8*cos2x;
//      dVdy=-8*cos2y;
//      laplacianV=-8*(cos2x+cos2y);
      potential_param[0]=4;
   }
   else if (potential_type==potentialfunc::QFP)
   {
      if (time_dependent_potential)
      {
         potentialfunc::evaluate_current_potential_params(
            potential_type,potential_t1,potential_t2,t,potential_param);
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
      } // time_dependent_potential conditional

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
   }
}

// ---------------------------------------------------------------------
// This overloaded version of member function potential computes the
// potential along with its 1st derivative, laplacian and double
// laplacian for several different models:

void quantum_2Dwavefunction::potential(
   double x,double y,double& V,double& dVdx,double& dVdy,double& laplacianV,
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
            potential_type,potential_t1,potential_t2,t,potential_param);
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
   }
}

// ---------------------------------------------------------------------
void quantum_2Dwavefunction::set_potential_spatial_period()
{
   if (potential_type==potentialfunc::box || 
       potential_type==potentialfunc::harmonic_osc ||
       potential_type==potentialfunc::QFP)
   {
      xperiod=yperiod=POSITIVEINFINITY;
   }
   else if (potential_type==potentialfunc::doublewell || 
            potential_type==potentialfunc::mathieu)
   {
      xperiod=yperiod=2*PI;
   }
}

// ---------------------------------------------------------------------
// Member function renormalize_wavefunction renormalizes the
// wavefunction to some user specified value norm:

void quantum_2Dwavefunction::renormalize_wavefunction(
   complex curr_value[nxbins_max][nybins_max])
{
   double curr_norm=compute_normalization(curr_value);
   double sqrt_norm_over_currnorm=sqrt(norm/curr_norm);
//   double max_renormalized_value=NEGATIVEINFINITY;
   
   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins; i++)
      {
         curr_value[i][j]=sqrt_norm_over_currnorm*curr_value[i][j];
//         max_renormalized_value=max(max_renormalized_value,
//                                    curr_value[i][j].getreal());
      }
   }

//   cout << "Max renormalized value = " << max_renormalized_value << endl;
}

// ---------------------------------------------------------------------
// Routine simpsonsum performs two-dimensional quadrature by
// iteratively calling the 1D simsonsum routine:

double quantum_2Dwavefunction::double_simpsonsum(
   double f[nxbins_max][nybins_max])
{
   double fcolumn[nybins_max];
   double singlesum[nxbins_max];

   for (int i=0; i<nxbins_max; i++)
   {
      for (int j=0; j<nybins_max; j++)
      {
         fcolumn[j]=f[i][j];
      }
      singlesum[i]=mathfunc::simpsonsum(fcolumn,0,nybins_max-1);
   }
   double doublesum=mathfunc::simpsonsum(singlesum,0,nxbins_max-1);
   return doublesum;
}

// ---------------------------------------------------------------------
// This overloaded version of simpsonsum performs quadrature on
// complex two-dimensional arrays:

complex quantum_2Dwavefunction::double_simpsonsum(
   complex f[nxbins_max][nybins_max])
{
   complex fcolumn[nybins_max];
   complex singlesum[nxbins_max];

   for (int i=0; i<nxbins_max; i++)
   {
      for (int j=0; j<nybins_max; j++)
      {
         fcolumn[j]=f[i][j];
      }
      singlesum[i]=simpsonsum_complex(fcolumn,0,nybins_max-1);
   }
   return simpsonsum_complex(singlesum,0,nxbins_max-1);
}

// =====================================================================
// Wavefunction evolution member functions
// =====================================================================

// Member function FFT_step_wavefunction implements a momentum space
// approach to evolving the wavefunction by either exp (-iHt) or its
// Wick rotated counterpart exp(-Ht):

// |psi(t+dt)> = ... exp(-i dt^3/3 [V,[V,P^2]]) * exp(i dt^3/6 [[V,P^2],P^2)
// 	       * exp(dt^2/2 [V,P^2]) exp(-i dt V) exp(-i dt P^2) |psi(t)>,

//		   	or the Wick rotated version

// |psi(t+dt)> = ... exp(dt^3/3 [V,[V,P^2]]) * exp(dt^3/6 [[V,P^2],P^2)
//	       * exp(-dt^2/2 [V,P^2]) exp(-dt V) exp(-dt P^2) |psi(t)>. 

// O(dt^4) terms represented by the initial ellipses are neglected
// below.  These formulas are only truly accurate to O(dt^2), for we
// have to approximate the potential's second derivative by some
// constant K.  In order to preserve amplitude content, the
// wavefunction is renormalized after it has been evolved by one time
// step.

void quantum_2Dwavefunction::FFT_step_wavefunction( 
   bool Wick_rotate,complex curr_value[nxbins_max][nybins_max])
{
   int k,l,kplusone,lplusone;
   double x,y,kx,ky;
   double xstar,ystar,xk,yl;
   double term0,term1,term2,term3;

   complex value1a[nxbins_max][nybins_max];
   complex value1b,value1c;
   complex value1d[nxbins_max][nybins_max];
   complex curr_tilde[nxbins_max][nybins_max];
   complex tilde1a[nxbins_max][nybins_max];

// First evaluate |psi_1a> =  exp({-2/3 K i dt^3 - i dt} P^2) |psi(t)>

// or (Wick rotated)

//	        |psi_1a> = exp({2/3 K i dt^3 - dt} P^2) |psi(t)>

// As of 11/4/01, we have left out the term depending upon K12 P_x P_y.
// We'll fix this later...

   fouriertransform(curr_value,curr_tilde);
   double V,dVdx,dVdy,laplacianV,doublelaplacianV,Kconst,K12;
   potential(0,0,V,dVdx,dVdy,laplacianV,doublelaplacianV,Kconst,K12);

   for (int i=0; i<nxbins; i++)
   {
      kx=kx_lo+i*delta_kx;
      for (int j=0; j<nybins; j++)
      {
         ky=ky_lo+j*delta_ky;
         
         if (Wick_rotate)
         {
            term0=1-0.666*Kconst*sqr(deltat);
            term1=sqr(2*PI)*(sqr(kx)+sqr(ky))*deltat*term0;
            tilde1a[i][j]=exp(-term1)*curr_tilde[i][j];
         }
         else
         {
            term0=1+0.666*Kconst*sqr(deltat);
            term1=sqr(2*PI)*(sqr(kx)+sqr(ky))*deltat*term0;
            tilde1a[i][j]=complex(cos(term1),-sin(term1))*curr_tilde[i][j];
         }
      }
   }
   inversefouriertransform(tilde1a,value1a);

// Next evaluate |psi_1b> = exp(-i dt V(x)) |psi_1a> 
//               |psi_1c> = exp(1/2 dt^2 d^2V/dx^2) |psi_1b>, or
// 	       |psi_1d> =exp(i dt^3 { 2/3 (dV/dX)^2 + 1/6 d^4V/dX^4}) |psi_1c>

// or (Wick rotated)

// 		 |psi_1b> = exp(-dt V(x)) |psi_1a> 
//               |psi_1c> = exp(-1/2 dt^2 d^2V/dx^2) |psi_1b> (Wick rotated)
//           |psi_1d> =exp(-i dt^3 { 2/3 (dV/dX)^2 + 1/6 d^4V/dX^4}) |psi_1c> 

   for (int i=0; i<nxbins; i++)
   {
      x=xlo+i*deltax;
      for (int j=0; j<nybins; j++)
      {
         y=ylo+j*deltay;

         potential(x,y,V,dVdx,dVdy,laplacianV,doublelaplacianV,Kconst,K12);

// Set position space wavefunction values to zero at points where
// potential is infinite:

// We commented out the following lines on 4/25/02 due to the QFP
// potential's values exceeding 1E8:

//         if (V >= POSITIVEINFINITY)
//         {
//            value1d[i][j]=0;
//         }
//         else
         {
            term2=V*deltat;
            term3=(0.666*(sqr(dVdx)+sqr(dVdy))
                   +doublelaplacianV/6.)*pow(deltat,3);
            if (Wick_rotate)
            {
               value1b=exp(-term2)*value1a[i][j];
               value1c=exp(-0.5*sqr(deltat)*laplacianV)*value1b;
               value1d[i][j]=exp(-term3)*value1c;
            }
            else
            {
               value1b=complex(cos(term2),-sin(term2))*value1a[i][j];
               value1c=exp(0.5*sqr(deltat)*laplacianV)*value1b;
               value1d[i][j]=complex(cos(term3),sin(term3))*value1c;
            }
         } // V >= POSITIVEINFINITY conditional
      }	// loop over row index j
   } // loop over column index i

// Set |psi_new(x)> = exp(i dt^2 dV/dx P) |psi_1d(x)> 
//		    = |psi_1d(x+dt^2 dV/dx)> or

// or (Wick rotated)

//     |psi_new(x)> = exp(-i dt^2 dV/dx P) |psi_1d(x)> 
// 		    = |psi_1d(x-dt^2 dV/dx)> 

   for (int i=0; i<nxbins; i++)
   {
      x=xlo+i*deltax;
      for (int j=0; j<nybins; j++)
      {
         y=ylo+j*deltay;
         potential(x,y,V,dVdx,dVdy,laplacianV);
          
// On 4/25/02, we commented out the next 3 lines due to the QFP
// potential exceeding 1E8:

//         if (fabs(V) >= POSITIVEINFINITY)
//         {
//            curr_value[i][j]=0;
//         }
//         else
         {
            if (Wick_rotate)
            {
               xstar=x-sqr(deltat)*dVdx;
               ystar=y-sqr(deltat)*dVdy;
            }
            else
            {
               xstar=x+sqr(deltat)*dVdx;
               ystar=y+sqr(deltat)*dVdy;
            }
            k=basic_math::round(floor((xstar-xlo)/deltax));
            l=basic_math::round(floor((ystar-ylo)/deltay));
            xk=xlo+k*deltax;
            yl=ylo+l*deltay;

// If V(x,y) is periodic, we must exercise care when shifting the
// wavefunction to discretized values of x above xhi or below xlo (and
// similarly for discretized y values).  If the wavefunction is
// periodic, we cannot generally set its value at such hi and lo
// points to zero without doing violence to wavefunction continuity.
// Instead, we recall that x_{nxbins} maps onto x_{0}, x_{nxbins-1}
// maps onto x_{-1}, etc...

            if (k>=0 && k<nxbins-1 && l>=0 && l<nybins-1)
            {
               curr_value[i][j]=value1d[k][l]
                  +(xstar-xk)/deltax*(value1d[k+1][l]-value1d[k][l])
                  +(ystar-yl)/deltay*(value1d[k][l+1]-value1d[k][l])
                  +(xstar-xk)*(ystar-yl)/(deltax*deltay)
                  *(value1d[k+1][l+1]-value1d[k+1][l]-value1d[k][l+1]
                    +value1d[k][l]);
            }
            else
            {
               if (xperiod==POSITIVEINFINITY && yperiod==POSITIVEINFINITY)
               {
                  curr_value[i][j]=0;
               }
               else  // As of 2/1/01, we assume periodic potentials are
		     // periodic in BOTH the x and y directions
               {
                  if (k >= nxbins)
                  {
                     k -= nxbins;
                  }
                  else if (k < 0)
                  {
                     k += nxbins;
                  }
                  kplusone=k+1;
                  if (kplusone >= nxbins)
                  {
                     kplusone -= nxbins;
                  }
                  else if (kplusone < 0)
                  {
                     kplusone += nxbins;
                  }

                  if (l >= nybins)
                  {
                     l -= nybins;
                  }
                  else if (l < 0)
                  {
                     l += nybins;
                  }
                  lplusone=l+1;
                  if (lplusone >= nybins)
                  {
                     lplusone -= nybins;
                  }
                  else if (lplusone < 0)
                  {
                     lplusone += nybins;
                  }
                  
                  if (k < 0 || k >= nxbins)
                  {
                     cout << "Error: k = " << k << endl;
                  }
                  if (kplusone < 0 || kplusone >= nxbins)
                  {
                     cout << "Error: kplusone = " << kplusone << endl;
                  }
                  
                  if (l < 0 || l >= nybins)
                  {
                     cout << "Error: l = " << l << endl;
                  }
                  if (lplusone < 0 || lplusone >= nybins)
                  {
                     cout << "Error: lplusone = " << lplusone << endl;
                  }
                  
                  curr_value[i][j]=value1d[k][l]
                     +(xstar-xk)/deltax*(value1d[kplusone][l]-value1d[k][l])
                     +(ystar-yl)/deltay*(value1d[k][lplusone]-value1d[k][l])
                     +(xstar-xk)*(ystar-yl)/(deltax*deltay)
                     *(value1d[kplusone][lplusone]
                       -value1d[kplusone][l]
                       -value1d[k][lplusone]
                       +value1d[k][l]);

               } // xperiod==POSITIVEINFINITY && yperiod==POSITIVEINFINITY)
            } // k>=0 && k<nxbins-1 && l>=0 && l<nybins-1
         } // V >= POSITIVEINFINITY conditional
      }	// loop over row index j
   } // loop over column index i

   renormalize_wavefunction(curr_value);
}

// ---------------------------------------------------------------------
// Member function project_low_energy_states calculates the lowest
// energy eigenstates for any 2D potential.  Recall that an arbitrary
// trial wavefunction can be decomposed in terms of energy eigenstates
// as

// 		psi_trial = C0 psi_0 + C1 psi_1 + C2 psi_2 + ...

// In order to project out the ground state psi_0, we first evolve the
// trial wavefunction according to the Wick rotated time evolution
// operator exp(-H t).  The wavefunction is renormalized after each
// time step in order to preserve amplitude content.  As the iteration
// proceeds, the excited energy state content of the trial
// wavefunction becomes exponentially suppressed compared to its
// ground state component.  In this way, the trial wavefunction
// relaxes to the ground state.

// Once the ground state is known, we compute C0=<psi_0|psi_trial> and
// then remove the lowest energy state from a new, odd trial
// wavefunction to obtain the reduced wavefunction psi_reduced =
// psi_trial-C0 psi_0.  At this point, we can project out the first
// excited eigenstate by following the procedure described above.  

// Following this strategy of evolving and projecting, we can compute
// any low energy eigenstate so long as build-up of numerical
// inaccuracies does not become overwhelmingly large.

void quantum_2Dwavefunction::project_low_energy_states(
   int n_energystates)
{
   bool time_dependent_potential_orig=time_dependent_potential;
   complex curr_value[nxbins_max][nybins_max];

  time_dependent_potential=false;

// First read in all previously calculated eigenfunctions:

   for (int m=0; m<Nmax_energystates; m++)
   {
      for (int n=0; n<Nmax_energystates; n++)
      {
         efunc_calculated[m][n]=false;
         efunc_calculated[m][n]=restore_eigenfunction(m,n);
      }
   }

// Next compute new eigenfunctions which were not previously calculated:

   for (int m=0; m<n_energystates; m++)
   {
      for (int n=0; n<n_energystates-m; n++)
      {
         if (!efunc_calculated[m][n])
         {
            cout << "Calculating eigenfunction (" << m << ","
                 << n << ")" << endl;
            trial_wavefunction(m,n,curr_value);
            remove_overlap(m,n,curr_value);
            project_energy_eigenstate(m,n,curr_value);
         }
         
         double E_mn=compute_energy(energy_eigenstate[m][n]);
         cout << "m = " << m << " n = " << n 
              << " E_mn = " << E_mn << endl;
         cout << "normalization = " 
              << compute_normalization(energy_eigenstate[m][n])
              << endl;

         for (int i=0; i<=m; i++)
         {
            for (int j=0; j<=n; j++)
            {
               cout << "m = " << m << " n = " << n 
                    << " i = " << i << " j = " << j 
                    << " < i,j | m,n > = " 
                    << energystate_overlap(i,j,energy_eigenstate[m][n]) 
                    << endl;
            }
         }
      }
   }
   time_dependent_potential=time_dependent_potential_orig;
}

// ---------------------------------------------------------------------
// Member function energystate_overlap calculates the projection
// coefficient <m,n|psi> where |m,n> is the energy eigenstate labeled
// by x and y quantum numbers m and n, and |psi> is the 2D state
// specified within input array curr_value:

complex quantum_2Dwavefunction::energystate_overlap(
   int m,int n,complex curr_value[nxbins_max][nybins_max])
{
   complex overlap[nxbins_max][nybins_max];
   for (int i=0; i<nxbins; i++)
   {
      x=xlo+i*deltax;
      for (int j=0; j<nybins; j++)
      {
         y=ylo+j*deltay;
         overlap[i][j]=energy_eigenstate[m][n][i][j].Conjugate()
            *curr_value[i][j];
      }
   }
   complex C_mn=double_simpsonsum(overlap)*deltax*deltay;
   return C_mn;
}

// ---------------------------------------------------------------------
// Member function remove_overlap takes in a wavefunction contained
// within input array curr_value and subtracts away the energy
// eigenstates corresponding to quantum numbers m < mmax and n < nmax.
// The reduced wavefunction with these lower energy eigenstates
// removed is returned within array curr_value:

void quantum_2Dwavefunction::remove_overlap(
   int mmax,int nmax,complex curr_value[nxbins_max][nybins_max])
{
   complex C_mn;

/*
// Note: Since the energy of a state grows with the number of
// wavefunction nodes in the x and y directions which are basically
// labeled by quantum numbers m and n, we experiment (as of April 02)
// with subtracting the highest energy states first and the ground
// state last from curr_value.  Perhaps this will help solve eliminate
// the eigenfunction projection problems which we are experiencing
// with the (2,0) and (0,2) states....
*/

// Project out every previously calculated energy eigenfunction from
// current wavefunction.  Subtract off all of these projections.

   for (int m=0; m<Nmax_energystates; m++)
   {
      for (int n=0; n<Nmax_energystates; n++)
      {
         if (efunc_calculated[m][n])
         {
            C_mn=energystate_overlap(m,n,curr_value);
            for (int i=0; i<nxbins; i++)
            {
               x=xlo+i*deltax;
               for (int j=0; j<nybins; j++)
               {
                  y=ylo+j*deltay;
                  curr_value[i][j]=curr_value[i][j]
                     -C_mn*energy_eigenstate[m][n][i][j];
               }
            }
         } // efunc_calculated conditional
      }	// mmax loop
   } // nmax loop
}

// ---------------------------------------------------------------------
// Member function project_energy_eigenstate evolves a wavefunction
// contained within input array prev_value by the Wick rotated time
// evolution operator exp(-Ht).  The wavefunction is renormalized
// after each time step in order to preserve amplitude content.  The
// evolution continues until the fractional difference in each bin is
// smaller than some specified tolerance value.  At this point, all
// higher energy eigenstates are exponentially suppressed compared to
// the lowest lying energy state.  The surviving energy eigenstate
// values are copied into the array energy_eigenstate.

void quantum_2Dwavefunction::project_energy_eigenstate(
   int m,int n,complex prev_value[nxbins_max][nybins_max])
{
   const bool Wick_rotate=true;
   const double EXTREMELY_TINY=1E-10;

   int imax,jmax;
   imax=jmax=0;
   double tolerance,max_frac_diff;
   complex max_prevvalue,max_currvalue,curr_diff;

   int nt_min,nt_max;
   if (potential_type != potentialfunc::QFP)
   {
      tolerance=1E-4;
//      tolerance=1E-2;   // For faster but sloppier eigenfunction 
				     //  determination
      nt_min=200;
//      nt_max=250;	// For faster e'func determination
      nt_max=3000;	// For faster e'func determination
   }
   else
   {
      tolerance=1E-3;   // For faster but sloppier eigenfunction 
      nt_min=750;
      nt_max=1500;	// For faster e'func determination
   }

   complex curr_value[nxbins_max][nybins_max];
   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins; j++)
      {
         curr_value[i][j]=prev_value[i][j];
      }
   }

// As an experiment, we attempt as of 4/24/02 to increase the time
// step deltat in order to speed up energy eigenfunction computations:

   double deltat_orig=deltat;
   if (potential_type != potentialfunc::QFP)
   {
//      deltat=4*deltat_orig;
   }
   
   int nt=0;
   do
   {
      t=tmin+nt*deltat;
      FFT_step_wavefunction(Wick_rotate,curr_value);

      max_frac_diff=0;
      for (int i=0; i<nxbins; i++)
      {
         for (int j=0; j<nybins; j++)
         {
            curr_diff=curr_value[i][j]-prev_value[i][j];
            if (curr_value[i][j].getmod() > EXTREMELY_TINY && 
                prev_value[i][j].getmod() > EXTREMELY_TINY)
            {
               max_frac_diff=max(
                  max_frac_diff,
                  fabs(curr_diff.getmod())/fabs(curr_value[i][j].getmod()));
               max_currvalue=curr_value[i][j];
               max_prevvalue=prev_value[i][j];
               imax=i;
               jmax=j;
            }
            prev_value[i][j]=curr_value[i][j];
         }
      }

      outputfunc::newline();
      cout << "nt = " << nt << " max_frac_diff = " << max_frac_diff << endl;
      cout << "imax = " << imax << " jmax = " << jmax << endl;
      cout << "max currvalue = " << max_currvalue 
           << " max prevvalue = " << max_prevvalue << endl;
      nt++;

// After each time step, subtract away any residual eigenstates
// corresponding to quantum numbers LESS than m and n:

      remove_overlap(m,n,curr_value);
   }	
//   while ((max_frac_diff > tolerance) && (nt < nt_max));
   while ((max_frac_diff > tolerance || nt < nt_min) && (nt < nt_max));
   
   cout << "Number of iterations needed to project out energy eigenstate "
        << "(" << m << "," << n << ") = " << nt << endl;
   cout << "max_frac_diff = " << max_frac_diff << endl;

   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins; j++)
      {
         energy_eigenstate[m][n][i][j]=curr_value[i][j];
      }
   }

   deltat=deltat_orig;
   if (save_eigenfunction) dump_eigenfunction(m,n);
}

// =====================================================================
// Wavefunction properties member functions:
// =====================================================================

double quantum_2Dwavefunction::compute_normalization(
   const complex curr_value[nxbins_max][nybins_max])
{
   double magsq[nxbins_max][nybins_max];

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins; i++)
      {
         magsq[i][j]=sqr(curr_value[i][j].getreal())
            +sqr(curr_value[i][j].getimag());
      }
   }
   double curr_norm=double_simpsonsum(magsq)*deltax*deltay;
   return curr_norm;
}

// ---------------------------------------------------------------------
// Member function compute_energy calculates the matrix element
// <psi|H|psi>/<psi|psi> where psi denotes any (true or trial;
// normalized or unnormalized) state vector:

double quantum_2Dwavefunction::compute_energy(
   const complex curr_value[nxbins_max][nybins_max])
{
   double magsq[nxbins_max][nybins_max];
   double KEdensity[nxbins_max][nybins_max];
   double PEdensity[nxbins_max][nybins_max];
   complex curr_term;
   complex curr_tilde[nxbins_max][nybins_max];

   fouriertransform(curr_value,curr_tilde);
   for (int i=0; i<nxbins; i++)
   {
      double kx=kx_lo+i*delta_kx;
      for (int j=0; j<nybins; j++)
      {
         double ky=ky_lo+j*delta_ky;
         curr_term=curr_tilde[i][j].Conjugate()*
            (sqr(2*PI*kx)+sqr(2*PI*ky))*curr_tilde[i][j];
         KEdensity[i][j]=curr_term.getreal();
      }
   }

   for (int i=0; i<nxbins; i++)
   {
      double x=xlo+i*deltax;
      for (int j=0; j<nybins; j++)
      {
         double y=ylo+j*deltay;
         double V=potential(x,y);
         
// Set position space density to zero at points where potential is
// infinite.  On 4/25/02, we experimented with removing this
// constraint due to the QFP's potential values being very large and
// exceeding 1E8.  

//         if (fabs(V) >= POSITIVEINFINITY)
//         {
//            PEdensity[i][j]=0;
//            magsq[i][j]=0;
//         }
//         else
         {
            curr_term=curr_value[i][j].Conjugate()*V*curr_value[i][j];
            PEdensity[i][j]=curr_term.getreal();
            magsq[i][j]=sqr(curr_value[i][j].getmod());
         }
      } // loop over index j
   } // loop over index i

   double KE=double_simpsonsum(KEdensity)*delta_kx*delta_ky;
   double PE=double_simpsonsum(PEdensity)*deltax*deltay;
   double curr_norm=double_simpsonsum(magsq)*deltax*deltay;
   double Etot=(KE+PE)/curr_norm;
   return Etot;
}

// =====================================================================
// Wavefunction dumping and restoration member functions:
// =====================================================================

// Member function dump_eigenfunction writes out the contents of the
// eigenfunction specified by quantum numbers m and n to a gzipped
// file for later retrieval by subroutine restore_eigenfunction:

void quantum_2Dwavefunction::dump_eigenfunction(int m,int n)
{
   string suffix=stringfunc::number_to_string(m)+
      stringfunc::number_to_string(n);
   string eigendir=sysfunc::get_projectsrootdir()
      +"src/mains/quantum/eigenfunctions/2D/";
   if (!filefunc::direxist(eigendir))
   {
      string unixcommandstr="mkdir -p "+eigendir;
      sysfunc::unix_command(unixcommandstr);
   }
   string dumpfilename=
      eigendir+potentialfunc::get_potential_str(potential_type)
      +"_"+suffix+".dump";
   ofstream dumpstream;
   filefunc::openfile(dumpfilename,dumpstream);

   for (int i=0; i<nxbins_max; i++)
   {
      for (int j=0; j<nybins_max; j++)
      {
         dumpstream << energy_eigenstate[m][n][i][j].getreal();
         dumpstream << " ";
         dumpstream << energy_eigenstate[m][n][i][j].getimag();
         dumpstream << " ";
      }
      dumpstream << endl;
   }
   filefunc::closefile(dumpfilename,dumpstream);
   filefunc::gzip_file(dumpfilename);
}

// ---------------------------------------------------------------------
// Member function restore_eigenfunction retrieves the eigenfunction
// data stored within gzipped textfiles by subroutine
// dump_eigenfunction.  This subroutine returns true or false,
// depending upon whether or not a dump file exists.

bool quantum_2Dwavefunction::restore_eigenfunction(int m,int n)
{
   double x,y;
   string suffix,eigendir,gzipped_dumpfilename,dumpfilename;
   ifstream restorestream;
   ofstream dumpstream;

   eigendir=sysfunc::get_projectsrootdir()+
      "src/mains/quantum/eigenfunctions/2D/";
   suffix=stringfunc::number_to_string(m)+stringfunc::number_to_string(n);
   dumpfilename=eigendir+potentialfunc::get_potential_str(potential_type)
      +"_"+suffix+".dump";
   gzipped_dumpfilename=dumpfilename+".gz";

   if (filefunc::fileexist(gzipped_dumpfilename))
   {
      cout << "Reading from dumpfile eigenstate (" << m << "," << n 
           << "):" << endl;

      filefunc::gunzip_file(gzipped_dumpfilename);
      filefunc::openfile(dumpfilename,restorestream);
      for (int i=0; i<nxbins_max; i++)
      {
         for (int j=0; j<nybins_max; j++)
         {
            restorestream >> x;
            restorestream >> y;
            energy_eigenstate[m][n][i][j]=complex(x,y);
         } // loop over index j
      } // loop over index i
      filefunc::closefile(dumpfilename,restorestream);
      filefunc::gzip_file(dumpfilename);

      return true;
   }
   else
   {
      return false;
   } // fileexist(gzipped_dumpfilename) conditional
}

// ---------------------------------------------------------------------
// Member function dump_wavefunction writes out the contents of the
// wavefunction contained within input array curr_value to an output
// file for later retrieval by subroutine restore_wavefunction:

void quantum_2Dwavefunction::dump_wavefunction(
   const complex curr_value[nxbins_max][nybins_max])
{
   outputfunc::newline();
   cout << "Dumping contents of current wavefunction:" << endl;
   outputfunc::newline();
   
   string dumpfilename=imagedir+"wavefunction.dump";
   ofstream dumpstream;
   filefunc::openfile(dumpfilename,dumpstream);

// First save current time into dump file:

   dumpstream << t << endl;

   for (int i=0; i<nxbins_max; i++)
   {
      for (int j=0; j<nybins_max; j++)
      {
         dumpstream << curr_value[i][j].getreal();
         dumpstream << " ";
         dumpstream << curr_value[i][j].getimag();
         dumpstream << " ";
      }
      dumpstream << endl;
   }
   filefunc::closefile(dumpfilename,dumpstream);
   filefunc::gzip_file(dumpfilename);
}

// ---------------------------------------------------------------------
// Member function restore_wavefunction retrieves the wavefunction
// data stored within gzipped textfiles by subroutine
// dump_wavefunction.  This subroutine returns true or false,
// depending upon whether or not a dump file exists.

bool quantum_2Dwavefunction::restore_wavefunction(
   bool input_param_file,string inputline[],int currlinenumber,
   complex curr_value[nxbins_max][nybins_max])
{
   string outputline[10];
   ifstream restorestream;

   outputfunc::newline();
   outputline[0]="Enter path relative to src/mains/quantum/images/";
   outputline[1]="for dumpfile subdirectory:";
   string dumpdir=stringfunc::mygetstring(
      2,outputline,input_param_file,inputline,currlinenumber);
   string dumpfilename=sysfunc::get_projectsrootdir()
      +"src/mains/quantum/images/"+dumpdir+"/wavefunction.dump";
   string gzipped_dumpfilename=dumpfilename+".gz";
   ofstream dumpstream;
   filefunc::openfile(dumpfilename,dumpstream);

   bool contents_restored=false;
   if (filefunc::fileexist(gzipped_dumpfilename))
   {
      outputfunc::newline();
      cout << "Restoring contents of dumped wavefunction:" << endl;
      outputfunc::newline();

      filefunc::gunzip_file(gzipped_dumpfilename);
      filefunc::openfile(dumpfilename,restorestream);

// Read in wavefunction time from dumpfile:

      restorestream >> t;
      
      for (int i=0; i<nxbins_max; i++)
      {
         for (int j=0; j<nybins_max; j++)
         {
            double x,y;
            restorestream >> x;
            restorestream >> y;
            curr_value[i][j]=complex(x,y);
            prev_arg[i][j]=curr_arg[i][j]=curr_value[i][j].getarg();
         }  // loop over index j
      }  // loop over index i
      filefunc::closefile(dumpfilename,restorestream);
      filefunc::gzip_file(dumpfilename);
      contents_restored=true;
   }
   return contents_restored;
}

