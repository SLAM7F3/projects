// ==========================================================================
// Purely virtual GraphicalPickHandler class to interpret mouse picks
// as 2D and 3D worldspace events
// ==========================================================================
// Last modified on 8/18/09; 12/4/10; 2/9/11; 3/20/11; 4/6/14
// ==========================================================================

#include <fstream>
#include <iostream>
#include <osg/Group>

#include "astro_geo/Ellipsoid_model.h"
#include "osg/osgGraphicals/Graphical.h"
#include "osg/osgGraphicals/GraphicalPickHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "math/threevector.h"
#include "osg/Transformer.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void GraphicalPickHandler::allocate_member_objects()
{
   ParentVisitor_refptr=new ParentVisitor();
}		       

void GraphicalPickHandler::initialize_member_objects()
{
   disable_input_flag=false;
   pass_ptr=NULL;
   passnumber_ptr=NULL;
   Nimages_ptr=NULL;
   imagenumber_ptr=NULL;
   grid_origin_ptr=NULL;
   ModeController_ptr=NULL;
   two_mouse_posns_detected=false;
   Messenger_ptr=NULL;

   surface_picking_flag=cloud_picking_flag=Zplane_picking_flag=true;
}		       

GraphicalPickHandler::GraphicalPickHandler(
   const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
   GraphicalsGroup* GG_ptr,ModeController* MC_ptr,
   WindowManager* WM_ptr,threevector* GO_ptr,osg::Node* DN_ptr):
   PickHandler(p_ndims,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   pass_ptr=PI_ptr;
   CustomManipulator_refptr=CM_ptr;
   GraphicalsGroup_ptr=GG_ptr;
   ModeController_ptr=MC_ptr;
   WindowManager_ptr=WM_ptr;
   grid_origin_ptr=GO_ptr;
   DataNode_refptr=DN_ptr;
}

GraphicalPickHandler::~GraphicalPickHandler() 
{
}

// --------------------------------------------------------------------------
threevector GraphicalPickHandler::get_grid_origin() 
{
//   cout << "inside GPH::get_grid_origin()" << endl;

   if (grid_origin_ptr==NULL)
   {
      cout << "Error in GraphicalPickHandler::get_grid_origin()" << endl;
      cout << "grid_origin_ptr=NULL" << endl;
      cout << "Exiting now..." << endl;
      exit(-1);
   }
   else
   {
      return *grid_origin_ptr;
   }
}

// ==========================================================================
// Set & get methods
// ==========================================================================

void GraphicalPickHandler::set_Messenger_ptr(Messenger* M_ptr)
{
   Messenger_ptr=M_ptr;
}

Messenger* GraphicalPickHandler::get_Messenger_ptr()
{
   return Messenger_ptr;
}

const Messenger* GraphicalPickHandler::get_Messenger_ptr() const
{
   return Messenger_ptr;
}

// ==========================================================================
// Mouse pick event handling methods
// ==========================================================================

bool GraphicalPickHandler::pick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside GraphicalPickHandler::pick()" << endl;
   if (get_ndims()==2)
   {
//      cout << "ea.getX() = " << ea.getX() << " ea.getY() = " << ea.getY()
//           << endl;
//      cout << "ea.getXnormalized() = " << ea.getXnormalized()
//           << " ea.getYnormalized() = " << ea.getYnormalized() << endl;
//      cout << "ea.getXmin() = " << ea.getXmin() 
//           << " ea.getXmax() = " << ea.getXmax() << endl;
//      cout << "ea.getYmin() = " << ea.getYmin() 
//           << " ea.getYmax() = " << ea.getYmax() << endl;

      return compute_intersections(ea.getX(),ea.getY());
   }
   else if (get_ndims()==3)
   {
      curr_voxel_screenspace_posn=threevector(ea.getX(),ea.getY(),0);
      return true;
   }
   return false;
}

// --------------------------------------------------------------------------
bool GraphicalPickHandler::pick_box(
   float oldX, float oldY, const osgGA::GUIEventAdapter& ea)
{
   if (get_ndims()==2)
   {
      return pick_2D_box(oldX, oldY, ea.getX(),ea.getY());
   }
   else if (get_ndims()==3)
   {
      return pick_3D_box(oldX, oldY, ea.getX(),ea.getY());
   }
   return false;
}

// --------------------------------------------------------------------------
bool GraphicalPickHandler::pick_2D_box(
   float oldX, float oldY, float X,float Y)
{
   cout << "The first point (x,y): " << oldX << "," << oldY << endl;
   cout << "The second point (x,y): " << X << "," << Y << endl;
   return true;
}

// --------------------------------------------------------------------------
bool GraphicalPickHandler::pick_3D_box(
   float oldX, float oldY, float X,float Y)
{
   cout << "The first point (x,y): " << oldX << "," << oldY << endl;
   cout << "The second point (x,y): " << X << "," << Y << endl;
   return true;
}

// --------------------------------------------------------------------------
bool GraphicalPickHandler::pick_point_on_Zplane(float X,float Y,double Zplane)
{
//   cout << "inside GPH::pick_point_on_Zplane()" << endl;
//   cout << "X = " << X << " Y = " << Y << " Zplane = " << Zplane << endl;

   threevector closest_world_point;
   if (get_CM_3D_ptr()->get_Transformer_ptr()->
      compute_screen_ray_intercept_with_zplane(
         X,Y,Zplane,closest_world_point))
   {
      world_coords=osg::Vec3(
         closest_world_point.get(0),closest_world_point.get(1),
         closest_world_point.get(2));
      set_pick_handler_voxel_coords();
      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
// Method pick_3D_point performs a search over all CURRENTLY VISIBLE
// world points within the 3D cloud for the one whose screen X and Y
// coordinates lie closest to the input pair (X,Y).  If we're in
// center setting interaction mode, the picked 3D point is mapped to
// the physical center of the screen.  Otherwise, a new Graphical is
// instantiated at the world-point corresponding to screen coordinates
// (X,Y).  If no 3D point is found lying reasonably close the ray
// passing through the screen X and Y coordinates, this boolean method
// returns false.

bool GraphicalPickHandler::pick_3D_point(float X,float Y)
{
//   cout << "inside GraphicalPickHandler::pick_3D_point()" << endl;
//   cout << "X = " << X << " Y = " << Y << endl;

   threevector eye_posn=get_CM_3D_ptr()->get_eye_world_posn();
//   cout << "eye_posn = " << eye_posn << endl;

   threevector closest_surface_point(NEGATIVEINFINITY,NEGATIVEINFINITY,
                                     NEGATIVEINFINITY);
   threevector closest_cloud_point(NEGATIVEINFINITY,NEGATIVEINFINITY,
                                   NEGATIVEINFINITY);

/*
   ParentVisitor_refptr->reset_matrix_transform();
   ParentVisitor_refptr->set_pointcloud_flag(false);
   ParentVisitor_refptr->set_earth_flag(false);
   geode_ptr->accept(*(ParentVisitor_refptr.get()));
   bool earth_flag=ParentVisitor_refptr->get_earth_flag();
   bool pointcloud_flag=ParentVisitor_refptr->get_pointcloud_flag();

   cout << "earth_flag = " << earth_flag << endl;
   cout << "pointcloud_flag = " << pointcloud_flag << endl;
*/

// As of Jan 7, 2007 [Oct 14, 2006], we assume that a scene graph
// either contains continuous surfaces, point clouds or just a world
// grid but not all three.  We'll clearly have to relax this
// assumption later.  But for now, we first search for intersections
// with a 3D surface.  If none are found, we then search for the
// closest XYZ point.  If no point is found, we search for a Z-plane
// grid.

   osgUtil::Hit hit;
   if (surface_picking_flag && closest_intersection_hit(X,Y,hit))
   {
      closest_surface_point = hit.getWorldIntersectPoint();
      osg::Geode* geode_ptr=hit.getGeode();
         
      ParentVisitor_refptr->reset_matrix_transform();
      ParentVisitor_refptr->set_earth_flag(false);
      geode_ptr->accept(*(ParentVisitor_refptr.get()));
      bool earth_flag=ParentVisitor_refptr->get_earth_flag();

//      cout << "earth_flag = " << earth_flag << endl;
      if (earth_flag)
      {
         double longitude,latitude,altitude;
         if (GraphicalsGroup_ptr->convert_ECI_to_LongLatAlt(
            closest_surface_point,longitude,latitude,altitude))
         {
            cout << "Longitude = " << longitude
                 << " Latitude = " << latitude
                 << " Altitude = " << altitude << endl;
            GraphicalsGroup_ptr->compute_local_ellipsoid_directions(
               longitude,latitude);
         }
      } // earth_flag conditional
    
      ParentVisitor_refptr->set_pointcloud_flag(false);
   }

   PointFinder* PointFinder_ptr=get_CM_3D_ptr()->get_PointFinder_ptr();
   if (cloud_picking_flag && PointFinder_ptr != NULL)
   {
      if (PointFinder_ptr->find_closest_world_point(
             get_CM_3D_ptr(),X,Y,closest_cloud_point))
      {
//      ParentVisitor_refptr->set_earth_flag(false);
//      ParentVisitor_refptr->set_pointcloud_flag(true);
//         cout << "closest_cloud_point = " << closest_cloud_point << endl;
         PointFinder_ptr->set_nearest_worldspace_point(closest_cloud_point);
      }
   }
   
//   cout << "closest_surface_point = " << closest_surface_point << endl;

   double eye_surface_dist=(closest_surface_point-eye_posn).magnitude();
   double eye_cloud_dist=(closest_cloud_point-eye_posn).magnitude();
//   cout << "eye_surface_dist = " << eye_surface_dist
//        << " eye_cloud_dist = " << eye_cloud_dist << endl;
   const double max_eye_cloud_dist=1E6;	// meters

   threevector closest_world_point(NEGATIVEINFINITY,NEGATIVEINFINITY,
                                   NEGATIVEINFINITY);
   if (surface_picking_flag && 
       closest_surface_point.magnitude() < POSITIVEINFINITY &&
       closest_cloud_point.magnitude() > POSITIVEINFINITY)
   {
      closest_world_point=closest_surface_point;
      ParentVisitor_refptr->set_pointcloud_flag(false);
   }
   else if (cloud_picking_flag && 
            closest_surface_point.magnitude() > POSITIVEINFINITY &&
            closest_cloud_point.magnitude() < POSITIVEINFINITY &&
            eye_cloud_dist < max_eye_cloud_dist)
   {
      closest_world_point=closest_cloud_point;
      ParentVisitor_refptr->set_pointcloud_flag(true);
   }
   else if (surface_picking_flag && cloud_picking_flag &&
            closest_surface_point.magnitude() < POSITIVEINFINITY &&
            closest_cloud_point.magnitude() < POSITIVEINFINITY)
   {
      if (eye_cloud_dist < eye_surface_dist &&
          eye_cloud_dist < max_eye_cloud_dist)
      {
         closest_world_point=closest_cloud_point;
         ParentVisitor_refptr->set_pointcloud_flag(true);
      }
      else
      {
         closest_world_point=closest_surface_point;
         ParentVisitor_refptr->set_pointcloud_flag(false);
      }
   }
   else if (Zplane_picking_flag)
   {
      if (grid_origin_ptr != NULL &&
          CustomManipulator_refptr->getName() != "EarthManipulator")
      {
         return pick_point_on_Zplane(X,Y,grid_origin_ptr->get(2));
      }
   }
   else
   {
      cout << "No point picked!" << endl;
      return false;
   }
//   cout << "closest_world_point = " << closest_world_point << endl;

   if (closest_world_point.get(0)+closest_world_point.get(1)+
       closest_world_point.get(2) > NEGATIVEINFINITY)
   {
      world_coords=osg::Vec3(
         closest_world_point.get(0),closest_world_point.get(1),
         closest_world_point.get(2));
      set_pick_handler_voxel_coords();

      return true;
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
// Member function set_pick_handler_voxel_coords stores current voxel
// coordinates measured in screen and world coordinates corresponding
// to the location where the user clicked his mouse within member
// objects curr_voxel_worldspace_posn and curr_voxel_screenspace_posn.

void GraphicalPickHandler::set_pick_handler_voxel_coords()
{   
//   cout << "inside GraphicalPickHandler::set_pick_handler_voxel_coords()" 
//        << endl;
   
   curr_voxel_worldspace_posn=
      threevector(world_coords.x(),world_coords.y(),world_coords.z());
   prev_voxel_screenspace_posn=curr_voxel_screenspace_posn;
   two_mouse_posns_detected=true;
   if (get_ndims()==2)
   {
      double px=world_coords.x();
      double py=world_coords.z();
      curr_voxel_screenspace_posn=threevector(px,py,0);
   }
   else if (get_ndims()==3)
   {
      curr_voxel_screenspace_posn=
         get_CM_3D_ptr()->get_Transformer_ptr()->
         world_to_screen_transformation(curr_voxel_worldspace_posn);
   }

//   cout << "curr_voxel_worldspace_posn = "
//        << curr_voxel_worldspace_posn << endl;
//   cout << "curr_voxel_screenspace_posn = "
//        << curr_voxel_screenspace_posn << endl;
}

// ==========================================================================
// Mouse dragging event handling methods
// ==========================================================================

bool GraphicalPickHandler::drag(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside GraphicalPickHandler::drag, ndims = " << get_ndims() 
//        << endl;

   if (get_ndims()==2)
   {
      if (compute_intersections(ea.getX(),ea.getY()))
      {
         return move_Graphical();
      }
      else return false;
   }
   else if (get_ndims()==3)
   {
      curr_voxel_screenspace_posn=threevector(ea.getX(),ea.getY(),0);
//      cout << "curr_voxel_screenspace_posn = " 
//           << curr_voxel_screenspace_posn << endl;
      return move_Graphical();
   }
   return false;
}

// --------------------------------------------------------------------------
bool GraphicalPickHandler::drag_box(
   float oldX, float oldY, const osgGA::GUIEventAdapter& ea)
{
   if (get_ndims()==2)
   {
      return drag_2D_box(oldX, oldY, ea.getX(),ea.getY());
   }
   else if (get_ndims()==3)
   {
      return drag_3D_box(ea.getX(), oldX, oldY, ea.getY());
   }
   return false;
}

// --------------------------------------------------------------------------
bool GraphicalPickHandler::drag_2D_box(float oldX,float oldY,float X,float Y)
{
   return true;
}

// --------------------------------------------------------------------------
bool GraphicalPickHandler::drag_3D_box(
   float oldX, float oldY, float X,float Y)
{
    if (X < oldX)
    {
        float temp;
        temp = X;
        X = oldX;
        oldX = temp;
    }
    if (Y > oldY)
    {
        float temp;
        temp = Y;
        Y = oldY;
        oldY = temp;
    }
//    float totalX = X - oldX;
//    float totalY = oldY - Y;

    // COULD DRAW BOX HERE AND IT WOULD UPDATE AS YOU DRAG
    /*
    behind_box_geom = new osg::Geometry();
    geode_ptr->addDrawable(behind_box_geom);

    osg::Vec3Array* vertices = new osg::Vec3Array;
    vertices->push_back( osg::Vec3( 1, current_ymin, current_zmin) ); // front left
    vertices->push_back( osg::Vec3(1, current_ymax, current_zmin) ); // front right
    vertices->push_back( osg::Vec3(1, current_ymax, current_zmax) ); // back right
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
    */

    return true;
}

// ==========================================================================
// Mouse double click event handling methods
// ==========================================================================

bool GraphicalPickHandler::doubleclick(const osgGA::GUIEventAdapter& ea)
{
//   cout << "inside GraphicalPickHandler::doubleclick()" << endl;
   return false;
}

// ==========================================================================
// Graphical generation & manipulation methods
// ==========================================================================

// Method instantiate_Graphical sets a new Graphical's position equal
// to the current voxel screen space or world space position and sets
// the selected_Graphical_ID equal to the Graphical's ID:

threevector GraphicalPickHandler::instantiate_Graphical(
   Graphical* curr_Graphical_ptr)
{   
//   cout << "inside GraphicalPickHandler::instantiate_Graphical()" << endl;
//   cout << "curr_Graphical_ptr = " << curr_Graphical_ptr << endl;
   threevector UVW;
   if (get_ndims()==2)
   {
      UVW=curr_voxel_screenspace_posn;
   }
   else if (get_ndims()==3)
   {
      UVW=curr_voxel_worldspace_posn;
   }
//   cout << "UVW = " << UVW << endl;

   curr_Graphical_ptr->set_UVW_coords(
      get_GraphicalsGroup_ptr()->get_curr_t(),
      get_GraphicalsGroup_ptr()->get_passnumber(),UVW);

   set_selected_Graphical_ID(curr_Graphical_ptr->get_ID());
   return UVW;
}

// --------------------------------------------------------------------------
// Member function find_closest_Graphical_nearby_mouse_posn loops over
// all Graphical objects within the current data set's Graphicalslist.
// It computes the distance between each Graphical's position and the
// current mouse location.  If the minimal distance is less than some
// small tolerance value, this boolean method returns true as well as
// the ID of the closest Graphical within the Graphicalslist.

bool GraphicalPickHandler::find_closest_Graphical_nearby_mouse_posn(
   int& closest_Graphical_ID)
{   
//   cout << "inside GraphicalPickHandler::find_closest_Graphical_nearby_mouse_posn()" << endl;
//   cout << "this = " << this << endl;
//   cout << "get_passnumber() = " << get_passnumber() << endl;
   
   closest_Graphical_ID=-1;
   int node_counter=0;
   float min_sqrd_dist=POSITIVEINFINITY;

   for (unsigned int n=0; n<get_GraphicalsGroup_ptr()->get_n_Graphicals(); n++)
   {
      Graphical* curr_Graphical_ptr=get_GraphicalsGroup_ptr()->
         get_Graphical_ptr(n);
//      cout << "n = " << n << " curr_Graphical_ptr = " << curr_Graphical_ptr
//           << endl;

// Do not select any Graphical which is currently masked!

      bool mask_flag=
         curr_Graphical_ptr->get_mask(get_curr_t(),get_passnumber());
//      cout << "mask_flag = " << mask_flag << endl;

      threevector Graphical_posn;
      if (!mask_flag && curr_Graphical_ptr->get_UVW_coords(
         get_curr_t(),get_passnumber(),Graphical_posn))
      {

// If ndims==3, we need to convert Graphical_posn from world to screen
// space coordinates:

         threevector Graphical_screenspace_posn=Graphical_posn;
//         cout << "get_ndims() = " << get_ndims() << endl;
//         cout << "get_CM_3D_ptr() = " << get_CM_3D_ptr() << endl;

         if (get_ndims()==3)
         {
            Graphical_screenspace_posn=
               get_CM_3D_ptr()->get_Transformer_ptr()->
               world_to_screen_transformation(Graphical_posn);
            Graphical_screenspace_posn.put(2,0);
         }

//         cout << "n = " << node_counter
//              << " Graphical_screenspace_posn = " 
//              << Graphical_screenspace_posn
//              << " curr_voxel_screenspace_posn = " 
//              << curr_voxel_screenspace_posn << endl;
//         cout << "Graphical_posn = " << Graphical_posn << endl;
         
         float sqrd_dist=sqrd_screen_dist(
            Graphical_screenspace_posn,curr_voxel_screenspace_posn);

         threevector scale;
         curr_Graphical_ptr->get_scale(get_curr_t(),get_passnumber(),scale);
         double s=basic_math::max(scale.get(0),scale.get(1),scale.get(2));
         
// As of Aug 17, 2009, we experiment with setting a floor value on the
// scale s.  If s becomes too small, it's nearly impossible to pick a
// Graphical.  So try forcing s to be at least as large as some
// reasonable small value:

         s=basic_math::max(s,0.5);
         
//         cout << "sqrt(sqrd_dist) = " << sqrt(sqrd_dist) << endl;
//         cout << "max_dist_to_Graphical = "
//              << get_max_distance_to_Graphical() << endl;
//         cout << "scale = " << scale << " s = " << s << endl;
//         cout << "s*get_max_distance_to_Graphical() = "
//              << s*get_max_distance_to_Graphical() << endl;
//         cout << "sqrt(min_sqrd_dist) = " << sqrt(min_sqrd_dist) << endl;

         if (sqrd_dist < sqr(s*get_max_distance_to_Graphical()) &&
             sqrd_dist < min_sqrd_dist)
         {
            min_sqrd_dist=sqrd_dist;
            closest_Graphical_ID=curr_Graphical_ptr->get_ID();
         }
      } // !mask_flag && get_UVW_coords boolean conditional

      node_counter++;
   } // loop over index n labeling Graphicals

//   cout << "closest_Graphical_ID = " << closest_Graphical_ID << endl;
   return (closest_Graphical_ID != -1);
}

// --------------------------------------------------------------------------
// Method select_Graphical assigns selected_Graphical_ID equal to
// the ID of an existing Graphical which lies sufficiently close to a
// point picked by the user with his mouse.  If no Graphical is nearby
// the selected point, selected_Graphical_ID is set equal to -1,
// and all Graphicals are effectively de-selected.  The selected
// Graphical number is returned by this method.

int GraphicalPickHandler::select_Graphical()
{   
//   cout << "inside GraphicalPickHandler::select_Graphical()" << endl;
//   cout << "get_ndims() = " << get_ndims() << endl;
   int closest_Graphical_ID;
   if (find_closest_Graphical_nearby_mouse_posn(closest_Graphical_ID))
   {
      set_selected_Graphical_ID(closest_Graphical_ID);
   }
   else
   {
      set_selected_Graphical_ID(-1);
   }
//   cout << "selected Graphical number = "
//        << get_selected_Graphical_ID() << endl;
   return get_selected_Graphical_ID();
}

// --------------------------------------------------------------------------
// Method move_Graphical repositions the Graphical within the current
// Graphicallist whose ID equals selected_Graphical_ID to the
// current location of the mouse.

bool GraphicalPickHandler::move_Graphical()
{   
//   cout << "inside GraphicalPickHandler::move_Graphical()" << endl;
   
//   Graphical* curr_Graphical_ptr=get_GraphicalsGroup_ptr()->
//      get_ID_labeled_Graphical_ptr(get_selected_Graphical_ID());

   Graphical* curr_Graphical_ptr=get_GraphicalsGroup_ptr()->
      get_selected_Graphical_ptr();

   bool graphical_moved_flag=false;

   if (curr_Graphical_ptr != NULL)
   {
      threevector Graphical_posn;
      if (curr_Graphical_ptr->get_UVW_coords(
         get_curr_t(),get_passnumber(),Graphical_posn))
      {
//         cout << "ID = " << curr_Graphical_ptr->get_ID() << endl;
//         cout << "name = " << curr_Graphical_ptr->get_name() << endl;
         cout << "Graphical_posn = " << Graphical_posn << endl;

// If ndims==3, we need to convert Graphical_posn from world to screen
// space coordinates:

         threevector Graphical_screenspace_posn=Graphical_posn;

         if (get_ndims()==3)
         {
            Graphical_screenspace_posn=
               get_CM_3D_ptr()->get_Transformer_ptr()->
               world_to_screen_transformation(Graphical_posn);
         }

//         cout << "Graphical_screenspace_posn = " 
//              << Graphical_screenspace_posn << endl;
//         cout << "curr_voxel_screenspace_posn = "
//              << curr_voxel_screenspace_posn << endl;
//         cout << "max_dist_to_graphicals = "
//              << get_max_distance_to_Graphical() << endl;

         if (sqrd_screen_dist(
            Graphical_screenspace_posn,curr_voxel_screenspace_posn) < 
             sqr(get_max_distance_to_Graphical())) 
         {
            threevector curr_voxel_posn(curr_voxel_screenspace_posn);

// If ndims==3, we form a new Graphical screen space position using the
// current X and Y screen coordinates obtained from the mouse and the
// Graphical's existing Z screen coordinate.  We then transform this new
// Graphical screen space position back to world-space coordinates.  We
// next restore the Graphical's original world-space z value.  Mouse
// dragging of 3D Graphical's consequently affects only their
// world-space x and y coordinates.  (Manipulation of world-space z
// values is much better performed using the key-driven move_z
// method!)  The final modified x and y world-space coordinates are
// stored within the Graphical's UVW coordinates:

            if (get_ndims()==3)
            {
               threevector new_Graphical_screenspace_posn(
                  curr_voxel_screenspace_posn.get(0),
                  curr_voxel_screenspace_posn.get(1),
                  Graphical_screenspace_posn.get(2));
               threevector new_Graphical_worldspace_posn=
                  get_CM_3D_ptr()->get_Transformer_ptr()->
                  screen_to_world_transformation(
                     new_Graphical_screenspace_posn);

               curr_voxel_posn=new_Graphical_worldspace_posn;
//               cout << "curr_voxel_posn = " << curr_voxel_posn << endl;
//               cout << "Graphical worldspace posn = " 
//                    << new_Graphical_worldspace_posn << endl;

               Ellipsoid_model* Ellipsoid_model_ptr=
                  get_GraphicalsGroup_ptr()->get_Ellipsoid_model_ptr();
               if (Ellipsoid_model_ptr != NULL)
               {
                  
// If Ellipsoid_model_ptr is defined, we assume that the Graphical to
// be moved is located near the blue marble surface.  We then alter
// its longitude & latitude but keep its altitude fixed:
                  
                  double longitude,latitude,orig_altitude,curr_altitude;
                  get_GraphicalsGroup_ptr()->convert_XYZ_to_LongLatAlt(
                     Graphical_posn,longitude,latitude,orig_altitude);
                  get_GraphicalsGroup_ptr()->convert_XYZ_to_LongLatAlt(
                     curr_voxel_posn,longitude,latitude,curr_altitude);
                  get_GraphicalsGroup_ptr()->convert_LongLatAlt_to_XYZ(
                     longitude,latitude,orig_altitude,curr_voxel_posn);
               }
               else
               {
                  curr_voxel_posn.put(2,Graphical_posn.get(2));
               } // Ellipsoid_model_ptr != NULL conditional
            } // ndims==3 conditional
            
            curr_Graphical_ptr->set_UVW_coords(
               get_curr_t(),get_passnumber(),curr_voxel_posn);
            graphical_moved_flag=true;
               
// Make a note that we have manually manipulated the Graphical's UVW
// coordinates for one particular time in one particular pass:

            curr_Graphical_ptr->set_coords_manually_manipulated(
               get_curr_t(),get_passnumber());

// On 3/4/13, we commented out the following section in order to
// NOT have 2D UV feature coordinates automatically (and incorrectly)
// propagated from one video frame to the next!

/*
// To expedite 2D video Graphical selection, we generally reset all
// Graphical locations for images with numbers greater than or equal
// to the current one to curr_voxel_posn.  We cease performing this
// automatic reset when a Graphical whose UVW coordinates have already
// been manually manipulated is encountered in the loop over image
// numbers:

            for (unsigned int n=GraphicalsGroup_ptr->get_curr_framenumber()+1; 
                 n <= GraphicalsGroup_ptr->get_last_framenumber(); n++)
            {

// As of 6/5/05, we simply set the time associated with each image in
// pass #0 equal to its imagenumber.  This will eventually need to be
// generalized so that the time field corresponds to a true temporal
// measurement...

               double future_t=static_cast<double>(n);
               if (curr_Graphical_ptr->get_coords_manually_manipulated(
                  future_t,get_passnumber()))
               {
                  break;
               }
               else
               {
                  curr_Graphical_ptr->set_UVW_coords(
                     future_t,get_passnumber(),curr_voxel_screenspace_posn);
               }
            } // loop over index n labeling image number
*/
        
         } // dist < max_dist_to_crosshairs conditional
      } // get_UVW_coords boolean conditional
   } // currnode_ptr != NULL conditional

   return graphical_moved_flag;
}

// ==========================================================================
// Mouse scaling event handling methods
// ==========================================================================

bool GraphicalPickHandler::toggle_scaling_mode()
{
//   cout << "Graphical scaling mode toggled off" << endl;
   return false;
}

// --------------------------------------------------------------------------
bool GraphicalPickHandler::scale(const osgGA::GUIEventAdapter& ea)
{
   if (get_ndims()==3)
   {
      return false;
   }
   return false;
}

bool GraphicalPickHandler::scale(
   float oldX,float oldY,const osgGA::GUIEventAdapter& ea)
{
   if (get_ndims()==3)
   {
      return scale(ea.getX(),ea.getY(),oldX,oldY);
   }
   return false;
}

bool GraphicalPickHandler::scale(float X,float Y,float oldX,float oldY)
{
   return false;
}

bool GraphicalPickHandler::scale_Graphical(float deltaX,float deltaY)
{   
//   cout << "inside GPH::scale_Graphical()" << endl;
//   cout << "deltaX = " << deltaX << " deltaY = " << deltaY << endl;

// As of 6/5/05, we simply set the time associated with each image in
// pass #0 equal to its imagenumber.  This will eventually need to be
// generalized so that the time field corresponds to a true temporal
// measurement...

   Graphical* curr_Graphical_ptr=get_GraphicalsGroup_ptr()->
      get_ID_labeled_Graphical_ptr(get_selected_Graphical_ID());

   if (curr_Graphical_ptr != NULL)
   {
      threevector graphical_posn;
      curr_Graphical_ptr->get_UVW_coords(
         get_curr_t(),get_passnumber(),graphical_posn);

      if (two_mouse_posns_detected)
      {
         threevector r_prev=prev_voxel_screenspace_posn-graphical_posn;
         threevector r_curr=curr_voxel_screenspace_posn-graphical_posn;
         for (unsigned int d=0; d<3; d++)
         {
            if ( !nearly_equal(r_prev.get(d),0) )
            {
               curr_screenspace_scale.put(d,r_curr.get(d)/r_prev.get(d));
            }
         } // loop over index d labeling dims
      } // two_mouse_posns_detected conditional   

      threevector s;
      if (curr_Graphical_ptr->get_scale(get_curr_t(),get_passnumber(),s))
      {
         float sX=1.0;
         float sY=1.0;
         if (deltaX > 0 && deltaY > 0)
         {
            sX=sY=1.01;
         }
         if (deltaX < 0 && deltaY < 0)
         {
            sX=sY=0.99;
         }

         threevector s_new(sX*s.get(0),s.get(1),sY*s.get(2));
         curr_Graphical_ptr->set_scale(get_curr_t(),get_passnumber(),s_new);

         cout << "scale = " << s_new.get(0) << ","
              << s_new.get(1) << "," << s_new.get(2) << endl;

         curr_Graphical_ptr->set_coords_manually_manipulated(
            get_curr_t(),get_passnumber());
      }
      
   } // curr_Graphical_ptr != NULL conditional

   return false;
}

// ==========================================================================
// Mouse rotating event handling methods
// ==========================================================================

bool GraphicalPickHandler::toggle_rotate_mode()
{
//   cout << "Graphical rotation mode toggled off" << endl;
   return false;
}

// --------------------------------------------------------------------------
bool GraphicalPickHandler::rotate(
   float oldX,float oldY,const osgGA::GUIEventAdapter& ea)
{
   if (get_ndims()==2)
   {
      if (compute_intersections(ea.getX(),ea.getY()))
      {
         return rotate_Graphical();
      }
      else
      {
         return false;
      }
   }
   else if (get_ndims()==3)
   {
      prev_voxel_screenspace_posn=curr_voxel_screenspace_posn;
      curr_voxel_screenspace_posn=threevector(ea.getX(),ea.getY(),0);
      return rotate_Graphical();
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
// Method rotate_Graphical rotates the Graphical within the current
// Graphicallist whose ID equals selected_Graphical_ID about its
// central position.

bool GraphicalPickHandler::rotate_Graphical()
{   
   cout << "inside GraphicalPickHandler::rotate_Graphical()" << endl;

// As of 6/5/05, we simply set the time associated with each image in
// pass #0 equal to its imagenumber.  This will eventually need to be
// generalized so that the time field corresponds to a true temporal
// measurement...

   Graphical* curr_Graphical_ptr=get_GraphicalsGroup_ptr()->
      get_ID_labeled_Graphical_ptr(get_selected_Graphical_ID());

   if (curr_Graphical_ptr != NULL)
   {
      threevector graphical_posn;
      curr_Graphical_ptr->get_UVW_coords(
         get_curr_t(),get_passnumber(),graphical_posn);

      threevector graphical_screenspace_posn(graphical_posn);

      if (get_ndims()==3)
      {
         graphical_screenspace_posn=
            get_CM_3D_ptr()->get_Transformer_ptr()->
            world_to_screen_transformation(graphical_posn);
         graphical_screenspace_posn.put(2,0);
      }
//      cout << "Graphical_screenspace_posn = " 
//           << graphical_screenspace_posn << endl;
    
      curr_screenspace_rotation.put(1,0.1*PI/180);

      if (two_mouse_posns_detected)
      {
         threevector r_prev=prev_voxel_screenspace_posn-
            graphical_screenspace_posn;
         threevector r_curr=curr_voxel_screenspace_posn-
            graphical_screenspace_posn;
         threevector crossproduct=r_prev.cross(r_curr);

         if (get_ndims()==2)
         {
            curr_screenspace_rotation.put(
               1,(crossproduct.get(2)/0.001)*PI/180);
         }
         else if (get_ndims()==3)
         {
            curr_screenspace_rotation.put(
               2,(crossproduct.get(2)/0.001)*PI/180);
         }
      } // two_mouse_posns_detected conditional   

      osg::Quat q;
      if (curr_Graphical_ptr->get_quaternion(get_curr_t(),get_passnumber(),q))
      {
         if (get_ndims()==2)
         {
            osg::Quat screenY_rot_quat;
            screenY_rot_quat.makeRotate (-curr_screenspace_rotation.get(1),
                                         osg::Vec3f(0,1,0));
            curr_Graphical_ptr->set_quaternion(
               get_curr_t(),get_passnumber(),screenY_rot_quat*q);
         }
         else if (get_ndims()==3)
         {
            osg::Quat screenZ_rot_quat;

            screenZ_rot_quat.makeRotate(curr_screenspace_rotation.get(2),
                                        osg::Vec3f(0,0,1));
//            cout << "curr_screenspace_rotation.get(2) = "
//                 << curr_screenspace_rotation.get(2) << endl;
            
//            cout << "screenZ_rot  = " 
//                 << screenZ_rot_quat._v[0] << " , "
//                 << screenZ_rot_quat._v[1] << " , "
//                 << screenZ_rot_quat._v[2] << " , "
//                 << screenZ_rot_quat._v[3] << endl;
//            osg::Quat final_rot=screenZ_rot_quat*q;
//            cout << "final_rot = "
//                 << final_rot._v[0] << " , "
//                 << final_rot._v[1] << " , "
//                 << final_rot._v[2] << " , "
//                 << final_rot._v[3] << endl;

            curr_Graphical_ptr->set_quaternion(
               get_curr_t(),get_passnumber(),screenZ_rot_quat*q);
         }
            
// Make a note that we have manually manipulated the Graphical's
// quaternion for one particular time in one particular pass:

         curr_Graphical_ptr->set_coords_manually_manipulated(
            get_curr_t(),get_passnumber());

         osg::Quat curr_q;
         curr_Graphical_ptr->get_quaternion(
            get_curr_t(),get_passnumber(),curr_q);

//         cout << "curr_t = " << curr_t << " curr_q = "
//              << curr_q._v[0] << " , "
//              << curr_q._v[1] << " , "
//              << curr_q._v[2] << " , "
//              << curr_q._v[3] << endl;

// To expedite 2D video Graphical manipulation, we generally reset all
// Graphical orientations for images with numbers greater than or
// equal to the current one to the current quaterion..  We cease
// performing this automatic reset when a Graphical whose quaterion
// has already been manually manipulated is encountered in the loop
// over image numbers:

         for (unsigned int n=GraphicalsGroup_ptr->get_curr_framenumber()+1; 
              n <= GraphicalsGroup_ptr->get_last_framenumber(); n++)
         {

// As of 6/5/05, we simply set the time associated with each image in
// pass #0 equal to its imagenumber.  This will eventually need to be
// generalized so that the time field corresponds to a true temporal
// measurement...
            
            double future_t=static_cast<double>(n);
            if (curr_Graphical_ptr->get_coords_manually_manipulated(
               future_t,get_passnumber()))
            {
               break;
            }
            else
            {
               curr_Graphical_ptr->set_quaternion(
                  future_t,get_passnumber(),curr_q);
            }
         } // loop over index n labeling image number

         return true;
      } // get_quaternion boolean conditional
   } // currnode_ptr != NULL conditional
   return false;
}

// ==========================================================================
// Intersection methods
// ==========================================================================

bool GraphicalPickHandler::compute_intersections(float X,float Y)
{   
//   cout << "inside GraphicalPickHandler::compute_intersections(), X = " 
//        << X << " Y = " << Y << endl;

   osgUtil::Hit hit;
   if (closest_intersection_hit(X,Y,hit))
   {
      world_coords = hit.getWorldIntersectPoint();
      set_pick_handler_voxel_coords();
      return true;
   } // renorm_window_coords inside video geometry object conditional
   return false;
}

// --------------------------------------------------------------------------
// Member function closest_intersection_hit determines the world
// coordinates corresponding to input mouse location -1 <= X , Y <= +1
// in window coordinates.

bool GraphicalPickHandler::closest_intersection_hit(
   float X,float Y,osgUtil::Hit& hit)
{   
//   cout << "inside GraphicalPickHandler::closest_intersection_hit, X = " 
//        << X << " Y = " << Y << endl;
//   cout << "WindowManager_ptr = " << WindowManager_ptr << endl;

// Convert from normalized window coords -1 <= X,Y <= 1 to pixel
// coordinates pixel_x, pixel_y:

   twovector pixel_mouse_coords=WindowManager_ptr->
      convert_renormalized_to_pixel_window_coords(X,Y);
//   cout << "pixel_mouse_coords = " << pixel_mouse_coords << endl;

   osgUtil::IntersectVisitor::HitList hlist;
   if (ComputeIntersectionsFromViewerCode(pixel_mouse_coords,hlist))
   {

// On 1/19/07, we empirically found that we should take the hit
// located at the BACK rather than at the FRONT of the hlist in order
// to obtain the correct geolocation on the blue marble:

      hit = hlist.back();
//      hit = hlist.front();
//      cout << "hlist.size() = " << hlist.size() << endl;

/*
      threevector eye_posn=get_CM_3D_ptr()->get_eye_world_posn();
      for (unsigned int j=0; j<hlist.size(); j++)
      {
         threevector curr_surface_point=hlist[j].getWorldIntersectPoint();
         double surfacept_to_eye_dist=
            (curr_surface_point-eye_posn).magnitude();
         cout << "j = " << j << " surfacept_to_eye_dist = " 
              << surfacept_to_eye_dist << endl;
         cout << "curr_surface_point = " << curr_surface_point << endl;
      }
*/

      string drawable_classname=hit._drawable->className();
//      cout << "drawable_classname = " << drawable_classname << endl;
      if (drawable_classname=="Geometry")
      {
//         intersection_point = hit.getWorldIntersectPoint();
         return true;
      } // drawable_classname=="Geometry" conditional
   } // renorm_window_coords inside video geometry object conditional
   return false;
}

// --------------------------------------------------------------------------
// Member function ComputeIntersectionsFromViewerCode is a slightly
// modified version of osgProducer::Viewer::computeIntersections().
// It takes as input coordinates (pixel_x, pixel_y) which are measured
// in pixels.  These coordinates change as the size of the window is
// changed.  In contrast, normalized window coordinates -1 <= X,Y <= 1
// do not depend upon the physical size of the window.

// We incorporate this method into GraphicalPickHandler in order to
// eliminate the dependence of this class upon osgProducer::Viewer.

bool GraphicalPickHandler::ComputeIntersectionsFromViewerCode(
   const twovector& pixel_mouse_coords,
   osgUtil::IntersectVisitor::HitList& hitlist,
   osg::Node::NodeMask traversalMask)
{
//   cout << "inside GraphicalPickHandler::ComputeIntersectionsFromViewerCode()" 
//        << endl;
//   cout << "this = " << this << endl;

//   cout << "WindowManager_ptr = " << WindowManager_ptr
//        << " DataNode_ptr = " << DataNode_refptr.get() << endl;
//   cout << "pixel_mouse_coords = " << pixel_mouse_coords << endl;
   if (WindowManager_ptr==NULL || !DataNode_refptr.valid())
   {
      return false;
   }

   osg::Matrixd view = WindowManager_ptr->getViewMatrix();
   osg::Matrixd proj = WindowManager_ptr->getProjectionMatrix();
   osg::Viewport* viewport = WindowManager_ptr->getViewport_ptr();
   osg::Node* rootNode = WindowManager_ptr->getSceneData_ptr();

   unsigned int numHitsBefore = hitlist.size();
   osg::NodePathList parentNodePaths = DataNode_refptr->getParentalNodePaths(
      rootNode);

//   cout << "parentNodePaths.size() = " << parentNodePaths.size() << endl;
   for (unsigned int i=0; i<parentNodePaths.size(); ++i)
   {
      osg::NodePath& nodePath = parentNodePaths[i];

      // remove the intersection node from the nodePath as it'll
      // be accounted for in the PickVisitor traversal, so we
      // don't double account for its transform.
      if (!nodePath.empty()) nodePath.pop_back();  
            
      osg::Matrixd modelview(view);

      // modify the view matrix so that it accounts for this
      // nodePath's accumulated transform

      if (!nodePath.empty()) 
      {
         modelview.preMult(computeLocalToWorld(nodePath));
      }

      osgUtil::PickVisitor pick(
         viewport, proj, modelview, 
         pixel_mouse_coords.get(0),pixel_mouse_coords.get(1));

      pick.setTraversalMask(traversalMask);
      DataNode_refptr->accept(pick);

      // copy all the hits across to the external hits list
      for (osgUtil::PickVisitor::LineSegmentHitListMap::iterator 
             itr = pick.getSegHitList().begin();
          itr != pick.getSegHitList().end(); ++itr)
      {
         hitlist.insert(hitlist.end(),itr->second.begin(), 
                        itr->second.end());
      }
   }
        
   // return true if we now have more hits than before

//   cout << "hitlist.size() = " << hitlist.size() << endl;
   return hitlist.size() > numHitsBefore;
}
