#ifndef LINEAR_COMBINATION_KERNEL_H
#define LINEAR_COMBINATION_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;

// args are scales
// and are of the form: x0 y0 z0 w0 x1 y1 z1 w2 ...
// which also determines the number of actual ports
class LinearCombinationKernel : public GPUKernel
{
public:

	static void initializeCg();
	static LinearCombinationKernel* create( QString args );
	virtual ~LinearCombinationKernel();

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

	LinearCombinationKernel( QString args );
	void deleteSharedParameters();
	void deleteOutputTexture();
	void reallocate();

	void initializeCgPrograms();
	float parseFloatOrExit( QString stringToken );

	// =========================================================================
	// Fields
	// =========================================================================

	static bool s_bCgInitialized;

	// shared
	GLFramebufferObject* m_pFBO;

	// Cg
	CgProgramWrapper* m_pFragmentProgram;
		CGparameter m_cgp_F_inputSamplersArray; // unsized
		CGparameter m_cgp_F_f4_scalesArray; // unsized

		CGparameter m_cgp_shared_inputSamplersArray;
		CGparameter m_cgp_shared_f4_scalesArray;

	// input ports
	QVector< InputKernelPort* > m_qvInputTexturesInputPort;

	// output port
	OutputKernelPort* m_pOutputTextureOutputPort;

	// input data
	QVector< GLTextureRectangle* > m_qvInputTextures;

	// output data
	GLTextureRectangle* m_pOutputTexture;	

	// derived parameters
	int m_iInputWidth;
	int m_iInputHeight;

	// internal
	bool m_bReallocationNeeded;
	int m_nInputTextures;
	QVector< float > m_qvScaleXs;
	QVector< float > m_qvScaleYs;
	QVector< float > m_qvScaleZs;
	QVector< float > m_qvScaleWs;
};

#endif // LINEAR_COMBINATION_KERNEL_H
