#ifndef CONTROLS_H
#define CONTROLS_H

#include <common/BasicTypes.h>
#include <QHash>
#include <QPair>
#include <QSet>
#include <QWidget>

#include "FloatSliderUIData.h"
#include "IntSliderUIData.h"

class FilenameToRGBArrayKernel;
class GPUKernel;
class QCheckBox;
class QComboBox;
class QLabel;
class QLayout;
class QVBoxLayout;
class QPushButton;
class QSlider;
class QSpinBox;
class QSplineWidget;
class QTabWidget;
class KernelGraph;
class OutputWidget;

class Controls : public QWidget
{
	Q_OBJECT

public:

	static Controls* getInstance();
	void initialize( KernelGraph* pGraph,
		QVector< QPair< QString, int > > viewedOutputPorts,
		OutputWidget* pOutputWidget );

	// special cases for specific kernels
	// video
	void handleVideoFrameRead( int64 iFrameIndex );	

private slots:

	void handleHack();

	void handleCheckBoxStateChanged( int state );
	void handleFloatSliderValueChanged( int value );
	void handleIntSliderValueChanged( int value );
	void handleOutputTextureActivated( int index );
		void handleOutputTextureActivatedHelper( QComboBox* pComboBox, int index );
	void handleTextureArrayIndexChanged( int index );
		void handleTextureArrayIndexChangedHelper( QSpinBox* pSpinBox, int index );

	// special cases for specific kernels
	// video
	void handleFilenameButtonPushed();
	void handleVideoPaused();
	void handleVideoSeeked( int sliderValue );	

	// bilateral
	void handleBilateralSigmasChanged( QVector< float > spatials, QVector< float > ranges );

	// stylize
	void handleBaseRemappingCurveChanged( QVector< float > qvBaseTransferFunction );
	void handleDetailRemappingCurveChanged( QVector< float > qvDetailTransferFunction );

private:

	static Controls* s_pSingleton;
	Controls();

	void initializeControlsForGraphSources( KernelGraph* pGraph, QTabWidget* pTabWidget );
	void initializeOutputControls( KernelGraph* pGraph, QVector< QPair< QString, int > > viewedOutputPorts, QWidget* pWidget );

	void addCheckBox( QString checkBoxName,
		QVBoxLayout* pLayout,
		InputKernelPort* pInputPort );

	void addFloatSlider( QString sliderName,		
		QVBoxLayout* pLayout,
		InputKernelPort* pInputPort );

	void addIntSlider( QString sliderName,		
		QVBoxLayout* pLayout, InputKernelPort* pInputPort );	

	void addVideoWidget( GPUKernel* pKernel, QVBoxLayout* pLayout );

	// special case for video source
	QHash< QObject*, FilenameToRGBArrayKernel* > m_qhSlidersToVideoKernels;

	// special case for bilateral filter
	void addBilateralSigmaWidget( QString kernelName, GPUKernel* pKernel, QVBoxLayout* pLayout );
	QHash< QObject*, GPUKernel* > m_qhSigmaWidgetsToKernels;

	// special case for stylize
	void addStylizeCurveWidgets( QString kernelName, GPUKernel* pKernel, QVBoxLayout* pLayout );
	QHash< QObject*, GPUKernel* > m_qhStylizeBaseRemappingWidgetsToKernels;
	QHash< QObject*, GPUKernel* > m_qhStylizeDetailRemappingWidgetsToKernels;

	QHash< QObject*, InputKernelPort* > m_qhCheckBoxesToPorts;
	QHash< QObject*, IntSliderUIData > m_qhIntSlidersToUIData;
	QHash< QObject*, FloatSliderUIData > m_qhFloatSlidersToUIData;
	QHash< QObject*, GPUKernel* > m_qhButtonsToKernels;

	QVector< QComboBox* > m_qvOutputSelectionComboBoxes;
	QVector< QSpinBox* > m_qvOutputSelectionSpinBoxes;

	KernelGraph* m_pGraph;
	OutputWidget* m_pOutputWidget;
};

#endif
