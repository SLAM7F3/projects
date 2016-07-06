// ==========================================================================
// Header file for DECORATIONS class
// ==========================================================================
// Last modified on 12/21/09; 1/9/11; 4/9/11; 1/22/16
// ==========================================================================

#ifndef DECORATIONS_H
#define DECORATIONS_H

#include <string>
#include <vector>
#include <osg/Group>
#include <osgText/Font>

#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osgAnnotators/ArmySymbolsGroup.h"
#include "osg/osgAnnotators/ArmySymbolPickHandler.h"
#include "osg/osgGeometry/ArrowsGroup.h"
#include "osg/osgGeometry/BoxesGroup.h"
#include "osg/osgGeometry/ConesGroup.h"
#include "osg/osgGeometry/CylindersGroup.h"
#include "osg/osgGeometry/CylindersKeyHandler.h"
#include "osg/osgGeometry/CylinderPickHandler.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "osg/osgFeatures/FeaturePickHandler.h"
#include "osg/osgAnnotators/GraphNodesGroup.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"

#include "osg/osgModels/LOSMODELSGROUP.h"
#include "osg/osgModels/LOSMODELSKeyHandler.h"
#include "osg/osgModels/LOSMODELPickHandler.h"

#include "osg/osgModels/ModelsGroup.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osgModels/ModelsKeyHandler.h"
#include "osg/osgModels/MODELSKeyHandler.h"
#include "osg/osgModels/ModelPickHandler.h"
#include "osg/osgModels/MODELPickHandler.h"
#include "osg/osgModels/ObsFrustaGroup.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osgModels/ObsFrustumPickHandler.h"
#include "osg/osgModels/OBSFRUSTAKeyHandler.h"
#include "osg/osgModels/OBSFRUSTUMPickHandler.h"
#include "osg/osgGeometry/PlanesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "osg/osgGeometry/PointPickHandler.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolygonPickHandler.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyhedraKeyHandler.h"
#include "osg/osgGeometry/PolyhedronPickHandler.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGeometry/PolyLinesKeyHandler.h"
#include "osg/osgGeometry/PolyLinePickHandler.h"
#include "osg/osgAnnotators/PowerPointsGroup.h"
#include "osg/osgAnnotators/PowerPointPickHandler.h"
#include "osg/osgGeometry/PyramidsGroup.h"
#include "osg/osgGeometry/RectanglesGroup.h"
#include "osg/osgGeometry/RectanglePickHandler.h"
#include "osg/osgRegions/RegionPolyLinesGroup.h"
#include "osg/osgRegions/RegionPolyLinesKeyHandler.h"
#include "osg/osgRegions/RegionPolyLinePickHandler.h"
#include "osg/osgRTPS/ROI_PolyhedraGroup.h"
#include "osg/osgRTPS/ROI_PolyhedraKeyHandler.h"
#include "osg/osgRTPS/ROI_PolyhedronPickHandler.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "osg/osgAnnotators/SignPostsKeyHandler.h"
#include "osg/osgAnnotators/SignPostPickHandler.h"
#include "osg/osgAnnotators/SphereSegmentsGroup.h"
#include "osg/osgAnnotators/SphereSegmentPickHandler.h"
#include "osg/osgGeometry/TrianglesGroup.h"

class AlirtGrid;
class AnimationController;
class Clock;
class DecorationsKeyHandler;
class Ellipsoidal_model;
class ModeController;
class MoviesGroup;
class Pass;
class postgis_database;
class threevector;
class TreeVisitor;
class WindowManager;

class Decorations 
{

  public:

// Initialization, constructor and destructor functions:

   Decorations(WindowManager* WCC_ptr,ModeController* MC_ptr,
               osgGA::CustomManipulator* CustomManipulator_ptr);
   Decorations(WindowManager* WCC_ptr,ModeController* MC_ptr,
               osgGA::CustomManipulator* CustomManipulator_ptr,
               threevector* GO_ptr);
   virtual ~Decorations();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Decorations& D);

// Set & get methods:

   void set_disable_keyhandlers_flag(bool flag);
   void set_disable_pickhandlers_flag(bool flag);

   osg::Group* get_OSGgroup_ptr();
   const osg::Group* get_OSGgroup_ptr() const;
   void set_grid_origin_ptr(threevector* GO_ptr);

   AlirtGridsGroup* get_AlirtGridsGroup_ptr();
   const AlirtGridsGroup* get_AlirtGridsGroup_ptr() const;

   ArmySymbolsGroup* get_ArmySymbolsGroup_ptr();
   const ArmySymbolsGroup* get_ArmySymbolsGroup_ptr() const;
   ArmySymbolPickHandler* get_ArmySymbolPickHandler_ptr();
   const ArmySymbolPickHandler* get_ArmySymbolPickHandler_ptr() const;

   ArrowsGroup* get_ArrowsGroup_ptr();
   const ArrowsGroup* get_ArrowsGroup_ptr() const;
   BoxesGroup* get_BoxesGroup_ptr();
   const BoxesGroup* get_BoxesGroup_ptr() const;

   CylindersGroup* get_CylindersGroup_ptr();
   const CylindersGroup* get_CylindersGroup_ptr() const;
   CylinderPickHandler* get_CylinderPickHandler_ptr();
   const CylinderPickHandler* get_CylinderPickHandler_ptr() const;
   CylindersKeyHandler* get_CylindersKeyHandler_ptr();
   const CylindersKeyHandler* get_CylindersKeyHandler_ptr() const;

   ConesGroup* get_ConesGroup_ptr();
   const ConesGroup* get_ConesGroup_ptr() const;

   FeaturesGroup* get_FeaturesGroup_ptr();
   const FeaturesGroup* get_FeaturesGroup_ptr() const;
   FeaturePickHandler* get_FeaturePickHandler_ptr();
   const FeaturePickHandler* get_FeaturePickHandler_ptr() const;

   GraphNodesGroup* get_GraphNodesGroup_ptr();
   const GraphNodesGroup* get_GraphNodesGroup_ptr() const;

   LineSegmentsGroup* get_LineSegmentsGroup_ptr();
   const LineSegmentsGroup* get_LineSegmentsGroup_ptr() const;

   int get_n_LOSMODELSGROUPs() const;
   LOSMODELSGROUP* get_LOSMODELSGROUP_ptr(int n=0);
   const LOSMODELSGROUP* get_LOSMODELSGROUP_ptr(int n=0) const;
   LOSMODELSKeyHandler* get_LOSMODELSKeyHandler_ptr(int n=0);
   const LOSMODELSKeyHandler* get_LOSMODELSKeyHandler_ptr(int n=0) const;
   LOSMODELPickHandler* get_LOSMODELPickHandler_ptr(int n=0);
   const LOSMODELPickHandler* get_LOSMODELPickHandler_ptr(int n=0) const;

   int get_n_ModelsGroups() const;
   int get_n_MODELSGROUPs() const;
   ModelsGroup* get_ModelsGroup_ptr(int n=0);
   const ModelsGroup* get_ModelsGroup_ptr(int n=0) const;
   MODELSGROUP* get_MODELSGROUP_ptr(int n=0);
   const MODELSGROUP* get_MODELSGROUP_ptr(int n=0) const;
   
   ModelsKeyHandler* get_ModelsKeyHandler_ptr(int n=0);
   const ModelsKeyHandler* get_ModelsKeyHandler_ptr(int n=0) const;
   MODELSKeyHandler* get_MODELSKeyHandler_ptr(int n=0);
   const MODELSKeyHandler* get_MODELSKeyHandler_ptr(int n=0) const;

   ModelPickHandler* get_ModelPickHandler_ptr(int n=0);
   const ModelPickHandler* get_ModelPickHandler_ptr(int n=0) const;
   MODELPickHandler* get_MODELPickHandler_ptr(int n=0);
   const MODELPickHandler* get_MODELPickHandler_ptr(int n=0) const;
   MODELPickHandler* get_last_MODELPickHandler_ptr();
   const MODELPickHandler* get_last_MODELPickHandler_ptr() const;

   ObsFrustaGroup* get_ObsFrustaGroup_ptr();
   const ObsFrustaGroup* get_ObsFrustaGroup_ptr() const;
   OBSFRUSTAGROUP* get_OBSFRUSTAGROUP_ptr();
   const OBSFRUSTAGROUP* get_OBSFRUSTAGROUP_ptr() const;
   ObsFrustumPickHandler* get_ObsFrustumPickHandler_ptr();
   const ObsFrustumPickHandler* get_ObsFrustumPickHandler_ptr() const;
   OBSFRUSTUMPickHandler* get_OBSFRUSTUMPickHandler_ptr();
   const OBSFRUSTUMPickHandler* get_OBSFRUSTUMPickHandler_ptr() const;
   OBSFRUSTAKeyHandler* get_OBSFRUSTAKeyHandler_ptr();
   const OBSFRUSTAKeyHandler* get_OBSFRUSTAKeyHandler_ptr() const;

   PlanesGroup* get_PlanesGroup_ptr();
   const PlanesGroup* get_PlanesGroup_ptr() const;

   osgGeometry::PointsGroup* get_PointsGroup_ptr();
   const osgGeometry::PointsGroup* get_PointsGroup_ptr() const;
   osgGeometry::PointPickHandler* get_PointPickHandler_ptr();
   const osgGeometry::PointPickHandler* get_PointPickHandler_ptr() const;

   int get_n_PolyLinesGroups() const;
   PolyLinesGroup* get_PolyLinesGroup_ptr(int n=0);
   const PolyLinesGroup* get_PolyLinesGroup_ptr(int n=0) const;
   PolyLinePickHandler* get_PolyLinePickHandler_ptr(int n=0);
   const PolyLinePickHandler* get_PolyLinePickHandler_ptr(int n=0) const;
   PolyLinesKeyHandler* get_PolyLinesKeyHandler_ptr(int n=0);
   const PolyLinesKeyHandler* get_PolyLinesKeyHandler_ptr(int n=0) const;

   PowerPointsGroup* get_PowerPointsGroup_ptr();
   const PowerPointsGroup* get_PowerPointsGroup_ptr() const;
   PowerPointPickHandler* get_PowerPointPickHandler_ptr();
   const PowerPointPickHandler* get_PowerPointPickHandler_ptr() const;

   osgGeometry::PolygonsGroup* get_PolygonsGroup_ptr();
   const osgGeometry::PolygonsGroup* get_PolygonsGroup_ptr() const;
   osgGeometry::PolygonPickHandler* get_PolygonPickHandler_ptr();
   const osgGeometry::PolygonPickHandler* get_PolygonPickHandler_ptr() const;

   PolyhedraGroup* get_PolyhedraGroup_ptr();
   const PolyhedraGroup* get_PolyhedraGroup_ptr() const;
   PolyhedraKeyHandler* get_PolyhedraKeyHandler_ptr();
   const PolyhedraKeyHandler* get_PolyhedraKeyHandler_ptr() const;
   PolyhedronPickHandler* get_PolyhedronPickHandler_ptr();
   const PolyhedronPickHandler* get_PolyhedronPickHandler_ptr() const;

   PyramidsGroup* get_PyramidsGroup_ptr();
   const PyramidsGroup* get_PyramidsGroup_ptr() const;
   
   RectanglesGroup* get_RectanglesGroup_ptr();
   const RectanglesGroup* get_RectanglesGroup_ptr() const;
   RectanglePickHandler* get_RectanglePickHandler_ptr();
   const RectanglePickHandler* get_RectanglePickHandler_ptr() const;

   int get_n_RegionPolyLinesGroups() const;
   RegionPolyLinesGroup* get_RegionPolyLinesGroup_ptr(int n=0);
   const RegionPolyLinesGroup* get_RegionPolyLinesGroup_ptr(int n=0) const;
   RegionPolyLinePickHandler* get_RegionPolyLinePickHandler_ptr(int n=0);
   const RegionPolyLinePickHandler* get_RegionPolyLinePickHandler_ptr(
      int n=0) const;
   RegionPolyLinesKeyHandler* get_RegionPolyLinesKeyHandler_ptr(int n=0);
   const RegionPolyLinesKeyHandler* get_RegionPolyLinesKeyHandler_ptr(
      int n=0) const;

   ROI_PolyhedraGroup* get_ROI_PolyhedraGroup_ptr();
   const ROI_PolyhedraGroup* get_ROI_PolyhedraGroup_ptr() const;
   ROI_PolyhedraKeyHandler* get_ROI_PolyhedraKeyHandler_ptr();
   const ROI_PolyhedraKeyHandler* get_ROI_PolyhedraKeyHandler_ptr() const;
   ROI_PolyhedronPickHandler* get_ROI_PolyhedronPickHandler_ptr();
   const ROI_PolyhedronPickHandler* get_ROI_PolyhedronPickHandler_ptr() const;

   int get_n_SignPostsGroups() const;
   SignPostsGroup* get_SignPostsGroup_ptr(int n);
   const SignPostsGroup* get_SignPostsGroup_ptr(int n) const;
   SignPostsKeyHandler* get_SignPostsKeyHandler_ptr();
   const SignPostsKeyHandler* get_SignPostsKeyHandler_ptr() const;
   SignPostPickHandler* get_SignPostPickHandler_ptr();
   const SignPostPickHandler* get_SignPostPickHandler_ptr() const;

   SphereSegmentsGroup* get_SphereSegmentsGroup_ptr(int n);
   const SphereSegmentsGroup* get_SphereSegmentsGroup_ptr(int n) const;
   SphereSegmentPickHandler* get_SphereSegmentPickHandler_ptr();
   const SphereSegmentPickHandler* get_SphereSegmentPickHandler_ptr() const;

   TrianglesGroup* get_TrianglesGroup_ptr();
   const TrianglesGroup* get_TrianglesGroup_ptr() const;

   void set_PointCloudsGroup_ptr(PointCloudsGroup* PCG_ptr);

// Decoration instantiation member functions

   AlirtGrid* add_AlirtGrid(int ndims,Pass* pass_ptr);
   AlirtGrid* add_AlirtGrid(
      int ndims,Pass* pass_ptr,
      double min_X,double max_X,double min_Y,double max_Y,double min_Z,
      bool wopillc_flag=false);
   ArmySymbolsGroup* add_ArmySymbols(Pass* pass_ptr);
   ArmySymbolsGroup* add_ArmySymbols(
      Pass* pass_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr);

   ArrowsGroup* add_Arrows(int ndims,Pass* pass_ptr);
   BoxesGroup* add_Boxes(Pass* pass_ptr);

   ConesGroup* add_Cones(Pass* pass_ptr);
   CylindersGroup* add_Cylinders(
      Pass* pass_ptr,AnimationController* AC_ptr,
      osg::MatrixTransform* MT_ptr=NULL);
   CylindersGroup* add_Cylinders(
      Pass* pass_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
      osg::MatrixTransform* MT_ptr=NULL);

   FeaturesGroup* add_Features(
      int ndims,Pass* pass_ptr,TrianglesGroup* TrianglesGroup_ptr=NULL);
   FeaturesGroup* add_Features(
      int ndims,Pass* pass_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr);
   FeaturesGroup* add_Features(
      int ndims,Pass* pass_ptr,CentersGroup* CG_ptr,Movie* movie_ptr,
      TrianglesGroup* TG_ptr,LineSegmentsGroup* LSG_ptr=NULL,
      AnimationController* AC_ptr=NULL);

   GraphNodesGroup* add_GraphNodes(
      Pass* pass_ptr,TreeVisitor* TV_ptr,AnimationController* AC_ptr);

   LineSegmentsGroup* add_LineSegments(
      int ndims,Pass* pass_ptr,AnimationController* AC_ptr=NULL);

   ModelsGroup* add_Models(
      Pass* pass_ptr,AnimationController* AnimationController_ptr=NULL,
      bool include_into_Decorations_OSGgroup_flag=true);
   ModelsGroup* add_Models(
      Pass* pass_ptr,PolyLinesGroup* PLG_ptr,
      AnimationController* AnimationController_ptr);

   MODELSGROUP* add_MODELS(
      Pass* pass_ptr,AnimationController* AnimationController_ptr=NULL,
      bool include_into_Decorations_OSGgroup_flag=true);
   MODELSGROUP* add_MODELS(
      Pass* pass_ptr,PolyLinesGroup* PLG_ptr,
      AnimationController* AnimationController_ptr);
   MODELSGROUP* add_MODELS(
      Pass* pass_ptr,PolyLinesGroup* PLG_ptr,PolyLinePickHandler* PLPH_ptr,
      AnimationController* AnimationController_ptr);
   MODELSGROUP* add_MODELS(
      Pass* pass_ptr,PolyLinesGroup* PLG_ptr,PolyLinePickHandler* PLPH_ptr,
      Operations* Operations_ptr);

   LOSMODELSGROUP* add_LOSMODELS(
      Pass* pass_ptr,AnimationController* AnimationController_ptr,
      bool include_into_Decorations_OSGgroup_flag=true);
   LOSMODELSGROUP* add_LOSMODELS(
      Pass* pass_ptr,PolyLinesGroup* PLG_ptr,PolyLinePickHandler* PLPH_ptr,
      Operations* Operations_ptr);
   LOSMODELSGROUP* add_LOSMODELS(
      Pass* pass_ptr,PolyLinesGroup* PLG_ptr,PolyLinePickHandler* PLPH_ptr,
      Operations* Operations_ptr,MoviesGroup* MG_ptr);

   ObsFrustaGroup* add_ObsFrusta(
      Pass* pass_ptr,AnimationController* AC_ptr,
      bool include_into_Decorations_OSGgroup_flag=true,int n_subPAT=0);
   OBSFRUSTAGROUP* add_OBSFRUSTA(
      Pass* pass_ptr,AnimationController* AC_ptr,
      bool include_into_Decorations_OSGgroup_flag=true,int n_subPAT=0);

   PlanesGroup* add_Planes(Pass* pass_ptr);
   osgGeometry::PointsGroup* add_Points(
      int ndims,Pass* pass_ptr,AnimationController* AnimationController_ptr);
   osgGeometry::PolygonsGroup* add_Polygons(
      int ndims,Pass* pass_ptr,AnimationController* AnimationController_ptr);

   PolyLinesGroup* add_PolyLines(int ndims,Pass* pass_ptr);
   PolyLinesGroup* add_PolyLines(int ndims,Pass* pass_ptr,
                                 AnimationController* AnimationController_ptr);

   PolyhedraGroup* add_Polyhedra(Pass* pass_ptr);
   PowerPointsGroup* add_PowerPoints(Pass* pass_ptr);
   PowerPointsGroup* add_PowerPoints(
      Pass* pass_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr);
   PyramidsGroup* add_Pyramids(Pass* pass_ptr);

   RectanglesGroup* add_Rectangles(int ndims,Pass* pass_ptr);
   ROI_PolyhedraGroup* add_ROI_Polyhedra(Pass* pass_ptr);

   RegionPolyLinesGroup* add_RegionPolyLines(int ndims,Pass* pass_ptr, 
                                             AnimationController* AC_ptr);
   void instantiate_RegionPolyLines_key_and_pick_handlers(
      int ndims,Pass* pass_ptr);

   SignPostsGroup* add_SignPosts(
      int ndims,Pass* pass_ptr,
      osg::MatrixTransform* MT_ptr=NULL,postgis_database* SKS_db_ptr=NULL);
   SignPostsGroup* add_SignPosts(
      Pass* pass_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
      bool include_into_Decorations_OSGgroup_flag=true,
      osg::MatrixTransform* MT_ptr=NULL);
   SignPostsGroup* add_SignPosts(
      Pass* pass_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
      postgis_database* SKS_db_ptr,PointCloudsGroup* PCG_ptr,
      bool include_into_Decorations_OSGgroup_flag=true,
      osg::MatrixTransform* MT_ptr=NULL);
   SphereSegmentsGroup* add_SphereSegments(
      Pass* pass_ptr,Clock* clock_ptr=NULL,
      Ellipsoid_model* EM_ptr=NULL,double radius=100,
      double az_min=0,double az_max=2*PI,double el_min=0,double el_max=PI/2,
      bool display_spokes_flag=false,bool include_blast_flag=false);

   TrianglesGroup* add_Triangles(int ndims,Pass* pass_ptr);

   void set_DataNode_ptr(osg::Node* datanode_ptr);

// File output member functions:

   void write_IVE_file(
      std::string output_filename="output",std::string subdir="./IVE/");

  protected:

  private:

   bool disable_keyhandlers_flag,disable_pickhandlers_flag;

   osg::ref_ptr<osg::Group> OSGgroup_refptr;
   osg::ref_ptr<osgGA::CustomManipulator> CM_refptr;

   threevector* grid_origin_ptr;
   ModeController* ModeController_ptr;
   WindowManager* WindowManager_ptr;
   DecorationsKeyHandler* DecorationsKeyHandler_ptr;

   AlirtGridsGroup* AlirtGridsGroup_ptr;
   ArmySymbolsGroup* ArmySymbolsGroup_ptr;
   ArmySymbolPickHandler* ArmySymbolPickHandler_ptr;
   ArrowsGroup* ArrowsGroup_ptr;
   BoxesGroup* BoxesGroup_ptr;
   ConesGroup* ConesGroup_ptr;
   CylindersGroup* CylindersGroup_ptr;
   CylinderPickHandler* CylinderPickHandler_ptr;
   CylindersKeyHandler* CylindersKeyHandler_ptr;
   FeaturesGroup* FeaturesGroup_ptr;
   FeaturePickHandler* FeaturePickHandler_ptr;
   GraphNodesGroup* GraphNodesGroup_ptr;
   LineSegmentsGroup* LineSegmentsGroup_ptr;
   ObsFrustaGroup* ObsFrustaGroup_ptr;   
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;   
   OBSFRUSTAKeyHandler* OBSFRUSTAKeyHandler_ptr;
   ObsFrustumPickHandler* ObsFrustumPickHandler_ptr;
   OBSFRUSTUMPickHandler* OBSFRUSTUMPickHandler_ptr;

   std::vector<LOSMODELSGROUP*> LOSMODELSGROUP_ptrs;
   std::vector<LOSMODELSKeyHandler*> LOSMODELSKeyHandler_ptrs;
   std::vector<LOSMODELPickHandler*> LOSMODELPickHandler_ptrs;

   std::vector<ModelsGroup*> ModelsGroup_ptrs;
   std::vector<MODELSGROUP*> MODELSGROUP_ptrs;
   std::vector<ModelsKeyHandler*> ModelsKeyHandler_ptrs;
   std::vector<MODELSKeyHandler*> MODELSKeyHandler_ptrs;
   std::vector<ModelPickHandler*> ModelPickHandler_ptrs;
   std::vector<MODELPickHandler*> MODELPickHandler_ptrs;

   PlanesGroup* PlanesGroup_ptr;
   PointCloudsGroup* PointCloudsGroup_ptr;
   osgGeometry::PointsGroup* PointsGroup_ptr;
   osgGeometry::PointPickHandler* PointPickHandler_ptr;
   osgGeometry::PolygonsGroup* PolygonsGroup_ptr;
   osgGeometry::PolygonPickHandler* PolygonPickHandler_ptr;
   PolyhedraGroup* PolyhedraGroup_ptr;
   PolyhedraKeyHandler* PolyhedraKeyHandler_ptr;
   PolyhedronPickHandler* PolyhedronPickHandler_ptr;
   std::vector<PolyLinesKeyHandler*> PolyLinesKeyHandler_ptrs;
   std::vector<PolyLinesGroup*> PolyLinesGroup_ptrs;
   std::vector<PolyLinePickHandler*> PolyLinePickHandler_ptrs;
   PowerPointsGroup* PowerPointsGroup_ptr;
   PowerPointPickHandler* PowerPointPickHandler_ptr;
   PyramidsGroup* PyramidsGroup_ptr;

   RectanglesGroup* RectanglesGroup_ptr;
   RectanglePickHandler* RectanglePickHandler_ptr;
   std::vector<RegionPolyLinesKeyHandler*> RegionPolyLinesKeyHandler_ptrs;
   std::vector<RegionPolyLinesGroup*> RegionPolyLinesGroup_ptrs;
   std::vector<RegionPolyLinePickHandler*> RegionPolyLinePickHandler_ptrs;
   ROI_PolyhedraGroup* ROI_PolyhedraGroup_ptr;
   ROI_PolyhedraKeyHandler* ROI_PolyhedraKeyHandler_ptr;
   ROI_PolyhedronPickHandler* ROI_PolyhedronPickHandler_ptr;

   std::vector<SignPostsGroup*> SignPostsGroup_ptrs;
   SignPostsKeyHandler* SignPostsKeyHandler_ptr;
   SignPostPickHandler* SignPostPickHandler_ptr;
   std::vector<SphereSegmentsGroup*> SphereSegmentsGroup_ptrs;
   SphereSegmentPickHandler* SphereSegmentPickHandler_ptr;
   TrianglesGroup* TrianglesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Decorations& D);

   void instantiate_GridKeyHandler(AlirtGrid* grid_ptr);
   void instantiate_ArmySymbols_key_and_pick_handlers(Pass* pass_ptr);
   void instantiate_cylinder_key_and_pick_handlers(Pass* pass_ptr);
   void instantiate_feature_key_and_pick_handlers(int ndims,Pass* pass_ptr);
   void instantiate_model_key_and_pick_handlers(Pass* pass_ptr);
   void instantiate_MODEL_key_and_pick_handlers(Pass* pass_ptr);
   void instantiate_MODEL_key_and_pick_handlers(
      Pass* pass_ptr,MODELSGROUP* curr_MODELSGROUP_ptr);

   void instantiate_LOSMODEL_key_and_pick_handlers(Pass* pass_ptr);
   void instantiate_LOSMODEL_key_and_pick_handlers(
      Pass* pass_ptr,LOSMODELSGROUP* curr_LOSMODELSGROUP_ptr);

   void instantiate_ObsFrustum_key_and_pick_handlers(Pass* pass_ptr);
   void instantiate_OBSFRUSTUM_key_and_pick_handlers(Pass* pass_ptr);
   void instantiate_Polygons_key_and_pick_handlers(Pass* pass_ptr);
   void instantiate_Polyhedra_key_and_pick_handlers(Pass* pass_ptr);
   void instantiate_PolyLines_key_and_pick_handlers(int ndims,Pass* pass_ptr);
   void instantiate_PowerPoints_key_and_pick_handlers(Pass* pass_ptr);
   void instantiate_ROI_Polyhedra_key_and_pick_handlers(Pass* pass_ptr);
   void instantiate_signpost_key_and_pick_handlers(int ndims,Pass* pass_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void Decorations::set_disable_keyhandlers_flag(bool flag)
{
   disable_keyhandlers_flag=flag;
}

inline void Decorations::set_disable_pickhandlers_flag(bool flag)
{
   disable_pickhandlers_flag=flag;
}

// --------------------------------------------------------------------------
inline osg::Group* Decorations::get_OSGgroup_ptr() 
{
   return OSGgroup_refptr.get();
}

inline const osg::Group* Decorations::get_OSGgroup_ptr() const
{
   return OSGgroup_refptr.get();
}

// --------------------------------------------------------------------------
inline void Decorations::set_grid_origin_ptr(threevector* GO_ptr)
{
   grid_origin_ptr=GO_ptr;
}

// --------------------------------------------------------------------------
inline void Decorations::set_PointCloudsGroup_ptr(PointCloudsGroup* PCG_ptr)
{
   PointCloudsGroup_ptr=PCG_ptr;
}

// --------------------------------------------------------------------------
inline AlirtGridsGroup* Decorations::get_AlirtGridsGroup_ptr()
{
   return AlirtGridsGroup_ptr;
}

inline const AlirtGridsGroup* Decorations::get_AlirtGridsGroup_ptr() const
{
   return AlirtGridsGroup_ptr;
}

// --------------------------------------------------------------------------
inline ArmySymbolsGroup* Decorations::get_ArmySymbolsGroup_ptr()
{
   return ArmySymbolsGroup_ptr;
}

inline const ArmySymbolsGroup* Decorations::get_ArmySymbolsGroup_ptr() const
{
   return ArmySymbolsGroup_ptr;
}

inline ArmySymbolPickHandler* Decorations::get_ArmySymbolPickHandler_ptr()
{
   return ArmySymbolPickHandler_ptr;
}

inline const ArmySymbolPickHandler* Decorations::get_ArmySymbolPickHandler_ptr() const
{
   return ArmySymbolPickHandler_ptr;
}

// --------------------------------------------------------------------------
inline ArrowsGroup* Decorations::get_ArrowsGroup_ptr()
{
   return ArrowsGroup_ptr;
}

inline const ArrowsGroup* Decorations::get_ArrowsGroup_ptr() const
{
   return ArrowsGroup_ptr;
}

// --------------------------------------------------------------------------
inline BoxesGroup* Decorations::get_BoxesGroup_ptr()
{
   return BoxesGroup_ptr;
}

inline const BoxesGroup* Decorations::get_BoxesGroup_ptr() const
{
   return BoxesGroup_ptr;
}

// --------------------------------------------------------------------------
inline CylindersGroup* Decorations::get_CylindersGroup_ptr()
{
   return CylindersGroup_ptr;
}

inline const CylindersGroup* Decorations::get_CylindersGroup_ptr() const
{
   return CylindersGroup_ptr;
}

inline CylinderPickHandler* Decorations::get_CylinderPickHandler_ptr()
{
   return CylinderPickHandler_ptr;
}

inline const CylinderPickHandler* Decorations::get_CylinderPickHandler_ptr() 
   const
{
   return CylinderPickHandler_ptr;
}

inline CylindersKeyHandler* Decorations::get_CylindersKeyHandler_ptr()
{
   return CylindersKeyHandler_ptr;
}

inline const CylindersKeyHandler* Decorations::get_CylindersKeyHandler_ptr() const
{
   return CylindersKeyHandler_ptr;
}

// --------------------------------------------------------------------------
inline ConesGroup* Decorations::get_ConesGroup_ptr()
{
   return ConesGroup_ptr;
}

inline const ConesGroup* Decorations::get_ConesGroup_ptr() const
{
   return ConesGroup_ptr;
}

// --------------------------------------------------------------------------
inline FeaturesGroup* Decorations::get_FeaturesGroup_ptr()
{
   return FeaturesGroup_ptr;
}

inline const FeaturesGroup* Decorations::get_FeaturesGroup_ptr() const
{
   return FeaturesGroup_ptr;
}

inline FeaturePickHandler* Decorations::get_FeaturePickHandler_ptr()
{
   return FeaturePickHandler_ptr;
}

inline const FeaturePickHandler* Decorations::get_FeaturePickHandler_ptr() 
   const
{
   return FeaturePickHandler_ptr;
}

// --------------------------------------------------------------------------
inline GraphNodesGroup* Decorations::get_GraphNodesGroup_ptr() 
{
   return GraphNodesGroup_ptr;
}

inline const GraphNodesGroup* Decorations::get_GraphNodesGroup_ptr() const
{
   return GraphNodesGroup_ptr;
}

// --------------------------------------------------------------------------
inline LineSegmentsGroup* Decorations::get_LineSegmentsGroup_ptr()
{
   return LineSegmentsGroup_ptr;
}

inline const LineSegmentsGroup* Decorations::get_LineSegmentsGroup_ptr() const
{
   return LineSegmentsGroup_ptr;
}

// --------------------------------------------------------------------------
inline int Decorations::get_n_LOSMODELSGROUPs() const
{
   return LOSMODELSGROUP_ptrs.size();
}


inline LOSMODELSGROUP* Decorations::get_LOSMODELSGROUP_ptr(int n)
{
   if (n < get_n_LOSMODELSGROUPs())
   {
      return LOSMODELSGROUP_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const LOSMODELSGROUP* Decorations::get_LOSMODELSGROUP_ptr(int n) const
{
   if (n < get_n_LOSMODELSGROUPs())
   {
      return LOSMODELSGROUP_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline LOSMODELSKeyHandler* Decorations::get_LOSMODELSKeyHandler_ptr(int n)
{
   if (n < int(LOSMODELSKeyHandler_ptrs.size()))
   {
      return LOSMODELSKeyHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const LOSMODELSKeyHandler* Decorations::get_LOSMODELSKeyHandler_ptr(int n) 
   const
{
   if (n < int(LOSMODELSKeyHandler_ptrs.size()))
   {
      return LOSMODELSKeyHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline LOSMODELPickHandler* Decorations::get_LOSMODELPickHandler_ptr(int n)
{
   if (n < int(LOSMODELPickHandler_ptrs.size()))
   {
      return LOSMODELPickHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const LOSMODELPickHandler* Decorations::get_LOSMODELPickHandler_ptr(
   int n) 
   const
{
   if (n < int(LOSMODELPickHandler_ptrs.size()))
   {
      return LOSMODELPickHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

// --------------------------------------------------------------------------
inline int Decorations::get_n_ModelsGroups() const
{
   return ModelsGroup_ptrs.size();
}

inline int Decorations::get_n_MODELSGROUPs() const
{
   return MODELSGROUP_ptrs.size();
}

inline ModelsGroup* Decorations::get_ModelsGroup_ptr(int n)
{
   return ModelsGroup_ptrs[n];
}

inline const ModelsGroup* Decorations::get_ModelsGroup_ptr(int n) const
{
   return ModelsGroup_ptrs[n];
}

inline MODELSGROUP* Decorations::get_MODELSGROUP_ptr(int n)
{
   if (n < get_n_MODELSGROUPs())
   {
      return MODELSGROUP_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const MODELSGROUP* Decorations::get_MODELSGROUP_ptr(int n) const
{
   if (n < get_n_MODELSGROUPs())
   {
      return MODELSGROUP_ptrs[n];
   }
   else
   {
      return NULL;
   }
}


inline ModelsKeyHandler* Decorations::get_ModelsKeyHandler_ptr(int n)
{
   if (n < int(ModelsKeyHandler_ptrs.size()))
   {
      return ModelsKeyHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const ModelsKeyHandler* Decorations::get_ModelsKeyHandler_ptr(int n) 
   const
{
   if (n < int(ModelsKeyHandler_ptrs.size()))
   {
      return ModelsKeyHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline MODELSKeyHandler* Decorations::get_MODELSKeyHandler_ptr(int n)
{
   if (n < int(MODELSKeyHandler_ptrs.size()))
   {
      return MODELSKeyHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const MODELSKeyHandler* Decorations::get_MODELSKeyHandler_ptr(int n) 
   const
{
   if (n < int(MODELSKeyHandler_ptrs.size()))
   {
      return MODELSKeyHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline ModelPickHandler* Decorations::get_ModelPickHandler_ptr(int n)
{
   if (n < int(ModelPickHandler_ptrs.size()))
   {
      return ModelPickHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const ModelPickHandler* Decorations::get_ModelPickHandler_ptr(int n) 
   const
{
   if (n < int(ModelPickHandler_ptrs.size()))
   {
      return ModelPickHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline MODELPickHandler* Decorations::get_MODELPickHandler_ptr(int n)
{
   if (n < int(MODELPickHandler_ptrs.size()))
   {
      return MODELPickHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const MODELPickHandler* Decorations::get_MODELPickHandler_ptr(int n) 
   const
{
   if (n < int(MODELPickHandler_ptrs.size()))
   {
      return MODELPickHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline MODELPickHandler* Decorations::get_last_MODELPickHandler_ptr()
{
   if (MODELPickHandler_ptrs.size() > 0)
   {
      return MODELPickHandler_ptrs.back();
   }
   else
   {
      return NULL;
   }
}

inline const MODELPickHandler* Decorations::get_last_MODELPickHandler_ptr() const
{
   if (MODELPickHandler_ptrs.size() > 0)
   {
      return MODELPickHandler_ptrs.back();
   }
   else
   {
      return NULL;
   }
}

// --------------------------------------------------------------------------
inline ObsFrustaGroup* Decorations::get_ObsFrustaGroup_ptr()
{
   return ObsFrustaGroup_ptr;
}

inline const ObsFrustaGroup* Decorations::get_ObsFrustaGroup_ptr() const
{
   return ObsFrustaGroup_ptr;
}

inline OBSFRUSTAGROUP* Decorations::get_OBSFRUSTAGROUP_ptr()
{
   return OBSFRUSTAGROUP_ptr;
}

inline const OBSFRUSTAGROUP* Decorations::get_OBSFRUSTAGROUP_ptr() const
{
   return OBSFRUSTAGROUP_ptr;
}


inline ObsFrustumPickHandler* Decorations::get_ObsFrustumPickHandler_ptr()
{
   return ObsFrustumPickHandler_ptr;
}

inline const ObsFrustumPickHandler* 
Decorations::get_ObsFrustumPickHandler_ptr() const
{
   return ObsFrustumPickHandler_ptr;
}

inline OBSFRUSTUMPickHandler* Decorations::get_OBSFRUSTUMPickHandler_ptr()
{
   return OBSFRUSTUMPickHandler_ptr;
}

inline const OBSFRUSTUMPickHandler* Decorations::get_OBSFRUSTUMPickHandler_ptr() const
{
   return OBSFRUSTUMPickHandler_ptr;
}

inline OBSFRUSTAKeyHandler* Decorations::get_OBSFRUSTAKeyHandler_ptr()
{
   return OBSFRUSTAKeyHandler_ptr;
}

inline const OBSFRUSTAKeyHandler* Decorations::get_OBSFRUSTAKeyHandler_ptr() const
{
   return OBSFRUSTAKeyHandler_ptr;
}

// --------------------------------------------------------------------------
inline PlanesGroup* Decorations::get_PlanesGroup_ptr()
{
   return PlanesGroup_ptr;
}

inline const PlanesGroup* Decorations::get_PlanesGroup_ptr() const
{
   return PlanesGroup_ptr;
}

// --------------------------------------------------------------------------
inline osgGeometry::PointsGroup* Decorations::get_PointsGroup_ptr()
{
   return PointsGroup_ptr;
}

inline const osgGeometry::PointsGroup* Decorations::get_PointsGroup_ptr() 
   const
{
   return PointsGroup_ptr;
}

inline osgGeometry::PointPickHandler* Decorations::get_PointPickHandler_ptr()
{
   return PointPickHandler_ptr;
}

inline const osgGeometry::PointPickHandler* 
Decorations::get_PointPickHandler_ptr() const
{
   return PointPickHandler_ptr;
}

// --------------------------------------------------------------------------
inline osgGeometry::PolygonsGroup* Decorations::get_PolygonsGroup_ptr()
{
   return PolygonsGroup_ptr;
}

inline const osgGeometry::PolygonsGroup* Decorations::get_PolygonsGroup_ptr() 
   const
{
   return PolygonsGroup_ptr;
}

inline osgGeometry::PolygonPickHandler* 
Decorations::get_PolygonPickHandler_ptr()
{
   return PolygonPickHandler_ptr;
}

inline const osgGeometry::PolygonPickHandler* 
Decorations::get_PolygonPickHandler_ptr() const
{
   return PolygonPickHandler_ptr;
}

// --------------------------------------------------------------------------
inline PolyhedraGroup* Decorations::get_PolyhedraGroup_ptr()
{
   return PolyhedraGroup_ptr;
}

inline const PolyhedraGroup* Decorations::get_PolyhedraGroup_ptr() const
{
   return PolyhedraGroup_ptr;
}

inline PolyhedraKeyHandler* Decorations::get_PolyhedraKeyHandler_ptr()
{
   return PolyhedraKeyHandler_ptr;
}

inline const PolyhedraKeyHandler* Decorations::get_PolyhedraKeyHandler_ptr() 
   const
{
   return PolyhedraKeyHandler_ptr;
}

inline PolyhedronPickHandler* Decorations::get_PolyhedronPickHandler_ptr()
{
   return PolyhedronPickHandler_ptr;
}

inline const PolyhedronPickHandler* 
Decorations::get_PolyhedronPickHandler_ptr() const
{
   return PolyhedronPickHandler_ptr;
}

// --------------------------------------------------------------------------
inline PyramidsGroup* Decorations::get_PyramidsGroup_ptr()
{
   return PyramidsGroup_ptr;
}

inline const PyramidsGroup* Decorations::get_PyramidsGroup_ptr() const
{
   return PyramidsGroup_ptr;
}

// --------------------------------------------------------------------------
inline int Decorations::get_n_PolyLinesGroups() const
{
   return PolyLinesGroup_ptrs.size();
}

inline PolyLinesGroup* Decorations::get_PolyLinesGroup_ptr(int n)
{
   if (n < get_n_PolyLinesGroups())
   {
      return PolyLinesGroup_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const PolyLinesGroup* Decorations::get_PolyLinesGroup_ptr(int n) const
{
   if (n < get_n_PolyLinesGroups())
   {
      return PolyLinesGroup_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline PolyLinePickHandler* Decorations::get_PolyLinePickHandler_ptr(int n)
{
   if (n < int(PolyLinePickHandler_ptrs.size()))
   {
      return PolyLinePickHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const PolyLinePickHandler* Decorations::get_PolyLinePickHandler_ptr(
   int n) const
{
   if (n < int(PolyLinePickHandler_ptrs.size()))
   {
      return PolyLinePickHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline PolyLinesKeyHandler* Decorations::get_PolyLinesKeyHandler_ptr(int n)
{
   if (n < int(PolyLinesKeyHandler_ptrs.size()))
   {
      return PolyLinesKeyHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const PolyLinesKeyHandler* Decorations::get_PolyLinesKeyHandler_ptr(
   int n) const
{
   if (n < int(PolyLinesKeyHandler_ptrs.size()))
   {
      return PolyLinesKeyHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

// --------------------------------------------------------------------------
inline RectanglesGroup* Decorations::get_RectanglesGroup_ptr()
{
   return RectanglesGroup_ptr;
}

inline const RectanglesGroup* Decorations::get_RectanglesGroup_ptr() const
{
   return RectanglesGroup_ptr;
}

inline RectanglePickHandler* Decorations::get_RectanglePickHandler_ptr()
{
   return RectanglePickHandler_ptr;
}

inline const RectanglePickHandler* Decorations::get_RectanglePickHandler_ptr()
   const
{
   return RectanglePickHandler_ptr;
}

// --------------------------------------------------------------------------
inline int Decorations::get_n_RegionPolyLinesGroups() const
{
   return RegionPolyLinesGroup_ptrs.size();
}

inline RegionPolyLinesGroup* Decorations::get_RegionPolyLinesGroup_ptr(int n)
{
   if (n < get_n_RegionPolyLinesGroups())
   {
      return RegionPolyLinesGroup_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const RegionPolyLinesGroup* 
Decorations::get_RegionPolyLinesGroup_ptr(int n) const
{
   if (n < get_n_RegionPolyLinesGroups())
   {
      return RegionPolyLinesGroup_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline RegionPolyLinePickHandler* 
Decorations::get_RegionPolyLinePickHandler_ptr(int n)
{
   if (n < int(RegionPolyLinePickHandler_ptrs.size()))
   {
      return RegionPolyLinePickHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const RegionPolyLinePickHandler* 
Decorations::get_RegionPolyLinePickHandler_ptr(int n) const
{
   if (n < int(RegionPolyLinePickHandler_ptrs.size()))
   {
      return RegionPolyLinePickHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline RegionPolyLinesKeyHandler* 
Decorations::get_RegionPolyLinesKeyHandler_ptr(int n)
{
   if (n < int(RegionPolyLinesKeyHandler_ptrs.size()))
   {
      return RegionPolyLinesKeyHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const RegionPolyLinesKeyHandler* 
Decorations::get_RegionPolyLinesKeyHandler_ptr(int n) const
{
   if (n < int(RegionPolyLinesKeyHandler_ptrs.size()))
   {
      return RegionPolyLinesKeyHandler_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

// --------------------------------------------------------------------------
inline ROI_PolyhedraGroup* Decorations::get_ROI_PolyhedraGroup_ptr()
{
   return ROI_PolyhedraGroup_ptr;
}

inline const ROI_PolyhedraGroup* Decorations::get_ROI_PolyhedraGroup_ptr() const
{
   return ROI_PolyhedraGroup_ptr;
}

inline ROI_PolyhedraKeyHandler* Decorations::get_ROI_PolyhedraKeyHandler_ptr()
{
   return ROI_PolyhedraKeyHandler_ptr;
}

inline const ROI_PolyhedraKeyHandler* Decorations::get_ROI_PolyhedraKeyHandler_ptr() 
   const
{
   return ROI_PolyhedraKeyHandler_ptr;
}

inline ROI_PolyhedronPickHandler* Decorations::get_ROI_PolyhedronPickHandler_ptr()
{
   return ROI_PolyhedronPickHandler_ptr;
}

inline const ROI_PolyhedronPickHandler* 
Decorations::get_ROI_PolyhedronPickHandler_ptr() const
{
   return ROI_PolyhedronPickHandler_ptr;
}

// --------------------------------------------------------------------------
inline int Decorations::get_n_SignPostsGroups() const
{
   return SignPostsGroup_ptrs.size();
}

inline SignPostsGroup* Decorations::get_SignPostsGroup_ptr(int n)
{
   if (n < get_n_SignPostsGroups())
   {
      return SignPostsGroup_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const SignPostsGroup* Decorations::get_SignPostsGroup_ptr(int n) const
{
   if (n < get_n_SignPostsGroups())
   {
      return SignPostsGroup_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline SignPostsKeyHandler* Decorations::get_SignPostsKeyHandler_ptr() 
{
   return SignPostsKeyHandler_ptr;
}

inline const SignPostsKeyHandler* Decorations::get_SignPostsKeyHandler_ptr()  const
{
   return SignPostsKeyHandler_ptr;
}

inline SignPostPickHandler* Decorations::get_SignPostPickHandler_ptr() 
{
   return SignPostPickHandler_ptr;
}

inline const SignPostPickHandler* Decorations::get_SignPostPickHandler_ptr() 
   const
{
   return SignPostPickHandler_ptr;
}

// --------------------------------------------------------------------------
inline SphereSegmentsGroup* Decorations::get_SphereSegmentsGroup_ptr(int n)
{
   return SphereSegmentsGroup_ptrs[n];
}

inline const SphereSegmentsGroup* Decorations::get_SphereSegmentsGroup_ptr(
   int n) const
{
   return SphereSegmentsGroup_ptrs[n];
}

inline SphereSegmentPickHandler* Decorations::get_SphereSegmentPickHandler_ptr() 
{
   return SphereSegmentPickHandler_ptr;
}

inline const SphereSegmentPickHandler* Decorations::get_SphereSegmentPickHandler_ptr() const
{
   return SphereSegmentPickHandler_ptr;
}

// --------------------------------------------------------------------------
inline TrianglesGroup* Decorations::get_TrianglesGroup_ptr() 
{
   return TrianglesGroup_ptr;
}

inline const TrianglesGroup* Decorations::get_TrianglesGroup_ptr() const
{
   return TrianglesGroup_ptr;
}

#endif // Decorations.h
