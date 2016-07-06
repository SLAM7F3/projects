// ==========================================================================
// QUANTUMIMAGE class member function definitions
// ==========================================================================
// Last modified on 5/22/05
// ==========================================================================

#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "quantum/quantumimage.h"
#include "general/stringfuncs.h"

using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

quantumimage::quantumimage(void)
{
}

// Copy constructor:

quantumimage::quantumimage(const quantumimage& q):
   quantum_wavefunction(q)
{
   docopy(q);
}

quantumimage::~quantumimage()
{
}

// ---------------------------------------------------------------------
void quantumimage::docopy(const quantumimage& q)
{
   nxbins_to_display=q.nxbins_to_display;
   nybins_to_display=q.nybins_to_display;
  
   kx_hi=q.kx_hi;
   kx_lo=q.kx_lo;
   delta_kx=q.delta_kx;
   ky_hi=q.ky_hi;
   ky_lo=q.ky_lo;
   delta_ky=q.delta_ky;
   xperiod=q.xperiod;
   yperiod=q.yperiod;
}	

// Overload = operator:

quantumimage& quantumimage::operator= (const quantumimage& q)
{
   if (this==&q) return *this;
   quantum_wavefunction::operator=(q);
   docopy(q);
   return *this;
}

// =====================================================================
// FFT member functions:
// =====================================================================

// Function fouriertransform returns the *true* momentum space Fourier
// transform H(kx,ky) of the 2D spatial signal contained within the
// complex value array.  The number of rows and columns within the
// incoming data array must be ODD!  Recall that the *discrete*
// fourier transform H(m,n) is related to the continuous true Fourier
// transform H(kx,ky) by H(kx,ky) = dx * dy * H(m,n) where dx and dy
// denote the sampling step sizes in the x and y directions.  As of
// 11/00, we take dx=dy.  We *do* include a factor of dx*dy into the
// momentum space tilde array values which are computed by this
// routine.

void quantumimage::fouriertransform(
   const complex value_in[nxbins_max][nybins_max],
   complex tilde_out[nxbins_max][nybins_max])
{
   const double EXTREMELY_TINY=1.0E-15;

   int iskip,jskip;
   complex (*tmp_value)[nybins]=new complex[nxbins][nybins];
   complex (*tmp_value2)[nybins]=new complex[nxbins][nybins];
   complex (*tmp_tilde)[nybins]=new complex[nxbins][nybins];
   complex (*tmp_tilde2)[nybins]=new complex[nxbins][nybins];

// Introduce arrays in_fftwnd and out_fftwnd of type fftw_complex in
// order to use the FFTW ("fastest fourier transform in the west")
// algorithms.  See section 2.1 within the FFTW user's manual:

   fftw_complex *in_fftwnd=new fftw_complex[nxbins*nybins];
   fftw_complex *out_fftwnd=new fftw_complex[nxbins*nybins];

// Wrap input data columns:

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins/2+1; i++)
      {
         iskip=i+nxbins/2;
         tmp_value[i][j]=value_in[iskip][j];
      }
      for (int i=0; i<nxbins/2; i++)
      {
         iskip=i+nxbins/2;
         tmp_value[iskip+1][j]=value_in[i][j];
      }
   }

// Wrap input data rows:

   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins/2+1; j++)
      {
         jskip=j+nybins/2;
         tmp_value2[i][j]=tmp_value[i][jskip];
      }
      for (int j=0; j<nybins/2; j++)
      {
         jskip=j+nybins/2;
         tmp_value2[i][jskip+1]=tmp_value[i][j];
      }
   }

// Copy real and imaginary values into in_fftwnd array in "row major
// order":

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins; i++)
      {
         in_fftwnd[i+nxbins*j].re=tmp_value2[i][j].x;
         in_fftwnd[i+nxbins*j].im=tmp_value2[i][j].y;
      }
   }

// Take Fourier transform using fftwnd:

   fftwnd_one(theforward_2d,in_fftwnd,NULL);
//   fftwnd_one(theforward_2d,in_fftwnd,out_fftwnd);

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins; i++)
      {
//         tmp_tilde2[i][j]=complex(out_fftwnd[i+nxbins*j].re,
//                                  out_fftwnd[i+nxbins*j].im);
         tmp_tilde2[i][j]=complex(in_fftwnd[i+nxbins*j].re,
                                  in_fftwnd[i+nxbins*j].im);
      }
   }
  
// Unwrap output data rows:

   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins/2+1; j++)
      {
         jskip=j+nybins/2;
         tmp_tilde[i][jskip]=tmp_tilde2[i][j];
      }
      for (int j=0; j<nybins/2; j++)
      {
         jskip=j+nybins/2;
         tmp_tilde[i][j]=tmp_tilde2[i][jskip+1];
      }
   }

// Unwrap output data columns and return results within array tilde.
// Multiply *discrete* fourier transform results by deltax*deltay to
// turn them into *true* continuous fourier transform values.  See eqn
// (12.1.8) in Numerical Recipes.

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins/2+1; i++)
      {
         iskip=i+nxbins/2;
         tilde_out[iskip][j]=deltax*deltay*tmp_tilde[i][j];
      }

      for (int i=0; i<nxbins/2; i++)
      {
         iskip=i+nxbins/2;
         tilde_out[i][j]=deltax*deltay*tmp_tilde[iskip+1][j];
      }
   }

// To minimize build-up of machine round-off errors, we null out any
// fourier transformed value less than EXTREMELY TINY:

   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins; j++)
      {
         if (fabs(tilde_out[i][j].x) < EXTREMELY_TINY) tilde_out[i][j].x=0;
         if (fabs(tilde_out[i][j].y) < EXTREMELY_TINY) tilde_out[i][j].y=0;
      }
   }

   delete [] tmp_value;
   delete [] tmp_value2;
   delete [] tmp_tilde;
   delete [] tmp_tilde2;
   delete [] in_fftwnd;
   delete [] out_fftwnd;
}

// ---------------------------------------------------------------------
// Function inversefouriertransform returns the *true* spatial domain
// inverse Fourier transform h(x,y) of the 2D momentum space signal
// contained within the complex array tilde.  The number of rows and
// columns within the incoming data array must be ODD!  Recall that
// the *discrete* inverse fourier transform H(m,n) is related to the
// continuous true inverse Fourier transform h(x,y) by h(x,y) =
// H(m,n)/(nxbins*nybins).  We *do* include a factor of
// 1/(nxbins*nybins) into the spatial domain value array computed by
// this subroutine.

void quantumimage::inversefouriertransform(
   const complex tilde_in[nxbins_max][nybins_max],
   complex value_out[nxbins_max][nybins_max])
{
   const double EXTREMELY_TINY=1.0E-15;

   int iskip,jskip;
   complex (*tmp_tilde)[nybins]=new complex[nxbins][nybins];
   complex (*tmp_tilde2)[nybins]=new complex[nxbins][nybins];
   complex (*tmp_value)[nybins]=new complex[nxbins][nybins];
   complex (*tmp_value2)[nybins]=new complex[nxbins][nybins];

// Introduce arrays in_fftwnd and out_fftwnd of type fftw_complex in
// order to use the FFTW ("fastest fourier transform in the west")
// algorithms.  See section 2.1 within the FFTW user's manual:

   fftw_complex *in_fftwnd=new fftw_complex[nxbins*nybins];
   fftw_complex *out_fftwnd=new fftw_complex[nxbins*nybins];

// Wrap input data columns.  Recall the tilde_in array contains *true*
// continuous fourier transform values.  We must first divide these by
// (deltax*deltay) to convert them into *discrete* fourier transform
// values before calling the inverse FFTW subroutine:

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins/2+1; i++)
      {
         iskip=i+nxbins/2;
         tmp_tilde[i][j]=tilde_in[iskip][j]/(deltax*deltay);
      }
      for (int i=0; i<nxbins/2; i++)
      {
         iskip=i+nxbins/2;
         tmp_tilde[iskip+1][j]=tilde_in[i][j]/(deltax*deltay);
      }
   }

// Wrap input data rows:

   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins/2+1; j++)
      {
         jskip=j+nybins/2;
         tmp_tilde2[i][j]=tmp_tilde[i][jskip];
      }
      for (int j=0; j<nybins/2; j++)
      {
         jskip=j+nybins/2;
         tmp_tilde2[i][jskip+1]=tmp_tilde[i][j];
      }
   }

// Copy real and imaginary values into in_fftwnd array in "row major
// order":

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins; i++)
      {
         in_fftwnd[i+nxbins*j].re=tmp_tilde2[i][j].x;
         in_fftwnd[i+nxbins*j].im=tmp_tilde2[i][j].y;
      }
   }

// Take Fourier transform using fftwnd:

   fftwnd_one(thebackward_2d,in_fftwnd,NULL);
//   fftwnd_one(thebackward_2d,in_fftwnd,out_fftwnd);

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins; i++)
      {
//         tmp_value2[i][j]=complex(out_fftwnd[i+nxbins*j].re,
//                                  out_fftwnd[i+nxbins*j].im);
         tmp_value2[i][j]=complex(in_fftwnd[i+nxbins*j].re,
                                  in_fftwnd[i+nxbins*j].im);
      }
   }
  
// Unwrap output data rows:

   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins/2+1; j++)
      {
         jskip=j+nybins/2;
         tmp_value[i][jskip]=tmp_value2[i][j];
      }
      for (int j=0; j<nybins/2; j++)
      {
         jskip=j+nybins/2;
         tmp_value[i][j]=tmp_value2[i][jskip+1];
      }
   }

// Unwrap output data columns and return results within array value.
// Multiply *discrete* inverse fourier transform results
// 1/(nxbins*nybins) to turn them into *true* continuous inverse
// fourier transform values.  See eqn (12.1.8) in Numerical Recipes.

   for (int j=0; j<nybins; j++)
   {
      for (int i=0; i<nxbins/2+1; i++)
      {
         iskip=i+nxbins/2;
         value_out[iskip][j]=tmp_value[i][j]/(nxbins*nybins);
      }

      for (int i=0; i<nxbins/2; i++)
      {
         iskip=i+nxbins/2;
         value_out[i][j]=tmp_value[iskip+1][j]/(nxbins*nybins);
      }
   }

// To minimize build-up of machine round-off errors, we null out any
// inverse fourier transformed value less than EXTREMELY TINY:

   for (int i=0; i<nxbins; i++)
   {
      for (int j=0; j<nybins; j++)
      {
         if (fabs(value_out[i][j].x) < EXTREMELY_TINY) value_out[i][j].x=0;
         if (fabs(value_out[i][j].y) < EXTREMELY_TINY) value_out[i][j].y=0;
      }
   }

   delete [] tmp_value;
   delete [] tmp_value2;
   delete [] tmp_tilde;
   delete [] tmp_tilde2;
   delete [] in_fftwnd;
   delete [] out_fftwnd;
}

// =====================================================================
// Meta file member functions:
// =====================================================================

// Member function label_axes writes label and tic information to
// metafile output.

void quantumimage::label_axes(
   double& xmin,double& xmax,double& ymin,double& ymax)
{
   if (xperiod < POSITIVEINFINITY)
   {

// We assume that periodic potentials have a 2*PI period:

      xmin=-PI;
      xmax=PI;
      imagestream << "x axis min "+stringfunc::number_to_string(xmin)
         +" max "+stringfunc::number_to_string(xmax) << endl;
      imagestream << "label '^g\152^u^-X^n'" << endl;
      imagestream << "tics "+stringfunc::number_to_string(PI/2)+" "
         +stringfunc::number_to_string(PI/4)
         +" [-^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u]" << endl;
   }
   else
   {
      xmin=display_frac*(xlo)-0.5*deltax;
      xmax=display_frac*(xhi)+0.5*deltax;
      imagestream << "x axis min "+stringfunc::number_to_string(xmin)
         +" max "+stringfunc::number_to_string(xmax) << endl;
      if (potential_type==potentialfunc::QFP)
      {
         imagestream << "label '^g\152^u^-+^n'" << endl;
      }
      else
      {
         imagestream << "label 'X'" << endl;
      }
      
// Allow for manual overriding of automatic horizontal axis tic settings

      if (xtic==0)
      {
         double xtic0=trunclog(xmax);
         double logxtic0=log10(xtic0);
         if (logxtic0 < 0)
         {
            xtic=(fabs(logxtic0)+1)*xtic0;
         }
         else
         {
            xtic=xtic0;
         }
         xsubtic=xtic/NSUBTICS;         
         imagestream << "tics "+stringfunc::number_to_string(xtic)+" "
            +stringfunc::number_to_string(xsubtic) << endl;
      }
      else
      {
         imagestream << "tics "+stringfunc::scinumber_to_string(xtic)
            +" "+stringfunc::scinumber_to_string(xsubtic) << endl;
      }  
   }

   if (yperiod < POSITIVEINFINITY)
   {
      ymin=-PI;
      ymax=PI;
      imagestream << "y axis min "+stringfunc::number_to_string(ymin)
         +" max "+stringfunc::number_to_string(ymax) << endl;
      imagestream << "label '^g\152^u^-Y^n'" << endl;
      imagestream << "tics "+stringfunc::number_to_string(PI/2)+" "
         +stringfunc::number_to_string(PI/4)
         +" [-^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u]" << endl;
   }
   else
   {
      ymin=display_frac*ylo-0.5*deltay;
      ymax=display_frac*yhi+0.5*deltay;
      imagestream << "y axis min "+stringfunc::number_to_string(ymin)
         +" max "+stringfunc::number_to_string(ymax) << endl;
      if (potential_type==potentialfunc::QFP)
      {
         imagestream << "label '^g\152^u^--^n'" << endl;
      }
      else
      {
         imagestream << "label 'Y'" << endl;
      }
      
// Allow for manual overriding of automatic vertical axis tic settings

      if (ytic==0)
      {
         double ytic0=trunclog(ymax);
         double logytic0=log10(ytic0);
         if (logytic0 < 0)
         {
            ytic=(fabs(logytic0)+1)*ytic0;
         }
         else
         {
            ytic=ytic0;
         }
         ysubtic=ytic/NSUBTICS;
         imagestream << "tics "+stringfunc::number_to_string(ytic)+" "
            +stringfunc::number_to_string(ysubtic) << endl;
      }
      else
      {
         imagestream << "tics "+stringfunc::scinumber_to_string(ytic)
            +" "+stringfunc::scinumber_to_string(ysubtic) << endl;
      }
   }
}

// ---------------------------------------------------------------------
// Member function header_end writes image size and colortable
// information at the end of a metafile image's header prior to the
// intensity data section:

void quantumimage::header_end(double xmin,double xmax,double ymin,double ymax)
{
   imagestream << "image" << endl;

// Coordinates of lower left hand and upper right hand corners:

   if (domain_name==quantum::position_space)
   {
      imagestream << stringfunc::number_to_string(xmin) << "\t" 
                  << stringfunc::number_to_string(ymin) << endl;
      imagestream << stringfunc::number_to_string(xmax) << "\t" 
                  << stringfunc::number_to_string(ymax) << endl;
   }
   else if (domain_name==quantum::momentum_space)
   {
      imagestream << stringfunc::number_to_string(kx_lo) << "\t" 
                  << stringfunc::number_to_string(ky_lo) << endl;
      imagestream << stringfunc::number_to_string(kx_hi) << "\t" 
                  << stringfunc::number_to_string(ky_hi) << endl;
   }
   
// Number of pixels within a row and in a column.  The total number of
// pixels must not exceed 500,000:

   nxbins_to_display=basic_math::round(display_frac*nxbins);
   if (is_even(nxbins_to_display)) nxbins_to_display++;
   nybins_to_display=basic_math::round(display_frac*nybins);
   if (is_even(nybins_to_display)) nybins_to_display++;

   imagestream << endl;
   imagestream << stringfunc::number_to_string(nxbins_to_display) << endl;
   imagestream << stringfunc::number_to_string(nybins_to_display) << endl;
   imagestream << endl;
}

// ---------------------------------------------------------------------
// Member function append_dynamic_colortable adds a dynamically
// generated colortable into the current metafile output.  This
// particular subroutine should never be called for phase plots where
// the range automatically equals [-PI,PI].  But for magnitude plots,
// the maximum range goes like the system's dimensionless coupling
// constant alpha raised to the 1/8 power.

void quantumimage::append_dynamic_colortable(
   double height,double width,double legloc_x,double legloc_y)
{
   double min_value,max_value;
   double alpha=potential_param[0];	// dimensionless coupling

   max_value=pow(alpha,0.125);
   if (complex_plot_type==quantum::mag_phase || 
       complex_plot_type==quantum::sqd_amp ||
       complex_plot_type==quantum::energy_prob)
   {
      min_value=0;
   }
   else
   {
      min_value=-max_value;
   }
   generate_colortable(min_value,max_value,height,width,legloc_x,legloc_y);
   imagestream << endl;
   imagestream << "data" << endl;
}

// ---------------------------------------------------------------------
// Member function append_colortable inserts into an output metafile
// the colortable contents stored within some pre-existing file.

void quantumimage::append_colortable(
   double height,double width,double legloc_x,double legloc_y)
{
// Read in colortable information from colortable file.  We
// incorporate this info straight into the meta file, but we do NOT
// use it to generate a colorbar legend in order to avoid spurious
// white spots:

   myimage::insert_colortable_header(height,width,legloc_x,legloc_y);

   int nlines;
   string lines[100];
   filefunc::ReadInfile(colortablefilename,lines,nlines);
   for (int i=0; i<nlines; i++)
   {
      imagestream << lines[i] << endl;
   }
   imagestream << "data" << endl;
}

// ---------------------------------------------------------------------
void quantumimage::insert_colortable_head()
{
   imagestream << endl;
   imagestream << "# ====================================================="
               << endl;
   imagestream << "title ''" << endl;
   imagestream << "size 0 0" << endl;
   imagestream << "x axis min -100 max -99" << endl;
   imagestream << "y axis min -100 max -99" << endl;
   imagestream << "legend ''" << endl;
   imagestream << "image -100 -100 -99 -99 1 1" << endl;
}

// ---------------------------------------------------------------------
void quantumimage::insert_colortable_foot()
{
   imagestream << " 0 " << endl;
   imagestream << "# ====================================================="
               << endl;
   imagestream << endl;   
}

// ---------------------------------------------------------------------
void quantumimage::insert_nowhite_dynamic_colortable(
   double height,double width,double legloc_x,double legloc_y)
{
   insert_colortable_head();
   append_dynamic_colortable(height,width,legloc_x,legloc_y);
   insert_colortable_foot();
}

// ---------------------------------------------------------------------
// Member function insert_nowhite_colortable adds a colorbar legend
// which includes no white space into the output metafile.  After
// speaking with Iva Mooney on 4/24/00, we realized that we would have
// to include the following dumb cluge into our metafile output in
// order to circumvent a limitation of the metafile plotting program.

void quantumimage::insert_nowhite_colortable(
   double height,double width,double legloc_x,double legloc_y)
{
   insert_colortable_head();
   append_colortable(height,width,legloc_x,legloc_y);
   insert_colortable_foot();
}

// ---------------------------------------------------------------------
// Member function potential_header writes out preliminary potential
// energy header information:

void quantumimage::potential_header(string plot_type)
{
   double xmin,xmax,ymin,ymax;

   colortablefilename=sysfunc::get_projectsrootdir()+
      "src/quantum/colortables/colortable."+
      potentialfunc::get_potential_str(potential_type);

   if (plot_type=="triplet")
   {
      insert_nowhite_colortable(2,0.15,1,3.05);
   }
   else if (plot_type=="singlet")
   {
      insert_nowhite_colortable(6.2,0.3,3.8,3.1);
   }

   if (V_displayfactor != 1)
   {
      imagestream << "pagetitle '"+stringfunc::scinumber_to_string(
         V_displayfactor,3)+" * Potential'" << endl;
   }
   else
   {
      imagestream << "pagetitle 'Potential'" << endl;
   }
   imagestream << "title ' '" << endl;
   if (plot_type=="triplet")
   {
      imagestream << "size 2 2" << endl;
      imagestream << "physor 4.2 5.3" << endl;
   }
   else if (plot_type=="singlet")
   {
      imagestream << "size 6.5 6.5" << endl;
   }

   label_axes(xmin,xmax,ymin,ymax);

   domain_name=quantum::position_space;
   header_end(xmin,xmax,ymin,ymax);
   append_colortable(6.2,0.3,3.8,3.1);
}

// ---------------------------------------------------------------------
// Member function singletfile_header writes out preliminary header
// information at top of meta file to set up for viewing 1 image:

void quantumimage::singletfile_header(int imagenumber)
{
   if (complex_plot_type==quantum::sqd_amp)
   {
      colortablefilename=sysfunc::get_projectsrootdir()
         +"src/quantum/colortables/colortable.sqdamp";
   }
   insert_nowhite_colortable(6.2,0.3,3.8,3.1);
   
   if (complex_plot_type==quantum::sqd_amp)
   {
      imagestream << "title ' '" << endl;
   }
   imagestream << "size 6.5 6.5" << endl;

   double xmin,xmax,ymin,ymax;
   if (domain_name==quantum::position_space)
   {
      label_axes(xmin,xmax,ymin,ymax);
   }
   else if (domain_name==quantum::momentum_space)
   {
      imagestream << "x axis min "
         +stringfunc::number_to_string(kx_lo-0.5*delta_kx)
         +" max "+stringfunc::number_to_string(kx_hi+0.5*delta_kx) << endl;
      imagestream << "label 'Kx'" << endl;
      if (xtic==0)
      {
         imagestream << "tics "+stringfunc::number_to_string(
            trunclog(kx_hi-kx_lo))+" "
            +stringfunc::number_to_string(trunclog(kx_hi-kx_lo)/NSUBTICS) << endl;
      }
      else
      {
         imagestream << "tics "+stringfunc::scinumber_to_string(xtic)
            +" "+stringfunc::scinumber_to_string(xsubtic) << endl;
      }  

      imagestream << "y axis min "
         +stringfunc::number_to_string(ky_lo-0.5*delta_ky)
         +" max "+stringfunc::number_to_string(ky_hi+0.5*delta_ky) << endl;
      imagestream << "label 'Ky'" << endl;
      if (ytic==0)
      {
         imagestream << "tics "+stringfunc::number_to_string(
            trunclog(ky_hi-ky_lo))+" "
            +stringfunc::number_to_string(trunclog(ky_hi-ky_lo)/NSUBTICS) << endl;
      }
      else
      {
         imagestream << "tics "+stringfunc::scinumber_to_string(ytic)
            +" "+stringfunc::scinumber_to_string(ysubtic) << endl;
      }
   }
   header_end(xmin,xmax,ymin,ymax);
   append_dynamic_colortable(6.2,0.3,3.8,3.1);
}

// ---------------------------------------------------------------------
// Member function doubletfile_header writes out preliminary header
// information at top of meta file to set up for viewing 2 images
// side-by-side.  As of 6/10/02, this subroutine has been specialized
// for displaying potential energy vs squared amplitude information
// for viewgraph generation purposes.

void quantumimage::doubletfile_header(int doublet_pair_member)
{
   double xmin,xmax,ymin,ymax;
   xmin=xmax=ymin=ymax=0;

// After speaking with Iva Mooney on 4/24/00, we realized that we would have
// to include the following dumb cluge into our metafile output in order to
// circumvent a limitation of the metafile plotting program.  The lines
// below draw a colorbar legend which includes no white space.  Recall that
// white regions within myimages represent zones where a missile cannot
// intercept a target.  Since the miss distance in such zones is effectively
// infinite, there should be no white appearing within the miss distance
// colorbar legend.  We need to incorporate a separate colortable after this
// section of metafile code which does include white RGB values so that
// portions of the myimage itself can be colored white in the appropriate
// dead zones.  But we cannot use that 2nd colortable for the color legend
// bar, for some white will inevitably appear within the legend.

   if (doublet_pair_member==0)
   {
      if (complex_plot_type==quantum::mag_phase)
      {
         colortablefilename=sysfunc::get_projectsrootdir()
            +"src/quantum/colortables/colortable.magnitude";
      }
      else if (complex_plot_type==quantum::real_imag)
      {
         colortablefilename=sysfunc::get_projectsrootdir()
            +"src/quantum/colortables/colortable.realimag";
      }
      else if (complex_plot_type==quantum::energy_prob)
      {
         colortablefilename=sysfunc::get_projectsrootdir()
            +"src/quantum/colortables/colortable."+
            potentialfunc::get_potential_str(potential_type);
      }
      insert_nowhite_colortable(4,0.15,-0.7,1.75); 
   }
   else if (doublet_pair_member==1)
   {
      if (complex_plot_type==quantum::mag_phase)
      {
         colortablefilename=sysfunc::get_projectsrootdir()
            +"src/quantum/colortables/colortable.phase";
      }
      else if (complex_plot_type==quantum::real_imag)
      {
         colortablefilename=sysfunc::get_projectsrootdir()
            +"src/quantum/colortables/colortable.realimag";
      }
      else if (complex_plot_type==quantum::energy_prob)
      {
         colortablefilename=sysfunc::get_projectsrootdir()
            +"src/quantum/colortables/colortable.sqdamp";
      }
      insert_nowhite_colortable(4,0.15,4.25,1.75); 
   }

   if (doublet_pair_member==0)
   {
      if (pagetitle != "")
      {
         imagestream << "pagetitle '"+pagetitle+"'" << endl;
      }
      if (complex_plot_type==quantum::mag_phase)
      {
         imagestream << "title 'Magnitude'" << endl;
      }
      else if (complex_plot_type==quantum::real_imag)
      {
         imagestream << "title 'Real Part'" << endl;
      }
      else if (complex_plot_type==quantum::energy_prob)
      {
         imagestream << "title 'Potential Energy'" << endl;
      }
      imagestream << "size 4 4" << endl;
      imagestream << "physor 0.7 2" << endl;
   }
   else if (doublet_pair_member==1)
   {
      if (complex_plot_type==quantum::mag_phase)
      {
         imagestream << "title 'Phase'" << endl;
      }
      else if (complex_plot_type==quantum::real_imag)
      {
         imagestream << "title 'Imaginary Part'" << endl;
      }
      else if (complex_plot_type==quantum::energy_prob)
      {
         imagestream << "title 'Probability Density'" << endl;
      }
      imagestream << "size 4 4" << endl;
      imagestream << "physor 5.6 2" << endl;
   }

   if (domain_name==quantum::position_space)
   {

// We assume that period for periodic potentials is the interval [-PI,PI]:

      if (xperiod<POSITIVEINFINITY)
      {
         xmin=-PI;
         xmax=PI;
         imagestream << "x axis min "+stringfunc::number_to_string(xmin)
            +" max "+stringfunc::number_to_string(xmax) << endl;
         imagestream << "label '^g\152^u^-X^n'" << endl;
         imagestream << "tics "+stringfunc::number_to_string(PI/2)+" "
            +stringfunc::number_to_string(PI/4)
            +" [-^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u]" << endl;
      }
      else
      {
         xmin=display_frac*(xlo)-0.5*deltax;
         xmax=display_frac*(xhi)+0.5*deltax;
         imagestream << "x axis min "+stringfunc::number_to_string(xmin)
            +" max "+stringfunc::number_to_string(xmax) << endl;

         if (potential_type==potentialfunc::QFP)
         {
            imagestream << "label '^g\152^u^-+^n'" << endl;
         }
         else
         {
            imagestream << "label 'X'" << endl;
         }

// Allow for manual overriding of automatic horizontal axis tic settings

         if (xtic==0)
         {
            double xtic0=trunclog(xmax);
            double logxtic0=log10(xtic0);
            if (logxtic0 < 0)
            {
               xtic=(fabs(logxtic0)+1)*xtic0;
            }
            else
            {
               xtic=xtic0;
            }
            xsubtic=xtic/NSUBTICS;
            imagestream << "tics "+stringfunc::number_to_string(xtic)+" "
               +stringfunc::number_to_string(xsubtic) << endl;
         }
         else
         {
            imagestream << "tics "+stringfunc::scinumber_to_string(xtic)
               +" "+stringfunc::scinumber_to_string(xsubtic) << endl;
         }  
      }
   }
   else if (domain_name==quantum::momentum_space)
   {
      imagestream << "x axis min "
         +stringfunc::number_to_string(kx_lo-0.5*delta_kx)
         +" max "+stringfunc::number_to_string(kx_hi+0.5*delta_kx) << endl;
      imagestream << "label 'Kx'" << endl;
      if (xtic==0)
      {
         imagestream << "tics "+stringfunc::number_to_string(
            trunclog(kx_hi-kx_lo))+" "
            +stringfunc::number_to_string(trunclog(kx_hi-kx_lo)/NSUBTICS) 
                     << endl;
      }
      else
      {
         imagestream << "tics "+stringfunc::scinumber_to_string(xtic)
            +" "+stringfunc::scinumber_to_string(xsubtic) << endl;
      }  
   }

// Write out y axis information irrespective of doublet pair member value:

   if (domain_name==quantum::position_space)
   {

// We assume that period for periodic potentials is the interval [-PI,PI]:

      if (yperiod<POSITIVEINFINITY)
      {
         ymin=-PI;
         ymax=PI;
         imagestream << "y axis min "+stringfunc::number_to_string(ymin)
            +" max "+stringfunc::number_to_string(ymax) << endl;
      }
      else
      {
         ymin=display_frac*ylo-0.5*deltay;
         ymax=display_frac*yhi+0.5*deltay;
         imagestream << "y axis min "+stringfunc::number_to_string(ymin)
            +" max "+stringfunc::number_to_string(ymax) << endl;
      }
   }
   else if (domain_name==quantum::momentum_space)
   {
      imagestream << "y axis min "
         +stringfunc::number_to_string(ky_lo-0.5*delta_ky)
         +" max "+stringfunc::number_to_string(ky_hi+0.5*delta_ky) << endl;
   }

// Add vertical labels and tics only to first member of doublet pair:

   if (doublet_pair_member==0)
   {
      if (domain_name==quantum::position_space)
      {
         if (yperiod<POSITIVEINFINITY)
         {
            imagestream << "label '^g\152^u^-Y^n'" << endl;
            imagestream << "tics "+stringfunc::number_to_string(PI/2)+" "
               +stringfunc::number_to_string(PI/4)
               +" [-^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u]" << endl;
         }
         else
         {
            if (potential_type==potentialfunc::QFP)
            {
               imagestream << "label '^g\152^u^--^n'" << endl;
            }
            else
            {
               imagestream << "label 'Y'" << endl;
            }

// Allow for manual overriding of automatic vertical axis tic settings

            if (ytic==0)
            {
               double ytic0=trunclog(ymax);
               double logytic0=log10(ytic0);
               if (logytic0 < 0)
               {
                  ytic=(fabs(logytic0)+1)*ytic0;
               }
               else
               {
                  ytic=ytic0;
               }
               ysubtic=ytic/NSUBTICS;
               imagestream << "tics "+stringfunc::number_to_string(ytic)+" "
                  +stringfunc::number_to_string(ysubtic) << endl;
            }
            else
            {
               imagestream << "tics "+stringfunc::scinumber_to_string(ytic)
                  +" "+stringfunc::scinumber_to_string(ysubtic) << endl;
            }
         }
      }
      else if (domain_name==quantum::momentum_space)
      {
         imagestream << "label 'Ky'" << endl;
         if (ytic==0)
         {
            imagestream << "tics "+stringfunc::number_to_string(
               trunclog(ky_hi-ky_lo))+" "
               +stringfunc::number_to_string(trunclog(ky_hi-ky_lo)/NSUBTICS) 
                        << endl;
         }
         else
         {
            imagestream << "tics "+stringfunc::scinumber_to_string(ytic)
               +" "+stringfunc::scinumber_to_string(ysubtic) << endl;
         }
      }
   }
   header_end(xmin,xmax,ymin,ymax);

   if (doublet_pair_member==0 && complex_plot_type==quantum::energy_prob)
   {
      append_colortable(4,0.15,-0.7,1.75);
   }
   else if (doublet_pair_member==1 && 
            complex_plot_type==quantum::energy_prob)
   {
      append_colortable(4,0.15,4.25,1.75);
//      append_dynamic_colortable(4,0.15,4.25,1.75);
   }
}

// ---------------------------------------------------------------------
// Member function tripletfileheader writes out preliminary header
// information at the top of a meta file to set up for viewing 2
// wavefunction images side-by-side underneath a 3rd image of the
// potential energy function:

void quantumimage::tripletfile_header(int triplet_pair_member)
{
   double xmin,xmax,ymin,ymax;
   xmin=xmax=ymin=ymax=0;

// After speaking with Iva Mooney on 4/24/00, we realized that we would have
// to include the following dumb cluge into our metafile output in order to
// circumvent a limitation of the metafile plotting program.  The lines
// below draw a colorbar legend which includes no white space.  Recall that
// white regions within myimages represent zones where a missile cannot
// intercept a target.  Since the miss distance in such zones is effectively
// infinite, there should be no white appearing within the miss distance
// colorbar legend.  We need to incorporate a separate colortable after this
// section of metafile code which does include white RGB values so that
// portions of the myimage itself can be colored white in the appropriate
// dead zones.  But we cannot use that 2nd colortable for the color legend
// bar, for some white will inevitably appear within the legend.

   if (triplet_pair_member==0)
   {
      colortablefilename=sysfunc::get_projectsrootdir()
         +"src/quantum/colortables/colortable_triplet.magnitude";
      insert_nowhite_colortable(3,0.15,-1,-0.25);
//      insert_nowhite_dynamic_colortable(3,0.15,-1,-0.25);
   }
   else
   {
      if (complex_plot_type==quantum::mag_phase)
      {
         colortablefilename=sysfunc::get_projectsrootdir()
            +"src/quantum/colortables/colortable_triplet.phase";
         insert_nowhite_colortable(3,0.15,3.7,-0.25);
      }
      else if (complex_plot_type==quantum::real_imag)
      {
         insert_nowhite_dynamic_colortable(3,0.15,3.7,-0.25);
      }
   }

   if (triplet_pair_member==0)
   {
      if (complex_plot_type==quantum::mag_phase)
      {
         imagestream << "title 'Magnitude'" << endl;
      }
      else if (complex_plot_type==quantum::real_imag)
      {
         imagestream << "title 'Real Part'" << endl;
      }
      imagestream << "size 3 3" << endl;
      imagestream << "physor 1.2 1" << endl;
   }
   else if (triplet_pair_member==1)
   {
      if (complex_plot_type==quantum::mag_phase)
      {
         imagestream << "title 'Phase'" << endl;
      }
      else if (complex_plot_type==quantum::real_imag)
      {
         imagestream << "title 'Imaginary Part'" << endl;
      }
      imagestream << "size 3 3" << endl;
      imagestream << "physor 6 1" << endl;
   }

   if (domain_name==quantum::position_space)
   {

// We assume that period for periodic potentials is the interval [-PI,PI]:

      if (xperiod < POSITIVEINFINITY)
      {
         xmin=-PI;
         xmax=PI;
         imagestream << "x axis min "+stringfunc::number_to_string(xmin)
            +" max "+stringfunc::number_to_string(xmax) << endl;
         imagestream << "label '^g\152^u^-X^n'" << endl;
         imagestream << "tics "+stringfunc::number_to_string(PI/2)+" "
            +stringfunc::number_to_string(PI/4)
            +" [-^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u]" << endl;
      }
      else
      {
         xmin=display_frac*(xlo)-0.5*deltax;
         xmax=display_frac*(xhi)+0.5*deltax;
         imagestream << "x axis min "+stringfunc::number_to_string(xmin)
            +" max "+stringfunc::number_to_string(xmax) << endl;

         if (potential_type==potentialfunc::QFP)
         {
            imagestream << "label '^g\152^u^-+^n'" << endl;
         }
         else
         {
            imagestream << "label 'X'" << endl;
         }
         
// Allow for manual overriding of automatic horizontal axis tic settings

         if (xtic==0)
         {
            double xtic0=trunclog(xmax);
            double logxtic0=log10(xtic0);
            if (logxtic0 < 0)
            {
               xtic=(fabs(logxtic0)+1)*xtic0;
            }
            else
            {
               xtic=xtic0;
            }
            xsubtic=xtic/NSUBTICS;
            imagestream << "tics "+stringfunc::number_to_string(xtic)+" "
               +stringfunc::number_to_string(xsubtic) << endl;
         }
         else
         {
            imagestream << "tics "+stringfunc::scinumber_to_string(xtic)
               +" "+stringfunc::scinumber_to_string(xsubtic) << endl;
         }  
      }
   }
   else if (domain_name==quantum::momentum_space)
   {
      imagestream << "x axis min "
         +stringfunc::number_to_string(basic_math::round(kx_lo-0.5*delta_kx))
         +" max "+stringfunc::number_to_string(
            basic_math::round(kx_hi+0.5*delta_kx)) << endl;
      imagestream << "label 'Kx'" << endl;
      if (xtic==0)
      {
         imagestream << "tics "+stringfunc::number_to_string(
            trunclog(kx_hi-kx_lo))+" "
            +stringfunc::number_to_string(trunclog(kx_hi-kx_lo)/NSUBTICS) 
                     << endl;
      }
      else
      {
         imagestream << "tics "+stringfunc::scinumber_to_string(xtic)
            +" "+stringfunc::scinumber_to_string(xsubtic) << endl;
      }  
   }

// Write out y axis information irrespective of triplet pair member value:

   if (domain_name==quantum::position_space)
   {

// We assume that period for periodic potentials is the interval [-PI,PI]:

      if (yperiod < POSITIVEINFINITY)
      {
         ymin=-PI;
         ymax=PI;
         imagestream << "y axis min "+stringfunc::number_to_string(ymin)
            +" max "+stringfunc::number_to_string(ymax) << endl;
      }
      else
      {
         ymin=display_frac*ylo-0.5*deltay;
         ymax=display_frac*yhi+0.5*deltay;
         imagestream << "y axis min "+stringfunc::number_to_string(ymin)
            +" max "+stringfunc::number_to_string(ymax) << endl;
      }
   }
   else if (domain_name==quantum::momentum_space)
   {
      imagestream << "y axis min "
         +stringfunc::number_to_string(basic_math::round(ky_lo-0.5*delta_ky))
         +" max "+stringfunc::number_to_string(
            basic_math::round(ky_hi+0.5*delta_ky)) << endl;
   }

// Add vertical labels and tics only to first member of triplet pair:

   if (triplet_pair_member==0)
   {
      if (domain_name==quantum::position_space)
      {
         if (yperiod < POSITIVEINFINITY)
         {
            imagestream << "label '^g\152^u^-Y^n'" << endl;
            imagestream << "tics "+stringfunc::number_to_string(PI/2)+" "
               +stringfunc::number_to_string(PI/4)
               +" [-^g\160^u -^g\160^u/2 0 ^g\160^u/2 ^g\160^u]" << endl;
         }
         else
         {
            if (potential_type==potentialfunc::QFP)
            {
               imagestream << "label '^g\152^u^--^n'" << endl;
            }
            else
            {
               imagestream << "label 'Y'" << endl;
            }
            
// Allow for manual overriding of automatic vertical axis tic settings

            if (ytic==0)
            {
               double ytic0=trunclog(ymax);
               double logytic0=log10(ytic0);
               if (logytic0 < 0)
               {
                  ytic=(fabs(logytic0)+1)*ytic0;
               }
               else
               {
                  ytic=ytic0;
               }
               ysubtic=ytic/NSUBTICS;
               imagestream << "tics "+stringfunc::number_to_string(ytic)+" "
                  +stringfunc::number_to_string(ysubtic) << endl;
            }
            else
            {
               imagestream << "tics "+stringfunc::scinumber_to_string(ytic)
                  +" "+stringfunc::scinumber_to_string(ysubtic) << endl;
            }
         }
      }
      else if (domain_name==quantum::momentum_space)
      {
         imagestream << "label 'Ky'" << endl;
         if (ytic==0)
         {
            imagestream << "tics "+stringfunc::number_to_string(
               trunclog(ky_hi-ky_lo))+" "
               +stringfunc::number_to_string(trunclog(ky_hi-ky_lo)/NSUBTICS) 
                        << endl;
         }
         else
         {
            imagestream << "tics "+stringfunc::scinumber_to_string(ytic)
               +" "+stringfunc::scinumber_to_string(ysubtic) << endl;
         }
      }
   }

   header_end(xmin,xmax,ymin,ymax);
   if (triplet_pair_member==1 && complex_plot_type==quantum::mag_phase)
   {
      append_colortable(3,0.15,3.7,-0.25);
   }
   else
   {
      append_colortable(3,0.15,-1,-0.25);
//      append_dynamic_colortable(3,0.15,-1+4.7*triplet_pair_member,-0.25);
   }
}

// ---------------------------------------------------------------------
void quantumimage::potential_footer()
{
   if (potential_type==potentialfunc::QFP)
   {
      imagestream << "title ''" << endl;
      imagestream << "size 9.1 7.6" << endl;
      imagestream << "physor 1.3 0.4" << endl;
      imagestream << "x axis min 0 max 1" << endl;
      imagestream << "y axis min 0 max 1" << endl;
      imagestream << "textcolor 'purple'" << endl;
      imagestream << "textsize 1.5" << endl;
      imagestream << "text 0 -0.02 'phiQ = "+
         stringfunc::number_to_string(potential_param[1]*180/PI,2)+"'" << endl;
      imagestream << "text 0.3 -0.02 'betaQ = "+
         stringfunc::number_to_string(potential_param[2],2)+"'" << endl;
      imagestream << "text 0.55 -0.02 'phiE = "+
         stringfunc::number_to_string(potential_param[3]*180/PI,2)+"'" << endl;
      imagestream << "text 0.85 -0.02 'betaE = "+
         stringfunc::number_to_string(potential_param[4],2)+"'" << endl;
   }
}

// ---------------------------------------------------------------------
void quantumimage::singletfile_footer(double E)
{
   imagestream << "title ''" << endl;
   imagestream << "size 9.1 7.4" << endl;
   imagestream << "physor 1.3 0.4" << endl;
   imagestream << "x axis min 0 max 1" << endl;
   imagestream << "y axis min 0 max 1" << endl;
   imagestream << "textsize 3" << endl;
   imagestream << "text 0.3 1.06 'Probability Density'" << endl;
   imagestream << "textcolor 'purple'" << endl;
   imagestream << "textsize 1.5" << endl;
   imagestream << "text 0.27 -0.02 't = "+
      stringfunc::number_to_string(t,2)+"'" << endl;
   imagestream << "text 0.61 -0.02 'E = "
      +stringfunc::number_to_string(E,2)+"'" << endl;
}

// ---------------------------------------------------------------------
void quantumimage::doubletfile_footer(int imagenumber,double E)
{
   imagestream << "title ''" << endl;
   imagestream << "size 10.5 5" << endl;
   imagestream << "physor 0.1 1.4" << endl;
   imagestream << "x axis min 0 max 1" << endl;
   imagestream << "y axis min 0 max 1" << endl;
   imagestream << "textsize 2" << endl;
   imagestream << "text 0.43 1.15 'Image "+stringfunc::number_to_string(
      imagenumber)+"'" << endl;
   imagestream << "textcolor 'purple'" << endl;
   imagestream << "textsize 1.5" << endl;
   imagestream << "text 0.22 -0.15 't = "+
      stringfunc::number_to_string(t,2)+"'" << endl;
   imagestream << "text 0.45 -0.15 'E = "
      +stringfunc::number_to_string(E,3)+"'" << endl;
   imagestream << "text 0.67 -0.15 'Normalization = "
      +stringfunc::number_to_string(norm,3)+"'" << endl;
}

// ---------------------------------------------------------------------
void quantumimage::tripletfile_footer(int imagenumber,double E)
{
   imagestream << "title ''" << endl;
   imagestream << "size 9.5 4" << endl;
   imagestream << "physor 0.45 0.45" << endl;
   imagestream << "x axis min 0 max 1" << endl;
   imagestream << "y axis min 0 max 1" << endl;
   imagestream << "textsize 2" << endl;
   imagestream << "text 0.45 1.97 'Image "+stringfunc::number_to_string(
      imagenumber-1)+"'" << endl;

   imagestream << "textsize 1.5" << endl;
   imagestream << "text 0.33 1.05 'Position Space Wavefunction'" << endl;

   imagestream << "textcolor 'purple'" << endl;
   imagestream << "textsize 1.5" << endl;
   imagestream << "text 0.05 -0.05 't = "+
      stringfunc::scinumber_to_string(t,2)+"'" << endl;
   imagestream << "text 0.3 -0.05 'alpha = "+
      stringfunc::scinumber_to_string(potential_param[0],1)+"'" << endl;
   imagestream << "text 0.6 -0.05 'E = "+
      stringfunc::number_to_string(E,3)+"'" << endl;
   imagestream << "text 0.85 -0.05 'Norm = "+
      stringfunc::number_to_string(norm,3)+"'" << endl;
}



