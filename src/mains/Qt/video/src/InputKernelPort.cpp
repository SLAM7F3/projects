#include "InputKernelPort.h"

#include <cassert>
#include <cstdio>

#include "GPUKernel.h"

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

InputKernelPort::InputKernelPort( QString name,
								 GPUKernel* pKernel,
								 KernelPortDataType type ) :

	KernelPort( name, pKernel, type ),	
	m_pConnectedUpstreamPort( NULL )

{
	setIsSource( true );
}

// virtual
InputKernelPort::~InputKernelPort()
{

}

bool InputKernelPort::getBoolDefault()
{
	assert( getType() == KERNEL_PORT_DATA_TYPE_BOOL );

	return m_data.getBoolData();
}

void InputKernelPort::setBoolDefault( bool value )
{
	assert( getType() == KERNEL_PORT_DATA_TYPE_BOOL );

	m_data = KernelPortData( value );
}

void InputKernelPort::getFloatMinMaxDelta( float* pValue, float* pMin, float* pMax, float* pDelta )
{
	assert( getType() == KERNEL_PORT_DATA_TYPE_FLOAT );	
	
	*pValue = m_data.getFloatData();
	*pMin = m_fFloatMin;
	*pMax = m_fFloatMax;
	*pDelta = m_fFloatDelta;
}

void InputKernelPort::setFloatMinMaxDelta( float value, float minimum, float maximum, float delta )
{
	assert( getType() == KERNEL_PORT_DATA_TYPE_FLOAT );
	
	m_fFloatMin = minimum;
	m_fFloatMax = maximum;
	m_fFloatDelta = delta;

	m_data = KernelPortData( value );
}

void InputKernelPort::getIntMinMaxDelta( int* pValue, int* pMin, int* pMax, int* pDelta )
{
	assert( getType() == KERNEL_PORT_DATA_TYPE_INT );

	*pValue = m_data.getIntData();
	*pMin = m_iIntMin;
	*pMax = m_iIntMax;
	*pDelta = m_iIntDelta;
}

void InputKernelPort::setIntMinMaxDelta( int value, int minimum, int maximum, int delta )
{
	assert( getType() == KERNEL_PORT_DATA_TYPE_INT );

	m_iIntMin = minimum;
	m_iIntMax = maximum;
	m_iIntDelta = delta;

	m_data = KernelPortData( value );
}

bool InputKernelPort::isSource() const
{
	return m_bIsSource;
}

// virtual
bool InputKernelPort::isConnected() const
{
	return( m_pConnectedUpstreamPort != NULL );
}

void InputKernelPort::setUpstreamPort( OutputKernelPort* pOutputPort )
{
	m_pConnectedUpstreamPort = pOutputPort;

	bool isSource = ( pOutputPort == NULL );
	setIsSource( isSource );
}

// virtual
void InputKernelPort::makeDirty()
{
	KernelPort::makeDirty();
	m_pKernel->makeDirty( getName() );
}

void InputKernelPort::pushData( const KernelPortData& data )
{
	assert( isSource() );

	// data is being pushed as a source
	// if the incoming data is the same, then do nothing
	// otherwise, store the data and mark this node dirty
	// and if this node needs reallocation, mark that as well
	if( isSource() )
	{
		m_data = data;
		makeDirty();
	}	
}

KernelPortData InputKernelPort::pullData()
{
	if( isDirty() )
	{
		if( !isSource() )
		{
			m_data = m_pConnectedUpstreamPort->pullData();
		}		
		setIsDirty( false );		
	}

	assert( m_data.getType() != KERNEL_PORT_DATA_TYPE_NULL );

	return m_data;
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

void InputKernelPort::setIsSource( bool isSource )
{
	m_bIsSource = isSource;
}