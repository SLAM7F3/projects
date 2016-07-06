#ifndef INTERACTIVE_TONE_MAP_KERNEL_H
#define INTERACTIVE_TONE_MAP_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <common/BasicTypes.h>

#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;

class InteractiveToneMapKernel : public GPUKernel
{
	Q_OBJECT

public:

	static void initializeCg();
	static InteractiveToneMapKernel* create( QString args );
	virtual ~InteractiveToneMapKernel();

	virtual bool isInputComplete();

	virtual void makeDirty( QString inputPortName );
	virtual void compute( QString outputPortName );

public slots:

	void handleMousePressed( int x, int y, int button );
	void handleMouseMoved( int x, int y );
	void handleMouseReleased( int x, int y, int button );

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

	static bool s_bCgInitialized;

	InteractiveToneMapKernel();
	void initializeCgPrograms();

	void cleanup();
	void reallocate();

	void initializeGridPass();
	void sliceGridAndToneMapPass();

	void getGridXYFromClick( int x, int y, int* pGridXOut, int* pGridYOut );
	void getGridXYZFromClick( int x, int y, int* pGridXOut, int* pGridYOut, float* pGridZOut );
	void readGridAtTwoZs( int gridX, int gridY, int gridZ0, int gridZ1, float* pGridZ0ValueOut, float* pGridZ1ValueOut );

	// =========================================================================
	// Fields
	// =========================================================================

	// ports
	InputKernelPort* m_pInputTextureInputPort;
	InputKernelPort* m_pBaseTextureInputPort;
	InputKernelPort* m_pLogBaseMinInputPort;
	InputKernelPort* m_pLogBaseMaxInputPort;
	InputKernelPort* m_pTargetContrastInputPort;
	InputKernelPort* m_pSigmaSpatialInputPort;
	InputKernelPort* m_pSigmaRangeInputPort;
	InputKernelPort* m_pDodgeBurnDeltaInputPort; 
	OutputKernelPort* m_pOutputTextureOutputPort;

	// Cg Programs
	CgProgramWrapper* m_pInitializeGridProgram;
		CGparameter m_cgp_IG_f2LogLuminanceMinMax;
		CGparameter m_cgp_IG_fCompressionFactor;
		CGparameter m_cgp_IG_f3GridWidthHeightDepth;	

	CgProgramWrapper* m_pSliceGridAndToneMapProgram;
		CGparameter m_cgp_SGTM_inputSampler;
		CGparameter m_cgp_SGTM_logBaseSampler;
		CGparameter m_cgp_SGTM_baseGridSampler;
		CGparameter m_cgp_SGTM_f3_rcpSigma;
		CGparameter m_cgp_SGTM_f3_gridSize;
		CGparameter m_cgp_SGTM_f2LogBaseMinMax;
		CGparameter m_cgp_SGTM_fLogAbsoluteScale;

	// input data
	GLTextureRectangle* m_pInputTexture;
	GLTextureRectangle* m_pBaseTexture;	
	float m_fLogBaseMin;
	float m_fLogBaseMax;
	float m_fTargetContrast;
	float m_fSigmaSpatial;
	float m_fSigmaRange;
	float m_fDodgeBurnDelta;

	// derived data
	int m_iInputWidth;
	int m_iInputHeight;
	float m_fCompressionFactor;
	int m_iGridWidth;
	int m_iGridHeight;
	int m_iGridDepth;
	int m_iGridTextureWidth;
	int m_iGridTextureHeight;

	int m_iOutputBaseRemappingGridIndex;
	GLTextureRectangle* m_pGridTexture;

	// output data
	GLTextureRectangle* m_pOutputTexture;

	// internal
	GLFramebufferObject* m_pFBO;
	bool m_bReallocationNeeded;
	int m_iPaddingXY;
	int m_iPaddingZ;	

	// mouse events
	int m_iLastPressedButton;
	int m_iMousePressedGridZ0;
	int m_iMousePressedGridZ1;
	int m_iLastGridX;
	int m_iLastGridY;
};

#endif // INTERACTIVE_TONE_MAP_KERNEL_H
