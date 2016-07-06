#include "KernelPort.h"

KernelPort::KernelPort( QString name,
					   GPUKernel* pKernel,
					   KernelPortDataType type ) :

	m_qsName( name ),	
	m_pKernel( pKernel ),
	m_type( type ),
	m_bIsDirty( true )
{

}

// virtual
KernelPort::~KernelPort()
{

}

QString KernelPort::getName() const
{
	return m_qsName;
}

KernelPortDataType KernelPort::getType() const
{
	return m_type;
}

bool KernelPort::isDirty() const
{
	return m_bIsDirty;
}

// virtual
void KernelPort::makeDirty()
{
	setIsDirty( true );
}


void KernelPort::setIsDirty( bool bDirty )
{
	m_bIsDirty = bDirty;
}
