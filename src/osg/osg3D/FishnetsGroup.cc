// ==========================================================================
// FISHNETSGROUP class member function definitions
// ==========================================================================
// Last modified on 12/1/11; 12/5/11; 12/6/11
// ==========================================================================

#include <vector>
#include "osg/osg3D/FishnetsGroup.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void FishnetsGroup::allocate_member_objects()
{
}		       

void FishnetsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="FishnetsGroup";
   stop_ground_refinement_flag=false;
   iter_counter=fishnet_refinement_counter=0;
   min_frac_dE=0.0005;
   dz_frac_step=0.5;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<FishnetsGroup>(
         this, &FishnetsGroup::update_display));
}		       

FishnetsGroup::FishnetsGroup(
   Pass* PI_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

FishnetsGroup::~FishnetsGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const FishnetsGroup& FG)
{
   for (unsigned int n=0; n<FG.get_n_Graphicals(); n++)
   {
      cout << *FG.get_Fishnet_ptr(n) << endl;
   }
   
   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// ==========================================================================
// Fishnet creation and manipulation methods
// ==========================================================================

// Member function generate_new_Fishnet() 

Fishnet* FishnetsGroup::generate_new_Fishnet(bool fall_downwards_flag)
{
//   cout << "inside FishnetsGroup::generate_new_Fishnet()" << endl;

   Fishnet* Fishnet_ptr=new Fishnet(
      get_pass_ptr(),get_n_Graphicals(),fall_downwards_flag);
   initialize_new_Fishnet(Fishnet_ptr);
   Fishnet_ptr->initialize();

   return Fishnet_ptr;
}

// ---------------------------------------------------------------------
void FishnetsGroup::initialize_new_Fishnet(
   Fishnet* Fishnet_ptr,int OSGsubPAT_number)
{
//   cout << "inside FishnetsGroup::initialize_new_Fishnet" << endl;

   GraphicalsGroup::insert_Graphical_into_list(Fishnet_ptr);
   initialize_Graphical(Fishnet_ptr);

   osg::Geode* geode_ptr=Fishnet_ptr->generate_drawable_geode();
   Fishnet_ptr->get_PAT_ptr()->addChild(geode_ptr);
   insert_graphical_PAT_into_OSGsubPAT(Fishnet_ptr,OSGsubPAT_number);
}

// --------------------------------------------------------------------------
// Member function update_display() 

void FishnetsGroup::update_display()
{
//   cout << "inside FishnetsGroup::update_display()" << endl;

   Fishnet* Fishnet_ptr=get_Fishnet_ptr(0);
   if (Fishnet_ptr->get_Emin() > 0.5*POSITIVEINFINITY) return;
   
   const int n_refinement_iters=3;	// Peter's coincidence processing
//   const int n_refinement_iters=4;	// Pender MPSCP

   if (!stop_ground_refinement_flag) 
   {
      refine_ground_surface();
   }
   else if (iter_counter > 0 && stop_ground_refinement_flag &&
            fishnet_refinement_counter < n_refinement_iters)
   {
      refine_Fishnet();
      stop_ground_refinement_flag=false;      
   }
   
   GraphicalsGroup::update_display();
}

// ==========================================================================
// Ground surface member functions
// ==========================================================================

// Member function compute_initial_surface_energy() withs with the
// zeroth Fishnet in *this.  It computes and writes out the total
// energy of the fishnet in its current configuration.

void FishnetsGroup::compute_initial_surface_energy()
{
//   cout << "inside FishnetsGroup::compute_initial_surface_energy()" << endl;

   Fishnet* Fishnet_ptr=get_Fishnet_ptr(0);

   double E_min=Fishnet_ptr->compute_total_current_energy();
   Fishnet_ptr->set_Emin(E_min);

   stop_ground_refinement_flag=false;

   string banner="Starting E_min = "+stringfunc::number_to_string(E_min);
   outputfunc::write_big_banner(banner);
}

// --------------------------------------------------------------------------
// Member function refine_ground_surface() loops over all masses
// within the fishnet and perturbs their Z locations.  If the
// perturbation is energetically disfavorable, the mass's dZ value is
// halved and its original Z location is restored.  If the fishnet's
// total fractional energy change is less than min_frac_dE,
// stop_ground_refinement_flag is set equal to true.

void FishnetsGroup::refine_ground_surface()
{
//   cout << "inside FishnetsGroup::refine_ground_surface()" << endl;

   Fishnet* Fishnet_ptr=get_Fishnet_ptr(0);
   twoDarray* ztwoDarray_ptr=Fishnet_ptr->get_ztwoDarray_ptr();
   twoDarray* dz_twoDarray_ptr=Fishnet_ptr->get_dz_twoDarray_ptr();

   int mdim=ztwoDarray_ptr->get_mdim();
   int ndim=ztwoDarray_ptr->get_ndim();

   double prev_Emin=Fishnet_ptr->get_Emin();
//   cout << "prev_Emin = " << prev_Emin << endl;

   int n_frozen_cells=0;
   for (int px=0; px<mdim; px++)
   {
      outputfunc::update_progress_fraction(px,1,mdim);
      for (int py=0; py<ndim; py++)
      {
         double z_orig=ztwoDarray_ptr->get(px,py);
     
         double delta_E=Fishnet_ptr->perturb_ground_surface_trial(px,py,Zstop);
//         cout << "px = " << px << " py = " << py 
//              << " dE = " << delta_E << endl;
         
         if (delta_E < 0.5*NEGATIVEINFINITY)
         {
            n_frozen_cells++;
         }
         else if (delta_E > 0)
         {
            ztwoDarray_ptr->put(px,py,z_orig);
            dz_twoDarray_ptr->put(
               px,py,dz_frac_step*dz_twoDarray_ptr->get(px,py));
         }
      } // loop over fishnet index py
   } // loop over fishnet index px

   double curr_Emin=Fishnet_ptr->compute_total_current_energy();
//   cout << "curr_Emin = " << curr_Emin << endl;
   Fishnet_ptr->set_Emin(curr_Emin);

   string banner="E_min = "+stringfunc::number_to_string(
      Fishnet_ptr->get_Emin())+" after perturbation iter = "
      +stringfunc::number_to_string(iter_counter);
   outputfunc::write_big_banner(banner);

   double frozen_cell_frac=double(n_frozen_cells)/double(mdim*ndim);
   cout << "Total fishnet cells = mdim * ndim = " << mdim*ndim << endl;
   int n_changed_cells=mdim*ndim-n_frozen_cells;
   cout << "Number of changed fishnet cells = " << n_changed_cells << endl;
   cout << "Frozen cell fraction = " << frozen_cell_frac << endl;

   Fishnet_ptr->regenerate_PolyLines();

   double Delta_E=prev_Emin-Fishnet_ptr->get_Emin();
   double frac_dE=Delta_E/prev_Emin;
   cout << "Fractional dE = " << frac_dE 
        << " minimum fraction dE = " << min_frac_dE << endl;

   if (frac_dE < min_frac_dE)
   {
      stop_ground_refinement_flag=true;
   }

   iter_counter++;
//   return change_flag;
}

// --------------------------------------------------------------------------
// Member function refine_Fishnet() is called after some iteration of
// fishnet "falling" has occured.  The fishnet masses are moved in Z
// from their current locations by z_offset which diminishes in
// magnitude with growing iteration number.  The fishnet's cellsize is
// halved, and its minimum energy is initialzed to 2 times its current
// value.  

void FishnetsGroup::refine_Fishnet()
{
//   cout << "inside FishnetsGroup::refine_Fishnet()" << endl;
   string banner="*** REFINING FISHNET CELL SIZE ***";
   outputfunc::write_big_banner(banner);

   fishnet_refinement_counter++;

   cout << "fishnet_refinement_counter = " << fishnet_refinement_counter
        << endl;

   double z_offset=0.25;
   if (fishnet_refinement_counter==0)		// 40 m cell size
   {
      z_offset=1;
   }
   else if (fishnet_refinement_counter==1) 	// 20 m cell size
   {
      z_offset=1;
   }
   else if (fishnet_refinement_counter==2) 	// 10 m cell size
   {
      z_offset=0.5;
   }
   else if (fishnet_refinement_counter==3) 	// 5 m cell size
   {
      z_offset=0.25;
   }
   cout <<  "z_offset = " << z_offset << endl;

   min_frac_dE=0.004;
   if (fishnet_refinement_counter==0)		// 40 m cell size 
   {
      min_frac_dE=0.0005;
   }
   else if (fishnet_refinement_counter==1)	// 20 m cell size
   {
      min_frac_dE=0.001;
   }
   else if (fishnet_refinement_counter==2)	// 10 m cell size
   {
      min_frac_dE=0.002;
   }

   min_frac_dE=0.001;
   cout << "min_frac_dE = " << min_frac_dE << endl;

   Fishnet* Fishnet_ptr=get_Fishnet_ptr(0);
   double dx_new=Fishnet_ptr->refine_coord_system(z_offset);
   cout << "New dx = dy = " << dx_new << endl;

   Fishnet_ptr->regenerate_PolyLines();

// FAKE FAKE:  Thurs Dec 8, 2011 at 10:43 pm
// Expt with doubling Emin whenever fishnet cellsize is halved:

   Fishnet_ptr->set_Emin(Fishnet_ptr->get_Emin()*2);

}

// --------------------------------------------------------------------------
// Member function relax_ground_surface() turns off the mass and
// pressure term coefficients within the fishnet energy function.  It
// also resets all entries within *dz_twoDarray_ptr from +1 to -0.5.
// As a result, the fishnet masses no longer "fall" but rather "rise".
// We wrote this method in order to allow springs potential energy to
// be relaxes after a fishnet has "fallen" onto a ground surface in
// order to remove improbable high spatial frequency kinks in the
// fishnet.

void FishnetsGroup::relax_ground_surface()
{
   cout << "inside FishnetsGroup::relax_ground_surface()" << endl;

   Fishnet* Fishnet_ptr=get_Fishnet_ptr(0);
   Fishnet_ptr->set_weights_term_coeff(0);
   Fishnet_ptr->set_springs_term_coeff(100);
   Fishnet_ptr->set_pressure_term_coeff(0);
   Fishnet_ptr->get_dz_twoDarray_ptr()->initialize_values(-0.5);

//   dz_frac_step=0.8;
   dz_frac_step=0.95;
   compute_initial_surface_energy();
}
