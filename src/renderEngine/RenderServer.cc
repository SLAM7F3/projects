// ==========================================================================
// Brad Boven's RenderServer class
// ==========================================================================
// Last modified on 9/13/05
// ==========================================================================

#include <iostream>
#include "osg/osgfuncs.h"
#include "renderEngine/RenderServer.h"

using std::cout;
using std::endl;
using std::string;

RenderServer::RenderServer(int argc, char** argv)
{
    killed = false;

    // Use an ArgumentParser object to manage the program arguments:
    osg::ArgumentParser arguments(&argc,argv);
    usage = new osg::MyApplicationUsage;
    arguments.setApplicationUsage(usage);

    // Construct the viewer:
    viewer = new osgProducer::Viewer(arguments);

    // Set up the viewer with sensible default event handlers;
    viewer->setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE);

    // Initialize viewer window:
    Producer::RenderSurface* rs_ptr = viewer->getCameraConfig()->getCamera(0)->getRenderSurface();
    osgfunc::initialize_window(rs_ptr,"4D ladar video sequences");

    // Create OSG root node and black backdrop:
    root = new osg::Group;
    osgfunc::generate_black_backdrop(root.get());

    // Add a custom manipulator to the event handler list:
    c3Dm_ptr= new osgGA::ExampleCustomManipulator();
    viewer->addCameraManipulator(c3Dm_ptr.get());

    viewer->setSceneData(root.get());
}

RenderServer::~RenderServer()
{
}

void RenderServer::run()
{
    bool first_object_flag = true;

    //must do this here, anywhere else causes real OpenGL issues
    viewer->realize();

    //keep running until the user wants to kill the program
    while(killed == false)
    {
        //when data ready is set to true, it means now data (presumed to be
        //similar in nature) will start coming in.  Thus, once you get your
        //first data, you know to reset the camera, so that you can see the
        //data in its home position.  Setting data ready more than once will
        //reset the camera
        if (data_ready == true)
        {
            if (first_object_flag == true)
            {
                c3Dm_ptr->home(1);
                first_object_flag = false;
            }
            data_ready = false;
        }

        // Wait for all cull and draw threads to complete:
        viewer->sync();

        // Update the scene by traversing it with the the update visitor which
        // will call all node update callbacks and animations:
        viewer->update();

        // Fire off the cull and draw traversals of the scene:
        viewer->frame();

        // if user quit the viewing window, kill the program so that the
        // calling program knows to stop doing computation
        if (viewer->done() == true)
            killed=true;
    }

    // Wait for all cull and draw threads to complete before exiting:
    viewer->sync();
}

void RenderServer::set_data_ready()
{
    data_ready = true;
}

void RenderServer::send_osg_group(osg::ref_ptr<osg::Group> data, std::string name)
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

    // if the capabilities were extended to use MyViewerEventHandler with the
    // Viewer, then you would want to make sure the thread knew about the
    // communicator, and have it register data_root_3D in the case when
    // data_root is updated.  This is because MyViewerEventHandler always
    // expects a valid handle to data_root in order to write out the scene into
    // an .ive file by pressing 'o'.  
    //if (name == "data_root")
    //    com_ptr->submit_pointer("data_root_3D",data);
}

void RenderServer::delete_osg_group(std::string name)
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

                // if the capabilities were extended to use MyViewerEventHandler with the
                // Viewer, then you would want to make sure the thread knew about the
                // communicator, and have it register data_root_3D in the case when
                // data_root is updated.  This is because MyViewerEventHandler always
                // expects a valid handle to data_root in order to write out the scene into
                // an .ive file by pressing 'o'.  
                //if (name == "data_root")
                //    com_ptr->submit_pointer("data_root_3D",root);
            }
        }
    }
}

void RenderServer::replace_osg_group(osg::ref_ptr<osg::Group> data, std::string name)
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

                // if the capabilities were extended to use MyViewerEventHandler with the
                // Viewer, then you would want to make sure the thread knew about the
                // communicator, and have it register data_root_3D in the case when
                // data_root is updated.  This is because MyViewerEventHandler always
                // expects a valid handle to data_root in order to write out the scene into
                // an .ive file by pressing 'o'.  
                //if (name == "data_root")
                //    com_ptr->submit_pointer("data_root_3D",root);
            }
        }
    }
}

bool RenderServer::if_killed()
{
    return killed;
}

void RenderServer::kill()
{
    killed = true;
}

