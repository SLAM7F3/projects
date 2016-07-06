#ifndef OUTPUT_KERNEL_PORT_H
#define OUTPUT_KERNEL_PORT_H

#include <QVector>

#include "KernelPort.h"

class GPUKernel;
class InputKernelPort;

class OutputKernelPort : public KernelPort
{
  public:

   OutputKernelPort( QString name, GPUKernel* pKernel, KernelPortDataType type );
   virtual ~OutputKernelPort();

   bool isSink() const;	

   virtual void makeDirty();
	
   // returns true if there are any connected downstream ports
   //	or if this port is a sink
   // returns false otherwise
   virtual bool isConnected() const;
   void addConnection( InputKernelPort* pDownstreamPort );
   void removeConnection( InputKernelPort* pDownstreamPort );

   // called by the kernel to set the new output data
   virtual void pushData( const KernelPortData& data );
   virtual KernelPortData pullData();

  private:

   void setIsSink( bool isSink );

   bool m_bIsSink;
   QVector< InputKernelPort* > m_qvConnectedPorts;
};

#endif // OUTPUT_KERNEL_PORT_H
