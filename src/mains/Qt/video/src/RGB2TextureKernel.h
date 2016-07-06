#ifndef RGB_2_TEXTURE_KERNEL_H
#define RGB_2_TEXTURE_KERNEL_H

#include <common/BasicTypes.h>
#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <QSize>

#include "GPUKernel.h"

class GLTextureRectangle;

class RGB2TextureKernel : public GPUKernel
{
public:

	static RGB2TextureKernel* create( QString args );
	virtual ~RGB2TextureKernel();

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
	RGB2TextureKernel( QString args );	
	void cleanup();
	void reallocate();

	// =========================================================================
	// Fields
	// =========================================================================

	// ports
	InputKernelPort* m_pInputRGBInputPort;
	InputKernelPort* m_pSizeInputPort;
	OutputKernelPort* m_pOutputTextureOutputPort;

	// input data
	ArrayWithLength< ubyte > m_aubInputRGB;
	QVector< float > m_qvfInputRGB;
	QSize m_qSize;

	// derived data
	int m_iWidth;
	int m_iHeight;

	// output data
	GLTextureRectangle* m_pOutputTexture;

	// internal
	bool m_bReallocationNeeded;
	bool m_bIsUnsignedByte;
};

#endif // RGB_2_TEXTURE_KERNEL_H
