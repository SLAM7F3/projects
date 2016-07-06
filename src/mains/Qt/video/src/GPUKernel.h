#ifndef GPU_KERNEL_H
#define GPU_KERNEL_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <QHash>
#include <QSet>
#include <QString>
#include <common/BasicTypes.h>
#include <GL/GLInitializable.h>

#include "InputKernelPort.h"
#include "OutputKernelPort.h"

class GLTexture1D;
class GLTexture2D;
class GLBufferObject;

// abstract
class GPUKernel : public QObject, public GLInitializable
{
public:

	// ---- Constructors ----
	static GPUKernel* createKernelWithClassname( QString className, QString args );
	virtual ~GPUKernel();

	// ---- Graph connections ----	
	virtual bool connect( QString outputPortName, GPUKernel* pReceiver, QString inputPortName );
	virtual bool disconnect( QString outputPortName, GPUKernel* pReceiver, QString inputPortName );

	// ---- Computation ----
	virtual void makeDirty( QString inputPortName ) = 0;
	virtual void compute( QString outputPortName ) = 0;

	// ---- Input Ports ----	
	virtual bool isInputComplete(); // returns true if all inputs are complete and is ready to compute
	QHash< QString, InputKernelPort* > getInputPorts();
	QHash< QString, OutputKernelPort* > getOutputPorts();

	InputKernelPort* getInputPortByName( QString name );
	OutputKernelPort* getOutputPortByName( QString name );

	// ---- Utility ----
	QString getClassName();

protected:

	// =========================================================================
	// Methods
	// =========================================================================

	GPUKernel( QString className );

	// ---- Initialization ----	
	virtual void initializeGL();
	virtual void initializePorts();
		
	virtual void onInputConnected( QString inputPortName, InputKernelPort* pPort );
	virtual void onInputDisconnected( QString inputPortName, InputKernelPort* pPort );
	virtual void onOutputConnected( QString outputPortName, OutputKernelPort* pPort );
	virtual void onOutputDisconnected( QString outputPortName, OutputKernelPort* pPort );
	
	// ---- Called by subclasses to add ports ----
	InputKernelPort* addInputPort( QString name, KernelPortDataType type );
	OutputKernelPort* addOutputPort( QString name, KernelPortDataType type );

	// =========================================================================
	// Fields
	// =========================================================================

	// Ports
	QHash< QString, InputKernelPort* > m_qhInputPorts;
	QHash< QString, OutputKernelPort* > m_qhOutputPorts;

private:

	// =========================================================================
	// Methods
	// =========================================================================

	// =========================================================================
	// Fields
	// =========================================================================	

	QString m_className;
};

#endif // GPU_KERNEL_H
