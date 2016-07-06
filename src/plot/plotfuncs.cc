// ==========================================================================
// PLOTFUNCS stand-alone methods
// ==========================================================================
// Last modified on 7/8/05; 2/26/06; 4/1/12
// ==========================================================================

#include <unistd.h>	// needed for sleep()
#include "math/basic_math.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "datastructures/Linkedlist.h"
#include "plot/metafile.h"
#include "datastructures/Mynode.h"
#include "general/outputfuncs.h"
#include "plot/plotfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "waveform/waveform.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ostream;
using std::ofstream;
using std::string;

namespace plotfunc
{

// ==========================================================================
// Waveform plotting methods
// ==========================================================================

   void plot_time_func_real(
      waveform& w,metafile* metafile_ptr,string filename,double display_frac)
      {
         double max_value_real,min_value_real;
         w.extremal_time_domain_function_values(
            max_value_real,min_value_real);
         metafile_ptr->set_parameters(
            filename,"Time domain function","Time (secs)",
            "Wavefunction real value",
            display_frac*w.get_tlo(),display_frac*w.get_thi(),
            1.2*min_value_real,1.2*max_value_real);

         metafile_ptr->openmetafile();
         metafile_ptr->write_header();
         write_time_domain_real_data(w,metafile_ptr);
         metafile_ptr->closemetafile();
         filefunc::meta_to_jpeg(metafile_ptr->get_filename());
      }

   void plot_time_func_imag(
      waveform& w,metafile* metafile_ptr,string filename,
      double display_frac)
      {
         double max_value_real,min_value_real;
         w.extremal_time_domain_function_values(
            max_value_real,min_value_real);
         metafile_ptr->set_parameters(
            filename,"Time domain function","Time (secs)",
            "Wavefunction imaginary value",
            display_frac*w.get_tlo(),display_frac*w.get_thi(),
            1.2*min_value_real,1.2*max_value_real);

         metafile_ptr->openmetafile();
         metafile_ptr->write_header();
         write_time_domain_imag_data(w,metafile_ptr);
         metafile_ptr->closemetafile();
         filefunc::meta_to_jpeg(metafile_ptr->get_filename());
      }

   void plot_time_func_magnitude(
      waveform& w,metafile* metafile_ptr,string filename,double display_frac)
      {
         double max_value_mag=w.max_time_domain_function_magnitude();
         metafile_ptr->set_parameters(
            filename,"Time domain function","Time (secs)",
            "Wavefunction magnitude",
            display_frac*w.get_tlo(),display_frac*w.get_thi(),
            0,1.2*max_value_mag);

         metafile_ptr->openmetafile();
         metafile_ptr->write_header();
         write_time_domain_magnitude_data(w,metafile_ptr);
         metafile_ptr->closemetafile();
         filefunc::meta_to_jpeg(metafile_ptr->get_filename());
      }

   void plot_time_func_phase(
      waveform& w,metafile* metafile_ptr,string filename,double display_frac)
      {
         metafile_ptr->set_parameters(
            filename,"Time domain function","Time (secs)",
            "Wavefunction phase",
            display_frac*w.get_tlo(),display_frac*w.get_thi(),
            -1.2*PI,1.2*PI);

         metafile_ptr->openmetafile();
         metafile_ptr->write_header();
         write_time_domain_phase_data(w,metafile_ptr);
         metafile_ptr->closemetafile();
         filefunc::meta_to_jpeg(metafile_ptr->get_filename());
      }

   void plot_freq_func_magnitude(
      waveform& w,metafile* metafile_ptr,string filename)
      {
         double max_tilde_mag=w.max_freq_domain_function_magnitude();
         metafile_ptr->set_parameters(
            filename,"Frequency domain function","Frequency (Hz)",
            "Wavefunction magnitude",w.get_freqlo(),w.get_freqhi(),
            0,1.2*max_tilde_mag);

         metafile_ptr->openmetafile();
         metafile_ptr->write_header();
         write_freq_domain_magnitude_data(w,metafile_ptr);
         metafile_ptr->closemetafile();
         filefunc::meta_to_jpeg(metafile_ptr->get_filename());
      }

   void plot_freq_func_phase(
      waveform& w,metafile* metafile_ptr,string filename)
      {
         metafile_ptr->set_parameters(
            filename,"Frequency domain function","Frequency (Hz)",
            "Wavefunction phase",w.get_freqlo(),w.get_freqhi(),
            -1.2*PI,1.2*PI);

         metafile_ptr->openmetafile();
         metafile_ptr->write_header();
         write_freq_domain_phase_data(w,metafile_ptr);
         metafile_ptr->closemetafile();
         filefunc::meta_to_jpeg(metafile_ptr->get_filename());
      }

   void write_time_domain_real_data(waveform& w,metafile* metafile_ptr)
      {
         int nprecision=metafile_ptr->set_metastream_precision();
         metafile_ptr->get_metastream()  << "curve color red" << endl;
         for (int i=0; i<waveform::N; i++)
         {
            metafile_ptr->get_metastream()
               << stringfunc::scinumber_to_string(
                  w.get_time(i),nprecision) << " \t\t" 
               << stringfunc::scinumber_to_string(
                  w.get_value(i).get_real(),nprecision)
               << endl;
         }
      }

   void write_time_domain_imag_data(waveform& w,metafile* metafile_ptr)
      {
         int nprecision=metafile_ptr->set_metastream_precision();
         metafile_ptr->get_metastream()  << "curve color red" << endl;
         for (int i=0; i<waveform::N; i++)
         {
            metafile_ptr->get_metastream()
               << stringfunc::scinumber_to_string(
                  w.get_time(i),nprecision) << " \t\t" 
               << stringfunc::scinumber_to_string(
                  w.get_value(i).get_imag(),nprecision)
               << endl;
         }
      }

   void write_time_domain_magnitude_data(waveform& w,metafile* metafile_ptr)
      {
         int nprecision=metafile_ptr->set_metastream_precision();
         metafile_ptr->get_metastream()  << "curve color red" << endl;
         for (int i=0; i<waveform::N; i++)
         {
            metafile_ptr->get_metastream()
               << stringfunc::scinumber_to_string(
                  w.get_time(i),nprecision) << " \t\t" 
               << stringfunc::scinumber_to_string(
                  w.get_value(i).get_mod(),nprecision)
               << endl;
         }
      }

   void write_time_domain_phase_data(waveform& w,metafile* metafile_ptr)
      {
         int nprecision=metafile_ptr->set_metastream_precision();
         metafile_ptr->get_metastream()  << "curve color red" << endl;
         for (int i=0; i<waveform::N; i++)
         {
            double curr_phase=basic_math::phase_to_canonical_interval(
               w.get_value(i).get_arg(),-PI,PI);
            metafile_ptr->get_metastream()
               << stringfunc::scinumber_to_string(
                  w.get_time(i),nprecision) << " \t\t" 
               << stringfunc::scinumber_to_string(curr_phase,nprecision)
               << endl;
         }
      }

   void write_freq_domain_magnitude_data(waveform& w,metafile* metafile_ptr)
      {
         int nprecision=metafile_ptr->set_metastream_precision();
         metafile_ptr->get_metastream()  << "curve color red" << endl;
         for (int i=0; i<waveform::N; i++)
         {
            metafile_ptr->get_metastream()
               << stringfunc::scinumber_to_string(
                  w.get_freq(i),nprecision) << " \t\t" 
               << stringfunc::scinumber_to_string(
                  w.get_tilde(i).get_mod(),nprecision)
               << endl;
         }
      }

   void write_freq_domain_phase_data(waveform& w,metafile* metafile_ptr)
      {
         int nprecision=metafile_ptr->set_metastream_precision();
         metafile_ptr->get_metastream()  << "curve color red" << endl;
         for (int i=0; i<waveform::N; i++)
         { 
            double curr_phase=basic_math::phase_to_canonical_interval(
               w.get_tilde(i).get_arg(),-PI,PI);
            metafile_ptr->get_metastream()
               << stringfunc::scinumber_to_string(
                  w.get_freq(i),nprecision) << " \t\t" << curr_phase << endl;
         }
      }

/*   
// ==========================================================================
// Linked list plotting methods
// ==========================================================================

// Method writelist_data writes 1D linked list function values to meta
// file output:

   void writelist_data(
      linkedlist& list,int depend_var_to_display,ofstream& metastream)
      {
         int nprecision=list.get_metafile_ptr()->set_metastream_precision();
         list.get_metafile_ptr()->add_extralines();
         
// Traverse list in forward direction, and write each node's function
// value to metafile output:

         mynode *currnode_ptr=list.get_start_ptr();
         if (!list.get_metafile_ptr()->get_plot_only_points())
         {
            metastream << "curve color ";
            metastream << colorfunc::get_colorstr(
               currnode_ptr->get_data().get_color()) << endl;
            if (list.get_metafile_ptr()->get_thickness() != 1) 
               metastream << " thick " << list.get_metafile_ptr()->
                  get_thickness() << endl;
            if (list.get_metafile_ptr()->get_style() != 0) 
               metastream << " style " << list.get_metafile_ptr()->get_style()
                          << endl;
            if (list.get_metafile_ptr()->get_legend_flag())
            {
               metastream << "label " << list.get_metafile_ptr()->
                  get_legendlabel() << endl;
            }
         }
   
         while (currnode_ptr != NULL)
         {
            if (list.get_metafile_ptr()->get_plot_only_points())
            {
               if (list.get_metafile_ptr()->get_number_points())
               {
                  if ((currnode_ptr->get_ID()+1)%
                      list.get_metafile_ptr()->get_point_number_interval()==0)
                  {
                     metastream << "textcolor black" << endl;
                     metastream << "textsize 1" << endl;
                     metastream << "text "+stringfunc::scinumber_to_string(
                        currnode_ptr->get_data().get_var(0),nprecision)+" "
                        +stringfunc::scinumber_to_string(
                           currnode_ptr->get_data().get_func(
                              depend_var_to_display),nprecision)
                        +" "+stringfunc::number_to_string(
                           currnode_ptr->get_ID()+1) << endl;
                  }
               }
               metastream << "curve marks -1 markersize "+
                  stringfunc::number_to_string(
                     currnode_ptr->get_data().get_point_size());
               metastream << " color "+colorfunc::get_colorstr(
                  currnode_ptr->get_data().get_color())+"\t";
            } // plot_only_points conditional
            if (list.get_metafile_ptr()->get_swap_axes())
            {
               metastream 
                  << stringfunc::scinumber_to_string(
                     currnode_ptr->get_data().get_func(depend_var_to_display),
                     nprecision) << "\t\t"
                  << stringfunc::scinumber_to_string(
                     currnode_ptr->get_data().get_var(0),nprecision) 
                  << endl;
            }
            else
            {
               metastream 
                  << stringfunc::scinumber_to_string(
                     currnode_ptr->get_data().get_var(0),nprecision) 
                  << "\t\t"
                  << stringfunc::scinumber_to_string(
                     currnode_ptr->get_data().get_func(depend_var_to_display),
                     nprecision) << endl;
            }
            currnode_ptr=currnode_ptr->get_nextptr();
         }
      }

// ---------------------------------------------------------------------
// Method writelist_data_and_1D_errors writes linked list function and
// 1D error values to meta file output.  Data values are assumed to
// reside within the 0th dependent function value of each node in the
// list, while corresponding errors are assumed to reside in the 1st
// dependent function value.  If member flag plot_only_points==false,
// the central values are connected together as a solid curve in the
// metafile output, while the error bars are displayed as connected
// dotted curves.  Otherwise, individual points with individual error
// bars are written to metafile output.

   void writelist_data_and_1D_errors(linkedlist& list,ofstream& metastream)
      {
         if (list.get_metafile_ptr()->get_plot_only_points())
         {
            writelist_data_and_1D_errors_as_points(list,metastream);
         }
         else
         {
            writelist_data(list,0,metastream);	
				// solid curve for central values

            linkedlist value_plus_error_list(true);
            linkedlist value_minus_error_list(true);

            mynode *currnode_ptr=list.get_start_ptr();
            while (currnode_ptr != NULL)
            {
               value_plus_error_list.append_node(datapoint(
                  currnode_ptr->get_data().get_var(0),currnode_ptr->
                  get_data().get_func(0)
                  +currnode_ptr->get_data().get_func(1)));
               value_minus_error_list.append_node(datapoint(
                  currnode_ptr->get_data().get_var(0),currnode_ptr->
                  get_data().get_func(0)
                  -currnode_ptr->get_data().get_func(1)));
               value_plus_error_list.get_stop_ptr()->get_data().set_color(
                  colorfunc::red);
               value_minus_error_list.get_stop_ptr()->get_data().set_color(
                  colorfunc::red);
               currnode_ptr=currnode_ptr->get_nextptr();
            }
      
            value_plus_error_list.get_metafile_ptr()->
               set_plot_only_points(false);
            value_plus_error_list.get_metafile_ptr()->set_style(2);
            // dotted curve for errors
            writelist_data(value_plus_error_list,0,metastream);
            value_minus_error_list.get_metafile_ptr()->
               set_plot_only_points(false);
            value_minus_error_list.get_metafile_ptr()->set_style(2);
            // dotted curve for errors
            writelist_data(value_minus_error_list,0,metastream);
         } // plot_only_points conditional
      }

// ---------------------------------------------------------------------
   void writelist_data_and_1D_errors_as_points(
      linkedlist& list,ofstream& metastream)
      {
         int nprecision=list.get_metafile_ptr()->set_metastream_precision();
         list.get_metafile_ptr()->add_extralines();

// Traverse list in forward direction, and write each node's function
// value to metafile output:

         mynode *currnode_ptr=list.get_start_ptr();
         while (currnode_ptr != NULL)
         {
            if (list.get_metafile_ptr()->get_number_points())
            {
               if ((currnode_ptr->get_ID()+1)%
                   list.get_metafile_ptr()->get_point_number_interval()==0)
               {
                  metastream << "textcolor black" << endl;
                  metastream << "textsize 1" << endl;
                  metastream << "text "+stringfunc::scinumber_to_string(
                     currnode_ptr->get_data().get_var(0),nprecision)+" "
                     +stringfunc::scinumber_to_string(
                        currnode_ptr->get_data().get_func(0),nprecision)
                     +" "+stringfunc::number_to_string(
                        currnode_ptr->get_ID()+1) << endl;
               }
            }

            metastream << "error_bars" << endl;
            if (list.get_metafile_ptr()->get_swap_axes()) 
               metastream << "horizontal" << endl;
            metastream << "mstyle 0" << endl;	// solid circle marker
            metastream << "msize "+stringfunc::number_to_string(
               currnode_ptr->get_data().get_point_size()) << endl;
            metastream << "linecolor "+colorfunc::get_colorstr(
               currnode_ptr->get_data().get_color()) << endl;
            metastream << "markcolor "+colorfunc::get_colorstr(
               currnode_ptr->get_data().get_color()) << endl;
            metastream << "data" << endl;

// Recall that error bar entries must be non-negative!

            currnode_ptr->get_data().set_func(1,max(0.0,
               currnode_ptr->get_data().get_func(1)));

            if (list.get_metafile_ptr()->get_swap_axes())
            {
               metastream 
                  << stringfunc::number_to_string(
                     currnode_ptr->get_data().get_func(0),nprecision) 
                  << "\t\t"
                  << stringfunc::number_to_string(
                     currnode_ptr->get_data().get_var(0),nprecision) 
                  << "\t\t"
                  << stringfunc::number_to_string(
                     currnode_ptr->get_data().get_func(1),nprecision) 
                  << "\t\t"
                  << stringfunc::number_to_string(
                     currnode_ptr->get_data().get_func(1),nprecision)
                  << endl;
            }
            else
            {
               metastream 
                  << stringfunc::number_to_string(
                     currnode_ptr->get_data().get_var(0),nprecision) 
                  << "\t\t"
                  << stringfunc::number_to_string(
                     currnode_ptr->get_data().get_func(0),nprecision) 
                  << "\t\t"
                  << stringfunc::number_to_string(
                     currnode_ptr->get_data().get_func(1),nprecision) 
                  << "\t\t"
                  << stringfunc::number_to_string(
                     currnode_ptr->get_data().get_func(1),nprecision)
                  << endl;
            }
            metastream << "end" << endl << endl;
            currnode_ptr=currnode_ptr->get_nextptr();
         } // while currnode_ptr != NULL condtional
      }

// ---------------------------------------------------------------------
// Method writelist_data_and_2D_errors writes linked list function and
// 1D error values to meta file output.  Data values are assumed to
// reside within the 0th dependent function value of each node in the
// list, while corresponding dependent function and independent
// variable errors are assumed to reside in the 1st and 2nd dependent
// function values.

   void writelist_data_and_2D_errors(linkedlist& list,ofstream& metastream)
      {
         int nprecision=list.get_metafile_ptr()->set_metastream_precision();
         list.get_metafile_ptr()->add_extralines();

// Traverse list in forward direction, and write each node's function
// value to metafile output:

         int i=0;
         mynode *currnode_ptr=list.get_start_ptr();
         while (currnode_ptr != NULL)
         {
            if (list.get_metafile_ptr()->get_number_points())
            {
               if ((currnode_ptr->get_ID()+1)%list.get_metafile_ptr()
                   ->get_point_number_interval()==0)
               {
                  metastream << "textcolor black" << endl;
                  metastream << "textsize 1" << endl;
                  metastream << "text "+stringfunc::scinumber_to_string(
                     currnode_ptr->get_data().get_var(0),nprecision)+" "
                     +stringfunc::scinumber_to_string(
                        currnode_ptr->get_data().get_func(0),nprecision)
                     +" "+stringfunc::number_to_string(
                        currnode_ptr->get_ID()+1) << endl;
               }
            }

            metastream << "error_bars both" << endl;
//      metastream << "mstyle 0" << endl;	// solid circle marker
//      metastream << "msize "+stringfunc::number_to_string(currnode_ptr->get_data().
//		get_point_size()) << endl;
            metastream << "linecolor "+colorfunc::get_colorstr(
               currnode_ptr->get_data().get_color()) << endl;
//      metastream << "markcolor "+colorfunc::get_colorstr(
//	currnode_ptr->get_data().get_color()) << endl;
            metastream << "data" << endl;

// Recall that error bar entries must be non-negative!

            currnode_ptr->get_data().set_func(1,max(0.0,
               currnode_ptr->get_data().get_func(1)));
            currnode_ptr->get_data().set_func(2,max(0.0,
               currnode_ptr->get_data().get_func(2)));

            metastream 
               << stringfunc::number_to_string(
                  currnode_ptr->get_data().get_var(0),nprecision) << "\t\t"
               << stringfunc::number_to_string(
                  currnode_ptr->get_data().get_func(0),nprecision) << "\t\t"
               << stringfunc::number_to_string(
                  currnode_ptr->get_data().get_func(1),nprecision) << "\t\t"
               << stringfunc::number_to_string(
                  currnode_ptr->get_data().get_func(1),nprecision) << "\t\t"
               << stringfunc::number_to_string(
                  currnode_ptr->get_data().get_func(2),nprecision) << "\t\t"
               << stringfunc::number_to_string(
                  currnode_ptr->get_data().get_func(2),nprecision) << endl;
            metastream << "end" << endl << endl;
            currnode_ptr=currnode_ptr->get_nextptr();
            i++;
         }
      }

// ---------------------------------------------------------------------
// Method writemetafile generates metafile output which depends upon
// the linked list's nodes' number N of dependent variables.  (As of
// Sept 02, we assume that the number of dependent variables remains
// fixed at either 1 or 2 for all calls to this method.)  If N==2 and
// boolean flag display_vert_errorbars==true, this method assumes that
// the second dependent variable corresponds to an error bar sigma for
// the first dependent variable mu.  It then generates a plot of mu
// +/- sigma versus the independent variable var[0].

   void writemetafile(linkedlist& list)
      {
         writemetafile(list,false,0);
      }

   void writemetafile(linkedlist& list,int depend_var_to_display)
      {
         writemetafile(list,false,depend_var_to_display);
      }

   void writemetafile(
      linkedlist& list,bool display_vert_errorbars,int depend_var_to_display)
      {
         writemetafile(
            list,display_vert_errorbars,false,depend_var_to_display);
      }

   void writemetafile(
      linkedlist& list,bool display_vert_errorbars,
      bool display_horiz_errorbars,int depend_var_to_display)
      {
         list.get_metafile_ptr()->openmetafile();
         list.get_metafile_ptr()->write_header();
         if (display_vert_errorbars && !display_horiz_errorbars)
         {
            writelist_data_and_1D_errors(
               list,list.get_metafile_ptr()->get_metastream());
         }
         else if (display_vert_errorbars && display_horiz_errorbars)
         {
            writelist_data_and_2D_errors(
               list,list.get_metafile_ptr()->get_metastream());
         }
         else 
         {
            writelist_data(
               list,depend_var_to_display,list.get_metafile_ptr()->
               get_metastream());
         }
         list.get_metafile_ptr()->closemetafile();
      }

// ---------------------------------------------------------------------
   void writelist(linkedlist& list)
      {
         writelist(list,0);
      }

   void writelist(linkedlist& list,int depend_var_to_display)
      {
         writelist(list,false,depend_var_to_display);
      }

   void writelist(
      linkedlist& list,bool display_vert_errorbars,int depend_var_to_display)
      {
         writelist(list,display_vert_errorbars,false,depend_var_to_display);
      }

   void writelist(
      linkedlist& list,bool display_vert_errorbars,
      bool display_horiz_errorbars,int depend_var_to_display)
      {
         writemetafile(list,display_vert_errorbars,display_horiz_errorbars,
                       depend_var_to_display);
         filefunc::meta_to_jpeg(list.get_metafile_ptr()->get_filename());
      }

// ---------------------------------------------------------------------
// Append current list to RHS part of doublet meta file:

   void writelist_member(linkedlist& list)
      {
         writelist_member(list,0);
      }

   void writelist_member(linkedlist& list,int depend_var_to_display)
      {
         list.get_metafile_ptr()->appendmetafile();
         list.get_metafile_ptr()->write_header();
         writelist_data(
            list,depend_var_to_display,list.get_metafile_ptr()->
            get_metastream());
         list.get_metafile_ptr()->closemetafile();
      }

// ---------------------------------------------------------------------
   void append_metafile(linkedlist& list)
      {
         append_metafile(list,false,0);
      }

   void append_metafile(linkedlist& list,int depend_var_to_display)
      {
         append_metafile(list,false,depend_var_to_display);
      }

   void append_metafile(
      linkedlist& list,bool display_vert_errorbars,int depend_var_to_display)
      {
         append_metafile(
            list,display_vert_errorbars,false,depend_var_to_display);
      }

   void append_metafile(
      linkedlist& list,bool display_vert_errorbars,
      bool display_horiz_errorbars,int depend_var_to_display)
      {
         list.get_metafile_ptr()->appendmetafile();

         if (display_vert_errorbars && !display_horiz_errorbars)
         {
            writelist_data_and_1D_errors(
               list,list.get_metafile_ptr()->get_metastream());
         }
         else if (display_vert_errorbars && display_horiz_errorbars)
         {
            writelist_data_and_2D_errors(
               list,list.get_metafile_ptr()->get_metastream());
         }
         else 
         {
            writelist_data(
               list,depend_var_to_display,
               list.get_metafile_ptr()->get_metastream());
         }
         list.get_metafile_ptr()->closemetafile();
      }

// ---------------------------------------------------------------------
   void append_plot(linkedlist& list)
      {
         append_plot(list,0);
      }

   void append_plot(linkedlist& list,int depend_var_to_display)
      {
         append_plot(list,false,depend_var_to_display);
      }

   void append_plot(
      linkedlist& list,bool display_vert_errorbars,int depend_var_to_display)
      {
         append_plot(list,display_vert_errorbars,false,depend_var_to_display);
      }

   void append_plot(
      linkedlist& list,bool display_vert_errorbars,
      bool display_horiz_errorbars,int depend_var_to_display)
      {
         append_metafile(list,display_vert_errorbars,display_horiz_errorbars,
                         depend_var_to_display);
         filefunc::meta_to_jpeg(list.get_metafile_ptr()->get_filename());
      }
*/

} // plotfunc namespace
