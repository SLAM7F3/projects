#ifndef INPUT_KERNEL_PORT_H
#define INPUT_KERNEL_PORT_H

#include "KernelPort.h"
#include <QPair>

class GPUKernel;
class OutputKernelPort;

class InputKernelPort : public KernelPort
{
public:

	InputKernelPort( QString name, GPUKernel* pKernel, KernelPortDataType type );
	virtual ~InputKernelPort();

	bool getBoolDefault();
	void setBoolDefault( bool value );

	void getFloatMinMaxDelta( float* pValue, float* pMin, float* pMax, float* pDelta );
	void setFloatMinMaxDelta( float value, float minimum, float maximum, float delta );

	void getIntMinMaxDelta( int* pValue, int* pMin, int* pMax, int* pDelta );
	void setIntMinMaxDelta( int value, int minimum, int maximum, int delta );

	bool isSource() const;

	// returns true if there is an upstream port connected
	//	or if this port is a source
	// returns false otherwise
	virtual bool isConnected() const;

	// do not call this directly
	// use GPUKernel::connect()
	void setUpstreamPort( OutputKernelPort* pOutputPort );

	// marks this port as dirty and tells the parent kernel
	// which port was marked dirty
	virtual void makeDirty();

	// can only be called when not connected
	// directly feeds data to the port instead of via another port on the graph
	// allows an external application to directly set the data
	// useful for source nodes
	void pushData( const KernelPortData& data );
	KernelPortData pullData();

private:

	void setIsSource( bool isSource );

	bool m_bIsSource;
	OutputKernelPort* m_pConnectedUpstreamPort;

	float m_fFloatMin;
	float m_fFloatMax;
	float m_fFloatDelta;

	int m_iIntMin;
	int m_iIntMax;
	int m_iIntDelta;
};

#endif // INPUT_KERNEL_PORT_H
