// ==========================================================================
// RegionPolyLinePickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 5/4/09; 12/4/10; 1/9/11
// ==========================================================================

#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgRegions/RegionPolyLinesGroup.h"
#include "osg/osgRegions/RegionPolyLinePickHandler.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void RegionPolyLinePickHandler::allocate_member_objects()
{
}		       

void RegionPolyLinePickHandler::initialize_member_objects()
{
//   cout << "inside RegionPolyLinePickHandler::init_member_objs()" << endl;

   process_pick_flag=true;
   RegionPolyLinesGroup_ptr=NULL;
   label_prefix="";
}		       

RegionPolyLinePickHandler::RegionPolyLinePickHandler(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
   RegionPolyLinesGroup* RPLG_ptr,
   ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr):
   PolyLinePickHandler(3,PI_ptr,CM_ptr,dynamic_cast<PolyLinesGroup*>(RPLG_ptr),
                       MC_ptr,WCC_ptr,GO_ptr)
{
//   cout << "inside RegionPolyLinePickHandler::constructor()" << endl;
   allocate_member_objects();
   initialize_member_objects();
   RegionPolyLinesGroup_ptr=RPLG_ptr;
}

RegionPolyLinePickHandler::~RegionPolyLinePickHandler() 
{
}

// ==========================================================================
// Mouse event handling methods
// ==========================================================================

bool RegionPolyLinePickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside RegionPolyLinePickHandler::pick()" << endl;
//   cout << "RegionPolyLinePickHandler this = " << this << endl;
//   cout << "PolyLine_rather_than_Line_mode = "
//        << PolyLine_rather_than_Line_mode << endl;
//   cout << "Allow_Insertion_flag = " << Allow_Insertion_flag << endl;
//   cout << "Allow_Manipulation_flag = " << Allow_Manipulation_flag << endl;

   if (!process_pick_flag) return false;

   return PolyLinePickHandler::pick(ea);
}

// --------------------------------------------------------------------------
bool RegionPolyLinePickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside RegionPolyLinePickHandler::drag()" << endl;

   if (!process_pick_flag) return false;

   return PolyLinePickHandler::drag(ea);
}

// --------------------------------------------------------------------------
// Member function doubleclick signals user termination of RegionPolyLine
// input.

bool RegionPolyLinePickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside RegionPolyLinePickHandler::doubleclick()" << endl;
//  cout << "RegionPolyLinesGroup_ptr = " << RegionPolyLinesGroup_ptr << endl;

   int n_RegionPolyLines=RegionPolyLinesGroup_ptr->get_n_Graphicals();
//   cout << "n_RegionPolyLines = " << n_RegionPolyLines << endl;

   if (!process_pick_flag || n_RegionPolyLines==0) return false;

   if (label_prefix.size() > 0)
   {
      string label=label_prefix+" "+stringfunc::number_to_string(
         (n_RegionPolyLines-1)/2);
      RegionPolyLinesGroup_ptr->set_next_PolyLine_label(label);
   }

   bool flag=PolyLinePickHandler::doubleclick(ea);

// Compute bounding boxes for last two ROI RegionPolyLine:

   if (RegionPolyLinesGroup_ptr->get_ROI_PolyLinesGroup_flag())
   {
      PolyLine* next_to_last_PolyLine_ptr=RegionPolyLinesGroup_ptr->
         get_PolyLine_ptr(RegionPolyLinesGroup_ptr->get_n_Graphicals()-2);

      PolyLine* last_PolyLine_ptr=RegionPolyLinesGroup_ptr->
         get_PolyLine_ptr(RegionPolyLinesGroup_ptr->get_n_Graphicals()-1);

      next_to_last_PolyLine_ptr->compute_bbox();
      last_PolyLine_ptr->compute_bbox();

      next_to_last_PolyLine_ptr->instantiate_objects_inside_PolyLine_map();
      last_PolyLine_ptr->instantiate_objects_inside_PolyLine_map();
   }

   if (RegionPolyLinesGroup_ptr->get_KOZ_PolyLinesGroup_flag())
   {
//      cout << "Setting KOZ bbox" << endl;
      movers_group* movers_group_ptr=RegionPolyLinesGroup_ptr->
         get_movers_group_ptr();
      mover* KOZ_mover_ptr=movers_group_ptr->get_mover_ptr(
         mover::KOZ,movers_group_ptr->get_latest_KOZ_ID());
      if (KOZ_mover_ptr==NULL) return false;
      
      track* KOZ_track_ptr=KOZ_mover_ptr->get_track_ptr();

// Retrieve last two PolyLines within RegionPolyLinesGroup.  They
// should correspond to the top and bottom skeleton PolyLines for the
// KOZ:

      int n_PolyLines=RegionPolyLinesGroup_ptr->get_n_Graphicals();
      PolyLine* bottom_PolyLine_ptr=RegionPolyLinesGroup_ptr->
         get_PolyLine_ptr(n_PolyLines-2);
//      cout << "bottom_PolyLine_ptr = " << bottom_PolyLine_ptr << endl;
      PolyLine* top_PolyLine_ptr=RegionPolyLinesGroup_ptr->
         get_PolyLine_ptr(n_PolyLines-1);
//      cout << "top_PolyLine_ptr = " << top_PolyLine_ptr << endl;
      
      polyline* bottom_polyline_ptr=bottom_PolyLine_ptr->
         get_or_set_polyline_ptr();
      polyline* top_polyline_ptr=top_PolyLine_ptr->
         get_or_set_polyline_ptr();
//      cout << "*bottom_polyline_ptr = " << *bottom_polyline_ptr << endl;
//      cout << "*top_polyline_ptr = " << *top_polyline_ptr << endl;

      bounding_box bottom_polyline_bbox(bottom_polyline_ptr);
      bounding_box top_polyline_bbox(top_polyline_ptr);

      double xmin=basic_math::min(bottom_polyline_bbox.get_xmin(),
                      top_polyline_bbox.get_xmin());
      double xmax=basic_math::max(bottom_polyline_bbox.get_xmax(),
                      top_polyline_bbox.get_xmax());
      double ymin=basic_math::min(bottom_polyline_bbox.get_ymin(),
                      top_polyline_bbox.get_ymin());
      double ymax=basic_math::max(bottom_polyline_bbox.get_ymax(),
                      top_polyline_bbox.get_ymax());
      double zmin=basic_math::min(bottom_polyline_bbox.get_zmin(),
                      top_polyline_bbox.get_zmin());
      double zmax=basic_math::max(bottom_polyline_bbox.get_zmax(),
                      top_polyline_bbox.get_zmax());
      bounding_box KOZ_bbox(xmin,xmax,ymin,ymax,zmin,zmax);
      KOZ_track_ptr->set_bbox(KOZ_track_ptr->get_earliest_time(),KOZ_bbox);

//      cout << "KOZ_bbox = " << KOZ_bbox << endl;
//      cout << "KOZ_track_ptr = " << KOZ_track_ptr << endl;
      
   } // RegionPolyLinesGroup_ptr->get_KOZ_PolyLinesGroup_flag() conditional
//   cout << "flag = " << flag << endl;
   
   return flag;
}

// --------------------------------------------------------------------------
bool RegionPolyLinePickHandler::release()
{
//   cout << "inside RegionPolyLinePickHandler::release()" << endl;

   if (!process_pick_flag) return false;

   return PolyLinePickHandler::release();
}
