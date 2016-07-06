// ==========================================================================
// Program AFFINE begins to implement a crude simulation of a solar
// panel arbitarily rotated in 3D being projected down into an ISAR
// image plane.
// ==========================================================================
// Last updated on 2/14/06; 12/11/09; 1/29/12
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/Custom2DManipulator.h"
#include "color/colorfuncs.h"
#include "image/drawfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "geometry/mybox.h"
#include "image/myimage.h"
#include "numrec/nrfuncs.h"
#include "osg/osgOperations/Operations.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "geometry/plane.h"
#include "geometry/polygon.h"
#include "structmotion/reconstruction.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "threeDgraphics/threeDstring.h"
#include "math/threevector.h"
#include "image/TwoDarray.h"
#include "threeDgraphics/xyzpfuncs.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ==========================================================================
// Method generate_box takes in 3 rotation angles in degrees.  It
// instantiates a 2*unit box and then rotates it about the XYZ axes
// by the 3 input angles.  The rotated box is returned by this method.

mybox generate_box(double psi,double phi,double theta)
{
//   mybox box(-1,1,-1,1,-1,1);
//   mybox box(0,1,0,1,0,2);
   mybox box(0,1,0,2,0,3);

   psi *= PI/180;
   phi *= PI/180;
   theta *= PI/180;

   rotation R(psi,phi,theta);
   box.rotate(R);
   
   return box;
}

// ==========================================================================
void rotate_box(double alpha,double beta,double gamma,mybox& box)
{
// Rotate box about axes to some new random orientation:

   rotation R(alpha*PI/180,beta*PI/180,gamma*PI/180);
   box.rotate(R);
}

// ==========================================================================
void project_box_vertices(
   double alpha,double beta,double gamma,double scale_factor,
   mybox& box,vector<twovector>& XY)
{
// Rotate box about axes to some new random orientation:

   rotation R(alpha*PI/180,beta*PI/180,gamma*PI/180);
   box.rotate(R);
   box.XY_vertex_projections(XY,scale_factor);
}

// ==========================================================================
double box_orthogonality_score(
   const threevector& U,const threevector& V,const threevector& W)
{
   threevector Uhat=U.unitvector();
   threevector Vhat=V.unitvector();
   threevector What=W.unitvector();
   double dotproduct1=Uhat.dot(Vhat);
   double dotproduct2=Vhat.dot(What);
   double dotproduct3=What.dot(Uhat);
   double score=sqr(dotproduct1)+sqr(dotproduct2)+sqr(dotproduct3);
   return score;
}

// ==========================================================================
void enforce_planar_constraints(genmatrix* P0_ptr)
{
   vector< vector<int> > planes;
   vector<int> curr_plane;

   curr_plane.push_back(0);
   curr_plane.push_back(1);
   curr_plane.push_back(2);
   curr_plane.push_back(3);
   planes.push_back(curr_plane);

   curr_plane.clear();
   curr_plane.push_back(4);
   curr_plane.push_back(5);
   curr_plane.push_back(6);
   curr_plane.push_back(7);
   planes.push_back(curr_plane);

   curr_plane.clear();
   curr_plane.push_back(0);
   curr_plane.push_back(3);
   curr_plane.push_back(5);
   curr_plane.push_back(4);
   planes.push_back(curr_plane);

   curr_plane.clear();
   curr_plane.push_back(1);
   curr_plane.push_back(7);
   curr_plane.push_back(6);
   curr_plane.push_back(2);
   planes.push_back(curr_plane);

   curr_plane.clear();
   curr_plane.push_back(0);
   curr_plane.push_back(4);
   curr_plane.push_back(7);
   curr_plane.push_back(1);
   planes.push_back(curr_plane);

   curr_plane.clear();
   curr_plane.push_back(3);
   curr_plane.push_back(2);
   curr_plane.push_back(6);
   curr_plane.push_back(5);
   planes.push_back(curr_plane);

   for (int p=0; p<planes.size(); p++)
   {
      vector<threevector> V;
      threevector corner;
      for (int j=0; j<4; j++)
      {
         int i=(planes[p])[j];
         P0_ptr->get_column(i,corner);
         V.push_back(corner);
//         cout << "plane p = " << p << " corner j = " << j
//              << " vertex = " << V.back() << endl;
      } // loop over index j labeling corners in current planar surface
      plane face(V);

      for (int j=0; j<4; j++)
      {
         threevector new_V=face.projection_into_plane(V[j]);
         int i=(planes[p])[j];
         P0_ptr->put(0,i,new_V.get(0));
         P0_ptr->put(1,i,new_V.get(1));
         P0_ptr->put(2,i,new_V.get(2));
      } // loop over index j labeling corners in current planar surface
   } // loop over index p labeling planar surfaces
}

// ==========================================================================
void enforce_parallelepiped_constraint(genmatrix* P0_ptr)
{
   threevector top_corner,bottom_corner;
   vector<threevector> T,B;
   P0_ptr->get_column(0,top_corner);
   T.push_back(top_corner);
   P0_ptr->get_column(4,top_corner);
   T.push_back(top_corner);
   P0_ptr->get_column(5,top_corner);
   T.push_back(top_corner);
   P0_ptr->get_column(3,top_corner);
   T.push_back(top_corner);

   P0_ptr->get_column(1,bottom_corner);
   B.push_back(bottom_corner);
   P0_ptr->get_column(2,bottom_corner);
   B.push_back(bottom_corner);
   P0_ptr->get_column(6,bottom_corner);
   B.push_back(bottom_corner);
   P0_ptr->get_column(7,bottom_corner);
   B.push_back(bottom_corner);

   parallelepiped p(T,B);

   polygon top_face(p.get_topface());
   polygon bottom_face(p.get_bottomface());
   for (int j=0; j<4; j++)
   {
      threevector t(top_face.get_vertex(j));
      threevector b(bottom_face.get_vertex(j));
      P0_ptr->put(0,j,t.get(0));
      P0_ptr->put(1,j,t.get(1));
      P0_ptr->put(2,j,t.get(2));
      P0_ptr->put(0,j+4,b.get(0));
      P0_ptr->put(1,j+4,b.get(1));
      P0_ptr->put(2,j+4,b.get(2));
   } // loop over index j labeling corners in current planar surface
}

// ==========================================================================
// Method generate_edge_label_array dynamically instantiates a
// symmetric integer-valued matrix which labels every edge within a
// box.  Any two box vertices which are not connected by some edge
// receive a -1 entry within the matrix.  Otherwise, a pair of box
// vertices is labeled within the matrix by a unique edge integer ID.

int generate_edge_label_array(int n_tiepoints,Genarray<int>* edge_label_ptr)
{
   edge_label_ptr->initialize_values(-1);
   edge_label_ptr->put(0,1,1);
   edge_label_ptr->put(1,2,1);
   edge_label_ptr->put(2,3,1);
   edge_label_ptr->put(3,0,1);

   edge_label_ptr->put(4,5,1);
   edge_label_ptr->put(5,6,1);
   edge_label_ptr->put(6,7,1);   
   edge_label_ptr->put(7,4,1);

   edge_label_ptr->put(0,4,1);
   edge_label_ptr->put(1,7,1);
   edge_label_ptr->put(2,6,1);
   edge_label_ptr->put(3,5,1);
   for (int m=0; m<n_tiepoints; m++)
   {
      for (int n=0; n<n_tiepoints; n++)
      {
         if (edge_label_ptr->get(m,n)==1) edge_label_ptr->put(n,m,1);
      }
   }

// Assign unique integer ID to every parallelepiped edge:

   int edge_ID=0;
   for (int m=0; m<n_tiepoints; m++)
   {
      for (int n=m; n<n_tiepoints; n++)
      {
         if (edge_label_ptr->get(m,n)==1)
         {
            edge_label_ptr->put(m,n,edge_ID);
            edge_label_ptr->put(n,m,edge_ID);
            edge_ID++;
         }
      }
   }

//   cout << "Edge matrix = " << *edge_label_ptr << endl;
   return edge_ID;
}

// ==========================================================================
// Method generate_box_edges takes in the number of box corner
// vertices (=8) as well as genmatrix *P0_ptr containing their
// reconstructed 3D affine world positions.  It returns an STL vector
// containing linesegments corresponding to each edge within the box.

vector<linesegment>* generate_box_edges(int n_tiepoints,genmatrix* P0_ptr)
{
   Genarray<int>* edge_label_ptr=new Genarray<int>(n_tiepoints,n_tiepoints);

   int n_edges=generate_edge_label_array(n_tiepoints,edge_label_ptr);
   vector<linesegment>* edge_ptr=new vector<linesegment>;

   threevector V1(0,0);
   threevector V2(1,0);
   linesegment dummy(V1,V2);
   for (int e=0; e<n_edges; e++)
   {
      edge_ptr->push_back(dummy);
   }

   threevector corner0,corner1;
   for (int m=0; m<n_tiepoints; m++)
   {
      for (int n=m; n<n_tiepoints; n++)
      {
         int curr_edge_label=edge_label_ptr->get(m,n);
         if (curr_edge_label >= 0)
         {
            P0_ptr->get_column(m,corner0);
            P0_ptr->get_column(n,corner1);
            linesegment curr_edge(corner0,corner1);
            (*edge_ptr)[curr_edge_label]=curr_edge;
//            cout << "label = " << curr_edge_label
//                 << " edge = " << (*edge_ptr)[curr_edge_label] << endl;
         }
      } // loop over index n labeling columns in sym edge label matrix
   } // loop over index m labeling rows in sym edge label matrix

   delete edge_label_ptr;
   return edge_ptr;
}

// ==========================================================================
// Method generate_boxface_edge_array builds an integer valued array.
// Its first independent index labels box faces, while its second
// independent index cycles over box face edges 0 thru 3.  Its
// dependent integer index corresponds to *P0_ptr edge number.

void generate_boxface_edge_array(Genarray<int>* face_edge_ptr)
{
   face_edge_ptr->put(0,0,0);
   face_edge_ptr->put(0,1,3);
   face_edge_ptr->put(0,2,5);
   face_edge_ptr->put(0,3,1);

   face_edge_ptr->put(1,0,8);
   face_edge_ptr->put(1,1,10);
   face_edge_ptr->put(1,2,11);
   face_edge_ptr->put(1,3,9);

   face_edge_ptr->put(2,0,1);
   face_edge_ptr->put(2,1,7);
   face_edge_ptr->put(2,2,8);
   face_edge_ptr->put(2,3,2);

   face_edge_ptr->put(3,0,3);
   face_edge_ptr->put(3,1,4);
   face_edge_ptr->put(3,2,11);
   face_edge_ptr->put(3,3,6);

   face_edge_ptr->put(4,0,2);
   face_edge_ptr->put(4,1,9);
   face_edge_ptr->put(4,2,4);
   face_edge_ptr->put(4,3,0);

   face_edge_ptr->put(5,0,7);
   face_edge_ptr->put(5,1,5);
   face_edge_ptr->put(5,2,6);
   face_edge_ptr->put(5,3,10);
}

// ==========================================================================
// Method specify_perpendicular_edges hardwires the perpendicular
// relationships between edges for all 6 faces of the box.  Additional
// orthogonal dotproduct constraints between edges on different faces
// are NOT incorporated as of 2/5/06.

int specify_perpendicular_edges(genmatrix* perpendicular_edges_ptr)
{
   perpendicular_edges_ptr->initialize_values(NEGATIVEINFINITY);
   
   perpendicular_edges_ptr->put(0,3,0);
   perpendicular_edges_ptr->put(0,1,0);
   perpendicular_edges_ptr->put(1,5,0);
   perpendicular_edges_ptr->put(3,5,0);

   perpendicular_edges_ptr->put(8,9,0);
   perpendicular_edges_ptr->put(8,10,0);
   perpendicular_edges_ptr->put(9,11,0);
   perpendicular_edges_ptr->put(10,11,0);

   perpendicular_edges_ptr->put(1,2,0);
   perpendicular_edges_ptr->put(1,7,0);
   perpendicular_edges_ptr->put(2,8,0);
   perpendicular_edges_ptr->put(7,8,0);

   perpendicular_edges_ptr->put(3,4,0);
   perpendicular_edges_ptr->put(3,6,0);
   perpendicular_edges_ptr->put(4,11,0);
   perpendicular_edges_ptr->put(6,11,0);

   perpendicular_edges_ptr->put(0,2,0);
   perpendicular_edges_ptr->put(0,4,0);
   perpendicular_edges_ptr->put(2,9,0);
   perpendicular_edges_ptr->put(4,9,0);

   perpendicular_edges_ptr->put(5,6,0);
   perpendicular_edges_ptr->put(5,7,0);
   perpendicular_edges_ptr->put(6,10,0);
   perpendicular_edges_ptr->put(7,10,0);

// Symmetrize non-null valued entries in *perpendicular_edges_ptr:

   int n_constraints=0;
   for (int i=0; i<12; i++)
   {
      for (int j=0; j<12; j++)
      {
         double curr_dotproduct=perpendicular_edges_ptr->get(i,j);
         if (nearly_equal(curr_dotproduct,0))
         {
            perpendicular_edges_ptr->put(j,i,curr_dotproduct);
            n_constraints++;
         }
      }
   }

// Number of independent constraints = n_constraints/2:

   n_constraints /= 2;

   cout << "*perpendicular_edges_ptr = " << *perpendicular_edges_ptr << endl;
   return n_constraints;
}

// ==========================================================================
// Method specify_equal_length_edges hardwires same-length
// relationships between edges for all 6 faces of the box.  Additional
// orthogonal dotproduct constraints between edges on different faces
// are NOT incorporated as of 2/5/06.

int specify_equal_length_edges(genmatrix* equal_length_edges_ptr)
{
   equal_length_edges_ptr->initialize_values(NEGATIVEINFINITY);
   
   equal_length_edges_ptr->put(0,5,1);
   equal_length_edges_ptr->put(1,3,1);

   equal_length_edges_ptr->put(8,11,1);
   equal_length_edges_ptr->put(9,10,1);

   equal_length_edges_ptr->put(1,8,1);
   equal_length_edges_ptr->put(2,7,1);

   equal_length_edges_ptr->put(3,11,1);
   equal_length_edges_ptr->put(4,6,1);

   equal_length_edges_ptr->put(0,9,1);
   equal_length_edges_ptr->put(2,4,1);

   equal_length_edges_ptr->put(5,10,1);
   equal_length_edges_ptr->put(6,7,1);

// Symmetrize non-null valued entries in *equal_length_edges_ptr:

   int n_constraints=0;
   for (int i=0; i<12; i++)
   {
      for (int j=0; j<12; j++)
      {
         double curr_dotproduct=equal_length_edges_ptr->get(i,j);
         if (nearly_equal(curr_dotproduct,1))
         {
            equal_length_edges_ptr->put(j,i,curr_dotproduct);
            n_constraints++;
         }
      }
   }

// Number of independent constraints = n_constraints/2:

   n_constraints /= 2;

   cout << "*equal_length_edges_ptr = " << *equal_length_edges_ptr << endl;
   return n_constraints;
}

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

/*
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Initialize constants and parameters read in from command line as
// well as ascii text file:

   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("2D imagery");

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
//   bool display_movie_world_time=true;
//   bool display_movie_state=false;
//   bool display_movie_number=false;
   bool display_movie_world_time=false;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,display_movie_state,
      display_movie_number,display_movie_world_time);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   ModeController_ptr->setState(ModeController::RUN_MOVIE);
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = 
      new osgGA::Custom2DManipulator(ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_2D_ptr);

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);

   string movie_filename=
      passes_group.get_videopass_ptr()->get_first_filename();
   texture_rectangle* texture_rectangle_ptr=
      movies_group.generate_new_texture_rectangle(movie_filename);
   Movie* movie_ptr=movies_group.generate_new_Movie(texture_rectangle_ptr);

   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

// Attach the scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);
*/

// ==========================================================================
// Constant definitions
// ==========================================================================

   const int nxbins=501;
   const int nybins=501;
   const double max_x=3;  // meters
   const double max_y=3;  // meters
	
   bool input_param_file;
   unsigned int ninputlines,currlinenumber=0;
   string inputline[200];
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Initialize 3D image output:

   threevector origin(0,0,0);
   string xyzp_filename="./images/fitimage/affine.xyzp";
   filefunc::deletefile(xyzp_filename);
   draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
      xyzp_filename,origin,false);

// Enter box rotation angles:

   double psi,phi,theta;
   psi=10;
   phi=20;
   theta=30;

   mybox box_orig=generate_box(psi,phi,theta);
   vector<threevector> XYZ;
   box_orig.XYZ_vertex_positions(XYZ);

// ==========================================================================
// Generate projected vertices for all affine images:

   int n_images=5;
   cout << "Enter number of affine images to process:" << endl;
   cin >> n_images;

   vector<double> alpha,beta,gamma,scale_factor;
   vector<colorfunc::Color> color;

   alpha.reserve(n_images);
   beta.reserve(n_images);
   gamma.reserve(n_images);
   scale_factor.reserve(n_images);
   color.reserve(n_images);
   
   for (int i=0; i<n_images; i++)
   {
      alpha.push_back(10);
//      alpha.push_back(5);
      beta.push_back(-10);
      gamma.push_back(15);
//      gamma.push_back(15);
      scale_factor.push_back(0.5+i*0.25);
   }

   color.push_back(colorfunc::purple);
   color.push_back(colorfunc::cyan);
   color.push_back(colorfunc::green);
   color.push_back(colorfunc::yellow);
   color.push_back(colorfunc::red);

   vector<twovector> UV[n_images];
   mybox box[n_images];
   mybox curr_box(box_orig);
   for (int i=0; i < n_images; i++)
   {
      rotate_box(alpha[i],beta[i],gamma[i],curr_box);
      box[i]=curr_box;
      box[i].XY_vertex_projections(UV[i],scale_factor[i]);

//      ztwoDarray_ptr->clear_values();
//      drawfunc::draw_parallelepiped(
//         box[i],color[i],ztwoDarray_ptr,scale_factor[i]);
   }
   
   int n_tiepoints=UV[0].size();

// ==========================================================================
// Add noise to all tiepoint locations:

   double noise_mag=0;
   cout << "Enter noise magnitude:" << endl;
   cin >> noise_mag;

   double delta[2];
   for (int i=0; i < n_images; i++)
   {
      for (int j=0; j<n_tiepoints; j++)
      {
         delta[0]=noise_mag*(2*(nrfunc::ran1()-0.5));
         delta[1]=noise_mag*(2*(nrfunc::ran1()-0.5));
         (UV[i])[j] += twovector(delta[0],delta[1]);
      }
   }
   
// Instantiate reconstruction F object and load it with tiepoint
// information stored in UV STL vectors:

   Reconstruction reconstruction;
   vector< vector<twovector> > tiepoints;
   for (int i=0; i < n_images; i++)
   {
      tiepoints.push_back(UV[i]);
   }

   int mdim=2*n_images;
   int ndim=n_tiepoints;

   reconstruction.parse_multiimage_tiepoints(mdim,ndim,tiepoints);

// Perform affine reconstruction of camera orientations and target
// structure:

   genmatrix* A0_ptr=new genmatrix(mdim,3);

// *P0_ptr holds reconstructed affine XYZ points with no imposed
// target constraints:

   genmatrix* P0_ptr=new genmatrix(3,ndim);

   reconstruction.recenter_measurement_matrix(mdim,ndim);
   reconstruction.reconstruct_affine_structure(mdim,ndim,A0_ptr,P0_ptr);
   genmatrix* Dnew_ptr=reconstruction.recompute_measurement_matrix(
      mdim,ndim,A0_ptr,P0_ptr);


   vector<linesegment>* edge_ptr=generate_box_edges(n_tiepoints,P0_ptr);
   for(int e=0; e<edge_ptr->size(); e++)
   {
      cout << "Edge e = " << e << endl;
      cout << (*edge_ptr)[e] << endl;
   }

// Impose target reconstruction constraints.  In particular, declare
// certain angles to be 90 degrees in world space.  Moreover,
// stipulate that various pairs of edges are parallel and of equal
// length:
 
   genmatrix* perpendicular_edges_ptr=new genmatrix(12,12);
   genmatrix* equal_length_edges_ptr=new genmatrix(12,12);
   int n_perpendicular_constraints=specify_perpendicular_edges(
      perpendicular_edges_ptr);
   int n_equal_length_constraints=specify_equal_length_edges(
      equal_length_edges_ptr);

// Genmatrix A represents invertible linear transformation which
// transforms an affine to a Euclidean reconstruction:

   genmatrix A(3,3);
   reconstruction.affine_to_euclidean_transformation(
      n_images,n_perpendicular_constraints,n_equal_length_constraints,
      A0_ptr,edge_ptr,perpendicular_edges_ptr,equal_length_edges_ptr,A);

// Reconstruct affine image planes and cross range scales:

   vector<threevector> camera_axes;
   vector<double> u_scale;
   reconstruction.extract_imageplanes_and_uscales(
      A0_ptr,A,camera_axes,u_scale);

// *P1_ptr holds reconstructed Euclidean box XYZ points:

   genmatrix* P1_ptr=new genmatrix(*P0_ptr);
   reconstruction.reconstruct_Euclidean_worldpoints(A,P1_ptr);

   vector<threevector> UVW;
   reconstruction.compute_orthographic_coords(P1_ptr,camera_axes,u_scale,UVW);

/*
// Draw noisy affine image plane points plus reconstructed
// orthographic coordinates:

   int counter=0;
   for (int i=0; i < n_images; i++)
   {
      ztwoDarray_ptr->clear_values();

      box[i].translate(-box[i].get_center().get_pnt());
      drawfunc::draw_parallelepiped(
         box[i],color[i],ztwoDarray_ptr,scale_factor[i]);

      for (int j=0; j<n_tiepoints; j++)
      {
         twovector curr_UV=(UV[i])[j];
         drawfunc::draw_hugepoint(curr_UV,0.03,1000,ztwoDarray_ptr);

         drawfunc::draw_hugepoint(
            twovector(UVW[counter++]),0.03,60,ztwoDarray_ptr);

      } // loop over index j labeling tiepoint
//      string metafilename="noisy_affine"+stringfunc::number_to_string(i);
//      xyzimage.writeimage(metafilename,ztwoDarray_ptr);
   } // loop over index i labeling image
*/

// Draw reconstructed 3D parallelepiped:

   vector<threevector> corner;
   for (int j=0; j<n_tiepoints; j++)
   {
      threevector curr_corner(P1_ptr->get(0,j),P1_ptr->get(1,j),
                              P1_ptr->get(2,j));
      corner.push_back(curr_corner);
   }

   origin=threevector(0,0,0);
   for (int c=0; c<8; c++)
   {
      origin += 0.125*corner[c];
   }
   threevector U(0.25*(
      (corner[7]-corner[4])+(corner[6]-corner[5])+
      (corner[1]-corner[0])+(corner[2]-corner[3])));
   threevector V(0.25*(
      (corner[5]-corner[4])+(corner[6]-corner[7])+
      (corner[2]-corner[1])+(corner[3]-corner[0])));
   threevector W(0.25*(
      (corner[0]-corner[4])+(corner[3]-corner[5])+
      (corner[1]-corner[7])+(corner[2]-corner[6])));
   
   parallelepiped p(U,V,W,origin);
   p.translate(-0.5*(U+V+W));
   p.calculate_symmetry_vectors_and_lengths();

   draw3Dfunc::ds=0.001;
   draw3Dfunc::draw_parallelepiped(p,xyzp_filename,1.0);

   cout << endl;
   cout << "Reconstructed parallelepiped: " << p << endl;

   for (int j=0; j<n_tiepoints; j++)
   {
      draw3Dfunc::draw_tiny_cube(corner[j],xyzp_filename,j*0.1,0.1);

      threeDstring vertex_label(stringfunc::number_to_string(j));
      double vertex_label_size=0.1;
      vertex_label.scale(vertex_label.get_origin().get_pnt(),
                         vertex_label_size);
      threevector vertex_label_location(corner[j]);
      vertex_label.center_upon_location(vertex_label_location);
//      draw3Dfunc::draw_threeDstring(vertex_label,xyzp_filename,
//                                    draw3Dfunc::annotation_value2);
   }

   delete A0_ptr;
   delete P0_ptr;
   delete Dnew_ptr;
   delete P1_ptr;
   delete edge_ptr;
   delete perpendicular_edges_ptr;
   delete equal_length_edges_ptr;

// ==========================================================================
// Draw U and V camera axes for all affine images:

   threevector u0_hat=camera_axes[0];
   threevector v0_hat=camera_axes[1];
   threevector w0_hat=camera_axes[2];

   for (int i=0; i<n_images; i++)
   {
      threevector u_hat=camera_axes[3*i+0];
      threevector v_hat=camera_axes[3*i+1];
      threevector w_hat=camera_axes[3*i+2];

// Compute rotation matrix R which maps current (u_hat,v_hat,w_hat)
// into (u0,v0,w0).  Then decompose R into 3 independent angles:

      genmatrix R(3,3);
      R.put(0,0,u0_hat.dot(u_hat));
      R.put(0,1,u0_hat.dot(v_hat));
      R.put(0,2,u0_hat.dot(w_hat));
      R.put(1,0,v0_hat.dot(u_hat));
      R.put(1,1,v0_hat.dot(v_hat));
      R.put(1,2,v0_hat.dot(w_hat));
      R.put(2,0,w0_hat.dot(u_hat));
      R.put(2,1,w0_hat.dot(v_hat));
      R.put(2,2,w0_hat.dot(w_hat));
      
      double Theta,Phi,Chi;
      mathfunc::decompose_orthogonal_matrix(R,Theta,Phi,Chi);

      cout << "image i = " << i 
           << " cross range scale = " << u_scale[i] << endl;
      cout << "image i = " << i 
           << " Theta = " << Theta*180/PI << " Phi = " << Phi*180/PI
           << " Chi = " << Chi*180/PI << endl;
      
//      cout << "image i = " << i << " u_hat = " << u_hat
//           << " v_hat = " << v_hat << " w_hat = " << w_hat << endl;
//      cout << "w_hat.mag = " << w_hat.magnitude() << endl;

      const double window_size=4;
      const double radius=5;

      vector<threevector> vertex;
      vertex.push_back(window_size*u_hat+window_size*v_hat);
      vertex.push_back(-window_size*u_hat+window_size*v_hat);
      vertex.push_back(-window_size*u_hat-window_size*v_hat);
      vertex.push_back(window_size*u_hat-window_size*v_hat);
      polygon window(vertex);
      window.absolute_position(radius*w_hat);

//      cout << "window = " << window << endl;

//      draw3Dfunc::draw_polygon(window,xyzp_filename,0.1*(i+1));
//      draw3Dfunc::draw_polygon(window,xyzp_filename,1.0);

      double annotation_value;
      switch (i)
      {
         case 0:
            annotation_value=0.96;	// purple
            break;
         case 1:
//            annotation_value=0.78;	// blue
            annotation_value=0.59;	// cyan
            break;
         case 2:
            annotation_value=0.4;	// green
            break;
         case 3:
            annotation_value=0.2;	// yellow
            break;
         case 4:
            annotation_value=0.02;	// red
            break;
         default:
            annotation_value=0.5;
      }
      draw3Dfunc::draw_polygon(window,xyzp_filename,annotation_value);

   } // loop over index i labeling affine image

/*
// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }
*/

}

