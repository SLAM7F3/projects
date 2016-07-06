// =========================================================================
// LINESEGMENTSGROUP class member function definitions
// =========================================================================
// Last modified on 10/8/09; 10/21/09; 10/22/09; 4/5/14
// =========================================================================

#include <iomanip>
#include <vector>
#include <osg/Geode>
#include "osg/osgGraphicals/AnimationController.h"
#include "math/basic_math.h"
#include "math/constant_vectors.h"
#include "datastructures/containerfuncs.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "osg/osgGeometry/LineSegment.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "osg/ViewFrustum.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::pair;
using std::setw;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void LineSegmentsGroup::allocate_member_objects()
{
}		       

void LineSegmentsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="LineSegmentsGroup";
   size[3]=1.0;
   ViewFrustum_ptr=NULL;
   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<LineSegmentsGroup>(
         this, &LineSegmentsGroup::update_display));
}		       

LineSegmentsGroup::LineSegmentsGroup(
   const int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr):
   GraphicalsGroup(p_ndims,PI_ptr,AC_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

LineSegmentsGroup::LineSegmentsGroup(
   const int p_ndims,Pass* PI_ptr,osgGA::Custom3DManipulator* CM_ptr,
   ViewFrustum* VF_ptr):
   GraphicalsGroup(p_ndims,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   CM_3D_refptr=CM_ptr;
   ViewFrustum_ptr=VF_ptr;
}		       

LineSegmentsGroup::~LineSegmentsGroup()
{
//   cout << "inside LineSegmentsGroup destructor" << endl;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const LineSegmentsGroup& f)
{
   for (unsigned int n=0; n<f.get_n_Graphicals(); n++)
   {
      LineSegment* LineSegment_ptr=f.get_LineSegment_ptr(n);
      outstream << "LineSegment node # " << n << endl;
      outstream << "LineSegment = " << *LineSegment_ptr << endl;
   }
   return(outstream);
}

// =========================================================================
// LineSegment creation and manipulation methods
// =========================================================================

// Member function generate_new_segment creates a new linesegment
// using input threevectors V1 and V2.

LineSegment* LineSegmentsGroup::generate_new_segment(
   const threevector& V1,const threevector& V2,
   bool draw_arrow_flag,int ID)
{
//   cout << "inside LSG::generate_new_segment, V1 = " << V1
//        << " V2 = " << V2 << endl;

   LineSegment* curr_LineSegment_ptr=
      generate_new_canonical_LineSegment(ID,draw_arrow_flag);
   curr_LineSegment_ptr->set_scale_attitude_posn(
      get_curr_t(),get_passnumber(),V1,V2);
   return curr_LineSegment_ptr;
}		       

// ----------------------------------------------------------------
// Member function generate_canonical_segments dynamically
// instantiates some input number of canonically oriented line
// segments which can be later used to form 2D polygons or 3D
// polyhedra.

void LineSegmentsGroup::generate_canonical_segments(
   unsigned int n_segments)
{
//    cout << "inside LSG::generate_canonical_segments()" << endl;
   for (unsigned int n=0; n<n_segments; n++)
   {
      generate_new_canonical_LineSegment(-1,false);
   }
}

// ----------------------------------------------------------------
// Member function generate_new_canonical_LineSegment returns a
// dynamically instantiated linesegment running from (0,0,0) to
// (1,0,0).  As of 3/10/06, all OSG linesegments are first created in
// this canonical form and subsequently scaled, rotated and translated
// via a call to LineSegment::set_scale_attitude_posn().

LineSegment* LineSegmentsGroup::generate_new_canonical_LineSegment(
   int ID,bool draw_arrow_flag)
{
//    cout << "inside LineSegmentsGroup::generate_new_canonical_LineSegment()" << endl;
   if (ID==-1) ID=get_next_unused_ID();
   LineSegment* curr_LineSegment_ptr=new LineSegment(
      get_ndims(),Zero_vector,x_hat,ID,draw_arrow_flag,
      AnimationController_ptr);
   initialize_new_LineSegment(curr_LineSegment_ptr);
   return curr_LineSegment_ptr;
}

LineSegment* LineSegmentsGroup::generate_new_canonical_LineSegment(
   int ID,bool draw_endpoint1_flag,bool draw_endpoint2_flag,
   double endpoint_size_prefactor)
{
   if (ID==-1) ID=get_next_unused_ID();
   LineSegment* curr_LineSegment_ptr=new LineSegment(
      get_ndims(),Zero_vector,x_hat,ID,draw_endpoint1_flag,
      draw_endpoint2_flag,endpoint_size_prefactor,AnimationController_ptr);
   initialize_new_LineSegment(curr_LineSegment_ptr);
   return curr_LineSegment_ptr;
}

/*
void LineSegmentsGroup::setup_canonical_LineSegment(
   LineSegment* curr_LineSegment_ptr)
{
//   cout << "inside LSG::setup_canonical_LineSegment()" << endl;
   GraphicalsGroup::insert_Graphical_into_list(curr_LineSegment_ptr);

   osg::Geode* geode_ptr=curr_LineSegment_ptr->generate_drawable_geode();
   curr_LineSegment_ptr->get_PAT_ptr()->addChild(geode_ptr);
   get_OSGgroup_ptr()->addChild(curr_LineSegment_ptr->get_PAT_ptr());

   reset_colors();
}
*/

// ---------------------------------------------------------------------
void LineSegmentsGroup::initialize_new_LineSegment(
   LineSegment* LineSegment_ptr,int OSGsubPAT_number)
{
//   cout << "inside LineSegmentsGroup::initialize_new_LineSegment" << endl;

   GraphicalsGroup::insert_Graphical_into_list(LineSegment_ptr);

// On 10/22/09, we empirically found that uncommenting out the next
// line ends up masking ESB video OBSFRUSTUM in NYC demo...

//   initialize_Graphical(LineSegment_ptr);

// Recall that as of Sep 2009, we store relative vertex information
// with respect to an average reference_origin point to avoid floating
// point problems.  So we need to translate the Arrow by its reference
// origin in order to globally position it:

//   LineSegment_ptr->set_UVW_coords(
//      get_curr_t(),get_passnumber(),
//      LineSegment_ptr->get_reference_origin());

   osg::Geode* geode_ptr=LineSegment_ptr->generate_drawable_geode();
   LineSegment_ptr->get_PAT_ptr()->addChild(geode_ptr);
   insert_graphical_PAT_into_OSGsubPAT(LineSegment_ptr,OSGsubPAT_number);
}

// -------------------------------------------------------------------------
// Member function change_size multiplies the size parameter for
// LineSegment objects corresponding to the current dimension by input
// parameter factor.

void LineSegmentsGroup::change_size(double factor)
{   
   size[get_ndims()] *= factor;
   GraphicalsGroup::change_size(factor);
//    for (unsigned int n=0; n<get_n_Graphicals(); n++)
//    {
//      LineSegment* LineSegment_ptr=get_LineSegment_ptr(n);
//    }
}

// -------------------------------------------------------------------------
// Member function set_width should be called AFTER all linesegment
// objects within the current LineSegmentsGroup have been instantiated.

void LineSegmentsGroup::set_width(double width)
{   
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      get_LineSegment_ptr(n)->get_LineWidth_ptr()->setWidth(width);
   }
}

// -------------------------------------------------------------------------
// Member function reset_colors loops over all entries within the
// current video image's LineSegmentlist.  It colors blue the segment
// whose ID equals selected_Graphical_ID.  All other LineSegments
// are colored according to their permanent colors.

void LineSegmentsGroup::reset_colors()
{   
//   cout << "inside LineSegmentsGroup::reset_colors()" << endl;
   
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      LineSegment* LineSegment_ptr=get_LineSegment_ptr(n);
//      cout << "LineSegment_ptr = " << LineSegment_ptr << endl;
      if (get_selected_Graphical_ID()==LineSegment_ptr->get_ID())
      {
         LineSegment_ptr->set_curr_color(colorfunc::blue);
      }
      else
      {
//         cout << "permanent color = " << endl;
//         osgfunc::print_Vec4(
//            LineSegment_ptr->get_permanent_color());
         
         LineSegment_ptr->set_curr_color(
            LineSegment_ptr->get_permanent_color());
      }
//      cout << "n = " << n << " curr_color = " 
//           << LineSegment_ptr->get_curr_color().r() << "," 
//           << LineSegment_ptr->get_curr_color().g() << "," 
//           << LineSegment_ptr->get_curr_color().b() 
//           << " perm color = " 
//           << LineSegment_ptr->get_permanent_color().r() << ","
//           << LineSegment_ptr->get_permanent_color().g() << ","
//           << LineSegment_ptr->get_permanent_color().b() 
//           << endl;
      
   } // loop over LineSegments in LineSegmentlist
}

// -------------------------------------------------------------------------
// Member function update_display()

void LineSegmentsGroup::update_display()
{
//   cout << "inside LineSegmentsGroup::update_display()" << endl;
//   cout << "get_curr_t() = " << get_curr_t() << endl;

//   cout << "this = " << this << endl;

//   for (unsigned int n=0; n<get_n_Graphicals(); n++)
//   {
//      LineSegment* LineSegment_ptr=get_LineSegment_ptr(n);
//      threevector V1(LineSegment_ptr->get_V1());
//      threevector V2(LineSegment_ptr->get_V2());
//      cout << "n = " << n << " V1 = " << V1 << " V2 = " << V2 << endl;
//      cout << "stationary_flag = " 
//           << LineSegment_ptr->get_stationary_Graphical_flag() << endl;
//   }

   GraphicalsGroup::update_display();
}

// =========================================================================
// LineSegment destruction member functions
// =========================================================================

// Member function destroy_all_LineSegments() first fills an STL vector
// with LineSegment pointers.  It then iterates over each vector entry
// and calls destroy_LineSegment for each LineSegment pointer.  On 5/3/08,
// we learned the hard and painful way that this two-step process is
// necessary in order to correctly purge all LineSegments.

void LineSegmentsGroup::destroy_all_LineSegments()
{   
//   cout << "inside LineSegmentsGroup::destroy_all_LineSegments()" << endl;
   unsigned int n_LineSegments=get_n_Graphicals();
//   cout << "n_LineSegments = " << n_LineSegments << endl;

   vector<LineSegment*> LineSegments_to_destroy;
   for (unsigned int p=0; p<n_LineSegments; p++)
   {
      LineSegment* LineSegment_ptr=get_LineSegment_ptr(p);
//      cout << "p = " << p << " LineSegment_ptr = " << LineSegment_ptr << endl;
      LineSegments_to_destroy.push_back(LineSegment_ptr);
   }

   for (unsigned int p=0; p<n_LineSegments; p++)
   {
      destroy_LineSegment(LineSegments_to_destroy[p]);
   }
}

// -------------------------------------------------------------------------
bool LineSegmentsGroup::destroy_LineSegment(int ID)
{   
//   cout << "inside LineSegmentsGroup::destroy_LineSegment(int ID)" << endl;
//   cout << "int ID = " << ID << endl;
   if (ID >= 0)
   {
      return destroy_LineSegment(get_ID_labeled_LineSegment_ptr(ID));
   }
   else
   {
      return false;
   }
}

// ---------------------------------------------------------------------
bool LineSegmentsGroup::destroy_LineSegment(LineSegment* curr_LineSegment_ptr)
{
//   cout << "inside LineSegmentsGroup::destroy_LineSegment(curr_LineSegment_ptr)" 
//        << endl;
//   cout << "curr_LineSegment_ptr = " << curr_LineSegment_ptr << endl;
//   cout << "LineSegmentsGroup this = " << this << endl;

   if (curr_LineSegment_ptr==NULL) return false;

//   cout << "Before call to destroy_Graphical, get_n_Graphicals() = "
//        << get_n_Graphicals() << endl;
   bool flag=destroy_Graphical(curr_LineSegment_ptr);
//   cout << "After call to destroy_Graphical, get_n_Graphicals() = "
//        << get_n_Graphicals() << endl;

//   for (unsigned int n=0; n<get_n_Graphicals(); n++)
//   {
//      Geometrical* Geometrical_ptr=get_Geometrical_ptr(n);
//      cout << "n = " << n 
//           << " Geometrical_ptr = " << Geometrical_ptr 
//           << " Geometrical_ptr->get_ID() = " << Geometrical_ptr->get_ID()
//           << endl;
//   }
   
   return flag;
}

// =========================================================================
// Ascii feature file I/O methods
// =========================================================================

void LineSegmentsGroup::save_info_to_file()
{
   outputfunc::write_banner("Saving line segment information to ascii file:");
   string output_filename="line_segments.txt";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   for (unsigned int imagenumber=get_first_framenumber(); 
        imagenumber <= get_last_framenumber(); imagenumber++)
   {
      double curr_t=static_cast<double>(imagenumber);
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         LineSegment* LineSegment_ptr=get_LineSegment_ptr(n);
         threevector V1(LineSegment_ptr->get_V1());
         threevector V2(LineSegment_ptr->get_V2());
         threevector UVW;
         LineSegment_ptr->get_UVW_coords(curr_t,get_passnumber(),UVW);
//         cout << "UVW = " << UVW << endl;
         V1 += UVW;
         V2 += UVW;

         const int column_width=9;
         outstream.setf(ios::showpoint);
         outstream << setw(3) << curr_t 
                   << setw(4) << LineSegment_ptr->get_ID() 
                   << setw(3) << get_passnumber();
         outstream << setw(column_width) 
                   << stringfunc::number_to_string(V1.get(0),5)
                   << setw(column_width) 
                   << stringfunc::number_to_string(V1.get(1),5)
                   << setw(column_width) 
                   << stringfunc::number_to_string(V1.get(2),5);
         outstream << setw(column_width) 
                   << stringfunc::number_to_string(V2.get(0),5)
                   << setw(column_width) 
                   << stringfunc::number_to_string(V2.get(1),5)
                   << setw(column_width) 
                   << stringfunc::number_to_string(V2.get(2),5);
         outstream << setw(column_width) 
                   << colorfunc::get_colorfunc_color(
                      LineSegment_ptr->get_permanent_color()) << endl;
      } // loop over index n labeling LineSegment
   } // loop over image numbers

   filefunc::closefile(output_filename,outstream);
}

// ---------------------------------------------------------------------
// Member function read_info_from_file parses the ascii text file
// generated by member function save_info_to_file().  After purging
// the LineSegmentslist, this method regenerates the LineSegments
// within the list based upon the ascii text file information.  This
// boolean member function returns false if it cannot successfully
// parse the input ascii file.

bool LineSegmentsGroup::read_info_from_file(
   string segments_filename,vector<double>& curr_time,
   vector<int>& segment_ID,vector<int>& pass_number,
   vector<threevector>& V1,vector<threevector>& V2,
   vector<colorfunc::Color>& color)
{
   if (!filefunc::ReadInfile(segments_filename))
   {
      cout << "Trouble in LineSegmentsGroup::read_info_from_file()"
           << endl;
      cout << "Couldn't open segments_filename = " << segments_filename
           << endl;
      return false;
   }

   unsigned int nlines=filefunc::text_line.size();
   curr_time.reserve(nlines);
   pass_number.reserve(nlines);
   V1.reserve(nlines);
   V2.reserve(nlines);
   color.reserve(nlines);

   const int n_fields=10;
   double X[n_fields];
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << "i = " << i
//           << " text_line = " << filefunc::text_line[i] << endl;
      stringfunc::string_to_n_numbers(n_fields,filefunc::text_line[i],X);
      curr_time.push_back(X[0]);
      segment_ID.push_back(basic_math::round(X[1]));
      pass_number.push_back(basic_math::round(X[2]));
      double V1_x=X[3];
      double V1_y=X[4];
      double V1_z=X[5];
      V1.push_back(threevector(V1_x,V1_y,V1_z));
      double V2_x=X[6];
      double V2_y=X[7];
      double V2_z=X[8];
      V2.push_back(threevector(V2_x,V2_y,V2_z));
      color.push_back(static_cast<colorfunc::Color>(int(X[9])));
   } // loop over index i labeling ascii file line number
   
//   for (unsigned int i=0; i<curr_time.size(); i++)
//   {
//      threevector delta_V=V2[i]-V1[i];
//      cout << "time = " << curr_time[i]
//           << " ID = " << segment_ID[i]
//           << " pass = " << pass_number[i]
//           << " V1 = " << V1[i] 
//           << " V2 = " << V2[i] 
//           << " dV = " << delta_V.magnitude()
//           << " color = " << color[i] 
//           << endl;
//   }
   return true;
}

// This overloaded version of read_info_from_file takes in continuous
// RGB color information rather than discrete color labels:

bool LineSegmentsGroup::read_info_from_file(
   string segments_filename,vector<double>& curr_time,
   vector<int>& segment_ID,vector<int>& pass_number,
   vector<threevector>& V1,vector<threevector>& V2,
   vector<osg::Vec4>& color)
{
   if (!filefunc::ReadInfile(segments_filename))
   {
      cout << "Trouble in LineSegmentsGroup::read_info_from_file()"
           << endl;
      cout << "Couldn't open segments_filename = " << segments_filename
           << endl;
      return false;
   }

   unsigned int nlines=filefunc::text_line.size();
   curr_time.reserve(nlines);
   pass_number.reserve(nlines);
   V1.reserve(nlines);
   V2.reserve(nlines);
   color.reserve(nlines);

//   const int n_fields=12;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << "i = " << i
//           << " text_line = " << filefunc::text_line[i] << endl;
      vector<double> X=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      curr_time.push_back(X[0]);
      segment_ID.push_back(basic_math::round(X[1]));
      pass_number.push_back(basic_math::round(X[2]));
      V1.push_back(threevector(X[3],X[4],X[5]));
      V2.push_back(threevector(X[6],X[7],X[8]));
      color.push_back(osg::Vec4(X[9],X[10],X[11],1));

   } // loop over index i labeling ascii file line number
   
//   for (unsigned int i=0; i<curr_time.size(); i++)
//   {
//      threevector delta_V=V2[i]-V1[i];
//      cout << "time = " << curr_time[i]
//           << " ID = " << segment_ID[i]
//           << " pass = " << pass_number[i]
//           << " V1 = " << V1[i] 
//           << " V2 = " << V2[i] 
//           << " dV = " << delta_V.magnitude()
//           << " color = " << color[i] 
//           << endl;
//   }
   return true;
}

// -------------------------------------------------------------------------
// Member function reconstruct_lines_from_file_info parses the line
// segment information written to an ascii file by member function
// save_info_to_file().  It then destroys all existing line segments
// and instantiates a new set using the input information.  This
// boolean method returns false it it cannot successfully reconstruct
// line segments from the input file information.

bool LineSegmentsGroup::reconstruct_lines_from_file_info(
   string lines_filename,bool RGB_flag)
{
//   cout << "Enter name of file containing line segment information:" << endl;
//   cin >> lines_filename;

   vector<double> curr_time;
   vector<int> segment_ID,pass_number;
   vector<threevector> V1,V2;
   vector<colorfunc::Color> color_index;
   vector<osg::Vec4> color;

   bool file_parsed=false;
   if (RGB_flag)
   {
      file_parsed=read_info_from_file(
         lines_filename,curr_time,segment_ID,pass_number,V1,V2,color);
   }
   else
   {
      file_parsed=read_info_from_file(
         lines_filename,curr_time,segment_ID,pass_number,V1,V2,color_index);
   }
   
   if (file_parsed)
   {

// Destroy all existing LineSegments before creating a new LineSegment list
// from the input ascii file:

      destroy_all_Graphicals();

      vector<int> sorted_segment_ID;

      for (unsigned int i=0; i<segment_ID.size(); i++)
      {
         sorted_segment_ID.push_back(segment_ID[i]);
      }
      std::sort(sorted_segment_ID.begin(),sorted_segment_ID.end());

      int prev_segment_ID=-1;
      for (unsigned int i=0; i<segment_ID.size(); i++)
      {
         cout << i << " " << flush;
         if (segment_ID[i] != prev_segment_ID)
         {
            if (RGB_flag)
            {
               regenerate_segment(
                  segment_ID[i],curr_time,segment_ID,V1,V2,color);
            }
            else
            {
               regenerate_segment(
                  segment_ID[i],curr_time,segment_ID,V1,V2,color_index);
            }
            prev_segment_ID=segment_ID[i];
         }
      } // loop over index i labeling entries in segment_ID STL vector
      cout << endl;
      return true;
   } // boolean flag for reading input ascii file
   return false;
}

// -------------------------------------------------------------------------
// Member function regenerate_segment takes in linesegment vertex
// information which was presumably previously generated as a function
// of time and pass number.  This method instantiates a single unit
// linesegment along the +Xhat direction.  It then rescales, rotates
// and translates the canonical segment so it reproduces the input
// vertex information over time.

void LineSegmentsGroup::regenerate_segment(
   int curr_segment_ID,
   const vector<double>& curr_time,const vector<int>& segment_ID,
   const vector<threevector>& V1,const vector<threevector>& V2,
   const vector<colorfunc::Color>& color_index)
{
   cout << "inside LineSegmentsGroup::regenerate_segment()" << endl;
   LineSegment* LineSegment_ptr=generate_new_canonical_LineSegment(
      curr_segment_ID,false);

// Loop over all entries in STL vectors V1 and V2 corresponding to the
// nth linesegment.  Store the scaling, rotational and translational
// transformation which needs to be performed to map canonical
// linesegment ranging from (0,0,0) to (1,0,0) into the segment
// ranging from V1 to V2 within *curr_LineSegment_ptr instantaneous
// observation objects:

   for (unsigned int l=0; l<curr_time.size(); l++)
   {
      if (curr_segment_ID==segment_ID[l])
      {
         LineSegment_ptr->set_stationary_Graphical_flag(false);
         LineSegment_ptr->set_AnimationController_ptr(
            AnimationController_ptr);
         LineSegment_ptr->set_scale_attitude_posn(
            curr_time[l],get_passnumber(),V1[l],V2[l]);
         LineSegment_ptr->set_permanent_color(color_index[l]);
         LineSegment_ptr->set_coords_manually_manipulated(
            curr_time[l],get_passnumber());
//         cout << "l = " << l << " curr_time = " << curr_time[l]
//              << " color index = " << color_index[l] << endl;
//         cout << " V1 = " << V1[l].get(0) << "," << V1[l].get(1)
//              << "," << V1[l].get(2) << endl;
//         cout << " V2 = " << V2[l].get(0) << "," << V2[l].get(1)
//              << "," << V2[l].get(2) << endl;
      }
   } // loop over index l labeling lines within input text file
   reset_colors();
}

void LineSegmentsGroup::regenerate_segment(
   int curr_segment_ID,
   const vector<double>& curr_time,const vector<int>& segment_ID,
   const vector<threevector>& V1,const vector<threevector>& V2,
   const vector<osg::Vec4>& color)
{
   cout << "inside LineSegmentsGroup::regenerate_segment()" << endl;
   LineSegment* LineSegment_ptr=generate_new_canonical_LineSegment(
      curr_segment_ID,false);

// Loop over all entries in STL vectors V1 and V2 corresponding to the
// nth linesegment.  Store the scaling, rotational and translational
// transformation which needs to be performed to map canonical
// linesegment ranging from (0,0,0) to (1,0,0) into the segment
// ranging from V1 to V2 within *curr_LineSegment_ptr instantaneous
// observation objects:

   for (unsigned int l=0; l<curr_time.size(); l++)
   {
      if (curr_segment_ID==segment_ID[l])
      {
         LineSegment_ptr->set_stationary_Graphical_flag(false);
         LineSegment_ptr->set_AnimationController_ptr(
            AnimationController_ptr);
         LineSegment_ptr->set_scale_attitude_posn(
            curr_time[l],get_passnumber(),V1[l],V2[l]);
         LineSegment_ptr->set_permanent_color(color[l]);
         LineSegment_ptr->set_coords_manually_manipulated(
            curr_time[l],get_passnumber());
//         cout << "l = " << l << " curr_time = " << curr_time[l]
//              << " color = " << color[l] << endl;
//         cout << " V1 = " << V1[l].get(0) << "," << V1[l].get(1)
//              << "," << V1[l].get(2) << endl;
//         cout << " V2 = " << V2[l].get(0) << "," << V2[l].get(1)
//              << "," << V2[l].get(2) << endl;
      }
   } // loop over index l labeling lines within input text file
   reset_colors();
}

// -------------------------------------------------------------------------
// Member function generate_FOV_segments is a specialized method which
// takes in time-dependent UV coordinates for an airborne video
// camera's quadrilateral field-of-view.  It also takes in the
// aircraft's position as a function of time.  This method
// instantiates 8 linesegments.  4 of them define the FOV
// quadrilateral within the ground z-plane.  The other 4 segments
// emanate outwards from the aircraft's world-space postion down
// towards the ground z-plane.  These latter lines form a pyramid that
// illustrates the field-of-view of the camera mounted on the bottom
// of the airplane.

void LineSegmentsGroup::generate_FOV_segments(
   const vector<double>& curr_time,const vector<int>& pass_number,
   const vector<threevector>& V1,const vector<threevector>& V2,
   const vector<colorfunc::Color>& color,
   const vector<threevector>& plane_posn)
{
   cout << "inside LSG::generate_FOV_segments()" << endl;

// Destroy all existing LineSegments before creating a new LineSegment list
// from the input ascii file:

   destroy_all_Graphicals();

   const unsigned int n_segments=4;
   for (unsigned int n=0; n<n_segments; n++)
   {
      for (unsigned int iter=0; iter<2; iter++)
      {
         LineSegment* curr_LineSegment_ptr=
            generate_new_canonical_LineSegment(-1,false);

// Loop over all entries in STL vectors V1 and V2 corresponding to the
// nth linesegment.  Store the scaling, rotational and translational
// transformation which needs to be performed to map canonical
// linesegment ranging from (0,0,0) to (1,0,0) into the segment
// ranging from V1 to V2 within *curr_LineSegment_ptr instantaneous
// observation objects:

         for (unsigned int j=0; j<curr_time.size()/n_segments; j++)
         {
            int i=n_segments*j+n;
            if (iter==0)
            {
               curr_LineSegment_ptr->set_scale_attitude_posn(
                  curr_time[i],pass_number[i],V1[i],V2[i]);
            }
            else if (iter==1)
            {
               curr_LineSegment_ptr->set_scale_attitude_posn(
                  curr_time[i],pass_number[i],V1[i],plane_posn[j]);
            }
         } // loop over index j labeling times

         curr_LineSegment_ptr->set_permanent_color(color[n]);
         curr_LineSegment_ptr->set_coords_manually_manipulated(
            curr_time[n],pass_number[n]);
         curr_LineSegment_ptr->set_mask(
            curr_time[n],pass_number[n],false);

      } // loop over iter index labeling FOV square or FOV pyramid segment
   } // loop over index n labeling FOV corner
   update_display();
   reset_colors();
}

// -------------------------------------------------------------------------
void LineSegmentsGroup::generate_FOV_segments(
   double rho,const threevector& camera_XYZ,
   const vector<threevector>& UV_corner_dir)
{
   cout << "inside LSG::generate_FOV_segments(), rho = " << rho << endl;

   const unsigned int n_segments=4;
   const osg::Vec4 FOV_linecolor=colorfunc::get_OSG_color(colorfunc::white);

   for (unsigned int n=0; n<n_segments; n++)
   {
      threevector V1(camera_XYZ);
      threevector V2(V1+rho*UV_corner_dir[n]);

      int curr_ID=n;
      LineSegment* LineSegment_ptr=generate_new_canonical_LineSegment(
         curr_ID,false);

// Loop over all entries in STL vectors V1 and V2 corresponding to the
// nth linesegment.  Store the scaling, rotational and translational
// transformation which needs to be performed to map canonical
// linesegment ranging from (0,0,0) to (1,0,0) into the segment
// ranging from V1 to V2 within *curr_LineSegment_ptr instantaneous
// observation objects:

      LineSegment_ptr->set_scale_attitude_posn(
         get_curr_t(),get_passnumber(),V1,V2);
      LineSegment_ptr->set_permanent_color(FOV_linecolor);
      LineSegment_ptr->set_coords_manually_manipulated(
         get_curr_t(),get_passnumber());

   } // loop over index n labeling FOV line segments
   reset_colors();
}

// =========================================================================
// View frustum display methods
// =========================================================================

void LineSegmentsGroup::draw_FOV_frustum()
{
   if (ViewFrustum_ptr==NULL)
   {
      cout << "Error in LineSegmentsGroup::draw_FOV_frustum()" << endl;
      cout << "ViewFrustum_ptr = NULL!" << endl;
      return;
   }
   
   threevector tip(CM_3D_refptr->get_eye_world_posn());
   const double radius=tip.magnitude();
   
//    LineSegment* l0_ptr=
      generate_new_segment(tip,tip+radius*ViewFrustum_ptr->get_ray().at(0));
//    LineSegment* l1_ptr=
      generate_new_segment(tip,tip+radius*ViewFrustum_ptr->get_ray().at(1));
//    LineSegment* l2_ptr=
      generate_new_segment(tip,tip+radius*ViewFrustum_ptr->get_ray().at(2));
//   LineSegment* l3_ptr=
      generate_new_segment(tip,tip+radius*ViewFrustum_ptr->get_ray().at(3));

/*
//   ViewFrustum_ptr->get_vertex().at(4));
   LineSegment* l1_ptr=generate_new_segment(
      tip,ViewFrustum_ptr->get_vertex().at(5));
   LineSegment* l2_ptr=generate_new_segment(
      tip,ViewFrustum_ptr->get_vertex().at(6));
   LineSegment* l3_ptr=generate_new_segment(
      tip,ViewFrustum_ptr->get_vertex().at(7));
*/

//   LineSegment* l01_ptr=
      generate_new_segment(
         ViewFrustum_ptr->get_vertex().at(4),
         ViewFrustum_ptr->get_vertex().at(5));
//   LineSegment* l12_ptr=
      generate_new_segment(
         ViewFrustum_ptr->get_vertex().at(5),
         ViewFrustum_ptr->get_vertex().at(7));
//   LineSegment* l23_ptr=
      generate_new_segment(
         ViewFrustum_ptr->get_vertex().at(7),
         ViewFrustum_ptr->get_vertex().at(6));
//   LineSegment* l30_ptr=
      generate_new_segment(
         ViewFrustum_ptr->get_vertex().at(6),
         ViewFrustum_ptr->get_vertex().at(4));

   set_width(3);
}
