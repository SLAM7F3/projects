// ==========================================================================
// DATAGRAPHSGROUP class member function definitions
// ==========================================================================
// Last modified on 11/27/11; 12/2/11; 12/18/11; 4/6/14
// ==========================================================================

#include <iomanip>
#include <osg/ArgumentParser>
#include <osg/Notify>
#include <osgUtil/Optimizer>
#include <osgDB/WriteFile>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgSceneGraph/DataGraph.h"
#include "osg/osgSceneGraph/DataGraphsGroup.h"
#include "general/filefuncs.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void DataGraphsGroup::allocate_member_objects()
{
//   cout << "inside DataGraphsGroup::allocate_member_objs()" << endl;
   CommonCallbacks_ptr=new CommonCallbacks;
   MyHyperFilter_refptr=new model::MyHyperFilter;
   Switch_refptr=new osg::Switch;
   LeafNodeVisitor_refptr=new LeafNodeVisitor();
   TreeVisitor_refptr=new TreeVisitor();
}		       

void DataGraphsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="DataGraphsGroup";
}		       

DataGraphsGroup::DataGraphsGroup(
   const int p_ndims,Pass* PI_ptr,threevector* GO_ptr):
   GraphicalsGroup(p_ndims,PI_ptr,GO_ptr)
{	
   allocate_member_objects();
   initialize_member_objects();
}		       

DataGraphsGroup::DataGraphsGroup(
   const int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr,
   threevector* GO_ptr):
   GraphicalsGroup(p_ndims,PI_ptr,AC_ptr,GO_ptr)
{	
   allocate_member_objects();
   initialize_member_objects();
}		       

DataGraphsGroup::~DataGraphsGroup()
{
   delete CommonCallbacks_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const DataGraphsGroup& D)
{
   for (unsigned int n=0; n<D.get_n_Graphicals(); n++)
   {
//      DataGraph* DataGraph_ptr=D.get_DataGraph_ptr(n);
      outstream << "DataGraph # " << n << endl;
//      outstream << "DataGraph = " << *DataGraph_ptr << endl;
   }
   return outstream;
}

// ---------------------------------------------------------------------
// Member function compute_total_xyz_and_hyper_bboxes() computes and
// returns the union of each individual DataGraph's xyz_bbox and
// hyper_bbox.

void DataGraphsGroup::compute_total_xyz_and_hyper_bboxes()
{ 
//   cout << "inside DataGraphsGroup::compute_total_xyz_and_hyper_bboxes()" 
//        << endl;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
//      cout << "n = " << n << endl;
      xyz_bbox.expandBy(get_DataGraph_ptr(n)->get_xyz_bbox());
      hyper_bbox.expandBy(get_DataGraph_ptr(n)->get_hyper_bbox());
   }

//   cout << "hyper_bbox.dims = " << hyper_bbox.dim() << endl;
//   cout << "hyper_bbox.xMin() = " << hyper_bbox.xMin()
//        << " xmax = " << hyper_bbox.xMax() << endl;
//   cout << "hyper_bbox.yMin() = " << hyper_bbox.yMin()
//        << " ymax = " << hyper_bbox.yMax() << endl;
//   cout << "hyper_bbox.zMin() = " << hyper_bbox.zMin()
//        << " zmax = " << hyper_bbox.zMax() << endl;

   if (hyper_bbox.dim()==4)
   {
      hyper_bbox.set(4 , 0.0 , 1.0);
      
//      hyper_bbox.expandBy(0.0 , 4);
//      hyper_bbox.expandBy(1.0 , 4);

//      cout << "hyper_bbox.pMin() = " << hyper_bbox.wMin(0)
//           << " hyper_bbox.pMax() = " << hyper_bbox.wMax(0) << endl;
   }
   
   MyHyperFilter_refptr->setHyperBoundingBox(hyper_bbox);
}

// ==========================================================================
// osg::Geometries member functions
// ==========================================================================

// Member function geometries_along_ray loops over all Datagraphs and
// calls their individual geometries_along_ray methods.  It returns a
// concatenation of each Datagraph's results.

vector<pair<osg::Geometry*,osg::Matrix> > 
DataGraphsGroup::geometries_along_ray(
   const threevector& ray_basepoint,const threevector& ray_ehat,
   double max_sphere_to_ray_frac_dist)
{ 
//   cout << "inside DataGraphsGroup::geoms_along_ray()" << endl;
   
   vector<pair<osg::Geometry*,osg::Matrix> > total_geoms_along_ray;

   const unsigned int max_total_geoms_along_ray=2;
   const double maximum_sphere_to_ray_frac_dist=2.0;
   
   while (total_geoms_along_ray.size() < max_total_geoms_along_ray && 
          max_sphere_to_ray_frac_dist < maximum_sphere_to_ray_frac_dist)
   {
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         vector<pair<osg::Geometry*,osg::Matrix> > curr_geoms_along_ray=
            get_DataGraph_ptr(n)->geometries_along_ray(
               ray_basepoint,ray_ehat,max_sphere_to_ray_frac_dist);
         for (unsigned int i=0; i<curr_geoms_along_ray.size(); i++)
         {
            total_geoms_along_ray.push_back(curr_geoms_along_ray[i]);
         } // loop over index i
      } // loop over index n
      max_sphere_to_ray_frac_dist *= 1.333;
   } // total_geoms_along_ray.size() < max_total_geoms_along_ray &&
     //   max_sphere_to_ray_frac_dist < maximum_sphere_to_ray_frac_dist
     //   while loop
   
   return total_geoms_along_ray;
}

// ==========================================================================
// Output member functions
// ==========================================================================

// Member function write_IVE_file writes the contents of the OSGsubPAT
// labeled by the input integer ID to an output .IVE file.  We wrote
// this method in December 2007 for real, rotated satellite OSG output
// purposes.

void DataGraphsGroup::write_IVE_file(string output_filename, string subdir,
                                     int OSGsubPAT_ID)
{
   outputfunc::write_banner("Writing OSGsubPAT to output IVE file:");
   
   ofstream binary_outstream;
   filefunc::dircreate(subdir);
   output_filename=subdir+output_filename+".ive";
   filefunc::deletefile(output_filename);
   

   if (osgDB::writeNodeFile(
      *(get_OSGsubPAT_ptr(OSGsubPAT_ID)) , output_filename ) )
   {
      osg::notify(osg::NOTICE) << "Wrote .ive file: "
                               << output_filename << "\n";
   }
   else
   {
      osg::notify(osg::WARN) << "Could not write .ive file. \n";
   }
}

// ==========================================================================
// 4D data animation member functions
// ==========================================================================

// These next member functions implement Ross Anderson's suggestion
// for animating large DataGraph objects over time.  We add each 3D
// DataGraph object (e.g. 3D point cloud corresponding to some
// particular .osga file) to an OSG switch with a label corresponding
// to its frame number.  When the AnimationController's frame number
// equals a DataGraph's label, that node within the Switch group is
// turned on while all other children nodes are shut off.

void DataGraphsGroup::AddSwitchChild(int frame_number,osg::Node* child_ptr)
{
   Switch_refptr->addChild(child_ptr);
   Switch_refptr->setChildValue(child_ptr,frame_number);
}

// --------------------------------------------------------------------------
// Member function add_DataGraphs_to_Switch() loops over all
// DataGraphs and adds their PAT's to the *Switch_refptr.

void DataGraphsGroup::add_Data_Nodes_to_Switch()
{
//   cout << "inside DataGraphsGroup::add_Data_Nodes_to_Switch()" << endl;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
//      cout << "n = " << n << endl;
      DataGraph* DataGraph_ptr=get_DataGraph_ptr(n);
      AddSwitchChild(n,DataGraph_ptr->get_PAT_ptr());
   } // loop over index n labeling DataGraphs
}

// --------------------------------------------------------------------------
// Member function SetSwitchChildOn() turns off all switch children
// and turns on the one corresponding to input frame_number.

void DataGraphsGroup::SetSwitchChildOn(int frame_number)
{
   Switch_refptr->setAllChildrenOff();
   Switch_refptr->setSingleChildOn(frame_number);
}

// --------------------------------------------------------------------------
// Member function update_Switch() resets the label for the
// switch child to be displayed to the current frame number.

void DataGraphsGroup::update_Switch()
{
//   cout << "inside DataGraphsGroup::update_Switch()" << endl;

//   cout << "n switch children = "
//        << Switch_refptr->getNumChildren() << endl;

   int frame_number=AnimationController_ptr->get_curr_framenumber();
   SetSwitchChildOn(frame_number);

// On 11/22/11, we observed a very distracting stutter when the
// AnimationController loops from its last frame number to its
// starting one.  If we instead slave the osg switch to system time,
// the annoying hiccup goes away...

/*
// FAKE FAKE:  Thurs Dec 22, 2011 at 3:14 pm

   double elapsed_time=timefunc::elapsed_timeofday_time();
   elapsed_time *= 10;
   int frame=basic_math::mytruncate(elapsed_time);
   frame = frame%10;

   cout << "frame = " << frame << endl;
   SetSwitchChildOn(frame);
*/

}
