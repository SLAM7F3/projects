#ifndef KERNEL_PORT_H
#define KERNEL_PORT_H

#include <common/BasicTypes.h>
#include <QString>

#include "KernelPortData.h"

class GLBufferObject;
class GLTexture2D;
class GPUKernel;

// interface
class KernelPort
{
  public:

   KernelPort( QString name, GPUKernel* pKernel, KernelPortDataType type );		
   virtual ~KernelPort();

   QString getName() const;
   KernelPortDataType getType() const;

   bool isDirty() const;
   virtual void makeDirty();

   virtual bool isConnected() const = 0;

   virtual void pushData( const KernelPortData& data ) = 0;
   virtual KernelPortData pullData() = 0;

  protected:

   void setIsDirty( bool bDirty );
	
   GPUKernel* m_pKernel;
   KernelPortDataType m_type;
   KernelPortData m_data;

  private:

   QString m_qsName;
   bool m_bIsDirty;
};

#endif // KERNEL_PORT_H
