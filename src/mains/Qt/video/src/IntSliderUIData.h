#ifndef INT_SLIDER_UI_DATA_H
#define INT_SLIDER_UI_DATA_H

#include <QLabel>
#include <QString>

class InputKernelPort;

class IntSliderUIData
{
public:

	QLabel* label;
	QString prefix;
	InputKernelPort* inputPort;
};

#endif
