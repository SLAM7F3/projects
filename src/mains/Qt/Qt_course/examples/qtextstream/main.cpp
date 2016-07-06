#include<QtGui>

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QFile file("myfile.txt");

    // Let's read it if it exists
    if ( file.open( QIODevice::ReadOnly ) )
    {
        QTextStream stream( &file );
        QString str;
        stream >> str; // Reads _one_ word
        int number;
        stream >> number;
        qDebug() <<  str << "--" << number;
        file.close();
    }

    // Otherwise let's create the file
    else if ( file.open( QIODevice::WriteOnly ) )
    {
        QTextStream stream( &file );
        stream << "HelloWorld ";
        stream << 4711;
        file.close();
        qDebug("File written.");
    }
}
