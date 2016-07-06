// ==========================================================================
// POINTSGROUP class member function definitions
// ==========================================================================
// Last modified on 12/17/09; 12/4/10; 12/30/10; 4/5/14
// ==========================================================================

#include <iomanip>
#include <vector>
#include <osg/Geode>
#include <osg/Group>
#include "osg/osgGraphicals/AnimationController.h"
#include "math/basic_math.h"
#include "astro_geo/Clock.h"
#include "math/constant_vectors.h"
#include "geometry/convexhull.h"
#include "astro_geo/Ellipsoid_model.h"
#include "general/filefuncs.h"
#include "general/inputfuncs.h"
#include "osg/osgGraphicals/instantaneous_obs.h"
#include "osg/ModeController.h"
#include "general/outputfuncs.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "geometry/polygon.h"
#include "general/stringfuncs.h"

#include "osg/osgfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::setw;
using std::string;
using std::vector;

namespace osgGeometry
{

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

   void PointsGroup::allocate_member_objects()
      {
      }		       

// Modified cross hair sizes in December 2007 for satellite imagery
// analysis purposes:

   void PointsGroup::initialize_member_objects()
      {
         GraphicalsGroup_name="PointsGroup";

         crosshairs_size[2]=0.025;

//         crosshairs_size[3]=0.1;	// for real satellites
         crosshairs_size[3]=3.0;

         crosshairs_text_size[2]=0.025;

//         crosshairs_text_size[3]=0.1;	// for real satellites
         crosshairs_text_size[3]=4.0;


         get_OSGgroup_ptr()->setUpdateCallback( 
            new AbstractOSGCallback<PointsGroup>(
               this, &PointsGroup::update_display));
      }		       

   PointsGroup::PointsGroup(const int p_ndims,Pass* PI_ptr,
                            AnimationController* AC_ptr):
      GeometricalsGroup(p_ndims,PI_ptr,AC_ptr)
      {	
         initialize_member_objects();
         allocate_member_objects();
      }		       

   PointsGroup::PointsGroup(const int p_ndims,Pass* PI_ptr,
                            threevector* GO_ptr):
      GeometricalsGroup(p_ndims,PI_ptr,GO_ptr)
      {	
         initialize_member_objects();
         allocate_member_objects();
      }		       

   PointsGroup::PointsGroup(
      const int p_ndims,Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
      threevector* GO_ptr):
      GeometricalsGroup(p_ndims,PI_ptr,clock_ptr,EM_ptr,GO_ptr)
      {	
         initialize_member_objects();
         allocate_member_objects();
      }		       

   PointsGroup::PointsGroup(const int p_ndims,Pass* PI_ptr,
                            AnimationController* AC_ptr,threevector* GO_ptr):
      GeometricalsGroup(p_ndims,PI_ptr,AC_ptr,GO_ptr)
      {	
         initialize_member_objects();
         allocate_member_objects();
      }		       

   PointsGroup::~PointsGroup()
      {
//         cout << "inside PointsGroup destructor" << endl;
      }

// ---------------------------------------------------------------------
// Overload << operator

   ostream& operator<< (ostream& outstream,const PointsGroup& f)
      {
         int node_counter=0;
         for (unsigned int n=0; n<f.get_n_Graphicals(); n++)
         {
            Point* point_ptr=f.get_Point_ptr(n);
            outstream << "point node # " << node_counter++ << endl;
            outstream << "point = " << *point_ptr << endl;
         }
         return outstream;
      }

// ==========================================================================
// Point creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Point from all other graphical insertion
// and manipulation methods...

   Point* PointsGroup::generate_new_Point(
      bool draw_text_flag,bool earth_point_flag,int ID,
      unsigned int OSGsubPAT_number)
      {
         if (ID==-1) ID=get_next_unused_ID();
         Point* curr_Point_ptr=new Point(
            get_ndims(),ID,AnimationController_ptr);
         initialize_new_Point(Zero_vector,curr_Point_ptr,OSGsubPAT_number);

         osg::Geode* geode_ptr=generate_point_geode(
            curr_Point_ptr,draw_text_flag,earth_point_flag);
         curr_Point_ptr->get_PAT_ptr()->addChild(geode_ptr);

         return curr_Point_ptr;
      }

// --------------------------------------------------------------------------
// Member function generate_new_Point creates a new point located at
// input threevector V.

   Point* PointsGroup::generate_new_Point(
      const threevector& V,bool draw_text_flag,bool earth_point_flag,int ID,
      unsigned int OSGsubPAT_number)
      {
//         cout << "inside PointsGroup::generate_new_Point #2, V = " 
//              << V << endl;
         
         if (ID==-1) ID=get_next_unused_ID();
         Point* curr_Point_ptr=new Point(
            get_ndims(),ID,AnimationController_ptr);
         initialize_new_Point(V,curr_Point_ptr,OSGsubPAT_number);
 
         osg::Geode* geode_ptr=generate_point_geode(
            curr_Point_ptr,draw_text_flag,earth_point_flag);
         curr_Point_ptr->get_PAT_ptr()->addChild(geode_ptr);

         return curr_Point_ptr;
      }		       

// ---------------------------------------------------------------------
   void PointsGroup::initialize_new_Point(
      const threevector& V,Point* Point_ptr,unsigned int OSGsubPAT_number)
      {
//   cout << "inside PointsGroup::initialize_new_Point()" << endl;
         
         GraphicalsGroup::insert_Graphical_into_list(Point_ptr);
         initialize_Graphical(V,Point_ptr);
         insert_graphical_PAT_into_OSGsubPAT(Point_ptr,OSGsubPAT_number);
      }

// --------------------------------------------------------------------------
   osg::Geode* PointsGroup::generate_point_geode(
      Point* Point_ptr,bool draw_text_flag,bool earth_point_flag)
      {
//         cout << "inside PointsGroup::generate_point_geode()" << endl;

         threevector UVW;
         Point_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),UVW);
//         cout << "UVW = " << UVW << endl;

         if (earth_point_flag && Ellipsoid_model_ptr != NULL && 
             Clock_ptr != NULL)
         {
            double latitude,longitude,altitude;
            Ellipsoid_model_ptr->ConvertECIToLongLatAlt(
               UVW,*Clock_ptr,longitude,latitude,altitude);
      
            genmatrix* R_enr_to_ECI_ptr=Ellipsoid_model_ptr->
               east_north_radial_to_ECI_rotation(
                  latitude,longitude,*Clock_ptr);

            threevector east_hat,north_hat;
            R_enr_to_ECI_ptr->get_column(0,east_hat);
            R_enr_to_ECI_ptr->get_column(1,north_hat);

            Point_ptr->set_UVW_dirs(
               get_curr_t(),get_passnumber(),east_hat,north_hat);
         }

         osg::Geode* geode_ptr=Point_ptr->generate_drawable_geode(
            get_passnumber(),get_crosshairs_size(),get_crosshairs_text_size(),
            draw_text_flag,earth_point_flag);
         return geode_ptr;

      }

// --------------------------------------------------------------------------
// Member function edit_label allows the user to change the ID number
// associated with a point.  The new ID number must not conflict with
// any other existing point's ID.  It must also be non-negative.  The
// user enters the replacement ID for a selected point within the main
// console window.  (As of 7/10/05, we are unfortunately unable to
// robustly retrieve user input from the point text dialog window...)

   void PointsGroup::edit_label()
      {   
         cout << "Editing label for point " 
              << get_selected_Graphical_ID() << endl;

         Point* curr_Point_ptr=get_ID_labeled_Point_ptr(
            get_selected_Graphical_ID());
         if (curr_Point_ptr != NULL)
         {
            cout << endl;
            if (get_selected_Graphical_ID() != -1)
            {
               string label_command="Enter new number for point:";
               int new_point_ID=inputfunc::enter_nonnegative_integer(
                  label_command);
         
               Point* new_point_ptr=get_ID_labeled_Point_ptr(new_point_ID);
               if (new_point_ptr != NULL)
               {
                  cout << 
                     "Current label unchanged since it already exists in point list !! " 
                       << endl << endl;
               }
               else
               {
                  renumber_Graphical(curr_Point_ptr,new_point_ID);
                  curr_Point_ptr->set_ID(new_point_ID);
                  curr_Point_ptr->reset_text_label();
               }
            } // selected_Graphical_ID != -1 conditional

//      cout << "pointlist = " << *(get_pointlist_ptr()) << endl;
         } // currnode_point != NULL conditional
      }

// --------------------------------------------------------------------------
// Member function erase_point sets boolean entries within the
// member map coords_erased to true for the current point.  When
// point points are drawn within
// PointsGroup::reassign_PAT_ptrs(), entries within this STL map are
// first checked and their positions are set to large negative values
// to prevent them from appearing within the OSG data window.  Yet the
// point itself continues to exist.

   bool PointsGroup::erase_point()
      {   
//         cout << "inside PointsGroup::erase_point()" << endl;
         bool point_erased=erase_Graphical();
         return point_erased;
      }

// --------------------------------------------------------------------------
// Member function unerase_point queries the user to enter the ID
// for some erased point.  It then unerases that point within the
// current image.

   bool PointsGroup::unerase_point()
      {   
//         cout << "inside PointsGroup::unerase_point()" << endl;
         bool point_unerased_flag=unerase_Graphical();
         if (point_unerased_flag)
         {
            reset_colors();
         }
         return point_unerased_flag;
      }

// ==========================================================================
// Point destruction member functions
// ==========================================================================

// Member function destroy_all_Points() first fills an STL vector with
// Point pointers.  It then iterates over each vector entry and calls
// destroy_Point for each Point pointer.  On 5/3/08, we learned the
// hard and painful way that this two-step process is necessary in
// order to correctly purge all Points.

void PointsGroup::destroy_all_Points()
{   
//   cout << "inside PointsGroup::destroy_all_Points()" << endl;
   unsigned int n_Points=get_n_Graphicals();

   vector<Point*> Points_to_destroy;
   for (unsigned int p=0; p<n_Points; p++)
   {
      Point* Point_ptr=get_Point_ptr(p);
//      cout << "p = " << p << " Point_ptr = " << Point_ptr << endl;
      Point_ptr->dirtyDisplay();
      Points_to_destroy.push_back(Point_ptr);
   }

   for (unsigned int p=0; p<n_Points; p++)
   {
      destroy_Point(Points_to_destroy[p]->get_ID());
   }
//   n_Points=get_n_Graphicals();
}

// --------------------------------------------------------------------------
// Member function destroy_Point removes the selected point from
// the pointlist and the OSG points group.  If the point is
// successfully destroyed, its number is returned by this method.
// Otherwise, -1 is returned.

   int PointsGroup::destroy_Point()
      {   
//         cout << "inside PointsGroup::destroy_Point()" << endl;
         int destroyed_point_ID=destroy_Graphical();
         return destroyed_point_ID;
      }

   bool PointsGroup::destroy_Point(int Point_ID)
      {   
//         cout << "inside PointsGroup::destroy_Point(Point_ID)" << endl;
         return destroy_Graphical(Point_ID);
      }

// --------------------------------------------------------------------------
// Member function change_size multiplies the size parameter for cross
// hair objects corresponding to the current dimension by input
// parameter factor.  It returns the new crosshair size.

   double PointsGroup::change_size(double factor)
      {   
//         cout << "inside PointsGroup::change_size()" << endl;
//         cout << "get_ndims = " << get_ndims() << endl;
         
         crosshairs_size[get_ndims()] *= factor;
         crosshairs_text_size[get_ndims()] *= factor;

         for (unsigned int n=0; n<get_n_Graphicals(); n++)
         {
            Point* point_ptr=get_Point_ptr(n);
            point_ptr->set_crosshairs_coords(get_crosshairs_size());

            for (unsigned int m=0; m<point_ptr->get_n_text_messages(); m++)
            {
               osgText::Text* text_ptr=point_ptr->get_text_ptr(m);
               if (text_ptr != NULL) 
                  point_ptr->change_text_size(text_ptr,factor);
            }

            point_ptr->set_crosshairsnumber_text_posn(
               get_crosshairs_text_size());

// Follow Ross Anderson's suggestion from Feb 2008 and add call to
// dirtyDisplay whenever cross hairs are resized:

            point_ptr->dirtyDisplay();
         }
         return get_crosshairs_size();
      }

// --------------------------------------------------------------------------
// Member function move_z moves 3D points up or down in world-space
// z.  This specialized method was constructed to facilitate movement
// of 3D cursors in the z direction which is highly special for ALIRT
// imagery.

   void PointsGroup::move_z(int sgn)
      {
         if (get_ndims()==3)
         {
            GraphicalsGroup::move_z(sgn);
         } // get_ndims()==3 conditional
      }

// --------------------------------------------------------------------------
// Member function refresh_crosshair_coords resets the coordinates for
// the current set of crosshairs.  This method is needed by main
// program FUSION where we have both 2D and 3D crosshairs appearing in
// separate windows.  Both sets of crosshairs are refreshed each time
// update_display() is executed via a callback.  

   void PointsGroup::refresh_crosshair_coords()
      {   
         for (unsigned int n=0; n<get_n_Graphicals(); n++)
         {
            Point* Point_ptr=get_Point_ptr(n);
            Point_ptr->set_crosshairs_coords(get_crosshairs_size());
         }
      }

// ==========================================================================
// Ascii point file I/O methods
// ==========================================================================

// Member function save_point_info_to_file loops over all points
// within *get_pointlist_ptr() and prints their times, IDs, pass numbers
// and UVW coordinates to the output ofstream.  This point
// information can later be read back in via member function
// read_point_info_from_file.

   void PointsGroup::save_point_info_to_file()
      {
         string output_filename=get_output_filename("points");

         int output_passnumber;
         cout << "Current working passnumber = " << get_passnumber() << endl;
         cout << "Enter output passnumber for points to be saved to ascii file:"
              << endl;
         cin >> output_passnumber;

         ofstream outstream;
         outstream.precision(6);
         filefunc::openfile(output_filename,outstream);
         for (unsigned int imagenumber=get_first_framenumber(); 
              imagenumber <= get_last_framenumber(); imagenumber++)
         {
            bool data_written_flag=false;
            double curr_t=static_cast<double>(imagenumber);
            for (unsigned int n=0; n<get_n_Graphicals(); n++)
            {
               Point* point_ptr=get_Point_ptr(n);
               instantaneous_obs* curr_obs_ptr=
                  point_ptr->get_particular_time_obs(
                     curr_t,get_passnumber());
               if (curr_obs_ptr != NULL)
               {
                  bool erased_point_flag=false;

// Store UVW coords measured wrt instantaneous U_hat, V_hat and W_hat
// which need not coincide with X_hat, Y_hat and Z_hat within
// p_transformed:
         
                  threevector p_transformed;
                  vector<double> column_data;
                  if (point_ptr->get_transformed_UVW_coords(
                     curr_t,get_passnumber(),p_transformed))
                  {
                     if (point_ptr->get_mask(curr_t,get_passnumber()))
                     {
                        erased_point_flag=true;
                     }
                     else
                     {
                        for (unsigned int j=0; j<get_ndims(); j++)
                        {
                           column_data.push_back(p_transformed.get(j));
                        }
                     }
                  }
                  if (!erased_point_flag) 
                  {
                     outstream.setf(ios::showpoint);
                     outstream << setw(5) << curr_t 
                               << setw(6) << point_ptr->get_ID() 
                               << setw(5) << output_passnumber;
                     for (unsigned int j=0; j<get_ndims(); j++)
                     {
                        outstream.precision(10);
                        outstream << setw(14) << column_data[j];
                     }

//               outstream << endl;
                     data_written_flag=true;
                  } // !erased_point_flag conditional
               } // curr_obs_ptr != NULL conditional
            } // loop over nodes in *get_pointlist_ptr()
            if (data_written_flag) outstream << endl;
         } // loop over imagenumber index

         filefunc::closefile(output_filename,outstream);
      }

// --------------------------------------------------------------------------
// Member function read_point_info_from_file parses the ascii text
// file generated by member function save_point_info_to_file().
// After purging the pointlist, this method regenerates the points
// within the list based upon the ascii text file information.

   void PointsGroup::read_point_info_from_file(
      bool propagate_points_flag)
      {
         string input_filename=get_input_filename("points");
         filefunc::ReadInfile(input_filename);

         vector<string> fields=stringfunc::decompose_string_into_substrings(
            filefunc::text_line[0]);
         unsigned int n_fields=fields.size();

         double X[n_fields];
         vector<double> curr_time;
         vector<int> point_ID,pass_number;
         vector<threevector> UVW;
         curr_time.reserve(filefunc::text_line.size());
         point_ID.reserve(filefunc::text_line.size());
         pass_number.reserve(filefunc::text_line.size());
         UVW.reserve(filefunc::text_line.size());

         for (unsigned int i=0; i<filefunc::text_line.size(); i++)
         {
            stringfunc::string_to_n_numbers(n_fields,filefunc::text_line[i],X);
            double U=X[3];
            double V=X[4];
            double W=0;
            if (get_ndims()==3) W=X[5];

// On 9/22/05, we discovered that KLT point tracking results
// sometimes are nonsensical and lie outside the allowed UV imageplane
// region.  We therefore perform a sanity check on U and V before
// pushing them onto the STL vectors:

            {
               curr_time.push_back(X[0]);

               point_ID.push_back(basic_math::round(X[1]));
               pass_number.push_back(get_passnumber());
//         pass_number.push_back(basic_math::round(X[2]));
               UVW.push_back(threevector(U,V,W));
         
            } // UV_OK conditional
         } // loop over index i labeling ascii file line number

//   for (unsigned int i=0; i<curr_time.size(); i++)
//   {
//      cout << "time = " << curr_time[i]
//           << " ID = " << point_ID[i]
//           << " pass = " << pass_number[i];
//      cout << " UVW = " << UVW[i].get(0) << " " 
//           << UVW[i].get(1) << " " << UVW[i].get(2) 
//           << endl;
//   }

// Destroy all existing points before creating a new point list
// from the input ascii file:

         destroy_all_Graphicals();

         outputfunc::write_banner(
            "Instantiating & loading new set of points:");
         for (unsigned int i=0; i<point_ID.size(); i++)
         {
            if (i%1000==0) cout << i/1000 << " " << flush;

            Point* point_ptr=get_ID_labeled_Point_ptr(point_ID[i]);
            if (point_ptr == NULL)
            {
               point_ptr=generate_new_Point(point_ID[i]);
//         initialize_Graphical(UVW[i],point_ptr,NULL,get_OSGgroup_ptr());
               insert_graphical_PAT_into_OSGsubPAT(point_ptr,0);

// Initialize point's coords_erased flag to true for all images
// within the current pass:
      
               for (unsigned int n=get_first_framenumber(); 
                    n<=get_last_framenumber(); n++)
               {
                  double curr_t=static_cast<double>(n);
                  point_ptr->set_mask(curr_t,get_passnumber(),true);
               }

            } // point_ptr==NULL conditional

// Load time, imagenumber, passnumber and UVW coordinate information
// into current point.  Set manually_manipulated flag to true and
// coords_erased flag to false for each STL vector entry:

            point_ptr->set_UVW_coords(curr_time[i],pass_number[i],UVW[i]);
            point_ptr->set_coords_manually_manipulated(
               curr_time[i],pass_number[i]);
            point_ptr->set_mask(curr_time[i],pass_number[i],false);

         } // loop over index i labeling entries in STL time, point_ID
         // and UVW vectors
         outputfunc::newline();

         reset_colors();

// Recall that we typically initialize the Points OSG group's nodemask
// to zero.  In order to effectively reattach the read-in points to
// the scenegraph, we need to reset their group's node mask to 1:

         get_OSGgroup_ptr()->setNodeMask(1);
      }

// --------------------------------------------------------------------------
// Member functions minimum[maximum]_point_ID loops over all points'
// IDs and returns their minimal [maximal] value.

   int PointsGroup::minimum_point_ID()
      {
//         cout << "inside PointsGroup::minimum_point_ID()" << endl;
         int min_ID=POSITIVEINFINITY;
         for (unsigned int n=0; n<get_n_Graphicals(); n++)
         {
            if (n%1000==0) cout << n/1000 << " " << flush;
            Point* point_ptr=get_Point_ptr(n);
            min_ID=basic_math::min(min_ID,point_ptr->get_ID());
         }
         cout << endl;
         return min_ID;
      }

   int PointsGroup::maximum_point_ID()
      {
//         cout << "inside PointsGroup::maximum_point_ID()" << endl;
         int max_ID=-1;
         for (unsigned int n=0; n<get_n_Graphicals(); n++)
         {
            Point* point_ptr=get_Point_ptr(n);
            max_ID=basic_math::max(max_ID,point_ptr->get_ID());
         }
         return max_ID;
      }

// --------------------------------------------------------------------------
// Member function update_display()

void PointsGroup::update_display()
{
//   cout << "inside PointsGroup::update_display()" << endl;
//   cout << "this = " << this << endl;
//   cout << "get_n_Graphicals() = " << get_n_Graphicals() << endl;

   reset_colors();

   GraphicalsGroup::update_display();
//   cout << "at end of PointsGroup::update_display()" << endl;
}

// ==========================================================================
// Region generation methods:
// ==========================================================================

// Auxilliary member function generate_convexhull_poly computes and
// returns the 2D convex hull of the (U,V) points within the current
// PointsGroup object.  We experimented with this method on 9/24/06 in
// the hope that it could magically determine the correct ordering of
// a random set of 2D points needed to construct an arbitrary polygon.
// But we convinced ourselves that even knowing the convex hull of an
// arbitrarily complicated set of points is insufficient to uniquely
// determine their ordering for polygon formation purposes.

   polygon* PointsGroup::generate_convexhull_poly(
      double U_to_pu_factor,double V_to_pv_factor)
      {
         polygon* convexhull_ptr=NULL;
         if (get_ndims()==2)
         {
            threevector curr_UVW;
            vector<int> vertex_order;
            vector<threevector> pixel_vertex;

            for (unsigned int n=0; n<get_n_Graphicals(); n++)
            {
               Point* point_ptr=get_Point_ptr(n);
               point_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),
                                         curr_UVW);
               int pu=basic_math::round(curr_UVW.get(0)*U_to_pu_factor);
               int pv=basic_math::round(curr_UVW.get(1)*V_to_pv_factor);
               int pw=0;

               pixel_vertex.push_back(threevector(pu,pv,pw));
               vertex_order.push_back(n);
               cout << "n = " << n << " curr_UVW = " << curr_UVW << endl;
               cout << " pixel_vertex = " << pixel_vertex.back() << endl;
            }

            convexhull_ptr=convexhull::convex_hull_poly(
               pixel_vertex,vertex_order);
            for (unsigned int n=0; n<pixel_vertex.size(); n++)
            {
               cout << "n = " << n 
                    << " pixel_vertex = " << pixel_vertex[n].get(0) << ","
                    << pixel_vertex[n].get(1) << " vertex order = "
                    << vertex_order[n] << endl;
            }
      
            cout << "*convexhull_ptr = " << *convexhull_ptr << endl;
            delete convexhull_ptr;
         } // ndims==2 conditional
         return convexhull_ptr;
      }

// --------------------------------------------------------------------------
// Member function generate_poly assumes that the points within the
// current PointsGroup object are monotonically ordered in an
// ascending fashion to form a well-defined polygon.  It dynamically
// instantiates and returns the 2D polygon formed by the points.

   polygon* PointsGroup::generate_poly()
      {
         polygon* poly_ptr=NULL;
         if (get_ndims()==2)
         {
            threevector curr_UVW;
            vector<int> vertex_order;
            vector<threevector> vertex;

            for (unsigned int n=0; n<get_n_Graphicals(); n++)
            {
               Point* point_ptr=get_Point_ptr(n);
               if (!point_ptr->get_mask(get_curr_t(),get_passnumber()))
               {
                  vertex_order.push_back(point_ptr->get_ID());
                  point_ptr->get_UVW_coords(
                     get_curr_t(),get_passnumber(),curr_UVW);
                  vertex.push_back(curr_UVW);
               }
            }
            templatefunc::Quicksort(vertex_order,vertex);
            poly_ptr=new polygon(vertex);

//      for (unsigned int n=0; n<get_n_Graphicals(); n++)
//      {
//         cout << "n = " << n  << " vertex order = " << vertex_order[n]
//              << " U = " << vertex[n].get(0) 
//              << " V = " << vertex[n].get(1) << endl;
//      }

         }
         return poly_ptr;
      }

} // osgGeometry namespace
