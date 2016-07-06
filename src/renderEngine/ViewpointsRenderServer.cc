// ========================================================================
// Header file for Brad Boven's ViewpointsRenderServer class
// ========================================================================
// Last updated on 9/6/05
// ========================================================================

#include "renderEngine/ViewpointsRenderServer.h"

using std::cout;
using std::endl;
using std::string;

ViewpointsRenderServer::ViewpointsRenderServer(
   int argc, char** argv, communicator* com_ptr)
{
   killed = false;
   callbacks_done = false;

   ndims = 3;
   this->com_ptr = com_ptr;

// Use an ArgumentParser object to manage the program arguments:
// osg::ArgumentParser arguments(&argc,argv);

   arguments = new osg::ArgumentParser(&argc,argv);
   usage = new osg::MyApplicationUsage;
   arguments->setApplicationUsage(usage);

   // Construct the viewer:
   viewer = new osgProducer::Viewer(*arguments);

   // Set up the viewer with sensible default event handlers;
   viewer->setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE);
   com_ptr->submit_pointer("Viewer_3D",viewer.get());

// Initialize constants and parameters read in from command line as
// well as ascii text file:

   const string parameter_filename="processing_params.txt";
   colormap = new ColorMap();
   threeDdatafunc::parse_input_parameters(parameter_filename,colormap);

   // Initialize viewer window:
   Producer::RenderSurface* rs_ptr = viewer->getCameraConfig()->
      getCamera(0)->getRenderSurface();
   osgfunc::initialize_window(rs_ptr,"4D ladar video sequences");

   // Create OSG root node and black backdrop:
   root = new osg::Group;
   osgfunc::generate_black_backdrop(root.get());
   com_ptr->submit_pointer("data_root_3D",root.get());

   // Instantiate a transformer in order to convert between screen and
   // world space coordinate systems:
   transform = new Transformer(com_ptr);

   // Instantiate a mode controller and key event handler:
   ModeController_ptr=new ModeController(ndims,com_ptr);
   viewer->getEventHandlerList().push_back(new ModeKeyHandler(
      ModeController_ptr));

   FlyOverController_ptr=new FlyOverController(com_ptr);

   // Add a custom manipulator to the event handler list:
   c3Dm_ptr= new osgGA::Custom3DManipulator(com_ptr);
   viewer->addCameraManipulator(c3Dm_ptr.get());

   // Instantiate signpost controller, key handler and pick handler:
   //SignPostsController_ptr=new SignPostsController(com_ptr);
   //viewer->getEventHandlerList().push_back(new SignPostsKeyHandler(com_ptr));
   //SignPostPickHandler* SignPostPickHandler_ptr=new SignPostPickHandler(
   //                            c3Dm_ptr,signposts_group,&cloud,com_ptr);
   //viewer->getEventHandlerList().push_back(SignPostPickHandler_ptr);

   // CAN'T DO THIS WITHOUT A GRID
   // Setup a controller to allow for spinning of the scene along an axis
   //axis_spinner = new AxisSpinner(com_ptr, root.get(), c3Dm_ptr.get());
   //viewer->getEventHandlerList().push_back(axis_spinner);
    
   // Optimize the scene graph, remove redundent nodes and states, and
   // then attach it to the viewer:
   osgUtil::Optimizer optimizer;
   optimizer.optimize(root.get());
   viewer->setSceneData(root.get());
}

ViewpointsRenderServer::~ViewpointsRenderServer()
{
}

void ViewpointsRenderServer::run()
{
   bool first_object_flag = true;

//      viewer->setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);
//      viewer->setUpViewer(osgProducer::Viewer::HEAD_LIGHT_SOURCE |
//                          osgProducer::Viewer::VIEWER_MANIPULATOR |
//                          osgProducer::Viewer::ESCAPE_SETS_DONE);
//      viewer->setUpViewer(osgProducer::Viewer::SKY_LIGHT_SOURCE);
//      viewer->setUpViewer(osgProducer::Viewer::HEAD_LIGHT_SOURCE);

   //must do this here, anywhere else causes real OpenGL issues
   viewer->realize();

   // Add an animation path creator to the event handler list AFTER the
   // viewer has been realized:

   AnimationPathCreator* animation_path_handler = 
      new AnimationPathCreator(com_ptr, ndims);
   viewer->getEventHandlerList().push_back(animation_path_handler);

   //keep running until the user wants to kill the program
   while(killed == false)
   {
      if (callbacks_done == true)
      {
         myVEH_ptr = new osgProducer::MyViewerEventHandler(com_ptr, ndims);
         viewer->getEventHandlerList().push_front(myVEH_ptr);

         viewer->getUsage(*(arguments->getApplicationUsage()));
         callbacks_done = false;
      }

      // when data ready is set to true, it means now data (presumed
      // to be similar in nature) will start coming in.  Thus, once
      // you get your first data, you know to reset the camera, so
      // that you can see the data in its home position.  Setting data
      // ready more than once will reset the camera

      if (data_ready == true)
      {
         if (first_object_flag == true)
         {
            c3Dm_ptr->home(1);
            first_object_flag = false;
         }

         // Wait for all cull and draw threads to complete:
         viewer->sync();

         // Update the scene by traversing it with the the update
         // visitor which will call all node update callbacks and
         // animations:
         viewer->update();

         /*
           if (axis_spinner->get_if_on() == true)
           {
           osg::Matrixd m = axis_spinner->get_orbit()->getWCMatrix();
           osg::Matrixd i = m.inverse(m);
           viewer->setViewByMatrix( Producer::Matrix( i.ptr() )* Producer::Matrix::rotate( osg::DegreesToRadians(-90.0), 1, 0, 0 ) );
           //viewer.setViewByMatrix(Producer::Matrix(i.ptr()) * Producer::Matrix::rotate(-M_PI/180.0, 1, 0, 0));
           }
         */

         // Fire off the cull and draw traversals of the scene:
         viewer->frame();

         // if user quit the viewing window, kill the program so that the
         // calling program knows to stop doing computation
         if (viewer->done() == true)
            killed=true;
      }
   }

   // Wait for all cull and draw threads to complete before exiting:
   viewer->sync();
}

void ViewpointsRenderServer::set_data_ready()
{
   data_ready = true;
}

void ViewpointsRenderServer::send_osg_group(
   osg::ref_ptr<osg::Group> data, std::string name)
{
   data->setName(name);
   int n = root->getNumChildren();
   for (int i = 0; i < n; i++)
   {
      if (root->getChild(i) != NULL)
      {
         if (name == root->getChild(i)->getName())
         {
            replace_osg_group(data, name);
            return;
         }
      }
   }
   root->addChild(data.get());

   // if the capabilities were extended to use MyViewerEventHandler
   // with the Viewer, then you would want to make sure the thread
   // knew about the communicator, and have it register data_root_3D
   // in the case when data_root is updated.  This is because
   // MyViewerEventHandler always expects a valid handle to data_root
   // in order to write out the scene into an .ive file by pressing
   // 'o'.
//   if (name == "data_root")
//       com_ptr->submit_pointer("data_root_3D",data.get());
}

void ViewpointsRenderServer::delete_osg_group(std::string name)
{
   int n = root->getNumChildren();
   for (int i = 0; i < n; i++)
   {
      if (root->getChild(i) != NULL)
      {
         if (name == root->getChild(i)->getName())
         {
            root->removeChild(i);
            return;

            // if the capabilities were extended to use
            // MyViewerEventHandler with the Viewer, then you would
            // want to make sure the thread knew about the
            // communicator, and have it register data_root_3D in the
            // case when data_root is updated.  This is because
            // MyViewerEventHandler always expects a valid handle to
            // data_root in order to write out the scene into an .ive
            // file by pressing 'o'.
            //if (name == "data_root")
            //    com_ptr->submit_pointer("data_root_3D",root);
         }
      }
   }
}

void ViewpointsRenderServer::replace_osg_group(
   osg::ref_ptr<osg::Group> data, std::string name)
{
   int n = root->getNumChildren();
   for (int i = 0; i < n; i++)
   {
      if (root->getChild(i) != NULL)
      {
         if (name == root->getChild(i)->getName())
         {
            root->replaceChild(root->getChild(i), data.get());
            return;

            // if the capabilities were extended to use
            // MyViewerEventHandler with the Viewer, then you would
            // want to make sure the thread knew about the
            // communicator, and have it register data_root_3D in the
            // case when data_root is updated.  This is because
            // MyViewerEventHandler always expects a valid handle to
            // data_root in order to write out the scene into an .ive
            // file by pressing 'o'.
            //if (name == "data_root")
            //    com_ptr->submit_pointer("data_root_3D",root);
         }
      }
   }
}

void ViewpointsRenderServer::add_osg_callback(osg::Projection* projection_ptr)
{
   root->addChild(projection_ptr);
}

void ViewpointsRenderServer::add_osg_callback(osg::Geode* geode_ptr)
{
   root->addChild(geode_ptr);
}

void ViewpointsRenderServer::set_callbacks_done()
{
   callbacks_done = true;    
}

bool ViewpointsRenderServer::if_killed()
{
   return killed;
}

void ViewpointsRenderServer::kill()
{
   killed = true;
}

