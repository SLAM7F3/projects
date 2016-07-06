// =========================================================================
// Header file for stand-alone plotting functions.
// =========================================================================
// Last modified on 5/22/05
// =========================================================================

#ifndef PLOTFUNCS_H
#define PLOTFUNCS_H

#include <fstream>
#include <iostream>
#include <string>
#include "datastructures/datapoint.h"
template <class T> class Linkedlist;
typedef Linkedlist<datapoint> linkedlist;

class waveform;
class metafile;

namespace plotfunc
{
   
// Waveform plotting methods:

   void plot_time_func_real(waveform& w,metafile* metafile_ptr,
                            std::string filename,double display_frac=1.0);
   void plot_time_func_imag(waveform& w,metafile* metafile_ptr,
                            std::string filename,double display_frac=1.0);
   void plot_time_func_magnitude(
      waveform& w,metafile* metafile_ptr,
      std::string filename,double display_frac=1.0);
   void plot_time_func_phase(waveform& w,metafile* metafile_ptr,
                             std::string filename,double display_frac=1.0);
   void plot_freq_func_magnitude(waveform& w,metafile* metafile_ptr,
                                 std::string filename);
   void plot_freq_func_phase(waveform& w,metafile* metafile_ptr,
                             std::string filename);
   void write_time_domain_real_data(waveform& w,metafile* metafile_ptr);
   void write_time_domain_imag_data(waveform& w,metafile* metafile_ptr);
   void write_time_domain_magnitude_data(waveform& w,metafile* metafile_ptr);
   void write_time_domain_phase_data(waveform& w,metafile* metafile_ptr);
   void write_freq_domain_magnitude_data(waveform& w,metafile* metafile_ptr);
   void write_freq_domain_phase_data(waveform& w,metafile* metafile_ptr);

/*
// Linked list plotting methods:

   void writelist_data(
      linkedlist& list,int depend_var_to_display,std::ofstream& metastream);
   void writelist_data_and_1D_errors(
      linkedlist& list,std::ofstream& metastream);
   void writelist_data_and_1D_errors_as_points(
      linkedlist& list,std::ofstream& metastream);
   void writelist_data_and_2D_errors(
      linkedlist& list,std::ofstream& metastream);

   void writemetafile(linkedlist& list);
   void writemetafile(linkedlist& list,int depend_var_to_display);
   void writemetafile(linkedlist& list,bool display_vert_errorbars,
                      int depend_var_to_display);
   void writemetafile(
      linkedlist& list,bool display_vert_errorbars,
      bool display_horiz_errorbars,int depend_var_to_display);

   void writelist(linkedlist& list);
   void writelist(linkedlist& list,int depend_var_to_display);
   void writelist(linkedlist& list,bool display_vert_errorbars,
                  int depend_var_to_display);
   void writelist(
      linkedlist& list,bool display_vert_errorbars,
      bool display_horiz_errorbars,int depend_var_to_display);

   void writelist_member(linkedlist& list);
   void writelist_member(linkedlist& list,int depend_var_to_display);

   void append_metafile(linkedlist& list);
   void append_metafile(linkedlist& list,int depend_var_to_display);
   void append_metafile(linkedlist& list,bool display_vert_errorbars,
                        int depend_var_to_display);
   void append_metafile(
      linkedlist& list,bool display_vert_errorbars,
      bool display_horiz_errorbars,int depend_var_to_display);

   void append_plot(linkedlist& list);
   void append_plot(linkedlist& list,int depend_var_to_display);
   void append_plot(linkedlist& list,bool display_vert_errorbars,
                    int depend_var_to_display);
   void append_plot(
      linkedlist& list,bool display_vert_errorbars,
      bool display_horiz_errorbars,int depend_var_to_display);

*/


}

#endif // plot/plotfuncs.h




