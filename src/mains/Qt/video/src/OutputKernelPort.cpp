#include <iostream>

#include "OutputKernelPort.h"

#include "GPUKernel.h"
#include "InputKernelPort.h"

using std::cout;
using std::endl;

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

OutputKernelPort::OutputKernelPort( QString name,
                                    GPUKernel* pKernel,
                                    KernelPortDataType type ) :

   KernelPort( name, pKernel, type )

{
   setIsSink( true );
}

// virtual
OutputKernelPort::~OutputKernelPort()
{

}

bool OutputKernelPort::isSink() const
{
   return m_bIsSink;
}

// virtual
void OutputKernelPort::makeDirty()
{
   // TODO: optimize dirtying process for large graphs, if( !isDirty() ), but what about initialization?
   KernelPort::makeDirty();

   foreach( InputKernelPort* p, m_qvConnectedPorts )
      {
         p->makeDirty();
      }
}

// virtual
bool OutputKernelPort::isConnected() const
{
   return( !( m_qvConnectedPorts.isEmpty() ) );
}

void OutputKernelPort::addConnection( InputKernelPort* pDownstreamPort )
{
   if( !( m_qvConnectedPorts.contains( pDownstreamPort ) ) )
   {
      m_qvConnectedPorts.append( pDownstreamPort );
      pDownstreamPort->setUpstreamPort( this );
   }

   setIsSink( false );
}

void OutputKernelPort::removeConnection( InputKernelPort* pDownstreamPort )
{
   int containsIndex = m_qvConnectedPorts.indexOf( pDownstreamPort );
   if( containsIndex != -1 )
   {
      pDownstreamPort->setUpstreamPort( NULL );
      m_qvConnectedPorts.remove( containsIndex );
   }

   if( m_qvConnectedPorts.isEmpty() )
   {
      setIsSink( true );
   }
}

// virtual
void OutputKernelPort::pushData( const KernelPortData& data )
{
   cout << "inside OutputKernelPort::pushData()" << endl;
   m_data = data;
}

// virtual
KernelPortData OutputKernelPort::pullData()
{
   cout << "inside OutputKernelPort::pullData()" << endl;
   if( isDirty() )
   {
      cout << "Before call to compute(getName())" << endl;
      cout << "getName() = " << getName().toStdString() << endl;
     
      m_pKernel->compute( getName() );
      setIsDirty( false );
   }
   return m_data;
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

void OutputKernelPort::setIsSink( bool isSink )
{
   m_bIsSink = isSink;
}
