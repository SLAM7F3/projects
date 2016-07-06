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
// Last updated on 6/9/02
// ==========================================================================

#include "myinclude.h"

// ==========================================================================
// Constants
// ==========================================================================

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   const bool Wick_rotate=false;
   const bool viewgraph_mode=false;

// Following values for deltat_min, deltat_max and dt_plot appropriate
// for systems (such as harmonic and Mathieu oscillators) where PE
// magnitude is of order unity.

//   const double deltat_min=0.001; // Min allowed size for dynamic timestep
//   const double deltat_max=0.1; // Max allowed size for dynamic timestep

// Following values for deltat_min and deltat_max are MAYBE
// appropriate for QFP where EJ/EC = 1E6.

   const double deltat_min=1E-7;
   const double deltat_max=1E-6;
   const double Efrac_max=1E-5; // Max allowed frac error in energy
				//  between time step i and i+1
   const double Efrac_min=1E-7; // Min allowed frac error in energy 
				//  between timestep i and i+1

   bool input_param_file,evolve_again;
   int i,j,n;
   int nimage,n_energystates,n_redos=0;
   int ninputlines,currlinenumber;
   double currt,dt_plot,t_lastplot;
   double E,Enew,deltaE,Efinal;
   string basefilename,inputline[200];
   quantum_2Dwavefunction psi;
   complex (*value_copy)[psi.nybins_max]=
      new complex[psi.nxbins_max][psi.nybins_max];

   rmlocks(".");
   clearscreen();
   parameter_input(argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Initialize output metafile parameters:

   psi.select_output_directory(input_param_file,inputline,currlinenumber);
   psi.select_plot_output(input_param_file,inputline,currlinenumber);

// Select potential and determine characteristic time and length
// scales which depend upon the system's dimensionless coupling
// constant:

   psi.ndims=2;
   psi.init_fftw();
   psi.select_potential(input_param_file,inputline,currlinenumber);
   psi.specify_potential_timedependence(
      input_param_file,inputline,currlinenumber);
   psi.potential_period();
   psi.initialize_wavefunction_parameters(dt_plot);
//   psi.plot_potential("potential");
   if (psi.time_dependent_potential) psi.plot_potential_param();

// Compute low-lying energy eigenstates and prepare initial state:

//   n_energystates=psi.select_initial_state(
//      input_param_file,inputline,currlinenumber);
//   psi.project_low_energy_states(n_energystates);
//   psi.initialize_wavefunction();
//   nimage=0; 
//   psi.t=t_lastplot=psi.tmin;

   psi.restore_wavefunction(
      input_param_file,inputline,currlinenumber,psi.value);
   nimage=0; 
   t_lastplot=psi.t;

// ------------------------------------------------------------------------
// Loop over time starts here:

   psi.start_processing_time=time(NULL);
   n=0;	// index n counts number of time steps

   do // while loop over time
   {
      E=psi.compute_energy(psi.value);

// Only write wavefunction information to metafile output at times
// corresponding to integer multiples of dt_plot:

      if ((psi.t==t_lastplot) || (mytrunc((psi.t-t_lastplot)/dt_plot) >= 1))
      {
         outputfunc::newline();
         cout << "Processing image " << nimage+1
              << " out of " << mytrunc((psi.tmax-psi.tmin)/dt_plot)+1
              << ":" << endl;
         outputfunc::newline();

// Update momentum space wavefunction only if it is to be written to
// metafile output:

         if (psi.domainname=="momentum space") 
         {
            psi.fouriertransform(psi.value,psi.tilde);
         }
     
// Save energy into linked list:

         psi.save_energy_value(E,psi.energylist);

// For time dependent potentials, compute energy of current
// wavefunction using final system's potential.  We included these
// lines on 11/16/01 in an attempt to deduce the optimal time at which
// the NOT gate potential should be raised in order to capture and
// retain most of the wavefunction probability density into the second
// well of the 1D SQUID potential.

         if (psi.time_dependent_potential)
         {
            currt=psi.t;
            psi.t=psi.tmax;
            Efinal=psi.compute_energy(psi.value);
            psi.t=currt;
            psi.save_energy_value(Efinal,psi.Efinallist);
            cout << "t = " << psi.t << " Energy = " << E 
                 << " Efinal = " << Efinal << endl;
         }
         else if (!psi.time_dependent_potential)
         {
            cout << "t = " << psi.t << " Energy = " << E << endl;
         }
         outputfunc::newline();

         psi.imagefilenamestr=psi.imagedir+"image"
            +number_to_string(nimage)+".meta";
//         psi.imagefilenamestr=psi.imagedir+"image"
//            +integer_to_string(nimage,psi.ndigits_max)+".meta";
         openfile(psi.imagefilenamestr,psi.imagestream);

         if (psi.complex_plot_type=="sqd_amp")
         {
            psi.singletfile_header(nimage+1);
            psi.writeimagedata();
            psi.singletfile_footer(E);
         }
         else if (psi.complex_plot_type=="energy_prob")
         {
            for (j=0; j<2; j++)
            {
               psi.doubletfile_header(j);
               psi.writeimagedata(j);
            }
            psi.doubletfile_footer(nimage+1,E);
         }
         else
         {
            if (psi.domainname=="position space")
            {
               psi.potential_header("triplet");
               psi.write_potential_data();
            }
         
            for (j=0; j<2; j++)
            {
               if (psi.domainname=="position space")
               {
                  psi.tripletfile_header(j);
               }
               else
               {
                  psi.doubletfile_header(j);
               }
               psi.writeimagedata(j);
            }

            if (psi.domainname=="position space")
            {
               psi.tripletfile_footer(nimage+1,E);
            }
            else
            {
               psi.doubletfile_footer(nimage+1,E);
            }
         } // psi.complex_plot_type=="sqd_amp" conditional
         
         closefile(psi.imagefilenamestr,psi.imagestream);
         meta_to_jpeg(psi.imagefilenamestr);
         gzip_file(psi.imagefilenamestr);

// Dump contents of current wavefunction to output file for possible
// future restoration:

         psi.dump_wavefunction(psi.value);

         nimage++;
         t_lastplot=psi.t;

// Generate updated animation script and energy plot after each new
// image is written to metafile output:

         psi.generate_animation_script("image","show_images",nimage);
         psi.write_energies(psi.imagedir);

      } // mytrunc((psi.t-psi.tmin)/dt_plot)==nimage conditional

// Evolve wavefunction through one dynamically controlled time step:

      psi.copy_wavefunction(psi.value,value_copy);
      do // while loop over current timestep 
      {
         psi.FFT_step_wavefunction(Wick_rotate,psi.value);
         Enew=psi.compute_energy(psi.value);
         deltaE=Enew-E;

         if (abs(deltaE)/E > Efrac_max && psi.deltat > deltat_min)
         {
            psi.deltat /= 2;
            evolve_again=true;
            psi.copy_wavefunction(value_copy,psi.value);
            n_redos++;
         }
         else if (abs(deltaE)/E < Efrac_min && psi.deltat < deltat_max)
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
      psi.t += psi.deltat;

      cout << "n = " << n 
           << " t = " << psi.t 
           << " dt = " << psi.deltat 
//           << " deltaE/E = " << deltaE/E  
           << " n_redos = " << n_redos << endl;

   } 
   while (psi.t < psi.tmax);

   psi.summarize_results(psi.imagedir,n);
   delete [] value_copy;
}

