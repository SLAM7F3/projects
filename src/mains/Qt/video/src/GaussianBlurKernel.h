#ifndef GAUSSIAN_BLUR_KERNEL_H
#define GAUSSIAN_BLUR_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;

class GaussianBlurKernel : public GPUKernel
{
public:

	static void initializeCg();
	static GaussianBlurKernel* create( QString args );
	virtual ~GaussianBlurKernel();

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

	GaussianBlurKernel();
	void cleanup();
	void reallocate();

	void initializeCgPrograms();

	void downsamplePass();
	void blurPass();
	void upsamplePass();

	// =========================================================================
	// Fields
	// =========================================================================

	static bool s_bCgInitialized;

	// shared
	GLFramebufferObject* m_pFBO;

	// Cg
#if 0
	CgProgramWrapper* m_pDownsampleFragmentProgram;
	CGparameter m_cgp_downsample_inputSampler;
	CGparameter m_cgp_downsample_sigma;

	CgProgramWrapper* m_pBlurFragmentProgram;
	CGparameter m_cgp_blur_inputSampler;
	CGparameter m_cgp_blur_delta_twoDelta;

	CgProgramWrapper* m_pUpsampleFragmentProgram;
	CGparameter m_cgp_upsample_inputSampler;
	CGparameter m_cgp_upsample_downsampledSampler;
	CGparameter m_cgp_upsample_fSigma;
#endif

	CgProgramWrapper* m_pGaussianBlurFragmentProgram;
	CGparameter m_cgp_gb_inputSampler;
	CGparameter m_cgp_gb_f2_Delta;
	CGparameter m_cgp_gb_f2_sigmaTwoSigmaSquared;

	CgProgramWrapper* m_pAbsHighPassFragmentProgram;
	CGparameter m_cgp_ahp_inputSampler;
	CGparameter m_cgp_ahp_blurredOnceSampler;
	CGparameter m_cgp_ahp_f2_Delta;
	CGparameter m_cgp_ahp_f2_sigmaTwoSigmaSquared;

	// input port
	InputKernelPort* m_pInputTextureInputPort;
	InputKernelPort* m_pSigmaInputPort;

	// output port
	OutputKernelPort* m_pOutputTextureOutputPort;

	// input data
	GLTextureRectangle* m_pInputTexture;
	float m_fSigma;

	// temp data
	// downsample to temp0
	// blurX to temp1
	// blurY back to temp0
	GLTextureRectangle* m_pTempTexture0;
	GLTextureRectangle* m_pTempTexture1;

	// output data
	GLTextureRectangle* m_pOutputTexture;	

	// derived parameters
	int m_iInputWidth;
	int m_iInputHeight;
	int m_iDownsampledWidth;
	int m_iDownsampledHeight;

	// internal
	bool m_bReallocationNeeded;
	int m_iPadding;
};

#endif // GAUSSIAN_BLUR_KERNEL_H
