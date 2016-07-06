#ifndef RGB_2_VBO_KERNEL_H
#define RGB_2_VBO_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <common/BasicTypes.h>

#include "GPUKernel.h"

class GLBufferObject;

class RGB2VBOKernel : public GPUKernel
{
public:

	static RGB2VBOKernel* create( QString args );
	virtual ~RGB2VBOKernel();

	// TODO: use this eventually
	virtual bool isInputComplete();

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
	RGB2VBOKernel( QString args );	
	void cleanup();
	void reallocate();

	// =========================================================================
	// Fields
	// =========================================================================

	// ports
	InputKernelPort* m_pInputRGBInputPort;
	OutputKernelPort* m_pBufferObjectOutputPort;

	// input data
	ArrayWithLength< ubyte > m_aubInputRGB;
	QVector< float > m_qvfInputRGB;
	int m_nBytes;
	int m_iWidth;
	int m_iHeight;

	// output data
	GLBufferObject* m_pOutputVBO;

	// internal
	bool m_bReallocationNeeded;
	bool m_bIsUnsignedByte;
};

#endif // RGB_2_VBO_KERNEL_H
