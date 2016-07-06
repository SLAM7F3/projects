#include <QtGui>

class Test :public QDialog
{
    Q_OBJECT
public:
    Test( QWidget* parent = 0 ) :QDialog(parent)
    {
        QLabel* label = new QLabel("<p>This example shows that you should override accept and reject rather than "
                                   "implementing new slots like slotOK() and slotCancel().</p>"
                                   "<p>The reason is that pressing escape or pressing the window manager close button will make reject() fire, "
                                   "but of course slotCancel() would not fire, how would Qt know to hook the escape key up with that slot?</p>");
        label->setWordWrap(true);

        QLineEdit* edit = new QLineEdit;

        QDialogButtonBox* buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
        connect( buttons, SIGNAL( accepted() ), this, SLOT( slotOK() ) );
        connect( buttons, SIGNAL( rejected() ), this, SLOT( slotCancel() ) );

        QVBoxLayout* vlay = new QVBoxLayout( this );
        vlay->addWidget( label );
        vlay->addWidget( edit );
        vlay->addWidget( buttons );
    }

public slots:
    void accept()
    {
        // Override from QDialog.
        qDebug( "accept" );
        QDialog::accept();
    }

    void reject()
    {
        qDebug("reject");
        QDialog::reject();
    }

protected slots:
    void slotOK()
    {
        qDebug( "slotOK");
        accept();
    }

    void slotCancel()
    {
        qDebug( "slotCancel");
        reject();
    }
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    Test* test = new Test;
    test->exec();

    // return app.exec();
}

#include "main.moc"
