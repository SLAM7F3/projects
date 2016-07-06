#include "FilenameToRGBArrayKernel.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <QDir>
#include <QFileInfo>

#include <QImage>

#include <video/FFMPEGVideo.h>

#include <color/ColorUtils.h>
#include <io/PortableFloatMapIO.h>
#include <video/QDecoderThread.h>

using std::cout;
using std::endl;

// static
FilenameToRGBArrayKernel* FilenameToRGBArrayKernel::create( QString args )
{
   cout << "inside FilenameToRGBArrayKernel::create()" << endl;
   FilenameToRGBArrayKernel* pInstance = new FilenameToRGBArrayKernel( args );
   pInstance->initializeGL();
   return pInstance;
}

// virtual
FilenameToRGBArrayKernel::~FilenameToRGBArrayKernel()
{
   cleanup();
}

// virtual
bool FilenameToRGBArrayKernel::isInputComplete()
{
   return true;
}

bool FilenameToRGBArrayKernel::isVideo()
{
   return m_bIsVideo;
}

bool FilenameToRGBArrayKernel::isImageSequence()
{
   return m_bIsImageSequence;
}

bool FilenameToRGBArrayKernel::isPaused()
{
   return m_bPaused;
}

void FilenameToRGBArrayKernel::togglePause()
{
   m_bPaused = !m_bPaused;
}

Reference< IVideo > FilenameToRGBArrayKernel::getVideo()
{
   return m_pVideo;
}

QDecoderThread* FilenameToRGBArrayKernel::getDecoderThread()
{
   return m_pDecoderThread;
}

// virtual
void FilenameToRGBArrayKernel::makeDirty( QString inputPortName )
{
   cout << "inside FilenameToRGBArrayKernel::makeDirty()" << endl;
   m_pSizeOutputPort->makeDirty();
   m_pRGBArrayOutputPort->makeDirty();

   m_bReallocationNeeded = true;
}

// virtual
void FilenameToRGBArrayKernel::compute( QString outputPortName )
{
   cout << "inside FilenameToRGBArrayKernel::compute()" << endl;

   if( m_bReallocationNeeded )
   {
      reallocate();
      m_bReallocationNeeded = false;
   }

   if( outputPortName == "size" )
   {
      m_pSizeOutputPort->pushData( KernelPortData( m_qSize ) );
   }
   else
   {
      cout << "isVideo() = " << isVideo() << endl;

      if( isVideo() )
      {
         if( !isPaused() )
         {
            cout << "Before call to DecoderThread->getNextFrame()"
                 << endl;
            ubyte* pFrame = m_pDecoderThread->getNextFrame();
            cout << "After call to DecoderThread->getNextFrame()"
                 << endl;

            if( pFrame != NULL )
            {
               ArrayWithLength< ubyte > rgb( pFrame, m_nBytes );
               m_pRGBArrayOutputPort->pushData( KernelPortData( rgb ) );

            }
         }			
      }
      else if( isImageSequence() )
      {
         QString filename = m_qsDirectoryName + "/" + m_qslFilenames[ m_iImageSequenceIndex ];

         QImage image( filename );
         if( image.isNull() )
         {
            fprintf( stderr, "Error reading %s\n", qPrintable( filename ) );
            exit( -1 );
         }

         int index = 0;
         for( int y = 0; y < m_qSize.height(); ++y )	
         {
            for( int x = 0; x < m_qSize.width(); ++x )
            {
               QRgb rgb = image.pixel( x, y );

               m_aubImagePixels[ index ] = qRed( rgb );
               m_aubImagePixels[ index + 1 ] = qGreen( rgb );
               m_aubImagePixels[ index + 2 ] = qBlue( rgb );

               index += 3;
            }
         }

         m_iImageSequenceIndex = ( m_iImageSequenceIndex + 1 ) % m_qslFilenames.size();

         ArrayWithLength< ubyte > rgb( m_aubImagePixels, m_nBytes );
         m_pRGBArrayOutputPort->pushData( KernelPortData( rgb ) );
      }
      else if( m_bIsUnsignedByte )
      {
         ArrayWithLength< ubyte > rgb( m_aubImagePixels, m_nBytes );
         m_pRGBArrayOutputPort->pushData( KernelPortData( rgb ) );
      }
      else
      {
         m_pRGBArrayOutputPort->pushData( KernelPortData( m_qvFloatImagePixels ) );
      }
   }	
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void FilenameToRGBArrayKernel::initializeGL()
{
   GPUKernel::initializeGL();
}

// virtual
void FilenameToRGBArrayKernel::initializePorts()
{
   // input port
   m_pFilenameInputPort = addInputPort( "filename", KERNEL_PORT_DATA_TYPE_STRING );	

	// output port
   if( m_bIsUnsignedByte )
   {
      m_pRGBArrayOutputPort = addOutputPort( "outputRGBArray", KERNEL_PORT_DATA_TYPE_UNSIGNED_BYTE_ARRAY );
   }
   else
   {
      m_pRGBArrayOutputPort = addOutputPort( "outputRGBArray", KERNEL_PORT_DATA_TYPE_FLOAT_ARRAY );
   }
	
   m_pSizeOutputPort = addOutputPort( "size", KERNEL_PORT_DATA_TYPE_SIZE_2D );
   m_pLogLuminanceMinOutputPort = addOutputPort( "logLuminanceMin", KERNEL_PORT_DATA_TYPE_FLOAT );
   m_pLogLuminanceMaxOutputPort = addOutputPort( "logLuminanceMax", KERNEL_PORT_DATA_TYPE_FLOAT );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

FilenameToRGBArrayKernel::FilenameToRGBArrayKernel( QString args ) :

   GPUKernel( "FilenameToRGBArray" ),

// input data

// m_qsFilename is automatically the Null QString (and will be != to
// every other string)

		// video
		m_pDecoderThread( NULL ),
		m_pVideo( NULL ),

		// image
		m_aubImagePixels( NULL ),
		m_fImagePixels( NULL ),

	m_bReallocationNeeded( true ),
	m_bPaused( false ),

	m_bIsVideo( false ),
	m_bIsImageSequence( false )
{

   cout << "inside FilenameToRGBArrayKernel constructor" << endl;

   if( args == "ubyte" )
   {
      cout << "Args = ubyte " << endl;
      m_bIsUnsignedByte = true;
   }
   else if( args == "float" )
   {
      m_bIsUnsignedByte = false;
   }
   else
   {
      fprintf( stderr, "FilenameToRGBArrayKernel: args must be \"ubyte\" or \"float\"\n" );
      exit( -1 );
   }
}

void FilenameToRGBArrayKernel::cleanup()
{
   if( m_aubImagePixels != NULL )
   {
      delete[] m_aubImagePixels;
   }

   if( m_pDecoderThread != NULL )
   {
      // stop decoder
      m_pDecoderThread->stop();
      m_pDecoderThread->wait();
      delete m_pDecoderThread;
      m_pDecoderThread = NULL;

      // delete video		
      m_pVideo = NULL;
   }
}

void FilenameToRGBArrayKernel::reallocate()
{
   cout << "inside FilenameToRGBArrayKernel::reallocate()" << endl;

   // only one input port anyway
   QString filename = m_pFilenameInputPort->pullData().getFilenameData();
   cout << "filename = " << filename.toStdString() << endl;
   cout << "m_qsFilename = " << m_qsFilename.toStdString() << endl;

   if( filename != m_qsFilename )
   {		
      cleanup();

      m_qsFilename = filename;

      // TODO: break this up into separate filters
      if( isVideoFilename( filename ) )
      {
         m_pVideo = FFMPEGVideo::fromFile( filename );
         if( m_pVideo == NULL )
         {
            fprintf( stderr, "Error reading %s\n", qPrintable( filename ) );
            exit( -1 );
         }

         int64 nFrames = m_pVideo->numFrames();
         printf( "nFrames = %I64d\n", nFrames );

         m_pDecoderThread = new QDecoderThread( m_pVideo, 24, NULL );

         /*
           QObject::connect( m_pDecoderThread, SIGNAL( frameRead( int64 ) ),
           this, SLOT( handleVideoFrameRead( int64 ) ) );
         */

         m_pDecoderThread->start();

         m_qSize = QSize( m_pVideo->width(), m_pVideo->height() );
         m_nBytes = 3 * m_pVideo->width() * m_pVideo->height();

         m_bIsVideo = true;
      }
      else if( isDirectory( filename ) )
      {
         m_qsDirectoryName = filename;

         QDir directory( filename );

         QStringList nameFilters;
         nameFilters << "*.png" << "*.bmp";

         directory.setSorting( QDir::Name );
         m_qslFilenames = directory.entryList( nameFilters, QDir::Files );
         m_iImageSequenceIndex = 0;

         QString filename0 = m_qsDirectoryName + "/" + m_qslFilenames[ 0 ];
         printf( "filename0 = %s\n", qPrintable( filename0 ) );

         QImage image( filename0 );
         if( image.isNull() )
         {
            fprintf( stderr, "Error reading %s\n", qPrintable( filename0 ) );
            exit( -1 );
         }			

         m_qSize = image.size();
         m_nBytes = 3 * image.width() * image.height();

         m_aubImagePixels = new ubyte[ m_nBytes ];
         int index = 0;
         for( int y = 0; y < m_qSize.height(); ++y )	
         {
            for( int x = 0; x < m_qSize.width(); ++x )
            {
               QRgb rgb = image.pixel( x, y );

               m_aubImagePixels[ index ] = qRed( rgb );
               m_aubImagePixels[ index + 1 ] = qGreen( rgb );
               m_aubImagePixels[ index + 2 ] = qBlue( rgb );

               index += 3;
            }
         }

         m_bIsVideo = false;
         m_bIsImageSequence = true;
      }
      else
      {
         QImage image( filename );
         if( image.isNull() )
         {
            fprintf( stderr, "Error reading %s\n", qPrintable( filename ) );
            exit( -1 );
         }			

         m_qSize = image.size();
         m_nBytes = 3 * image.width() * image.height();

         m_aubImagePixels = new ubyte[ m_nBytes ];
         int index = 0;
         for( int y = 0; y < m_qSize.height(); ++y )	
         {
            for( int x = 0; x < m_qSize.width(); ++x )
            {
               QRgb rgb = image.pixel( x, y );

               m_aubImagePixels[ index ] = qRed( rgb );
               m_aubImagePixels[ index + 1 ] = qGreen( rgb );
               m_aubImagePixels[ index + 2 ] = qBlue( rgb );

               index += 3;
            }
         }

         float luminance = ColorUtils::rgb2luminance( &( m_aubImagePixels[0] ) );
         float logLuminanceMin = log10( luminance + ColorUtils::LOG_LUMINANCE_EPSILON );
         float logLuminanceMax = logLuminanceMin;

         // compute min and max
         for( int i = 3; i < 3 * image.width() * image.height(); i += 3 )
         {
            luminance = ColorUtils::rgb2luminance( &( m_aubImagePixels[i] ) );
            float currentLogLuminance = log10( luminance + ColorUtils::LOG_LUMINANCE_EPSILON );

            if( currentLogLuminance < logLuminanceMin )
            {
               logLuminanceMin = currentLogLuminance;
            }
            if( currentLogLuminance > logLuminanceMax )
            {
               logLuminanceMax = currentLogLuminance;
            }
         }

         m_fLogLuminanceMin = logLuminanceMin;
         m_fLogLuminanceMax = logLuminanceMax;

         m_pLogLuminanceMinOutputPort->pushData( KernelPortData( m_fLogLuminanceMin ) );
         m_pLogLuminanceMaxOutputPort->pushData( KernelPortData( m_fLogLuminanceMax ) );

         m_bIsVideo = false;
      }

      m_pSizeOutputPort->pushData( KernelPortData( m_qSize ) );
      
   } // filename != m_qsFilename conditional
}

bool FilenameToRGBArrayKernel::isDirectory( QString dirname )
{
   QFileInfo qfi( dirname );
   return qfi.isDir();
}

bool FilenameToRGBArrayKernel::isVideoFilename( QString filename )
{
   cout << "inside FilenameToRGBArrayKernel::isVideoFilename()" << endl;

   QString extension = filename.right( 3 );
   return( extension == "avi" ||
           extension == "mkv" ||
           extension == "mpg" ||
           extension == "wmv" ||
           extension == "mp4" ||
           extension == "mov" );
}

bool FilenameToRGBArrayKernel::isPortableFloatMapFilename( QString filename )
{
   QString extension = filename.right( 3 );
   return( extension == "pfm" );
}
