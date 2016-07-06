#ifndef STYLIZE_KERNEL_H
#define STYLIZE_KERNEL_H

#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;

class StylizeKernel : public GPUKernel
{
public:
	
	static void initializeCg();
	static StylizeKernel* create( QString args );
	virtual ~StylizeKernel();

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

	StylizeKernel();
	void reallocate();	
	void deleteOutputTexture();	
	
	void initializeCgPrograms();

	void rescaleRemappingCurves();
	void deleteRemappingCurveData();

	// =========================================================================
	// Fields
	// =========================================================================

	static bool s_bCgInitialized;

	// shared
	GLFramebufferObject* m_pFBO;

	// Cg
	CgProgramWrapper* m_pFragmentProgram;
		CGparameter m_cgp_inputRGBASampler;
		CGparameter m_cgp_bfDenoisedLabASampler;
		CGparameter m_cgp_bfBaseLabASampler;
		CGparameter m_cgp_remappingCurveSampler;
		CGparameter m_cgp_fRemappingCurveWidth;
		CGparameter m_cgp_fLambdaDetail;
		CGparameter m_cgp_fDetailMax;

	// Input Ports
	InputKernelPort* m_pInputRGBTextureInputPort;
	InputKernelPort* m_pBFDenoisedTextureInputPort;
	InputKernelPort* m_pBFBaseTextureInputPort;

	InputKernelPort* m_pLambdaDetailInputPort;
	InputKernelPort* m_pBaseRemappingCurveInputPort;
	InputKernelPort* m_pDetailRemappingCurveInputPort;

	// Output Ports
	OutputKernelPort* m_pOutputTextureOutputPort;

	// Input data	
	GLTextureRectangle* m_pInputRGBTexture;
	GLTextureRectangle* m_pBFDenoisedLabTexture;
	GLTextureRectangle* m_pBFBaseLabTexture;
	QVector< float > m_qvBaseRemappingCurve;
	QVector< float > m_qvDetailRemappingCurve;
	ubyte* m_aubScaledRemappingCurve;

	float m_fLambda;

	// Derived data
	int m_iWidth;
	int m_iHeight;
	int m_iRemappingCurveLength;
	GLTextureRectangle* m_pRemappingCurveTexture;

	// Output data
	GLTextureRectangle* m_pOutputTexture;

	// internal
	bool m_bReallocationNeeded;
	bool m_bBaseCurveTextureUpdateNeeded;
	bool m_bDetailCurveTextureUpdatedNeeded;
};

#endif
