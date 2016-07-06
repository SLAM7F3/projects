#ifndef FILENAME_TO_RGB_ARRAY_KERNEL_H
#define FILENAME_TO_RGB_ARRAY_KERNEL_H

#include <QStringList>
#include <video/FFMPEGVideo.h>

#include "GPUKernel.h"

class QDecoderThread;

class FilenameToRGBArrayKernel : public GPUKernel
{
  public:

   static FilenameToRGBArrayKernel* create( QString args );
   virtual ~FilenameToRGBArrayKernel();

	// TODO: isInitialized(), don't let them initialize twice
   virtual bool isInputComplete();

   bool isVideo();
   bool isImageSequence();
   bool isPaused();
   void togglePause();

   Reference< IVideo > getVideo();
   QDecoderThread* getDecoderThread();
	
   virtual void makeDirty( QString inputPortName );
   virtual void compute( QString outputPortName );

  protected:

// =========================================================================
// Methods
// =========================================================================

	// ---- Initialization ----		
   virtual void initializeGL();
   virtual void initializePorts();	

  private:

// =========================================================================
// Methods
// =========================================================================

   FilenameToRGBArrayKernel( QString args );
   void cleanup();
   void reallocate();

   bool isDirectory( QString dirname );
   bool isVideoFilename( QString filename );	
   bool isPortableFloatMapFilename( QString filename );

// =========================================================================
// Fields
// =========================================================================

	// Input ports
   InputKernelPort* m_pFilenameInputPort;	

   // Input Data

		// video or still image
   QString m_qsFilename;

   // image sequence
   QString m_qsDirectoryName;
   QStringList m_qslFilenames;
   int m_iImageSequenceIndex;

   // video
   QDecoderThread* m_pDecoderThread;
   Reference< IVideo > m_pVideo;

		// pixels
   ubyte* m_aubImagePixels;
   float* m_fImagePixels;
   QVector< float > m_qvFloatImagePixels;

   // Output ports
   OutputKernelPort* m_pRGBArrayOutputPort;	
   OutputKernelPort* m_pSizeOutputPort;
   OutputKernelPort* m_pLogLuminanceMinOutputPort;
   OutputKernelPort* m_pLogLuminanceMaxOutputPort;

   // Derived data
   bool m_bIsVideo;
   bool m_bIsImageSequence;

   bool m_bPaused;
   int m_nBytes;
   QSize m_qSize;
   float m_fLogLuminanceMin;
   float m_fLogLuminanceMax;

   // internal
   bool m_bReallocationNeeded;
   bool m_bIsUnsignedByte;
};

#endif // FILENAME_TO_RGB_ARRAY_KERNEL_H
