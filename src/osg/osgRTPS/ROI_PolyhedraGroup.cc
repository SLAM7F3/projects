// ==========================================================================
// ROI_POLYHEDRAGROUP class member function definitions
// ==========================================================================
// Last modified on 12/21/09
// ==========================================================================

#include <iomanip>
#include <set>
#include <string>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGeometry/Cylinder.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
// #include "messenger/Messenger.h"
#include "osg/osgRTPS/ROI_PolyhedraGroup.h"
// #include "Qt/rtps/RTPSMessenger.h"
// #include "Qt/rtps/MessageWrapper.h"
#include "general/outputfuncs.h"


using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ROI_PolyhedraGroup::allocate_member_objects()
{
}		       

void ROI_PolyhedraGroup::initialize_member_objects()
{
//   cout << "inside ROI_PolyhedraGroup::initialize_member_objects()" << endl;

   GraphicalsGroup_name="ROI_PolyhedraGroup";
   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<ROI_PolyhedraGroup>(
         this, &ROI_PolyhedraGroup::update_display));
}		       

ROI_PolyhedraGroup::ROI_PolyhedraGroup(Pass* PI_ptr,threevector* GO_ptr,
                                       AnimationController* AC_ptr):
   PolyhedraGroup(PI_ptr,GO_ptr,AC_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

ROI_PolyhedraGroup::~ROI_PolyhedraGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const ROI_PolyhedraGroup& P)
{
   int node_counter=0;
   for (unsigned int n=0; n<P.get_n_Graphicals(); n++)
   {
      ROI_Polyhedron* ROI_Polyhedron_ptr=P.get_ROI_Polyhedron_ptr(n);
      outstream << "ROI_Polyhedron node # " << node_counter++ << endl;
      outstream << "ROI_Polyhedron = " << *ROI_Polyhedron_ptr << endl;
   }
   return outstream;
}

// ==========================================================================
// ROI_Polyhedron creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_ROI_Polyhedron() from all other graphical
// insertion and manipulation methods...

ROI_Polyhedron* ROI_PolyhedraGroup::generate_new_ROI_Polyhedron(
   polyhedron* p_ptr,int ID,int OSGsubPAT_number)
{
   cout << "inside ROI_PolyhedraGroup::generate_new_ROI_Polyhedron()" << endl;
//   cout << "polyhedron *p_ptr = " << *p_ptr << endl;
   if (ID==-1) ID=get_next_unused_ID();

   ROI_Polyhedron* curr_ROI_Polyhedron_ptr=new ROI_Polyhedron(
      pass_ptr,get_grid_world_origin(),p_ptr,font_refptr.get(),ID);
   initialize_new_ROI_Polyhedron(curr_ROI_Polyhedron_ptr,OSGsubPAT_number);

   osg::Geode* geode_ptr=curr_ROI_Polyhedron_ptr->generate_drawable_geode();
   curr_ROI_Polyhedron_ptr->get_PAT_ptr()->addChild(geode_ptr);

   return curr_ROI_Polyhedron_ptr;
}

// ---------------------------------------------------------------------
void ROI_PolyhedraGroup::initialize_new_ROI_Polyhedron(
   ROI_Polyhedron* ROI_Polyhedron_ptr,int OSGsubPAT_number)
{
//   cout << "inside ROI_PolyhedraGroup::initialize_new_ROI_Polyhedron()" 
//        << endl;
   
   GraphicalsGroup::insert_Graphical_into_list(ROI_Polyhedron_ptr);

// Note added on 1/17/07: We should someday write a
// destroy_Polyhedron() method which should explicitly remove the
// following LineSegmentsGroup->OSGGroup_ptr from curr_Polyhedron
// before deleting curr_Polyhedron...

   initialize_Graphical(ROI_Polyhedron_ptr);
   ROI_Polyhedron_ptr->get_PAT_ptr()->addChild(
      ROI_Polyhedron_ptr->get_LineSegmentsGroup_ptr()->get_OSGgroup_ptr());
   ROI_Polyhedron_ptr->get_LineSegmentsGroup_ptr()->
      set_AnimationController_ptr(AnimationController_ptr);

   insert_graphical_PAT_into_OSGsubPAT(ROI_Polyhedron_ptr,OSGsubPAT_number);
}

// --------------------------------------------------------------------------
// Member function generate_bbox() instantiates a 3D bounding box
// based upon the top_left and bottom_right geopoint eastings and
// northings passed in as arguments.  The max [min] height for the
// bbox is assumed to be specified by the altitude of the top_left
// [bottom_right] geopoint.  This method instantiates an OSG
// ROI_Polyhedron and attaches it to *ROI_PolyhedraGroup_ptr.

ROI_Polyhedron* ROI_PolyhedraGroup::generate_bbox(
   int Polyhedra_subgroup,double alpha)
{
//   cout << "inside ROI_PolyhedraGroup::generate_bbox()" << endl;
   colorfunc::Color bbox_color=colorfunc::string_to_color(bbox_color_str);
   Polyhedron* Polyhedron_ptr=
      PolyhedraGroup::generate_bbox(Polyhedra_subgroup,bbox_color,alpha);
   return static_cast<ROI_Polyhedron*>(Polyhedron_ptr);
}

// ==========================================================================
// Polyhedron destruction member functions
// ==========================================================================

void ROI_PolyhedraGroup::destroy_all_ROI_Polyhedra()
{
//   cout << "inside ROI_PolyhedraGroup::destroy_all_ROIPolyhedra()" << endl;
   int n_ROI_Polyhedra=get_n_Graphicals();
//   cout << "n_ROI_Polyhedra = " << n_ROI_Polyhedra << endl;

   vector<ROI_Polyhedron*> ROI_Polyhedra_to_destroy;
   for (int p=0; p<n_ROI_Polyhedra; p++)
   {
      ROI_Polyhedron* ROI_Polyhedron_ptr=get_ROI_Polyhedron_ptr(p);
//      cout << "p = " << p << " ROI_Polyhedron_ptr = " << ROI_Polyhedron_ptr << endl;
      ROI_Polyhedra_to_destroy.push_back(ROI_Polyhedron_ptr);
   }

   for (int p=0; p<n_ROI_Polyhedra; p++)
   {
      destroy_ROI_Polyhedron(ROI_Polyhedra_to_destroy[p]);
   }
}

bool ROI_PolyhedraGroup::destroy_ROI_Polyhedron()
{   
//   cout << "inside ROI_PolyhedraGruop::destroy_ROI_Polyhedron()" << endl;
   return destroy_ROI_Polyhedron(get_selected_Graphical_ID());
}

bool ROI_PolyhedraGroup::destroy_ROI_Polyhedron(int ID)
{
   if (ID >= 0)
   {
      return destroy_ROI_Polyhedron(get_ID_labeled_ROI_Polyhedron_ptr(ID));
   }
   else
   {
      return false;
   }
}

bool ROI_PolyhedraGroup::destroy_ROI_Polyhedron(
   ROI_Polyhedron* curr_ROI_Polyhedron_ptr)
{
   Cylinder* Cylinder_ptr=curr_ROI_Polyhedron_ptr->get_Cylinder_ptr();
   if (Cylinder_ptr != NULL)
   {
      Cylinder_ptr->set_Polyhedron_ptr(NULL);
   }
   bool flag=destroy_Graphical(curr_ROI_Polyhedron_ptr);
   return flag;
}

// --------------------------------------------------------------------------
// Member function update_display()

void ROI_PolyhedraGroup::update_display()
{
//   cout << "******************************************************" << endl;
//   cout << "inside ROI_PolyhedraGroup::update_display()" << endl;
//   cout << "this = " << this << endl;
//   cout << "get_n_Graphicals() = " << get_n_Graphicals() << endl;

//   parse_latest_messages();

   if (altitude_dependent_volume_alphas_flag) adjust_Polyhedra_alphas();

   GraphicalsGroup::update_display();
}

// ==========================================================================
// ROI_Polyhedra display member functions
// ==========================================================================

// ==========================================================================
// Message handling member functions
// ==========================================================================

bool ROI_PolyhedraGroup::parse_next_message_in_queue(message& curr_message)
{
   cout << "inside ROI_PolyhedraGroup::parse_next_message_in_queue()" << endl;
//   cout << "curr_message.get_text_message() = "
//        << curr_message.get_text_message() << endl;

   bool message_handled_flag=false;
   return message_handled_flag;
}

/*
// --------------------------------------------------------------------------
// Member function broadcast_bbox_corners() retrieves the currently
// selected ROI_Polyhedron which we assume is a bounding box.  It extracts
// the bbox's central position as well as the relative positions of
// its 4 (upper) corners.  This method broadcasts the selected bbox's
// ID as well as the XY coordinates of its 4 corners.

void ROI_PolyhedraGroup::broadcast_bbox_corners()
{
//   cout << "inside ROI_PolyhedraGroup::broadcast_bbox_corners()" << endl;

   ROI_Polyhedron* selected_ROI_Polyhedron_ptr=dynamic_cast<ROI_Polyhedron*>(
      get_selected_Graphical_ptr());
   if (selected_ROI_Polyhedron_ptr==NULL) return;

   polyhedron* polyhedron_ptr=selected_ROI_Polyhedron_ptr->get_polyhedron_ptr();
   
//   cout << "selected_ROI_Polyhedron_ptr->get_ID() = "
//        << selected_ROI_Polyhedron_ptr->get_ID() << endl;
//   cout << "*selected_ROI_Polyhedron_ptr = " << *selected_ROI_Polyhedron_ptr
//        << endl;

   threevector ROI_Polyhedron_posn;
   if (selected_ROI_Polyhedron_ptr->get_UVW_coords(
      get_curr_t(),get_passnumber(),ROI_Polyhedron_posn))
   {         
//      cout << "ROI_Polyhedron_posn = " << ROI_Polyhedron_posn << endl;
   }

   string command="UPDATE_ROI";
   string key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   key="ID";
   value=stringfunc::number_to_string(selected_ROI_Polyhedron_ptr->get_ID());
   properties.push_back(property(key,value));

   for (int c=0; c<4; c++)
   {
      threevector curr_vertex_posn=polyhedron_ptr->get_vertex(c).get_posn();
      threevector corner_posn=ROI_Polyhedron_posn+curr_vertex_posn;
//      cout << "c = " << c 
//           << " vertex posn = " << curr_vertex_posn 
//           << " corner posn = " << corner_posn 
//           << endl;

      key="ROI corner "+stringfunc::number_to_string(c);
      value=stringfunc::number_to_string(corner_posn.get(0))+" "
         +stringfunc::number_to_string(corner_posn.get(1));
      properties.push_back(property(key,value));
   }

   get_Messenger_ptr()->broadcast_subpacket(command,properties);
} 
*/

/*
// ==========================================================================
// ActiveMQ message handling member functions
// ==========================================================================

void ROI_PolyhedraGroup::pushback_RTPSMessenger_ptr(RTPSMessenger* M_ptr)
{
   RTPSMessenger_ptrs.push_back(M_ptr);
}

RTPSMessenger* ROI_PolyhedraGroup::get_RTPSMessenger_ptr()
{
   return get_RTPSMessenger_ptr(RTPSMessenger_ptrs.size()-1);
}

const RTPSMessenger* ROI_PolyhedraGroup::get_RTPSMessenger_ptr() const
{
   return get_RTPSMessenger_ptr(RTPSMessenger_ptrs.size()-1);
}

int ROI_PolyhedraGroup::get_n_RTPSMessenger_ptrs() const
{
   return RTPSMessenger_ptrs.size();
}

RTPSMessenger* ROI_PolyhedraGroup::get_RTPSMessenger_ptr(int i)
{
   if (i >= 0 && i < RTPSMessenger_ptrs.size())
   {
      return RTPSMessenger_ptrs[i];
   }
   else
   {
      return NULL;
   }
}

const RTPSMessenger* ROI_PolyhedraGroup::get_RTPSMessenger_ptr(int i) const
{
   if (i >= 0 && i < RTPSMessenger_ptrs.size())
   {
      return RTPSMessenger_ptrs[i];
   }
   else
   {
      return NULL;
   }
}
*/
