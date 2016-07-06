// ==========================================================================
// Program MOVIEPLAYER is a massively modified version of osgmovie
// which uses Anye Li's ffmpeg OSG plugin to play mp4 and other video
// files.

//			movieplayer clip-20.mp4

// ==========================================================================
// Last updated on 9/18/07; 1/9/08; 1/11/08
// ==========================================================================

#include <iostream>
#include <osg/ImageStream>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"


/*
osg::ImageStream* s_imageStream = 0;


// ==========================================================================
class MovieEventHandler : public osgGA::GUIEventHandler
{
  public:

   MovieEventHandler() {}
    
   void set(osg::Node* node);

   virtual void accept(osgGA::GUIEventHandlerVisitor& v) { 
      v.visit(*this); }

   virtual bool handle(const osgGA::GUIEventAdapter& ea,
                       osgGA::GUIActionAdapter& aa, osg::Object*, 
                       osg::NodeVisitor* nv);
    
   virtual void getUsage(osg::ApplicationUsage& usage) const;

   typedef std::vector< osg::ref_ptr<osg::ImageStream> > ImageStreamList;

  protected:

   virtual ~MovieEventHandler() {}

   class FindImageStreamsVisitor : public osg::NodeVisitor
      {
        public:
         FindImageStreamsVisitor(ImageStreamList& imageStreamList):
            _imageStreamList(imageStreamList) {}
            
         virtual void apply(osg::Geode& geode)
            {
               apply(geode.getStateSet());

               for(unsigned int i=0;i<geode.getNumDrawables();++i)
               {
                  apply(geode.getDrawable(i)->getStateSet());
               }
        
               traverse(geode);
            }

         virtual void apply(osg::Node& node)
            {
               apply(node.getStateSet());
               traverse(node);
            }
        
         inline void apply(osg::StateSet* stateset)
            {
               if (!stateset) return;
            
               osg::StateAttribute* attr = stateset->getTextureAttribute(
                  0,osg::StateAttribute::TEXTURE);
               if (attr)
               {
                  osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(
                     attr);
                  if (texture2D) apply(dynamic_cast<osg::ImageStream*>(
                     texture2D->getImage()));

                  osg::TextureRectangle* textureRec = dynamic_cast<
                     osg::TextureRectangle*>(attr);
                  if (textureRec) apply(dynamic_cast<osg::ImageStream*>(
                     textureRec->getImage()));
               }
            }
        
         inline void apply(osg::ImageStream* imagestream)
            {
               if (imagestream)
               {
                  _imageStreamList.push_back(imagestream); 
                  s_imageStream = imagestream;
               }
            }
        
         ImageStreamList& _imageStreamList;
      };


   ImageStreamList _imageStreamList;
};


void MovieEventHandler::set(osg::Node* node)
{
   _imageStreamList.clear();
   if (node)
   {
      FindImageStreamsVisitor fisv(_imageStreamList);
      node->accept(fisv);
   }
}

bool MovieEventHandler::handle(const osgGA::GUIEventAdapter& ea,
                               osgGA::GUIActionAdapter& aa, osg::Object*, 
                               osg::NodeVisitor* nv)
{
   switch(ea.getEventType())
   {
      case(osgGA::GUIEventAdapter::MOVE):
      case(osgGA::GUIEventAdapter::PUSH):
      case(osgGA::GUIEventAdapter::RELEASE):
      {
         osgProducer::Viewer* viewer = dynamic_cast<osgProducer::Viewer*>(&aa);
         osgUtil::IntersectVisitor::HitList hlist;
         if (viewer->computeIntersections(ea.getX(),ea.getY(), nv->getNodePath().back(), hlist))
         {
            if (!hlist.empty())
            {
               // use the nearest intersection                 
               osgUtil::Hit& hit = hlist.front();
               osg::Drawable* drawable = hit.getDrawable();
               osg::Geometry* geometry = drawable ? drawable->asGeometry() : 0;
               osg::Vec3Array* vertices = geometry ? dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray()) : 0;

               if (vertices)
               {
                  // get the vertex indices.
                  const osgUtil::Hit::VecIndexList& vil = hit.getVecIndexList();
                        
                  if (vil.size()==3)
                  {

                     int i1 = vil[0];
                     int i2 = vil[1];
                     int i3 = vil[2];
                     osg::Vec3 v1 = (*vertices)[i1];
                     osg::Vec3 v2 = (*vertices)[i2];
                     osg::Vec3 v3 = (*vertices)[i3];
                     osg::Vec3 v = hit.getLocalIntersectPoint();
                     osg::Vec3 p1 = hit.getLocalLineSegment()->start();
                     osg::Vec3 p2 = hit.getLocalLineSegment()->end();
                            
                     osg::Vec3 p12 = p1-p2;
                     osg::Vec3 v13 = v1-v3;
                     osg::Vec3 v23 = v2-v3;
                     osg::Vec3 p1v3 = p1-v3;
                            
                     osg::Matrix matrix(p12.x(), v13.x(), v23.x(), 0.0,
                                        p12.y(), v13.y(), v23.y(), 0.0,
                                        p12.z(), v13.z(), v23.z(), 0.0,
                                        0.0,    0.0,    0.0,    1.0);
                                               
                     osg::Matrix inverse;
                     inverse.invert(matrix);
                            
                     osg::Vec3 ratio = inverse*p1v3;

                     // extract the baricentric coordinates.                            
                     float r1 = ratio.y();
                     float r2 = ratio.z();
                     float r3 = 1.0f-r1-r2;

                     osg::Array* texcoords = (geometry->getNumTexCoordArrays()>0) ? geometry->getTexCoordArray(0) : 0;
                     osg::Vec2Array* texcoords_Vec2Array = dynamic_cast<osg::Vec2Array*>(texcoords);
                     if (texcoords_Vec2Array)
                     {
                                // we have tex coord array so now we can compute the final tex coord at the point of intersection.                                
                        osg::Vec2 tc1 = (*texcoords_Vec2Array)[i1];
                        osg::Vec2 tc2 = (*texcoords_Vec2Array)[i2];
                        osg::Vec2 tc3 = (*texcoords_Vec2Array)[i3];
                        osg::Vec2 tc = tc1*r1 + tc2*r2 + tc3*r3;
                                
//                        osg::notify(osg::NOTICE)<<"We hit tex coords "<<tc<<std::endl;
                                
                     }
                            
                            
                  }
                  else
                  {
                     osg::notify(osg::NOTICE)<<"Hit but insufficient indices to work with";
                  }
    
               }

            } 
         }
         else
         {
//            osg::notify(osg::NOTICE)<<"No hit"<<std::endl;
         }
         break;
      }
      case(osgGA::GUIEventAdapter::KEYDOWN):
      {
         if (ea.getKey()=='s')
         {
            for(ImageStreamList::iterator itr=_imageStreamList.begin();
                itr!=_imageStreamList.end();
                ++itr)
            {
               std::cout<<"Play"<<std::endl;
               (*itr)->play();
            }
            return true;
         }
         else if (ea.getKey()=='p')
         {
            for(ImageStreamList::iterator itr=_imageStreamList.begin();
                itr!=_imageStreamList.end();
                ++itr)
            {
               std::cout<<"Pause"<<std::endl;
               (*itr)->pause();
            }
            return true;
         }
         else if (ea.getKey()=='r')
         {
            for(ImageStreamList::iterator itr=_imageStreamList.begin();
                itr!=_imageStreamList.end();
                ++itr)
            {
               std::cout<<"Restart"<<std::endl;
               (*itr)->rewind();
               (*itr)->play();
            }
            return true;
         }
         else if (ea.getKey()=='l')
         {
            for(ImageStreamList::iterator itr=_imageStreamList.begin();
                itr!=_imageStreamList.end();
                ++itr)
            {
               if ( (*itr)->getLoopingMode() == osg::ImageStream::LOOPING)
               {
                  std::cout<<"Toggle Looping Off"<<std::endl;
                  (*itr)->setLoopingMode( osg::ImageStream::NO_LOOPING );
               }
               else
               {
                  std::cout<<"Toggle Looping On"<<std::endl;
                  (*itr)->setLoopingMode( osg::ImageStream::LOOPING );
               }
            }
            return true;
         }
         return false;
      }

      default:
         return false;
   }
   return false;
}

void MovieEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
   usage.addKeyboardMouseBinding("p","Pause movie");
   usage.addKeyboardMouseBinding("s","Play movie");
   usage.addKeyboardMouseBinding("r","Restart movie");
   usage.addKeyboardMouseBinding("l","Toggle looping of movie");
}

*/


// =========================================================================

int main(int argc, char** argv)
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;

   timefunc::initialize_timeofday_clock();

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
   cout << "videopass_ID = " << videopass_ID << endl;

// Construct the viewer and instantiate a ViewerManager:

   osgProducer::Viewer viewer(arguments);
   WindowManager* window_mgr_ptr=new ViewerManager(&viewer);
   window_mgr_ptr->initialize_window("2D imagery");
  
// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   Operations operations(ndims,window_mgr_ptr,display_movie_state,
                         display_movie_number);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = 
      new osgGA::Custom2DManipulator(ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_2D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      window_mgr_ptr,ModeController_ptr,CM_2D_ptr);

// Instantiate points, polygons, linesegments, triangles, rectangles
// and features decorations group:

   decorations.add_Points(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
//   LineSegmentsGroup* LineSegmentsGroup_ptr=
   decorations.add_LineSegments(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   decorations.add_Triangles(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   decorations.add_Rectangles(
      ndims,passes_group.get_pass_ptr(videopass_ID));

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      decorations.get_PointsGroup_ptr(),
      decorations.get_PolygonsGroup_ptr(),AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group);

   AnimationController_ptr->set_nframes(movie_ptr->get_Nimages());
   cout << "n_images = " << movie_ptr->get_Nimages() << endl;
   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

/*
// Pass the model to the MovieEventHandler so it can pick out
// ImageStreams to manipulate:

   MovieEventHandler* meh = new MovieEventHandler();
   osg::Geode* geode_ptr=movie_ptr->getGeode();
   geode_ptr->setEventCallback(meh);
   meh->set(geode_ptr);
*/

// Attach the scene graph to the viewer:

   root->addChild(decorations.get_OSGgroup_ptr());

   viewer.setSceneData( root );

// Create the windows and run the threads:

   cout << "Before call to viewer.realize() in main" << endl;

   viewer.realize();

   while( !viewer.done() )
   {
      window_mgr_ptr->process();
   }

}
