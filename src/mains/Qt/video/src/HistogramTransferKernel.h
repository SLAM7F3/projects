#ifndef HISTOGRAM_TRANSFER_KERNEL_H
#define HISTOGRAM_TRANSFER_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <common/BasicTypes.h>

#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;
class GLTextureRectangle;

class HistogramTransferKernel : public GPUKernel
{
public:

	static void initializeCg();
	static HistogramTransferKernel* create( QString args );
	virtual ~HistogramTransferKernel();

	// TODO: isInitialized(), don't let them initialize twice
	virtual bool isInputComplete();

	virtual void compute( QString );
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
	HistogramTransferKernel( QString args );
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
	CGparameter m_cgp_F_inputLabASampler;
	CGparameter m_cgp_F_inputCDFSampler;
	CGparameter m_cgp_F_modelInverseCDFSampler;
	CGparameter m_cgp_F_f2InputCDFNumBinsModelInverseCDFNumBins;

	/*
	uniform samplerRECT inputLabASampler,
	uniform samplerRECT inputCDF,
	uniform samplerRECT modelInverseCDF,
	uniform float2 inputCDFNumBinsModelInverseCDFNumBins,
	*/

	/*
	CGpass m_cgPass;
	CGparameter m_cgParameterModelViewProjection;
	CGparameter m_cgParameter_samplerInputLab;
	CGparameter m_cgParameter_samplerInputCDF;
	CGparameter m_cgParameter_samplerInputModelInverseCDF;
	*/

	// Input ports
	InputKernelPort* m_pInputTextureInputPort;
	InputKernelPort* m_pInputCDFInputPort;

	// Output ports
	OutputKernelPort* m_pOutputPort;

	// Output data
	GLTextureRectangle* m_pOutputTexture;

	// input data
	GLTextureRectangle* m_pInputTexture;
	GLTextureRectangle* m_pInputCDF;

	// derived data
	int m_iInputWidth;
	int m_iInputHeight;	

	// internal	
	bool m_bReallocationNeeded;

	bool m_bIsAbsoluteValue;
	int m_iModelInverseCDFNumBins;
	float* m_afModelInverseCDFPixels;
	GLTextureRectangle* m_pModelInverseCDF;
};

#endif // HISTOGRAM_TRANSFER_KERNEL_H
