#ifndef FILE_H
#define FILE_H
#include <qfile.h>
#include <qtextstream.h>
#include <qobject.h>
#include <qstring.h>

class File :public QObject
{
    Q_OBJECT
    Q_ENUMS( Mode );

public:
    File();
    File( const QString& fileName );
    enum Mode {
        ReadOnly = IO_ReadOnly,
        WriteOnly = IO_WriteOnly,
        ReadWrite = IO_ReadWrite,
        Append = IO_Append,
        Truncate = IO_Truncate,
        Translate = IO_Translate
    };

public slots:
    bool open( int mode );
    void close();
    void flush();
    QString errorString();

    QString readLine();
    void writeLine( const QString& );
    bool atEnd() const;

    // "static" methods
    QString read( const QString& fileName );
    bool write( const QString& fileName, const QString& content );
    bool exists( const QString& fileName );
    bool remove( const QString& fileName );

private:
    QFile _file;
    QTextStream _stream;
};


#endif /* FILE_H */

