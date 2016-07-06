#ifndef COMBINE_TEXTURENESS_KERNEL_H
#define COMBINE_TEXTURENESS_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;

class CombineTexturenessKernel : public GPUKernel
{
public:

	static void initializeCg();
	static CombineTexturenessKernel* create( QString args );
	virtual ~CombineTexturenessKernel();

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

	CombineTexturenessKernel();
	void deleteOutputTexture();
	void reallocate();

	void initializeCgPrograms();

	// =========================================================================
	// Fields
	// =========================================================================

	static bool s_bCgInitialized;

	// shared
	GLFramebufferObject* m_pFBO;

	// Cg
	CgProgramWrapper* m_pFragmentProgram;
		CGparameter m_cgp_F_basePrimeSampler;
		CGparameter m_cgp_F_detailSampler;
		CGparameter m_cgp_F_texturenessPrimeSampler;
		CGparameter m_cgp_F_texturenessBasePrimeSampler;
		CGparameter m_cgp_F_texturenessDetailSampler;

	// input port
	InputKernelPort* m_pBasePrimeInputPort;
	InputKernelPort* m_pDetailInputPort;
	InputKernelPort* m_pTexturenessPrimeInputPort;
	InputKernelPort* m_pTexturenessBasePrimeInputPort;
	InputKernelPort* m_pTexturenessDetailInputPort;

	// output port
	OutputKernelPort* m_pOutputTextureOutputPort;

	// input data
	GLTextureRectangle* m_pBasePrimeTexture;
	GLTextureRectangle* m_pDetailTexture;
	GLTextureRectangle* m_pTexturenessPrimeTexture;
	GLTextureRectangle* m_pTexturenessBasePrimeTexture;
	GLTextureRectangle* m_pTexturenessDetailTexture;

	// output data
	GLTextureRectangle* m_pOutputTexture;	

	// derived parameters
	int m_iInputWidth;
	int m_iInputHeight;

	// internal
	bool m_bReallocationNeeded;
};

#endif // COMBINE_TEXTURENESS_KERNEL_H
