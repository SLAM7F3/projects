#ifndef CROSS_BILATERAL_KERNEL_H
#define CROSS_BILATERAL_KERNEL_H

#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;
class GLVertexBufferObject;

// args: inputColorSpace outputColorSpace outputDomain
// inputColorSpace: rgb or lab
// outputColorSpace: luminance or lab
// outputDomain: linear or log
// not all combinations are possible
// lab can only go to lab
// rgb can go to either luminance of lab
class CrossBilateralKernel : public GPUKernel
{
public:

	static void initializeCg();
	static CrossBilateralKernel* create( QString args );
	virtual ~CrossBilateralKernel();

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

	CrossBilateralKernel( QString args );
	void deleteInternalVBOs();
	void deleteOutputTexture();
	void reallocate();

	void initializeCgPrograms();

	// ---- Passes ----
	void pointScatterPass();
	void blurXPass();
	void blurYPass();
	void blurZAndDividePass();
	void slicePass();

	void upsampleTiledPass( int level );
	void upsampleTiledPassCore( GLTextureRectangle* pQuotient, GLTextureRectangle* pOutput, CGpass pass );

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
	InputKernelPort* m_pSigmaSpatialInputPort;
	InputKernelPort* m_pSigmaRangeInputPort;
	InputKernelPort* m_pEdgeMinInputPort;
	InputKernelPort* m_pEdgeMaxInputPort;
	InputKernelPort* m_pEdgeTextureInputPort;
	InputKernelPort* m_pDataTextureInputPort;

	// ---- Output Ports ----
	OutputKernelPort* m_pOutputPort;

	// ---- Input Data ----
	float m_fSigmaSpatial;
	float m_fSigmaRange;
	GLTextureRectangle* m_pInputEdgeTexture;
	GLTextureRectangle* m_pInputDataTexture;
	float m_fEdgeMin;
	float m_fEdgeMax;
	float m_fEdgeDelta;

	// ---- Output Data ----
	GLTextureRectangle* m_pOutputTexture;

	// ---- Derived Data ----
	int m_iInputWidth;
	int m_iInputHeight;
	int m_nInputPixels;

	int m_iGridWidth;
	int m_iGridHeight;
	int m_iGridDepth;

	// how much we actually need
	int m_iGridTextureWidth;
	int m_iGridTextureHeight;

	// ---- Internal Parameters ----
	GLTextureRectangle* m_pTempGrid0;
	GLTextureRectangle* m_pTempGrid1;

	// how big the actual allocated texture is
	int m_iInternalTextureWidth;
	int m_iInternalTextureHeight;

	// -- Internal VBOs --
	GLBufferObject* m_pInputXYCoordinateVBO;

	// ---- Cg ----
	CgProgramWrapper* m_pPointScatterVertexProgram;
	CGparameter m_cgp_PSV_f44_mvp;
	CGparameter m_cgp_PSV_f3_rcpSigma;
	CGparameter m_cgp_PSV_f3_gridSize;
	CGparameter m_cgp_PSV_f_edgeMin;
	CGparameter m_cgp_PSV_edgeSampler;
	CGparameter m_cgp_PSV_dataSampler;

	CgProgramWrapper* m_pPointScatterFragmentProgram;

	CgProgramWrapper* m_pGaussianBlurLineProgram;
	CGparameter m_cgp_GBL_gridSampler;
	CGparameter m_cgp_GBL_f2Delta;
	CGparameter m_cgp_GBL_f2TwoDelta;	

	CgProgramWrapper* m_pGaussianBlurZLineAndDivideProgram;
	CGparameter m_cgp_GBZLDD_gridSampler;
	CGparameter m_cgp_GBZLDD_f2Delta;
	CGparameter m_cgp_GBZLDD_f2TwoDelta;

	CgProgramWrapper* m_pSliceProgram;
	CGparameter m_cgp_S_edgeSampler;
	CGparameter m_cgp_S_quotientGridSampler;
	CGparameter m_cgp_S_f3_rcpSigma;
	CGparameter m_cgp_S_f3_gridSize;
	CGparameter m_cgp_S_f_edgeMin;

	// internal
	bool m_bReallocationNeeded;
	int m_iPadding;
};

#endif
