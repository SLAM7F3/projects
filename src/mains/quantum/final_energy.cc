// ==========================================================================
// Program FINAL_ENERGY computes final state energies for quantum
// systems which evolve in the presence of a time-varying classical
// background.  Potential parameters are held fixed for 0 < t < t1.
// Then for t1 < t < t2, one or more of the potential parameters are
// altered.  Finally, all potential parameters are returned to their
// original values for t2 < t.  FINAL_ENERGY computes quantum system's
// energy for times greater than t2 as a function of t2-t1.
// ==========================================================================
// Last updated on 3/26/03
// ==========================================================================

#include "myinclude.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::string;
   using std::ostream;
   using std::cout;
   using std::endl;
   std::set_new_handler(out_of_memory);

   bool input_param_file;
   int ninputlines;
   string inputline[200];
   parameter_input(argc,argv,input_param_file,inputline,ninputlines);
   int currlinenumber=0;

   quantum_1Dwavefunction psi;
   double dt_plot=psi.initialize_simulation(
      input_param_file,inputline,currlinenumber);
   dt_plot=0.5;
   psi.plot_potential("potential");
   
   psi.set_potential_time_dependence(
      input_param_file,inputline,currlinenumber);

// Compute low-lying energy eigenstates and prepare initial state:

   psi.prepare_initial_state(input_param_file,inputline,currlinenumber);

// ------------------------------------------------------------------------
   psi.start_processing_time=time(NULL);

   complex value_copy[psi.nxbins_max];
   quantumarray psi_dataarray;
   twoDarray psi_twoDarray(4,psi.nxbins);
   twoDarray position_twoDarray(3,psi.nxbins);
   twoDarray momentum_twoDarray(2,psi.nxbins);
   linkedlist Efinallist(true);

   int n=0;	// index n counts number of time steps
   psi.set_potential_t1(0.5);
   double min_potential_Tinterval=0;
   double max_potential_Tinterval=15;
   for (double potential_Tinterval=min_potential_Tinterval; 
        potential_Tinterval <= max_potential_Tinterval;
        potential_Tinterval += 0.5)
   {
      psi.set_t(psi.get_tmin());
      psi.set_potential_t2(psi.get_potential_t1()+potential_Tinterval);

      outputfunc::newline();
      cout << "t1 = " << psi.get_potential_t1() 
           << " t2 = " << psi.get_potential_t2() 
           << " t2-t1 = " << potential_Tinterval << endl;

/*
      potentialfunc::plot_potential_param_time_dependence(
         psi.get_potential_type(),psi.get_tmin(),psi.get_tmax(),
         psi.get_potential_t1(),psi.get_potential_t2(),
         psi.imagedir,psi.potential_param);
*/

// We must re-initialize the wavefunction before starting a new
// simulation corresponding to a different value of potential time
// parameter t2:

      psi.initialize_wavefunction();

// Loop over time starts here:

      int n_redos=0;
      int nimage=0;
      double E,Efinal;
      double t_lastplot=psi.get_tmin();
      while (psi.get_t() < psi.get_potential_t2()+0.5)
      {
         E=psi.compute_energy(psi.value);
      
// Only write wavefunction information to metafile output at times
// corresponding to integer multiples of dt_plot:

         if ((psi.get_t()==psi.get_tmin()) || 
             (floor((psi.get_t()-t_lastplot)/dt_plot) >= 1))
         {

// Save current and "final" energies into linked lists:

            psi.energylist.append_node(psi.get_t(),E);
         
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
               Efinal=psi.compute_energy(psi.value);
               psi.set_t(currt);
               psi.Efinallist.append_node(psi.get_t(),Efinal);
               cout << "t = " << psi.get_t() << " Energy = " << E 
                    << " Efinal = " << Efinal << endl;
            }

            t_lastplot=psi.get_t();
         } // floor((psi.get_t()-t_lastplot)/dt_plot) >= 1) conditional
// Evolve wavefunction through one dynamically controlled time step:

         psi.evolve_wavefunction_thru_one_timestep(n_redos,E,value_copy);
         psi.evolve_phase();

         n++;
         psi.set_t(psi.get_t()+psi.deltat);

//      cout << "n = " << n 
//           << " t = " << psi.get_t() 
//           << " dt = " << psi.deltat 
//           << " deltaE/E = " << deltaE/E  
//           << " n_redos = " << n_redos << endl;
//      cout << "E = " << E << endl;
      } // while t < potential_t2+0.5

      Efinallist.append_node(potential_Tinterval,E);

// Plot system energy for t > t2 as a function of t2-t1:

      Efinallist.find_max_min_func_values();
      Efinallist.set_plot_only_points(false);
      Efinallist.get_metafile_ptr()->set_title("Final System Energy");
      Efinallist.get_metafile_ptr()->set_subtitle(
         "Potential parameter shifts occur at t1 and t2");
      Efinallist.get_metafile_ptr()->set_labels("t2-t1","Energy");
      Efinallist.get_metafile_ptr()->set_filename(psi.imagedir+"Efinal");
      Efinallist.get_metafile_ptr()->set_xbounds(0,max_potential_Tinterval);
      Efinallist.get_metafile_ptr()->set_ybounds(
         -0.001,1.2*Efinallist.get_fmax());
      Efinallist.get_metafile_ptr()->set_ytic(
         trunclog(1.2*Efinallist.get_fmax()));
      Efinallist.get_metafile_ptr()->set_ysubtic(
         0.5*Efinallist.get_metafile_ptr()->get_ytic());
      Efinallist.writelist();

   } // loop over potential_Tinterval
   
   psi.summarize_results(n);
}






