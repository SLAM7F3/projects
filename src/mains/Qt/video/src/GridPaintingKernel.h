#ifndef GRID_PAINTING_KERNEL_H
#define GRID_PAINTING_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <common/BasicTypes.h>

#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;

class GridPaintingKernel : public GPUKernel
{
	Q_OBJECT

public:

	static void initializeCg();
	static GridPaintingKernel* create( QString args );
	virtual ~GridPaintingKernel();

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

	GridPaintingKernel();
	void initializeCgPrograms();

	void cleanup();
	void reallocate();

	void clearGridPass();
	void sliceGridAndRotateColorsPass();

	// =========================================================================
	// Fields
	// =========================================================================

	// ports
	InputKernelPort* m_pInputTextureInputPort;
	InputKernelPort* m_pSigmaSpatialInputPort;
	InputKernelPort* m_pSigmaRangeInputPort;

	OutputKernelPort* m_pOutputTextureOutputPort;

	// Cg Programs
	CgProgramWrapper* m_pSliceGridAndRotateColorsProgram;
	CGparameter m_cgp_SGRC_inputSampler;
	CGparameter m_cgp_SGRC_gridSampler;
	CGparameter m_cgp_SGRC_f3_rcpSigma;
	CGparameter m_cgp_SGRC_f3_gridSize;

	// input data
	GLTextureRectangle* m_pInputTexture;

	// derived data
	int m_iInputWidth;
	int m_iInputHeight;
	int m_iGridWidth;
	int m_iGridHeight;
	int m_iGridDepth;
	int m_iGridTextureWidth;
	int m_iGridTextureHeight;
	float m_fSigmaSpatial;
	float m_fSigmaRange;

	// output data
	GLTextureRectangle* m_pOutputTexture;

	// internal
	GLFramebufferObject* m_pFBO;
	bool m_bReallocationNeeded;
	int m_iPaddingXY;
	int m_iPaddingZ;

	GLTextureRectangle* m_pGridTexture;

	// mouse events
	int m_iLastPressedButton;
	int m_iMousePressedGridX;
	int m_iMousePressedGridY;
	int m_iMousePressedGridZ0;
	int m_iMousePressedGridZ1;
};

#endif // GRID_PAINTING_KERNEL_H
