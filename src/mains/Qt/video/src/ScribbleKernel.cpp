#include "ScribbleKernel.h"

#include <cassert>
#include <Cg/CgProgramManager.h>
#include <Cg/CgProgramWrapper.h>
#include <Cg/CgShared.h>
#include <Cg/CgUtilities.h>
#include <common/ArrayWithLength.h>
#include <common/ArrayUtils.h>
#include <color/ColorUtils.h>
#include <GL/GLFramebufferObject.h>
#include <GL/GLOcclusionQuery.h>
#include <GL/GLShared.h>
#include <GL/GLTextureRectangle.h>
#include <GL/GLUtilities.h>
#include <math/Arithmetic.h>
#include <math/MathUtils.h>
#include <time/StopWatch.h>

#include "AppData.h"

int min(int a, int b)
{
  if( a < b )
    return a;

  return b;
}
// static
void ScribbleKernel::initializeCg()
{
	QString cgPrefix = AppData::getInstance()->getCgPathPrefix();
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();
	CGprofile latestCgFragmentProfile = CgShared::getInstance()->getLatestFragmentProfile();

	pProgramManager->loadProgramFromFile( "SCRIBBLE_restrictConstraints", cgPrefix + "Scribble.cg",
		"restrictConstraints", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "SCRIBBLE_smoothGrid", cgPrefix + "Scribble.cg",
		"smoothGrid", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "SCRIBBLE_prolongEstimate", cgPrefix + "Scribble.cg",
		"prolongEstimate", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "SCRIBBLE_convergenceTest", cgPrefix + "Scribble.cg",
		"convergenceTest", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "SCRIBBLE_slice", cgPrefix + "Scribble.cg",
		"slice", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "SCRIBBLE_composite", cgPrefix + "Scribble.cg",
		"composite", latestCgFragmentProfile, NULL );

	pProgramManager->loadProgramFromFile( "SCRIBBLE_colorChange", cgPrefix + "Scribble.cg",
		"colorChange", latestCgFragmentProfile, NULL );	

	s_bCgInitialized = true;
}

// static
ScribbleKernel* ScribbleKernel::create( QString args )
{
	if( !s_bCgInitialized )
	{
		initializeCg();
	}

	ScribbleKernel* pInstance = new ScribbleKernel;
	pInstance->initializeGL();
	return pInstance;	
}

// virtual
ScribbleKernel::~ScribbleKernel()
{
	cleanup();
}

// virtual
bool ScribbleKernel::isInputComplete()
{
	return true;
}

// virtual
void ScribbleKernel::makeDirty( QString inputPortName )
{
	if( inputPortName == "inputTexture" )
	{
		m_bReallocationNeeded = true;
	}

	m_pScribbledTextureOutputPort->makeDirty();
	m_pColorChangeTextureOutputPort->makeDirty();
}

// virtual
void ScribbleKernel::compute( QString outputPortName )
{
	if( m_bReallocationNeeded )
	{
		reallocate();
	}

	if( outputPortName == "scribbledTexture" )
	{
		compositePass();
	}

	if( outputPortName == "colorChangeTexture" )
	{
		colorChangePass();
	}
}

//////////////////////////////////////////////////////////////////////////
// Public Slots
//////////////////////////////////////////////////////////////////////////

#define NUM_RELEASES 0

void ScribbleKernel::handleMousePressed( int x, int y, int button )
{
	m_iLastButtonPressed = button;
	setGridConstraints( x, y );

	if( m_nReleases > NUM_RELEASES )
	{
		restrictConstraints();
		solveGrid( 20 );
	}

	m_pScribbledTextureOutputPort->makeDirty();

	AppData::getInstance()->updateAndDraw();
}

void ScribbleKernel::handleMouseReleased( int x, int y, int button )
{
	++m_nReleases;

	if( m_nReleases > NUM_RELEASES )
	{
		m_iLastButtonPressed = 0;

		restrictConstraints();
		solveGrid( 20 );

		m_pOutputTextureOutputPort->makeDirty();
	}

	AppData::getInstance()->updateAndDraw();
}

void ScribbleKernel::handleMouseMoved( int x, int y )
{
	printf( "setting grid constraints..." );
	setGridConstraints( x, y );
	printf( "done!\n" );

	if( m_nReleases > NUM_RELEASES )
	{
		restrictConstraints();
		solveGrid( 20 );
	}

	m_pScribbledTextureOutputPort->makeDirty();

	AppData::getInstance()->updateAndDraw();
}

//////////////////////////////////////////////////////////////////////////
// protected
//////////////////////////////////////////////////////////////////////////

// virtual
void ScribbleKernel::initializeGL()
{
	GPUKernel::initializeGL();

	initializeCgPrograms();	
	m_pOcclusionQuery = new GLOcclusionQuery;
}

// virtual					
void ScribbleKernel::initializePorts()
{
	// ---- Input ----
	m_pInputTextureInputPort = addInputPort( "inputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pInputRGBArrayInputPort = addInputPort( "inputRGBArray", KERNEL_PORT_DATA_TYPE_UNSIGNED_BYTE_ARRAY );
	
	m_pRadiusInputPort = addInputPort( "radius", KERNEL_PORT_DATA_TYPE_INT );
	m_pRadiusInputPort->setIntMinMaxDelta( 10, 1, 50, 1 );

	m_pSigmaSpatialInputPort = addInputPort( "sigmaSpatial", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaSpatialInputPort->setFloatMinMaxDelta( 16, 1, 128, 1 );

	m_pSigmaRangeInputPort = addInputPort( "sigmaRange", KERNEL_PORT_DATA_TYPE_FLOAT );
	m_pSigmaRangeInputPort->setFloatMinMaxDelta( 0.2f, 0.001f, 0.8f, 0.001f );

	// ---- Output ----
	m_pOutputTextureOutputPort = addOutputPort( "outputTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pScribbledTextureOutputPort = addOutputPort( "scribbledTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
	m_pColorChangeTextureOutputPort = addOutputPort( "colorChangeTexture", KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
bool ScribbleKernel::s_bCgInitialized = false;

ScribbleKernel::ScribbleKernel() :

	GPUKernel( "Scribble" ),

	m_pFBO( GLShared::getInstance()->getSharedFramebufferObject() ),

	m_bReallocationNeeded( true ),

	m_pInputTexture( NULL ),
	m_pOutputTexture( NULL ),
	m_pScribbledOutputTexture( NULL ),
	m_pMask2DTexture( NULL ),
	m_pSource2DTexture( NULL ),
	m_pColorChangedOutputTexture( NULL ),

	m_iInputWidth( -1 ), // invalid
	m_iInputHeight( -1 ) // invalid

{
	m_nReleases = 0;
}

void ScribbleKernel::cleanup()
{
	if( m_pColorChangedOutputTexture != NULL )
	{
		delete m_pColorChangedOutputTexture;
		m_pColorChangedOutputTexture = NULL;
	}

	if( m_pOutputTexture != NULL )
	{
		delete m_pOutputTexture;
		m_pOutputTexture = NULL;
	}

	if( m_pScribbledOutputTexture != NULL )
	{
		delete m_pScribbledOutputTexture;
		m_pScribbledOutputTexture = NULL;
	}

	if( m_pMask2DTexture != NULL )
	{
		delete m_pMask2DTexture;
		m_pMask2DTexture = NULL;
	}

	if( m_pSource2DTexture != NULL )
	{
		delete m_pSource2DTexture;
		m_pSource2DTexture = NULL;
	}

	for( int i = 0; i < m_qvGrids[0].size(); ++i )
	{
		delete m_qvGrids[1].at( i );
		delete m_qvGrids[0].at( i );
	}

	m_qvGrids[1].clear();
	m_qvGrids[0].clear();
	m_qvSmoothOutputGridIndex.clear();

	m_qvWidths.clear();
	m_qvHeights.clear();
	m_qvDepths.clear();
	m_qvGridTextureWidths.clear();
	m_qvGridTextureHeights.clear();
}

// virtual
void ScribbleKernel::reallocate()
{
	// inputs
	m_pInputTexture = m_pInputTextureInputPort->pullData().getGLTextureRectangleData();
	m_aubInputRGBArray = m_pInputRGBArrayInputPort->pullData().getUnsignedByteArrayData();

	bool sizeChanged = false;
	if( m_iInputWidth != m_pInputTexture->getWidth() )
	{
		m_iInputWidth = m_pInputTexture->getWidth();
		sizeChanged = true;
	}
	if( m_iInputHeight != m_pInputTexture->getHeight() )
	{
		m_iInputHeight = m_pInputTexture->getHeight();
		sizeChanged = true;
	}

	float sigmaSpatial = m_pSigmaSpatialInputPort->pullData().getFloatData();
	float sigmaRange = m_pSigmaRangeInputPort->pullData().getFloatData();

	bool sigmaChanged = false;
	if( sigmaSpatial != m_fSigmaSpatial )
	{
		m_fSigmaSpatial = sigmaSpatial;
		sigmaChanged = true;
	}
	if( sigmaRange != m_fSigmaRange )
	{
		m_fSigmaRange = sigmaRange;
		sigmaChanged = true;
	}

	if( sizeChanged || sigmaChanged )
	{
		cleanup();

		// create 2D textures
		ArrayWithLength< float > float1Zeroes = ArrayUtils::createFloatArray( m_iInputWidth * m_iInputHeight, 0 );
		ArrayWithLength< float > float4Zeroes = ArrayUtils::createFloatArray( 4 * m_iInputWidth * m_iInputHeight, 0 );
		
		m_pOutputTexture = GLTextureRectangle::createFloat1Texture( m_iInputWidth, m_iInputHeight, 32, float1Zeroes );
		m_pScribbledOutputTexture = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, 32, float4Zeroes );
		m_pMask2DTexture = GLTextureRectangle::createFloat1Texture( m_iInputWidth, m_iInputHeight, 32, float1Zeroes );
		m_pSource2DTexture = GLTextureRectangle::createFloat1Texture( m_iInputWidth, m_iInputHeight, 32, float1Zeroes );
		m_pColorChangedOutputTexture = GLTextureRectangle::createFloat4Texture( m_iInputWidth, m_iInputHeight, 32, float4Zeroes );

		float4Zeroes.destroy();
		float1Zeroes.destroy();

		// create 3D grids
		m_iGridWidth = static_cast< int >( ( m_iInputWidth - 1 ) / m_fSigmaSpatial ) + 1;
		m_iGridHeight = static_cast< int >( ( m_iInputHeight - 1 ) / m_fSigmaSpatial ) + 1;
		m_iGridDepth = static_cast< int >( 1.0f / m_fSigmaRange ) + 1;

		int actualGridWidth = Arithmetic::roundUpToNearestPowerOfTwo( m_iGridWidth );
		int actualGridHeight = Arithmetic::roundUpToNearestPowerOfTwo( m_iGridHeight );
		int actualGridDepth = Arithmetic::roundUpToNearestPowerOfTwo( m_iGridDepth );

		printf( "width, height, depth = %d, %d, %d\n", actualGridWidth, actualGridHeight, actualGridDepth );
		printf( "nVars = %d\n", actualGridWidth * actualGridHeight * actualGridDepth );

		m_iPaddingX = ( actualGridWidth - m_iGridWidth ) / 2;
		m_iPaddingY = ( actualGridHeight - m_iGridHeight ) / 2;
		m_iPaddingZ = ( actualGridDepth - m_iGridDepth ) / 2;

		m_nLevels = Arithmetic::log2ToInt( min( min( actualGridWidth, actualGridHeight ), actualGridDepth ) ) - 1;
		for( int i = 0; i < m_nLevels; ++i )
		{
			float1Zeroes = ArrayUtils::createFloatArray( actualGridWidth * actualGridHeight * actualGridDepth, 0 );

			m_qvWidths.append( actualGridWidth );
			m_qvHeights.append( actualGridHeight );
			m_qvDepths.append( actualGridDepth );

			int gridTextureWidth = actualGridWidth * actualGridDepth;
			int gridTextureHeight = actualGridHeight;
			m_qvGridTextureWidths.append( gridTextureWidth );
			m_qvGridTextureHeights.append( gridTextureHeight );

			GLTextureRectangle* pRHS = GLTextureRectangle::createFloat4Texture( gridTextureWidth, gridTextureHeight, 32, float4Zeroes );
			m_qvRHSTextures.append( pRHS );

			GLTextureRectangle* pGrid0 = GLTextureRectangle::createFloat4Texture( gridTextureWidth, gridTextureHeight, 32, float4Zeroes );
			GLTextureRectangle* pGrid1 = GLTextureRectangle::createFloat4Texture( gridTextureWidth, gridTextureHeight, 32, float4Zeroes );

			m_qvGrids[0].append( pGrid0 );
			m_qvGrids[1].append( pGrid1 );

			actualGridWidth = actualGridWidth / 2;
			actualGridHeight = actualGridHeight / 2;
			actualGridDepth = actualGridDepth / 2;

			float1Zeroes.destroy();

			// fill it with 0's
			m_qvSmoothOutputGridIndex.append( 0 );
		}

		m_pOutputTextureOutputPort->pushData( KernelPortData( m_pOutputTexture ) );
		m_pScribbledTextureOutputPort->pushData( KernelPortData( m_pScribbledOutputTexture ) );
		m_pColorChangeTextureOutputPort->pushData( KernelPortData( m_pColorChangedOutputTexture ) );
	}

	m_bReallocationNeeded = false;
}

void ScribbleKernel::initializeCgPrograms()
{
	CgProgramManager* pProgramManager = CgProgramManager::getInstance();

	/////////////////////////////////////////////////////////////////////

	m_pRestrictConstraintsProgram = pProgramManager->getNamedProgram( "SCRIBBLE_restrictConstraints" );
	m_cgp_RC_fullConstraintsSampler = m_pRestrictConstraintsProgram->getNamedParameter( "fullConstraintsSampler" );
	m_cgp_RC_f3RestrictedGridSize = m_pRestrictConstraintsProgram->getNamedParameter( "restrictedGridSize" );
	m_cgp_RC_f3FullGridSize = m_pRestrictConstraintsProgram->getNamedParameter( "fullGridSize" );

	/////////////////////////////////////////////////////////////////////

	m_pSmoothGridProgram = pProgramManager->getNamedProgram( "SCRIBBLE_smoothGrid" );
	m_cgp_SG_gridSampler = m_pSmoothGridProgram->getNamedParameter( "gridSampler" );
	m_cgp_SG_rhsSampler = m_pSmoothGridProgram->getNamedParameter( "rhsSampler" );
	m_cgp_SG_f3GridSize = m_pSmoothGridProgram->getNamedParameter( "gridSize" );

	/////////////////////////////////////////////////////////////////////

	m_pProlongEstimateProgram = pProgramManager->getNamedProgram( "SCRIBBLE_prolongEstimate" );
	m_cgp_PEST_restrictedGridLinearSampler = m_pProlongEstimateProgram->getNamedParameter( "restrictedGridLinearSampler" );
	m_cgp_PEST_fullGridSize = m_pProlongEstimateProgram->getNamedParameter( "fullGridSize" );
	m_cgp_PEST_restrictedGridSize = m_pProlongEstimateProgram->getNamedParameter( "restrictedGridSize" );

	/////////////////////////////////////////////////////////////////////

	m_pConvergenceTestProgram = pProgramManager->getNamedProgram( "SCRIBBLE_convergenceTest" );
	m_cgp_CT_gridSampler = m_pConvergenceTestProgram->getNamedParameter( "gridSampler" );
	m_cgp_CT_rhsSampler = m_pConvergenceTestProgram->getNamedParameter( "rhsSampler" );
	m_cgp_CT_f3GridSize = m_pConvergenceTestProgram->getNamedParameter( "gridSize" );

	/////////////////////////////////////////////////////////////////////

	m_pSliceProgram = pProgramManager->getNamedProgram( "SCRIBBLE_slice" );
	m_cgp_SP_inputRGBASampler = m_pSliceProgram->getNamedParameter( "inputRGBASampler" );
	m_cgp_SP_gridSampler = m_pSliceProgram->getNamedParameter( "gridSampler" );
	m_cgp_SP_f3_rcpSigma = m_pSliceProgram->getNamedParameter( "rcpSigma" );
	m_cgp_SP_f3_gridSize = m_pSliceProgram->getNamedParameter( "gridSize" );
	m_cgp_SP_f3_paddingXYZ = m_pSliceProgram->getNamedParameter( "paddingXYZ" );

	/////////////////////////////////////////////////////////////////////

	m_pCompositeProgram = pProgramManager->getNamedProgram( "SCRIBBLE_composite" );
	m_cgp_C_inputRGBASampler = m_pCompositeProgram->getNamedParameter( "inputRGBASampler" );
	m_cgp_C_maskSampler = m_pCompositeProgram->getNamedParameter( "maskSampler" );
	m_cgp_C_valueSampler = m_pCompositeProgram->getNamedParameter( "valueSampler" );

	/////////////////////////////////////////////////////////////////////

	m_pColorChangeProgram = pProgramManager->getNamedProgram( "SCRIBBLE_colorChange" );
	m_cgp_CC_inputRGBASampler = m_pColorChangeProgram->getNamedParameter( "inputRGBASampler" );
	m_cgp_CC_influenceSampler = m_pColorChangeProgram->getNamedParameter( "influenceSampler" );
}

GLTextureRectangle* ScribbleKernel::getInputGrid( int level )
{
	int i = ( m_qvSmoothOutputGridIndex.at( level ) + 1 ) % 2;
	return m_qvGrids[i].at( level );
}

GLTextureRectangle* ScribbleKernel::getOutputGrid( int level )
{
	int i = m_qvSmoothOutputGridIndex.at( level );
	return m_qvGrids[i].at( level );
}

void ScribbleKernel::swapGrids( int level )
{
	int currentGrid = m_qvSmoothOutputGridIndex.at( level );
	currentGrid = ( currentGrid + 1 ) % 2;
	m_qvSmoothOutputGridIndex[ level ] = currentGrid;
}

void ScribbleKernel::setGridConstraints( int x, int y )
{
	// retrieve inputs
	int radius = m_pRadiusInputPort->pullData().getIntData();

	GLTextureRectangle* pRHS = m_qvRHSTextures.at( 0 );
	int gridWidth = m_qvWidths.at( 0 );

	for( int yy = y - radius; yy <= y + radius; ++yy )
	{
		if( yy >= 0 && yy < m_iInputHeight )
		{
			for( int xx = x - radius; xx <= x + radius; ++xx )
			{
				if( xx >= 0 && xx < m_iInputWidth )
				{
					if( MathUtils::distanceSquared( xx, yy, x, y ) <= radius * radius )
					{
						ubyte* clickedPixelRGB = &( m_aubInputRGBArray[ 3 * ( yy * m_iInputWidth + xx ) ] );

						float luminance = ColorUtils::rgb2luminance( clickedPixelRGB );
						int gridX = Arithmetic::roundToInt( xx / m_fSigmaSpatial + m_iPaddingX );
						int gridY = Arithmetic::roundToInt( yy / m_fSigmaSpatial + m_iPaddingY );
						int gridZ = Arithmetic::roundToInt( luminance / m_fSigmaRange + m_iPaddingZ );

						float one = 1;
						float value = ( m_iLastButtonPressed == 1 ) ? 0.f : 1.f;

						// stupidly rasterize the 1 pixel into the 3d texture

						float valueAndOne[4];
						valueAndOne[0] = value;
						valueAndOne[1] = 1;
						valueAndOne[2] = 0;
						valueAndOne[3] = 0;
						pRHS->setFloat4SubData( valueAndOne, gridZ * gridWidth + gridX, gridY, 1, 1 );

						// and do the same for the 2d
						m_pMask2DTexture->setFloat1SubData( &one, xx, yy, 1, 1 );
						m_pSource2DTexture->setFloat1SubData( &value, xx, yy, 1, 1 );
					}
				}
			}
		}
	}
}

void ScribbleKernel::solveGrid( int maxNumIterations )
{
	// TODO: call it while !passed() or something

	StopWatch w;

	for( int k = m_nLevels - 1; k >= 0; --k )
	{
		int nSamplesPassed = 1;
		int iteration = 0;
		while( iteration < maxNumIterations &&
			nSamplesPassed > 0 )
		{
			// TODO: put camera here
			int width = m_qvWidths.at( k );
			int height = m_qvHeights.at( k );
			int depth = m_qvDepths.at( k );

			int gridTextureWidth = m_qvGridTextureWidths.at( k );
			int gridTextureHeight = m_qvGridTextureHeights.at( k );
			GLUtilities::setupOrthoCamera( gridTextureWidth, gridTextureHeight );

			for( int i = 0; i < 50; ++i )
			{
				smoothGridPass( k, width, height, depth );
			}

			convergenceTestPass( k );
			nSamplesPassed = m_pOcclusionQuery->getResult();
			printf( "level = %d, nLevels = %d, occlusion query convergence test: nSamplesPassed = %d\n", k, m_nLevels, nSamplesPassed );

			++iteration;
		}

		if( k > 0 )
		{
			int gridTextureWidth = m_qvGridTextureWidths.at( k - 1 );
			int gridTextureHeight = m_qvGridTextureHeights.at( k - 1 );
			GLUtilities::setupOrthoCamera( gridTextureWidth, gridTextureHeight );

			prolongEstimatePass( k, k - 1 );
		}		
	}

	printf( "solver took %f ms\n", w.millisecondsElapsed() );
	slicePass();
}

void ScribbleKernel::restrictConstraints()
{
	for( int i = 0; i < m_nLevels - 1; ++i )
	{
		GLTextureRectangle* pFullInputConstraints = m_qvRHSTextures.at( i );
		GLTextureRectangle* pRestrictedOutputConstraints = m_qvRHSTextures.at( i + 1 );

		int restrictedWidth = m_qvWidths.at( i + 1 );
		int restrictedHeight = m_qvHeights.at( i + 1 );
		int restrictedDepth = m_qvDepths.at( i + 1 );
		int fullWidth = m_qvWidths.at( i );
		int fullHeight = m_qvHeights.at( i );
		int fullDepth = m_qvDepths.at( i );	

		int restrictedGridTextureWidth = m_qvGridTextureWidths.at( i + 1 );
		int restrictedGridTextureHeight = m_qvGridTextureHeights.at( i + 1 );

		// setup output
		m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, pRestrictedOutputConstraints );

		// setup inputs
		cgGLSetTextureParameter( m_cgp_RC_fullConstraintsSampler,
			pFullInputConstraints->getTextureId() );

		// setup uniforms
		cgGLSetParameter3f( m_cgp_RC_f3RestrictedGridSize,
			restrictedWidth, restrictedHeight, restrictedDepth );
		cgGLSetParameter3f( m_cgp_RC_f3FullGridSize,
			fullWidth, fullHeight, fullDepth );

		GLUtilities::setupOrthoCamera( restrictedGridTextureWidth, restrictedGridTextureHeight );		
		glClear( GL_COLOR_BUFFER_BIT );
		m_pRestrictConstraintsProgram->bind();
		GLUtilities::drawQuad( restrictedGridTextureWidth, restrictedGridTextureHeight );
	}
}

void ScribbleKernel::smoothGridPass( int level, int width, int height, int depth )
{
	// retrieve inputs
	GLTextureRectangle* pInput = getInputGrid( level );
	GLTextureRectangle* pRHS = m_qvRHSTextures.at( level );
	GLTextureRectangle* pOutput = getOutputGrid( level );	

	int gridTextureWidth = m_qvGridTextureWidths.at( level );
	int gridTextureHeight = m_qvGridTextureHeights.at( level );

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, pOutput );

	// setup inputs
	cgGLSetTextureParameter( m_cgp_SG_gridSampler, pInput->getTextureId() );
	cgGLSetTextureParameter( m_cgp_SG_rhsSampler, pRHS->getTextureId() );

	// set uniforms
	cgGLSetParameter3f( m_cgp_SG_f3GridSize,
		width, height, depth );

	glClear( GL_COLOR_BUFFER_BIT );	
	m_pSmoothGridProgram->bind();	
	GLUtilities::drawQuad( gridTextureWidth, gridTextureHeight );

	swapGrids( level );
}

void ScribbleKernel::prolongEstimatePass( int inputLevel, int outputLevel )
{
	GLTextureRectangle* pRestrictedInputGrid = getInputGrid( inputLevel );
	GLTextureRectangle* pFullOutputGrid = getInputGrid( outputLevel );

	// retrieve parameters
	int fullGridWidth = m_qvWidths.at( outputLevel );
	int fullGridHeight = m_qvHeights.at( outputLevel );
	int fullGridDepth = m_qvDepths.at( outputLevel );

	int restrictedGridWidth = m_qvWidths.at( inputLevel );
	int restrictedGridHeight = m_qvHeights.at( inputLevel );
	int restrictedGridDepth = m_qvDepths.at( inputLevel );

	int fullGridTextureWidth = m_qvGridTextureWidths.at( outputLevel );
	int fullGridTextureHeight = m_qvGridTextureHeights.at( outputLevel );

	// setup output
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, pFullOutputGrid );

	// setup input
	pRestrictedInputGrid->setFilterMode( GLTexture::FILTER_MODE_LINEAR, GLTexture::FILTER_MODE_LINEAR );
	cgGLSetTextureParameter( m_cgp_PEST_restrictedGridLinearSampler,
		pRestrictedInputGrid->getTextureId() );

	// set uniforms
	cgGLSetParameter3f( m_cgp_PEST_fullGridSize, fullGridWidth, fullGridHeight, fullGridDepth );
	cgGLSetParameter3f( m_cgp_PEST_restrictedGridSize, restrictedGridWidth, restrictedGridHeight, restrictedGridDepth );

	glClear( GL_COLOR_BUFFER_BIT );
	m_pProlongEstimateProgram->bind();
	GLUtilities::drawQuad( fullGridTextureWidth, fullGridTextureHeight );

	// reset input state
	pRestrictedInputGrid->setFilterMode( GLTexture::FILTER_MODE_NEAREST, GLTexture::FILTER_MODE_NEAREST );

	// do not swap
}

void ScribbleKernel::convergenceTestPass( int level )
{
	GLTextureRectangle* pInput = getInputGrid( level );
	GLTextureRectangle* pRHS = m_qvRHSTextures.at( level );
	GLTextureRectangle* pOutput = getOutputGrid( level );

	int width = m_qvWidths.at( level );
	int height = m_qvHeights.at( level );
	int depth = m_qvDepths.at( level );

	int gridTextureWidth = m_qvGridTextureWidths.at( level );
	int gridTextureHeight = m_qvGridTextureHeights.at( level );

	// setup outputs
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, pOutput );

	// setup inputs
	cgGLSetTextureParameter( m_cgp_CT_gridSampler, pInput->getTextureId() );
	cgGLSetTextureParameter( m_cgp_CT_rhsSampler, pRHS->getTextureId() );

	// setup uniform
	cgGLSetParameter3f( m_cgp_CT_f3GridSize,
		width, height, depth );

	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( gridTextureWidth, gridTextureHeight ); // TODO: remove
	m_pConvergenceTestProgram->bind();
	m_pOcclusionQuery->begin();
	GLUtilities::drawQuad( gridTextureWidth, gridTextureHeight );
	m_pOcclusionQuery->end();

	// do not swap
}

void ScribbleKernel::slicePass()
{
	// retrieve inputs	
	GLTextureRectangle* pGridTexture = getInputGrid( 0 );
	int gridWidth = m_qvWidths.at( 0 );
	int gridHeight = m_qvHeights.at( 0 );
	int gridDepth = m_qvDepths.at( 0 );
	int inputWidth = m_pInputTexture->getWidth();
	int inputHeight = m_pInputTexture->getHeight();

	// setup outputs
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pOutputTexture );

	// setup inputs
	cgGLSetTextureParameter( m_cgp_SP_inputRGBASampler, m_pInputTexture->getTextureId() );
	pGridTexture->setFilterMode( GLTexture::FILTER_MODE_LINEAR, GLTexture::FILTER_MODE_LINEAR );
	cgGLSetTextureParameter( m_cgp_SP_gridSampler, pGridTexture->getTextureId() );

	// setup uniforms
	cgGLSetParameter3f( m_cgp_SP_f3_rcpSigma,
		1.f / m_fSigmaSpatial, 1.f / m_fSigmaSpatial, 1.f / m_fSigmaRange );
	cgGLSetParameter3f( m_cgp_SP_f3_gridSize,
		gridWidth, gridHeight, gridDepth );
	cgGLSetParameter3f( m_cgp_SP_f3_paddingXYZ,
		m_iPaddingX, m_iPaddingY, m_iPaddingZ );

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( inputWidth, inputHeight );
	m_pSliceProgram->bind();
	GLUtilities::drawQuad( inputWidth, inputHeight );

	// restore state
	pGridTexture->setFilterMode( GLTexture::FILTER_MODE_NEAREST, GLTexture::FILTER_MODE_NEAREST );
}

void ScribbleKernel::compositePass()
{
	// retrieve inputs
	int inputWidth = m_pInputTexture->getWidth();
	int inputHeight = m_pInputTexture->getHeight();

	// setup outputs
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pScribbledOutputTexture );

	// setup inputs
	cgGLSetTextureParameter( m_cgp_C_inputRGBASampler, m_pInputTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_C_maskSampler, m_pMask2DTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_C_valueSampler, m_pSource2DTexture->getTextureId() );

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( inputWidth, inputHeight );
	m_pCompositeProgram->bind();
	GLUtilities::drawQuad( inputWidth, inputHeight );
}

void ScribbleKernel::colorChangePass()
{
	// setup outputs
	m_pFBO->attachTexture( GL_COLOR_ATTACHMENT0_EXT, m_pColorChangedOutputTexture );

	// setup inputs
	cgGLSetTextureParameter( m_cgp_CC_inputRGBASampler, m_pInputTexture->getTextureId() );
	cgGLSetTextureParameter( m_cgp_CC_influenceSampler, m_pOutputTexture->getTextureId() );

	// render
	glClear( GL_COLOR_BUFFER_BIT );
	GLUtilities::setupOrthoCamera( m_iInputWidth, m_iInputHeight );
	m_pColorChangeProgram->bind();
	GLUtilities::drawQuad( m_iInputWidth, m_iInputHeight );
}
