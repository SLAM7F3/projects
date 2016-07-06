#ifndef LOCAL_HISTOGRAM_EQUALIZATION_KERNEL_H
#define LOCAL_HISTOGRAM_EQUALIZATION_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include "GPUKernel.h"

class CgProgramWrapper;
class GLBufferObject;
class GLFramebufferObject;

class LocalHistogramEqualizationKernel : public GPUKernel
{
public:

	static void initializeCg();
	static LocalHistogramEqualizationKernel* create( QString args );
	virtual ~LocalHistogramEqualizationKernel();

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

	LocalHistogramEqualizationKernel();
	void cleanup();
	void reallocate();

	void initializeCgPrograms();

	GLTextureRectangle* getInputGrid();
	GLTextureRectangle* getOutputGrid();
	void swapGrids();

	void buildPDFGridPass();
	void buildCDFGridPass();
	void normalizeCDFGridPass();
	void sliceGridPass();

	// =========================================================================
	// Fields
	// =========================================================================

	static bool s_bCgInitialized;

	// shared
	GLFramebufferObject* m_pFBO;

	// Cg
	CgProgramWrapper* m_pBuildPDFGridVertexProgram;
		CGparameter m_cgp_BPGV_f44_mvp;
		CGparameter m_cgp_BPGV_f3_rcpSigma;
		CGparameter m_cgp_BPGV_f3_gridSize;
		CGparameter m_cgp_BPGV_f_inputMin;
		CGparameter m_cgp_BPGV_inputSampler;

	CgProgramWrapper* m_pBuildPDFGridFragmentProgram;

	CgProgramWrapper* m_pBuildCDFGridFragmentProgram;
		CGparameter m_cgp_BCGF_pdfGridSampler;
		CGparameter m_cgp_BCGF_fGridWidth;

	CgProgramWrapper* m_pNormalizeCDFGridFragmentProgram;
		CGparameter m_cgp_NCGF_cdfGridSampler;
		CGparameter m_cgp_NCGF_f3GridWidthHeightDepth;

	CgProgramWrapper* m_pSliceGridFragmentProgram;
		CGparameter m_cgp_SGF_inputSampler;
		CGparameter m_cgp_SGF_equalizedGridSampler;
		CGparameter m_cgp_SGF_f3_rcpSigma;
		CGparameter m_cgp_SGF_f3_gridSize;
		CGparameter m_cgp_SGF_f_inputMin;
		CGparameter m_cgp_SGF_f_inputDelta;

	// input port
	InputKernelPort* m_pInputTextureInputPort;
	InputKernelPort* m_pSigmaSpatialInputPort;
	InputKernelPort* m_pSigmaRangeInputPort;
	InputKernelPort* m_fInputMinInputPort;
	InputKernelPort* m_fInputMaxInputPort;

	// output port
	OutputKernelPort* m_pOutputTextureOutputPort;

	// input data
	float m_fSigmaSpatial;
	float m_fSigmaRange;
	float m_fInputMin;
	float m_fInputMax;
	GLTextureRectangle* m_pInputTexture;

	// output data
	GLTextureRectangle* m_pOutputTexture;	

	// derived parameters
	int m_iInputWidth;
	int m_iInputHeight;
	int m_nPixels;
	int m_iGridWidth;
	int m_iGridHeight;
	int m_iGridDepth;
	float m_fInputDelta;

	// internal
	bool m_bReallocationNeeded;
	int m_iPadding;
	int m_iGridTextureWidth;
	int m_iGridTextureHeight;

	GLBufferObject* m_pXYCoordinateVBO;
	int m_iOutputGridIndex;
	GLTextureRectangle* m_apGrids[2];
};

#endif // LOCAL_HISTOGRAM_EQUALIZATION_KERNEL_H
