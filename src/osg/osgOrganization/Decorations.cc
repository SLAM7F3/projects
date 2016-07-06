// ==========================================================================
// DECORATIONS class member function definitions
// ==========================================================================
// Last modified on 1/10/11; 10/9/11; 12/2/11; 1/22/16
// ==========================================================================

#include <iostream>
#include <osg/MatrixTransform>
#include <osgDB/WriteFile>
#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgAnnotators/ArmySymbolsKeyHandler.h"
#include "osg/osgAnnotators/ArmySymbolPickHandler.h"
#include "osg/osgGeometry/BoxesKeyHandler.h"
#include "osg/osgGeometry/BoxPickHandler.h"
#include "astro_geo/Clock.h"
#include "osg/osgGeometry/ConePickHandler.h"
#include "osg/osgGeometry/ConesKeyHandler.h"
#include "osg/CustomManipulator.h"
#include "postgres/database.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgOrganization/DecorationsKeyHandler.h"
#include "astro_geo/Ellipsoid_model.h"
#include "osg/osgFeatures/FeaturesKeyHandler.h"
#include "osg/osgAnnotators/GraphNodesKeyHandler.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGeometry/LineSegmentsKeyHandler.h"
#include "osg/ModeController.h"
#include "osg/osgModels/ObsFrustaKeyHandler.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgOperations/Operations.h"
#include "osg/osgGeometry/PlanesKeyHandler.h"
#include "osg/osgGeometry/PointsKeyHandler.h"
#include "osg/osgGeometry/PolygonsKeyHandler.h"
#include "osg/osgGIS/postgis_database.h"
#include "osg/osgAnnotators/PowerPointsKeyHandler.h"
#include "osg/osgGeometry/RectanglesKeyHandler.h"
#include "osg/osgAnnotators/SphereSegmentsKeyHandler.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/Transformer.h"
#include "osg/osgSceneGraph/TreeVisitor.h"
#include "osg/osgGeometry/TrianglesKeyHandler.h"
#include "osg/osgWindow/WindowManager.h"

#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Decorations::allocate_member_objects()
{
   OSGgroup_refptr=new osg::Group();
   OSGgroup_refptr->setName("Decorations");
   DecorationsKeyHandler_ptr=new DecorationsKeyHandler(
      this,ModeController_ptr);
   WindowManager_ptr->get_EventHandlers_ptr()->push_back(
      DecorationsKeyHandler_ptr);
}		       

void Decorations::initialize_member_objects()
{
   disable_keyhandlers_flag=disable_pickhandlers_flag=false;

   AlirtGridsGroup_ptr=NULL;
   ArmySymbolsGroup_ptr=NULL;
   ArmySymbolPickHandler_ptr=NULL;
   ArrowsGroup_ptr=NULL;
   BoxesGroup_ptr=NULL;
   ConesGroup_ptr=NULL;
   CylindersGroup_ptr=NULL;
   CylindersKeyHandler_ptr=NULL;
   CylinderPickHandler_ptr=NULL;
   FeaturesGroup_ptr=NULL;
   FeaturePickHandler_ptr=NULL;
   GraphNodesGroup_ptr=NULL;
   LineSegmentsGroup_ptr=NULL;
   ObsFrustaGroup_ptr=NULL;
   OBSFRUSTAGROUP_ptr=NULL;
   ObsFrustumPickHandler_ptr=NULL;
   OBSFRUSTAKeyHandler_ptr=NULL;
   OBSFRUSTUMPickHandler_ptr=NULL;
   PlanesGroup_ptr=NULL;
   PointCloudsGroup_ptr=NULL;
   PointsGroup_ptr=NULL;
   PointPickHandler_ptr=NULL;
   PolygonsGroup_ptr=NULL;   
   PolygonPickHandler_ptr=NULL;
   PolyhedraGroup_ptr=NULL;   
   PolyhedraKeyHandler_ptr=NULL;   
   PolyhedronPickHandler_ptr=NULL;   
   PyramidsGroup_ptr=NULL;
   RectanglesGroup_ptr=NULL;
   RectanglePickHandler_ptr=NULL;
   ROI_PolyhedraGroup_ptr=NULL;   
   ROI_PolyhedraKeyHandler_ptr=NULL;   
   ROI_PolyhedronPickHandler_ptr=NULL;   
   SignPostPickHandler_ptr=NULL;
   SignPostsKeyHandler_ptr=NULL;
   SphereSegmentPickHandler_ptr=NULL;
   TrianglesGroup_ptr=NULL;
}		       

Decorations::Decorations(
   WindowManager* WCC_ptr,ModeController* MC_ptr,
   osgGA::CustomManipulator* CustomManipulator_ptr)
{	
   WindowManager_ptr=WCC_ptr;
   ModeController_ptr=MC_ptr;
   initialize_member_objects();
   allocate_member_objects();

   CM_refptr=CustomManipulator_ptr;
}		      

Decorations::Decorations(
   WindowManager* WCC_ptr,ModeController* MC_ptr,
   osgGA::CustomManipulator* CustomManipulator_ptr,threevector* GO_ptr)
{	
   WindowManager_ptr=WCC_ptr;
   ModeController_ptr=MC_ptr;
   initialize_member_objects();
   allocate_member_objects();

   CM_refptr=CustomManipulator_ptr;
   grid_origin_ptr=GO_ptr;
}		      

Decorations::~Decorations()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Decorations& D)
{
   return(outstream);
}

// ==========================================================================
// Decoration instantiation member functions
// ==========================================================================

AlirtGrid* Decorations::add_AlirtGrid(int ndims,Pass* pass_ptr)
{
//   cout << "inside Decorations::add_AlirtGrid()" << endl;
   
   AlirtGridsGroup_ptr=new AlirtGridsGroup(ndims,pass_ptr);
   OSGgroup_refptr->addChild(AlirtGridsGroup_ptr->get_OSGgroup_ptr());
   AlirtGrid* grid_ptr=AlirtGridsGroup_ptr->generate_new_Grid();
   instantiate_GridKeyHandler(grid_ptr);
   return grid_ptr;
}

AlirtGrid* Decorations::add_AlirtGrid(
   int ndims,Pass* pass_ptr,
   double min_X,double max_X,double min_Y,double max_Y,double min_Z,
   bool wopillc_flag)
{
   AlirtGridsGroup_ptr=new AlirtGridsGroup(ndims,pass_ptr);
   OSGgroup_refptr->addChild(AlirtGridsGroup_ptr->get_OSGgroup_ptr());
   AlirtGrid* grid_ptr=AlirtGridsGroup_ptr->generate_new_Grid(
      min_X,max_X,min_Y,max_Y,min_Z,wopillc_flag);
   instantiate_GridKeyHandler(grid_ptr);
   return grid_ptr;
}

void Decorations::instantiate_GridKeyHandler(AlirtGrid* grid_ptr)
{
   if (!disable_keyhandlers_flag) 
   {
      GridKeyHandler* GridKeyHandler_ptr=new GridKeyHandler(
         ModeController_ptr,grid_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         GridKeyHandler_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function add_ArmySymbols instantiates an OSG group to hold
// ArmySymbol information.

ArmySymbolsGroup* Decorations::add_ArmySymbols(Pass* pass_ptr)
{
   ArmySymbolsGroup_ptr=new ArmySymbolsGroup(pass_ptr,grid_origin_ptr);
   OSGgroup_refptr->addChild(ArmySymbolsGroup_ptr->get_OSGgroup_ptr());
   ArmySymbolsGroup_ptr->set_wlhd(20,20,2,0.1);	// meters
   instantiate_ArmySymbols_key_and_pick_handlers(pass_ptr);
   return ArmySymbolsGroup_ptr;
}

ArmySymbolsGroup* Decorations::add_ArmySymbols(
   Pass* pass_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr)
{
   ArmySymbolsGroup_ptr=new ArmySymbolsGroup(
      pass_ptr,clock_ptr,EM_ptr,grid_origin_ptr);
   OSGgroup_refptr->addChild(ArmySymbolsGroup_ptr->get_OSGgroup_ptr());
   ArmySymbolsGroup_ptr->set_wlhd(20,20,2,0.1);	// meters
   instantiate_ArmySymbols_key_and_pick_handlers(pass_ptr);
   return ArmySymbolsGroup_ptr;
}

void Decorations::instantiate_ArmySymbols_key_and_pick_handlers(
   Pass* pass_ptr)
{
   if (!disable_keyhandlers_flag)
   {
      ArmySymbolsKeyHandler* ArmySymbolsKeyHandler_ptr=
         new ArmySymbolsKeyHandler(
         ArmySymbolsGroup_ptr,ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         ArmySymbolsKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      ArmySymbolPickHandler_ptr=new ArmySymbolPickHandler(
         pass_ptr,CM_refptr.get(),ArmySymbolsGroup_ptr,ModeController_ptr,
         WindowManager_ptr,grid_origin_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         ArmySymbolPickHandler_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function add_Arrows instantiates an OSG group to hold
// Arrow information.

ArrowsGroup* Decorations::add_Arrows(int ndims,Pass* pass_ptr)
{
   ArrowsGroup* ArrowsGroup_ptr=new ArrowsGroup(
      ndims,pass_ptr,grid_origin_ptr);

   OSGgroup_refptr->addChild(ArrowsGroup_ptr->get_OSGgroup_ptr());
   return ArrowsGroup_ptr;
}

// ---------------------------------------------------------------------
// Member function add_Boxes instantiates an OSG group to hold Box
// information.

BoxesGroup* Decorations::add_Boxes(Pass* pass_ptr)
{
   BoxesGroup_ptr=new BoxesGroup(pass_ptr,grid_origin_ptr);
   OSGgroup_refptr->addChild(BoxesGroup_ptr->get_OSGgroup_ptr());

// Instantiate Box key handler and pick handler:

   if (!disable_keyhandlers_flag)
   {
      BoxesKeyHandler* BoxesKeyHandler_ptr=new BoxesKeyHandler(
         BoxesGroup_ptr,ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         BoxesKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      BoxPickHandler* BoxPickHandler_ptr=new BoxPickHandler(
         pass_ptr,CM_refptr.get(),BoxesGroup_ptr,ModeController_ptr,
         WindowManager_ptr,grid_origin_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         BoxPickHandler_ptr);
   }
   
   return BoxesGroup_ptr;
}

// ---------------------------------------------------------------------
// Member function add_Cones instantiates an OSG group to hold Cone
// information.

ConesGroup* Decorations::add_Cones(Pass* pass_ptr)
{
   ConesGroup_ptr=new ConesGroup(pass_ptr,grid_origin_ptr);
   OSGgroup_refptr->addChild(ConesGroup_ptr->get_OSGgroup_ptr());

// Instantiate Cone key handler and pick handler:

   if (!disable_keyhandlers_flag)
   {
      ConesKeyHandler* ConesKeyHandler_ptr=new ConesKeyHandler(
         ConesGroup_ptr,ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         ConesKeyHandler_ptr);
   }

   if (!disable_pickhandlers_flag)
   {
      ConePickHandler* ConePickHandler_ptr=new ConePickHandler(
         pass_ptr,CM_refptr.get(),ConesGroup_ptr,ModeController_ptr,
         WindowManager_ptr,grid_origin_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         ConePickHandler_ptr);
   }
   
   return ConesGroup_ptr;
}

// ---------------------------------------------------------------------
// Member function add_Cylinders instantiates an OSG group to hold
// Cylinder information.

CylindersGroup* Decorations::add_Cylinders(
   Pass* pass_ptr,AnimationController* AC_ptr,osg::MatrixTransform* MT_ptr)
{
   osgGA::Terrain_Manipulator* CM_3D_ptr=
      dynamic_cast<osgGA::Terrain_Manipulator*>(CM_refptr.get());
   CylindersGroup_ptr=new CylindersGroup(
      pass_ptr,AC_ptr,CM_3D_ptr,grid_origin_ptr);
   if (MT_ptr==NULL)
   {
      OSGgroup_refptr->addChild(CylindersGroup_ptr->get_OSGgroup_ptr());
   }
   else
   {
      OSGgroup_refptr->addChild(MT_ptr);
      CylindersGroup_ptr->set_MatrixTransform_ptr(MT_ptr);
   }
   instantiate_cylinder_key_and_pick_handlers(pass_ptr);
   return CylindersGroup_ptr;
}

CylindersGroup* Decorations::add_Cylinders(
   Pass* pass_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
   osg::MatrixTransform* MT_ptr)
{
   CylindersGroup_ptr=new CylindersGroup(pass_ptr,clock_ptr,EM_ptr);
   if (MT_ptr==NULL)
   {
      OSGgroup_refptr->addChild(CylindersGroup_ptr->get_OSGgroup_ptr());
   }
   else
   {
      OSGgroup_refptr->addChild(MT_ptr);
      CylindersGroup_ptr->set_MatrixTransform_ptr(MT_ptr);
   }
   instantiate_cylinder_key_and_pick_handlers(pass_ptr);
   return CylindersGroup_ptr;
}

void Decorations::instantiate_cylinder_key_and_pick_handlers(Pass* pass_ptr)
{
   if (!disable_keyhandlers_flag)
   {
      CylindersKeyHandler_ptr=new CylindersKeyHandler(
         CylindersGroup_ptr,ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         CylindersKeyHandler_ptr);
   }

   if (!disable_pickhandlers_flag)
   {
      CylinderPickHandler_ptr=new CylinderPickHandler(
         pass_ptr,CM_refptr.get(),CylindersGroup_ptr,
         ModeController_ptr,WindowManager_ptr,grid_origin_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         CylinderPickHandler_ptr);
   }

   if (CylinderPickHandler_ptr != NULL && CylindersKeyHandler_ptr != NULL)
   {
      CylindersKeyHandler_ptr->set_CylinderPickHandler_ptr(
         CylinderPickHandler_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function add_Features instantiates an OSG group to hold
// user-selected feature information.

FeaturesGroup* Decorations::add_Features(
   int ndims,Pass* pass_ptr,TrianglesGroup* TrianglesGroup_ptr)
{
   FeaturesGroup_ptr=new FeaturesGroup(
      ndims,pass_ptr,CM_refptr.get(),TrianglesGroup_ptr,grid_origin_ptr);
   OSGgroup_refptr->addChild(FeaturesGroup_ptr->get_OSGgroup_ptr());

   FeaturesGroup_ptr->set_delta_move_z(0.5);	// meters
//   FeaturesGroup_ptr->set_delta_move_z(2.0);	// meters
   instantiate_feature_key_and_pick_handlers(ndims,pass_ptr);
   return FeaturesGroup_ptr;
}

FeaturesGroup* Decorations::add_Features(
   int ndims,Pass* pass_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr)
{
   FeaturesGroup_ptr=new FeaturesGroup(
      ndims,pass_ptr,CM_refptr.get(),clock_ptr,EM_ptr,grid_origin_ptr);
   OSGgroup_refptr->addChild(FeaturesGroup_ptr->get_OSGgroup_ptr());
   instantiate_feature_key_and_pick_handlers(ndims,pass_ptr);
   return FeaturesGroup_ptr;
}

FeaturesGroup* Decorations::add_Features(
   int ndims,Pass* pass_ptr,CentersGroup* CG_ptr,Movie* movie_ptr,
   TrianglesGroup* TG_ptr,LineSegmentsGroup* LSG_ptr,
   AnimationController* AC_ptr)
{
//   cout << "inside Decorations::add_Features(movie_ptr)" << endl;
   
   FeaturesGroup_ptr=new FeaturesGroup(
      ndims,pass_ptr,CG_ptr,movie_ptr,CM_refptr.get(),TG_ptr,LSG_ptr,AC_ptr);
   OSGgroup_refptr->addChild(FeaturesGroup_ptr->get_OSGgroup_ptr());
   instantiate_feature_key_and_pick_handlers(ndims,pass_ptr);
   return FeaturesGroup_ptr;
}

void Decorations::instantiate_feature_key_and_pick_handlers(
   int ndims,Pass* pass_ptr)
{
   if (!disable_keyhandlers_flag)
   {
      FeaturesKeyHandler* FeaturesKeyHandler_ptr=
         new FeaturesKeyHandler(ndims,FeaturesGroup_ptr,ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         FeaturesKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      FeaturePickHandler_ptr=new FeaturePickHandler(
         ndims,pass_ptr,CM_refptr.get(),FeaturesGroup_ptr,
         OBSFRUSTAGROUP_ptr,ModeController_ptr,
         WindowManager_ptr,grid_origin_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         FeaturePickHandler_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function add_GraphNodes instantiates an OSG group to hold
// GraphNode information.

GraphNodesGroup* Decorations::add_GraphNodes(
   Pass* pass_ptr,TreeVisitor* TV_ptr,AnimationController* AC_ptr)
{
   GraphNodesGroup_ptr=new GraphNodesGroup(
      pass_ptr,grid_origin_ptr,TV_ptr,AC_ptr);
   OSGgroup_refptr->addChild(GraphNodesGroup_ptr->get_OSGgroup_ptr());

// Instantiate GraphNode key handler:

   if (!disable_keyhandlers_flag)
   {
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         new GraphNodesKeyHandler(GraphNodesGroup_ptr,ModeController_ptr));
   }

   return GraphNodesGroup_ptr;
}

// ---------------------------------------------------------------------
// Member function add_LineSegments instantiates an OSG group to hold
// LineSegment information.

LineSegmentsGroup* Decorations::add_LineSegments(
   int ndims,Pass* pass_ptr,AnimationController* AnimationController_ptr)
{
   LineSegmentsGroup_ptr=new LineSegmentsGroup(
      ndims,pass_ptr,AnimationController_ptr);
   OSGgroup_refptr->addChild(LineSegmentsGroup_ptr->get_OSGgroup_ptr());

// Instantiate LineSegment key handler:

   if (!disable_keyhandlers_flag)
   {
      LineSegmentsKeyHandler* LineSegmentsKeyHandler_ptr=
         new LineSegmentsKeyHandler(
            LineSegmentsGroup_ptr,ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         LineSegmentsKeyHandler_ptr);
   }
   
   return LineSegmentsGroup_ptr;
}

// ---------------------------------------------------------------------
// Member function add_Models instantiates an OSG group to hold
// Model information.

ModelsGroup* Decorations::add_Models(
   Pass* pass_ptr,AnimationController* AnimationController_ptr,
   bool include_into_Decorations_OSGgroup_flag)
{
   ModelsGroup* curr_ModelsGroup_ptr=new ModelsGroup(
      pass_ptr,grid_origin_ptr,AnimationController_ptr);
   ModelsGroup_ptrs.push_back(curr_ModelsGroup_ptr);

   if (include_into_Decorations_OSGgroup_flag)
      OSGgroup_refptr->addChild(curr_ModelsGroup_ptr->get_OSGgroup_ptr());

   instantiate_model_key_and_pick_handlers(pass_ptr);
   return curr_ModelsGroup_ptr;
}

ModelsGroup* Decorations::add_Models(
   Pass* pass_ptr,PolyLinesGroup* PLG_ptr,
   AnimationController* AnimationController_ptr)
{
   ModelsGroup* curr_ModelsGroup_ptr=new ModelsGroup(
      pass_ptr,PLG_ptr,grid_origin_ptr,AnimationController_ptr);
   ModelsGroup_ptrs.push_back(curr_ModelsGroup_ptr);
   OSGgroup_refptr->addChild(curr_ModelsGroup_ptr->get_OSGgroup_ptr());

   instantiate_model_key_and_pick_handlers(pass_ptr);
   return curr_ModelsGroup_ptr;
}

// ---------------------------------------------------------------------
MODELSGROUP* Decorations::add_MODELS(
   Pass* pass_ptr,AnimationController* AnimationController_ptr,
   bool include_into_Decorations_OSGgroup_flag)
{
   MODELSGROUP* curr_MODELSGroup_ptr=new MODELSGROUP(
      pass_ptr,grid_origin_ptr,AnimationController_ptr);
   MODELSGROUP_ptrs.push_back(curr_MODELSGroup_ptr);

   if (include_into_Decorations_OSGgroup_flag)
      OSGgroup_refptr->addChild(curr_MODELSGroup_ptr->get_OSGgroup_ptr());

   instantiate_MODEL_key_and_pick_handlers(pass_ptr);
   return curr_MODELSGroup_ptr;
}

MODELSGROUP* Decorations::add_MODELS(
   Pass* pass_ptr,PolyLinesGroup* PLG_ptr,
   AnimationController* AnimationController_ptr)
{
   MODELSGROUP* curr_MODELSGroup_ptr=new MODELSGROUP(
      pass_ptr,PLG_ptr,grid_origin_ptr,AnimationController_ptr);
   MODELSGROUP_ptrs.push_back(curr_MODELSGroup_ptr);
   OSGgroup_refptr->addChild(curr_MODELSGroup_ptr->get_OSGgroup_ptr());

   instantiate_MODEL_key_and_pick_handlers(pass_ptr);
   return curr_MODELSGroup_ptr;
}

MODELSGROUP* Decorations::add_MODELS(
   Pass* pass_ptr,PolyLinesGroup* PLG_ptr,PolyLinePickHandler* PLPH_ptr,
   AnimationController* AnimationController_ptr)
{
   osgGA::Terrain_Manipulator* CM_3D_ptr=
      dynamic_cast<osgGA::Terrain_Manipulator*>(CM_refptr.get());

   MODELSGROUP* curr_MODELSGroup_ptr=new MODELSGROUP(
      pass_ptr,PLG_ptr,PLPH_ptr,grid_origin_ptr,CM_3D_ptr,
      AnimationController_ptr);
   curr_MODELSGroup_ptr->set_ModeController_ptr(ModeController_ptr);
   curr_MODELSGroup_ptr->set_WindowManager_ptr(WindowManager_ptr);

   MODELSGROUP_ptrs.push_back(curr_MODELSGroup_ptr);
   OSGgroup_refptr->addChild(curr_MODELSGroup_ptr->get_OSGgroup_ptr());

   instantiate_MODEL_key_and_pick_handlers(pass_ptr);
   return curr_MODELSGroup_ptr;
}

MODELSGROUP* Decorations::add_MODELS(
   Pass* pass_ptr,PolyLinesGroup* PLG_ptr,PolyLinePickHandler* PLPH_ptr,
   Operations* Operations_ptr)
{
   osgGA::Terrain_Manipulator* CM_3D_ptr=
      dynamic_cast<osgGA::Terrain_Manipulator*>(CM_refptr.get());

   MODELSGROUP* curr_MODELSGroup_ptr=new MODELSGROUP(
      pass_ptr,PLG_ptr,PLPH_ptr,grid_origin_ptr,CM_3D_ptr,Operations_ptr);
   curr_MODELSGroup_ptr->set_ModeController_ptr(ModeController_ptr);
   curr_MODELSGroup_ptr->set_WindowManager_ptr(WindowManager_ptr);

   MODELSGROUP_ptrs.push_back(curr_MODELSGroup_ptr);
   OSGgroup_refptr->addChild(curr_MODELSGroup_ptr->get_OSGgroup_ptr());

   instantiate_MODEL_key_and_pick_handlers(pass_ptr);
   return curr_MODELSGroup_ptr;
}

// ---------------------------------------------------------------------
void Decorations::instantiate_model_key_and_pick_handlers(Pass* pass_ptr)
{
   if (!disable_keyhandlers_flag)
   {
      ModelsKeyHandler* curr_ModelsKeyHandler_ptr=new ModelsKeyHandler(
         ModelsGroup_ptrs.back(),ModeController_ptr);
      ModelsKeyHandler_ptrs.push_back(curr_ModelsKeyHandler_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         curr_ModelsKeyHandler_ptr);
   }

   if (!disable_pickhandlers_flag)
   {
      ModelPickHandler* curr_ModelPickHandler_ptr=new ModelPickHandler(
         pass_ptr,CM_refptr.get(),ModelsGroup_ptrs.back(),ModeController_ptr,
         WindowManager_ptr,grid_origin_ptr);
      ModelPickHandler_ptrs.push_back(curr_ModelPickHandler_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         curr_ModelPickHandler_ptr);
   }
}

void Decorations::instantiate_MODEL_key_and_pick_handlers(Pass* pass_ptr)
{
   instantiate_MODEL_key_and_pick_handlers(pass_ptr,MODELSGROUP_ptrs.back());
}

void Decorations::instantiate_MODEL_key_and_pick_handlers(
   Pass* pass_ptr,MODELSGROUP* curr_MODELSGROUP_ptr)
{
   if (!disable_keyhandlers_flag)
   {
      MODELSKeyHandler* curr_MODELSKeyHandler_ptr=new MODELSKeyHandler(
         curr_MODELSGROUP_ptr,ModeController_ptr);
      MODELSKeyHandler_ptrs.push_back(curr_MODELSKeyHandler_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         curr_MODELSKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      MODELPickHandler* curr_MODELPickHandler_ptr=new MODELPickHandler(
         pass_ptr,CM_refptr.get(),curr_MODELSGROUP_ptr,ModeController_ptr,
         WindowManager_ptr,grid_origin_ptr);
      MODELPickHandler_ptrs.push_back(curr_MODELPickHandler_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         curr_MODELPickHandler_ptr);
   }
}

// ---------------------------------------------------------------------
LOSMODELSGROUP* Decorations::add_LOSMODELS(
   Pass* pass_ptr,AnimationController* AnimationController_ptr,
   bool include_into_Decorations_OSGgroup_flag)
{
//   cout << "inside Decorations::add_LOSMODELS() #1" << endl;
   LOSMODELSGROUP* curr_LOSMODELSGroup_ptr=new LOSMODELSGROUP(
      pass_ptr,grid_origin_ptr,AnimationController_ptr);
   LOSMODELSGROUP_ptrs.push_back(curr_LOSMODELSGroup_ptr);

   if (include_into_Decorations_OSGgroup_flag)
      OSGgroup_refptr->addChild(curr_LOSMODELSGroup_ptr->get_OSGgroup_ptr());

   instantiate_LOSMODEL_key_and_pick_handlers(pass_ptr);
   return curr_LOSMODELSGroup_ptr;
}

LOSMODELSGROUP* Decorations::add_LOSMODELS(
   Pass* pass_ptr,PolyLinesGroup* PLG_ptr,PolyLinePickHandler* PLPH_ptr,
   Operations* Operations_ptr)
{
//   cout << "inside Decorations::add_LOSMODELS() #2" << endl;
   osgGA::Terrain_Manipulator* CM_3D_ptr=
      dynamic_cast<osgGA::Terrain_Manipulator*>(CM_refptr.get());

   LOSMODELSGROUP* curr_LOSMODELSGroup_ptr=new LOSMODELSGROUP(
      pass_ptr,PLG_ptr,PLPH_ptr,grid_origin_ptr,CM_3D_ptr,Operations_ptr);
   curr_LOSMODELSGroup_ptr->set_ModeController_ptr(ModeController_ptr);
   curr_LOSMODELSGroup_ptr->set_WindowManager_ptr(WindowManager_ptr);

   LOSMODELSGROUP_ptrs.push_back(curr_LOSMODELSGroup_ptr);
   OSGgroup_refptr->addChild(curr_LOSMODELSGroup_ptr->get_OSGgroup_ptr());

   instantiate_LOSMODEL_key_and_pick_handlers(pass_ptr);
   return curr_LOSMODELSGroup_ptr;
}

LOSMODELSGROUP* Decorations::add_LOSMODELS(
   Pass* pass_ptr,PolyLinesGroup* PLG_ptr,PolyLinePickHandler* PLPH_ptr,
   Operations* Operations_ptr,MoviesGroup* MG_ptr)
{
//   cout << "inside Decorations::add_LOSMODELS() #3" << endl;
   
   osgGA::Terrain_Manipulator* CM_3D_ptr=
      dynamic_cast<osgGA::Terrain_Manipulator*>(CM_refptr.get());

   LOSMODELSGROUP* curr_LOSMODELSGroup_ptr=new LOSMODELSGROUP(
      pass_ptr,PLG_ptr,PLPH_ptr,grid_origin_ptr,CM_3D_ptr,Operations_ptr,
      MG_ptr);
   curr_LOSMODELSGroup_ptr->set_ModeController_ptr(ModeController_ptr);
   curr_LOSMODELSGroup_ptr->set_WindowManager_ptr(WindowManager_ptr);

   LOSMODELSGROUP_ptrs.push_back(curr_LOSMODELSGroup_ptr);
   OSGgroup_refptr->addChild(curr_LOSMODELSGroup_ptr->get_OSGgroup_ptr());

   instantiate_LOSMODEL_key_and_pick_handlers(pass_ptr);
   return curr_LOSMODELSGroup_ptr;
}

void Decorations::instantiate_LOSMODEL_key_and_pick_handlers(Pass* pass_ptr)
{
   instantiate_LOSMODEL_key_and_pick_handlers(
      pass_ptr,LOSMODELSGROUP_ptrs.back());
}

void Decorations::instantiate_LOSMODEL_key_and_pick_handlers(
   Pass* pass_ptr,LOSMODELSGROUP* curr_LOSMODELSGROUP_ptr)
{
//   cout << "inside Decorations::instantiate_LOSMODEL_key_and_pick_handlers()"
//        << endl;
   if (!disable_keyhandlers_flag)
   {
      LOSMODELSKeyHandler* curr_LOSMODELSKeyHandler_ptr=
         new LOSMODELSKeyHandler(
            curr_LOSMODELSGROUP_ptr,ModeController_ptr);
      
      LOSMODELSKeyHandler_ptrs.push_back(curr_LOSMODELSKeyHandler_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         curr_LOSMODELSKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      LOSMODELPickHandler* curr_LOSMODELPickHandler_ptr=
         new LOSMODELPickHandler(
            pass_ptr,CM_refptr.get(),curr_LOSMODELSGROUP_ptr,
            ModeController_ptr,WindowManager_ptr,grid_origin_ptr);
      LOSMODELPickHandler_ptrs.push_back(curr_LOSMODELPickHandler_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         curr_LOSMODELPickHandler_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function add_ObsFrusta instantiates an OSG group to hold
// translucent observation frusta.

ObsFrustaGroup* Decorations::add_ObsFrusta(
   Pass* pass_ptr,AnimationController* AC_ptr,
   bool include_into_Decorations_OSGgroup_flag,int n_subPAT)
{
//   cout << "inside Decorations::add_ObsFrusta()" << endl;

   ObsFrustaGroup_ptr=new ObsFrustaGroup(
      pass_ptr,CM_refptr.get(),AC_ptr,grid_origin_ptr);
   if (include_into_Decorations_OSGgroup_flag)
      OSGgroup_refptr->addChild(
         ObsFrustaGroup_ptr->get_OSGgroup_ptr());

   instantiate_ObsFrustum_key_and_pick_handlers(pass_ptr);
   return ObsFrustaGroup_ptr;
}

OBSFRUSTAGROUP* Decorations::add_OBSFRUSTA(
   Pass* pass_ptr,AnimationController* AC_ptr,
   bool include_into_Decorations_OSGgroup_flag,int n_subPAT)
{
//   cout << "inside Decorations::add_OBSFRUSTA()" << endl;
   
   OBSFRUSTAGROUP_ptr=new OBSFRUSTAGROUP(
      pass_ptr,CM_refptr.get(),AC_ptr,grid_origin_ptr);
   OBSFRUSTAGROUP_ptr->set_WindowManager_ptr(WindowManager_ptr);
   if (include_into_Decorations_OSGgroup_flag)
      OSGgroup_refptr->addChild(OBSFRUSTAGROUP_ptr->get_OSGgroup_ptr());

// Need to set OBSFRUSTAGROUP's cloud pointer to zeroth member of
// PointCloudsGroup_ptr for backprojecting pixels from one image plane
// into another:

   if (PointCloudsGroup_ptr != NULL)
   {
      OBSFRUSTAGROUP_ptr->set_PointCloudsGroup_ptr(PointCloudsGroup_ptr);
      OBSFRUSTAGROUP_ptr->set_PointCloud_ptr(
         PointCloudsGroup_ptr->get_Cloud_ptr(0));
   }
   
   instantiate_OBSFRUSTUM_key_and_pick_handlers(pass_ptr);
   
   return OBSFRUSTAGROUP_ptr;
}

void Decorations::instantiate_ObsFrustum_key_and_pick_handlers(Pass* pass_ptr)
{
   if (!disable_keyhandlers_flag)
   {
      ObsFrustaKeyHandler* ObsFrustaKeyHandler_ptr=
         new ObsFrustaKeyHandler(
            ObsFrustaGroup_ptr,ModeController_ptr,
            dynamic_cast<osgGA::Terrain_Manipulator*>(CM_refptr.get()));
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         ObsFrustaKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      ObsFrustumPickHandler_ptr=new ObsFrustumPickHandler(
         pass_ptr,CM_refptr.get(),ObsFrustaGroup_ptr,ModeController_ptr,
         WindowManager_ptr,grid_origin_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         ObsFrustumPickHandler_ptr);
   }
}

void Decorations::instantiate_OBSFRUSTUM_key_and_pick_handlers(Pass* pass_ptr)
{
//   cout << "inside Decorations::instantiate_OBSFRUSTUM_key_and_pick_handlers()"
//        << endl;
   if (!disable_keyhandlers_flag)
   {
      OBSFRUSTAKeyHandler_ptr=new OBSFRUSTAKeyHandler(
         OBSFRUSTAGROUP_ptr,ModeController_ptr,
         dynamic_cast<osgGA::Terrain_Manipulator*>(CM_refptr.get()));
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         OBSFRUSTAKeyHandler_ptr);
   }

   if (!disable_pickhandlers_flag)
   {
      OBSFRUSTUMPickHandler_ptr=new OBSFRUSTUMPickHandler(
         pass_ptr,CM_refptr.get(),OBSFRUSTAGROUP_ptr,ModeController_ptr,
         WindowManager_ptr,grid_origin_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         OBSFRUSTUMPickHandler_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function add_Planes instantiates an OSG group to hold
// Plane information.

PlanesGroup* Decorations::add_Planes(Pass* pass_ptr)
{
   PlanesGroup_ptr=new PlanesGroup(pass_ptr);
   OSGgroup_refptr->addChild(PlanesGroup_ptr->get_OSGgroup_ptr());

// Instantiate Planes key handler:

   if (!disable_keyhandlers_flag)
   {
      PlanesKeyHandler* PlanesKeyHandler_ptr=new PlanesKeyHandler(
         PlanesGroup_ptr,ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         PlanesKeyHandler_ptr);
   }
   
   return PlanesGroup_ptr;
}

// ---------------------------------------------------------------------
// Member function add_Points instantiates an OSG group to hold
// Point information.

osgGeometry::PointsGroup* Decorations::add_Points(
   int ndims,Pass* pass_ptr,AnimationController* AnimationController_ptr)
{
   PointsGroup_ptr=new osgGeometry::PointsGroup(
      ndims,pass_ptr,AnimationController_ptr);
   OSGgroup_refptr->addChild(PointsGroup_ptr->get_OSGgroup_ptr());

// Instantiate Point key handler and pick handler:

   if (!disable_keyhandlers_flag)
   {
      osgGeometry::PointsKeyHandler* PointsKeyHandler_ptr=
         new osgGeometry::PointsKeyHandler(
            ndims,PointsGroup_ptr,ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         PointsKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      PointPickHandler_ptr=new osgGeometry::PointPickHandler(
         ndims,pass_ptr,CM_refptr.get(),PointsGroup_ptr,ModeController_ptr,
         WindowManager_ptr,grid_origin_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         PointPickHandler_ptr);
   }
   
   return PointsGroup_ptr;
}

// ---------------------------------------------------------------------
// Member function add_Polygons instantiates an OSG group to hold
// Polygon information.

osgGeometry::PolygonsGroup* Decorations::add_Polygons(
   int ndims,Pass* pass_ptr,AnimationController* AnimationController_ptr)
{
   PolygonsGroup_ptr=new osgGeometry::PolygonsGroup(
      ndims,pass_ptr,AnimationController_ptr,grid_origin_ptr);
   OSGgroup_refptr->addChild(PolygonsGroup_ptr->get_OSGgroup_ptr());
   instantiate_Polygons_key_and_pick_handlers(pass_ptr);
   return PolygonsGroup_ptr;
}

void Decorations::instantiate_Polygons_key_and_pick_handlers(
   Pass* pass_ptr)
{
   if (!disable_keyhandlers_flag)
   {
      osgGeometry::PolygonsKeyHandler* PolygonsKeyHandler_ptr=
         new osgGeometry::PolygonsKeyHandler(
            PolygonsGroup_ptr,ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         PolygonsKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      PolygonPickHandler_ptr=new osgGeometry::PolygonPickHandler(
         pass_ptr,CM_refptr.get(),PolygonsGroup_ptr,ModeController_ptr,
         WindowManager_ptr,grid_origin_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         PolygonPickHandler_ptr);
   }
}
   
// ---------------------------------------------------------------------
// Member function add_PolyLines instantiates an OSG group to hold
// PolyLine information.

PolyLinesGroup* Decorations::add_PolyLines(
   int ndims,Pass* pass_ptr)
{
   return add_PolyLines(ndims, pass_ptr, NULL);
}

PolyLinesGroup* Decorations::add_PolyLines(
   int ndims,Pass* pass_ptr,AnimationController* AnimationController_ptr)
{
//   cout << "inside Decorations::add_PolyLines()" << endl;
   PolyLinesGroup* curr_PolyLinesGroup_ptr=new PolyLinesGroup(
      ndims,pass_ptr,PolygonsGroup_ptr,PolyhedraGroup_ptr,
      AnimationController_ptr, grid_origin_ptr);
   PolyLinesGroup_ptrs.push_back(curr_PolyLinesGroup_ptr);

   OSGgroup_refptr->addChild(curr_PolyLinesGroup_ptr->get_OSGgroup_ptr());
   instantiate_PolyLines_key_and_pick_handlers(ndims,pass_ptr);
   return curr_PolyLinesGroup_ptr;
}

void Decorations::instantiate_PolyLines_key_and_pick_handlers(
   int ndims,Pass* pass_ptr)
{
//   cout << "inside Decorations::instantiate_PolyLines_key_and_pick_handlers()"
//        << endl;
//   cout << "disable_keyhandlers_flag = "
//        << disable_keyhandlers_flag << endl;
//   cout << "disable_pickhandlers_flag = "
//        << disable_pickhandlers_flag << endl;

   if (!disable_keyhandlers_flag)
   {
      PolyLinesKeyHandler* curr_PolyLinesKeyHandler_ptr=
         new PolyLinesKeyHandler(
         PolyLinesGroup_ptrs.back(),ModeController_ptr);
      PolyLinesKeyHandler_ptrs.push_back(curr_PolyLinesKeyHandler_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         curr_PolyLinesKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      PolyLinePickHandler* curr_PolyLinePickHandler_ptr=
         new PolyLinePickHandler(
            ndims,pass_ptr,CM_refptr.get(),PolyLinesGroup_ptrs.back(),
            ModeController_ptr,WindowManager_ptr,grid_origin_ptr);
      PolyLinePickHandler_ptrs.push_back(curr_PolyLinePickHandler_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         curr_PolyLinePickHandler_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function add_Polyhedra instantiates an OSG group to hold
// Polyhedron information.

PolyhedraGroup* Decorations::add_Polyhedra(Pass* pass_ptr)
{
   PolyhedraGroup_ptr=new PolyhedraGroup(pass_ptr,grid_origin_ptr);
   OSGgroup_refptr->addChild(PolyhedraGroup_ptr->get_OSGgroup_ptr());
   instantiate_Polyhedra_key_and_pick_handlers(pass_ptr);
   return PolyhedraGroup_ptr;
}

void Decorations::instantiate_Polyhedra_key_and_pick_handlers(
   Pass* pass_ptr)
{
   if (!disable_keyhandlers_flag)
   {
      PolyhedraKeyHandler_ptr=new PolyhedraKeyHandler(
         PolyhedraGroup_ptr,ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         PolyhedraKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      PolyhedronPickHandler_ptr=new PolyhedronPickHandler(
         pass_ptr,CM_refptr.get(),PolyhedraGroup_ptr,
         ModeController_ptr,WindowManager_ptr,grid_origin_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         PolyhedronPickHandler_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function add_PowerPoints instantiates an OSG group to hold
// PowerPoint information.

PowerPointsGroup* Decorations::add_PowerPoints(Pass* pass_ptr)
{
   PowerPointsGroup_ptr=new PowerPointsGroup(pass_ptr,grid_origin_ptr);
   OSGgroup_refptr->addChild(PowerPointsGroup_ptr->get_OSGgroup_ptr());
   PowerPointsGroup_ptr->set_wlhd(20,20,2,0.1);	// meters
   instantiate_PowerPoints_key_and_pick_handlers(pass_ptr);
   return PowerPointsGroup_ptr;
}

PowerPointsGroup* Decorations::add_PowerPoints(
   Pass* pass_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr)
{
   PowerPointsGroup_ptr=new PowerPointsGroup(
      pass_ptr,clock_ptr,EM_ptr,grid_origin_ptr);
   OSGgroup_refptr->addChild(PowerPointsGroup_ptr->get_OSGgroup_ptr());
   PowerPointsGroup_ptr->set_wlhd(20,20,2,0.1);	// meters
   instantiate_PowerPoints_key_and_pick_handlers(pass_ptr);
   return PowerPointsGroup_ptr;
}

void Decorations::instantiate_PowerPoints_key_and_pick_handlers(
   Pass* pass_ptr)
{
   if (!disable_keyhandlers_flag)
   {
      PowerPointsKeyHandler* PowerPointsKeyHandler_ptr=
         new PowerPointsKeyHandler(
         PowerPointsGroup_ptr,ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         PowerPointsKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      PowerPointPickHandler_ptr=new PowerPointPickHandler(
         pass_ptr,CM_refptr.get(),PowerPointsGroup_ptr,ModeController_ptr,
         WindowManager_ptr,grid_origin_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         PowerPointPickHandler_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function add_Pyramids instantiates an OSG group to hold
// Pyramid information.

PyramidsGroup* Decorations::add_Pyramids(Pass* pass_ptr)
{
   PyramidsGroup_ptr=new PyramidsGroup(pass_ptr,grid_origin_ptr);
   OSGgroup_refptr->addChild(PyramidsGroup_ptr->get_OSGgroup_ptr());
   return PyramidsGroup_ptr;
}

// ---------------------------------------------------------------------
// Member function add_Rectangles instantiates an OSG group to hold
// Rectangle information.

RectanglesGroup* Decorations::add_Rectangles(int ndims,Pass* pass_ptr)
{
   RectanglesGroup_ptr=new RectanglesGroup(ndims,pass_ptr);
   OSGgroup_refptr->addChild(RectanglesGroup_ptr->get_OSGgroup_ptr());

// Instantiate Rectangle key handler and pick handler:

   if (!disable_keyhandlers_flag)
   {
      RectanglesKeyHandler* RectanglesKeyHandler_ptr=new RectanglesKeyHandler(
         RectanglesGroup_ptr,ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         RectanglesKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      RectanglePickHandler_ptr=new RectanglePickHandler(
         pass_ptr,CM_refptr.get(),RectanglesGroup_ptr,ModeController_ptr,
         WindowManager_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         RectanglePickHandler_ptr);
   }
   
   return RectanglesGroup_ptr;
}

// ---------------------------------------------------------------------
// Member function add_ROI_Polyhedra instantiates an OSG group to hold
// ROI_Polyhedron information.

ROI_PolyhedraGroup* Decorations::add_ROI_Polyhedra(Pass* pass_ptr)
{
   ROI_PolyhedraGroup_ptr=new ROI_PolyhedraGroup(pass_ptr,grid_origin_ptr);
   OSGgroup_refptr->addChild(ROI_PolyhedraGroup_ptr->get_OSGgroup_ptr());
   instantiate_ROI_Polyhedra_key_and_pick_handlers(pass_ptr);
   return ROI_PolyhedraGroup_ptr;
}

void Decorations::instantiate_ROI_Polyhedra_key_and_pick_handlers(
   Pass* pass_ptr)
{
   if (!disable_keyhandlers_flag)
   {
      ROI_PolyhedraKeyHandler_ptr=new ROI_PolyhedraKeyHandler(
         ROI_PolyhedraGroup_ptr,ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         ROI_PolyhedraKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      ROI_PolyhedronPickHandler_ptr=new ROI_PolyhedronPickHandler(
         pass_ptr,CM_refptr.get(),ROI_PolyhedraGroup_ptr,
         ModeController_ptr,WindowManager_ptr,grid_origin_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         ROI_PolyhedronPickHandler_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function add_SignPosts instantiates an OSG group to hold
// signpost information.

SignPostsGroup* Decorations::add_SignPosts(
   int n_dims,Pass* pass_ptr,osg::MatrixTransform* MT_ptr,
   postgis_database* SKS_db_ptr)
{
//   cout << "inside Decorations::add_SignPosts(#1)" << endl;

   if (PointCloudsGroup_ptr==NULL)
   {
      SignPostsGroup_ptrs.push_back(new SignPostsGroup(
         n_dims,pass_ptr,grid_origin_ptr));
   }
   else
   {
      SignPostsGroup_ptrs.push_back(new SignPostsGroup(
         pass_ptr,grid_origin_ptr,SKS_db_ptr,PointCloudsGroup_ptr));
   }
   
   if (MT_ptr==NULL)
   {
      OSGgroup_refptr->addChild(SignPostsGroup_ptrs.back()->
                                get_OSGgroup_ptr());
   }
   else
   {
      SignPostsGroup_ptrs.back()->set_MatrixTransform_ptr(MT_ptr);
      OSGgroup_refptr->addChild(SignPostsGroup_ptrs.back()->
                                get_MatrixTransform_ptr());
   }

   instantiate_signpost_key_and_pick_handlers(n_dims,pass_ptr);

   return SignPostsGroup_ptrs.back();
}

// ---------------------------------------------------------------------
SignPostsGroup* Decorations::add_SignPosts(
   Pass* pass_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
   bool include_into_Decorations_OSGgroup_flag,osg::MatrixTransform* MT_ptr)
{
   SignPostsGroup_ptrs.push_back(new SignPostsGroup(
      pass_ptr,clock_ptr,EM_ptr,grid_origin_ptr));
   
   if (include_into_Decorations_OSGgroup_flag)
   {
      if (MT_ptr==NULL)
      {
         OSGgroup_refptr->addChild(
            SignPostsGroup_ptrs.back()->get_OSGgroup_ptr());
      }
      else
      {
         SignPostsGroup_ptrs.back()->set_MatrixTransform_ptr(MT_ptr);
         OSGgroup_refptr->addChild(SignPostsGroup_ptrs.back()->
                                   get_MatrixTransform_ptr());
      }
   } // include_into_Decorations_OSGgroup_flag conditional

   instantiate_signpost_key_and_pick_handlers(3,pass_ptr);
   return SignPostsGroup_ptrs.back();
}

// ---------------------------------------------------------------------
SignPostsGroup* Decorations::add_SignPosts(
   Pass* pass_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
   postgis_database* SKS_db_ptr,PointCloudsGroup* PCG_ptr,
   bool include_into_Decorations_OSGgroup_flag,osg::MatrixTransform* MT_ptr)
{
   SignPostsGroup_ptrs.push_back(new SignPostsGroup(
      pass_ptr,clock_ptr,EM_ptr,grid_origin_ptr,SKS_db_ptr,PCG_ptr));
   
   if (include_into_Decorations_OSGgroup_flag)
   {
      if (MT_ptr==NULL)
      {
         OSGgroup_refptr->addChild(
            SignPostsGroup_ptrs.back()->get_OSGgroup_ptr());
      }
      else
      {
         SignPostsGroup_ptrs.back()->set_MatrixTransform_ptr(MT_ptr);
         OSGgroup_refptr->addChild(SignPostsGroup_ptrs.back()->
                                   get_MatrixTransform_ptr());
      }
   } // include_into_Decorations_OSGgroup_flag conditional

   instantiate_signpost_key_and_pick_handlers(3,pass_ptr);
   return SignPostsGroup_ptrs.back();
}

// ---------------------------------------------------------------------
void Decorations::instantiate_signpost_key_and_pick_handlers(
   int ndims,Pass* pass_ptr)
{
   if (!disable_keyhandlers_flag)
   {
      SignPostsKeyHandler_ptr=
         new SignPostsKeyHandler(SignPostsGroup_ptrs.back(),
                                 ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         SignPostsKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      SignPostPickHandler_ptr=new SignPostPickHandler(
         ndims,pass_ptr,CM_refptr.get(),SignPostsGroup_ptrs.back(),
         ModeController_ptr,WindowManager_ptr,grid_origin_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         SignPostPickHandler_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function add_SphereSegments instantiates an OSG group to hold
// translucent sphere segments.

SphereSegmentsGroup* Decorations::add_SphereSegments(
   Pass* pass_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,double radius,
   double az_min,double az_max,double el_min,double el_max,
   bool display_spokes_flag,bool include_blast_flag)
{
   SphereSegmentsGroup* SphereSegmentsGroup_ptr=new SphereSegmentsGroup(
      pass_ptr,clock_ptr,EM_ptr,grid_origin_ptr,
      display_spokes_flag,include_blast_flag);
   SphereSegmentsGroup_ptrs.push_back(SphereSegmentsGroup_ptr);
   OSGgroup_refptr->addChild(SphereSegmentsGroup_ptr->get_OSGgroup_ptr());

// Instantiate spheresegment key handler and pick handler:
   
   if (!disable_keyhandlers_flag)
   {
      SphereSegmentsKeyHandler* SphereSegmentsKeyHandler_ptr=
         new SphereSegmentsKeyHandler(
            SphereSegmentsGroup_ptr,ModeController_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         SphereSegmentsKeyHandler_ptr);
   }

   if (!disable_pickhandlers_flag)
   {
      SphereSegmentPickHandler_ptr=new SphereSegmentPickHandler(
         pass_ptr,CM_refptr.get(),SphereSegmentsGroup_ptr,
         ModeController_ptr,WindowManager_ptr,grid_origin_ptr,
         radius,az_min,az_max,el_min,el_max,
         display_spokes_flag,include_blast_flag);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         SphereSegmentPickHandler_ptr);
   }
   
   return SphereSegmentsGroup_ptr;
}

// ---------------------------------------------------------------------
// Member function add_Triangles instantiates an OSG group to hold
// user-selected triangle information.

TrianglesGroup* Decorations::add_Triangles(int ndims,Pass* pass_ptr)
{
   TrianglesGroup_ptr=new TrianglesGroup(ndims,pass_ptr);
   OSGgroup_refptr->addChild(TrianglesGroup_ptr->get_OSGgroup_ptr());

// Instantiate Triangle key handler and pick handler:

   if (!disable_keyhandlers_flag)
   {
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         new TrianglesKeyHandler(TrianglesGroup_ptr,ModeController_ptr));
   }
   
//   TrianglePickHandler* TrianglePickHandler_ptr=
//      new TrianglePickHandler(
//         ndims,pass_ptr,CM_refptr.get(),TrianglesGroup_ptr,ModeController_ptr,WindowManager_ptr);
//   WindowManager_ptr->get_EventHandlers_ptr()->push_back(TrianglePickHandler_ptr);

   return TrianglesGroup_ptr;
}

// ---------------------------------------------------------------------
void Decorations::set_DataNode_ptr(osg::Node* datanode_ptr)
{
//   cout << "inside Decorations::set_DataNode_ptr(), datanode_ptr = " 
//        << datanode_ptr << endl;
   
   if (datanode_ptr != NULL)
   {
      if (ArmySymbolPickHandler_ptr != NULL)
      {
         ArmySymbolPickHandler_ptr->set_DataNode_ptr(datanode_ptr);
      }

      if (CylinderPickHandler_ptr != NULL)
      {
         CylinderPickHandler_ptr->set_DataNode_ptr(datanode_ptr);
      }

      if (FeaturePickHandler_ptr != NULL)
      {
         FeaturePickHandler_ptr->set_DataNode_ptr(datanode_ptr);
      }

      for (int i=0; i<int(ModelPickHandler_ptrs.size()); i++)
      {
         ModelPickHandler_ptrs[i]->set_DataNode_ptr(datanode_ptr);
      }

      for (int i=0; i<int(MODELPickHandler_ptrs.size()); i++)
      {
         MODELPickHandler_ptrs[i]->set_DataNode_ptr(datanode_ptr);
      }

      if (PointPickHandler_ptr != NULL)
      {
         PointPickHandler_ptr->set_DataNode_ptr(datanode_ptr);
      }

      for (int i=0; i<int(PolyLinePickHandler_ptrs.size()); i++)
      {
         PolyLinePickHandler_ptrs[i]->set_DataNode_ptr(datanode_ptr);
      }

      if (RectanglePickHandler_ptr != NULL)
      {
         RectanglePickHandler_ptr->set_DataNode_ptr(datanode_ptr);
      }

      if (SphereSegmentPickHandler_ptr != NULL)
      {
         SphereSegmentPickHandler_ptr->set_DataNode_ptr(datanode_ptr);
      }

      if (SignPostPickHandler_ptr != NULL)
      {
         SignPostPickHandler_ptr->set_DataNode_ptr(datanode_ptr);
      }
   } // datanode_ptr != NULL conditional
}

// ==========================================================================
// File output member functions
// ==========================================================================

void Decorations::write_IVE_file(string output_filename,string subdir)
{
   outputfunc::write_banner("Writing Decorations IVE file:");

   ofstream binary_outstream;
   filefunc::dircreate(subdir);
   output_filename=subdir+output_filename+".ive";
   filefunc::deletefile(output_filename);

   if ( osgDB::writeNodeFile( *(OSGgroup_refptr.get()), output_filename) )
      osg::notify(osg::NOTICE) << "Wrote .ive file: " 
                               << output_filename << "\n";
   else
      osg::notify(osg::WARN) << "Could not write .ive file.\n";
}

// ---------------------------------------------------------------------
// Member function add_RegionPolyLines instantiates an OSG group to
// hold RegionPolyLine information.

RegionPolyLinesGroup* Decorations::add_RegionPolyLines(
   int ndims,Pass* pass_ptr, AnimationController* AC_ptr)
{
//   cout << "inside Decorations::add_RegionPolyLines()" << endl;
   RegionPolyLinesGroup* curr_RegionPolyLinesGroup_ptr=
      new RegionPolyLinesGroup(ndims,pass_ptr, AC_ptr, grid_origin_ptr);
   RegionPolyLinesGroup_ptrs.push_back(curr_RegionPolyLinesGroup_ptr);

   OSGgroup_refptr->addChild(
      curr_RegionPolyLinesGroup_ptr->get_OSGgroup_ptr());
   instantiate_RegionPolyLines_key_and_pick_handlers(ndims,pass_ptr);
   return curr_RegionPolyLinesGroup_ptr;
}

void Decorations::instantiate_RegionPolyLines_key_and_pick_handlers(
   int ndims,Pass* pass_ptr)
{
   if (!disable_keyhandlers_flag)
   {
      RegionPolyLinesKeyHandler* curr_RegionPolyLinesKeyHandler_ptr=
         new RegionPolyLinesKeyHandler(
         RegionPolyLinesGroup_ptrs.back(),ModeController_ptr);
      RegionPolyLinesKeyHandler_ptrs.push_back(
         curr_RegionPolyLinesKeyHandler_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         curr_RegionPolyLinesKeyHandler_ptr);
   }
   
   if (!disable_pickhandlers_flag)
   {
      RegionPolyLinePickHandler* curr_RegionPolyLinePickHandler_ptr=
         new RegionPolyLinePickHandler(
            pass_ptr,CM_refptr.get(),RegionPolyLinesGroup_ptrs.back(),
            ModeController_ptr,WindowManager_ptr,grid_origin_ptr);
      RegionPolyLinePickHandler_ptrs.push_back(
         curr_RegionPolyLinePickHandler_ptr);
      WindowManager_ptr->get_EventHandlers_ptr()->push_back(
         curr_RegionPolyLinePickHandler_ptr);
   }
}



