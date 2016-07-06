#ifndef RGB_2_LAB_KERNEL_H
#define RGB_2_LAB_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;

class RGB2LabKernel : public GPUKernel
{
public:

	static void initializeCg();
	static RGB2LabKernel* create( QString args );
	virtual ~RGB2LabKernel();

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

	RGB2LabKernel();
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
	CGparameter m_cgp_F_inputSampler;

	// input port
	InputKernelPort* m_pInputTextureInputPort;

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

#endif // RGB_2_LAB_KERNEL_H
