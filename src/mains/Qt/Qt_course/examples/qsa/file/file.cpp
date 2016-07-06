#include "file.h"
File::File()
{
}

File::File( const QString& fileName )
{
    _file.setName( fileName );
}

bool File::open( int mode )
{
    bool res = _file.open( mode );
    if ( res )
        _stream.setDevice( &_file );
    return res;
}

void File::close()
{
    _file.close();
}

void File::flush()
{
    _file.flush();
}

QString File::readLine()
{
    return _stream.readLine();
}

void File::writeLine( const QString& line )
{
    _stream << line << "\n";
}

bool File::atEnd() const
{
    return _stream.atEnd();
}

QString File::read( const QString& fileName )
{
    QFile f( fileName );
    if ( !f.open( IO_ReadOnly ) )
        return QString::null;
    QTextStream s( &f );
    QString txt = s.read();
    f.close();
    return txt;
}

bool File::write( const QString& fileName, const QString& content )
{
    QFile f( fileName );
    if ( !f.open( IO_WriteOnly ) )
        return false;
    QTextStream s( &f );
    s << content;
    f.close();
    return true;
}

bool File::exists( const QString& fileName )
{
    return QFile::exists( fileName );
}

bool File::remove( const QString& fileName )
{
    return QFile::remove( fileName );
}

QString File::errorString()
{
    return _file.errorString();
}
