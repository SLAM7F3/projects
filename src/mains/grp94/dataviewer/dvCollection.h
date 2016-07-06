/********************************************************************
 *
 *
 * Name: fif_file.cpp
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * template class for collections of data (dvData, dvDataPoints)
 * 
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/
#ifndef DVCOLLECTION
#define DVCOLLECTION
#include "dvData.h"
#include <vector>
using namespace std;
template<class T>
class dvCollection{
public:
	dvCollection();
	~dvCollection();
	int glDraw();
	void add(const T& data);
	void add(const dvCollection<T>& c);
	void setFPS(float fps);
	void modFPS(float mod);
	float getFPS();
	long size() const;
	void setFrame(int i);
	int getFrame() const;
	void nextFrame();
	void getLookPoint(float &x, float &y, float &z) const;
	void getViewPoint(float &x, float &y, float &z) const;
	void destroyElements();
	int getNumberOfElements() const;
protected:
	float fps;
	int stopped;
	vector<T> collection;
	float gltime;
	long frame_num;
};
#include <string>
#include <ctime>
template<class T>
dvCollection<T>::dvCollection()
{
	fps=0.0;
	stopped=1;
	gltime=(float)clock();
	frame_num=0;
}
template<class T>
dvCollection<T>::~dvCollection()
{

} 
template<class T>
void dvCollection<T>::setFrame(int n)
{
	frame_num=n;
}
template<class T>
int dvCollection<T>::getFrame() const
{
	return frame_num;
}
template<class T>
void dvCollection<T>::nextFrame()
{
	frame_num++;
	if(frame_num>=size()) frame_num=0;
}
template<class T>
int dvCollection<T>::glDraw()
{
	int returnvalue=0;
	if(stopped){
		for(int i=0;i<size();i++)
		{
			collection[i].glDraw();
		}
	}
	else if(fps<0.0)
	{
		collection.at(frame_num).glDraw();
	}
	else
	{
		collection[frame_num].glDraw();
		if(clock()>gltime)
		{
			nextFrame();		
			gltime=clock()+CLK_TCK/fps;
			returnvalue=1;
		}
	}
	return returnvalue;
}
template<class T>
void dvCollection<T>::destroyElements()
{
	for(int i=0;i<size();i++)
	{
		collection.at(i).destroyElements();
	}
	collection.clear();
}
template<class T>
void dvCollection<T>::setFPS(float f)
{
	if(f==0.0 || (f==fps && stopped==0)) stopped=1;
	else
	{
		fps=f;
		stopped=0;
	}
}
template<class T>
float dvCollection<T>::getFPS()
{
	return fps;
}
template<class T>
void dvCollection<T>::modFPS(float mod)
{
	fps*=mod;
}
template<class T>
void dvCollection<T>::add(const T& data)
{
	collection.push_back(data);
}
template<class T>
void dvCollection<T>::add(const dvCollection<T>& c)
{
	for(int i=0;i<c.size();i++)
	{
		collection.push_back(c.collection[i]);
	}
}
template<class T>
long dvCollection<T>::size() const
{
	return collection.size();
}
template<class T>
void dvCollection<T>::getLookPoint(float &x, float &y, float &z) const
{
	collection[frame_num].getLookPoint(x,y,z);
}
template<class T>
void dvCollection<T>::getViewPoint(float &x, float &y, float &z) const
{
	collection[frame_num].getViewPoint(x,y,z);
}
template<class T>
int dvCollection<T>::getNumberOfElements() const
{
	int n=0;
	for(int i=0;i<size();i++)
	{
		n+=collection.at(i).getNumberOfElements();
	}
	return n;
}
#endif

