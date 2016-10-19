// ==========================================================================
// Header file for METAFILE base class
// ==========================================================================
// Last modified on 3/21/07; 5/4/13; 11/1/15; 10/19/16
// ==========================================================================

#ifndef METAFILE_H
#define METAFILE_H

#include <fstream>
#include <string>
#include <vector>
#include "color/colorfuncs.h"

class metafile
{

  public:

// Initialization, constructor and destructor functions:

   metafile(void);
   metafile(const metafile& m);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~metafile();
   metafile& operator= (const metafile& m);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const metafile& m);
 
// Set and get member functions:

   void set_legend_flag(bool flag);
   void set_swap_axes(bool swap_axes_flag);
   void set_plot_only_points(bool plot_only_points_flag);
   void set_number_points(bool number_points_flag);
   void set_point_number_interval(int interval);
   void set_filename(std::string filename);
   void set_print_storylines_flag(bool storylines_flag);
   void set_title(std::string Title);
   void set_subtitle(std::string sub_title);
   void set_titles(std::string Title,std::string sub_title);
   void set_sizes(double x_size,double y_size);
   void set_xlabel(std::string x_label);
   void set_xbounds(double x_min,double x_max);
   void set_xtics(double x_tic,double x_subtic);
   void set_ylabel(std::string y_label);
   void set_labels(std::string x_label,std::string y_label);
   void set_ybounds(double y_min,double y_max);
   void set_ytic(double y_tic);
   void set_ysubtic(double y_subtic);
   void set_ytics(double y_tic,double y_subtic);
   void set_legend_params(double x,double y,double s);
   void set_legendlabel(std::string label);
   void set_nplots(int n_plots);
   void set_currplot(int curr_plot);
   void set_thickness(int Thickness);
   void set_style(int Style);
   void add_extrainfo(std::string infostr);
   void add_extraline(std::string linestr);
   void set_colored_curve_label(std::string colorstr);
   void set_colored_curve_label(std::string colorstr,colorfunc::Color lcolor);

   void set_parameters(
      std::string filename,std::string Title,std::string x_label,
      std::string y_label,double x_min,double x_max);
   void set_parameters(
      std::string filename,std::string Title,std::string x_label,
      std::string y_label,double x_min,double x_max,
      double y_min,double y_max);
   void set_parameters(
      std::string filename,std::string Title,std::string x_label,
      std::string y_label,
      double x_min,double x_max,
      double y_min,double y_max,double y_tic,double y_subtic);
   void set_parameters(
      std::string filename,std::string Title,std::string x_label,
      std::string y_label,
      double x_min,double x_max,double x_tic,double x_subtic,
      double y_min,double y_max,double y_tic,double y_subtic);
   void set_parameters(
      std::string filename,std::string Title,std::string x_label,
      std::string y_label,
      double x_min,double x_max,double x_size,double x_physor,
      double y_min,double y_max,double y_tic,double y_subtic,
      double y_size,double y_physor);
   void set_parameters(
      std::string filename,std::string Title,std::string x_label,
      std::string y_label,
      double x_min,double x_max,double x_tic,double x_subtic,
      double x_size,double x_physor,
      double y_min,double y_max,double y_tic,double y_subtic,
      double y_size,double y_physor);

   bool get_legend_flag() const;
   bool get_swap_axes() const;
   bool get_plot_only_points() const;
   bool get_number_points() const;
   int get_point_number_interval() const;
   std::string& get_filename();
   int get_thickness() const;
   int get_style() const;
   double get_xmin() const;
   double get_xmax() const;
   double get_ymin() const;
   double get_ymax() const;
   double get_xtic() const;
   double get_ytic() const;
   std::string get_legendlabel() const;
   std::string get_extraline(int i) const;
   std::ofstream& get_metastream();

// Plotting initialization member functions:

   void openmetafile();
   void write_header();
   void closemetafile();
   void appendmetafile();
   int set_metastream_precision();
   void add_extralines();

// Plotting member functions:

   void write_legendlabel(std::string label);
   void write_curve(double Xstart, double Xstop, const std::vector<double>& Y);
   void write_curve(double Xstart, double Xstop, const std::vector<double>& Y,
                    colorfunc::Color curve_color);
   void write_curve(const std::vector<double>& X,
                    const std::vector<double>& Y);
   void write_curve(
      const std::vector<double>& X,const std::vector<double>& Y,
      colorfunc::Color curve_color);
   void write_markers(const std::vector<double>& X,
                      const std::vector<double>& Y);
   void write_markers(std::string marker_color,
      const std::vector<double>& X,
      const std::vector<double>& Y);
   void write_markers(const std::vector<int>& labels,
                      const std::vector<double>& X,
                      const std::vector<double>& Y);

  private:

   bool legend_flag;	// true if legend is to be included in metafile
   bool swap_axes;	// swap x and y axes if this flag==true
   bool print_storylines;
   bool plot_only_points;
   bool number_points;	// flag controlling whether numals appear alongside
			//  discrete data points
   int point_number_interval;	// Controls point numbering frequency

// Parameter thickness controls curve line thickness.  Its default
// value equals 1:

   int thickness;

// Parameter style controls whether the curve is solid or dashed.  Its
// default 0 value corresponds to a solid curve:

   int style;

// Parameter nplots indicates how many separate plots are to be
// displayed together in a single metafile.  Its default values equals
// 1.  Parameter currplot acts as a plot counter index:

   int nplots,currplot;

   colorfunc::Color label_color;

   std::string metafilename;
   std::string pagetitle,title,subtitle,legendtitle;
   std::string xlabel,ylabel,colored_curve_label,legendlabel;
   std::vector<std::string> extrainfo;
   std::vector<std::string> extraline;

   double xmax,xmin,ymax,ymin;
   double xtic,xsubtic,xsize,xphysor;	
   double ytic,ysubtic,ysize,yphysor;		

// Coordinates for upper left corner of legend bounding box relative to 
// physor origin:

   double x_legend_posn,y_legend_posn;
   double legend_size; // Legend character size relative to unity default
   
   std::ofstream metastream;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const metafile& m);
   void include_legend_header();
   void include_colored_curve_labels();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void metafile::set_legend_flag(bool flag)
{
   legend_flag=flag;
}

inline void metafile::set_swap_axes(bool swap_axes_flag)
{
   swap_axes=swap_axes_flag;
}

inline void metafile::set_plot_only_points(bool plot_only_points_flag) 
{
   plot_only_points=plot_only_points_flag;
}

inline void metafile::set_number_points(bool number_points_flag) 
{
   number_points=number_points_flag;
}

inline void metafile::set_point_number_interval(int interval)
{
   point_number_interval=interval;
}

inline void metafile::set_filename(std::string filename)
{
   metafilename=filename;
}

inline void metafile::set_print_storylines_flag(bool storylines_flag)
{
   print_storylines=storylines_flag;
}

inline void metafile::set_title(std::string Title)
{
   title=Title;
}

inline void metafile::set_subtitle(std::string sub_title)
{
   subtitle=sub_title;
}

inline void metafile::set_titles(std::string Title,std::string sub_title)
{
   title=Title;
   subtitle=sub_title;
}

inline void metafile::set_sizes(double x_size,double y_size)
{
   xsize=x_size;
   ysize=y_size;
}

inline void metafile::set_labels(std::string x_label,std::string y_label)
{
   set_xlabel(x_label);
   set_ylabel(y_label);
}

inline void metafile::set_xlabel(std::string x_label)
{
   xlabel=x_label;
}

inline void metafile::set_xbounds(double x_min,double x_max)
{
   xmin=x_min;
   xmax=x_max;
}

inline void metafile::set_xtics(double x_tic,double x_subtic)
{
   xtic=x_tic;
   xsubtic=x_subtic;
}

inline void metafile::set_ylabel(std::string y_label)
{
   ylabel=y_label;
}

inline void metafile::set_ybounds(double y_min,double y_max)
{
   ymin=y_min;
   ymax=y_max;
}

inline void metafile::set_ytic(double y_tic)
{
   ytic=y_tic;
}

inline void metafile::set_ysubtic(double y_subtic)
{
   ysubtic=y_subtic;
}

inline void metafile::set_ytics(double y_tic,double y_subtic)
{
   ytic=y_tic;
   ysubtic=y_subtic;
}

inline void metafile::set_legend_params(double x,double y,double s)
{
   x_legend_posn=x;
   y_legend_posn=y;
   legend_size=s;
}

inline void metafile::set_legendlabel(std::string label)
{
   legendlabel=label;
}

inline void metafile::set_nplots(int n_plots)
{
   nplots=n_plots;
}

inline void metafile::set_currplot(int curr_plot)
{
   currplot=curr_plot;
}

inline void metafile::set_thickness(int Thickness)
{
   thickness=Thickness;
}

inline void metafile::set_style(int Style)
{
   style=Style;
}

inline void metafile::add_extrainfo(std::string infostr)
{
   extrainfo.push_back(infostr);
}

inline void metafile::add_extraline(std::string linestr)
{
   extraline.push_back(linestr);
}

inline void metafile::set_colored_curve_label(std::string colorstr)
{
   set_colored_curve_label(colorstr,colorfunc::get_color(currplot));
}

inline void metafile::set_colored_curve_label(
   std::string colorstr,colorfunc::Color lcolor)
{
   colored_curve_label=colorstr;
   label_color=lcolor;
}

inline void metafile::set_parameters(
   std::string filename,std::string Title,std::string x_label,
   std::string y_label,double x_min,double x_max)
{
   metafilename=filename;
   title=Title;
   xlabel=x_label;
   ylabel=y_label;
   xmin=x_min;
   xmax=x_max;
}

inline void metafile::set_parameters(
   std::string filename,std::string Title,std::string x_label,
   std::string y_label,double x_min,double x_max,double y_min,double y_max)
{
   set_parameters(filename,Title,x_label,y_label,x_min,x_max);
   ymin=y_min;
   ymax=y_max;
}

inline void metafile::set_parameters(
   std::string filename,std::string Title,std::string x_label,
   std::string y_label,double x_min,double x_max,
   double y_min,double y_max,double y_tic,double y_subtic)
{
   set_parameters(filename,Title,x_label,y_label,x_min,x_max,y_min,y_max);
   ytic=y_tic;
   ysubtic=y_subtic;
}

inline void metafile::set_parameters(
   std::string filename,std::string Title,
   std::string x_label,std::string y_label,
   double x_min,double x_max,double x_tic,double x_subtic,
   double y_min,double y_max,double y_tic,double y_subtic)
{
   set_parameters(
      filename,Title,x_label,y_label,
      x_min,x_max,y_min,y_max,y_tic,y_subtic);
   xtic=x_tic;
   xsubtic=x_subtic;
}

inline void metafile::set_parameters(
   std::string filename,std::string Title,
   std::string x_label,std::string y_label,
   double x_min,double x_max,double x_size,double x_physor,
   double y_min,double y_max,double y_tic,double y_subtic,
   double y_size,double y_physor)
{
   set_parameters(
      filename,Title,x_label,y_label,
      x_min,x_max,y_min,y_max,y_tic,y_subtic);
   xsize=x_size;
   xphysor=x_physor;
   ysize=y_size;
   yphysor=y_physor;
}

inline void metafile::set_parameters(
   std::string filename,std::string Title,
   std::string x_label,std::string y_label,
   double x_min,double x_max,double x_tic,double x_subtic,
   double x_size,double x_physor,
   double y_min,double y_max,double y_tic,double y_subtic,
   double y_size,double y_physor)
{
   set_parameters(
      filename,Title,x_label,y_label,
      x_min,x_max,x_tic,x_subtic,y_min,y_max,y_tic,y_subtic);
   xsize=x_size;
   xphysor=x_physor;
   ysize=y_size;
   yphysor=y_physor;
}

inline bool metafile::get_legend_flag() const
{
   return legend_flag;
}

inline bool metafile::get_swap_axes() const
{
   return swap_axes;
}

inline bool metafile::get_plot_only_points() const
{
   return plot_only_points;
}

inline bool metafile::get_number_points() const
{
   return number_points;
}

inline int metafile::get_point_number_interval() const
{
   return point_number_interval;
}

inline std::string& metafile::get_filename() 
{
   return metafilename;
}

inline int metafile::get_thickness() const
{
   return thickness;
}

inline int metafile::get_style() const
{
   return style;
}

inline double metafile::get_xmin() const
{
   return xmin;
}

inline double metafile::get_xmax() const
{
   return xmax;
}

inline double metafile::get_ymin() const
{
   return ymin;
}

inline double metafile::get_ymax() const
{
   return ymax;
}

inline double metafile::get_xtic() const
{
   return xtic;
}

inline double metafile::get_ytic() const
{
   return ytic;
}

inline std::string metafile::get_legendlabel() const
{
   return legendlabel;
}

inline std::string metafile::get_extraline(int i) const
{
   return extraline[i];
}

inline std::ofstream& metafile::get_metastream() 
{
   return metastream;
}

#endif // plot/metafile.h



