#ifndef PER_PIXEL_PROCESSOR_KERNEL_H
#define PER_PIXEL_PROCESSOR_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <QStringList>

#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;

// Performs a composition of per pixel operations
// over each sampler
// and performs a weighted sum over the results
// given functions f1, f2, ..., fn
// scales a1, a2, ..., am
// and inputs x1, x2, ..., xm
// output y = sum( a_i * fn( fn-1( ... ( f2( f1( xi ) ) ) ) ), i from 1 to m )
class PerPixelProcessorKernel : public GPUKernel
{
public:

	static PerPixelProcessorKernel* create( QString args );
	virtual ~PerPixelProcessorKernel();

	// TODO: allow null for processor or scale to give identity

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

	PerPixelProcessorKernel( int instanceId, QString args );
	void deleteSharedParameters();
	void deleteOutputTexture();
	void reallocate();

	void initializeCgPrograms();
		void instantiateProgram();
		void instantiateSamplersAndScales();
		void instantiateProcessors();
	float parseFloatOrExit( QString stringToken );

	// =========================================================================
	// Fields
	// =========================================================================

	static int s_nInstances;

	// instance id
	int m_iInstanceId;
	QString m_qsProgramKey;

	// shared
	GLFramebufferObject* m_pFBO;

	// Cg
	CgProgramWrapper* m_pFragmentProgram;
		CGparameter m_cgp_F_inputSamplersArray; // unsized
		CGparameter m_cgp_F_f4_scalesArray; // unsized
		CGparameter m_cgp_F_processorsArray; // unsized

		CGparameter m_cgp_shared_inputSamplersArray; // created dynamically
		CGparameter m_cgp_shared_f4_scalesArray; // created dynamically

		CGparameter m_cgp_shared_processorsArray; // created dynamically
		QVector< CGparameter > m_qvStructParameters; // each is created dynamically

	// input port
	QVector< InputKernelPort* > m_qvInputTexturesInputPort;

	// output port
	OutputKernelPort* m_pOutputTextureOutputPort;

	// input data
	int m_nInputs;
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

	int m_nProcessors;
	QStringList m_qvProcessorTypeNames;
};

#endif // PER_PIXEL_PROCESSOR_KERNEL_H
