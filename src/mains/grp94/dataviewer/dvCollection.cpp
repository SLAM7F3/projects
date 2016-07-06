#include "dvCollection.h"
#include <string>
#include <ctime>
template<class T>
dvCollection<T>::dvCollection()
{
	fps=0.0;
	gltime=(float)clock();
	frame_num=0;
}
template<class T>
dvCollection<T>::~dvCollection()
{

}
template<class T>
void dvCollection<T>::glDraw()
{
	if(fps==0.0){
		for(int i=0;i<size();i++)
		{
			collection[i].glDraw();
		}
	}
	else
	{
		if(clock()>gltime)
		{
			collection[frame_num++].glDraw();
			gltime=clock()+CLK_TCK/fps;
			if(frame_num>=size()) frame_num=0;
		}
	}
}
template<class T>
void dvCollection<T>::setFPS(float f)
{
	fps=f;
}
template<class T>
void dvCollection<T>::add(T data)
{
	collection.push_back(data);
}
template<class T>
void dvCollection<T>::add(dvCollection<T> c)
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
void dvCollection<T>::getDefaultLookPoint(float &x, float &y, float &z) const
{
	collection[frame_num].getDefaultLookPoint(x,y,z);
}
template<class T>
void dvCollection<T>::getDefaultViewPoint(float &x, float &y, float &z) const
{
	collection[frame_num].getDefaultViewPoint(x,y,z);
}

