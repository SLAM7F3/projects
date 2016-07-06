#include <QtGui>

class Printing :public QWidget
{
    Q_OBJECT

public:
    Printing(QWidget* parent = 0) : QWidget(parent)  {
        QCheckBox* cb = new QCheckBox("Hi");
        QLineEdit* edit = new QLineEdit;
        QPushButton *button = new QPushButton("Print");
        connect(button, SIGNAL(clicked()), this, SLOT(print()));

        QHBoxLayout* frameLay = new QHBoxLayout;
        frameLay->addWidget(cb);
        frameLay->addWidget(edit);

        QVBoxLayout* layout = new QVBoxLayout;
        layout->addLayout(frameLay);
        layout->addWidget(button);

        setLayout(layout);
    }

private slots:
    void print() {
        QPixmap m = QPixmap::grabWidget(this);

        QLabel* label = new QLabel;
        label->setPixmap(m);
        label->show();
    }
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    Printing* top = new Printing;
    top->show();

    return app.exec();
}


#include "main.moc"
