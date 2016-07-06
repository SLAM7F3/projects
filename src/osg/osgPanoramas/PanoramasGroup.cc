// ==========================================================================
// PANORAMASGROUP class member function definitions
// ==========================================================================
// Last modified on 8/17/11; 8/24/11; 8/28/11; 5/30/15
// ==========================================================================

#include <vector>
#include "osg/osgAnnotators/ArmySymbolsGroup.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osgPanoramas/Panorama.h"
#include "osg/osgPanoramas/PanoramasGroup.h"
#include "templates/mytemplates.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "time/timefuncs.h"
#include "geometry/voronoifuncs.h"

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

void PanoramasGroup::allocate_member_objects()
{
   Panoramas_network_ptr=new Network<Panorama*>(1000);
   jump_frusta_map_ptr=new INT_INT_MAP;
   frusta_panoramas_map_ptr=new INT_INT_MAP;
}		       

void PanoramasGroup::initialize_member_objects()
{
   GraphicalsGroup_name="PanoramasGroup";

   GPS_response_flag=false;
   selected_panorama_ID=start_panorama_ID=stop_panorama_ID=prev_panorama_ID=-1;
   start_left_OBSFRUSTUM_ID=start_right_OBSFRUSTUM_ID=-1;
   network_edge_counter=0;
   edge_width=5;
   ArmySymbolsGroup_ptr=NULL;
   global_Terrain_Manipulator_ptr=NULL;
   GPS_SignPostsGroup_ptr=NULL;
   last_pano_selection_time=timefunc::elapsed_timeofday_time();

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<PanoramasGroup>(
         this, &PanoramasGroup::update_display));
}		       

PanoramasGroup::PanoramasGroup(
   Pass* PI_ptr,OBSFRUSTAGROUP* OFG_ptr,OBSFRUSTUMPickHandler* OPH_ptr,
   osgGA::Terrain_Manipulator* CM_3D_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   OBSFRUSTAGROUP_ptr=OFG_ptr;
   OBSFRUSTUMPickHandler_ptr=OPH_ptr;
   Terrain_Manipulator_ptr=CM_3D_ptr;

   LineSegmentsGroup_ptr=new LineSegmentsGroup(3,PI_ptr);
   SignPostsGroup_ptr=new SignPostsGroup(3,PI_ptr,GO_ptr);
   generate_pointing_direction_arrow(PI_ptr,GO_ptr);
}		       

PanoramasGroup::PanoramasGroup(
   Pass* PI_ptr,OBSFRUSTAGROUP* OFG_ptr,OBSFRUSTUMPickHandler* OPH_ptr,
   ArmySymbolsGroup* ASG_ptr,osgGA::Terrain_Manipulator* CM_3D_ptr,
   osgGA::Terrain_Manipulator* CM_3D_global_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   OBSFRUSTAGROUP_ptr=OFG_ptr;
   OBSFRUSTUMPickHandler_ptr=OPH_ptr;
   Terrain_Manipulator_ptr=CM_3D_ptr;
   global_Terrain_Manipulator_ptr=CM_3D_global_ptr;
   ArmySymbolsGroup_ptr=ASG_ptr;

   LineSegmentsGroup_ptr=new LineSegmentsGroup(3,PI_ptr);
   SignPostsGroup_ptr=new SignPostsGroup(3,PI_ptr,GO_ptr);
   generate_pointing_direction_arrow(PI_ptr,GO_ptr);
}		       

PanoramasGroup::~PanoramasGroup()
{

// We believe that destroy_all_Graphicals() is called in
// GraphicalsGroup destructor.  Need to check this assumption...

   delete ArrowsGroup_ptr;
   delete Panoramas_network_ptr;
   delete jump_frusta_map_ptr;
   delete frusta_panoramas_map_ptr;
   delete LineSegmentsGroup_ptr;
   delete SignPostsGroup_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const PanoramasGroup& P)
{
   for (unsigned int n=0; n<P.get_n_Graphicals(); n++)
   {
      cout << *P.get_Panorama_ptr(n) << endl;
   }
   
   outstream << "Panorama network = " << endl;
   outstream << *(P.Panoramas_network_ptr) << endl;
   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// Member function get_panorama_ID_for_OBSFRUSTUM() takes in the ID
// for some OBSFRUSTUM.  It returns the ID for the OBSFRUSTUM's parent
// panorama.

int PanoramasGroup::get_panorama_ID_for_OBSFRUSTUM(int OBSFRUSTUM_ID)
{
   INT_INT_MAP::iterator OBSFRUSTUM_iter=frusta_panoramas_map_ptr->find(
      OBSFRUSTUM_ID);
   if (OBSFRUSTUM_iter != frusta_panoramas_map_ptr->end())
   {
      return OBSFRUSTUM_iter->second;
   }
   else
   {
      return -1;
   }
}

// ==========================================================================
// Panorama creation and manipulation methods
// ==========================================================================

// Member function generate_panoramas() assigns
// n_OBSFRUSTA_per_panorama to each panorama within this
// PanoramasGroup object.  It records the IDs for each OBSFRUSTUM
// assigned to each panorama within the panoramas themselves as well
// as in STL map member *frusta_panoramas_map_ptr.  This method sets
// the position of the panorama equal to the common position of each
// of its OBSFRUSTA.  It also adds a node within *Panorama_network_ptr
// for each new panorama and adds LineSegmentsGroup's OSGGroup to
// PanoramasGroup's OSGsubPAT.

int PanoramasGroup::generate_panoramas(
   int n_OBSFRUSTA_per_panorama,double label_delta_z,double label_text_size)
{
//   cout << "inside PanoramasGroup::generate_panoramas()" << endl;

   unsigned int n_OBSFRUSTA=OBSFRUSTAGROUP_ptr->get_n_Graphicals();
   unsigned int n_panoramas=n_OBSFRUSTA/n_OBSFRUSTA_per_panorama;

   for (unsigned int n=0; n<n_panoramas; n++)
   {
      Panorama* Panorama_ptr=new Panorama(
         OBSFRUSTAGROUP_ptr,font_refptr.get(),n);
      initialize_new_Panorama(Panorama_ptr);

      threevector pano_posn;
      for (int j=0; j<n_OBSFRUSTA_per_panorama; j++)
      {
         int i=n*n_OBSFRUSTA_per_panorama+j;
         OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(i);
         Panorama_ptr->pushback_OBSFRUSTUM_ID(OBSFRUSTUM_ptr->get_ID());

         (*frusta_panoramas_map_ptr)[OBSFRUSTUM_ptr->get_ID()]=
            Panorama_ptr->get_ID();

         Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
         camera* camera_ptr=Movie_ptr->get_camera_ptr();
         pano_posn=camera_ptr->get_world_posn();
         Panorama_ptr->set_posn(pano_posn);
      }

// Generate SignPost with panorama ID as its label:

//      Panorama_ptr->generate_ID_label(label_delta_z,label_text_size);
      double size=0.075;
//      double height_multiplier=0.75;
      double height_multiplier=1.0;
      SignPost* SignPost_ptr=SignPostsGroup_ptr->generate_new_SignPost(
         size,height_multiplier,pano_posn);
      SignPost_ptr->set_label(stringfunc::number_to_string(
         Panorama_ptr->get_ID()));
      SignPost_ptr->set_permanent_color(
         colorfunc::get_OSG_color(colorfunc::white));

      insert_panorama_into_network(Panorama_ptr);

   } // loop over index n labeling panoramas

   int OSGsubPAT_number=0;
   insert_OSGgroup_into_OSGsubPAT(
      LineSegmentsGroup_ptr->get_OSGgroup_ptr(),OSGsubPAT_number);
   insert_OSGgroup_into_OSGsubPAT(
      SignPostsGroup_ptr->get_OSGgroup_ptr(),OSGsubPAT_number);

//   cout << "Panoramas network = " << endl;
//   cout << *Panoramas_network_ptr << endl;

//   cout << "n_panoramas = " << n_panoramas << endl;
//   cout << "get_n_Graphicals() = " << get_n_Graphicals() << endl;

   return n_panoramas;
}

// ---------------------------------------------------------------------
void PanoramasGroup::initialize_new_Panorama(
   Panorama* Panorama_ptr,int OSGsubPAT_number)
{
//   cout << "inside PanoramasGroup::initialize_new_Panorama" << endl;

   GraphicalsGroup::insert_Graphical_into_list(Panorama_ptr);
   initialize_Graphical(Panorama_ptr);

   osg::Geode* geode_ptr=Panorama_ptr->generate_drawable_geode();
   Panorama_ptr->get_PAT_ptr()->addChild(geode_ptr);
   insert_graphical_PAT_into_OSGsubPAT(Panorama_ptr,OSGsubPAT_number);

//   insert_graphical_PAT_into_OSGsubPAT(curr_Cylinder_ptr,OSGsubPAT_number);
//   reset_colors();
}

// ---------------------------------------------------------------------
// Member function get_or_create_LineSegment_ptr() takes in ID integer
// n.  If n is larger than the number of LineSegments already within
// LineSegmentsGroup, this method instantiates new canonical
// LineSegments and appends them to LineSegmentsGroup.  It then
// returns the LineSegment pointer requested by the input integer
// argument.

LineSegment* PanoramasGroup::get_or_create_LineSegment_ptr(unsigned int n)
{
   if (n >= LineSegmentsGroup_ptr->get_n_Graphicals())
   {
//      cout << "inside Polyhedron::get_or_create_LineSegment_ptr(), n = " 
//           << n << endl;
      unsigned int n_start=LineSegmentsGroup_ptr->get_n_Graphicals();
      LineSegmentsGroup_ptr->generate_canonical_segments(n+1-n_start);
//      cout << "Created " << n+1-n_start 
//           << " new canonical LineSegments" << endl;
   }
   return LineSegmentsGroup_ptr->get_LineSegment_ptr(n);
}

// --------------------------------------------------------------------------
// Member function update_display() 

void PanoramasGroup::update_display()
{
//   cout << "inside PanoramasGroup::update_display()" << endl;

// First determine if any panorama has been effectively selected by
// checking virtual camera's position compared to all panorama
// centers:

   if (!GPS_response_flag) selected_panorama_ID=-1;

   if (Terrain_Manipulator_ptr->get_rotate_about_current_eyepoint_flag())
   {

      threevector camera_posn=Terrain_Manipulator_ptr->get_eye_world_posn();
      double camera_az=Terrain_Manipulator_ptr->compute_camera_az();
//      threevector camera_What=Terrain_Manipulator_ptr->get_camera_Zhat();

//      cout << "camera posn = " << camera_posn << endl;
//      cout << "camera az = " << camera_az*180/PI << endl;
//      cout << "-What = " << -camera_What << endl;

// Loop over panorama centers and determine which one is closest to
// camera_posn.  Set selected_panorama_ID based upon this proximity test:

      if (!GPS_response_flag)
      {
         double min_distance=POSITIVEINFINITY;
         int closest_panorama=-1;
         for (unsigned int n=0; n<get_n_Graphicals(); n++)
         {
            threevector panorama_center=get_Panorama_ptr(n)->get_posn();
//         cout << "panorama_center = " << panorama_center << endl;
            double distance_between_camera_and_center=
               (panorama_center-camera_posn).magnitude();
//         cout << "dist_between_camera_and_center = "
//              << distance_between_camera_and_center << endl;
            if (distance_between_camera_and_center < min_distance)
            {
               min_distance=distance_between_camera_and_center;
               closest_panorama=get_Panorama_ptr(n)->get_ID();
            }
         }
         selected_panorama_ID=closest_panorama;
      } // !GPS_response_flag conditional
      
      curr_OBSFRUSTUM_ID=OBSFRUSTUM_in_particular_direction(
         selected_panorama_ID,camera_az);
//      cout << "Current viewing OBSFRUSTUM ID = " 
//           << curr_OBSFRUSTUM_ID << endl;

      if (curr_OBSFRUSTUM_ID >= 0)
      {
         OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(curr_OBSFRUSTUM_ID);
         OBSFRUSTUM* selected_OBSFRUSTUM_ptr=dynamic_cast<OBSFRUSTUM*>(
            OBSFRUSTAGROUP_ptr->get_selected_Graphical_ptr());
         selected_OBSFRUSTUM_ptr->set_selected_color(colorfunc::green);
         OBSFRUSTAGROUP_ptr->reset_colors();

/*
         threevector curr_global_eye_posn=
            global_Terrain_Manipulator_ptr->get_eye_world_posn();
         threevector selected_panorama_posn=
            get_ID_labeled_Panorama_ptr(get_selected_Panorama_ID())->
            get_posn();
         selected_panorama_posn.put(2,curr_global_eye_posn.get(2));
         global_Terrain_Manipulator_ptr->set_eye_world_posn(
            selected_panorama_posn);
*/

         update_pointing_direction_arrow(
            selected_OBSFRUSTUM_ptr,camera_az);
      }

//      int jump_ID=get_jump_OBSFRUSTUM_ID(curr_OBSFRUSTUM_ID);
//      if (jump_ID >= 0)
//      {
//         cout << "Can jump to OBSFRUSTUM ID = " << jump_ID << endl;
//      }
   } // rotate_about_current_eyepoint_flag==true conditional 

   Terrain_Manipulator_ptr->update_compass_heading();

   bool disallow_flag=(selected_panorama_ID >= 0);
   OBSFRUSTUMPickHandler_ptr->
      set_disallow_OBSFRUSTUM_doubleclicking_flag(disallow_flag);

//   cout << "Selected panorama ID = " << selected_panorama_ID << endl;
//   cout << "disallow_OBSFRUSTUM_doubleclicking_flag = " 
//        << disallow_flag << endl;

   if (ArmySymbolsGroup_ptr != NULL && get_selected_Panorama_ID() >= 0)
   {
      check_armysymbols();

      ArmySymbol* ArmySymbol_ptr=static_cast<ArmySymbol*>(
         ArmySymbolsGroup_ptr->get_selected_Graphical_ptr());
      if (ArmySymbol_ptr != NULL)
      {
         threevector p_hat=ArmySymbol_ptr->get_pointing_direction();
         Panorama* next_Panorama_ptr=identify_next_panorama(p_hat);
         move_from_curr_to_next_panorama(next_Panorama_ptr,p_hat);

      } // ArmySymbol_ptr != NULL conditional
   } // ArmySymbolsGroup_ptr != NULL conditional

   double forward_translation_frac_completed=
      compute_forward_translation_frac();
   fade_panoramas(forward_translation_frac_completed);

// If GPS_SignPostsGroup_ptr != NULL, reset current panorama based
// upon closest panorama to current GPS location:

   if (GPS_SignPostsGroup_ptr != NULL && GPS_response_flag) 
      update_selected_panorama_based_on_GPS_posn();
      
   GraphicalsGroup::update_display();
}

// ==========================================================================
// Panorama network member functions
// ==========================================================================

void PanoramasGroup::insert_panorama_into_network(Panorama* curr_Panorama_ptr)
{
//   cout << "inside Panoramasgroup::insert_panorama_into_network()" << endl;
//   cout << "curr_Panorama_ptr->get_ID() = " << curr_Panorama_ptr->get_ID()
//        << endl;
//   cout << "Panoramas_network_ptr->size() = " << Panoramas_network_ptr->size()
//        << endl;

   Panoramas_network_ptr->insert_site(
      curr_Panorama_ptr->get_ID(),Site<Panorama*>(curr_Panorama_ptr));
}

// ---------------------------------------------------------------------
// Member function add_panorama_network_link() adds a symmetric link
// between two Panoramas within *Panoramas_network_ptr based upon
// their input IDs.  If the link is successfully generated, this
// boolean method returns true.

bool PanoramasGroup::add_panorama_network_link(int ID1,int ID2)
{
//   cout << "inside PanoramasGroup::add_panorama_network_link()" << endl;
//   cout << "First panorama ID = " << ID1 << endl;
//   cout << "Second panorama ID = " << ID2 << endl;

// First check if network link already exists.  If so, don't try
// instantiating it again:
   
   netlink* netlink_ptr=Panoramas_network_ptr->get_netlink_ptr(ID1,ID2);
   if (netlink_ptr != NULL) return false;

   if (ID1 >=0 && ID2 >= 0 && ID1 != ID2)
   {
      Panoramas_network_ptr->add_symmetric_link(ID1,ID2);
      draw_panorama_network_link(ID1,ID2);
      return true;
   }

   return false;
}

// ---------------------------------------------------------------------
// Member function draw_panorama_network_link() 

bool PanoramasGroup::draw_panorama_network_link(int ID1,int ID2)
{
//   cout << "inside PanoramasGroup::draw_panorama_network_link()" << endl;
//   cout << "First panorama ID = " << ID1 << endl;
//   cout << "Second panorama ID = " << ID2 << endl;

   Panorama* Panorama1_ptr=get_ID_labeled_Panorama_ptr(ID1);
   Panorama* Panorama2_ptr=get_ID_labeled_Panorama_ptr(ID2);
//      cout << "Panorama1_ptr = " << Panorama1_ptr << endl;
//      cout << "Panorama2_ptr = " << Panorama2_ptr << endl;
   if (Panorama1_ptr==NULL || Panorama2_ptr==NULL) return false;

   threevector V1=Panorama1_ptr->get_posn();
   threevector V2=Panorama2_ptr->get_posn();
//      cout << "V1 = " << V1 << " V2 = " << V2 << endl;
   LineSegment* curr_segment_ptr=get_or_create_LineSegment_ptr(
      network_edge_counter++);
//      cout << "curr_segment_ptr = " << curr_segment_ptr << endl;

// Lower V1 and V2 vertices in Z direction so that purple linesegment
// links between them remain visible when user zooms into Panorama
// centers.  Also, ArmySymbol arrows should lie below Panorama centers
// so that they too are visible:

   const double z_frac=0.95;
//      const double z_frac=0.98;

   OBSFRUSTUM* OBSFRUSTUM_ptr=Panorama1_ptr->get_OBSFRUSTUM_ptr(0);
   double movie_downrange_distance=
      OBSFRUSTUM_ptr->get_movie_downrange_distance();
//   cout << "movie_downrange_distance = " << movie_downrange_distance
//        << endl;
   const double delta_z=-0.1*movie_downrange_distance;

   if (nearly_equal(V1.get(2),0))
   {
      V1.put(2,delta_z);
   }
   else
   {
      V1.put(2,V1.get(2)*z_frac);
   }
   if (nearly_equal(V2.get(2),0))
   {
      V2.put(2,delta_z);
   }
   else
   {
      V2.put(2,V2.get(2)*z_frac);
   }

   curr_segment_ptr->set_scale_attitude_posn(
      get_curr_t(),get_passnumber(),V1,V2);
   curr_segment_ptr->get_LineWidth_ptr()->setWidth(edge_width);
   curr_segment_ptr->set_permanent_color(colorfunc::purple);
   curr_segment_ptr->set_curr_color(colorfunc::purple);

// Compute OBSFRUSTUM in panorama2 which viewer should jump to when
// translating from panorama1:

   associate_jump_OBSFRUSTA(ID1,ID2);
   associate_jump_OBSFRUSTA(ID2,ID1);

// Instantiate ArmySymbols containing forward arrows:

   ArmySymbol* AS_12_ptr=add_forward_translation_arrows(V1,V2,ID2);
   ArmySymbol* AS_21_ptr=add_forward_translation_arrows(V2,V1,ID1);

   Panorama1_ptr->pushback_ArmySymbol_ptr(AS_12_ptr);
   Panorama2_ptr->pushback_ArmySymbol_ptr(AS_21_ptr);
   
   return true;
}

// ---------------------------------------------------------------------
// Member function associate_jump_OBSFRUSTA() takes in IDs for two
// panoramas.  It first retrieves their centers and computes the
// direction vector r_hat flowing from the first to the second
// panorama.  This method next loops over all OBSFRUSTA within the
// second panorama and retrieves their pointing direction vectors.  It
// returns the ID of the OBSFRUSTUM whose pointing direction most
// closely matches r_hat.

void PanoramasGroup::associate_jump_OBSFRUSTA(int ID1,int ID2)
{
   Panorama* Panorama1_ptr=get_ID_labeled_Panorama_ptr(ID1);
   Panorama* Panorama2_ptr=get_ID_labeled_Panorama_ptr(ID2);

   threevector V1=Panorama1_ptr->get_posn();
   threevector V2=Panorama2_ptr->get_posn();

   threevector r_hat=(V2-V1).unitvector();
   double r_az=atan2(r_hat.get(1),r_hat.get(0));

//   cout << "r_az = " << r_az*180/PI << " degs" << endl;
   int starting_OBSFRUSTUM_ID=OBSFRUSTUM_in_particular_direction(ID1,r_az);
   int stopping_OBSFRUSTUM_ID=OBSFRUSTUM_in_particular_direction(ID2,r_az);
    
//   cout << "Matching OBSFRUSTUA IDs before and after jump from panorama " 
//        << ID1 << " to " << ID2 << " : " 
//        << starting_OBSFRUSTUM_ID << " --> " 
//        << stopping_OBSFRUSTUM_ID << endl;

   (*jump_frusta_map_ptr)[starting_OBSFRUSTUM_ID]=stopping_OBSFRUSTUM_ID;

//   for (INT_INT_MAP::iterator iter=jump_frusta_map_ptr->begin();
//        iter != jump_frusta_map_ptr->end(); ++iter)
//   {
//      int start=iter->first;
//      int stop=iter->second;
//      cout << "start = " << start << " stop = " << stop << endl;
//   }
}

// ---------------------------------------------------------------------
// Member function get_jump_OBSFRUSTUM_ID() takes in the ID for some
// OBSFRUSTUM.  If the corresponding OBSFRUSTUM has a partner
// OBSFRUSTUM on some other circular panorama to which the viewer can
// smoothly jump, this method returns the ID of that partner
// OBSFRUSTUM.  Otherwise, -1 is returned.

int PanoramasGroup::get_jump_OBSFRUSTUM_ID(int OBSFRUSTUM_ID)
{
   INT_INT_MAP::iterator OBSFRUSTUM_iter=jump_frusta_map_ptr->find(
      OBSFRUSTUM_ID);
   if (OBSFRUSTUM_iter != jump_frusta_map_ptr->end())
   {
      return OBSFRUSTUM_iter->second;
   }
   else
   {
      return -1;
   }
}

// ---------------------------------------------------------------------
// Member function OBSFRUSTUM_in_particular_direction() takes in the
// ID for some circular panorama as well as azimuth angle r_az.  It
// loops over all OBSFRUSTA within the specified panorama and returns
// the ID of the OBSFRUSTUM which is most closely aligned with r_hat.

int PanoramasGroup::OBSFRUSTUM_in_particular_direction(int ID,double r_az)
{
//   cout << "inside PanoramasGroup::OBSFRUSTUM_in_particular_direction()"
//        << endl;
//   cout << "input panorama ID = " << ID << endl;
   int OBSFRUSTUM_ID=-1;

   Panorama* Panorama_ptr=get_ID_labeled_Panorama_ptr(ID);
   if (Panorama_ptr != NULL)
   {
      double min_delta_az=POSITIVEINFINITY;
      for (int n=0; n<Panorama_ptr->get_n_OBSFRUSTA(); n++)
      {
         OBSFRUSTUM* OBSFRUSTUM_ptr=Panorama_ptr->get_OBSFRUSTUM_ptr(n);
         Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
         camera* camera_ptr=Movie_ptr->get_camera_ptr();

         double camera_az,camera_el,camera_roll;
         camera_ptr->get_az_el_roll_from_Rcamera(
            camera_az,camera_el,camera_roll);
         camera_az=basic_math::phase_to_canonical_interval(
            camera_az,r_az-PI,r_az+PI);
         
         if (fabs(r_az-camera_az) < min_delta_az)
         {
            min_delta_az=fabs(r_az-camera_az);
            OBSFRUSTUM_ID=OBSFRUSTUM_ptr->get_ID();
         }
      } // loop over index n labeling OBSFRUSTA in panorama1
   }
   
   return OBSFRUSTUM_ID;
}

// ---------------------------------------------------------------------
// Member function add_forward_translation_arrows() instantiates a 3D
// ArmySymbol with an arrow which points from V1 towards V2.  It also
// labels the arrow with the ID of the following panorama.  This
// method returns a pointer to the dynamically generated ArmySymbol.

ArmySymbol* PanoramasGroup::add_forward_translation_arrows(
   const threevector& V1,const threevector& V2,int next_panorama_ID)
{
//   cout << "inside PanoramasGroup::add_forward_translation_arrows()" << endl; 
   if (ArmySymbolsGroup_ptr == NULL) return NULL;

   ArmySymbol* ArmySymbol_ptr=ArmySymbolsGroup_ptr->generate_new_ArmySymbol();

//   int symbol_type=6;	// red arrow
   int symbol_type=7;	// grey arrow
   ArmySymbol_ptr->set_symbol_type(symbol_type);
   ArmySymbolsGroup_ptr->generate_armysymbol_group(ArmySymbol_ptr);

   OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(0);
   double movie_downrange_distance=
      OBSFRUSTUM_ptr->get_movie_downrange_distance();
//   cout << "movie_downrange_distance = " << movie_downrange_distance << endl;
   threevector r_hat=(V2-V1).unitvector();
   threevector ArmySymbol_posn=V1+0.65*movie_downrange_distance*r_hat;

   ArmySymbol_ptr->set_UVW_coords(
      ArmySymbolsGroup_ptr->get_curr_t(),
      ArmySymbolsGroup_ptr->get_passnumber(),ArmySymbol_posn);

//   const threevector scale(0.05,0.1,0.03);
//   const threevector scale(0.005,0.01,0.003);
   const threevector scale(0.0025,0.005,0.0015);
   ArmySymbol_ptr->set_scale(
      ArmySymbolsGroup_ptr->get_curr_t(),
      ArmySymbolsGroup_ptr->get_passnumber(),scale);

// Store forward translation pointing direction within corresponding
// arrow ArmySymbol;

   ArmySymbol_ptr->set_pointing_direction(r_hat);
//   threevector r_hat=(V2-V1).unitvector();
   double theta=-atan2(r_hat.get(1),r_hat.get(0));

//   theta=-90*PI/180;	// arrows point up
//   theta=0;		// arrows point right
//   theta=90*PI/180;	// arrows point down
//   theta=180*PI/180;	// arrows point left

// Start with ArmySymbols lying flat in XY plane with their arrows
// aligned with +y_hat.  Az rotates ArmySymbols about +x_hat.  Roll
// rotates ArmySymbols about +y_hat.  El rotates ArmySymbols about
// -z_hat.  

   double az=0*PI/180;
   double roll=0*PI/180;

//   double el=0;		// Arrow points up
//   double el=90*PI/180;	// Arrow points right
//   double el=180*PI/180;	// Arrow points down
//   double el=-90*PI/180;	// Arrow points left
   double el=90*PI/180+theta;

   bool enforce_el_limits_flag=false;
   rotation R;
   R=R.rotation_from_az_el_roll(az,el,roll,enforce_el_limits_flag);
   fourvector q=R.OSG_quat_corresponding_to_rotation();
   osg::Quat quat(q.get(0),q.get(1),q.get(2),q.get(3));

   ArmySymbol_ptr->set_quaternion(
      ArmySymbolsGroup_ptr->get_curr_t(),
      ArmySymbolsGroup_ptr->get_passnumber(),quat);

   int face_number=4;	// Initially select top face
   threevector origin(0,0,0);
   ArmySymbol_ptr->reset_selected_face_drawable(face_number,origin);

// Add next panorama ID as label to forward translation arrow:

   string label=stringfunc::number_to_string(next_panorama_ID);
   ArmySymbol_ptr->initialize_text(0);
   ArmySymbol_ptr->set_permanent_text_color(colorfunc::yellow);
   ArmySymbol_ptr->set_text_color(0,colorfunc::cyan);
   ArmySymbol_ptr->set_text_label(0,label);
   ArmySymbol_ptr->set_text_posn(
      0,ArmySymbol_ptr->get_text_posn(0)-7*y_hat-2*z_hat);
   double factor=5;
   ArmySymbol_ptr->change_text_size(0,factor);

   ArmySymbolsGroup_ptr->update_mybox(ArmySymbol_ptr);
   return ArmySymbol_ptr;
}

// --------------------------------------------------------------------------
// Member function Delaunay_triangulate_pano_centers() first computes the
// center locations for all panoramas within the ladar floorplan's texture
// image's UV coordinate system.  It then calculates a Delaunay triangulation
// of all panorama centers.  Looping over all triangle edges, this method
// discards any which pass over black or grey regions of the texture image
// which are physically non-traversible.  It also discards any Delaunay
// triangle side whose opening angle with some other triangle edge is small
// and whose edge is longer.  The surviving links within the triangulated
// panoramas network are then drawn so that they can be visualized in an OSG
// viewer.

void PanoramasGroup::Delaunay_triangulate_pano_centers(
   const threevector& bottom_left,const threevector& top_right,
   Movie* movie_ptr)
{
//   cout << "inside PanoramasGroup::Delaunay_triangulate_pano_centers()" 
//        << endl;
                                
//   double texture_width=(top_right-bottom_left).get(0);
   double texture_height=(top_right-bottom_left).get(1);
//   cout << "texture_width = " << texture_width
//        << " texture_height = " << texture_height << endl;

// First store panorama center positions in ladar floorplan texture
// image's UV coordinate system:

   vector<twovector> pano_UVs;
   vector<threevector> pano_centers;
   for (unsigned int i=0; i<get_n_Graphicals(); i++)
   {
      Panorama* Panorama_ptr=get_Panorama_ptr(i);
      pano_centers.push_back(Panorama_ptr->get_posn());
      twovector curr_UV=pano_centers.back()-bottom_left;
      curr_UV.put(0,curr_UV.get(0)/texture_height);
      curr_UV.put(1,curr_UV.get(1)/texture_height);
      pano_UVs.push_back(curr_UV);
//      movie_ptr->get_texture_rectangle_ptr()->
//         get_RGB_values(curr_UV.get(0),1-curr_UV.get(1),R,G,B);

//      cout << "i = " << i << " pano ID = " << Panorama_ptr->get_ID()
//           << " posn = " << pano_centers.back() 
//           << " UV = " << curr_UV << endl;
//      cout << "R = " << R << " G = " << G << " B = " << B << endl;
   }

// In order to convert pano_centers and ladar floorplan coordinates so
// that they agree with green/blue axes, we need to subtract
// *grid_origin_ptr=(-5,-2.5,0):

   int* delaunay_triangle_vertex = NULL;
   vector<polygon> triangles;
//   vector<polygon> triangles=
//      voronoifunc::generate_Delaunay_triangles(pano_centers,
//      delaunay_triangle_vertex);

   vector<int> pano_IDs;
   vector<twovector> small_opening_angle_links;
   for (unsigned int t=0; t<triangles.size(); t++)
   {
//      cout << "t = " << t << " triangle = " << triangles[t] << endl;

      pano_IDs.push_back(delaunay_triangle_vertex[3*t+0]);
      pano_IDs.push_back(delaunay_triangle_vertex[3*t+1]);
      pano_IDs.push_back(delaunay_triangle_vertex[3*t+2]);

// For each candidate Delaunay triangle link, determine if it
// intersects any wall within ladar floorplan by checking for
// non-white coloring along the link.  If not, add the Delaunay
// triangle edge as a panorama network link:
      
      for (int v=0; v<3; v++)
      {
         int prev_index=modulo(v-1,3);
         int start_index=v;
         int stop_index=modulo(v+1,3);

         int prev_ID=pano_IDs[prev_index];
         int start_ID=pano_IDs[start_index];
         int stop_ID=pano_IDs[stop_index];
         
         twovector prev_UV=pano_UVs[prev_ID];
         twovector start_UV=pano_UVs[start_ID];
         twovector stop_UV=pano_UVs[stop_ID];

         twovector r_prev=prev_UV-start_UV;
         twovector r_curr=stop_UV-start_UV;
         double theta=acos(r_prev.unitvector().dot(r_curr.unitvector()));

         const double min_theta=10*PI/180;
         if (fabs(theta) < min_theta)
         {
            if (r_curr.magnitude() > r_prev.magnitude()) 
            {
               small_opening_angle_links.push_back(
                  twovector(start_ID,stop_ID));
            }
            else
            {
               small_opening_angle_links.push_back(
                  twovector(prev_ID,start_ID));
            }

//            cout << "prev_ID = " << prev_ID << " start_ID = " << start_ID
//                 << " stop_ID = " << stop_ID << endl;
//            cout << "theta = " << theta*180/PI 
//                 << " small opening angle found" << endl;
//            cout << "r_prev = " << r_prev << endl;
//            cout << "r_curr = " << r_curr << endl;
         }

         bool link_blocked_flag=false;
         const int n_steps=500;
         for (int n=0; n<=n_steps; n++)
         {
            double frac=double(n)/double(n_steps);
            twovector curr_UV=start_UV+frac*(stop_UV-start_UV);

//            cout << "n = " << n << " frac = " << frac 
//                 << " currU = " << curr_UV.get(0)
//                 << " currV = " << curr_UV.get(1) << endl;

// As of 2/17/11, program VIEWPANOS reads in a vertically flipped
// version of the ladar floorplan.  So we need to check the greyscale
// coloring a texture coordinates (U,1-V) rather than (U,V):

            int R,G,B;
            movie_ptr->get_texture_rectangle_ptr()->
               get_RGB_values(curr_UV.get(0),1-curr_UV.get(1),R,G,B);

//            cout << "n = " << n << " R = " << R << " G = " << G
//                 << " B = " << B << endl;
            
            if (R < 250 || G < 250 || B < 250)
            {
               link_blocked_flag=true;
               break;
            }            
         } // loop over index n labeling samples between pano centers

         if (!link_blocked_flag)
         {
            Panoramas_network_ptr->add_symmetric_link(
               pano_IDs[start_index],pano_IDs[stop_index]);
         }
         
      } // loop over index v labeling Delaunay triangle vertices

      pano_IDs.clear();
   }

   delete [] delaunay_triangle_vertex;

// Remove any links from panorama network whose opening angles are too
// small and edges are too long:

   for (unsigned int s=0; s<small_opening_angle_links.size(); s++)
   {
      Panoramas_network_ptr->delete_symmetric_link(
         small_opening_angle_links[s].get(0),
         small_opening_angle_links[s].get(1));
   }
   vector<pair<int,int> > panorama_network_links=Panoramas_network_ptr->
      find_all_netlinks();

// Draw surviving links within panorama network:

   for (unsigned int l=0; l<panorama_network_links.size(); l++)
   {
      pair<int,int> curr_link=panorama_network_links[l];
      draw_panorama_network_link(curr_link.first,curr_link.second);
   }
}


// --------------------------------------------------------------------------
// Member function Delaunay_triangulate_pano_centers() first computes the
// center locations for all panoramas within the ladar floorplan's texture
// image's UV coordinate system.  It then calculates a Delaunay triangulation
// of all panorama centers.  Looping over all triangle edges, this method
// discards any which pass over black or grey regions of the texture image
// which are physically non-traversible.  It also discards any Delaunay
// triangle side whose opening angle with some other triangle edge is small
// and whose edge is longer.  The surviving links within the triangulated
// panoramas network are then drawn so that they can be visualized in an OSG
// viewer.

void PanoramasGroup::Delaunay_triangulate_pano_centers()
{
   cout << "inside PanoramasGroup::Delaunay_triangulate_pano_centers()" 
        << endl;
                                
// First store panorama center positions in ladar floorplan texture
// image's UV coordinate system:

   vector<threevector> pano_centers;
   for (unsigned int i=0; i<get_n_Graphicals(); i++)
   {
      Panorama* Panorama_ptr=get_Panorama_ptr(i);
      pano_centers.push_back(Panorama_ptr->get_posn());
   }

   int* delaunay_triangle_vertex = NULL;
   vector<polygon> triangles;
//   vector<polygon> triangles=
//      voronoifunc::generate_Delaunay_triangles(pano_centers,
//      delaunay_triangle_vertex);

   vector<int> pano_IDs;
   vector<twovector> small_opening_angle_links;
   for (unsigned int t=0; t<triangles.size(); t++)
   {
//      cout << "t = " << t << " triangle = " << triangles[t] << endl;

      pano_IDs.push_back(delaunay_triangle_vertex[3*t+0]);
      pano_IDs.push_back(delaunay_triangle_vertex[3*t+1]);
      pano_IDs.push_back(delaunay_triangle_vertex[3*t+2]);

// Add eacg Delaunay triangle edge as a panorama network link:
      
      for (int v=0; v<3; v++)
      {
         int prev_index=modulo(v-1,3);
         int start_index=v;
         int stop_index=modulo(v+1,3);

         int prev_ID=pano_IDs[prev_index];
         int start_ID=pano_IDs[start_index];
         int stop_ID=pano_IDs[stop_index];
         
         twovector prev_center=pano_centers[prev_ID];
         twovector start_center=pano_centers[start_ID];
         twovector stop_center=pano_centers[stop_ID];

         twovector r_prev=prev_center-start_center;
         twovector r_curr=stop_center-start_center;
         double theta=acos(r_prev.unitvector().dot(r_curr.unitvector()));

         const double min_theta=10*PI/180;
         if (fabs(theta) < min_theta)
         {
            if (r_curr.magnitude() > r_prev.magnitude()) 
            {
               small_opening_angle_links.push_back(
                  twovector(start_ID,stop_ID));
            }
            else
            {
               small_opening_angle_links.push_back(
                  twovector(prev_ID,start_ID));
            }

//            cout << "prev_ID = " << prev_ID << " start_ID = " << start_ID
//                 << " stop_ID = " << stop_ID << endl;
//            cout << "theta = " << theta*180/PI 
//                 << " small opening angle found" << endl;
//            cout << "r_prev = " << r_prev << endl;
//            cout << "r_curr = " << r_curr << endl;
         }

         Panoramas_network_ptr->add_symmetric_link(
            pano_IDs[start_index],pano_IDs[stop_index]);
         
      } // loop over index v labeling Delaunay triangle vertices

      pano_IDs.clear();
   }

   delete [] delaunay_triangle_vertex;

// Remove any links from panorama network whose opening angles are too
// small and edges are too long:

   for (unsigned int s=0; s<small_opening_angle_links.size(); s++)
   {
      Panoramas_network_ptr->delete_symmetric_link(
         small_opening_angle_links[s].get(0),
         small_opening_angle_links[s].get(1));
   }
   vector<pair<int,int> > panorama_network_links=Panoramas_network_ptr->
      find_all_netlinks();

// Draw surviving links within panorama network:

   for (unsigned int l=0; l<panorama_network_links.size(); l++)
   {
      pair<int,int> curr_link=panorama_network_links[l];
      draw_panorama_network_link(curr_link.first,curr_link.second);
   }

}

// ==========================================================================
// Panorama jump member functions
// ==========================================================================

// Member function identify_next_panorama() takes in pointing
// direction vector p_hat for the most recently selected arrow
// ArmySymbol.  It forms the direction vectors from the current
// panorama to its neighbors and compares with the input pointing
// direction.  This method returns a pointer to the neighboring
// panorama which lies most in the direction of p_hat.

Panorama* PanoramasGroup::identify_next_panorama(const threevector& p_hat)
{
//   cout << "inside PanoramasGroup::identify_next_panorama()" << endl;
   
   Panorama* curr_Panorama_ptr=get_ID_labeled_Panorama_ptr(
      selected_panorama_ID);
   threevector curr_panorama_posn=curr_Panorama_ptr->get_posn();

   vector<int> neighbor_IDs=Panoramas_network_ptr->
      find_nearest_neighbors(selected_panorama_ID);

//   cout << "selected panorama ID = " << selected_panorama_ID << endl;
//   cout << "neighbor IDs = " << endl;
//   templatefunc::printVector(neighbor_IDs);

   double max_dotproduct=NEGATIVEINFINITY;
   Panorama* next_neighbor_Panorama_ptr=NULL;
   for (unsigned int n=0; n<neighbor_IDs.size(); n++)
   {
      int neighbor_ID=neighbor_IDs[n];
      Panorama* neighbor_Panorama_ptr=get_ID_labeled_Panorama_ptr(
         neighbor_ID);
      threevector neighbor_panorama_posn=neighbor_Panorama_ptr->get_posn();
      threevector curr_rhat=(neighbor_panorama_posn-curr_panorama_posn).
         unitvector();
      double curr_dotproduct=curr_rhat.dot(p_hat);
      if (curr_dotproduct > max_dotproduct)
      {
         max_dotproduct=curr_dotproduct;
         next_neighbor_Panorama_ptr=neighbor_Panorama_ptr;
      }
   } // loop over index n labeling neighboring Panoramas

//   cout << "next_neighbor_Panorama.ID = "
//        << next_neighbor_Panorama_ptr->get_ID() << endl;
   return next_neighbor_Panorama_ptr;
}

// --------------------------------------------------------------------------
// Member function move_from_curr_to_next_panorama() first resets
// panels within the previous panorama (i.e. one visited before start
// panorama=selected_panorama) to thumbnail photos.  It then issues a
// fly_to_next_panorama() call with a final virtual camera heading set
// by input direction vector p_hat.  Member vars start_panorama_ID and
// stop_panorama_ID are updated, and the coloring for their
// corresponding panoramas are reset. 

void PanoramasGroup::move_from_curr_to_next_panorama(
   int pano_ID,const threevector& p_hat)
{
//   cout << "inside PanoramasGroup::move_from_curr_to_next_panorama() #1"
//        << endl;
//   cout << "pano_ID = " << pano_ID << endl;
   Panorama* next_Panorama_ptr=get_ID_labeled_Panorama_ptr(pano_ID);
   move_from_curr_to_next_panorama(next_Panorama_ptr,p_hat);
}

void PanoramasGroup::move_from_curr_to_next_panorama(
   Panorama* next_Panorama_ptr,const threevector& p_hat)
{
//   cout << "inside PanoramasGroup::move_from_curr_to_next_panorama() #2" 
//        << endl;

   if (prev_panorama_ID >= 0)
      load_thumbnail_panels(prev_panorama_ID);

   fly_to_next_panorama(next_Panorama_ptr,p_hat);

   start_panorama_ID=selected_panorama_ID;
   stop_panorama_ID=next_Panorama_ptr->get_ID();
   recolor_start_stop_panoramas(start_panorama_ID,stop_panorama_ID);

   prev_panorama_ID=start_panorama_ID;

   recenter_virtual_camera_over_wagonwheel(next_Panorama_ptr->get_posn());
}

// --------------------------------------------------------------------------
// Member function fly_to_next_panorama() takes in next_Panorama_ptr
// along with pointing direction vector p_hat.  It issues a
// Terrain_Manipulator::flyto() call.  This method also resets
// start_left[right]_OBSFRUSTUM_ID and sets the selected ArmySymbol ID
// to -1.

void PanoramasGroup::fly_to_next_panorama(
   Panorama* next_Panorama_ptr,const threevector& p_hat)
{
//   cout << "inside PanoramasGroup::fly_to_next_panorama()" << endl;
//   cout << "p_hat = " << p_hat << endl;
//   cout << "next panorama ID = " << next_Panorama_ptr->get_ID() << endl;
//   cout << "Selected_panorama_ID = " << selected_panorama_ID << endl;

   Panorama* curr_Panorama_ptr=get_ID_labeled_Panorama_ptr(
      selected_panorama_ID);

// Reset panels within next panorama to high resolution photos:

   load_hires_panels(next_Panorama_ptr->get_ID());
   
   threevector next_panorama_posn=next_Panorama_ptr->get_posn();
   threevector q_hat=-z_hat.cross(p_hat);
//   cout << "qhat = " << q_hat << endl;

   camera virtual_camera;
   virtual_camera.set_Rcamera(q_hat,z_hat);

// Adopt real camera's horizontal and vertical FOVs as final values
// for virtual camera's fields-of-view:

   int n_anim_steps=10;		  // OK value on LOST laptops
//   int n_anim_steps=30;         // OK value on netbooks

   double final_FOV_u=-1;
   double final_FOV_v=-1;

   bool write_to_file_flag=false;
   bool no_final_slowdown_flag=false;
   Terrain_Manipulator_ptr->
      flyto(next_panorama_posn,
            virtual_camera.get_Rcamera_ptr()->transpose(),
            write_to_file_flag,no_final_slowdown_flag,
            final_FOV_u,final_FOV_v,n_anim_steps);

   int curr_index=curr_Panorama_ptr->get_OBSFRUSTUM_index(curr_OBSFRUSTUM_ID);
   int n_OBSFRUSTA_per_panorama=curr_Panorama_ptr->get_n_OBSFRUSTA();
   int pos_index=modulo(curr_index+1,n_OBSFRUSTA_per_panorama);
   int neg_index=modulo(curr_index-1,n_OBSFRUSTA_per_panorama);
   int pos_OBSFRUSTUM_ID=curr_Panorama_ptr->get_OBSFRUSTUM_ID(pos_index);
   int neg_OBSFRUSTUM_ID=curr_Panorama_ptr->get_OBSFRUSTUM_ID(neg_index);

   OBSFRUSTUM* curr_OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(
      curr_OBSFRUSTUM_ID);
   OBSFRUSTUM* pos_OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(
      pos_OBSFRUSTUM_ID);
   OBSFRUSTUM* neg_OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(
      neg_OBSFRUSTUM_ID);

   camera* curr_camera_ptr=curr_OBSFRUSTUM_ptr->get_Movie_ptr()->
      get_camera_ptr();
   camera* pos_camera_ptr=pos_OBSFRUSTUM_ptr->get_Movie_ptr()->
      get_camera_ptr();
   camera* neg_camera_ptr=neg_OBSFRUSTUM_ptr->get_Movie_ptr()->
      get_camera_ptr();

   threevector curr_What=curr_camera_ptr->get_What();
   threevector pos_What=pos_camera_ptr->get_What();
   threevector neg_What=neg_camera_ptr->get_What();
   
   double curr_theta=acos(p_hat.dot(-curr_What));
   double pos_theta=acos(p_hat.dot(-pos_What));
   double neg_theta=acos(p_hat.dot(-neg_What));
//   cout << "curr_theta = " << curr_theta*180/PI 
//        << " pos_theta = " << pos_theta*180/PI
//        << " neg_theta = " << neg_theta*180/PI << endl;

   double pos_ratio=pos_theta/curr_theta;
   double neg_ratio=neg_theta/curr_theta;
//   cout << "pos_ratio = " << pos_ratio << " neg_ratio = " << neg_ratio 
//        << endl;
   
   if (pos_ratio < neg_ratio)
   {
      start_left_OBSFRUSTUM_ID=curr_OBSFRUSTUM_ID;
      start_right_OBSFRUSTUM_ID=pos_OBSFRUSTUM_ID;
   }
   else
   {
      start_left_OBSFRUSTUM_ID=curr_OBSFRUSTUM_ID;
      start_right_OBSFRUSTUM_ID=neg_OBSFRUSTUM_ID;
   }

   if (ArmySymbolsGroup_ptr != NULL)
      ArmySymbolsGroup_ptr->set_selected_Graphical_ID(-1);
}

// --------------------------------------------------------------------------
// Member function compute_forward_translation_frac() retrieves the
// positions for the starting and stopping Panoramas as well as the
// current location of the virtual camera.  It forms and returns the
// ratio of distance traversed to total distance between the
// panoramas.

double PanoramasGroup::compute_forward_translation_frac()
{
//   cout << "inside PanoramasGroup::compute_forward_trans_frac()" << endl;
   double forward_translation_frac_completed=0;
   if (start_panorama_ID >= 0 && stop_panorama_ID >= 0)
   {
      Panorama* start_Panorama_ptr=get_ID_labeled_Panorama_ptr(
         start_panorama_ID);
      Panorama* stop_Panorama_ptr=get_ID_labeled_Panorama_ptr(
         stop_panorama_ID);
      threevector start_panorama_posn=start_Panorama_ptr->get_posn();
      threevector stop_panorama_posn=stop_Panorama_ptr->get_posn();
      threevector curr_camera_posn=
         Terrain_Manipulator_ptr->get_eye_world_posn(); 
      forward_translation_frac_completed=
         (curr_camera_posn-start_panorama_posn).magnitude()/
         (stop_panorama_posn-start_panorama_posn).magnitude();
//      cout << "frac_completed = " << forward_translation_frac_completed 
//           << endl;
   } // start_panorama_ID && stop_panorama_ID >= 0 conditional

   return forward_translation_frac_completed;
}

// --------------------------------------------------------------------------
// Member function fade_panoramas() cross fades panorama photos to simulate
// forward translation/zooming from one panorama to another.

void PanoramasGroup::fade_panoramas(double forward_translation_frac_completed)
{
//   cout << "inside PanoramasGroup::fade_panoramas()" << endl;

   const double frac_threshold1=0.65;
   const double frac_threshold2=0.85;

// Fade away starting left & right OBSFRUSTA during first part of
// forward translation:

   if (start_panorama_ID >= 0 && forward_translation_frac_completed < 
       frac_threshold1)
   {
      for (unsigned int i=0; i<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); i++)
      {
         OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(i);
         int ID=OBSFRUSTUM_ptr->get_ID();

         if (ID==start_left_OBSFRUSTUM_ID || ID==start_right_OBSFRUSTUM_ID)
         {
            Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
            if (Movie_ptr != NULL)
            {
               double alpha=Movie_ptr->get_alpha();
//               alpha -= 0.002;	// OK value for LOST laptops
               alpha -= 0.02;		// OK value for LOST laptops
//               alpha -= 0.025;	// OK value for netbooks
               alpha=basic_math::max(0.0,alpha);
               Movie_ptr->set_alpha(alpha);
            }
         }
//         else
//         {
//            Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
//            if (Movie_ptr != NULL)
//            {
//               double alpha=0.1;
//               Movie_ptr->set_alpha(alpha);
//            }
//         }
      } // loop over index i labeling OBSFRUSTA
   }

// Fade up all OBSFRUSTA movies during second part of forward
// translation:

   if (start_panorama_ID >= 0 && forward_translation_frac_completed >= 
       frac_threshold1)
   {
      for (unsigned int i=0; i<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); i++)
      {
         OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
            get_OBSFRUSTUM_ptr(i);
         Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();

         if (Movie_ptr != NULL)
         {
            double alpha=Movie_ptr->get_alpha();
            alpha += 0.04;
            alpha=basic_math::min(1.0,alpha);
            Movie_ptr->set_alpha(alpha);
         }
      } 
   } 

// Force alpha=1 for all OBSFRUSTA during third part of forward
// translation:

   if (start_panorama_ID >= 0 && forward_translation_frac_completed >= 
       frac_threshold2)
   {
      start_panorama_ID=stop_panorama_ID=-1;
      start_left_OBSFRUSTUM_ID=start_right_OBSFRUSTUM_ID=-1;
      for (unsigned int i=0; i<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); i++)
      {
         OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
            get_OBSFRUSTUM_ptr(i);
         Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();

         if (Movie_ptr != NULL)
         {
            double alpha=1.0;
            Movie_ptr->set_alpha(alpha);
         }
      } 
   } 
}

// --------------------------------------------------------------------------
// Member function recenter_virtual_camera_over_wagonwheel() translates and
// rotates the virtual "global" camera so that it's located directly
// above the selected Panorama position while keeping its altitude unchanged.

void PanoramasGroup::recenter_virtual_camera_over_wagonwheel(
   int pano_ID)
{
   Panorama* Panorama_ptr=get_ID_labeled_Panorama_ptr(pano_ID);
   recenter_virtual_camera_over_wagonwheel(Panorama_ptr->get_posn());
}

void PanoramasGroup::recenter_virtual_camera_over_wagonwheel(
   const threevector& selected_wagonwheel_posn)
{
//   cout << "inside PanoramasGroup::recenter_virtual_camera_over_wagonwheel()" 
//        << endl;

   threevector virtual_camera_posn=selected_wagonwheel_posn;
   virtual_camera_posn.put(
      2,global_Terrain_Manipulator_ptr->get_eye_world_posn().get(2));
   genmatrix final_R(3,3);
   final_R.identity();
   global_Terrain_Manipulator_ptr->jumpto(virtual_camera_posn,final_R);
}

// ---------------------------------------------------------------------
// Member function load_hires_panels() loops over all OBSFRUSTA within
// the panorama specified by input panorama_ID.  It resets their
// images to high resolution versions.

void PanoramasGroup::load_hires_panels(int panorama_ID)
{
//   cout << "inside PanoramasGroup::load_hires_panels()" << endl;
//   cout << "panorama_ID = " << panorama_ID << endl;
   Panorama* Panorama_ptr=get_ID_labeled_Panorama_ptr(panorama_ID);

   if (Panorama_ptr==NULL) return;

   for (int i=0; i<Panorama_ptr->get_n_OBSFRUSTA(); i++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=Panorama_ptr->get_OBSFRUSTUM_ptr(i);
      OBSFRUSTAGROUP_ptr->load_hires_photo(OBSFRUSTUM_ptr->get_ID());
   }
}

// ---------------------------------------------------------------------
// Member function load_thumbnail_panels() loops over all OBSFRUSTA
// within the panorama specified by input panorama_ID.  It resets
// their images to thumbnail versions.

void PanoramasGroup::load_thumbnail_panels(int panorama_ID)
{
//   cout << "inside PanoramasGroup::load_thumbnail_panels()" << endl;
//   cout << "panorama_ID = " << panorama_ID << endl;
   Panorama* Panorama_ptr=get_ID_labeled_Panorama_ptr(panorama_ID);

   if (Panorama_ptr==NULL) return;

   for (int i=0; i<Panorama_ptr->get_n_OBSFRUSTA(); i++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=Panorama_ptr->get_OBSFRUSTUM_ptr(i);
      OBSFRUSTAGROUP_ptr->load_thumbnail_photo(OBSFRUSTUM_ptr->get_ID());
   }
}

// ==========================================================================
// Panorama annotation member functions
// ==========================================================================

// Member function project_world_posn_into_imageplanes() loops over
// all Panoramas within the current PanoramasGroup.  For each
// panorama, it computes the distance between world-space point XYZ to
// the panorama center as well as the closest OBSFRUSTUM's image
// plane.  This method returns an STL vector OBSFRUSTUM ID's sorted by
// increasing distance to XYZ as well as XYZ's projection into the
// OBSFRUSTA image planes.

void PanoramasGroup::project_world_posn_into_imageplanes(
   const threevector& XYZ,vector<int>& OBSFRUSTUM_IDs,
   vector<twovector>& UV_projections)
{
//   cout << "inside PanoramasGroup::project_world_posn_into_imageplanes()" 
//        << endl;
   
   vector<double> distance_to_cameras;
   vector<int> panorama_IDs;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Panorama* Panorama_ptr=get_Panorama_ptr(n);
//      cout << "Panorama ID = " << Panorama_ptr->get_ID() << endl;
//      cout << "Panorama posn = " << Panorama_ptr->get_posn() << endl;
      panorama_IDs.push_back(Panorama_ptr->get_ID());
      
      int curr_OBSFRUSTUM_ID;
      twovector curr_UV;
      
      double curr_distance_to_camera=
         Panorama_ptr->find_closest_imageplane_to_world_posn(
            XYZ,curr_OBSFRUSTUM_ID,curr_UV);
      distance_to_cameras.push_back(curr_distance_to_camera);
      OBSFRUSTUM_IDs.push_back(curr_OBSFRUSTUM_ID);
      UV_projections.push_back(curr_UV);
   } // loop over index n labeling panoramas

   templatefunc::Quicksort(distance_to_cameras,panorama_IDs,
                           OBSFRUSTUM_IDs,UV_projections);

//   cout << "Input XYZ = " << XYZ << endl;
//   for (int i=0; i<2; i++)
//   {
//      cout << "Distance to camera = " << distance_to_cameras[i]
//           << " Panorama ID = " << panorama_IDs[i]
//           << " OBSFRUSTUM_ID = " << OBSFRUSTUM_IDs[i]
//           << " U = " << UV_projections[i].get(0)
//           << " V = " << UV_projections[i].get(1) << endl << endl;
//   }
}

// ---------------------------------------------------------------------
// Member function project_SignPosts_into_imageplanes() loops over
// every SignPost within input *SignPostsGroup_ptr and extracts its
// XYZ tip position in world-space coordinates.  It then projects each
// XYZ tip position into every panorama within *this.  For the closest
// two panoramas within *this, this method instantiates a new
// imageplane SignPost which effectively appears superposed on the
// panorama photos.  We wrote this specialized method in Jan 2010 for
// RASR demo purposes.

void PanoramasGroup::project_SignPosts_into_imageplanes(
   SignPostsGroup* SignPostsGroup_ptr,bool blinking_flag)
{
   cout << "inside PanoramasGroup::project_SignPosts_into_imageplanes()"
        << endl;

   SignPostsGroup* imageplane_SignPostsGroup_ptr=
      SignPostsGroup_ptr->get_imageplane_SignPostsGroup_ptr();
   
   if (imageplane_SignPostsGroup_ptr==NULL)
   {
      cout << "Error in PanoramasGroup::project_SignPosts_into_imageplanes()" 
           << endl;
      cout << "imageplane_SignPostsGroup_ptr=NULL!" << endl;
      return;
   }
   imageplane_SignPostsGroup_ptr->destroy_all_SignPosts();

// Recall very little true 3D geometry is visible within 360
// panoramas.  So we resize imageplane SignPosts so that their exact
// tip location is intentionally vague.

   imageplane_SignPostsGroup_ptr->set_size(0.001,0.001);
//   imageplane_SignPostsGroup_ptr->set_size(0.1,0.015);


   unsigned int n_3D_SignPosts=SignPostsGroup_ptr->get_n_Graphicals();
   cout << "n_3D_signPosts = " << n_3D_SignPosts << endl;

   for (unsigned int s=0; s<n_3D_SignPosts; s++)
   {
      SignPost* SignPost_ptr=SignPostsGroup_ptr->get_SignPost_ptr(s);
      threevector XYZ;
      if (SignPost_ptr->get_UVW_coords(
         SignPostsGroup_ptr->get_curr_t(),
         SignPostsGroup_ptr->get_passnumber(),XYZ))
      {
//         cout << "XYZ = " << XYZ << endl;

         vector<int> OBSFRUSTUM_IDs;
         vector<twovector> UV_projections;
         project_world_posn_into_imageplanes(
            XYZ,OBSFRUSTUM_IDs,UV_projections);

         cout << "s = " << s 
              << " OBSFRUSTUM_IDs[0] = " << OBSFRUSTUM_IDs[0]
              << " OBSFRUSTUM_IDs[1] = " << OBSFRUSTUM_IDs[1] << endl;
         cout << "UV[0] = " << UV_projections[0]
              << " UV[1] = " << UV_projections[1] << endl;

//         if (OBSFRUSTUM_IDs[0] < 0) continue;

// As of 3/31/2011, project 3D SignPosts only into closest Panorama
// and not closest 2 panos:

         for (int i=0; i<1; i++)
//         for (int i=0; i<2; i++)
         {
            SignPost* imageplane_SignPost_ptr=
               OBSFRUSTAGROUP_ptr->generate_SignPost_at_imageplane_location(
                  UV_projections[i],OBSFRUSTUM_IDs[i],
                  imageplane_SignPostsGroup_ptr);
            imageplane_SignPost_ptr->set_label(SignPost_ptr->get_label());

            if (blinking_flag)
            {
               imageplane_SignPost_ptr->set_blinking_flag(true);
               imageplane_SignPost_ptr->set_blinking_start_time(
                  timefunc::elapsed_timeofday_time());
               imageplane_SignPost_ptr->set_single_blink_duration(0.25);
//               imageplane_SignPost_ptr->set_single_blink_duration(0.5);
               imageplane_SignPost_ptr->set_max_blink_period(3600);
            }
         } // loop over index i labeling 2 OBSFRUSTA closest to 3D SignPost

      } // SignPost get UVW coords conditional
   } // loop over index s labeling 3D SignPosts

}

// --------------------------------------------------------------------------
// Member function recolor_start_stop_panoramas() resets the permanent
// color for the starting panorama to the truly permanent panorama
// color specified in GeometricalsGroup::permanent_colorfunc_color.
// It also temporarily resets the permanent color for the stopping
// panorama to the selected panorama color specified in
// GeometricalsGroup::selected_colorfunc_color.  We wrote this method
// in Feb 2011 in order to provide a clear indication to the user of
// which wagon-wheel is currently selected in the overhead display
// window for RASR "Google Streets" demos.

void PanoramasGroup::recolor_start_stop_panoramas(
   int start_pano_ID,int stop_pano_ID)
{
//   cout << "inside PanoramasGroup::recolor_start_stop_panoramas()" << endl;
//   cout << "start_pano_ID = " << start_pano_ID << endl;
//   cout << "stop_pano_ID = " << stop_pano_ID << endl << endl;

   Panorama* start_Panorama_ptr=get_ID_labeled_Panorama_ptr(start_pano_ID);
   Panorama* stop_Panorama_ptr=get_ID_labeled_Panorama_ptr(stop_pano_ID);

   if (start_Panorama_ptr != NULL)
   {
      for (int i=0; i<start_Panorama_ptr->get_n_OBSFRUSTA(); i++)
      {
         OBSFRUSTUM* curr_OBSFRUSTUM_ptr=start_Panorama_ptr->
            get_OBSFRUSTUM_ptr(i);
         curr_OBSFRUSTUM_ptr->set_permanent_color(
            get_permanent_colorfunc_color());
         
      } // loop over index i labeling OBSFRUSTA within starting Panorama
   }
   
   if (stop_Panorama_ptr != NULL)
   {
      for (int i=0; i<stop_Panorama_ptr->get_n_OBSFRUSTA(); i++)
      {
         OBSFRUSTUM* curr_OBSFRUSTUM_ptr=stop_Panorama_ptr->
            get_OBSFRUSTUM_ptr(i);
         curr_OBSFRUSTUM_ptr->set_permanent_color(
            get_selected_colorfunc_color());
      } // loop over index i labeling OBSFRUSTA within stopping Panorama
   }
}

// ==========================================================================
// Manipulation member functions
// ==========================================================================

void PanoramasGroup::translate(const threevector& trans)
{
   cout << "inside PanoramasGroup::translate()" << endl;
   cout << "n_panoramas = " << get_n_Graphicals() << endl;
   cout << "trans = " << trans << endl;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Panorama* Panorama_ptr=get_Panorama_ptr(n);
      Panorama_ptr->translate(get_curr_t(),get_passnumber(),trans);
   
      for (int i=0; i<Panorama_ptr->get_n_OBSFRUSTA(); i++)
      {
         OBSFRUSTUM* OBSFRUSTUM_ptr=Panorama_ptr->get_OBSFRUSTUM_ptr(i);
         camera* camera_ptr=OBSFRUSTUM_ptr->get_Movie_ptr()->
            get_camera_ptr();
//         OBSFRUSTUM_ptr->translate(get_curr_t(),get_passnumber(),trans);
         cout << "Pano n = " << n << " Obs i = " << i
              << " orig camera posn = " << camera_ptr->get_world_posn() 
              << endl;
         camera_ptr->set_world_posn(
            camera_ptr->get_world_posn()+trans);

         cout << "New camera posn = " << camera_ptr->get_world_posn()
              << endl;
//         outputfunc::enter_continue_char();
      } // loop over index i labeling OBSFRUSTA inside each Panorama

   } // loop over index n labeling individual Panoramas
}

// --------------------------------------------------------------------------
// Member function check_armysymbols()

void PanoramasGroup::check_armysymbols()
{
//   cout << "inside PanoramasGroup::check_armysymbols()" << endl;
   
   Panorama* curr_Panorama_ptr=get_ID_labeled_Panorama_ptr(
      selected_panorama_ID);
   threevector curr_panorama_posn=curr_Panorama_ptr->get_posn();
   vector<ArmySymbol*> ArmySymbol_ptrs=curr_Panorama_ptr->
      get_ArmySymbol_ptrs();

   double min_dotproduct=cos(10*PI/180);
   threevector eye_hat=-Terrain_Manipulator_ptr->get_camera_Zhat();
//   cout << "eye_hat = " << eye_hat << endl;

   for (unsigned int a=0; a<ArmySymbol_ptrs.size(); a++)
   {
      ArmySymbol* ArmySymbol_ptr=ArmySymbol_ptrs[a];
      threevector p_hat=ArmySymbol_ptr->get_pointing_direction();
      double dotproduct=p_hat.dot(eye_hat);
      if (dotproduct > min_dotproduct)
      {
//         cout << "Armysymbol " << ArmySymbol_ptr->get_ID() 
//              << " in range!" << endl;

         if (ArmySymbol_ptr->get_symbol_type() != 6)
         {
            ArmySymbol_ptr->set_symbol_type(6);	// red arrow
            ArmySymbol_ptr->reset_symbol_image();
         }
      
         if (Terrain_Manipulator_ptr->get_hmi_select_flag())
         {
            ArmySymbolsGroup_ptr->set_selected_Graphical_ID(
               ArmySymbol_ptr->get_ID());
            Terrain_Manipulator_ptr->set_hmi_select_flag(false);
            Terrain_Manipulator_ptr->set_hmi_select_value(-1);
         }
      } // dotproduct > min_dotproduct conditional
      else
      {
         if (ArmySymbol_ptr->get_symbol_type() != 7)
         {
            ArmySymbol_ptr->set_symbol_type(7);	// grey arrow
            ArmySymbol_ptr->reset_symbol_image();
         }

         if (Terrain_Manipulator_ptr->get_hmi_select_value()==2)
         {
            OBSFRUSTAGROUP_ptr->zoom_virtual_camera(0.97); 	// zoom in
//            Terrain_Manipulator_ptr->set_hmi_select_flag(false);
            Terrain_Manipulator_ptr->set_hmi_select_value(-1);
         }
         else if (Terrain_Manipulator_ptr->get_hmi_select_value()==1)
         {
            OBSFRUSTAGROUP_ptr->zoom_virtual_camera(1.03);	// zoom out
//            Terrain_Manipulator_ptr->set_hmi_select_flag(false);
            Terrain_Manipulator_ptr->set_hmi_select_value(-1);
         }
      } // dotproduct > min_dotproduct conditional

   } // loop over index a labeling ArmySymbols
}

// ==========================================================================
// Pointing direction arrow member functions
// ==========================================================================

// Member function generate_pointing_direction_arrow()

void PanoramasGroup::generate_pointing_direction_arrow(
   Pass* PI_ptr,threevector* GO_ptr)
{
   ArrowsGroup_ptr=new ArrowsGroup(3,PI_ptr,GO_ptr);
//   ArrowsGroup_ptr->set_colors(colorfunc::blue,colorfunc::cyan);
   pointing_Arrow_ptr=ArrowsGroup_ptr->generate_new_Arrow();
}

// --------------------------------------------------------------------------
// Member function update_pointing_direction_arrow()

void PanoramasGroup::update_pointing_direction_arrow(
   OBSFRUSTUM* selected_OBSFRUSTUM_ptr,double camera_az)
{
//   cout << "inside PanoramasGroup::update_pointing_direction_arrow()" << endl;
   cout << "camera_az = " << camera_az*180/PI << endl;
   
   if (get_selected_Panorama_ptr()==NULL) return;
   
   threevector Vbase=get_selected_Panorama_ptr()->get_posn();
//   cout << "Vbase = " << Vbase << endl;
   double magnitude=5*selected_OBSFRUSTUM_ptr->
      get_movie_downrange_distance();
//   cout << "mag = " << magnitude << endl;
   threevector e_hat(cos(camera_az),sin(camera_az));

   double head_size=0.1;
   pointing_Arrow_ptr->set_magnitude_direction_and_base(
      magnitude,e_hat,Vbase,head_size);
   pointing_Arrow_ptr->set_color(colorfunc::blue);
}

// ---------------------------------------------------------------------
// Member function update_selected_panorama_based_on_GPS_posn()
// searches over all neighboring Panoramas relative to the current
// selected panorama.  It resets the selected panorama ID to the one
// which is closest to the current GPS location.  If one of the
// neighboring panoramas is closest to the GPS location, the global 3D
// viewer is commanded to move to the neighboring panorama's location.

void PanoramasGroup::update_selected_panorama_based_on_GPS_posn()
{
   
// In order to avoid panorama selection churning, we require some
// minimal time to have elapsed since the previous panorama selection
// event:

   double curr_time=timefunc::elapsed_timeofday_time();
   const double min_time_since_last_pano_selection=5;	// secs

   if (curr_time-last_pano_selection_time < min_time_since_last_pano_selection)
      return;

   cout << "inside PanoramasGroup::update_selected_panorama_based_on_GPS_posn()" << endl;
//   cout << "curr_time = " << curr_time << endl;

   SignPost* GPS_SignPost_ptr=GPS_SignPostsGroup_ptr->get_SignPost_ptr(0);
      
   threevector curr_GPS_XYZ_posn;
   GPS_SignPost_ptr->get_UVW_coords(
      GPS_SignPostsGroup_ptr->get_curr_t(),
      GPS_SignPostsGroup_ptr->get_passnumber(),curr_GPS_XYZ_posn);
//   cout << "curr_GPS_posn = " << curr_GPS_posn << endl;
   twovector curr_GPS_posn(curr_GPS_XYZ_posn);

   cout << "Selected_panorama_ID = " << selected_panorama_ID << endl;
   if (selected_panorama_ID < 0) selected_panorama_ID=0;

   if (curr_OBSFRUSTUM_ID < 0) return;

   twovector selected_Panorama_posn(
      get_selected_Panorama_ptr()->get_posn());
   double min_GPS_pano_distance=(curr_GPS_posn-selected_Panorama_posn).
      magnitude();
   int closest_panorama_ID=selected_panorama_ID;

   Site<Panorama*>* selected_Panorama_site_ptr=Panoramas_network_ptr->
      get_site_ptr(selected_panorama_ID);
   
   vector<int> neighbor_IDs=selected_Panorama_site_ptr->get_neighbors();
   threevector closest_panorama_posn;
   for (unsigned int n=0; n<neighbor_IDs.size(); n++)
   {
//      cout << "n = " << n
//           << " neighbor ID = " << neighbor_IDs[n] << endl;

      Panorama* neighbor_Panorama_ptr=
         get_ID_labeled_Panorama_ptr(neighbor_IDs[n]);
      twovector neighbor_Panorama_posn(neighbor_Panorama_ptr->get_posn());
      double neighbor_GPS_pano_distance=
         (curr_GPS_posn-neighbor_Panorama_posn).magnitude();
      if (neighbor_GPS_pano_distance < min_GPS_pano_distance)
      {
         min_GPS_pano_distance=neighbor_GPS_pano_distance;
         closest_panorama_ID=neighbor_IDs[n];
         closest_panorama_posn=neighbor_Panorama_ptr->get_posn();
      }
   } // loop over index n labeling panorama neighbors
   
   cout << "closest_panorama_ID = " << closest_panorama_ID << endl;
   last_pano_selection_time=timefunc::elapsed_timeofday_time();

   if (closest_panorama_ID==get_selected_Panorama_ID()) return;

   double camera_az=Terrain_Manipulator_ptr->compute_camera_az();
//   cout << "camera_az = " << camera_az*180/PI << endl;
   threevector e_hat(cos(camera_az),sin(camera_az));
//   cout << "e_hat = " << e_hat << endl;
   move_from_curr_to_next_panorama(closest_panorama_ID,e_hat);

   set_selected_Panorama_ID(closest_panorama_ID);
}
