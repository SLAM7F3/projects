// ==========================================================================
// Last updated on 6/6/04
// ==========================================================================

/********************************************************************
 *
 * Name: dvDataCollection.cpp
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * a container class for dvData
 *
 **********************************************************************/

#include "basic_math.h"
#include "dvDataCollection.h"
#include "dvDataTreeNode.h" // for debugging only!!!

#define doAll(f) for (int i=0;i<collection.size();i++) getData(i)->f;
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

dvDataCollection::dvDataCollection()
{
   finished_force_pos=0;
}

shared_ptr<dvData> & dvDataCollection::getData(long index)
{
   return collection[index];
}

void dvDataCollection::force()
{
   int i;
   finished_draw=false;
   for (i=0;i<collection.size();i++)
   {
      finished_force[i]=false;
   }
   finished_force_pos=0;
}

int dvDataCollection::glDraw(bool force_redraw,clock_t deadline,CFrustum * f)
{
   int i=0;
   clock_t now;
   int ret;

   bool local_changed=isChanged();
   if (local_changed || force_redraw)
   {
      force();
   }
   if (finished_draw) return 1;
   for (i=0;i<collection.size();i++)
   {
      if (deadline!=0)
      {
         now=clock();
			
         if (now>deadline)
         {
            return 0;
         }
      }
		
      ret=getData(i)->glDraw(!finished_force[i],deadline,f);
      if (finished_force[i]==false) 
      {
         finished_force_pos++;
         finished_force[i]=true;
      }
      if (ret==0)
      {
         return 0;
      }
   }
   return finished_draw=ret;
}
int dvDataCollection::glDraw(vector<float> & camera_position,bool force_redraw,clock_t deadline,CFrustum * f)
{
   int i=0;
   clock_t now;
   int ret;
   bool local_changed=isChanged();
   if (local_changed || force_redraw)
   {
      force();
   }
   if (finished_draw) return 1;
   for (i=0;i<sorter.size();i++)
   {
      sorter[i].first=getData(sorter[i].second)->distance(camera_position[0],camera_position[1],camera_position[2]);
   }
   sort(sorter.begin(),sorter.end());
   long index;
   for (i=0;i<sorter.size();i++)
   {
      if (deadline!=0)
      {
         now=clock();
			
         if (now>deadline)
         {
            return 0;
         }
      }
      index=sorter[i].second;
      ret=getData(index)->glDraw(camera_position,!finished_force[index],deadline,f);
      finished_force[index]=true;
      if (ret==0)
      {
         return 0;
      }
   }
   return finished_draw=ret;
}
void dvDataCollection::clear()
{
   doAll(clear())
      collection.clear();
}
void dvDataCollection::add(const shared_ptr<dvData> & data)
{
   pair<float,long> p(0,collection.size());
   collection.push_back(data);
   sorter.push_back(p);
   finished_force.resize(collection.size());
   for (int i=0;i<collection.size();i++)
   {
      finished_force[i]=false;
   }
   float x,y,z;
}
long dvDataCollection::size() const
{
   return collection.size();
}
int dvDataCollection::getNumberOfElements() const
{
   int n=0;
   for (long i=0;i<size();i++)
   {
      n+=collection[i]->getNumberOfElements();
   }
   return n;
}
bool dvDataCollection::isChanged()
{
   bool c=dvData::isChanged();
//	for (int i=0;i<size();i++)
//	{
//		c=c||getData(i)->isChanged();
//	}
   return c;
}
void dvDataCollection::setColormap(dvColormap* c)
{
   dvData::setColormap(c);
   doAll(setColormap(c));
}
void dvDataCollection::setConstraint(dvDataConstraint* d)
{
   dvData::setConstraint(d);
   doAll(setConstraint(d))
      }

void dvDataCollection::modColorCode(int mod)
{
   doAll(modColorCode(mod))
      if (size()>0) 
      {
         dvData::setColorCode(getData(0)->getColorCode());
      }
   
      else 
      {
         dvData::setColorCode(mod);
      }
}

void dvDataCollection::setColorCode(int c)
{
   doAll(setColorCode(c))
      if (size()>0) dvData::setColorCode(getData(0)->getColorCode());
      else dvData::setColorCode(c);
}
void dvDataCollection::data_dump(FILE * file)
{
   doAll(data_dump(file))
      }
void dvDataCollection::resetConstraint()
{
   if (data_constraint==NULL)
   {
      printf("no constraint\n");
      return;
   }
   int i,j;
   dvDataConstraint origc;
   dvDataConstraint * tempc;
   origc=*data_constraint;
   float tval,oval;
   float tfloat;
   if (size()>0)
   {
      getData(0)->resetConstraint();
      tempc=getData(0)->getConstraint();
      for (i=0;i<tempc->size();i++)
      {
         origc.setMin(i,tempc->getMin(i));
         origc.setMax(i,tempc->getMax(i));
			
      }
   }
   for (i=1;i<size();i++)
   {
      getData(i)->resetConstraint();
      tempc=getData(i)->getConstraint();
      for (j=0;j<tempc->size();j++)
      {
         tval=tempc->getMin(j);
         oval=origc.getMin(j);
         tfloat=MIN(tval,oval);
         origc.setMin(j,tfloat);
         tfloat=MAX(tempc->getMax(j),origc.getMax(j));
         origc.setMax(j,tfloat);
			
      }
   }
   *data_constraint=origc;
}
