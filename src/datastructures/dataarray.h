// Note added on 12/19/02: Eventually, we should add a metafile
// pointer as a member variable of this class.  We should then move as
// many metafile plot related member variables out of this class and
// use instead those belonging to the metafile class as possible!

// ==========================================================================
// Header file for DATAARRAY class which is intended to be an
// all-purpose storage and display object for multiple real
// one-dimensional arrays.
// ==========================================================================
// Last modified on 1/12/04
// ==========================================================================

#ifndef DATAARRAY_H
#define DATAARRAY_H

#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <string>
#include <new>
#include <sstream>	// Needed for using string streams within 
//#include <strstream>	// Needed for using string streams within 
			// "number_to_string"
#include "color/colorfuncs.h"
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

class dataarray
{
  private:

   void initialize_member_objects();
   void allocate_member_objects();
   void docopy(const dataarray& d);

  protected:

   static const int N_EXTRAINFO_LINES;
   static const int NSUBTICS;
   static const int NCOLORS;
   static const int NPRECISION;

   colorfunc::Color points_color;

   double charsize;

  public:

   bool swap_axes;	// Swap x and y axes if reverse_axes = true
   bool print_storylines;
   bool plot_only_points;
   bool histogram_flag;
   bool error_bars_flag; // Entries in row[1] are errors for values in row[0]
			 //  if error_bars_flag==true;
   bool log_xaxis;	// Display independent variables on log x axis scale
			//  if log_xaxis==true;

   std::string datafilenamestr,pagetitle,title,subtitle,xlabel,ylabel;
   std::string xticlabel,yticlabel;
   std::string *extrainfo;
   std::string *extraline;
   std::string *colorlabel;
   std::ofstream datastream;

// Parameter narrays corresponds to the number of separate arrays
// which the dataarray object holds.  Parameter npoints corresponds to
// the number of bins contained within each individual array:

   int narrays,npoints;

// Parameter thickness controls curve line thickness.  Its default
// value equals 1:

   int thickness;

// For presentation purposes only, we sometimes choose to *display*
// DATAARRAY results with renormalized y axis values.  However, the
// DATAARRAY y axis data itself is *not* renormalized by xnorm.  The
// only subroutines in which xnorm should appear are those where data
// are written out to files, and not in those where data are
// calculated:

   double xnorm;
   
   double maxval_real,minval_real,xmax,xmin,E;
   double xtic,xsubtic,ytic,ysubtic;
   double yplotmaxval,yplotminval;
   double xlabelsize,ylabelsize;
   double *X;
   twoDarray* T_ptr;
   
// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:

   dataarray(void);
   dataarray(const dataarray& d);
   dataarray(double minx,double delta,const twoDarray& Tdata);
   virtual ~dataarray();
   dataarray& operator= (const dataarray& d);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const dataarray& d);

// Set & get member functions:

   void set_charsize(double size);
   double get_charsize() const;

// Dataarray manipulation member functions:

   void find_max_min_vals(double& maxval,double& minval) const;
   void find_max_min_vals(double& maxval,double& minval,
                          int& max_bin,int& min_bin) const;
   void find_max_min_vals(int start_bin,int stop_bin,
                          double& maxval,double& minval) const;
   void find_max_min_vals(double& maxval,double& minval,
                          int start_array,int stop_array) const;

   void rescale_val(double a);
   void add_val(double a);

   void opendatafile();
   void closedatafile();
   void datafileheader();
   void singletfile_header();
   void doubletfile_header(int doublet_pair_member);
   void writedataarray();
   void writedataarray(double xshift);
   void writedataarray(double xshift,int ncolorrow);
   dataarray load_dataarray(std::string currline[],int nlines) const;
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void dataarray::set_charsize(double size)
{
   charsize=size;
}

inline double dataarray::get_charsize() const
{
   return charsize;
}

#endif // datastructures/dataarray.h




