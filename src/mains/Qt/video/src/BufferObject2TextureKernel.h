#ifndef BUFFER_OBJECT_2_TEXTURE_KERNEL_H
#define BUFFER_OBJECT_2_TEXTURE_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <common/BasicTypes.h>

#include "GPUKernel.h"

class GLTextureRectangle;
class GLVertexBufferObject;

// Input: OpenGL buffer object, width, height
// Output: A ( width x height ) 2D OpenGL texture
class BufferObject2TextureKernel : public GPUKernel
{
public:

	static BufferObject2TextureKernel* create( QString args );
	virtual ~BufferObject2TextureKernel();

	// TODO: isInitialized(), don't let them initialize twice
	virtual bool isInputComplete();

	virtual void compute( QString );
	virtual void makeDirty( QString inputPortName );	

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
	BufferObject2TextureKernel( QString args );
	void deleteOutputTexture();	
	void reallocate();

	// =========================================================================
	// Fields
	// =========================================================================	

	// Input ports
	InputKernelPort* m_pSizePort;
	InputKernelPort* m_pBufferObjectInputPort;

	// Output ports
	OutputKernelPort* m_pOutputPort;

	// Output data
	GLTextureRectangle* m_pOutputTexture;

	// derived data
	QSize m_qSize;

	// internal
	bool m_bReallocationNeeded;
	bool m_bIsUnsignedByte;
};

#endif // BUFFER_OBJECT_2_TEXTURE_KERNEL_H
