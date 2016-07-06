// ==========================================================================
// Program ONED numerically integrates Schrodinger's equation in one
// spatial dimension for a particle in box, harmonic oscillator,
// lambda phi^4 and SQUID potentials with various different initial
// conditions.  Either position or momentum space wavefunction values
// can be calculated as functions of time.  Magnitude/phase or
// real/imaginary wavefunction values are displayed in doublet meta
// file output.  Program ONED generates JPEG movies to facilitate
// viewing of wavefunction time evolution.

// NOTE: Be sure to make necessary alterations within
// quantum_1Dwavefunction::potential() involving potential_param
// member variables !!
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

//   const double dt_plot=0.01;
//   const double dt_plot=0.1;	// Appropriate for harmonic oscillator 
				   // powerpoint movies
//   const double dt_plot=0.15;	// Time step between wavefunction plots
//   const double dt_plot=0.2;
//   const double dt_plot=0.4;	// Appropriate for (notional) SQUID potential 
//   const double dt_plot=1.0;	// Time step between wavefunction plots
//   const double dt_plot=4.0;	// Appropriate for (notional) SQUID potential 

   bool input_param_file;
   int ninputlines;
   string inputline[200];
   parameter_input(argc,argv,input_param_file,inputline,ninputlines);
   int currlinenumber=0;

   quantum_1Dwavefunction psi;
   double dt_plot=psi.initialize_simulation(
      input_param_file,inputline,currlinenumber);
// FAKE FAKE:
   dt_plot=0.5;
//   psi.viewgraph_mode=true;
   psi.plot_potential("potential");
   psi.set_potential_time_dependence(
      input_param_file,inputline,currlinenumber);

// Compute low-lying energy eigenstates and prepare initial state:

   psi.prepare_initial_state(input_param_file,inputline,currlinenumber);
   
// ------------------------------------------------------------------------
// Loop over time starts here:

   psi.start_processing_time=time(NULL);
   int n=0;	// index n counts number of time steps
   int n_redos=0;
   int nimage=0;
   complex value_copy[psi.nxbins_max];
   quantumarray psi_dataarray;
   twoDarray psi_twoDarray(4,psi.nxbins);
   twoDarray position_twoDarray(3,psi.nxbins);
   twoDarray momentum_twoDarray(2,psi.nxbins);

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
              << " out of " 
              << floor((psi.get_tmax()-psi.get_tmin())/dt_plot)+1
              << ":" << endl;
         outputfunc::newline();

         psi.output_wavefunction_info(
            E,nimage,t_lastplot,psi_dataarray,psi_twoDarray,
            position_twoDarray,momentum_twoDarray);
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
   } // while t < tmax

   psi.summarize_results(n);
}






