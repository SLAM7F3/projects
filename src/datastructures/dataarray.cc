// Note added on 6/19/07: Must either eliminate this class' dependence
// upon image/TwoDarray or else move this class out of
// src/datastructures into src/image.

// ==========================================================================
// Dataarray class member function definitions
// ==========================================================================
// Last modified on 5/22/05; 12/4/10
// ==========================================================================

#include "math/basic_math.h"
#include "datastructures/dataarray.h"
#include "general/filefuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"
#include "image/TwoDarray.h"

using std::string;
using std::ostream;
using std::ios;
using std::cout;
using std::endl;

const int dataarray::N_EXTRAINFO_LINES=50;
const int dataarray::NSUBTICS=5;
const int dataarray::NCOLORS=100;
const int dataarray::NPRECISION=5;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void dataarray::initialize_member_objects()
{
   pagetitle=subtitle="";
   print_storylines=true;
   plot_only_points=false;
   histogram_flag=false;
   error_bars_flag=false;
   log_xaxis=false;
   xticlabel=yticlabel="";
   points_color=colorfunc::null;
   xlabelsize=ylabelsize=1.3;
   charsize=1.3;
   X=NULL;
   T_ptr=NULL;
}

void dataarray::allocate_member_objects()
{
   extrainfo=new string[N_EXTRAINFO_LINES];
   extraline=new string[N_EXTRAINFO_LINES];
   colorlabel=new string[NCOLORS];
}		       

dataarray::dataarray(void)
{
   initialize_member_objects();
   allocate_member_objects();
}

// Copy constructor:

dataarray::dataarray(const dataarray& d)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(d);
}

dataarray::dataarray(double minx,double delta,const twoDarray& Tdata)
{
   initialize_member_objects();
   allocate_member_objects();

   narrays=Tdata.get_mdim();
   npoints=Tdata.get_ndim();
   new_clear_array(X,npoints);
   T_ptr=new twoDarray(narrays,npoints);

   for (int i=0; i<N_EXTRAINFO_LINES; i++)
   {
      extrainfo[i]="";
      extraline[i]="";
   }
   for (int j=0; j<npoints; j++)
   {
      X[j]=minx+double(j*delta);
   }
   swap_axes=false;
   xmin=X[0];
   xmax=X[npoints-1];
   xnorm=1;
   xtic=xsubtic=ytic=ysubtic=0;
   yplotminval=yplotmaxval=0;
   maxval_real=NEGATIVEINFINITY;
   minval_real=POSITIVEINFINITY;
   thickness=1;

   for (int i=0; i<narrays; i++)
   {
      for (int j=0; j<npoints; j++)
      {
         T_ptr->put(i,j,Tdata.get(i,j));
      }
   }
   find_max_min_vals(maxval_real,minval_real);
}

dataarray::~dataarray()
{
   delete [] extrainfo;
   delete [] extraline;
   delete [] colorlabel;
   delete [] X;
   extrainfo=NULL;
   extraline=NULL;
   colorlabel=NULL;
   if (T_ptr != NULL)
   {
      delete T_ptr;
      T_ptr=NULL;
   }
}

// ---------------------------------------------------------------------
void dataarray::docopy(const dataarray& d)
{
   swap_axes=d.swap_axes;
   print_storylines=d.print_storylines;
   plot_only_points=d.plot_only_points;
   histogram_flag=d.histogram_flag;
   error_bars_flag=d.error_bars_flag;
   log_xaxis=d.log_xaxis;
   datafilenamestr=d.datafilenamestr;
   pagetitle=d.pagetitle;
   title=d.title;
   subtitle=d.subtitle;
   xlabel=d.xlabel;
   ylabel=d.ylabel;
   xticlabel=d.xticlabel;
   yticlabel=d.yticlabel;
   charsize=d.charsize;
   points_color=d.points_color;

   for (int i=0; i<N_EXTRAINFO_LINES; i++)
   {
      extrainfo[i]=d.extrainfo[i];
      extraline[i]=d.extraline[i];
   }
   for (int i=0; i<NCOLORS; i++)
   {
      colorlabel[i]=d.colorlabel[i];
   }
   narrays=d.narrays;
   npoints=d.npoints;
   thickness=d.thickness;
   swap_axes=d.swap_axes;
   maxval_real=d.maxval_real;
   minval_real=d.minval_real;
   xnorm=d.xnorm;
   xmax=d.xmax;
   xmin=d.xmin;
   E=d.E;
   xtic=d.xtic;
   xsubtic=d.xsubtic;
   ytic=d.ytic;
   ysubtic=d.ysubtic;
   yplotmaxval=d.yplotmaxval;
   yplotminval=d.yplotminval;
   xlabelsize=d.xlabelsize;
   ylabelsize=d.ylabelsize;

   if (X != NULL)
   {
      for (int i=0; i<npoints; i++) X[i]=d.X[i];
   }
   if (T_ptr != NULL) *T_ptr=*(d.T_ptr);
}	

// Overload = operator:

dataarray& dataarray::operator= (const dataarray& d)
{
   if (this==&d) return *this;
   docopy(d);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const dataarray& d)
{
   outstream << "datafilenamestr = " << d.datafilenamestr << endl;
   outstream << "narrays = " << d.narrays << endl;
   outstream << "npoints = " << d.npoints << endl;
   outstream << "maxval_real = " << d.maxval_real << endl;
   outstream << "minval_real = " << d.minval_real << endl;
   outstream << "xnorm = " << d.xnorm << endl;
   outstream << "xmax = " << d.xmax << endl;
   outstream << "xmin = " << d.xmin << endl;
   for (int j=0; j<20; j++)
   {
      outstream << "j = " << j << " X[j] = " << d.X[j] << endl;
   }

   if (d.T_ptr != NULL)
   {
      for (int i=0; i<d.narrays; i++)
      {
         for (int j=0; j<d.npoints; j++)
         {
            outstream << "i = " << i << " j = " << j << " value = " 
                      << d.T_ptr->get(i,j) << endl;
         }
      }
   }

   return(outstream);
}

// ==========================================================================
// Dataarray manipulation member functions
// ==========================================================================

void dataarray::find_max_min_vals(double& maxval,double& minval) const
{
   double currvalue;
   maxval=NEGATIVEINFINITY;
   minval=POSITIVEINFINITY;

   for (int i=0; i<narrays; i++)
   {
      for (int j=0; j<npoints; j++)
      {
         currvalue=T_ptr->get(i,j);
         if (currvalue > maxval && currvalue < POSITIVEINFINITY) 
            maxval=currvalue;
         if (currvalue < minval && currvalue > NEGATIVEINFINITY) 
            minval=currvalue;
      }
   }
}

// In this overloaded version of find_max_min_vals, the dataarray is
// assumed to contain just a single row of data.  The maximum and
// minimum values within this single row are returned in maxval and
// minval.  The corresponding max_bin and min_bin locations within the
// dataarray are also returned:

void dataarray::find_max_min_vals(double& maxval,double& minval,
                                  int& max_bin,int& min_bin) const
{
   double currvalue;
   maxval=NEGATIVEINFINITY;
   minval=POSITIVEINFINITY;

   for (int j=0; j<npoints; j++)
   {
      currvalue=T_ptr->get(0,j);
      if (currvalue > maxval && currvalue < POSITIVEINFINITY) 
      {
         maxval=currvalue;
         max_bin=j;
      }
      
      if (currvalue < minval && currvalue > NEGATIVEINFINITY) 
      {
         minval=currvalue;
         min_bin=j;
      }
   }
}

void dataarray::find_max_min_vals(
   int start_bin,int stop_bin,double& maxval,double& minval) const
{
   double currvalue;
   maxval=NEGATIVEINFINITY;
   minval=POSITIVEINFINITY;

   for (int i=0; i<narrays; i++)
   {
      for (int j=start_bin; j<stop_bin; j++)
      {
         currvalue=T_ptr->get(i,j);
         if (currvalue > maxval && currvalue < POSITIVEINFINITY) 
            maxval=currvalue;
         if (currvalue < minval && currvalue > NEGATIVEINFINITY) 
            minval=currvalue;
      }
   }
}

void dataarray::find_max_min_vals(
   double& maxval,double& minval,int start_array,int stop_array) const
{
   double currvalue;
   maxval=NEGATIVEINFINITY;
   minval=POSITIVEINFINITY;

   for (int i=start_array; i<stop_array; i++)
   {
      for (int j=0; j<npoints; j++)
      {
         currvalue=T_ptr->get(i,j);
         if (currvalue > maxval && currvalue < POSITIVEINFINITY) 
            maxval=currvalue;
         if (currvalue < minval && currvalue > NEGATIVEINFINITY) 
            minval=currvalue;
      }
   }
}

// ---------------------------------------------------------------------
void dataarray::rescale_val(double a)
{
   double currvalue;
   
   maxval_real=NEGATIVEINFINITY;
   minval_real=POSITIVEINFINITY;
   for (int i=0; i<narrays; i++)
   {
      for (int j=0; j<npoints; j++)
      {

// Rescale values within dataarrays provided that they do not equal
// +/- "infinity"

         currvalue=T_ptr->get(i,j);
         if (currvalue < POSITIVEINFINITY && currvalue > NEGATIVEINFINITY)
         {
            currvalue *= a;
            T_ptr->put(i,j,currvalue);
         }
         
         if (currvalue > maxval_real && currvalue < POSITIVEINFINITY) 
            maxval_real=currvalue;
         if (currvalue < minval_real && currvalue  > NEGATIVEINFINITY) 
            minval_real=currvalue;
      }
   }
}

// ---------------------------------------------------------------------
void dataarray::add_val(double a)
{
   double currvalue;

   maxval_real=NEGATIVEINFINITY;
   minval_real=POSITIVEINFINITY;
   for (int i=0; i<narrays; i++)
   {
      for (int j=0; j<npoints; j++)
      {

// Add constant a to elements within dataarrays provided that they do
// not equal +/- "infinity"

         currvalue=T_ptr->get(i,j);
         if (currvalue < POSITIVEINFINITY && currvalue > NEGATIVEINFINITY)
         {
            currvalue += a;
            T_ptr->put(i,j,currvalue);
         }
         
         if (currvalue > maxval_real && currvalue < POSITIVEINFINITY) 
            maxval_real=currvalue;
         if (currvalue < minval_real && currvalue > NEGATIVEINFINITY) 
            minval_real=currvalue;
      }
   }
}

// ---------------------------------------------------------------------
// Member function opendatafile opens up a datafile and initializes
// it.

void dataarray::opendatafile()
{
   if (filefunc::openfile(datafilenamestr+".meta",datastream))
   {
      datafileheader();
   }
}

void dataarray::closedatafile() 
{
   filefunc::closefile(datafilenamestr+".meta",datastream);
}

// ---------------------------------------------------------------------
// Member function datafileheader writes out preliminary header
// information at top of dataarray output file to set up for meta
// plotting.

void dataarray::datafileheader()
{
   double minval,maxval,xdiff,valdiff;
   double xstep,ystep;

// Two sets of "story" commands cannot be requested within one plot.
// So to get more than one column of story output to appear, we need
// to create dummy plots with no content and then call the story
// command.  Title, x axis and y axis calls are mandatory when setting
// up a plot.
   
   if (extrainfo[0] != "")
   {
      datastream << "title ''" << endl;
      datastream << "x axis min 0 max 0.0001" << endl;
      datastream << "y axis min 0 max 0.0001" << endl;
      
      int i=0;
      while (i < N_EXTRAINFO_LINES)
      {
         if (extrainfo[i] != "")
         {
            datastream << "story '"+extrainfo[i]+"'" << endl;
         }
         i++;
      }
      datastream << "charsize 0.8" << endl;
      datastream << "storyloc 7 6.25" << endl;
      datastream << "" << endl;
   }

   if (pagetitle != "")
   {
      datastream << "pagetitle '"+pagetitle+"'" << endl;
   }
   datastream << "title '"+title+"'" << endl;
   if (subtitle != "")
   {
      datastream << "subtitle '"+subtitle+"'" << endl;
   }

// Extend x bounds by half a bin for histogram plots:

   if (histogram_flag)
   {
      xmin -= 0.5*(X[1]-X[0]);
      xmax += 0.5*(X[1]-X[0]);
   }
   datastream << "x axis min "+stringfunc::number_to_string(xmin)+" max "
	+stringfunc::number_to_string(xmax) << endl;
   datastream << "label '"+xlabel+"'" << endl;

// Allow for manual overriding of automatic x axis tic settings

   datastream << "charsize "+stringfunc::number_to_string(charsize) << endl;
   if (xtic==0)
   {

// If xmax > 0 and xmin < 0, use larger absolute value of xmax
// and xmin to set tics rather than their difference:

      if (xmax*xmin > 0) 
      {
         xdiff=xmax-xmin;
      }
      else
      {
         xdiff=basic_math::max(fabs(xmax),fabs(xmin));
      }
      datastream << "tics "+stringfunc::number_to_string(trunclog(xdiff))+" "
         +stringfunc::number_to_string(trunclog(xdiff)/NSUBTICS) << endl;
   }
   else
   {
      datastream << "tics "+stringfunc::scinumber_to_string(xtic)
         +" "+stringfunc::scinumber_to_string(xsubtic) << endl;
   }

   minval=xnorm*minval_real;
   maxval=xnorm*maxval_real;
   minval=minval-0.2*fabs(minval);
   maxval=maxval+0.2*fabs(maxval);

// Allow for overriding of automatic y axis min/max value setting:
      
   if (yplotmaxval != 0) maxval=yplotmaxval;
   if (yplotminval != 0) minval=yplotminval;

   datastream << "y axis min "+stringfunc::scinumber_to_string(minval)
      +" max "+stringfunc::scinumber_to_string(maxval) << endl;
   datastream << "label '"+ylabel+"'" << endl;

// Allow for manual overriding of y axis tic settings

   datastream << "charsize "+stringfunc::number_to_string(charsize) << endl;
   if (ytic != 0)
   {
      datastream << "tics "+stringfunc::scinumber_to_string(ytic)
         +" "+stringfunc::scinumber_to_string(ysubtic) << endl;
   }

// If maxval > 0 and minval < 0, use larger absolute value of maxval
// and minval to set tics rather than their difference:

   else
   {
      if (maxval*minval > 0) 
      {
         valdiff=(maxval-minval);
      }
      else
      {
         valdiff=basic_math::max(fabs(maxval),fabs(minval));
      }
      datastream << "tics "+stringfunc::scinumber_to_string(trunclog(valdiff))
         +" "+stringfunc::scinumber_to_string(trunclog(valdiff)/NSUBTICS) 
                 << endl;
   }

   if (print_storylines)
   {
      outputfunc::print_filename_and_date(datastream,datafilenamestr);
   }
   xstep=(xmax-xmin)/10;
   if (narrays < 10)
   {
      ystep=(maxval-minval)/10;
   }
   else
   {
      ystep=(maxval-minval)/narrays;
   }
   
   for (int i=0; i<narrays; i++)
   {
      datastream << "textcolor "+colorfunc::getcolor(i) << endl;
      datastream << "textsize 1.5" << endl;
      datastream << "text "+stringfunc::number_to_string(xmin+xstep)+" "
         +stringfunc::number_to_string(maxval-(i+1)*ystep)+" '"
         +colorlabel[i]+"'" << endl;
   }
   datastream << endl;
}

// ---------------------------------------------------------------------
// Member function singletfile_header writes out preliminary header
// information at top of meta file to set up for viewing a single
// dataarray:

void dataarray::singletfile_header()
{
   double minval,maxval,valdiff;
   minval=maxval=valdiff=0;

// Two sets of "story" commands cannot be requested within one plot.
// So to get more than one column of story output to appear, we need
// to create dummy plots with no content and then call the story
// command.  Title, x axis and y axis calls are mandatory when setting
// up a plot.
   
   if (extrainfo[0] != "")
   {
      datastream << "title ''" << endl;
      datastream << "x axis min 0 max 0.0001" << endl;
      datastream << "y axis min 0 max 0.0001" << endl;
      
      int i=0;
      while (i < N_EXTRAINFO_LINES)
      {
         if (extrainfo[i] != "")
         {
            datastream << "story '"+extrainfo[i]+"'" << endl;
         }
         i++;
      }
      datastream << "charsize 0.8" << endl;
      datastream << "storyloc 7 6.25" << endl;
      datastream << "" << endl;
   }

   if (pagetitle != "")
   {
      datastream << "pagetitle '"+pagetitle+"'" << endl;
   }
   datastream << "title '"+title+"'" << endl;
   if (subtitle != "")
   {
      datastream << "subtitle '"+subtitle+"'" << endl;
   }
   
   datastream << "x axis min "+stringfunc::number_to_string(xmin)
      +" max "+stringfunc::number_to_string(xmax) << endl;
   datastream << "label '"+xlabel+"'" << endl;

// Display x axis values on log scale if boolean flag log_xaxis == true:

   if (log_xaxis) 
   {
      datastream << "log" << endl;
   }

// Allow for manual overriding of horizontal axis tic settings

   datastream << "charsize "+stringfunc::number_to_string(charsize) << endl;
   if (xtic==0)
   {
      datastream << "tics "+stringfunc::number_to_string(trunclog(xmax))+" "
         +stringfunc::number_to_string(trunclog(xmax)/NSUBTICS) << endl;
   }
   else
   {
      datastream << "tics "+stringfunc::scinumber_to_string(xtic)
         +" "+stringfunc::scinumber_to_string(xsubtic) << endl;
      if (xticlabel != "")
      {
         datastream << " " << xticlabel;
      }
   }  

// Allow for overriding of automatic y axis min/max value setting:
      
   if (yplotmaxval != 0) maxval=yplotmaxval;
   if (yplotminval != 0) minval=yplotminval;

   datastream << "y axis min "+stringfunc::scinumber_to_string(minval)
      +" max "+stringfunc::scinumber_to_string(maxval) << endl;
   datastream << "label '"+ylabel+"'" << endl;

// Allow for manual overriding of vertical axis tic settings
 
   datastream << "charsize "+stringfunc::number_to_string(charsize) << endl;
   if (ytic != 0)
   {
      datastream << "tics "+stringfunc::scinumber_to_string(ytic)
         +" "+stringfunc::scinumber_to_string(ysubtic);
      if (yticlabel != "")
      {
         datastream << " " << yticlabel;
      }
      datastream << endl;
   }

// If maxval > 0 and minval < 0, use larger absolute value of maxval
// and minval to set tics rather than their difference:

   else
   {
      if (maxval*minval > 0) 
      {
         valdiff=(maxval-minval);
      }
      else
      {
         valdiff=basic_math::max(fabs(maxval),fabs(minval));
      }
      datastream << "tics "+stringfunc::scinumber_to_string(trunclog(valdiff))
         +" "+stringfunc::scinumber_to_string(trunclog(valdiff)/NSUBTICS);
      if (yticlabel != "")
      {
         datastream << " " << yticlabel;
      }
      datastream << endl;
   }

// Write out filename and date at top of metafile:

   datastream << "story 'Filename = "+datafilenamestr+"'" << endl;
   datastream << "story '"+timefunc::getcurrdate()+"'" << endl;
   datastream << "storyloc -1.5 6.25 " << endl;
   datastream << endl;
}

// ---------------------------------------------------------------------
// Member function doubletfile_header writes out preliminary header
// information at top of meta file to set up for viewing 2 dataarrays
// side-by-side:

void dataarray::doubletfile_header(int doublet_pair_member)
{
   double minval,maxval,valdiff;
   minval=maxval=valdiff=0;

   if (doublet_pair_member==0)
   {
      if (pagetitle != "")
      {
         datastream << "pagetitle '"+pagetitle+"'" << endl;
      }
   }
   else if (doublet_pair_member==1)
   {
      datastream << endl;
   }
   datastream << "title '"+title+"'" << endl;
   if (subtitle != "")
   {
      datastream << "subtitle '"+subtitle+"'" << endl;
   }
   
   datastream << "size 4 4" << endl;

   if (doublet_pair_member==0)
   {
      datastream << "physor 1.1 2" << endl;
   }
   else if (doublet_pair_member==1)
   {
      datastream << "physor 6 2" << endl;
   }

   datastream << "x axis min "+stringfunc::number_to_string(xmin)
      +" max "+stringfunc::number_to_string(xmax) << endl;
   datastream << "label '"+xlabel+"'" << endl;
   if (xlabelsize != 1) datastream << "labelsize " << xlabelsize << endl;

// Allow for manual overriding of horizontal axis tic settings

   datastream << "charsize "+stringfunc::number_to_string(charsize) << endl;
   if (xtic==0)
   {
      datastream << "tics "+stringfunc::number_to_string(trunclog(xmax))+" "
         +stringfunc::number_to_string(trunclog(xmax)/NSUBTICS) << endl;
   }
   else
   {
      datastream << "tics "+stringfunc::scinumber_to_string(xtic)
         +" "+stringfunc::scinumber_to_string(xsubtic) << endl;
      if (xticlabel != "")
      {
         datastream << " " << xticlabel;
      }
   }  

// Allow for overriding of automatic y axis min/max value setting:
      
   if (yplotmaxval != 0) maxval=yplotmaxval;
   if (yplotminval != 0) minval=yplotminval;

   datastream << "y axis min "+stringfunc::scinumber_to_string(minval)
      +" max "+stringfunc::scinumber_to_string(maxval) << endl;
   datastream << "label '"+ylabel+"'" << endl;
   if (ylabelsize != 1) datastream << "labelsize " << ylabelsize << endl;

// Allow for manual overriding of vertical axis tic settings
 
   datastream << "charsize "+stringfunc::number_to_string(charsize) << endl;
   if (ytic != 0)
   {
      datastream << "tics "+stringfunc::scinumber_to_string(ytic)
         +" "+stringfunc::scinumber_to_string(ysubtic);
      if (yticlabel != "")
      {
         datastream << " " << yticlabel;
      }
      datastream << endl;
   }

// If maxval > 0 and minval < 0, use larger absolute value of maxval
// and minval to set tics rather than their difference:

   else
   {
      if (maxval*minval > 0) 
      {
         valdiff=(maxval-minval);
      }
      else
      {
         valdiff=basic_math::max(fabs(maxval),fabs(minval));
      }
      datastream << "tics "+stringfunc::scinumber_to_string(trunclog(valdiff))
         +" "+stringfunc::scinumber_to_string(trunclog(valdiff)/NSUBTICS);
      if (yticlabel != "")
      {
         datastream << " " << yticlabel;
      }
      datastream << endl;
   }

// Write out filename and date at top of metafile:

//   if (doublet_pair_member==0)
//   {
//      datastream << "story 'Filename = "+datafilenamestr+"'" << endl;
//      datastream << "story '"+timefunc::getcurrdate()+"'" << endl;
//      datastream << "storyloc -0.7 6.25 " << endl;
//      datastream << endl;
//   }
}

// ---------------------------------------------------------------------
// Member function writedatarray writes to metafile output the
// information stored within *T_ptr:

void dataarray::writedataarray()
{
   writedataarray(0);
}

// ---------------------------------------------------------------------
// In this overloaded version of writedataarray, the values of the
// dependent x-axis variables are shifted by some specified amount
// before being written out to file.  We incorporated this feature on
// 10/30/00 in order to allow for image numbers which internally start
// at value 0 to be displayed as starting from 1 instead.

void dataarray::writedataarray(double xshift)
{
   double currvalue,error;
   string unixcommandstr;
   
   datastream.setf(ios::fixed);
   datastream.setf(ios::showpoint);  
   datastream.precision(NPRECISION);

// Allow user to add extra lines into datastream by hand before the
// data stored within dataarray is written out to file:

   int i=0;
   while(i < N_EXTRAINFO_LINES && extraline[i] != "")
   {
      datastream << extraline[i] << endl;
      i++;
   }
   datastream << endl;

// If error_bars_flag==true, we assume that the dataarray contains
// only two rows.  The contents of row[1] are assumed to be the errors
// for the values contained within row[0]:

   if (error_bars_flag)
   {
      i=0;
      datastream << endl;
      datastream << "error_bars" << endl;
      datastream << "mstyle 0" << endl;
      datastream << "markcolor "+colorfunc::getcolor(i) << endl;
      datastream << "linecolor "+colorfunc::getcolor(i) << endl;;
      datastream << "data" << endl;

      for (int j=0; j<npoints; j++)
      {
         currvalue=T_ptr->get(i,j);
         error=T_ptr->get(i+1,j);

// Skip over any points whose absolute value >= POSITIVEINFINITY

         if (fabs(currvalue) < POSITIVEINFINITY)
         {
            datastream << X[j]+xshift << "\t\t" << currvalue
                       << "\t\t" << error << "\t\t" << error << endl;
         }
      }
      datastream << "end" << endl;
   }
   else
   {
      for (i=0; i<narrays; i++)
      {
         datastream << endl;
         if (histogram_flag)
         {
            datastream << "barchart" << endl;
         }
         else
         {
            datastream << "curve" << endl;
            if (thickness != 1) datastream << "thick " << thickness << endl;
         }
         
         if (plot_only_points)
         {
            datastream << "marks -1" << endl;
            if (points_color != colorfunc::null)
            {
               datastream << "color "+colorfunc::get_colorstr(points_color)
                          << endl;
            }
            else
            {
               datastream << "color "+colorfunc::getcolor(i) << endl;
            }
         }
         else if (!histogram_flag)
         {
            datastream << "color "+colorfunc::getcolor(i) << endl;
         }
         datastream << endl;
              
         for (int j=0; j<npoints; j++)
         {

// Skip over any points whose absolute value >= POSITIVEINFINITY

            currvalue=T_ptr->get(i,j);
            if (fabs(currvalue) < POSITIVEINFINITY)
            {

// Swap x and y values if swap_axes flag is set to true:

               if (swap_axes)
               {
                  datastream << xnorm*currvalue << "\t\t" 
                             << X[j]+xshift << endl;
               }
               else if (histogram_flag)
               {
                  datastream << "group loc " << X[j]+xshift
                             << " width " << 0.9*(X[1]-X[0])
                             << " blue 0 "
                             << currvalue << endl;
               }
               else
               {
                  datastream << X[j]+xshift << "\t\t" << xnorm*currvalue
                             << endl;
               }
            } 
         } // loop over index j
      } // loop over index i
      if (histogram_flag) datastream << "end" << endl;
   } // error_bars_flag conditional

   filefunc::closefile(datafilenamestr+".meta",datastream);
}

// ---------------------------------------------------------------------
// In this overloaded version of writedataarray, the values of the
// dependent x-axis variables are shifted by some specified amount
// before being written out to file.  We incorporated this feature on
// 10/30/00 in order to allow for image numbers which internally start
// at value 0 to be displayed as starting from 1 instead.  The values
// within the dataarray row labeled by input parameter ncolorrow are
// NOT written out to the meta file.  Instead, all other datapoints
// within the other rows are colored according to the values specified
// within the particular color row.  This overloaded function was
// written on 5/3/01 with just the intent of differentially coloring
// discrete points within metafile output in order to indicate good
// and bad fit values:

void dataarray::writedataarray(double xshift,int ncolorrow)
{
   double currvalue,error;
   string unixcommandstr;
   
   datastream.setf(ios::fixed);
   datastream.setf(ios::showpoint);  
   datastream.precision(NPRECISION);

// Allow user to add extra lines into datastream by hand before the
// data stored within dataarray is written out to file:

   int i=0;
   while(i < N_EXTRAINFO_LINES && extraline[i] != "")
   {
      datastream << extraline[i] << endl;
      i++;
   }
   datastream << endl;

// If error_bars_flag==true, we assume that the dataarray contains
// only two rows.  The contents of row[1] are assumed to be the errors
// for the values contained within row[0]:

   if (error_bars_flag)
   {
      i=0;
      datastream << endl;
      datastream << "error_bars" << endl;
      datastream << "mstyle 0" << endl;
      datastream << "markcolor "+colorfunc::getcolor(i) << endl;
      datastream << "linecolor "+colorfunc::getcolor(i) << endl;;
      datastream << "data" << endl;

      for (int j=0; j<npoints; j++)
      {

// Skip over any points whose absolute value >= POSITIVEINFINITY

         currvalue=T_ptr->get(i,j);
         error=T_ptr->get(i+1,j);
         if (fabs(currvalue) < POSITIVEINFINITY)
         {
            datastream << X[j]+xshift << "\t\t" << currvalue
                       << "\t\t" << error << "\t\t" << error << endl;
         }
      }
      datastream << "end" << endl;
   }
   else
   {
      for (i=0; i<narrays; i++)
      {
         if (i != ncolorrow)
         {
            datastream << endl;

            for (int j=0; j<npoints; j++)
            {

// Skip over any points whose absolute value >= POSITIVEINFINITY

               currvalue=T_ptr->get(i,j);
               if (fabs(currvalue) < POSITIVEINFINITY)
               {
                  datastream << "curve marks -1 color "
                     +colorfunc::getcolor(int(T_ptr->get(ncolorrow,j)))+" ";
                  datastream << X[j]+xshift << "\t\t" 
                             << currvalue << endl;
               }
            } // loop over index j labeling point number within current row
         } // loop over index i label dataarray row number
      } // i != ncolorrow conditional
   } // error_bars_flag conditional
   
   filefunc::closefile(datafilenamestr+".meta",datastream);
}

// ---------------------------------------------------------------------
// Member function load_dataarray converts the entire contents of a
// meta file which has been read into the string array currline via
// our ReadInfile routine into a dataarray.  The number of lines
// within the string array currline must be passed as the input
// parameter nlines to this routine. The number of curves as well as
// the spacing in the x direction are returned as output parameters.

dataarray dataarray::load_dataarray(string currline[],int nlines) const
{
// First scan through contents of currline string array and determine
// number of separate dataarrays as well as the number of bins which
// each dataarray contains:

   int numberpoints=0;
   int numberarrays=0;
   int linenumber=0;
   while (linenumber < nlines)
   {
      while(!stringfunc::is_number(currline[linenumber]) && 
            linenumber < nlines)
      {
         linenumber++;
      }

      if (linenumber >= nlines)
      {
         break;      
      }
      else
      {
         cout << "First (x,y) pair encountered at linenumber "
              << linenumber << endl;
      }
      
      numberpoints=0;
      while(stringfunc::is_number(currline[linenumber]))
      {
         numberpoints++;
         linenumber++;
      }
      numberarrays++;
   }

// Next re-scan currline string array and load (X,Y) pairs into
// X[] and twoDarray:

   double currX,currX0,currX1;
   currX=currX0=currX1=0;
   twoDarray data_twoDarray(numberarrays,numberpoints);
   linenumber=0;
   while (linenumber < nlines)
   {
      while(!stringfunc::is_number(currline[linenumber]) && 
            linenumber < nlines)
         linenumber++;
      if (linenumber >= nlines) break;
      
      numberpoints=0;
      while(stringfunc::is_number(currline[linenumber]))
      {
         double curry;
         stringfunc::string_to_two_numbers(currline[linenumber],currX,curry);
         if (numberpoints==0)
         {
            currX0=currX;
         }
         else if (numberpoints==1)
         {
            currX1=currX;
         }
         data_twoDarray.put(numberarrays,numberpoints,curry);
         numberpoints++;
         linenumber++;
      }
      numberarrays++;
      outputfunc::newline();
   }

   cout << "Number of points = " << numberpoints << endl;
   cout << "Number of arrays = " << numberarrays << endl;
   cout << "X[0] = " << currX0 << endl;
   cout << "X[1] = " << currX1 << endl;

   return dataarray(currX0,currX1-currX0,data_twoDarray);
}




