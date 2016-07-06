#ifndef DETAIL_MAP_KERNEL_H
#define DETAIL_MAP_KERNEL_H

#include <QSize>

#include "GPUKernel.h"

class CgProgramWrapper;
class GLFramebufferObject;

class DetailMapKernel : public GPUKernel
{
	Q_OBJECT

public:

	static void initializeCg();
	static DetailMapKernel* create( QString args );
	virtual ~DetailMapKernel();

	virtual bool isInputComplete();

	virtual void makeDirty( QString inputPortName );
	virtual void compute( QString outputPortName );

public slots:

	void handleMousePressed( int x, int y, int button );
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

	DetailMapKernel();

	void initializeCgPrograms();

	void deleteOutputTextures();
	void reallocate();

	void combineBFPass();
	void detailMapPass();

	// =========================================================================
	// Fields
	// =========================================================================

	static bool s_bCgInitialized;

	// shared
	GLFramebufferObject* m_pFBO;

	// Cg
	CgProgramWrapper* m_pMakeDetailMapProgram;
	CGparameter m_cgp_MDM_f2MouseXY;
	CGparameter m_cgp_MDM_f2SigmaRadius;

	// Input Ports
	InputKernelPort* m_pSizeInputPort;
	InputKernelPort* m_pSigmaInputPort;
	InputKernelPort* m_pRadiusInputPort;

	// Output ports
	OutputKernelPort* m_pDetailMapOutputPort;

	// Input data
	QSize m_qSize;

	// Output data
	GLTextureRectangle* m_pDetailMapTexture;

	// internal
	bool m_bReallocationNeeded;
	int m_iMouseMovedX;
	int m_iMouseMovedY;
};

#endif // DETAIL_MAP_KERNEL_H
