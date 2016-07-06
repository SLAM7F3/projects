#ifndef BILATERAL_KERNEL_H
#define BILATERAL_KERNEL_H

#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;
class GLVertexBufferObject;

class BilateralKernel : public GPUKernel
{
public:

	static void initializeCg();
	static BilateralKernel* create( QString args );
	virtual ~BilateralKernel();

	virtual void makeDirty( QString inputPortName );
	virtual void compute( QString outputPortName );

protected:

	// =========================================================================
	// Methods
	// =========================================================================

	virtual bool isInputComplete();

	// ---- Initialization ----	
	virtual void initializeGL();
	virtual void initializePorts();	

private:

	// =========================================================================
	// Methods
	// =========================================================================

	BilateralKernel( QString args );
	void deleteInternalVBOs();
	void deleteOutputTexture();
	void reallocate();

	void initializeCgPrograms();
	
	// ---- Passes ----
	void pointScatterPass();
	void blurXPass();
	void blurYPass();
	void blurZDecayDividePass();
	void slicePass();

	// ---- Reallocation Helper Methods ----
	void reallocateOutputTexture();
	void reallocateInternalVBOs();

	// =========================================================================
	// Fields
	// =========================================================================

	static bool s_bCgInitialized; 

	// ---- FBO ----
	GLFramebufferObject* m_pFBO;

	// ---- Input Ports ----
	InputKernelPort* m_pDropSamplesInputPort;
	InputKernelPort* m_pUseRandomPatternInputPort;
	InputKernelPort* m_pSigmaSpatialInputPort;
	InputKernelPort* m_pSigmaRangeInputPort;
	InputKernelPort* m_pFractionPixelsDroppedInputPort;
	InputKernelPort* m_pTemporalDecayLambdaInputPort;
	InputKernelPort* m_pInputTextureInputPort;
	InputKernelPort* m_pInputMinInputPort;
	InputKernelPort* m_pInputMaxInputPort;

	// ---- Output Ports ----
	OutputKernelPort* m_pBilateralFilterOutputPort;
	// OutputKernelPort* m_pRandomPatternTextureOutputPort;

	// ---- Input Data ----
	float m_fSigmaSpatial;
	float m_fSigmaRange;
	float m_fTemporalDecayLambda;
	GLTextureRectangle* m_pInputTexture;
	float m_fInputMin;
	float m_fInputMax;
	float m_fInputDelta;

	// ---- Output Data ----
	GLTextureRectangle* m_pOutputTexture;

	// ---- Derived Data ----
	int m_nOutputs;
	int m_iInputWidth;
	int m_iInputHeight;
	int m_nInputPixels;

	int m_iGridWidth;
	int m_iGridHeight;
	int m_iGridDepth;
	int m_iGridTextureWidth;
	int m_iGridTextureHeight;

	GLTextureRectangle* m_pTempGrid0;
	GLTextureRectangle* m_pTempGrid1;
	GLTextureRectangle* m_pPreviousGrid; // for exponential decay
	// ---- Internal Parameters ----

	// constants
	int m_iInternalTextureWidth;
	int m_iInternalTextureHeight;
	
	// -- Internal VBOs --
	GLBufferObject* m_pInputXYCoordinateVBO;

	// ---- Cg ----
	CgProgramWrapper* m_pPointScatterVertexProgram;
		CGparameter m_cgp_PSV_f44_mvp;
		CGparameter m_cgp_PSV_f3_rcpSigma;
		CGparameter m_cgp_PSV_f3_gridSize;
		CGparameter m_cgp_PSV_f_inputMin;
		CGparameter m_cgp_PSV_inputSampler;

	CgProgramWrapper* m_pPointScatterFragmentProgram;
	
	CgProgramWrapper* m_pGaussianBlurLineProgram;
		CGparameter m_cgp_GBL_gridSampler;
		CGparameter m_cgp_GBL_f2Delta;
		CGparameter m_cgp_GBL_f2TwoDelta;	

	CgProgramWrapper* m_pGaussianBlurZLineExponentialDecayAndDivideProgram;
		CGparameter m_cgp_GBZLEDD_gridSampler;
		CGparameter m_cgp_GBZLEDD_previousGridSampler;
		CGparameter m_cgp_GBZLEDD_f2Delta;
		CGparameter m_cgp_GBZLEDD_f2TwoDelta;
		CGparameter m_cgp_GBZLEDD_f2TemporalDecayLambdaNormalization;

	CgProgramWrapper* m_pSliceProgram;
		CGparameter m_cgp_S_inputSampler;
		CGparameter m_cgp_S_quotientGridSampler;
		CGparameter m_cgp_S_f3_rcpSigma;
		CGparameter m_cgp_S_f3_gridSize;
		CGparameter m_cgp_S_f_inputMin;

	// internal
	bool m_bReallocationNeeded;	
	int m_iPadding;

	// subsampling
	bool m_bUseSubsampling;
	int m_nPatterns;
	int m_nPixelsPerPattern;
	int m_iCurrentPattern;
};

#endif
