#ifndef SCRIBBLE_KERNEL_H
#define SCRIBBLE_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <QSize>

#include "GPUKernel.h"

class GLTextureRectangle;
class CgProgramWrapper;
class GLFramebufferObject;
class GLOcclusionQuery;

class ScribbleKernel : public GPUKernel
{
	Q_OBJECT

public:

	static void initializeCg();
	static ScribbleKernel* create( QString args );
	virtual ~ScribbleKernel();

	virtual bool isInputComplete();

	virtual void makeDirty( QString inputPortName );
	virtual void compute( QString outputPortName );

public slots:

	void handleMousePressed( int x, int y, int button );
	void handleMouseReleased( int x, int y, int button );
	void handleMouseMoved( int x, int y );

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

	ScribbleKernel();
	void cleanup();
	void reallocate();

	void initializeCgPrograms();
	void initializeCgPasses();
	void initializeCgParameters();

	GLTextureRectangle* getInputGrid( int level );
	GLTextureRectangle* getOutputGrid( int level );
	void swapGrids( int level );	
	
	void setGridConstraints( int x, int y );

	void solveGrid( int maxNumIterations );

	void restrictConstraints();

	void smoothGridPass( int level, int width, int height, int depth );

	void prolongEstimatePass( int inputLevel, int outputLevel );

	void convergenceTestPass( int level = 0 );

	void slicePass();

	void compositePass();

	void colorChangePass();

	// =========================================================================
	// Fields
	// =========================================================================

	static bool s_bCgInitialized;

	// shared
	GLFramebufferObject* m_pFBO;

	// Restrict Constraints
	CgProgramWrapper* m_pRestrictConstraintsProgram;
		CGparameter m_cgp_RC_fullConstraintsSampler;
		CGparameter m_cgp_RC_f3RestrictedGridSize;
		CGparameter m_cgp_RC_f3FullGridSize;

	// Smooth Grid
	CgProgramWrapper* m_pSmoothGridProgram;
		CGparameter m_cgp_SG_gridSampler;
		CGparameter m_cgp_SG_rhsSampler;
		CGparameter m_cgp_SG_f3GridSize;

	// Prolong Estimate
	CgProgramWrapper* m_pProlongEstimateProgram;
		CGparameter m_cgp_PEST_restrictedGridLinearSampler;
		CGparameter m_cgp_PEST_fullGridSize;
		CGparameter m_cgp_PEST_restrictedGridSize;

	// Convergence Test
	CgProgramWrapper* m_pConvergenceTestProgram;
		CGparameter m_cgp_CT_gridSampler;
		CGparameter m_cgp_CT_rhsSampler;
		CGparameter m_cgp_CT_f3GridSize;

	// Slicing
	CgProgramWrapper* m_pSliceProgram;
		CGparameter m_cgp_SP_inputRGBASampler;
		CGparameter m_cgp_SP_gridSampler;
		CGparameter m_cgp_SP_f3_rcpSigma;
		CGparameter m_cgp_SP_f3_gridSize;
		CGparameter m_cgp_SP_f3_paddingXYZ;

	// Compositing
	CgProgramWrapper* m_pCompositeProgram;
		CGparameter m_cgp_C_inputRGBASampler;
		CGparameter m_cgp_C_maskSampler;
		CGparameter m_cgp_C_valueSampler;

	// Color Changing
	CgProgramWrapper* m_pColorChangeProgram;
		CGparameter m_cgp_CC_inputRGBASampler;
		CGparameter m_cgp_CC_influenceSampler;

	// input port
	InputKernelPort* m_pInputTextureInputPort;
	InputKernelPort* m_pInputRGBArrayInputPort;
	InputKernelPort* m_pSigmaSpatialInputPort;
	InputKernelPort* m_pSigmaRangeInputPort;
	InputKernelPort* m_pRadiusInputPort;

	// output port
	OutputKernelPort* m_pOutputTextureOutputPort;
	OutputKernelPort* m_pScribbledTextureOutputPort;
	OutputKernelPort* m_pColorChangeTextureOutputPort;

	// input data
	GLTextureRectangle* m_pInputTexture;
	ArrayWithLength< ubyte > m_aubInputRGBArray;
	float m_fSigmaSpatial;
	float m_fSigmaRange;

	// output data
	GLTextureRectangle* m_pOutputTexture;
	GLTextureRectangle* m_pScribbledOutputTexture;
	GLTextureRectangle* m_pColorChangedOutputTexture;

	// derived parameters
	int m_iInputWidth;
	int m_iInputHeight;
	int m_iGridWidth;
	int m_iGridHeight;
	int m_iGridDepth;
	int m_iPaddingX;
	int m_iPaddingY;
	int m_iPaddingZ;

	// internal
	bool m_bReallocationNeeded;
	
	// for drawing the strokes
	GLTextureRectangle* m_pMask2DTexture;
	GLTextureRectangle* m_pSource2DTexture;
	
	// grids

	// 2 QVectors of grids for ping ponging
	// each QVector contains all the levels
	QVector< GLTextureRectangle* > m_qvGrids[2];	
	QVector< GLTextureRectangle* > m_qvRHSTextures;

	QVector< int > m_qvWidths;
	QVector< int > m_qvHeights;
	QVector< int > m_qvDepths;
	QVector< int > m_qvGridTextureWidths;
	QVector< int > m_qvGridTextureHeights;
	int m_nLevels;

	
	QVector< int > m_qvSmoothOutputGridIndex;

	// Occlusion Query for residual testing
	GLOcclusionQuery* m_pOcclusionQuery;

	//////////////////////////////////////////////////////////////////////////
	// UI
	//////////////////////////////////////////////////////////////////////////
	int m_iLastButtonPressed;
	int m_nReleases;
};

#endif // SCRIBBLE_KERNEL_H
