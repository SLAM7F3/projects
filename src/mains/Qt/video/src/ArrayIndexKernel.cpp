#include "ArrayIndexKernel.h"

#include <cassert>
#include <math/MathUtils.h>

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
ArrayIndexKernel* ArrayIndexKernel::create( QString args )
{
	ArrayIndexKernel* pInstance = new ArrayIndexKernel( args );
	pInstance->initializeGL();
	return pInstance;
}

// virtual
ArrayIndexKernel::~ArrayIndexKernel()
{

}

// virtual
bool ArrayIndexKernel::isInputComplete()
{
	return true;
}

// virtual
void ArrayIndexKernel::compute( QString outputPortName )
{
	int index = m_pIndexInputPort->pullData().getIntData();

	KernelPortData inputData = m_pInputArrayInputPort->pullData();
	KernelPortData outputData;

	if( m_bIsFloat )
	{
		QVector< float > floatArray = inputData.getFloatArrayData();
		int arraySize = floatArray.size();
		if( arraySize > 0 )
		{
			index = MathUtils::clampToRangeInt( index, 0, arraySize );
			outputData = KernelPortData( floatArray.at( index ) );
		}
		else
		{
			outputData = KernelPortData( 0 );
		}
	}
	else
	{
		QVector< GLTextureRectangle* > textureArray = inputData.getGLTextureRectangleArrayData();
		int arraySize = textureArray.size();
		if( arraySize > 0 )
		{
			index = MathUtils::clampToRangeInt( index, 0, arraySize );
			outputData = KernelPortData( textureArray.at( index ) );
		}
		else
		{
			outputData = KernelPortData( NULL );
		}
	}

	m_pOutpuElementOutputPort->pushData( outputData );
}

// virtual
void ArrayIndexKernel::makeDirty( QString inputPortName )
{
	m_pOutpuElementOutputPort->makeDirty();
}

//////////////////////////////////////////////////////////////////////////
// Protected
//////////////////////////////////////////////////////////////////////////

// virtual
void ArrayIndexKernel::initializeGL()
{
	GPUKernel::initializeGL();
}

// virtual
void ArrayIndexKernel::initializePorts()
{
	KernelPortDataType inputType;
	KernelPortDataType outputType;

	if( m_bIsFloat )
	{
		inputType = KERNEL_PORT_DATA_TYPE_FLOAT_ARRAY;
		outputType = KERNEL_PORT_DATA_TYPE_FLOAT;
	}
	else
	{
		inputType = KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE_ARRAY;
		outputType = KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE;
	}

	m_pIndexInputPort = addInputPort( "index", KERNEL_PORT_DATA_TYPE_INT );
	m_pIndexInputPort->setIntMinMaxDelta( 0, 0, 100, 1 );

	m_pInputArrayInputPort = addInputPort( "inputArray", inputType );
	m_pOutpuElementOutputPort = addOutputPort( "outputElement", outputType );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

ArrayIndexKernel::ArrayIndexKernel( QString args ) :

	GPUKernel( "ArrayIndex" )
	
{
	if( args == "float" )
	{
		m_bIsFloat = true;
	}
	else if( args == "gltexturerectangle" )
	{
		m_bIsFloat = false;
	}
	else
	{
		fprintf( stderr, "ArrayIndexKernel: args must be \"float\" or \"gltexturerectangle\"\n" );
		assert( false );
	}
}
