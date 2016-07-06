#ifndef FLOAT_SLIDER_UI
#define FLOAT_SLIDER_UI

#include <QLabel>
#include <QString>

class InputKernelPort;

class FloatSliderUIData
{
public:

	QLabel* label;
	QString prefix;
	float floatMinimum;
	float floatDelta;
	InputKernelPort* inputPort;

};

#endif
