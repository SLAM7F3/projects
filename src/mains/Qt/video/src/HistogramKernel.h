#ifndef HISTOGRAM_KERNEL_H
#define HISTOGRAM_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <common/BasicTypes.h>

#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;
class GLTextureRectangle;

class HistogramKernel : public GPUKernel
{
public:

	static void initializeCg();

	static HistogramKernel* create( QString args );
	virtual ~HistogramKernel();

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
	HistogramKernel( QString args );
	void deleteOutputTexture();	
	void reallocate();

	void initializeCgPrograms();

	// =========================================================================
	// Fields
	// =========================================================================	

	static bool s_bCgInitialized;

	// Cg
	CgProgramWrapper* m_pVertexProgram;
	CGparameter m_cgp_V_mvp;
	CGparameter m_cgp_V_fNumBins;
	CGparameter m_cgp_V_fReciprocalNumPixels;
	CGparameter m_cgp_V_inputLabASampler;

	CgProgramWrapper* m_pFragmentProgram;

	// Input ports
	InputKernelPort* m_pNumBinsInputPort;
	InputKernelPort* m_pSizeInputPort;
	InputKernelPort* m_pInputTextureInputPort;

	// Output ports
	OutputKernelPort* m_pOutputPort;

	// Output data
	GLTextureRectangle* m_pOutputTexture;

	// input data
	GLTextureRectangle* m_pInputTexture;
	int m_nBins;

	// derived data
	int m_iInputWidth;
	int m_iInputHeight;
	int m_nPixels;

	// internal
	GLFramebufferObject* m_pFBO;
	bool m_bReallocationNeeded;
	bool m_bIsAbsoluteValue;
	GLBufferObject* m_pXYCoordinateVBO;
};

#endif // HISTOGRAM_KERNEL_H
