#ifndef CDF_KERNEL_H
#define CDF_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <common/BasicTypes.h>

#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;
class GLTextureRectangle;

class CDFKernel : public GPUKernel
{
public:

	static void initializeCg();
	static CDFKernel* create( QString args );
	virtual ~CDFKernel();

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
	CDFKernel( QString args );
	void deleteOutputTexture();	
	void reallocate();

	void initializeCgPrograms();

	// =========================================================================
	// Fields
	// =========================================================================	

	static bool s_bCgInitialized;

	// Cg
	CgProgramWrapper* m_pFragmentProgram;
	CGparameter m_cgp_F_histogramSampler;

	// Input ports
	InputKernelPort* m_pInputTextureInputPort;

	// Output ports
	OutputKernelPort* m_pOutputPort;

	// Output data
	GLTextureRectangle* m_pOutputTexture;

	// input data	
	GLTextureRectangle* m_pInputTexture;

	// derived data
	int m_nBins;

	// internal
	GLFramebufferObject* m_pFBO;
	bool m_bReallocationNeeded;
};

#endif // CDF_KERNEL_H