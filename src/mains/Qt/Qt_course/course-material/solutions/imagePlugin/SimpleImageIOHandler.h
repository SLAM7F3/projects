#ifndef SIMPLEIMAGEIOHANDLER_H
#define SIMPLEIMAGEIOHANDLER_H

#include <QImageIOHandler>

#include <QSize>
#include <QVariant>

class SimpleImageIOHandler : public QImageIOHandler
{
public:
	SimpleImageIOHandler(QIODevice *device);
	virtual ~SimpleImageIOHandler();
	virtual bool canRead() const;
	virtual bool supportsOption(ImageOption option) const;
	virtual QVariant option(ImageOption option) const;
	virtual bool read(QImage * image);
	virtual bool write(const QImage & image);

	// Convienice
	static bool canRead(QIODevice *device);

};



#endif

