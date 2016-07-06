#ifndef COMBINE_QUANTIZED_LUMINANCE_AND_DOG_EDGES_KERNEL_H
#define COMBINE_QUANTIZED_LUMINANCE_AND_DOG_EDGES_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;

class CombineQuantizedLuminanceAndDoGEdgesKernel : public GPUKernel
{
public:

	static void initializeCg();
	static CombineQuantizedLuminanceAndDoGEdgesKernel* create( QString args );
	virtual ~CombineQuantizedLuminanceAndDoGEdgesKernel();

	// TODO: use this eventually
	virtual bool isInputComplete();

	virtual void compute( QString outputPortName );
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

	CombineQuantizedLuminanceAndDoGEdgesKernel();
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
		CGparameter m_cgp_quantizedLabAInputSampler;
		CGparameter m_cgp_dogEdgesInputSampler;
		CGparameter m_cgp_detailMapSampler;
		CGparameter m_cgp_f2GammaSaturation;

	// input ports
	InputKernelPort* m_pQuantizedLuminanceInputPort;
	InputKernelPort* m_pDoGEdgesInputPort;
	InputKernelPort* m_pDetailMapInputPort;

	InputKernelPort* m_pGammaInputPort;
	InputKernelPort* m_pSaturationInputPort;

	// output port
	OutputKernelPort* m_pOutputTextureOutputPort;

	// input data
	GLTextureRectangle* m_pQuantizedLuminanceInputTexture;
	GLTextureRectangle* m_pDoGEdgesInputTexture;
	GLTextureRectangle* m_pDetailMapTexture;

	// output data
	GLTextureRectangle* m_pOutputTexture;

	// derived parameters
	int m_iInputWidth;
	int m_iInputHeight;

	// internal
	bool m_bReallocationNeeded;
};

#endif // COMBINE_QUANTIZED_LUMINANCE_AND_DOG_EDGES_KERNEL_H
