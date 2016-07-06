#include "Controls.h"

#include <cassert>
#include <cmath>
#include <GL/GLTexture2D.h>
#include <math/Arithmetic.h>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QStringList>
#include <QTabWidget>
#include <video/QDecoderThread.h>

#include "AppData.h"
#include "QSplineWidget.h"
#include "QBilateralSigmaWidget.h"
#include "FilenameToRGBArrayKernel.h"
#include "KernelGraph.h"
#include "InputKernelPort.h"
#include "GPUKernel.h"
#include "OutputWidget.h"

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
Controls* Controls::getInstance()
{
   if( s_pSingleton == NULL )
   {
      s_pSingleton = new Controls;
   }
   return s_pSingleton;
}

#if 0
void Controls::initialize()
{
   m_pLayout = new QVBoxLayout;

   AppData* pAppData = AppData::getInstance();	

   QLabel* pVideoProgressLabel = new QLabel( "Video Progress", this );
   m_pLayout->addWidget( pVideoProgressLabel );
   m_qhLabels[ "videoProgress" ] = pVideoProgressLabel;

   QLabel* pDesiredFPSLabel = new QLabel( QString( "Desired FPS: %1, Period: %2 ms" ).arg( 1000 / 41 ).arg( 41 ), this );
   m_pLayout->addWidget( pDesiredFPSLabel );
   m_qhLabels[ "desiredFPS" ] = pDesiredFPSLabel;

   QSlider* pDesiredFPSSlider = new QSlider( Qt::Horizontal, this );
   QObject::connect( pDesiredFPSSlider, SIGNAL( valueChanged( int ) ), pAppData, SLOT( handleDesiredFPSChanged( int ) ) );
   pDesiredFPSSlider->setFixedSize( 200, 10 );
   pDesiredFPSSlider->setMinimum( 1 );
   pDesiredFPSSlider->setMaximum( 200 );
   pDesiredFPSSlider->setValue( 41 );
   m_pLayout->addWidget( pDesiredFPSSlider );
   m_qhSliders[ "desiredFPS" ] = pDesiredFPSSlider;	
}
#endif

void Controls::initialize( KernelGraph* pGraph,
                           QVector< QPair< QString, int > > viewedOutputPorts,
                           OutputWidget* pOutputWidget )
{
   m_pGraph = pGraph;
   m_pOutputWidget = pOutputWidget;

   QVBoxLayout* pLayout = new QVBoxLayout;
	
   // parameters for graph kernels
   QTabWidget* pGraphSourceControls = new QTabWidget;
   initializeControlsForGraphSources( pGraph, pGraphSourceControls );
   pLayout->addWidget( pGraphSourceControls );

   // which outputs to display
   QWidget* pOutputControls = new QWidget;
   initializeOutputControls( pGraph, viewedOutputPorts, pOutputControls );
   pLayout->addWidget( pOutputControls );

   setLayout( pLayout );
}

void Controls::handleVideoFrameRead( int64 iFrameIndex )
{
   /*
     float fraction = static_cast< float >( iFrameIndex ) / m_pVideo->getNumFrames();
     QSlider* pVideoSeekSlider = Controls::getInstance()->getSliderByName( "videoSeekSlider" );
     pVideoSeekSlider->setSliderPosition( fraction * pVideoSeekSlider->maximum() );
   */
}

//////////////////////////////////////////////////////////////////////////
// Private Slots
//////////////////////////////////////////////////////////////////////////

void Controls::handleHack()
{
   AppData::getInstance()->setInputIsReady( true );
}

void Controls::handleCheckBoxStateChanged( int state )
{
   QObject* pSender = sender();
   InputKernelPort* pIn = m_qhCheckBoxesToPorts[ pSender ];

   bool bChecked = ( state == Qt::Checked );
   pIn->pushData( KernelPortData( bChecked ) );
}

void Controls::handleFloatSliderValueChanged( int value )
{
   QObject* pSender = sender();
   FloatSliderUIData data = m_qhFloatSlidersToUIData[ pSender ];

   float floatValue = data.floatMinimum + value * data.floatDelta;

   QLabel* pLabel = data.label;
   pLabel->setText( data.prefix + QString( "%1" ).arg( floatValue ) );

   InputKernelPort* pIn = data.inputPort;
   pIn->pushData( KernelPortData( floatValue ) );
}

void Controls::handleIntSliderValueChanged( int value )
{	
   QObject* pSender = sender();
   IntSliderUIData data = m_qhIntSlidersToUIData[ pSender ];
	
   QLabel* pLabel = data.label;
   pLabel->setText( data.prefix + QString( "%1" ).arg( value ) );

   InputKernelPort* pIn = data.inputPort;
   pIn->pushData( KernelPortData( value ) );
}

void Controls::handleOutputTextureActivated( int index )
{
   QObject* pSender = sender();
   QComboBox* pComboBox = dynamic_cast< QComboBox* >( pSender );

   handleOutputTextureActivatedHelper( pComboBox, index );
}

void Controls::handleOutputTextureActivatedHelper( QComboBox* pComboBox, int index )
{
   // get which output i'm referring to
   int outputIndex = m_qvOutputSelectionComboBoxes.indexOf( pComboBox );

	// get associated spin box
   QSpinBox* pSpinBox = m_qvOutputSelectionSpinBoxes.at( outputIndex );

   QString itemText = pComboBox->itemText( index );
   if( itemText == "NULL" )
   {
      AppData::getInstance()->setObservedPort( NULL, outputIndex, 0 );
      pSpinBox->setEnabled( false );
   }
   else
   {
      QStringList tokens = itemText.split( "." );
      QString kernelName = tokens[0];
      QString outputPortName = tokens[1];

      OutputKernelPort* pOutputPort = m_pGraph->getOutputPortByKernelAndName( kernelName, outputPortName );		

      if( pOutputPort->getType() == KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE_ARRAY )
      {
         AppData::getInstance()->setObservedPort( pOutputPort, outputIndex, 0 );

         pSpinBox->setRange( 0, 3 );
         pSpinBox->setValue( 0 );
         pSpinBox->setEnabled( true );
      }
      else
      {			
         AppData::getInstance()->setObservedPort( pOutputPort, outputIndex, 0 );
         pSpinBox->setEnabled( false );
      }
   }
}

void Controls::handleTextureArrayIndexChanged( int index )
{
   QObject* pSender = sender();
   QSpinBox* pSpinBox = dynamic_cast< QSpinBox* >( pSender );
	
   handleTextureArrayIndexChangedHelper( pSpinBox, index );
}

void Controls::handleTextureArrayIndexChangedHelper( QSpinBox* pSpinBox, int index )
{
   // get which output i'm referring to
   int outputIndex = m_qvOutputSelectionSpinBoxes.indexOf( pSpinBox );

	// get associated combo box
   QComboBox* pComboBox = m_qvOutputSelectionComboBoxes.at( outputIndex );
   QString itemText = pComboBox->currentText();

   QStringList tokens = itemText.split( "." );
   QString outputPortName = tokens[ tokens.size() - 1 ];
   QString kernelName = itemText.left( itemText.length() - outputPortName.length() - 1 );

   OutputKernelPort* pOutputPort = m_pGraph->getOutputPortByKernelAndName( kernelName, outputPortName );
   AppData::getInstance()->setObservedPort( pOutputPort, outputIndex, index );
}

void Controls::handleFilenameButtonPushed()
{
   QObject* pSender = sender();	
   FilenameToRGBArrayKernel* pKernel = dynamic_cast< FilenameToRGBArrayKernel* >( m_qhButtonsToKernels[ pSender ] );
   InputKernelPort* pInputPort = pKernel->getInputPortByName( "filename" );

   // HACK: d:/
   QString filename = QFileDialog::getOpenFileName
      (
         NULL,
         "Choose a video or image",
         "d:/",
         "Videos or Images (*.avi *.wmv *.mp4 *.mov *.jpg *.png *.tif *.bmp)"
         );

   if( filename != "" )
   {
      pInputPort->pushData( KernelPortData( filename ) );
   }
}

void Controls::handleVideoPaused()
{
   QObject* pSender = sender();	
   FilenameToRGBArrayKernel* pKernel = dynamic_cast< FilenameToRGBArrayKernel* >( m_qhButtonsToKernels[ pSender ] );
   pKernel->togglePause();
}

void Controls::handleVideoSeeked( int sliderValue )
{
   // TODO: let it seek before it starts?	
   QObject* pSender = sender();
   FilenameToRGBArrayKernel* pKernel = m_qhSlidersToVideoKernels[ pSender ];
   QSlider* pVideoSeekSlider = dynamic_cast< QSlider* >( pSender );
	
   Reference< IVideo > pVideo = pKernel->getVideo();
   QDecoderThread* pDecoderThread = pKernel->getDecoderThread();

   float fraction = Arithmetic::divideIntsToFloat( sliderValue, pVideoSeekSlider->maximum() );
   int64 seekFrame = static_cast< int64 >( fraction * pVideo->numFrames() );
   pDecoderThread->setNextFrameIndex( seekFrame );
}

void Controls::handleBilateralSigmasChanged( QVector< float > spatials, QVector< float > ranges )
{
   QObject* pSender = sender();
   GPUKernel* pKernel = m_qhSigmaWidgetsToKernels[ pSender ];

   pKernel->getInputPortByName( "sigmaSpatials" )->pushData( KernelPortData( spatials ) );
   pKernel->getInputPortByName( "sigmaRanges" )->pushData( KernelPortData( ranges ) );

   // TODO: deal with changing number of outputs...
}

void Controls::handleBaseRemappingCurveChanged( QVector< float > qvBaseTransferFunction )
{
   QObject* pSender = sender();
   GPUKernel* pKernel = m_qhStylizeBaseRemappingWidgetsToKernels[ pSender ];

   pKernel->getInputPortByName( "baseRemappingCurve" )->pushData( KernelPortData( qvBaseTransferFunction ) );
}

void Controls::handleDetailRemappingCurveChanged( QVector< float > qvDetailTransferFunction )
{
   QObject* pSender = sender();
   GPUKernel* pKernel = m_qhStylizeDetailRemappingWidgetsToKernels[ pSender ];

   pKernel->getInputPortByName( "detailRemappingCurve" )->pushData( KernelPortData( qvDetailTransferFunction ) );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

// static
Controls* Controls::s_pSingleton = NULL;

Controls::Controls()
{

}

void Controls::initializeControlsForGraphSources( KernelGraph* pGraph, QTabWidget* pTabWidget )
{
   QHash< QString, QVector< QString > > kernelNamesToSourcePortNames = pGraph->getKernelNamesToSourcePortNames();
   QHash< QString, GPUKernel* > kernelNamesToKernels = pGraph->getKernelNames();

   foreach( QString kernelName, kernelNamesToSourcePortNames.keys() )
      {
         QWidget* pWidgetForKernel = new QWidget;
         QVBoxLayout* pLayout = new QVBoxLayout;

         GPUKernel* pKernel = kernelNamesToKernels[ kernelName ];

         QVector< QString > portNames = kernelNamesToSourcePortNames[ kernelName ];
         foreach( QString portName, portNames )
            {
               InputKernelPort* pInputPort = pKernel->getInputPortByName( portName );
               KernelPortDataType type = pInputPort->getType();			

               printf( "%s.%s\n", qPrintable( kernelName ), qPrintable( portName ) );

               switch( type )
               {
                  case KERNEL_PORT_DATA_TYPE_BOOL:
                     addCheckBox( portName, pLayout, pInputPort );
                     break;

                  case KERNEL_PORT_DATA_TYPE_FLOAT:								
                     addFloatSlider( portName, pLayout, pInputPort );
                     break;

                  case KERNEL_PORT_DATA_TYPE_INT:				
                     addIntSlider( portName, pLayout, pInputPort );
                     break;			
               }
            }

         QString className = pKernel->getClassName();
         printf( "className = %s\n", qPrintable( className ) );
         if( className == "Stylize" )
         {
            addStylizeCurveWidgets( kernelName, pKernel, pLayout );
         }		
         else if( className == "FilenameToRGBArray" )
         {			
            addVideoWidget( pKernel, pLayout );
         }

         pLayout->addStretch();
         pWidgetForKernel->setLayout( pLayout );
         pTabWidget->addTab( pWidgetForKernel, kernelName );
      }
}

void Controls::initializeOutputControls( KernelGraph* pGraph,
                                         QVector< QPair< QString, int > > viewedOutputPorts,
                                         QWidget* pWidget )
{
   // HACK
   QPushButton* pStartButton = new QPushButton( "Start!" );
   QObject::connect( pStartButton, SIGNAL( clicked() ),
                     this, SLOT( handleHack() ) );
   QVBoxLayout* pLayout = new QVBoxLayout;
   pLayout->addWidget( pStartButton );

   // HACK
   QPushButton* pSaveButton = new QPushButton( "Save Current Frame", this );
   QObject::connect( pSaveButton, SIGNAL( clicked() ), AppData::getInstance(), SLOT( handleSaveFrameClicked() ) );
   pLayout->addWidget( pSaveButton );

   // grab the list of all texture output ports	
   QStringList comboBoxItemNames;
   comboBoxItemNames.append( "NULL" );

   QHash< QString, GPUKernel* > kernelNamesToKernels = pGraph->getKernelNames();
	
   foreach( QString kernelName, kernelNamesToKernels.keys() )
      {
         GPUKernel* pKernel = kernelNamesToKernels[ kernelName ];
         QHash< QString, OutputKernelPort* > outputPortNamesToOutputPorts = pKernel->getOutputPorts();
		
         foreach( QString outputPortName, outputPortNamesToOutputPorts.keys() )
            {
               OutputKernelPort* pOutputPort = outputPortNamesToOutputPorts[ outputPortName ];
               if( ( pOutputPort->getType() == KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE ) ||
                   ( pOutputPort->getType() == KERNEL_PORT_DATA_TYPE_GL_TEXTURE_RECTANGLE_ARRAY ) )
               {
                  QString itemName = QString( "%1.%2" ).arg( kernelName ).arg( outputPortName );
                  comboBoxItemNames.append( itemName );
               }
            }
      }	

   QGridLayout* pComboBoxGridLayout = new QGridLayout;
   pLayout->addLayout( pComboBoxGridLayout );

   int nOutputs = m_pOutputWidget->getNumOutputs();
   for( int i = 0; i < nOutputs; ++i )
   {
      QComboBox* pComboBox = new QComboBox;
      pComboBox->setEditable( false );
      pComboBox->addItems( comboBoxItemNames );

      QSpinBox* pSpinBox = new QSpinBox;
      pSpinBox->setEnabled( false );

      pComboBoxGridLayout->addWidget( pComboBox, i, 0 );
      pComboBoxGridLayout->addWidget( pSpinBox, i, 1 );

      m_qvOutputSelectionComboBoxes.append( pComboBox );
      m_qvOutputSelectionSpinBoxes.append( pSpinBox );

      QObject::connect( pComboBox, SIGNAL( activated( int ) ),
			this, SLOT( handleOutputTextureActivated( int ) ) );
      QObject::connect( pSpinBox, SIGNAL( valueChanged( int ) ),
			this, SLOT( handleTextureArrayIndexChanged( int ) ) );
   }

   // set defaults
   for( int i = 0; i < viewedOutputPorts.size(); ++i )
   {
      QPair< QString, int > kernelDotPortAndIndex = viewedOutputPorts.at( i );
      QString kernelDotPort = kernelDotPortAndIndex.first;
      int index = kernelDotPortAndIndex.second;

      for( int j = 0; j < comboBoxItemNames.size(); ++j )
      {
         if( kernelDotPort == comboBoxItemNames[j] )
         {
            m_qvOutputSelectionComboBoxes[i]->setCurrentIndex( j );				
				
            if( index > -1 )
            {
               m_qvOutputSelectionSpinBoxes[i]->setValue( index );
            }

            handleOutputTextureActivatedHelper( m_qvOutputSelectionComboBoxes[i], j );
            handleTextureArrayIndexChangedHelper( m_qvOutputSelectionSpinBoxes[i], index );
         }
      }
   }
	
   pWidget->setLayout( pLayout );
}

void Controls::addCheckBox( QString checkBoxName, QVBoxLayout* pLayout, InputKernelPort* pInputPort )
{
   QCheckBox* pCheckBox = new QCheckBox( checkBoxName );
   bool checked = pInputPort->pullData().getBoolData();
   pCheckBox->setChecked( checked );
   pLayout->addWidget( pCheckBox );

   m_qhCheckBoxesToPorts[ pCheckBox ] = pInputPort;

   QObject::connect( pCheckBox, SIGNAL( stateChanged( int ) ),
                     this, SLOT( handleCheckBoxStateChanged( int ) ) );
}

void Controls::addFloatSlider( QString sliderName,							  
                               QVBoxLayout* pLayout,
                               InputKernelPort* pInputPort )
{
   float value;
   float min;
   float max;
   float delta;
   // TODO: clamp
   pInputPort->getFloatMinMaxDelta( &value, &min, &max, &delta );

   int nBins = Arithmetic::roundToInt( ( max - min ) / delta );
   // in case it divides evenly
   if( ( nBins * delta ) > ( max - min ) )
   {
      --nBins;
   }
   int intValue = Arithmetic::roundToInt( ( value - min ) / delta );
   float floatValue = min + intValue * delta;
	
   FloatSliderUIData uiData;
   uiData.prefix = sliderName + ": ";
   uiData.floatDelta = delta;
   uiData.floatMinimum = min;
   uiData.inputPort = pInputPort;

   QLabel* pLabel = new QLabel( uiData.prefix + QString( "%1" ).arg( floatValue ) );
   uiData.label = pLabel;

   QSlider* pSlider = new QSlider( Qt::Horizontal );
   pSlider->setValue( intValue );
   pSlider->setRange( 0, nBins );
   pSlider->setSingleStep( 1 );

   pLayout->addWidget( pLabel );
   pLayout->addWidget( pSlider );	

   m_qhFloatSlidersToUIData[ pSlider ] = uiData;	

   QObject::connect( pSlider, SIGNAL( valueChanged( int ) ),
                     this, SLOT( handleFloatSliderValueChanged( int ) ) );
}

void Controls::addIntSlider( QString sliderName,							
                             QVBoxLayout* pLayout,
                             InputKernelPort* pInputPort )
{
   int value;
   int min;
   int max;
   int delta;
   // TODO: clamp
   pInputPort->getIntMinMaxDelta( &value, &min, &max, &delta );

   IntSliderUIData uiData;
   uiData.prefix = sliderName + ": ";
   uiData.inputPort = pInputPort;

   QLabel* pLabel = new QLabel( uiData.prefix + QString( "%1" ).arg( value ) );
   uiData.label = pLabel;
	
   QSlider* pSlider = new QSlider( Qt::Horizontal );
   pSlider->setValue( value );
   pSlider->setRange( min, max );
   pSlider->setSingleStep( delta );

   pLayout->addWidget( pLabel );
   pLayout->addWidget( pSlider );	

   m_qhIntSlidersToUIData[ pSlider ] = uiData;

   QObject::connect( pSlider, SIGNAL( valueChanged( int ) ),
                     this, SLOT( handleIntSliderValueChanged( int ) ) );
}

void Controls::addVideoWidget( GPUKernel* pKernel, QVBoxLayout* pLayout )
{
   // load file button
   QPushButton* pLoadButton = new QPushButton( "Load file" );
   pLayout->addWidget( pLoadButton );
   m_qhButtonsToKernels[ pLoadButton ] = pKernel;	
   QObject::connect( pLoadButton, SIGNAL( clicked() ),
                     this, SLOT( handleFilenameButtonPushed() ) );

   // pause button
   QPushButton* pPauseButton = new QPushButton( "Pause" );
   pLayout->addWidget( pPauseButton );
   m_qhButtonsToKernels[ pPauseButton ] = pKernel;	
   QObject::connect( pPauseButton, SIGNAL( clicked() ),
                     this, SLOT( handleVideoPaused() ) );

   // label for slider
   QLabel* pLabel = new QLabel( "Seek" );
   pLayout->addWidget( pLabel );

	// slider
   QSlider* pVideoSeekSlider = new QSlider( Qt::Horizontal );
   /*
	QObject::connect( pVideoSeekSlider, SIGNAL( sliderPressed() ), pAppData, SLOT( pause() ) );
	QObject::connect( pVideoSeekSlider, SIGNAL( sliderReleased() ), pAppData, SLOT( unpause() ) );
	*/	
   pVideoSeekSlider->setMinimum( 0 );
   pVideoSeekSlider->setMaximum( 100 );
   pVideoSeekSlider->setValue( 0 );
   pVideoSeekSlider->setTracking( false );
   pLayout->addWidget( pVideoSeekSlider );
   m_qhSlidersToVideoKernels[ pVideoSeekSlider ] = dynamic_cast< FilenameToRGBArrayKernel* >( pKernel );
   QObject::connect( pVideoSeekSlider, SIGNAL( valueChanged( int ) ), this, SLOT( handleVideoSeeked( int ) ) );
}

void Controls::addBilateralSigmaWidget( QString kernelName, GPUKernel* pKernel, QVBoxLayout* pLayout )
{
   // get the sigma ports
   // if they're sources, add them
   InputKernelPort* pSamplingSpatialsInputPort = pKernel->getInputPortByName( "sigmaSpatials" );
   InputKernelPort* pSamplingRangesInputPort = pKernel->getInputPortByName( "sigmaRanges" );

   bool isSource = pSamplingSpatialsInputPort->isSource();
   assert( isSource == pSamplingRangesInputPort->isSource() );

	// for video
	// QBilateralSigmaWidget* pSigmaWidget = new QBilateralSigmaWidget( 200, 200, 10, 12.f, 50, 0.1f, 0.4f );

	// original
   QBilateralSigmaWidget* pSigmaWidget = new QBilateralSigmaWidget( 200, 200, 10, 5.f, 50.f, 0.01f, 1.f );
   if( isSource )
   {
      QVector< float > samplingSpatials = pSamplingSpatialsInputPort->pullData().getFloatArrayData();
      QVector< float > samplingRanges = pSamplingRangesInputPort->pullData().getFloatArrayData();

      int nControlPoints = samplingSpatials.size();
      assert( nControlPoints == samplingRanges.size() );

      for( int i = 0; i < nControlPoints; ++i )
      {
         float samplingSpatial = samplingSpatials.at( i );
         float samplingRange = samplingRanges.at( i );
         pSigmaWidget->appendControlPoint( samplingSpatial, samplingRange );
      }
   }
   pLayout->addWidget( pSigmaWidget );

	// associate widget with kernel
   m_qhSigmaWidgetsToKernels[ pSigmaWidget ] = pKernel;

   QObject::connect( pSigmaWidget, SIGNAL( controlPointsChanged( QVector< float >, QVector< float > ) ),
                     this, SLOT( handleBilateralSigmasChanged( QVector< float >, QVector< float > ) ) );
}

void Controls::addStylizeCurveWidgets( QString kernelName, GPUKernel* pKernel, QVBoxLayout* pLayout )
{
   // base
   QLabel* pBaseLabel = new QLabel( "Base" );
   pLayout->addWidget( pBaseLabel );	
	
   QSplineWidget* pBaseSplineWidget = new QSplineWidget;
   pLayout->addWidget( pBaseSplineWidget );	
   m_qhStylizeBaseRemappingWidgetsToKernels[ pBaseSplineWidget ] = pKernel;

	// detail
   QLabel* pDetailLabel = new QLabel( "Detail" );
   pLayout->addWidget( pDetailLabel );

   QSplineWidget* pDetailSplineWidget = new QSplineWidget;
   pLayout->addWidget( pDetailSplineWidget );
   m_qhStylizeDetailRemappingWidgetsToKernels[ pDetailSplineWidget ] = pKernel;

   // HACK
   pKernel->getInputPortByName( "baseRemappingCurve" )->pushData( KernelPortData( pBaseSplineWidget->getTransferFunction() ) );
   pKernel->getInputPortByName( "detailRemappingCurve" )->pushData( KernelPortData( pDetailSplineWidget->getTransferFunction() ) );

   QObject::connect( pBaseSplineWidget, SIGNAL( transferFunctionChanged( QVector< float > ) ),
                     this, SLOT( handleBaseRemappingCurveChanged( QVector< float > ) ) );

   QObject::connect( pDetailSplineWidget, SIGNAL( transferFunctionChanged( QVector< float > ) ),
                     this, SLOT( handleDetailRemappingCurveChanged( QVector< float > ) ) );
}
