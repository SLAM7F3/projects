#include <QtGui>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QStringList langs;
    langs << "chinese" << "danish" << "japanese" << "english" << "greek" << "japanese-wrong-encoding";
    QMap<QString,QString> encoding;
    encoding.insert("chinese", "Big5" );
    encoding.insert("danish", "ISO8859-1" );
    encoding.insert("japanese", "eucJP" );
    encoding.insert("english", "ISO8859-1");
    encoding.insert("greek", "ISO8859-7");
    encoding.insert("japanese-wrong-encoding", "ISO8859-1" );

    QTabWidget* tab = new QTabWidget(0);
    for( QStringList::Iterator it = langs.begin(); it != langs.end(); ++it ) {
        QFile f("text-"+*it);
        f.open(QIODevice::ReadOnly);
        QTextStream stream(&f);
        QString text = stream.read();
        QTextCodec *codec = QTextCodec::codecForName( encoding[*it] );
        text = codec->toUnicode(text);
        QLabel* label = new QLabel( text );
        label->setAlignment( Qt::TextWordWrap );

        tab->insertTab( label, *it );
    }

    tab->show();

    return app.exec();
}

