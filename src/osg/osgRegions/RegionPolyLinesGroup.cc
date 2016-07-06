// ==========================================================================
// REGIONPOLYLINESGROUP class member function definitions
// ==========================================================================
// Last modified on 12/13/08; 12/14/08; 1/22/09; 1/22/16
// ==========================================================================

#include "osg/osgRegions/RegionPolyLinesGroup.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void RegionPolyLinesGroup::allocate_member_objects()
{
}		       

void RegionPolyLinesGroup::initialize_member_objects()
{
   GraphicalsGroup_name="RegionPolyLinesGroup";
   ROI_PolyLinesGroup_flag=KOZ_PolyLinesGroup_flag=false;

   get_OSGgroup_ptr()->setUpdateCallback( 
        new AbstractOSGCallback<RegionPolyLinesGroup>(
         this, &RegionPolyLinesGroup::update_display));
}		       

RegionPolyLinesGroup::RegionPolyLinesGroup(
   const int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr, 
   threevector* GO_ptr): PolyLinesGroup(p_ndims, PI_ptr, AC_ptr, GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

RegionPolyLinesGroup::RegionPolyLinesGroup(
   const int p_ndims,Pass* PI_ptr,osgGeometry::PolygonsGroup* PG_ptr,
   AnimationController* AC_ptr, threevector* GO_ptr):PolyLinesGroup(
      p_ndims, PI_ptr, PG_ptr, AC_ptr, GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

RegionPolyLinesGroup::RegionPolyLinesGroup(
   const int p_ndims,Pass* PI_ptr,osgGeometry::PolygonsGroup* PG_ptr,
   PolyhedraGroup* PHG_ptr,AnimationController* AC_ptr, threevector* GO_ptr):
   PolyLinesGroup(p_ndims,PI_ptr,PG_ptr,PHG_ptr,AC_ptr, GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

RegionPolyLinesGroup::RegionPolyLinesGroup(
   const int p_ndims,Pass* PI_ptr,postgis_database* bgdb_ptr,
   threevector* GO_ptr):
   PolyLinesGroup(p_ndims,PI_ptr,bgdb_ptr,GO_ptr)
{
   initialize_member_objects();
   allocate_member_objects();
}

RegionPolyLinesGroup::~RegionPolyLinesGroup()
{
   cout << "inside RegionPolyLinesGroup destructor" << endl;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const RegionPolyLinesGroup& P)
{
   for (unsigned int n=0; n<P.get_n_Graphicals(); n++)
   {
      PolyLine* PolyLine_ptr=P.get_PolyLine_ptr(n);
      outstream << "PolyLine node # " << n << endl;
      outstream << "PolyLine = " << *PolyLine_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Update display member functions
// ==========================================================================

// Member function recolor_encountered_ROI_Polylines() loops over all
// ROIs which have been previously visited by some dynamic mover.  It
// recolors their bottom and top PolyLines so that they are
// distinguishable from ROIs which have not yet been visited.  We
// wrote this utility for the Baghdad UAV demo.

void RegionPolyLinesGroup::recolor_encountered_ROI_PolyLines()
{
//   cout << "inside RegionPolyLinesGroup:: recolor_encountered_ROI_PolyLines()"
//        << endl;
   if (movers_group_ptr==NULL) return;
  
   vector<int> encountered_ROI_IDs=movers_group_ptr->
      get_encountered_ROI_IDs();
   for (unsigned int r=0; r<encountered_ROI_IDs.size(); r++)
   {
//      cout << "ROI ID = " << encountered_ROI_IDs[r] 
//           << " previously encountered" << endl;
      PolyLine* PolyLine1_ptr=
         get_ID_labeled_PolyLine_ptr(2*encountered_ROI_IDs[r]);
      PolyLine* PolyLine2_ptr=
         get_ID_labeled_PolyLine_ptr(2*encountered_ROI_IDs[r]+1);

      colorfunc::Color encountered_color=colorfunc::pink;
      PolyLine1_ptr->set_permanent_color(encountered_color);
      PolyLine2_ptr->set_permanent_color(encountered_color);
   }
}

// ---------------------------------------------------------------------
// Member function update_display()

void RegionPolyLinesGroup::update_display()
{
//   cout << "inside RegionPolyLinesGroup::update_display()" << endl;

// Recolor PolyLines of ROIs which have been encountered by UAVs:

   if (ROI_PolyLinesGroup_flag)
   {
      recolor_encountered_ROI_PolyLines();
   }

   PolyLinesGroup::update_display();
}
