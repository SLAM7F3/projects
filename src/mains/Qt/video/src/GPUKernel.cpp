
#include <iostream>

#include "GPUKernel.h"

#include <cassert>
#include <QFile>
#include <QDomDocument>
#include <Cg/CgShared.h>
#include <math/MathUtils.h>

#include "ArrayIndexKernel.h"
#include "BilateralKernel.h"
#include "BufferObject2TextureKernel.h"
#include "CDFKernel.h"
#include "CombineQuantizedLuminanceAndDoGEdgesKernel.h"
#include "CombineTexturenessKernel.h"
#include "CrossBilateralKernel.h"
#include "DetailMapKernel.h"
#include "FilenameToRGBArrayKernel.h"
#include "GaussianBlurKernel.h"
#include "GazeBasedDetailAdjustmentKernel.h"
#include "GridPaintingKernel.h"
#include "HistogramKernel.h"
#include "HistogramTransferKernel.h"
#include "InteractiveToneMapKernel.h"
#include "Lab2RGBKernel.h"
#include "LinearCombinationKernel.h"
#include "LocalHistogramEqualizationKernel.h"
#include "LuminanceQuantizationKernel.h"
#include "PerPixelProcessorKernel.h"
#include "RGB2LabKernel.h"
#include "RGB2TextureKernel.h"
#include "RGB2VBOKernel.h"
#include "ScribbleKernel.h"
#include "StylizeKernel.h"
#include "DifferenceOfGaussiansEdgesKernel.h"
#include "Texture2BufferObjectKernel.h"

using std::cout;
using std::endl;

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
GPUKernel* GPUKernel::createKernelWithClassname( QString className, QString args )
{
   // TODO: register classes, etc


   cout << "inside GPUKernel::createKernelWithClassname()" << endl;
   cout << "className = " << className.toStdString() << endl;
   
   if( className == "ArrayIndex" )
   {
      return ArrayIndexKernel::create( args );
   }	
   else if( className == "BufferObject2Texture" )
   {
      return BufferObject2TextureKernel::create( args );
   }
   else if( className == "Bilateral" )
   {
      return BilateralKernel::create( args );
   }
   else if( className == "CDF" )
   {
      return CDFKernel::create( args );
   }
   else if( className == "CombineQuantizedLuminanceAndDoGEdges" )
   {
      return CombineQuantizedLuminanceAndDoGEdgesKernel::create( args );
   }
   else if( className == "CombineTextureness" )
   {
      return CombineTexturenessKernel::create( args );
   }
   else if( className == "CrossBilateral" )
   {
      return CrossBilateralKernel::create( args );
   }
   else if( className == "DetailMap" )
   {
      return DetailMapKernel::create( args );
   }
   else if( className == "DifferenceOfGaussiansEdges" )
   {
      return DifferenceOfGaussiansEdgesKernel::create( args );
   }
   else if( className == "FilenameToRGBArray" )
   {
      return FilenameToRGBArrayKernel::create( args );
   }
   else if( className == "GaussianBlur" )
   {
      return GaussianBlurKernel::create( args );
   }
   else if( className == "GazeBasedDetailAdjustment" )
   {
      return GazeBasedDetailAdjustmentKernel::create( args );
   }
   else if( className == "GridPainting" )
   {
      return GridPaintingKernel::create( args );
   }
   else if( className == "Histogram" )
   {
      return HistogramKernel::create( args );
   }
   else if( className == "HistogramTransfer" )
   {
      return HistogramTransferKernel::create( args );
   }
   else if( className == "InteractiveToneMap" )
   {
      return InteractiveToneMapKernel::create( args );
   }
   else if( className == "Lab2RGB" )
   {
      return Lab2RGBKernel::create( args );
   }
   else if( className == "LinearCombination" )
   {
      return LinearCombinationKernel::create( args );
   }
   else if( className == "LocalHistogramEqualization" )
   {
      return LocalHistogramEqualizationKernel::create( args );
   }
   else if( className == "LuminanceQuantization" )
   {
      return LuminanceQuantizationKernel::create( args );
   }
   else if( className == "PerPixelProcessor" )
   {
      return PerPixelProcessorKernel::create( args );
   }
   else if( className == "RGB2Lab" )
   {
      return RGB2LabKernel::create( args );
   }
   else if( className == "RGB2Texture" )
   {
      return RGB2TextureKernel::create( args );
   }
   else if( className == "RGB2VBO" )
   {
      return RGB2VBOKernel::create( args );
   }
   else if( className == "Scribble" )
   {
      return ScribbleKernel::create( args );
   }
   else if( className == "Stylize" )
   {
      return StylizeKernel::create( args );
   }
   else if( className == "Texture2BufferObject" )
   {
      return Texture2BufferObjectKernel::create( args );
   }

   return NULL;
}

// virtual
GPUKernel::~GPUKernel()
{

}

// virtual
bool GPUKernel::connect( QString outputPortName,
                         GPUKernel* pReceiver, QString inputPortName )
{
   OutputKernelPort* pOut = getOutputPortByName( outputPortName );
   if( pOut != NULL )
   {
      InputKernelPort* pIn = pReceiver->getInputPortByName( inputPortName );
      if( pIn != NULL )
      {
         if( pOut->getType() == pIn->getType() )
         {
            pOut->addConnection( pIn );
            pReceiver->onInputConnected( inputPortName, pIn );
            onOutputConnected( outputPortName, pOut );

            return true;
         }
         else
         {
            fprintf( stderr, "Warning: attempted to connect two ports of differing data types!\n" );
         }
      }	
   }

   return false;
}

// virtual
bool GPUKernel::disconnect( QString outputPortName,
                            GPUKernel* pReceiver, QString inputPortName )
{
   OutputKernelPort* pOut = getOutputPortByName( outputPortName );
   if( pOut != NULL )
   {
      InputKernelPort* pIn = pReceiver->getInputPortByName( inputPortName );
      if( pIn != NULL )
      {
         pOut->removeConnection( pIn );
         pReceiver->onInputDisconnected( inputPortName, pIn );
         onOutputDisconnected( outputPortName, pOut );

         return true;
      }
   }

   return false;
}

// virtual
bool GPUKernel::isInputComplete()
{
   assert( false );
   return false;
}

QHash< QString, InputKernelPort* > GPUKernel::getInputPorts()
{
   return m_qhInputPorts;
}

QHash< QString, OutputKernelPort* > GPUKernel::getOutputPorts()
{
   return m_qhOutputPorts;
}

InputKernelPort* GPUKernel::getInputPortByName( QString name )
{
   if( m_qhInputPorts.contains( name ) )
   {
      return m_qhInputPorts[ name ];
   }
   else
   {
      return NULL;
   }
}

OutputKernelPort* GPUKernel::getOutputPortByName( QString name )
{
   if( m_qhOutputPorts.contains( name ) )
   {
      return m_qhOutputPorts[ name ];
   }
   else
   {
      return NULL;
   }
}

QString GPUKernel::getClassName()
{
   return m_className;
}

//////////////////////////////////////////////////////////////////////////
// Protected
//////////////////////////////////////////////////////////////////////////

GPUKernel::GPUKernel( QString className ) :

   m_className( className )

{

}

// virtual
void GPUKernel::initializeGL()
{
   initializePorts();
}

// virtual
void GPUKernel::initializePorts()
{
   assert( false );
}

InputKernelPort* GPUKernel::addInputPort( QString name, KernelPortDataType type )
{
   // TODO: handle deletion of ports
   // not adding ports to kernels, but how to delete a connected port
	
   InputKernelPort* pIn = new InputKernelPort( name, this, type );
   m_qhInputPorts[ name ] = pIn;

   return pIn;
}

OutputKernelPort* GPUKernel::addOutputPort( QString name, KernelPortDataType type )
{
   // TODO: handle deletion of ports
   // not adding ports to kernels, but how to delete a connected port

   OutputKernelPort* pOut = new OutputKernelPort( name, this, type );
   m_qhOutputPorts[ name ] = pOut;

   return pOut;
}

// virtual
void GPUKernel::onInputConnected( QString inputPortName, InputKernelPort* pPort )
{

}

// virtual
void GPUKernel::onInputDisconnected( QString inputPortName, InputKernelPort* pPort )
{

}

// virtual
void GPUKernel::onOutputConnected( QString outputPortName, OutputKernelPort* pPort )
{

}

// virtual
void GPUKernel::onOutputDisconnected( QString outputPortName, OutputKernelPort* pPort )
{

}
