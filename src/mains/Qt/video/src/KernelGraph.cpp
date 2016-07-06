#include "KernelGraph.h"

#include <QFile>

#include "GPUKernel.h"

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
bool KernelGraph::loadFromXML( QString filename, KernelGraph** pOutputGraph,
							  QVector< QPair< QString, int > >* pOutputViewedPorts,
							  QVector< QPair< QString, QString > >* pOutputAutoDirtyKernelsAndPorts,
							  QVector< QString >* pOutputMouseListenerKernels )
{
	QString errorMessage;
	int errorLine;
	int errorColumn;

	QDomDocument document( "graphName" );
	QFile file( filename );
	if( !file.open( QIODevice::ReadOnly ) )
	{
		fprintf( stderr, "Error opening file %s\n", filename.toAscii().constData() );
		return false;
	}
	if( !document.setContent( &file, false, &errorMessage, &errorLine, &errorColumn ) )
	{
		fprintf( stderr, "Error loading graph: error setting content.\n" );
		fprintf( stderr, "Error message: %s, error line: %d, error column: %d\n",
			errorMessage.toAscii().constData(),
			errorLine, errorColumn );
		file.close();
		return false;
	}
	file.close();

	QDomNodeList kernelNodeList = document.elementsByTagName( "kernel" );
	QDomNodeList connectionNodeList = document.elementsByTagName( "connection" );
	QDomNodeList defaultsNodeList = document.elementsByTagName( "default" );
	QDomNodeList viewsNodeList = document.elementsByTagName( "view" );
	QDomNodeList autoDirtyNodeList = document.elementsByTagName( "autodirty" );
	QDomNodeList mouseListenerNodeList = document.elementsByTagName( "mouselistener" );

	QHash< QString, GPUKernel* > qhKernels;

	bool constructKernelsSucceeded = constructKernelsFromNodeList( kernelNodeList, &qhKernels );
	if( constructKernelsSucceeded )
	{
		bool connectPortsSucceeded = connectPortsFromNodeList( connectionNodeList, qhKernels );
		if( connectPortsSucceeded )
		{
			setSourcePortDefaultsFromNodeList( defaultsNodeList, qhKernels );
			
			*pOutputGraph = new KernelGraph( qhKernels );
			*pOutputViewedPorts = getViewedPortsFromNodeList( viewsNodeList );
			*pOutputAutoDirtyKernelsAndPorts = getAutoDirtyPortsFromNodeList( autoDirtyNodeList );
			*pOutputMouseListenerKernels = getMouseListenerKernelsFromNodeList( mouseListenerNodeList );

			return true;
		}		
	}
	// otherwise, fall through and delete
	foreach( QString kernelId, qhKernels.keys() )
	{
		delete( qhKernels.take( kernelId ) );
	}

	return false;
}

// virtual
KernelGraph::~KernelGraph()
{

}

QHash< QString, GPUKernel* > KernelGraph::getKernelNames()
{
	return m_qhKernels;
}

QHash< QString, QVector< QString > > KernelGraph::getKernelNamesToSourcePortNames()
{
	QHash< QString, QVector< QString > > qhKernelNamesToSourcePortNames;

	// iterate over every kernel
	foreach( QString kernelName, m_qhKernels.keys() )
	{
		GPUKernel* pKernel = getKernelByName( kernelName );
		QHash< QString, InputKernelPort* > qhInputPorts = pKernel->getInputPorts();
		
		// iterate over every input port of the kernel
		foreach( QString inputPortName, qhInputPorts.keys() )
		{
			InputKernelPort* pInputKernelPort = qhInputPorts[ inputPortName ];

			// if it's a source
			if( pInputKernelPort->isSource() )
			{
				// then put it into output hash
				if( !( qhKernelNamesToSourcePortNames.contains( kernelName ) ) )
				{
					qhKernelNamesToSourcePortNames[ kernelName ] = QVector< QString >();
				}
				qhKernelNamesToSourcePortNames[ kernelName ].append( inputPortName );
			}
		}
	}

	return qhKernelNamesToSourcePortNames;
}

GPUKernel* KernelGraph::getKernelByName( QString name )
{
	return m_qhKernels[ name ];
}

InputKernelPort* KernelGraph::getInputPortByKernelAndName( QString kernelName, QString inputPortName )
{
	return m_qhKernels[ kernelName ]->getInputPortByName( inputPortName );
}

OutputKernelPort* KernelGraph::getOutputPortByKernelAndName( QString kernelName, QString outputPortName )
{
	return m_qhKernels[ kernelName ]->getOutputPortByName( outputPortName );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

KernelGraph::KernelGraph( QHash< QString, GPUKernel* > kernels )						 
{
	m_qhKernels = kernels;
}

// static
bool KernelGraph::constructKernelsFromNodeList( QDomNodeList kernelNodeList,
											   QHash< QString, GPUKernel* >* pNamesToKernels )
{
	for( int i = 0; i < kernelNodeList.count(); ++i )
	{
		QDomNode node = kernelNodeList.at( i );
		if( node.hasAttributes() )
		{
			QDomNamedNodeMap attributes = node.attributes();
			QDomAttr kernelClassAttr = attributes.namedItem( "class" ).toAttr();
			QDomAttr kernelIdAttr = attributes.namedItem( "id" ).toAttr();
			QString kernelClass = kernelClassAttr.value();
			QString kernelId = kernelIdAttr.value();
			QString args = "";
			if( attributes.contains( "args" ) )
			{
				args = attributes.namedItem( "args" ).toAttr().value();
			}

			GPUKernel* pKernel = GPUKernel::createKernelWithClassname( kernelClass, args );
			if( pKernel != NULL )
			{
				pNamesToKernels->insert( kernelId, pKernel );
			}
			else
			{
				fprintf( stderr, "class not found: %s\n",
					qPrintable( kernelClass ) );
				
				// delete all the ones already created
				foreach( QString kernelId, pNamesToKernels->keys() )
				{
					delete( pNamesToKernels->take( kernelId ) );
				}

				return false;
			}
		}
	}
	
	return true;
}

// static
bool KernelGraph::connectPortsFromNodeList( QDomNodeList connectionNodeList,
										   QHash< QString, GPUKernel* > qhKernels )
{
	for( int i = 0; i < connectionNodeList.count(); ++i )
	{
		QDomNode node = connectionNodeList.at( i );
		if( node.hasAttributes() )
		{
			QDomNamedNodeMap attributes = node.attributes();
			QDomAttr sourceIdAttr = attributes.namedItem( "sourceKernelId" ).toAttr();
			QDomAttr outputPortAttr = attributes.namedItem( "outputPort" ).toAttr();
			QDomAttr targetIdAttr = attributes.namedItem( "targetKernelId" ).toAttr();
			QDomAttr inputPortAttr = attributes.namedItem( "inputPort" ).toAttr();

			QString sourceId = sourceIdAttr.value();
			QString outputPort = outputPortAttr.value();
			QString targetId = targetIdAttr.value();
			QString inputPort = inputPortAttr.value();

			printf( "Attempting to connect: %s.%s --> %s.%s\n",
				qPrintable( sourceId ), qPrintable( outputPort ),
				qPrintable( targetId ), qPrintable( inputPort ) );

			GPUKernel* sourceKernel = qhKernels[ sourceId ];
			if( sourceKernel != NULL )
			{
				GPUKernel* targetKernel = qhKernels[ targetId ];
				if( targetKernel != NULL )
				{
					bool bConnected = sourceKernel->connect( outputPort, targetKernel, inputPort );
					if( !bConnected )
					{
						fprintf( stderr, "either output port %s.%s not found or input port %s.%s not found!\n",
							qPrintable( sourceId ), qPrintable( outputPort ),
							qPrintable( targetId ), qPrintable( inputPort ) );
						return false;
					}
				}
				else
				{
					fprintf( stderr, "target kernel %s not found!\n",
						qPrintable( targetId ) );
					return false;
				}
			}
			else
			{
				fprintf( stderr, "source kernel %s not found!\n",
					qPrintable( sourceId ) );
				return false;
			}
		}
	}

	return true;
}

// static
void KernelGraph::setSourcePortDefaultsFromNodeList( QDomNodeList defaultsNodeList,
													QHash< QString, GPUKernel* > qhKernels )
{
	for( int i = 0; i < defaultsNodeList.count(); ++i )
	{
		QDomNode defaultNode = defaultsNodeList.at( i );
		if( defaultNode.hasAttributes() )
		{
			QDomNamedNodeMap attributes = defaultNode.attributes();
			// TODO: error checking
			QDomAttr kernelIdAttr = attributes.namedItem( "kernelId" ).toAttr();
			QDomAttr inputPortAttr = attributes.namedItem( "inputPort" ).toAttr();
			QDomAttr valueAttr = attributes.namedItem( "value" ).toAttr();

			QString kernelId = kernelIdAttr.value();
			QString inputPort = inputPortAttr.value();
			QString value = valueAttr.value();

			printf( "Attempting to set \"%s.%s\" to \"%s\"\n",
				qPrintable( kernelId ), qPrintable( inputPort ), qPrintable( value ) );

			if( qhKernels.contains( kernelId ) )
			{
				GPUKernel* pKernel = qhKernels[ kernelId ];
				InputKernelPort* pInputPort = pKernel->getInputPortByName( inputPort );

				if( pInputPort != NULL )
				{					
					pInputPort->pushData( KernelPortData::unserialize( value ) );
				}
				else
				{
					fprintf( stderr, "Error setting source: kernel.port %s.%s not found!\n",
						qPrintable( kernelId ), qPrintable( inputPort ) );
				}
			}
			else
			{
				fprintf( stderr, "Error setting source: kernel %s not found!\n",
					qPrintable( kernelId ) );
			}
		}
	}
}

// static
QVector< QPair< QString, int > > KernelGraph::getViewedPortsFromNodeList( QDomNodeList viewsNodeList )
{
	QVector< QPair< QString, int > > kernelsAndPorts;

	for( int i = 0; i < viewsNodeList.count(); ++i )
	{
		QDomNode viewNode = viewsNodeList.at( i );
		if( viewNode.hasAttributes() )
		{
			QDomNamedNodeMap attributes = viewNode.attributes();
			QDomAttr kernelIdAttr = attributes.namedItem( "kernelId" ).toAttr();
			QDomAttr outputPortAttr = attributes.namedItem( "outputPort" ).toAttr();
			
			QString kernelDotPort = kernelIdAttr.value() + "." + outputPortAttr.value();
			int index = -1;

			if( attributes.contains( "index" ) )
			{
				index = attributes.namedItem( "index" ).toAttr().value().toInt();
			}

			kernelsAndPorts.append( qMakePair( kernelDotPort, index ) );
		}
	}
	
	return kernelsAndPorts;
}

// static
QVector< QPair< QString, QString > > KernelGraph::getAutoDirtyPortsFromNodeList( QDomNodeList autoDirtyNodeList )
{
	QVector< QPair< QString, QString > > kernelsAndPorts;

	for( int i = 0; i < autoDirtyNodeList.count(); ++i )
	{
		QDomNode autoDirtyNode = autoDirtyNodeList.at( i );
		if( autoDirtyNode.hasAttributes() )
		{
			QDomNamedNodeMap attributes = autoDirtyNode.attributes();
			QDomAttr kernelIdAttr = attributes.namedItem( "kernelId" ).toAttr();
			QDomAttr inputPortAttr = attributes.namedItem( "inputPort" ).toAttr();

			kernelsAndPorts.append( qMakePair( kernelIdAttr.value(), inputPortAttr.value() ) );
		}
	}

	return kernelsAndPorts;
}

// static
QVector< QString > KernelGraph::getMouseListenerKernelsFromNodeList( QDomNodeList mouseListenerNodeList )
{
	QVector< QString > kernels;

	for( int i = 0; i < mouseListenerNodeList.count(); ++i )
	{
		QDomNode mouseListenerNode = mouseListenerNodeList.at( i );
		if( mouseListenerNode.hasAttributes() )
		{
			QDomNamedNodeMap attributes = mouseListenerNode.attributes();
			QDomAttr kernelIdAttr = attributes.namedItem( "kernelId" ).toAttr();

			kernels.append( kernelIdAttr.value() );
		}
	}

	return kernels;
}
