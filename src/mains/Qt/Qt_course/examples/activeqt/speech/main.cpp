#include <QtGui>

#include "sapi.h"

class MainWin : public QWidget {
	Q_OBJECT
public:
	MainWin();
public slots:
	void slotSpeak();

private:
	SpeechLib::SpVoice mVoice;
	QLineEdit* mEdit;
	QSlider* mVolume;
	QSlider* mRate;
};

MainWin::MainWin() : QWidget(0)
{
	setWindowTitle(tr("MS Speech ActiveQt Demo"));
	QBoxLayout* top = new QVBoxLayout(this);
	mEdit = new QLineEdit( this );
	mEdit->setText(tr("Resistance is futile, you will be assimilated!"));
	mEdit->setToolTip(tr("Text to be spoken"));

	mVolume = new QSlider( this );
	mVolume->setOrientation( Qt::Horizontal );
	mVolume->setRange(0,100); 
	mVolume->setValue(mVoice.Volume());
	mVolume->setToolTip(tr("Volume"));

	mRate = new QSlider( this );
	mRate->setOrientation( Qt::Horizontal );
	mRate->setRange(0,10);
	mRate->setValue(mVoice.Rate());
	mRate->setToolTip(tr("Rate"));

	QPushButton* b = new QPushButton( tr("Speak!"), this );

	top->addWidget( mEdit );
	top->addWidget( mVolume );
	top->addWidget( mRate );
	top->addWidget( b );

	connect( b, SIGNAL( clicked() ), this, SLOT( slotSpeak() ) );
}

void MainWin::slotSpeak()
{
	mVoice.SetRate(mRate->value());
	mVoice.SetVolume(mVolume->value());
	mVoice.Speak( mEdit->text() );
}

int main( int argc, char** argv )
{
	QApplication app(argc,argv);
	QObject::connect( &app, SIGNAL(lastWindowClosed()),
					  &app, SLOT(quit()));
	MainWin w;
	w.show();
	return app.exec();
}

#include "main.moc"