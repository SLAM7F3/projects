bool SimpleImageIOHandler::read(QImage * image)
{
	if ( !device() ) return false;
	qint64 oldPos = device()->pos();
	QString formatLine;
	int width, height;
	QTextStream stream( device() );

	stream >> formatLine >> height >> width;

	*image = QImage(width,height, QImage::Format_RGB32);

	for( int y=0; y<height; y++ )
		for( int x=0; x<width; x++ )
		{	
			int r, g, b;
			stream >> r >> g >> b;
			image->setPixel(x,y, qRgb(r,g,b) );
		}
	if (!device()->isSequential()) 
		device()->seek(oldPos);
	return true;
}