// ==========================================================================
// PathFinder class member function definitions
// ==========================================================================
// Last updated on 8/2/11; 8/3/11; 8/4/11; 1/16/13
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "graphs/graph.h"
#include "graphs/graphfuncs.h"
#include "graphs/node.h"
#include "numrec/nrfuncs.h"
#include "numerical/param_range.h"
#include "osg/osgAnnotators/PathFinder.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "image/raster_parser.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "image/terrainfuncs.h"
#include "image/TwoDarray.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PathFinder::initialize_member_objects()
{
//   pixel_skip=5;	// meters
   pixel_skip=10;	// meters
//   pixel_skip=15;	// meters
//   pixel_skip=20;	// meters
   
   Dijkstra_DTED_graph_ptr=NULL;
   PolyLinesGroup_ptr=NULL;
   SignPostsGroup_ptr=NULL;
   ztwoDarray_ptr=NULL;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<PathFinder>(
         this, &PathFinder::update_display));
}		       

void PathFinder::allocate_member_objects()
{
   RasterParser_ptr=new raster_parser;
}		       

PathFinder::PathFinder(
   Pass* PI_ptr,
   SignPostsGroup* SPG_ptr,PolyLinesGroup* PLG_ptr,
   AnimationController* AC_ptr,threevector* GO_ptr):
   GraphicalsGroup(3,PI_ptr,AC_ptr,GO_ptr)
{
//   cout << "inside PathFinder constructor" << endl;
   initialize_member_objects();
   allocate_member_objects();

   SignPostsGroup_ptr=SPG_ptr;
   PolyLinesGroup_ptr=PLG_ptr;
}

PathFinder::~PathFinder()
{
   delete ztwoDarray_ptr;
}

// ==========================================================================
// DTED Graph and path computation methods:
// ==========================================================================

// Member function import_DTED_height_map() reads in a DTED height map
// from an input geotif file.  It instantiates *ztwoDarray_ptr and
// fills its contents with terrain height values.

void PathFinder::import_DTED_height_map(string geotif_filename)
{
   cout << "inside PathFinder::import_DTED_height_map()" << endl;
   
   RasterParser_ptr->open_image_file(geotif_filename);
   int channel_ID=0;
   RasterParser_ptr->fetch_raster_band(channel_ID);

   ztwoDarray_ptr=new twoDarray(RasterParser_ptr->get_ztwoDarray_ptr());
   cout << "*ztwoDarray_ptr = " << *ztwoDarray_ptr << endl;
   RasterParser_ptr->read_raster_data(ztwoDarray_ptr);
   RasterParser_ptr->close_image_file();
}

// ---------------------------------------------------------------------
// Member function compute_Dijkstra_DTED_graph() calculate the nodes
// and edges in *Dijkstra_DTED_graph_ptr from the height map stored in
// member *ztwoDarray_ptr:

void PathFinder::compute_Dijkstra_DTED_graph(double alpha)
{
//   cout << "inside PathFinder::compute_Dijkstra_DTED_graph()" << endl;
   
   delete Dijkstra_DTED_graph_ptr;
   Dijkstra_DTED_graph_ptr=terrainfunc::generate_Dijkstra_DTED_graph(
      ztwoDarray_ptr,alpha,pixel_skip);
//   cout << "*Dijkstra_DTED_graph_ptr = " << *Dijkstra_DTED_graph_ptr << endl;
}

// ---------------------------------------------------------------------
void PathFinder::compute_starting_px_py()
{
//   cout << "inside PathFinder::compute_starting_px_py()" << endl;

   if (SignPostsGroup_ptr->get_n_Graphicals() < 1)
   {
      cout << "Need at least one SignPost starting px,py can be calculated" 
           << endl;
      return;
   }

   SignPost* start_SignPost_ptr=SignPostsGroup_ptr->get_SignPost_ptr(0);
   double curr_t=SignPostsGroup_ptr->get_curr_t();
   int pass_number=SignPostsGroup_ptr->get_passnumber();

   threevector start_posn;
   start_SignPost_ptr->get_UVW_coords(curr_t,pass_number,start_posn);
   cout << "start_posn = " << start_posn << endl;

   ztwoDarray_ptr->point_to_pixel(start_posn,px_start,py_start);
//   cout << "px_start = " << px_start << " py_start = " << py_start << endl;

// Reset px_start and py_start so that they are even multiples of
// pixel_skip:

   px_start=(px_start/pixel_skip)*pixel_skip;
   py_start=(py_start/pixel_skip)*pixel_skip;

//   cout << "After renormalizing, px_start = " << px_start 
//        << " py_start = " << py_start << endl;
}

// ---------------------------------------------------------------------
void PathFinder::compute_stopping_px_py()
{
//   cout << "inside PathFinder::compute_stopping_px_py()" << endl;

   if (SignPostsGroup_ptr->get_n_Graphicals() < 2)
   {
      cout << "Need 2 SignPosts before stopping px,py can be calculated" 
           << endl;
      return;
   }

   SignPost* stop_SignPost_ptr=SignPostsGroup_ptr->get_SignPost_ptr(1);

   double curr_t=SignPostsGroup_ptr->get_curr_t();
   int pass_number=SignPostsGroup_ptr->get_passnumber();
   threevector stop_posn;
   stop_SignPost_ptr->get_UVW_coords(curr_t,pass_number,stop_posn);
   cout << "stop_posn = " << stop_posn << endl;

   ztwoDarray_ptr->point_to_pixel(stop_posn,px_stop,py_stop);
//   cout << "px_stop = " << px_stop << " py_stop = " << py_stop << endl;

   px_stop=(px_stop/pixel_skip)*pixel_skip;
   py_stop=(py_stop/pixel_skip)*pixel_skip;
//   cout << "After renormalizing, px_stop = " << px_stop 
//        << " py_stop = " << py_stop << endl;
}

// ---------------------------------------------------------------------
void PathFinder::compute_Dijkstra_field()
{
   cout << "inside PathFinder::compute_Dijkstra_field()" << endl;

   if (Dijkstra_DTED_graph_ptr==NULL)
   {
      cout << "DTED graph not yet calculated!" << endl;
      return;
   }

   compute_starting_px_py();
   
   int mdim=ztwoDarray_ptr->get_mdim();
   int ID_start=terrainfunc::px_py_to_node_ID(mdim,px_start,py_start);
//   cout << "ID_start = " << ID_start << endl;
   
   node* start_node_ptr=Dijkstra_DTED_graph_ptr->get_node_ptr(ID_start);
   start_node_ptr->set_distance_from_start(0);
//   cout << "start_node_ptr = " << start_node_ptr << endl;

   Dijkstra_DTED_graph_ptr->compute_Dijkstra_edge_weights(start_node_ptr);

   cout << "Dijkstra field calculated" << endl;
}

// ---------------------------------------------------------------------
void PathFinder::compute_Dijkstra_path()
{
   cout << "inside PathFinder::compute_Dijkstra_path()" << endl;

   if (Dijkstra_DTED_graph_ptr==NULL)
   {
      cout << "DTED graph not yet calculated!" << endl;
      return;
   }

   compute_stopping_px_py();

   int mdim=ztwoDarray_ptr->get_mdim();
   int ID_stop=terrainfunc::px_py_to_node_ID(mdim,px_stop,py_stop);
//   cout << "ID_stop = " << ID_stop << endl;
   node* stop_node_ptr=Dijkstra_DTED_graph_ptr->get_node_ptr(ID_stop);
//   cout << "stop_node_ptr = " << stop_node_ptr << endl;

   waypoints.clear();
   waypoints=terrainfunc::compute_shortest_path(
         stop_node_ptr,ztwoDarray_ptr);

   draw_path();
}

// ---------------------------------------------------------------------
void PathFinder::compute_Astar_path()
{
   cout << "inside PathFinder::compute_Astar_path()" << endl << endl;

   cout << "----------------------------------------------------------------"
        << endl;
   cout << "As of 2011, our dimensionless path cost function contains 3 terms:"
        << endl << endl;
   cout << "  term1=ds/skip" << endl;
   cout << "  term2=sqr(dz)/(ds*skip)" << endl;
   cout << "  term3=sqr((z-avg_altitude)/250 meters) * ds/skip" << endl
        << endl;

   cout << "Term1 incurs a cost proportional to the step size" << endl;
   cout << "Term2 incurs a cost quadratic in vertical displacement" << endl;
   cout << "Term3 penalizes paths which move upwards where they are more visible from the sky" << endl << endl;

   cout << "The total cost to take one step = " << endl;
   cout << "  term1+vertical_displacement_term_weight*term2+zfrac_term_weight*term3" << endl << endl;
   cout << "----------------------------------------------------------------"
        << endl << endl;

   if (SignPostsGroup_ptr->get_n_Graphicals() < 1)
   {
      cout << "Need 2 SignPosts before Astar path can be calculated" 
           << endl;
      return;
   }

   double alpha_term_weight=20;
   cout << "Enter vertical displacement term weight:" << endl;
   cin >> alpha_term_weight;

   double beta_term_weight=20;
   cout << "Enter z fraction term weight:" << endl;
   cin >> beta_term_weight;

   cout << "Enter number of ztwoDarray pixels ( = meters) to skip when searching for path:" << endl;
   cout << "Suggested default value = 100" << endl;
   cin >> pixel_skip;

   compute_Astar_path(alpha_term_weight,beta_term_weight);
}

// ---------------------------------------------------------------------
void PathFinder::compute_Astar_paths_vs_alpha_beta()
{
   cout << "inside PathFinder::compute_Astar_paths_vs_alpha_beta()" << endl;

   if (SignPostsGroup_ptr->get_n_Graphicals() < 1)
   {
      cout << "Need 2 SignPosts before Astar path can be calculated" 
           << endl;
      return;
   }

//   param_range alpha_term_weight(0,100,5);	// Prefered range as of 6/29
   param_range alpha_term_weight(7,93,7);	// Mystery alpha range

//   param_range beta_term_weight(0,5,5);	// Prefered range as of 6/29
   param_range beta_term_weight(0.3,4.7,7);	// Mystery beta range

   cout << "Enter number of ztwoDarray pixels ( = meters) to skip when searching for path:" << endl;
   cin >> pixel_skip;

   int path_counter=0;
   while (alpha_term_weight.prepare_next_value())
   {
      while (beta_term_weight.prepare_next_value())
      {
         double alpha=alpha_term_weight.get_value();
         double beta=beta_term_weight.get_value();
         cout << "path_counter = " << path_counter
              << " alpha = " << alpha << " beta = " << beta << endl;

         compute_Astar_path(alpha,beta);
         string output_filename="path_"+stringfunc::number_to_string(alpha)+
            "_"+stringfunc::number_to_string(beta)+".dat";
         PolyLinesGroup_ptr->save_info_to_file(output_filename);

         path_counter++;
      } // loop over beta_term_weight
   } // loop over alpha_term_weight
}

// ---------------------------------------------------------------------
void PathFinder::compute_Astar_path(
   double alpha_term_weight,double beta_term_weight)
{
//   cout << "inside PathFinder::compute_Astar_path()" << endl;

   if (SignPostsGroup_ptr->get_n_Graphicals() < 1)
   {
      cout << "Need 2 SignPosts before Astar path can be calculated" 
           << endl;
      return;
   }

   waypoints.clear();

   int SearchCount=0;
   const int NumSearches=1;
   while (SearchCount < NumSearches)
   {
      compute_starting_px_py();
      compute_stopping_px_py();
      
// Create start and goal states:

      MapSearchNode nodeStart(
         px_start,py_start,pixel_skip,alpha_term_weight,beta_term_weight,
         zmin,zmax,ztwoDarray_ptr);
      MapSearchNode nodeEnd(
         px_stop,py_stop,pixel_skip,alpha_term_weight,beta_term_weight,
         zmin,zmax,ztwoDarray_ptr);

      astarsearch.SetStartAndGoalStates( nodeStart, nodeEnd );

      unsigned int SearchState;
      unsigned int SearchSteps = 0;
      do
      {
         if (SearchSteps%1000==0) cout << SearchSteps/1000 << " " << flush;
         SearchState = astarsearch.SearchStep();
         SearchSteps++;
      }
      while( SearchState == 
             AStarSearch<MapSearchNode>::SEARCH_STATE_SEARCHING );
      cout << endl;

      if (SearchState==AStarSearch<MapSearchNode>::SEARCH_STATE_SUCCEEDED)
      {
         cout << "Search found goal state" << endl;
         MapSearchNode* node_ptr = astarsearch.GetSolutionStart();
 
         int steps = 0;
         node_ptr->PrintNodeInfo();
         for( ;; )
         {
            waypoints.push_back(node_ptr->get_posn());
            
            node_ptr = astarsearch.GetSolutionNext();
            if (node_ptr==NULL) break;

/*
            if( !node_ptr )
            {
               break;
            }
*/

//            node_ptr->PrintNodeInfo();
            steps++;
         };

         cout << "Solution steps " << steps << endl;

// Once you're done with the solution, you can free the node_ptrs up:

         astarsearch.FreeSolutionNodes();

         draw_path();
      }
      else if( SearchState==AStarSearch<MapSearchNode>::SEARCH_STATE_FAILED) 
      {
         cout << "Search terminated. Did not find goal state" << endl;
      }

      // Display the number of loops the search went through
      cout << "SearchSteps : " << SearchSteps << endl;
      SearchCount++;
      astarsearch.EnsureMemoryFreed();
   }
}

// ---------------------------------------------------------------------
void PathFinder::draw_path()
{
   cout << "inside PathFinder::draw_path()" << endl;

// Offset waypoints from DTED surface by waypoint_dz for PolyLine
// viewing purposes:

   const double waypoint_dz=2;	// meters

   vector<double> dthetas;
   vector<threevector> raised_waypoints;
   for (unsigned int w=0; w<waypoints.size(); w++)
   {
      threevector curr_waypoint(waypoints[w]);
      raised_waypoints.push_back(curr_waypoint+waypoint_dz*z_hat);

      if (w < waypoints.size()-1)
      {
         threevector delta=waypoints[w+1]-curr_waypoint;
         double ds=sqrt(sqr(delta.get(0))+sqr(delta.get(1)));
         double curr_dtheta=atan2(fabs(delta.get(2)),ds);
         dthetas.push_back(curr_dtheta);
      }
   }
   std::sort(dthetas.begin(),dthetas.end());

//   PolyLinesGroup_ptr->destroy_all_PolyLines();

   double R=1;
   double G=1;
   double B=1;

   if (PolyLinesGroup_ptr->get_n_Graphicals()==0)
   {
//      R=G=B=0;
   }
   osg::Vec4 uniform_color(R,G,B,1);
   PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
      raised_waypoints,uniform_color);
//   cout << "PolyLinesGroup_ptr->get_n_Graphicals() = "
//        << PolyLinesGroup_ptr->get_n_Graphicals() << endl;
//   cout << "PolyLine_ptr = " << PolyLine_ptr << endl;

   polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
   double total_length=polyline_ptr->compute_total_length();
   cout << "Total length = " << total_length << endl;
//   cout << "Median dtheta = " << mathfunc::median_value(dthetas)*180/PI
//        << endl;
//   cout << "Max dtheta = " << dthetas.back()*180/PI << endl;
}

// ---------------------------------------------------------------------
void PathFinder::purge_paths()
{
//   cout << "inside PathFinder::purge_paths()" << endl;
   PolyLinesGroup_ptr->destroy_all_PolyLines();
}

// ---------------------------------------------------------------------
void PathFinder::purge_SignPosts()
{
//   cout << "inside PathFinder::purge_SignPosts()" << endl;
   SignPostsGroup_ptr->destroy_all_SignPosts();
}

// ==========================================================================
// Animation methods:
// ==========================================================================

// Member function update_display()

void PathFinder::update_display()
{
//   cout << "inside PathFinder::update_display()" << endl;
   
//   PolyLinesGroup_ptr->update_display();
   GraphicalsGroup::update_display();
}

// ==========================================================================
// Dynamic path point fitting member functions
// ==========================================================================

// Member function fit_Astar_paths_vs_alpha_beta()

void PathFinder::fit_Astar_paths_vs_alpha_beta()
{
   cout << "inside PathFinder::fit_Astar_paths_vs_alpha_beta()" << endl;

   if (SignPostsGroup_ptr->get_n_Graphicals() < 1)
   {
      cout << "Need 2 SignPosts before Astar path can be calculated" 
           << endl;
      return;
   }

   double frac_hits_to_keep;
   string mystery_path_filename;
   vector<threevector> dynamic_path_points=read_simulated_dynamic_path_points(
      mystery_path_filename,frac_hits_to_keep);

// path_counter = 6 alpha = 7 beta = 4.7
// path_counter = 21 alpha = 50 beta = 0.3
// path_counter = 44 alpha = 93 beta = 1.766666667

   double min_alpha_value=0;
   double max_alpha_value=100;
   double min_beta_value=0;
   double max_beta_value=5;

   int n_alpha_steps,n_beta_steps;
   double alpha_lo,alpha_hi,beta_lo,beta_hi;
   cout << "Enter alpha_lo:" << endl;
   cin >> alpha_lo;
   cout << "Enter alpha_hi:" << endl;
   cin >> alpha_hi;
   cout << "Enter number alpha steps:" << endl;
   cin >> n_alpha_steps;

   cout << "Enter beta_lo:" << endl;
   cin >> beta_lo;
   cout << "Enter beta_hi:" << endl;
   cin >> beta_hi;
   cout << "Enter number beta steps:" << endl;
   cin >> n_beta_steps;

   param_range alpha_term_weight(alpha_lo,alpha_hi,n_alpha_steps);
//   param_range alpha_term_weight(0,100,5);	// Prefered range as of 6/29
//   param_range alpha_term_weight(7,93,7);	// Mystery alpha range

   param_range beta_term_weight(beta_lo,beta_hi,n_beta_steps);
//   param_range beta_term_weight(0,5,5);	// Prefered range as of 6/29
//   param_range beta_term_weight(0.3,4.7,7);	// Mystery beta range

   cout << "Enter number of ztwoDarray pixels ( = meters) to skip when searching for path:" << endl;
   cin >> pixel_skip;

   ofstream score_stream;

   int n_iters=1;
//   int n_iters=2;
   for (int iter=0; iter<n_iters; iter++)
   {

      string score_filename="scores_iter_"+stringfunc::number_to_string(iter)
         +".dat";
      filefunc::openfile(score_filename,score_stream);
      score_stream << "Mystery path filename = "
                   << mystery_path_filename << endl << endl;

      score_stream << "Iteration = " << iter << endl;
      score_stream << "alpha_lo = " << alpha_term_weight.get_start()
                   << " alpha_hi = " << alpha_term_weight.get_stop() 
                   << endl;
      score_stream << "n_alpha_steps = " << alpha_term_weight.get_nbins()
                   << endl;
      score_stream << "beta_lo = " << beta_term_weight.get_start()
                   << " beta_hi = " << beta_term_weight.get_stop() 
                   << endl;
      score_stream << "n_beta_steps = " << beta_term_weight.get_nbins() 
                   << endl;
      score_stream << endl;
      score_stream << "Fraction of simulated dynamic hits actually kept = "
                   << frac_hits_to_keep << endl << endl;

// ========================================================================
// Begin while loop over camera position parameters
// ========================================================================

      int path_counter=0;
      vector<double> scores,alphas,betas;
      while (alpha_term_weight.prepare_next_value())
      {
         while (beta_term_weight.prepare_next_value())
         {
            double alpha=alpha_term_weight.get_value();
            double beta=beta_term_weight.get_value();
            alphas.push_back(alpha);
            betas.push_back(beta);
            cout << "iter = " << iter 
                 << " path_counter = " << path_counter
                 << " alpha = " << alpha 
                 << " beta = " << beta << endl;

            score_stream << "iter = " << iter 
                         << " path_counter = " << path_counter
                         << " alpha = " << alpha 
                         << " beta = " << beta << endl;

            compute_Astar_path(alpha,beta);
            string output_filename=
               "trialpath_"+stringfunc::number_to_string(alpha)+
               "_"+stringfunc::number_to_string(beta)+".dat";
            PolyLinesGroup_ptr->save_info_to_file(output_filename);

            PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_PolyLine_ptr(
               PolyLinesGroup_ptr->get_n_Graphicals()-1);
            polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();

            double curr_score=score_path_to_dynamic_path_points(
               dynamic_path_points,polyline_ptr);
            cout << "curr score = " << curr_score << endl;
            score_stream << "curr_score = " << curr_score << endl;
            scores.push_back(curr_score);

            path_counter++;
         } // loop over beta_term_weight
      } // loop over alpha_term_weight

      templatefunc::Quicksort(scores,alphas,betas);
      score_stream << "Sorted score results:" << endl << endl;

      for (unsigned int s=0; s<scores.size(); s++)
      {
         cout << "s = " << s
              << " score = " << scores[s]
              << " alpha = " << alphas[s]
              << " beta = " << betas[s] << endl;
         score_stream << "s = " << s
                      << " score = " << scores[s]
                      << " alpha = " << alphas[s]
                      << " beta = " << betas[s] << endl;
      }

// Form weighted averages of best alpha and beta values to form next
// iteration's starting values:

      int n_best_scores_to_keep=4;
      double alpha_numer=0;
      double beta_numer=0;
      double denom=0;
      for (int s=0; s<n_best_scores_to_keep; s++)
      {
         double curr_weight=1.0/scores[s];
         alpha_numer += curr_weight*alphas[s];
         beta_numer += curr_weight*betas[s];
         denom += curr_weight;
      } // loop over index s labeling best scores
   
      double best_alpha=alpha_numer/denom;
      double best_beta=beta_numer/denom;
      cout << "best_alpha = " << best_alpha << endl;
      cout << "best_beta = " << best_beta << endl;
      score_stream << "best_alpha = " << best_alpha << endl;
      score_stream << "best_beta = " << best_beta << endl;
      alpha_term_weight.set_best_value(best_alpha);
      beta_term_weight.set_best_value(best_beta);

      double frac=0.2;
      alpha_term_weight.shrink_search_interval(
         alpha_term_weight.get_best_value(),min_alpha_value,max_alpha_value,
         frac);
      beta_term_weight.shrink_search_interval(
         beta_term_weight.get_best_value(),min_beta_value,max_beta_value,
         frac);

   } // loop over iteration index iter
}

// ---------------------------------------------------------------------
// Member function read_simulated_dynamic_path_points()

vector<threevector> PathFinder::read_simulated_dynamic_path_points(
   string& mystery_path_file,double& frac_hits_to_keep)
{
   cout << "inside PathFinder::read_simulated_dynamic_path_points()" << endl;

   double phase_fraction=0;
   cout << "Enter aircraft phase orbit fraction f (0, 0.25, 0.5 or 0.75):"
        << endl;
   cin >> phase_fraction;
   int fraction_index=phase_fraction*4;

   string fstring="f0.0";
   if (fraction_index==1)
   {
      fstring="f0.25";
   }
   else if (fraction_index==2)
   {
      fstring="f0.5";
   }
   else if (fraction_index==3)
   {
      fstring="f0.75";
   }

   string mystery_path_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/pathplanning/Astar_paths/mystery_paths/";
   mystery_path_file=mystery_path_subdir+
      "mystery_target_a50_b0.3_"+fstring+".visibilities";
//      "mystery_target_a93_b1.7_"+fstring+".visibilities";
//      "mystery_target_a7_b4.7_"+fstring+".visibilities";

   if (!filefunc::fileexist(mystery_path_file))
   {
      cout << "Error in PathFinder::read_simulated_dynamic_path_points()!"
           << endl;
      cout << "Mystery path file = " << mystery_path_file
           << " doesn't exist!" << endl;
      exit(-1);
   }

   vector<threevector> dynamic_path_points;

   frac_hits_to_keep=1;
   cout << "Enter fraction of simulated dynamic hits to actually keep:" 
        << endl;
   cin >> frac_hits_to_keep;

   filefunc::ReadInfile(mystery_path_file);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=
         stringfunc::string_to_numbers(filefunc::text_line[i]);
      int tgt_visibility=column_values[7];
      if (nrfunc::ran1() > frac_hits_to_keep) tgt_visibility=0;
      
      if (tgt_visibility > 0.5)
         dynamic_path_points.push_back(threevector(
            column_values[4],column_values[5],column_values[6]));
   } // loop over index i labeling lines in mystery path data file

   return dynamic_path_points;
}

// ---------------------------------------------------------------------
// Member function score_path_to_dynamic_path_points() takes in a set
// of dynamically measured points along a mystery path.  It also takes
// in a candidate full path within *polyline_ptr.  This method
// performs a brute force computation of the average minimal distance
// of the dynamic points to *polyline_ptr.  

double PathFinder::score_path_to_dynamic_path_points(
   const vector<threevector>& dynamic_path_points,polyline* polyline_ptr)
{
   double min_distance_integral=0;
   threevector closest_point_on_polyline;

   unsigned int n_dyn_path_points=dynamic_path_points.size();
   for (unsigned int d=0; d<n_dyn_path_points; d++)
   {
      double min_distance=polyline_ptr->min_distance_to_point(
         dynamic_path_points[d],closest_point_on_polyline);
      min_distance_integral += min_distance;
   } // loop over index d labeling dynamic path points
   double avg_dyn_distance_to_path=min_distance_integral/n_dyn_path_points;
   
   return avg_dyn_distance_to_path;
}
