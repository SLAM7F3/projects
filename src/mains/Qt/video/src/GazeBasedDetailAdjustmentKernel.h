#ifndef GAZE_BASED_DETAIL_ADJUSTMENT_KERNEL_H
#define GAZE_BASED_DETAIL_ADJUSTMENT_KERNEL_H

#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;

class GazeBasedDetailAdjustmentKernel : public GPUKernel
{
public:

	static void initializeCg();
	static GazeBasedDetailAdjustmentKernel* create( QString args );
	virtual ~GazeBasedDetailAdjustmentKernel();

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
	
	GazeBasedDetailAdjustmentKernel();
	
	void initializeCgPrograms();

	void deleteOutputTextures();
	void reallocate();

	void combineBFPass();
	void detailMapPass();

	// =========================================================================
	// Fields
	// =========================================================================

	static bool s_bCgInitialized;

	// shared
	GLFramebufferObject* m_pFBO;

	// Cg
	CgProgramWrapper* m_pCombineBFFragmentProgram;
		CGparameter m_cgp_CBF_inputRGBASampler;
		CGparameter m_cgp_CBF_bf0Sampler;
		CGparameter m_cgp_CBF_bf1Sampler;
		CGparameter m_cgp_CBF_bf2Sampler;
		CGparameter m_cgp_CBF_detailMapSampler;

	CgProgramWrapper* m_pOverlayDetailMapOverOriginalFragmentProgram;
		CGparameter m_cgp_ODMOO_inputRGBASampler;
		CGparameter m_cgp_ODMOO_detailMapSampler;
		CGparameter m_cgp_ODMOO_fDetailOverlayAlpha;

	// Input Ports
	InputKernelPort* m_pInputTextureInputPort;
	InputKernelPort* m_pBilateralFilterTexturesInputPort;
	InputKernelPort* m_pDetailMapInputPort;
	InputKernelPort* m_pDetailOverlayAlphaInputPort;

	// Output ports
	OutputKernelPort* m_pOutputTextureOutputPort;
	OutputKernelPort* m_pOutputOverlayOutputPort;

	// Input data
	int m_iWidth;
	int m_iHeight;
	GLTextureRectangle* m_pInputTexture;
	QVector< GLTextureRectangle* > m_qvBFTextures;
	GLTextureRectangle* m_pDetailMapTexture;

	// Output data
	GLTextureRectangle* m_pOutputTexture;
	GLTextureRectangle* m_pOutputOverlayTexture;

	// internal
	bool m_bReallocationNeeded;
	int m_iMouseMovedX;
	int m_iMouseMovedY;
};

#endif // GAZE_BASED_DETAIL_ADJUSTMENT_KERNEL_H
