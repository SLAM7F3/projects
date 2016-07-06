#ifndef ARRAY_INDEX_KERNEL
#define ARRAY_INDEX_KERNEL

#include "GPUKernel.h"

class ArrayIndexKernel : public GPUKernel
{
public:

	static ArrayIndexKernel* create( QString args );
	virtual ~ArrayIndexKernel ();
	
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

	ArrayIndexKernel( QString args );

	InputKernelPort* m_pIndexInputPort;
	InputKernelPort* m_pInputArrayInputPort;
	OutputKernelPort* m_pOutpuElementOutputPort;

	bool m_bIsFloat;
};

#endif // ARRAY_INDEX_KERNEL
