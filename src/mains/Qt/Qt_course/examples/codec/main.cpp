#include <QtGui>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QMap<QString,QString> encoding;
    encoding.insert("chinese", "Big5" );
    encoding.insert("danish", "ISO8859-1" );
    encoding.insert("japanese", "eucJP" );
    encoding.insert("english", "ISO8859-1");
    encoding.insert("greek", "ISO8859-7");
    encoding.insert("japanese-wrong-encoding", "ISO8859-1" );

    QTabWidget* tab = new QTabWidget(0);
    Q_FOREACH ( const QString& language, encoding.keys() ) {
        QFile f("text-"+language);
        f.open(QIODevice::ReadOnly);
        QByteArray data = f.readAll();
        QTextCodec *codec = QTextCodec::codecForName( encoding[language] );
        QString text = codec->toUnicode(data);
        QLabel* label = new QLabel( text );
        label->setAlignment( Qt::TextWordWrap );

        tab->insertTab( label, language );
    }

    tab->show();

    return app.exec();
}

