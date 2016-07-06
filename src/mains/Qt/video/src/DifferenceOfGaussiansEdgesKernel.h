#ifndef DIFFERENCE_OF_GAUSSIANS_EDGES_KERNEL_H
#define DIFFERENCE_OF_GAUSSIANS_EDGES_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>

#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;

class DifferenceOfGaussiansEdgesKernel : public GPUKernel
{
public:

	static void initializeCg();
	static DifferenceOfGaussiansEdgesKernel* create( QString args );
	virtual ~DifferenceOfGaussiansEdgesKernel();

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

	DifferenceOfGaussiansEdgesKernel();
	void deleteTextures();
	void reallocate();

	void initializeCgPrograms();
	void initializeCgPasses();
	void initializeCgParameters();

	// passes
	void blurPass( float sigma,
		GLTextureRectangle* pInput, GLTextureRectangle* pOutput,
		float deltaX, float deltaY );
	void blurYSubtractAndSmoothStepPass( float sigma,
		GLTextureRectangle* pInputLargeXBlurred,
		GLTextureRectangle* pInputSmallBlurred,
		GLTextureRectangle* pOutput,
		float deltaX, float deltaY );

	// =========================================================================
	// Fields
	// =========================================================================

	static bool s_bCgInitialized;
	
	// shared FBO
	GLFramebufferObject* m_pFBO;

	// Cg
	CgProgramWrapper* m_pGaussianBlurFragmentProgram;
		CGparameter m_cgp_GB_inputLabASampler;
		CGparameter m_cgp_GB_f2Delta;
		CGparameter m_cgp_GB_f2SigmaTwoSigmaSquared;

	CgProgramWrapper* m_pGaussianBlurSubtractAndSmoothStepFragmentProgram;
		CGparameter m_cgp_GBSSS_largeXBlurredLabASampler;
		CGparameter m_cgp_GBSSS_smallBlurredLabASampler;
		CGparameter m_cgp_GBSSS_f2Delta;
		CGparameter m_cgp_GBSSS_f2SigmaTwoSigmaSquared;
		CGparameter m_cgp_GBSSS_f2TauPhi;

	// ports
	InputKernelPort* m_pInputTextureInputPort;
	InputKernelPort* m_pSigmaEInputPort;
	InputKernelPort* m_pPhiInputPort;
	InputKernelPort* m_pTauInputPort;
	OutputKernelPort* m_pOutputTextureOutputPort;	

	// input data
	GLTextureRectangle* m_pInputTexture;
	float m_fSigmaE;
	float m_fTau;
	float m_fPhi;

	// output data
	GLTextureRectangle* m_pOutputTexture;	

	// Internal Textures
	GLTextureRectangle* m_pInternalTempTexture0;
	GLTextureRectangle* m_pInternalTempTexture1;

	// derived parameters
	int m_iInputWidth;
	int m_iInputHeight;

	// internal
	bool m_bReallocationNeeded;	
};

#endif // DIFFERENCE_OF_GAUSSIANS_EDGES_KERNEL_H
