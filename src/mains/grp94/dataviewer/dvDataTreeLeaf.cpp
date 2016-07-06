// Last updated on 3/1/04
/********************************************************************
 *
 *
 * Name: dvDataTreeLeaf.cpp
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * Data container for the bottom (leaf) of an Data Tree.
 * Only used directly by dvDataTreeNode class.
 * 
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *********************************************************************/

#include "dvDataTreeLeaf.h"
#include "arch_dependent_headers.h"
#include "basic_math.h"

dvDataTreeLeaf::dvDataTreeLeaf(){
   colormap=NULL;
}
dvDataTreeLeaf::dvDataTreeLeaf(vector<float> min_, vector<float> max_, float * pts, CM_DATA_TYPE * colors_, int stride_, int poffset_, dvDataPointsViewProperties * p,int updir_, vector<long> & indices_):dvDataTreeNode()
{
   view_properties=p;
   colormap=NULL;
   indices=indices_;
   colors=colors_;
   stride=stride_;
   min.resize(stride);
   max.resize(stride);
   poffset=poffset_;
   updir=updir_;
   if(pts==NULL) return;
   points=(pts);
   minbox=min_;
   maxbox=max_;
   int numpoints=indices.size();
   pointstodraw=0;
   long * ni=new long[numpoints];
   if(ni==NULL)
   {
      printf("Out of Memory!\n");
      exit(1);
   }

   indicestodraw=(ni);
	

   updatePointInfo();
   float x,y,z;
   getFirstPoint(x,y,z);
   setSuggestedCameraLocationPoint(x,y,z);
   getCentroid(x,y,z);
   setSuggestedCameraLookPoint(x,y,z);
}
void dvDataTreeLeaf::modColorCode(int mod)
{
   color_code+=mod;
   color_code=modulo(color_code,4);
   if(color_code>=stride) color_code=0;
   updateColors();
}

void dvDataTreeLeaf::setColorCode(int c)
{
   color_code=c;
   color_code=modulo(color_code,4);
   if(color_code>=stride) color_code=0;
   updateColors();
}

int dvDataTreeLeaf::glDraw(bool force_redraw,clock_t deadline,CFrustum * f)
{
#ifdef NODRAW
   return;
#endif
   bool conchange=data_constraint->isChanged((long)this,color_code);
   bool pchange=data_constraint->isChanged((long)this,3);
   bool cmchange=colormap->isChanged((long)this);
   updatecolor=conchange || cmchange || updatecolor || pchange;
   bool local_changed=isChanged() || updatecolor;
   if(force_redraw || local_changed)
   {
      finished_draw=0;
   }
   if(finished_draw) return 1;
   finished_draw=1;
   if(f->BoxInFrustum(minbox[0],minbox[1],minbox[2],maxbox[0],maxbox[1],maxbox[2]))
   {
      if(updatecolor)
      {
         updateColors();
      }
      glPointSize(view_properties->getPointSize());
      float * tp;
      CM_DATA_TYPE * tc;
      tp=points;
      tc=colors;
      glVertexPointer(POSITION_LENGTH,GL_FLOAT,stride*sizeof(float),tp);
      glColorPointer(COLOR_LENGTH,COLOR_TYPE,COLOR_STRIDE,tc);
      glDrawElements(GL_POINTS,pointstodraw,GL_UNSIGNED_INT,indicestodraw);
   }
   return 1;
}
int dvDataTreeLeaf::glDraw(vector<float> & camera_position,bool force_redraw,clock_t deadline,CFrustum * f)
{
   return glDraw(force_redraw,deadline,f);
}
int dvDataTreeLeaf::getStride() const
{
   return stride;
}
void dvDataTreeLeaf::updatePointInfo()
{
   min.resize(stride);
   max.resize(stride);
   mean.resize(stride);
   int n_elements=indices.size();
   if(n_elements==0) return;
   float sum;
   long index;
   for(int i=0;i<stride;i++)
   {
      sum=0.0;
      max[i]=min[i]=points[i];
      for(int k=0;k<n_elements;k++)
      {
         index=indices[k];
         min[i]=MIN(min[i],points[index*stride+i]);
         max[i]=MAX(max[i],points[index*stride+i]);
         sum+=points[index*stride+i];
      }
      mean[i]=sum/n_elements;
   }
}

void dvDataTreeLeaf::updateColors()
{
   int j,k;
#ifdef NODRAW
   return;
#endif
   long n_elements=indices.size();
   if(colormap==NULL || view_properties==NULL || data_constraint==NULL || stride==3)
   {
      pointstodraw=0;
      for(j=0;j<n_elements;j++)
      {
         for(k=0;k<COLOR_LENGTH;k++)
         {
            colors[j*COLOR_LENGTH+k]=255;
         }
         indicestodraw[pointstodraw++]=j;
      }
   }
   else
   {
      double r;
      double minr,maxr;
      int offset=color_code;
      int apoffset=3+poffset;
      r=((maxr=data_constraint->getMax(offset))-(minr=data_constraint->getMin(offset)));
      float p,x,z,y;
      float pmin=data_constraint->getMin(apoffset),pmax=data_constraint->getMax(apoffset);
      float xmin=data_constraint->getMin(0),xmax=data_constraint->getMax(0);
      float ymin=data_constraint->getMin(1),ymax=data_constraint->getMax(1);
      float zmin=data_constraint->getMin(2),zmax=data_constraint->getMax(2);
      float v;
      int c;
      if(r==0.0) r=.1;
      float colormapsize=(float)colormap->getSize();
      pointstodraw=0;
      long index;
      for(j=0;j<n_elements;j++)
      {
         index=indices[j];
         p=points[index*stride+apoffset];
         z=points[index*stride+2];
         y=points[index*stride+1];
         x=points[index*stride];
         v=points[index*stride+offset];
         if(p>=pmin && p<=pmax && z>=zmin && z<=zmax && y>=ymin && y<=ymax && x<=xmax && x>=xmin){

            c=((v-minr)/r*(float)(colormapsize-1));

            if(c<0 || c>=colormap->getSize())
            {
               printf("BAD");
               colors[j*3]=0.;
               colors[j*3+1]=0.;
               colors[j*3+2]=0.;
            }
            else
            {
               colormap->getColor(c,colors+index*COLOR_LENGTH);
               indicestodraw[pointstodraw++]=index;
            }
         }
      }
   }
   updatecolor=false;
}
void dvDataTreeLeaf::data_dump(FILE * file)
{
   int i,j;
   bool conchange=data_constraint->isChanged((long)this,color_code);
   bool cmchange=colormap->isChanged((long)this);
   updatecolor=conchange || cmchange || updatecolor;
   if(updatecolor)
   {
      updateColors();
   }
   if(colormap==NULL || stride==3)
   {
      long nbytes=0;
      nbytes+=fwrite(points,sizeof(float),indices.size()*stride,file);
      printf("wrote %d bytes\n",nbytes*sizeof(float));
   }
   else{
      long nbytes=0;
      for(j=0;j<pointstodraw;j++)
      {
         nbytes+=fwrite(points+indices[j]*stride,sizeof(float),stride,file);
      }
      printf("Wrote %d bytes\n",nbytes);
   }
}
void dvDataTreeLeaf::getCentroid(float &x, float &y, float &z) const
{
   x=mean[0];
   y=mean[1];
   z=mean[2];
}
void dvDataTreeLeaf::getFirstPoint(float &x, float &y, float &z) const
{
   if(abs((long)min[0])<abs((long)max[0])) x=min[0];
   else x=max[0];
   if(abs((long)min[1])<abs((long)max[1])) y=min[1];
   else y=max[1];
   z=max[2];
}
void dvDataTreeLeaf::setConstraint(dvDataConstraint* d)
{
   dvData::setConstraint(d);
}
void dvDataTreeLeaf::rotateAxis(int n)
{
   /*float temp;
     for(int j=0;j<points.size();j++)
     {
     for(int i=0;i<npoints[j];i++)
     {
     temp=points[i*stride];
     points[i*stride]=points[j][i*stride+1];
     points[i*stride+1]=points[j][i*stride+2];
     points[i*stride+2]=temp;
     }
     }
     if(n>1)
     {
     rotateAxis(n-1);
     return;
     }
     updatePointInfo();
     updateColors();*/
}
void dvDataTreeLeaf::printInfo() const
{
   int showcolors=color_code;
   printf("min=%f max=%f mean=%f npts=%d\n",min[showcolors],max[showcolors],mean[showcolors],indices.size());
}
void dvDataTreeLeaf::setColormap(dvColormap * cm)
{
   colormap=cm;
}
