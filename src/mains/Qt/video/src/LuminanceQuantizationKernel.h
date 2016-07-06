#ifndef LUMINANCE_QUANTIZATION_KERNEL_H
#define LUMINANCE_QUANTIZATION_KERNEL_H

#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;

class LuminanceQuantizationKernel : public GPUKernel
{
public:

	static void initializeCg();
	static LuminanceQuantizationKernel* create( QString args );
	virtual ~LuminanceQuantizationKernel();

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

	LuminanceQuantizationKernel();
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
	CgProgramWrapper* m_pQuantizeLuminanceFragmentProgram;
		CGparameter m_cgp_QL_inputLabASampler;
		CGparameter m_cgp_QL_fNumBins;
		CGparameter m_cgp_QL_fBinMin;
		CGparameter m_cgp_QL_fBinSize;
		CGparameter m_cgp_QL_f2SharpnessAB;

	// input ports
	InputKernelPort* m_pInputTextureInputPort;
	InputKernelPort* m_pNumBinsInputPort;
	InputKernelPort* m_pSharpnessSlopeInputPort;
	InputKernelPort* m_pSharpnessOffsetInputPort;

	// output port
	OutputKernelPort* m_pOutputTextureOutputPort;

	// input data
	GLTextureRectangle* m_pInputTexture;

	// output data
	GLTextureRectangle* m_pOutputTexture;

	// derived parameters
	int m_iInputWidth;
	int m_iInputHeight;

	// internal
	bool m_bReallocationNeeded;
};

#endif // LUMINANCE_QUANTIZATION_KERNEL_H
