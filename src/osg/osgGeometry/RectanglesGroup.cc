// ==========================================================================
// RECTANGLESGROUP class member function definitions
// ==========================================================================
// Last modified on 2/10/08; 6/15/08; 2/17/09; 7/9/11; 4/5/14
// ==========================================================================

#include <iomanip>
#include <string>
#include <vector>
#include <osg/Geode>
#include "color/colorfuncs.h"
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "osg/osgGeometry/Rectangle.h"
#include "osg/osgGeometry/RectanglesGroup.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::setw;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void RectanglesGroup::allocate_member_objects()
{
}		       

void RectanglesGroup::initialize_member_objects()
{
   GraphicalsGroup_name="RectanglesGroup";

   size[2]=1.0;
//   size[3]=1.0;
   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<RectanglesGroup>(
         this, &RectanglesGroup::update_display));
}		       

RectanglesGroup::RectanglesGroup(const int p_ndims,Pass* PI_ptr):
   GeometricalsGroup(p_ndims,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

RectanglesGroup::~RectanglesGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const RectanglesGroup& f)
{
   int node_counter=0;
   for (unsigned int n=0; n<f.get_n_Graphicals(); n++)
   {
      Rectangle* Rectangle_ptr=f.get_Rectangle_ptr(n);
      outstream << "Rectangle node # " << node_counter++ << endl;
      outstream << "Rectangle = " << *Rectangle_ptr << endl;
   }
   return outstream;
}

// ==========================================================================
// Rectangle creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Rectangle from all other graphical insertion
// and manipulation methods...

Rectangle* RectanglesGroup::generate_new_Rectangle(
   int ID,unsigned int OSGsubPAT_number)
{
   threevector V(Zero_vector);
   Rectangle* curr_Rectangle_ptr=generate_new_Rectangle(
      V,ID,OSGsubPAT_number);

   return curr_Rectangle_ptr;
}

Rectangle* RectanglesGroup::generate_new_Rectangle(
   const threevector& V,int ID,unsigned int OSGsubPAT_number)
{
   if (ID==-1) ID=get_next_unused_ID();
   Rectangle* curr_Rectangle_ptr=new Rectangle(ID);

   initialize_new_Rectangle(V,curr_Rectangle_ptr,OSGsubPAT_number);

   osg::Geode* geode_ptr=curr_Rectangle_ptr->generate_drawable_geode();
   curr_Rectangle_ptr->get_PAT_ptr()->addChild(geode_ptr);

   reset_colors();
   
   return curr_Rectangle_ptr;
}

// ---------------------------------------------------------------------
void RectanglesGroup::initialize_new_Rectangle(
   const threevector& V,Rectangle* curr_Rectangle_ptr,
   unsigned int OSGsubPAT_number)
{
//   cout << "inside RectanglesGroup::initialize_new_Rectangle" << endl;

   GraphicalsGroup::insert_Graphical_into_list(curr_Rectangle_ptr);
   initialize_Graphical(V,curr_Rectangle_ptr);

   insert_graphical_PAT_into_OSGsubPAT(curr_Rectangle_ptr,OSGsubPAT_number);
}

// --------------------------------------------------------------------------
// Member function erase_Rectangle sets boolean entries within the
// member map coords_erased to true for the current Rectangle.  When
// Rectangle crosshairs are drawn within
// RectanglesGroup::reassign_PAT_ptrs(), entries within this STL map are
// first checked and their positions are set to large negative values
// to prevent them from appearing within the OSG data window.  Yet the
// Rectangle itself continues to exist.

bool RectanglesGroup::erase_Rectangle()
{   
   return erase_Graphical();
}

// --------------------------------------------------------------------------
// Member function unerase_Rectangle queries the user to enter the ID
// for some erased Rectangle.  It then unerases that Rectangle within the
// current image.

bool RectanglesGroup::unerase_Rectangle()
{   
   bool Rectangle_unerased_flag=unerase_Graphical();
   if (Rectangle_unerased_flag) reset_colors();
   return Rectangle_unerased_flag;
}

// --------------------------------------------------------------------------
// Member function reset_colors loops over all entries within the
// current video image's Rectanglelist.  It specially colors the
// rectangle whose ID equals selected_Graphical_ID.  All other
// rectangles are set to their permanent colors.

void RectanglesGroup::reset_colors()
{   
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Rectangle* Rectangle_ptr=get_Rectangle_ptr(n);
      if (get_selected_Graphical_ID()==Rectangle_ptr->get_ID())
      {
         Rectangle_ptr->set_curr_color(
            Rectangle_ptr->get_selected_color());
      }
      else
      {
         Rectangle_ptr->set_curr_color(
            Rectangle_ptr->get_permanent_color());
      }
   } // loop over Rectangles in Rectanglelist
}

// ==========================================================================
// Ascii file I/O methods
// ==========================================================================

// Member function save_info_to_file loops over all Rectangles within
// *get_Rectanglelist_ptr() and prints their times, IDs, pass numbers
// and UVW coordinates to the output ofstream.  This Rectangle
// information can later be read back in via member function
// read_info_from_file.

void RectanglesGroup::save_info_to_file()
{
   cout << "inside RectanglesGroup::save_info_to_file()" << endl;

//   string output_filename=get_output_filename("rectangles");
   string output_filename="rectangles.txt";

   ofstream outstream;
   outstream.precision(6);
   outstream.setf(ios::showpoint);
   filefunc::openfile(output_filename,outstream);

   const int column_width=11;
   const int n_vertices=4;
   threevector screenspace_vertex[n_vertices];
   for (unsigned int imagenumber=get_first_framenumber(); 
        imagenumber <= get_last_framenumber(); imagenumber++)
   {
      double curr_t=static_cast<double>(imagenumber);
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         Rectangle* Rectangle_ptr=get_Rectangle_ptr(n);
         
         instantaneous_obs* curr_obs_ptr=Rectangle_ptr->
            get_particular_time_obs(curr_t,get_passnumber());
         if (curr_obs_ptr != NULL)
         {
            if (!Rectangle_ptr->get_mask(curr_t,get_passnumber()))
            {
               outstream << setw(column_width) << curr_t
                         << setw(column_width) << Rectangle_ptr->get_ID() 
                         << setw(column_width) << get_passnumber() << endl;

               Rectangle_ptr->compute_screenspace_vertex_posns(
                  screenspace_vertex);

               for (int j=0; j<n_vertices; j++)
               {
                  outstream << setw(column_width) 
                            << screenspace_vertex[j].get(0)
                            << setw(column_width) 
                            << screenspace_vertex[j].get(1)
                            << setw(column_width) 
                            << screenspace_vertex[j].get(2) << endl;
               }
               outstream << endl;
            } // !erased_flag conditional
         } // curr_obs_ptr != NULL conditional
      } // loop over nodes in *get_Rectanglelist_ptr()
   } // loop over imagenumber index
}

// --------------------------------------------------------------------------
// Member function read_info_from_file parses the ascii text file
// generated by member function save_info_to_file().  This method
// regenerates the Rectangles within the list based upon the ascii
// text file information.

void RectanglesGroup::read_info_from_file()
{
   string input_filename=get_input_filename("rectangles");
//   string subdir="./bbox_data/";
//   string input_filename=subdir+"interp_bbox_corners.txt";
   filefunc::ReadInfile(input_filename);

   double X[3];
   vector<double> curr_time;
   vector<int> Rectangle_ID,pass_number;
   vector<twovector> bbox_corner[4];
   for (unsigned int i=0; i<filefunc::text_line.size(); i += 5)
   {
      stringfunc::string_to_n_numbers(3,filefunc::text_line[i],X);
      curr_time.push_back(X[0]);
      Rectangle_ID.push_back(int(X[1]));
      pass_number.push_back(int(X[2]));

      stringfunc::string_to_n_numbers(3,filefunc::text_line[i+1],X);
      bbox_corner[0].push_back(twovector(X[0],X[1]));
      stringfunc::string_to_n_numbers(3,filefunc::text_line[i+2],X);
      bbox_corner[1].push_back(twovector(X[0],X[1]));
      stringfunc::string_to_n_numbers(3,filefunc::text_line[i+3],X);
      bbox_corner[2].push_back(twovector(X[0],X[1]));
      stringfunc::string_to_n_numbers(3,filefunc::text_line[i+4],X);
      bbox_corner[3].push_back(twovector(X[0],X[1]));
   } // loop over index i labeling ascii file line number
   
//   for (unsigned int i=0; i<curr_time.size(); i++)
//   {
//      cout << "time = " << curr_time[i]
//           << " ID = " << Rectangle_ID[i]
//           << " pass = " << pass_number[i] << endl;
//      cout << " corner0 = " << (bbox_corner[0])[i] << endl;
//      cout << " corner1 = " << (bbox_corner[1])[i] << endl;
//      cout << " corner2 = " << (bbox_corner[2])[i] << endl;
//      cout << " corner3 = " << (bbox_corner[3])[i] << endl;
//   }

// Destroy all existing Rectangles before creating a new Rectangle list
// from the input ascii file:

   destroy_all_Graphicals();

   for (unsigned int i=0; i<Rectangle_ID.size(); i++)
   {
      int curr_ID=Rectangle_ID[i];
      Rectangle* curr_Rectangle_ptr=get_ID_labeled_Rectangle_ptr(curr_ID);
      if (curr_Rectangle_ptr == NULL)
      {
         curr_Rectangle_ptr=generate_new_Rectangle(curr_ID);
         osg::Geode* geode_ptr=curr_Rectangle_ptr->generate_drawable_geode();
         curr_Rectangle_ptr->get_PAT_ptr()->addChild(geode_ptr);
         insert_graphical_PAT_into_OSGsubPAT(curr_Rectangle_ptr,0);
      } // curr_Rectangle_ptr==NULL conditional
   } // loop over index i labeling entries in Rectangle_ID STL vector

   for (unsigned int r=0; r<get_n_Graphicals(); r++)
   {
      Rectangle* Rectangle_ptr=get_Rectangle_ptr(r);
      cout << "Loading info for Rectangle=" << r << endl;

// Initialize Rectangle's coords_erased flag to true for all images
// within the current pass:
      
      for (unsigned int n=get_first_framenumber(); n<=get_last_framenumber(); 
           n++)
      {
         double curr_t=static_cast<double>(n);
         Rectangle_ptr->set_mask(curr_t,get_passnumber(),true);
      }

// Load time, imagenumber, passnumber information into current
// Rectangle.  Compute midpoint position, quaternionic rotation about
// -y axis and scale factor needed to be applied to a canonical
// Rectangle in order to match vertices of each bbox.  Set
// manually_manipulated flag to true and coords_erased flag to false
// for each STL vector entry:

      for (unsigned int i=0; i<curr_time.size(); i++)
      {
         if (Rectangle_ID[i]==Rectangle_ptr->get_ID())
         {
            Rectangle_ptr->set_canonical_local_vertices();

            twovector bbox_center=0.25*(
               bbox_corner[0][i]+bbox_corner[1][i]+bbox_corner[2][i]+
               bbox_corner[3][i]);
            threevector bbox_midpnt(
               bbox_center.get(0),bbox_center.get(1),0);
            twovector e_hat=(bbox_corner[1][i]-bbox_corner[0][i]).
               unitvector();
            double theta=atan2(e_hat.get(1),e_hat.get(0));
            osg::Quat q;
            q.makeRotate(-theta,0,1,0);

            double length=(bbox_corner[1][i]-bbox_corner[0][i]).magnitude();
            double width=(bbox_corner[2][i]-bbox_corner[1][i]).magnitude();
            double l_ratio=length/Rectangle_ptr->get_canonical_length();
            double w_ratio=width/Rectangle_ptr->get_canonical_width();
            threevector scale(l_ratio,w_ratio,1);

            Rectangle_ptr->set_UVW_coords(
               curr_time[i],pass_number[i],bbox_midpnt);
            Rectangle_ptr->set_quaternion(
               curr_time[i],pass_number[i],q);
            Rectangle_ptr->set_scale(
               curr_time[i],pass_number[i],scale);

            Rectangle_ptr->set_coords_manually_manipulated(
               curr_time[i],pass_number[i]);
            Rectangle_ptr->set_mask(curr_time[i],pass_number[i],false);

// We would like to be able to unerase a Rectangle in an image before
// the first one in which UVW coordinates were saved into the ascii
// file.  So we copy the coordinate values backwards in time from the
// image where the Rectangle first appears:

            int n=basic_math::round(curr_time[i]);
            for (int m=0; m<n; m++)
            {
               threevector p;
               if (!Rectangle_ptr->get_UVW_coords(m,pass_number[i],p))
               {
                  Rectangle_ptr->set_UVW_coords(m,pass_number[i],bbox_midpnt);
               }
            } // loop over index m labeling images before image number n
          
         } // Rectangle_ID[i] == Rectangle_ptr->get_ID conditional
      } // loop over index i labeling entries in STL time, Rectangle_ID
      // and UVW vectors
   } // loop index r labeling over Rectangles in Rectanglelist

   update_display();
   reset_colors();
}

// --------------------------------------------------------------------------
// Member function update_display()

void RectanglesGroup::update_display()
{
//   cout << "inside RectanglesGroup::update_display()" << endl;
//   cout << "get_n_Graphicals() = " << get_n_Graphicals() << endl;
//   cout << "get_curr_t() = " << get_curr_t() << endl;

//   parse_latest_messages();
//   reset_colors();
   GraphicalsGroup::update_display();
}
