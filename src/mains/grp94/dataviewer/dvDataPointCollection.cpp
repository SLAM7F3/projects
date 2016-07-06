/********************************************************************
 *
 *
 * Name: dvDataPointCollection.cpp
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * container class for dvDataPoints
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/
#include "dvDataPointCollection.h"
#include "base_file.h"
#define doAll(f) for(int i=0;i<collection.size();i++) collection[i].f;
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
void dvDataPointCollection::modConstrainMax(float mod)
{
	doAll(modConstrainMax(mod))
}
void dvDataPointCollection::modConstrainMin(float mod)
{
	doAll(modConstrainMin(mod))
}
void dvDataPointCollection::setConstrainMin(float val)
{
	doAll(setConstrainMin(val))
}
void dvDataPointCollection::setConstrainMax(float val)
{
	doAll(setConstrainMax(val))
	//printf("collection.size()=%d\n",collection.size());
}
void dvDataPointCollection::setConstrainMin(int dim, float val)
{
	doAll(setConstrainMin(dim,val))
}
void dvDataPointCollection::setConstrainMax(int dim, float val)
{
	doAll(setConstrainMax(dim,val))
}
float dvDataPointCollection::getConstrainMax() const
{
	return collection[0].getConstrainMax();
}
float dvDataPointCollection::getConstrainMin() const
{
	return collection[0].getConstrainMin();
}
float dvDataPointCollection::getConstrainMax(int dim) const
{
	return collection[0].getConstrainMax(dim);
}
float dvDataPointCollection::getConstrainMin(int dim) const
{
	return collection[0].getConstrainMax(dim);
}
void dvDataPointCollection::modPoffsetConstrainMin(float mod)
{
	doAll(modPoffsetConstrainMin(mod))
}
void dvDataPointCollection::modPointSize(int n)
{
	doAll(modPointSize(n))
}
void dvDataPointCollection::cycleColorDirection()
{
	doAll(cycleColorDirection())
}
void dvDataPointCollection::setUpDirection(int n)
{
	doAll(setUpDirection(n))
}
void dvDataPointCollection::setPoffsetConstrainMin(float f)
{
	doAll(setPoffsetConstrainMin(f))
}
void dvDataPointCollection::setColormap(dvColormap * cm)
{
	doAll(setColormap(cm))
}
void dvDataPointCollection::fastColor(int yes)
{
	doAll(fastColor(yes))
}
float dvDataPointCollection::getPoffsetConstrainMin() const
{
	return collection[0].getPoffsetConstrainMin();
}
void dvDataPointCollection::setViewPoint(float x, float y, float z)
{
	doAll(setViewPoint(x,y,z))
}
void dvDataPointCollection::ResetConstraints()
{
	int stride=collection[0].getStride();
	float * min=new float[stride];
	float * max=new float[stride];
	int i,j;
	for(j=0;j<stride;j++)
	{
		min[j]=collection[0].getConstrainMin(j);
		max[j]=collection[0].getConstrainMax(j);
	}
	for(i=1;i<collection.size();i++)
	{
		for(j=0;j<stride;j++)
		{
			min[j]=MIN(collection[i].getConstrainMin(j),min[j]);
			max[j]=MAX(collection[i].getConstrainMax(j),max[j]);
		}
	}
	for(j=0;j<stride;j++)
	{
		setConstrainMin(j,min[j]);
		setConstrainMax(j,max[j]);
	}
}
void dvDataPointCollection::data_dump(const char * filename_base) const
{
	//doAll(data_dump(file))
	char filename[1024];
	base_file file;
	int i;
	if(stopped){
		printf("Dumping data\n");
		sprintf(filename,"%s.fxyz",filename_base);
		file.set_filename(filename);
		file.open_output_overwrite_binary();
		for(i=0;i<size();i++)
		{
			collection[i].data_dump(file.fp);
		}
		file.close();
	}
	else if(fps<0.0)
	{
		int fnum=frame_num;
		printf("Dumping frame number %d\n",fnum);
		sprintf(filename,"%s.%04d.fxyz",filename_base,fnum);
		file.set_filename(filename);
		file.open_output_overwrite_binary();
		collection[fnum].data_dump(file.fp);
		file.close();
	}
	else
	{
		printf("Dumping data sequence\n");
		for(i=0;i<size();i++)
		{
			sprintf(filename,"%s.%04d.fxyz",filename_base,i);
			file.set_filename(filename);
			file.open_output_overwrite_binary();
			collection[i].data_dump(file.fp);
			file.close();
		}
	}
}

