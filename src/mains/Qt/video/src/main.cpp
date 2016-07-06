#include <GL/GLUtilities.h>
#include <QApplication>
#include <QHBoxLayout>
#include <QPlastiqueStyle>

#include "AppData.h"
#include "Controls.h"
#include "GPUKernel.h"
#include "OutputWidget.h"
#include "KernelGraph.h"

int main( int argc, char* argv[] )
{
   QApplication a( argc, argv );
   QApplication::setStyle( new QPlastiqueStyle );

   if( argc < 4 )
   {
      printf( "\nUsage: %s graph.xml shaderPath [nBits] [sleep|nosleep] [width] [height] [nx] [ny]\n\n", argv[0] );
      printf( "graph.xml specifies the computation graph to run.\n\n" );
      printf( "shaderPath is the path (relative or absolute) to all the shaders.\n\tYou probably want to put \"..\\shaders\"\n\n" );
      printf( "nBits must be \"16\" or \"32\"\n\tSpecifies the *floating point* bit depth of texture render targets.\n\tDefault = 16\n" );
      printf( "\tNote that only the G80 or later supports 32-bit float blending.\nIf you have a G70 or earlier, please use 16-bit\n\n" );
      printf( "sleep|nosleep either enables or disables the frame limiter.\n\tsleep makes the video play back at its framerate.\n\tnosleep makes it run as fast as possible.  Default = sleep\n\n" );
      printf( "width and height specify the size of each viewport in the GUI.\n\tDefault = 720 x 480\n\n" );
      printf( "nx and ny specify the number of viewports.\n\tDefault = 2 x 2\n" );
   }
   else
   {
      QString graphXMLFilename = argv[1];
      QString shaderPathString = argv[2];
      QString nBitsString = argv[3];

      bool parseNBitsStringSucceeded = false;
      int nBits = nBitsString.toInt( &parseNBitsStringSucceeded );
      if( ( !parseNBitsStringSucceeded ) || !( nBits == 16 || nBits == 32 ) )
      {
         fprintf( stderr, "Failed to parse nBits: nBits must be \"16\" or \"32\" (and your hardware must support blending at that bitrate!)\n" );
         fprintf( stderr, "Assuming 16...\n" );
         nBits = 16;
      }

      bool bRunNoSleep = false;
      if( argc > 4 )
      {
         QString sleepString = argv[4];
         if( sleepString == "nosleep" )
         {
            bRunNoSleep = true;
         }
         else
         {
            bRunNoSleep = false;
         }			
      }

      int width = 720;
      int height = 480;	
      if( argc > 6 )
      {
         QString widthString = argv[5];
         QString heightString = argv[6];

         bool parseWidthSucceeded;
         bool parseHeightSucceeded;
         int tempWidth = widthString.toInt( &parseWidthSucceeded );
         int tempHeight = heightString.toInt( &parseHeightSucceeded );

         if( parseWidthSucceeded && parseHeightSucceeded )
         {
            width = tempWidth;
            height = tempHeight;
         }
      }

      int nViewportsX = 2;
      int nViewportsY = 2;
      if( argc > 8 )
      {
         QString nViewportsXString = argv[7];
         QString nViewportsYString = argv[8];

         bool parseNXSucceeded;
         bool parseNYSucceeded;
         int tempNX = nViewportsXString.toInt( &parseNXSucceeded );
         int tempNY = nViewportsYString.toInt( &parseNYSucceeded );

         if( parseNXSucceeded && parseNYSucceeded )
         {
            nViewportsX = tempNX;
            nViewportsY = tempNY;
         }
      }

      // instantiate main classes
      AppData* pAppData = AppData::getInstance();	
      Controls* pControls = Controls::getInstance();
      pAppData->setTextureNumBits( nBits );
      pAppData->setCgPathPrefix( shaderPathString );
      pAppData->setOutputWidthHeight( width, height );

      OutputWidget* pOutputWidget = new OutputWidget( width, height, nViewportsX, nViewportsY );
      pAppData->setOutputWidget( pOutputWidget );

      pOutputWidget->show(); // just to initialize OpenGL context

      // needs opengl context
      KernelGraph* pGraph;
      QVector< QPair< QString, int > > viewedOutputPorts;
      QVector< QPair< QString, QString > > autoDirtyKernelsAndPorts;
      QVector< QString > mouseListenerKernels;
      bool loadSucceeded = KernelGraph::loadFromXML( graphXMLFilename,
                                                     &pGraph,
                                                     &viewedOutputPorts,
                                                     &autoDirtyKernelsAndPorts,
                                                     &mouseListenerKernels );
      if( loadSucceeded )
      {
         pAppData->setGraph( pGraph );
         pAppData->setAutoDirtyKernelsAndPorts( autoDirtyKernelsAndPorts );

         // connect mouse listeners
         foreach( QString kernelName, mouseListenerKernels )
            {
               printf( "Connecting mouse listener: kernelName = %s\n", qPrintable( kernelName ) );

               GPUKernel* pKernel = pGraph->getKernelByName( kernelName );
               QObject* pKernelQObject = dynamic_cast< QObject* >( pKernel );
               QObject::connect( pOutputWidget, SIGNAL( mousePressed( int, int, int ) ),
                                 pKernelQObject, SLOT( handleMousePressed( int, int, int ) ) );
               QObject::connect( pOutputWidget, SIGNAL( mouseReleased( int, int, int ) ),
                                 pKernelQObject, SLOT( handleMouseReleased( int, int, int ) ) );
               QObject::connect( pOutputWidget, SIGNAL( mouseMoved( int, int ) ),
                                 pKernelQObject, SLOT( handleMouseMoved( int, int ) ) );
            }

         pControls->initialize( pGraph, viewedOutputPorts, pOutputWidget );

         int x0 = 25;
         int y0 = 25;
         pControls->move( QPoint( x0, y0 ) );
         pControls->show();

         QRect controlsGeometry = pControls->frameGeometry();
         pOutputWidget->move( QPoint( x0 + controlsGeometry.width(), y0 ) );
			
         if( bRunNoSleep )
         {
            pAppData->startNoSleep();
         }
         else
         {
            pAppData->start();
         }			
      }
   }
}
