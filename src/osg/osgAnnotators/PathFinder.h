// ==========================================================================
// Header file for PathFinder class
// ==========================================================================
// Last updated on 6/29/11; 8/2/11; 8/4/11
// ==========================================================================

#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "image/MapSearchNode.h"
#include "graphs/stlastar.h" 

class AnimationController;
class graph;
class PolyLinesGroup;
class raster_parser;
class SignPostsGroup;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

class PathFinder : public GraphicalsGroup
{

  public:
    
// Initialization, constructor and destructor functions:

   PathFinder(
      Pass* PI_ptr,
      SignPostsGroup* SPG_ptr,PolyLinesGroup* PLG_ptr,
      AnimationController* AC_ptr,threevector* GO_ptr);
   virtual ~PathFinder();

// Set & get methods:

   void set_ztwoDarray_ptr(twoDarray* z2Darray_ptr);
   void set_extremal_z_values(double zmin,double zmax);

// DTED graph and path computation methods:

   void import_DTED_height_map(std::string geotif_filename);
   void compute_Dijkstra_DTED_graph(double alpha);
   void compute_Dijkstra_field();
   void compute_Dijkstra_path();
   void compute_Astar_path();
   void compute_Astar_paths_vs_alpha_beta();
   void compute_Astar_path(double alpha_term_weight,double beta_term_weight);

   void draw_path();
   void purge_paths();
   void purge_SignPosts();

// Dynamic path point fitting member functions:

   void fit_Astar_paths_vs_alpha_beta();
   std::vector<threevector> read_simulated_dynamic_path_points(
      std::string& mystery_path_filename,double& frac_hits_to_keep);
   double score_path_to_dynamic_path_points(
      const std::vector<threevector>& dynamic_path_points,
      polyline* curr_polyline_ptr);

// Animation methods:

   void update_display();

  protected:

  private:

   int pixel_skip;
   unsigned int px_start,py_start,px_stop,py_stop;
   double zmin,zmax;
   graph* Dijkstra_DTED_graph_ptr;
   PolyLinesGroup* PolyLinesGroup_ptr;
   raster_parser* RasterParser_ptr;
   SignPostsGroup* SignPostsGroup_ptr;
   twoDarray* ztwoDarray_ptr;
   std::vector<threevector> waypoints;

   AStarSearch<MapSearchNode> astarsearch;

   void allocate_member_objects();
   void initialize_member_objects(); 

   void compute_starting_px_py();
   void compute_stopping_px_py();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void PathFinder::set_ztwoDarray_ptr(twoDarray* z2Darray_ptr)
{
   ztwoDarray_ptr=z2Darray_ptr;
}

inline void PathFinder::set_extremal_z_values(double zmin,double zmax)
{
   this->zmin=zmin;
   this->zmax=zmax;
}


#endif // PathFinder.h



