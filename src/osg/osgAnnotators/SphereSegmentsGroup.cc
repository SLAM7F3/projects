// ==========================================================================
// SPHERESEGMENTSGROUP class member function definitions
// ==========================================================================
// Last modified on 8/24/11; 9/7/11; 12/2/11
// ==========================================================================

#include <iomanip>
#include <osg/Geode>
#include <osgText/Text>
#include <vector>
#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "general/inputfuncs.h"
#include "osg/osgAnnotators/SphereSegmentsGroup.h"
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

void SphereSegmentsGroup::allocate_member_objects()
{
}		       

void SphereSegmentsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="SphereSegmentsGroup";

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<SphereSegmentsGroup>(
         this, &SphereSegmentsGroup::update_display));
}		       

SphereSegmentsGroup::SphereSegmentsGroup(
   Pass* PI_ptr,threevector* GO_ptr,
   bool display_spokes_flag,bool include_blast_flag):
   GeometricalsGroup(3,PI_ptr,NULL,GO_ptr), AnnotatorsGroup(3,PI_ptr)
{	
   this->display_spokes_flag=display_spokes_flag;
   this->include_blast_flag=include_blast_flag;

   initialize_member_objects();
   allocate_member_objects();
}		       

SphereSegmentsGroup::SphereSegmentsGroup(
   Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,threevector* GO_ptr,
   bool display_spokes_flag,bool include_blast_flag):
   GeometricalsGroup(3,PI_ptr,clock_ptr,EM_ptr,GO_ptr),AnnotatorsGroup(3,PI_ptr)
{	
   this->display_spokes_flag=display_spokes_flag;
   this->include_blast_flag=include_blast_flag;

   initialize_member_objects();
   allocate_member_objects();
}		       

SphereSegmentsGroup::~SphereSegmentsGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const SphereSegmentsGroup& f)
{
   int node_counter=0;
   for (unsigned int n=0; n<f.get_n_Graphicals(); n++)
   {
      SphereSegment* SphereSegment_ptr=f.get_SphereSegment_ptr(n);
      outstream << "SphereSegment node # " << node_counter++ << endl;
      outstream << "SphereSegment = " << *SphereSegment_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// SphereSegment creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_SphereSegment from all other graphical insertion
// and manipulation methods...

SphereSegment* SphereSegmentsGroup::generate_new_canonical_SphereSegment(
   double radius,double az_spread,double el_spread,int ID)
{

// Orient canonical SphereSegment so that its symmtry axis lies in the
// +x_hat direction:

   const double az_central=PI/2;	
   const double el_central=0;
   
   double az_min=az_central-0.5*az_spread;
   double az_max=az_central+0.5*az_spread;
   double el_min=el_central-0.5*el_spread;
   double el_max=el_central+0.5*el_spread;
   
   return generate_new_SphereSegment(radius,az_min,az_max,el_min,el_max,ID);
}

// --------------------------------------------------------------------------
SphereSegment* SphereSegmentsGroup::generate_new_SphereSegment(
   double radius,double az_min,double az_max,double el_min,double el_max,
   int ID)
{
   return generate_new_SphereSegment(
      radius,Zero_vector,az_min,az_max,el_min,el_max,ID);
}

// --------------------------------------------------------------------------
SphereSegment* SphereSegmentsGroup::generate_new_hemisphere(
   double radius,threevector& posn,int ID)
{
   return generate_new_SphereSegment(radius,posn,0,2*PI,0,PI/2,ID);
}

// --------------------------------------------------------------------------
SphereSegment* SphereSegmentsGroup::generate_new_SphereSegment(
   double radius,threevector posn,double az_min,double az_max,
   double el_min,double el_max,int ID)
{
//   cout << "inside SphereSegmentsGroup::generate_new_SphereSegment()" << endl;
   
   if (ID==-1) ID=get_next_unused_ID();

   osg::Vec3 segment_posn(posn.get(0),posn.get(1),posn.get(2));
   SphereSegment* curr_SphereSegment_ptr=new SphereSegment(
      ID,radius,segment_posn,az_min,az_max,el_min,el_max);

   initialize_new_SphereSegment(curr_SphereSegment_ptr);

   return curr_SphereSegment_ptr;
}

// ---------------------------------------------------------------------
void SphereSegmentsGroup::initialize_new_SphereSegment(
   SphereSegment* curr_SphereSegment_ptr,int OSGsubPAT_number)
{
//   cout << "inside SphereSegmentsGroup::initialize_new_SphereSegment()" 
//        << endl;

   GraphicalsGroup::insert_Graphical_into_list(curr_SphereSegment_ptr);
   initialize_Graphical(curr_SphereSegment_ptr);

   osg::Group* group_ptr=curr_SphereSegment_ptr->generate_drawable_group(
      display_spokes_flag,include_blast_flag);
   curr_SphereSegment_ptr->get_PAT_ptr()->addChild(group_ptr);

// Orient spheresegment so that it points radially inward wrt earth's
// center if Earth's ellipsoid_model_ptr != NULL:

   rotate_zhat_to_rhat(curr_SphereSegment_ptr);

   insert_graphical_PAT_into_OSGsubPAT(
      curr_SphereSegment_ptr,OSGsubPAT_number);
   reset_colors();
}

// --------------------------------------------------------------------------
// Member function initialize_vertical_posn sets the input
// SphereSegment's base so that it touches the 2D Grid.  As of
// 8/14/05, we haven't figured out exactly how to set the z value for
// the SphereSegment so that it precisely touches the grid.  But this
// method at least puts the tip only slightly above the grid.

// This member function really belongs in the SphereSegment class
// rather than in SphereSegmentsGroup.  But since the former does not
// already have get_curr_t() nor get_world_origin() method,
// we are willing to keep this member function here...

void SphereSegmentsGroup::initialize_vertical_posn(
   SphereSegment* curr_SphereSegment_ptr)
{
   threevector SphereSegment_posn;
   curr_SphereSegment_ptr->get_UVW_coords(
      get_curr_t(),get_passnumber(),SphereSegment_posn);
   threevector grid_world_origin(get_grid_world_origin());
   SphereSegment_posn.put(2,grid_world_origin.get(2));
   curr_SphereSegment_ptr->set_UVW_coords(
      get_curr_t(),get_passnumber(),SphereSegment_posn);
}

/*
// --------------------------------------------------------------------------
// Member function edit_SphereSegment_label allows the user to change the ID
// number associated with a SphereSegment.  The new ID number must not
// conflict with any other existing SphereSegment's ID.  It must also be
// non-negative.  The user enters the replacement ID for a selected
// SphereSegment within the main console window.  (As of 7/10/05, we are
// unfortunately unable to robustly retrieve user input from the
// SphereSegment text dialog window...)

void SphereSegmentsGroup::edit_SphereSegment_label()
{   
   SphereSegment* curr_SphereSegment_ptr=get_ID_labeled_SphereSegment_ptr(
      get_selected_Graphical_ID());

   if (curr_SphereSegment_ptr != NULL)
   {
      cout << endl;
      if (get_selected_Graphical_ID() != -1)
      {
            string label_command="Enter text label for SphereSegment " 
            +stringfunc::number_to_string(get_selected_Graphical_ID())
            +":";
            string label=inputfunc::enter_string(label_command);
            curr_SphereSegment_ptr->set_label(label);
            osgText::Text* text_ptr = curr_SphereSegment_ptr->get_text_ptr();
            osg::Geode* geode_ptr = curr_SphereSegment_ptr->get_geode_ptr();
            osg::Geometry* behind_box_geom = curr_SphereSegment_ptr->get_box_ptr();

            geode_ptr->removeDrawable(behind_box_geom);
            osg::BoundingBox bb;
            bb.expandBy(text_ptr->getBound());

            float totalY = bb.yMax()-bb.yMin();
            float totalZ = bb.zMax()-bb.zMin();
            osg::Vec3 txtpos = text_ptr->getPosition();
            float current_ymax = txtpos.y()-totalY-3;
            float current_ymin = txtpos.y()+3;
            float current_zmax = txtpos.z()+totalZ+3;
            float current_zmin = txtpos.z()-5;

            behind_box_geom = new osg::Geometry();
            geode_ptr->addDrawable(behind_box_geom); 

            osg::Vec3Array* vertices = new osg::Vec3Array;
            vertices->push_back( osg::Vec3( 1, current_ymin, current_zmin) ); // front left 
            vertices->push_back( osg::Vec3(1, current_ymax, current_zmin) ); // front right 
            vertices->push_back( osg::Vec3(1,current_ymax, current_zmax) ); // back right 
            vertices->push_back( osg::Vec3( 1,current_ymin, current_zmax) ); // back left 
            behind_box_geom->setVertexArray( vertices );

            osg::DrawElementsUInt* square = 
                new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
            square->push_back(3);
            square->push_back(2);
            square->push_back(1);
            square->push_back(0);
            behind_box_geom->addPrimitiveSet(square);

            osg::Vec4Array* colors = new osg::Vec4Array;
            colors->push_back(osg::Vec4(0.45,0.45,0.45, 1.0f) );

            osg::TemplateIndexArray
                <unsigned int, osg::Array::UIntArrayType,4,4> *colorIndexArray;
            colorIndexArray = 
                new osg::TemplateIndexArray<unsigned int, 
                osg::Array::UIntArrayType,4,4>;
            colorIndexArray->push_back(0); // vertex 0 assigned color array element 0
            colorIndexArray->push_back(0); // vertex 1 assigned color array element 1
            colorIndexArray->push_back(0); // vertex 2 assigned color array element 2
            colorIndexArray->push_back(0); // vertex 3 assigned color array element 3

            behind_box_geom->setColorArray(colors);
            behind_box_geom->setColorIndices(colorIndexArray);
            behind_box_geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
      } // selected_Graphical_ID != -1 conditional
   } // currnode_SphereSegment != NULL conditional
}
*/

// --------------------------------------------------------------------------
// Member function change_size multiplies the size parameter for
// SphereSegment objects corresponding to the current dimension by input
// parameter factor.

void SphereSegmentsGroup::change_size(double factor)
{   
   GraphicalsGroup::change_size(factor);
//   for (unsigned int n=0; n<get_n_Graphicals(); n++)
//   {
//      SphereSegment* SphereSegment_ptr=get_SphereSegment_ptr(n);
//      osg::Vec3Array* coords=static_cast<osg::Vec3Array*>(
//         SphereSegment_ptr->get_geom_ptr()->getVertexArray());
//      SphereSegment_ptr->set_crosshairs_coords(coords);
//      osgText::Text* text_ptr=SphereSegment_ptr->get_text_ptr();
//      text_ptr->setCharacterSize(text_ptr->getCharacterHeight()*factor);
//      SphereSegment_ptr->set_crosshairsnumber_text_posn();
//   }
}

// --------------------------------------------------------------------------
// Member function reset_colors loops over all SphereSegments and
// colors blue the one whose ID equals selected_Graphical_ID.  All
// other SphereSegments are colored red.

/*
void SphereSegmentsGroup::reset_colors()
{   
   cout << "inside SphereSegmentsGroup::reset_colors()" << endl;
   colorfunc::Color SphereSegment_color;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      SphereSegment* SphereSegment_ptr=get_SphereSegment_ptr(n);
      SphereSegment_color=colorfunc::red;
      if (get_selected_Graphical_ID()==SphereSegment_ptr->get_ID())
      {
         SphereSegment_color=colorfunc::orange;
      }
      SphereSegment_ptr->set_curr_color(SphereSegment_color);
   } // loop over SphereSegments in SphereSegmentlist
}
*/

// ==========================================================================
// Ascii feature file I/O methods
// ==========================================================================

/*
// Member function save_info_to_file loops over all SphereSegments within
// *get_SphereSegmentlist_ptr() and prints their times, IDs, pass numbers
// and UVW coordinates to the output ofstream.  This SphereSegment
// information can later be read back in via member function
// read_SphereSegment_info_from_file.

void SphereSegmentsGroup::save_info_to_file()
{
   outputfunc::write_banner("Saving spheresegment information to ascii file:");
   string output_filename="spheresegments.txt";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   for (int imagenumber=0; imagenumber < get_Nimages(); imagenumber++)
   {
      double curr_t=static_cast<double>(imagenumber);
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         SphereSegment* SphereSegment_ptr=get_SphereSegment_ptr(n);
         instantaneous_obs* curr_obs_ptr=SphereSegment_ptr->
            get_particular_time_obs(curr_t,get_passnumber());
         if (curr_obs_ptr != NULL)
         {
            bool erased_point_flag=false;
            threevector p;
            vector<double> column_data;
            if (SphereSegment_ptr->get_UVW_coords(curr_t,get_passnumber(),p))
            {
               if (SphereSegment_ptr->get_mask(curr_t,get_passnumber()))
               {
                  erased_point_flag=true;
               }
               else
               {
                  for (int j=0; j<get_ndims(); j++)
                  {
                     column_data.push_back(p.get(j));
                  }
               }
            }
            if (!erased_point_flag) 
            {
               int column_width=11;
               outstream.setf(ios::showpoint);
               outstream << setw(column_width) << curr_t 
                    << setw(column_width) << SphereSegment_ptr->get_ID() 
                    << setw(column_width) << get_passnumber();
               for (int j=0; j<get_ndims(); j++)
               {
                  outstream << setw(column_width) << column_data[j];
               }
               outstream << endl;
            } // !erased_point_flag conditional
         } // curr_obs_ptr != NULL conditional
      } // loop over nodes in *get_SphereSegmentlist_ptr()
      outstream << endl;
   } // loop over imagenumber index

   filefunc::closefile(output_filename,outstream);
}

// --------------------------------------------------------------------------
// Member function read_info_from_file parses the ascii text file
// generated by member function save__info_to_file().  After purging
// the featurelist, this method regenerates the SphereSegments within the
// list based upon the ascii text file information.

bool SphereSegmentsGroup::read_info_from_file()
{
   string input_filename="spheresegments.txt";
   if (!filefunc::ReadInfile(input_filename))
   {
      cout << "Trouble in SphereSegmentsGroup::read_info_from_file()"
           << endl;
      cout << "Couldn't open filename = " << input_filename << endl;
      return false;
   }

   const int n=get_ndims()+3;
   double X[n];
   vector<double> curr_time;
   vector<int> SphereSegment_ID,pass_number;
   vector<threevector> UVW;

   for (unsigned int i=0; i<filefunc::text_line.size(); i += 2)
   {
      stringfunc::string_to_n_numbers(n,filefunc::text_line[i],X);
      curr_time.push_back(X[0]);
      SphereSegment_ID.push_back(basic_math::round(X[1]));
      pass_number.push_back(basic_math::round(X[2]));
      double U=X[3];
      double V=X[4];
      double W=0;
      if (get_ndims()==3)
      {
         W=X[5];
      }
      UVW.push_back(threevector(U,V,W));
   } // loop over index i labeling ascii file line number
   
   for (unsigned int i=0; i<curr_time.size(); i++)
   {
      cout << "time = " << curr_time[i]
           << " ID = " << SphereSegment_ID[i]
           << " pass = " << pass_number[i]
           << " UVW = " << UVW[i] << endl;
   }

// Destroy all existing SphereSegments before creating a new
// SphereSegment list from the input ascii file:

   destroy_all_Graphicals();

   for (unsigned int i=0; i<SphereSegment_ID.size(); i++)
   {
      int curr_ID=SphereSegment_ID[i];

      SphereSegment* curr_SphereSegment_ptr=
         get_ID_labeled_SphereSegment_ptr(curr_ID);
      if (curr_SphereSegment_ptr == NULL)
      {
         curr_SphereSegment_ptr=generate_new_SphereSegment(curr_ID);
         curr_SphereSegment_ptr->set_UVW_coords(
            curr_time[i],pass_number[i],UVW[i]);
         curr_SphereSegment_ptr->set_coords_manually_manipulated(
            curr_time[i],pass_number[i]);
         curr_SphereSegment_ptr->set_mask(
            curr_time[i],pass_number[i],false);

         curr_SphereSegment_ptr->get_PAT_ptr()->addChild(geode_ptr);
         insert_graphical_PAT_into_OSGsubPAT(curr_SphereSegment_ptr,0);

      } // curr_SphereSegment_ptr==NULL conditional
   } // loop over index i labeling entries in SphereSegment_ID STL vector

   update_display();
   reset_colors();

   return true;
}
*/

// ==========================================================================
// SphereSegment animation methods
// ==========================================================================

// Member function update_display is repeatedly executed by a callback
// in a main program.

void SphereSegmentsGroup::update_display()
{   
//   cout << "inside SphereSegmentsGroup::update_display()" << endl;
//   cout << "n_segments = " << get_n_Graphicals() << endl;

   reset_colors();
   
   GraphicalsGroup::update_display();
}

// Member function generate_RF_direction_segments()

void SphereSegmentsGroup::generate_RF_direction_segments(
   string rf_dir_filename)
{   
   cout << "inside SphereSegmentsgroup::generate_RF_direction_segments()"
        << endl;
   
/*
   banner="Enter radius for spheresegments used to visualize RF direction finding results:";
   outputfunc::write_big_banner(banner);
   double radius=15;	// meters
   cin >> radius;
*/

   filefunc::ReadInfile(rf_dir_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double easting=column_values[1];
      double northing=column_values[2];
      double yaw=column_values[3];

// Yaw = 0 north    
// Yaw = 90 West   
// Yaw = 180 South
// Yaw = 270 = -90 

      double lobe_size=column_values[4];
      double peak_strength=column_values[5];

/*
      double rmin=20;
      double rmax=50;
      const double minimum_RF_sqrd_mag=-80;
      const double maximum_RF_sqrd_mag=-45;
//      double radius=rmin+(peak_strength-minimum_RF_sqrd_mag)/
//         (maximum_RF_sqrd_mag-minimum_RF_sqrd_mag)*(rmax-rmin);
//      double radius=rmin+(peak_strength+45)/(-80+45)*(rmax-rmin);
*/

// Linear approximation to 

// 	RF peak power (measured in dB) = 10 log_10 (S0**2/r**2):

      double radius=(peak_strength+41.844)/(-.1434);
      
      double phi=-yaw;
      double phi_min=(phi-0.5*lobe_size)*PI/180;
      double phi_max=(phi+0.5*lobe_size)*PI/180;
      double el_min=0;
      double el_max=1*PI/180;
//      double el_max=5*PI/180;
//      double el_max=20*PI/180;

// On 8/5/11, we empirically determined that 

// phi = 0 		--> north
// phi = 90		--> east
// phi = 180		--> south
// phi = 270		--> west

// Therefore phi = -yaw

      display_spokes_flag=true;
      include_blast_flag=false;
      SphereSegment* SphereSegment_ptr=generate_new_SphereSegment(
         radius,phi_min,phi_max,el_min,el_max);

      threevector position(easting,northing);
      SphereSegment_ptr->set_UVW_coords(
         get_curr_t(),get_passnumber(),position);
      initialize_vertical_posn(SphereSegment_ptr);

//      generate_spheresegment_group(
//         SphereSegment_ptr,display_spokes_flag,include_blast_flag);
      reset_colors();
   } // loop over index i labeling text line
   
}
