#ifndef KERNEL_GRAPH_H
#define KERNEL_GRAPH_H

#include <QDomDocument>
#include <QHash>
#include <QPair>
#include <QString>
#include <QVector>

class GPUKernel;
class InputKernelPort;
class OutputKernelPort;

class KernelGraph
{
public:

	static bool loadFromXML( QString filename, KernelGraph** pOutputGraph,
		QVector< QPair< QString, int > >* pOutputViewedPorts,
		QVector< QPair< QString, QString > >* pOutputAutoDirtyKernelsAndPorts,
		QVector< QString >* pOutputMouseListenerKernels );
	virtual ~KernelGraph();

	QHash< QString, GPUKernel* > getKernelNames();
	QHash< QString, QVector< QString > > getKernelNamesToSourcePortNames();	

	// convenience classes
	GPUKernel* getKernelByName( QString name );
	InputKernelPort* getInputPortByKernelAndName( QString kernelName, QString inputPortName );
	OutputKernelPort* getOutputPortByKernelAndName( QString kernelName, QString outputPortName );	

private:

	KernelGraph( QHash< QString, GPUKernel* > kernels );		

	static bool constructKernelsFromNodeList( QDomNodeList kernelNodeList, QHash< QString, GPUKernel* >* pNamesToKernels );
	static bool connectPortsFromNodeList( QDomNodeList connectionNodeList,
		QHash< QString, GPUKernel* > qhKernels );

	// sets the default values on the source ports
	static void setSourcePortDefaultsFromNodeList( QDomNodeList defaultsNodeList,
		QHash< QString, GPUKernel* > qhKernels );

	// returns a vector of strings
	// each string is of the form "kernelName.portName"
	// the index in the vector denotes the view number
	static QVector< QPair< QString, int > > getViewedPortsFromNodeList( QDomNodeList viewsNodeList );

	// returns a vector of pairs of strings
	// each pair is of the form ( kernelName, inputportName )
	// which denotes the kernel and port that should be automatically dirtied on every frame
	static QVector< QPair< QString, QString > > getAutoDirtyPortsFromNodeList( QDomNodeList autoDirtyNodeList );

	// returns a vector of kernel names that are mouse listeners
	static QVector< QString > getMouseListenerKernelsFromNodeList( QDomNodeList mouseListenerNodeList );

	QHash< QString, GPUKernel* > m_qhKernels;
};

#endif // KERNEL_GRAPH_H
