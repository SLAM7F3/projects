// ==========================================================================
// Program TWOD numerically integrates Schrodinger equation's in two
// spatial dimensions for various periodic and aperiodic potential
// functions.  Either position or momentum space wavefunction values
// can be calculated as functions of time.  Magnitude/phase or
// real/imaginary wavefunction values are displayed in doublet meta
// file output.  Program TWOD generates JPEG movies to facilitate
// viewing of wavefunction time evolution.

// NOTE: Be sure to make necessary alterations within
// quantum_2Dwavefunction::potential() involving potential_param
// member variables !!
// ==========================================================================
// Last updated on 5/22/05
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "datastructures/datapoint.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "plot/plotfuncs.h"
#include "quantum/quantum_2Dwavefunction.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "image/twoDarray.h"

// ==========================================================================
// Constants
// ==========================================================================

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::string;
   using std::ostream;
   using std::cout;
   using std::endl;
   std::set_new_handler(sysfunc::out_of_memory);

// Following values for deltat_min and deltat_max appropriate for
// systems (such as harmonic and Mathieu oscillators) where PE
// magnitude is of order unity.

   const double deltat_min=0.001; // Min allowed size for dynamic timestep
   const double deltat_max=0.1; // Max allowed size for dynamic timestep

// Following values for deltat_min and deltat_max are MAYBE
// appropriate for QFP where EJ/EC = 1E6.

//   const double deltat_min=1E-7;
//   const double deltat_max=1E-6;
   const double Efrac_max=1E-5; // Max allowed frac error in energy
				//  between time step i and i+1
   const double Efrac_min=1E-7; // Min allowed frac error in energy 
				//  between timestep i and i+1

//   rmlocks(".");
   sysfunc::clearscreen();

   bool input_param_file;
   int ninputlines;
   string inputline[200];
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   int currlinenumber=0;

   quantum_2Dwavefunction psi;
   double dt_plot=psi.initialize_simulation(
      input_param_file,inputline,currlinenumber);
//   psi.plot_potential("potential");

// Compute low-lying energy eigenstates and prepare initial state:

   psi.prepare_initial_state(input_param_file,inputline,currlinenumber);

// ------------------------------------------------------------------------
// Loop over time starts here:

   psi.start_processing_time=time(NULL);
   int n=0;	// index n counts number of time steps
   int nimage=0;
   int n_redos=0;

   complex value_copy[psi.nxbins_max][psi.nybins_max];

   double t_lastplot=psi.get_tmin();
   psi.set_t(psi.get_tmin());
   while (psi.get_t() < psi.get_tmax())
   {
      double E=psi.compute_energy(psi.value);

// Only write wavefunction information to metafile output at times
// corresponding to integer multiples of dt_plot:

      if ((psi.get_t()==psi.get_tmin()) || 
          (floor((psi.get_t()-t_lastplot)/dt_plot) >= 1))
      {
         outputfunc::newline();
         cout << "Processing image " << nimage+1
              << " out of " << floor((psi.get_tmax()-psi.get_tmin())/dt_plot)+1
              << ":" << endl;
         outputfunc::newline();

// Update momentum space wavefunction only if it is to be written to
// metafile output:

         if (psi.get_domain_name()==quantum::momentum_space) 
         {
            psi.fouriertransform(psi.value,psi.tilde);
         }
     
// Save energy into linked list:

         psi.energylist.append_node(datapoint(psi.get_t(),E));

// For time dependent potentials, compute energy of current
// wavefunction using final system's potential.  We included these
// lines on 11/16/01 in an attempt to deduce the optimal time at which
// the NOT gate potential should be raised in order to capture and
// retain most of the wavefunction probability density into the second
// well of the 1D SQUID potential.

         if (psi.time_dependent_potential)
         {
            double currt=psi.get_t();
            psi.set_t(psi.get_tmax());
            double Efinal=psi.compute_energy(psi.value);
            psi.set_t(currt);
            psi.Efinallist.append_node(datapoint(psi.get_t(),Efinal));
            cout << "t = " << psi.get_t() << " Energy = " << E 
                 << " Efinal = " << Efinal << endl;
         }
         else if (!psi.time_dependent_potential)
         {
            cout << "t = " << psi.get_t() << " Energy = " << E << endl;
         }
         outputfunc::newline();

         psi.imagefilenamestr=psi.imagedir+"image"
            +stringfunc::number_to_string(nimage)+".meta";
//         psi.imagefilenamestr=psi.imagedir+"image"
//            +integer_to_string(nimage,psi.ndigits_max)+".meta";
         filefunc::openfile(psi.imagefilenamestr,psi.imagestream);

         if (psi.get_complex_plot_type()==quantum::sqd_amp)
         {
            psi.singletfile_header(nimage+1);
            psi.writeimagedata();
            psi.singletfile_footer(E);
         }
         else if (psi.get_complex_plot_type()==quantum::energy_prob)
         {
            for (int j=0; j<2; j++)
            {
               psi.doubletfile_header(j);
               psi.writeimagedata(j);
            }
            psi.doubletfile_footer(nimage+1,E);
         }
         else
         {
            if (psi.get_domain_name()==quantum::position_space)
            {
               psi.potential_header("triplet");
               psi.write_potential_data();
            }
         
            for (int j=0; j<2; j++)
            {
               if (psi.get_domain_name()==quantum::position_space)
               {
                  psi.tripletfile_header(j);
               }
               else
               {
                  psi.doubletfile_header(j);
               }
               psi.writeimagedata(j);
            }

            if (psi.get_domain_name()==quantum::position_space)
            {
               psi.tripletfile_footer(nimage+1,E);
            }
            else
            {
               psi.doubletfile_footer(nimage+1,E);
            }
         } // psi.complex_plot_type==quantum::sqd_amp conditional
         
         filefunc::closefile(psi.imagefilenamestr,psi.imagestream);
         filefunc::meta_to_jpeg(psi.imagefilenamestr);
         filefunc::gzip_file(psi.imagefilenamestr);

// Dump contents of current wavefunction to output file for possible
// future restoration:

         psi.dump_wavefunction(psi.value);

         nimage++;
         t_lastplot=psi.get_t();

// Generate updated animation script and energy plot after each new
// image is written to metafile output:

//         psi.generate_animation_script("image","show_images",nimage);
         outputfunc::generate_animation_script(
            nimage,"image",psi.imagedir,"show_images");
         psi.plot_energies_vs_time();
      } // floor((psi.get_t()-psi.get_tmin())/dt_plot)==nimage conditional

// Evolve wavefunction through one dynamically controlled time step:

      psi.copy_wavefunction(psi.value,value_copy);

      bool evolve_again;
      do // while loop over current timestep 
      {
         const bool Wick_rotate=false;
         psi.FFT_step_wavefunction(Wick_rotate,psi.value);
         double Enew=psi.compute_energy(psi.value);
         double deltaE=Enew-E;

         if (fabs(deltaE)/E > Efrac_max && psi.deltat > deltat_min)
         {
            psi.deltat /= 2;
            evolve_again=true;
            psi.copy_wavefunction(value_copy,psi.value);
            n_redos++;
         }
         else if (fabs(deltaE)/E < Efrac_min && psi.deltat < deltat_max)
         {
            psi.deltat *= 2;
            evolve_again=false;
         }
         else
         {
            evolve_again=false;
         }
      }
      while (evolve_again==true);

      psi.evolve_phase();

      n++;
      psi.set_t(psi.get_t()+psi.deltat);

      cout << "n = " << n 
           << " t = " << psi.get_t() 
           << " dt = " << psi.deltat 
//           << " deltaE/E = " << deltaE/E  
           << " n_redos = " << n_redos << endl;
   } // while t < tmax

   delete [] value_copy;

   psi.summarize_results(n);
}

