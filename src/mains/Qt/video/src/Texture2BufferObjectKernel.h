#ifndef TEXTURE_2_BUFFER_OBJECT_2_KERNEL_H
#define TEXTURE_2_BUFFER_OBJECT_2_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <common/BasicTypes.h>

#include "GPUKernel.h"

class GLFramebufferObject;
class GLTextureRectangle;
class GLVertexBufferObject;

// Input: OpenGL buffer object, width, height
// Output: A ( width x height ) 2D OpenGL texture
class Texture2BufferObjectKernel : public GPUKernel
{
public:

	static Texture2BufferObjectKernel* create( QString args );
	virtual ~Texture2BufferObjectKernel();

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
	Texture2BufferObjectKernel( QString args );
	void deleteOutputBufferObject();	
	void reallocate();

	// =========================================================================
	// Fields
	// =========================================================================	

	GLFramebufferObject* m_pFBO;

	// Input ports
	InputKernelPort* m_pInputTextureInputPort;

	// Output ports
	OutputKernelPort* m_pOutputPort;

	// Input data
	GLTextureRectangle* m_pInputTexture;
	int m_iWidth;
	int m_iHeight;

	// Output data
	GLBufferObject* m_pOutputBufferObject;

	// internal
	bool m_bReallocationNeeded;
	bool m_bIsUnsignedByte;
};

#endif // TEXTURE_2_BUFFER_OBJECT_2_KERNEL_H
