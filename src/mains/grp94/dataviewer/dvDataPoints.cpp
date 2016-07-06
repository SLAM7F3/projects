// Last updated on 3/1/04

/********************************************************************
 *
 *
 * Name: dvDataPoints.cpp
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * draws points for openGL
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/
#include "dvDataPoints.h"
#include <string>
#include "arch_dependent_headers.h"
#include "basic_math.h"

#define DEFAULT_STRIDE 4
#define POSITION_LENGTH 3
#define COLOR_LENGTH 3
#define COLOR_TYPE GL_UNSIGNED_BYTE
#define COLOR_STRIDE 0
//COLOR_LENGTH*sizeof(COLOR_TYPE)
#define MIN(a,b) (((a)<(b))?(a):(b)) 
#define MAX(a,b) (((a)>(b))?(a):(b)) 
//#define NODRAW
void dvDataPoints::init(int s, int poff, float * pts, long numpoints, dvDataPointsViewProperties * p,int udir=2)
{
   updir=udir;
   colormap=NULL;
   stride=s;
   min.resize(stride);
   max.resize(stride);
   poffset=poff;
   n_elements=0;
   add(pts,numpoints);
   view_properties=p;
   color_code=2;
}
dvDataPoints::dvDataPoints()
{
   init(DEFAULT_STRIDE,0,NULL,0,NULL);
}
dvDataPoints::dvDataPoints(const dvDataPoints& dp)
{
   init(DEFAULT_STRIDE,0,NULL,0,NULL);
   *this=dp;
}
dvDataPoints::~dvDataPoints()
{
   for(int i=0;i<colors.size();i++)
   {
      delete [] colors[i];
      delete [] indices[i];
   }
}
void dvDataPoints::clear()
{
   for(int i=0;i<colors.size();i++)
   {
      delete [] colors[i];
      delete [] indices[i];
      delete [] points[i];
   }
   colors.clear();
   indices.clear();
   points.clear();
}
dvDataPoints::dvDataPoints(int s,dvDataPointsViewProperties * p)
{
   init(s,0,NULL,0,p);
}
dvDataPoints::dvDataPoints(int s, int poff,dvDataPointsViewProperties * p,int udir)
{
   init(s,poff,NULL,0,p,udir);
}
dvDataPoints::dvDataPoints(float * pts, long numpoints,dvDataPointsViewProperties * p)
{
   init(DEFAULT_STRIDE,0,pts,numpoints,p);
}
dvDataPoints::dvDataPoints(int s, int poff, float * pts, long numpoints,dvDataPointsViewProperties * p)
{
   init(s,0,pts,numpoints,p);
}
int dvDataPoints::getStride() const
{
   return stride;
}
void dvDataPoints::updatePointInfo()
{
   mean.resize(stride);
   if(n_elements==0) return;
   float sum;
   for(int i=0;i<stride;i++)
   {
      sum=0.0;
      max[i]=min[i]=points[0][i];
      for(int k=0;k<npoints.size();k++)
      {
         for(int j=0;j<npoints[k];j++)
         {
            min[i]=MIN(min[i],points[k][j*stride+i]);
            max[i]=MAX(max[i],points[k][j*stride+i]);
            sum+=points[k][j*stride+i];
         }
      }
      mean[i]=sum/n_elements;
   }
}
void dvDataPoints::setConstraint(dvDataConstraint* d)
{
   dvData::setConstraint(d);
   for(int i=0;i<stride;i++)
   {
      data_constraint->setMin(i,min[i]);
      data_constraint->setMax(i,max[i]);
   }
}
void dvDataPoints::add(float * pts, long numpoints)
{
   if(pts==NULL) return;
   points.push_back(pts);
   npoints.push_back(numpoints);
   points_to_draw.push_back(0);
   CM_DATA_TYPE * nc=new CM_DATA_TYPE[COLOR_LENGTH*numpoints];
   if(nc==NULL)
   {
      printf("Out of Memory!\n");
      exit(1);
   }
   long * ni=new long[numpoints];
   if(ni==NULL)
   {
      printf("Out of Memory!\n");
      exit(1);
   }
   colors.push_back(nc);
   indices.push_back(ni);
   n_elements+=numpoints;
   updatePointInfo();
   float x,y,z;
   getFirstPoint(x,y,z);
   dvData::setSuggestedCameraLocationPoint(x,y,z);
}
int dvDataPoints::glDraw(bool force_redraw,clock_t deadline,CFrustum * f)
{
#ifdef NODRAW
   return;
#endif
   if(data_constraint->isChanged((long)this,color_code)) updateColors();
   glPointSize(view_properties->getPointSize());
   float * tp;
   CM_DATA_TYPE * tc;
   for(int j=0;j<npoints.size();j++)
   {
      tp=points[j];
      tc=colors[j];
      pointstodraw=points_to_draw[j];
      glVertexPointer(POSITION_LENGTH,GL_FLOAT,stride*sizeof(float),tp);
      glColorPointer(COLOR_LENGTH,COLOR_TYPE,COLOR_STRIDE,tc);
      glDrawElements(GL_POINTS,pointstodraw,GL_UNSIGNED_INT,indices[j]);
   }
   return 0;
}
void dvDataPoints::updateColors()
{
   int i,j,k;
#ifdef NODRAW
   return;
#endif
   if(colormap==NULL || data_constraint==NULL || stride==3)
   {
      for(i=0;i<npoints.size();i++)
      {
         pointstodraw=0;
         for(j=0;j<npoints[i];j++)
         {
            for(k=0;k<COLOR_LENGTH;k++)
            {
               colors[i][j*COLOR_LENGTH+k]=255;
            }
            indices[i][pointstodraw++]=j;
         }
         points_to_draw[i]=pointstodraw;
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
      for(i=0;i<points.size();i++)
      {
         pointstodraw=0;
         for(j=0;j<npoints[i];j++)
         {
            p=points[i][j*stride+apoffset];
            z=points[i][j*stride+2];
            y=points[i][j*stride+1];
            x=points[i][j*stride];
            v=points[i][j*stride+offset];
            if(p>=pmin && p<=pmax && z>=zmin && z<=zmax && y>=ymin && y<=ymax && x<=xmax && x>=xmin){
               c=((v-minr)/r*(float)(colormapsize-1));

               if(c<0 || c>=colormap->getSize())
               {
				//		printf("BAD");
                  colors[i][j*3]=0.;
                  colors[i][j*3+1]=0.;
                  colors[i][j*3+2]=0.;
               }
               else
               {
                  colormap->getColor(c,colors[i]+j*COLOR_LENGTH);
                  indices[i][pointstodraw++]=j;
               }
            }
         }
         points_to_draw[i]=pointstodraw;
      }
   }
}
void dvDataPoints::data_dump(FILE * file)
{
   int i,j;
   if(colormap==NULL || stride==3)
   {
      long nbytes=0;
      printf("npoints=%d\n",npoints[0]);
      for(i=0;i<points.size();i++)
      {
         nbytes+=fwrite(points[i],sizeof(float),npoints[i]*stride,file);
      }
      printf("wrote %d bytes\n",nbytes*sizeof(float));
   }
   else{
      for(i=0;i<points.size();i++)
      {
         long nbytes=0;
         for(j=0;j<points_to_draw[i];j++)
         {
            nbytes+=fwrite(points[i]+indices[i][j]*stride,sizeof(float),stride,file);
         }
         printf("Wrote %d bytes\n",nbytes);
      }
   }
}
void dvDataPoints::getCentroid(float &x, float &y, float &z) const
{
   x=mean[0];
   y=mean[1];
   z=mean[2];
}
void dvDataPoints::getFirstPoint(float &x, float &y, float &z) const
{
   if(abs((long)min[0])<abs((long)max[0])) x=min[0];
   else x=max[0];
   if(abs((long)min[1])<abs((long)max[1])) y=min[1];
   else y=max[1];
   z=max[2];
}
void dvDataPoints::rotateAxis(int n)
{
   float temp;
   for(int j=0;j<points.size();j++)
   {
      for(int i=0;i<npoints[j];i++)
      {
         temp=points[j][i*stride];
         points[j][i*stride]=points[j][i*stride+1];
         points[j][i*stride+1]=points[j][i*stride+2];
         points[j][i*stride+2]=temp;
      }
   }
   if(n>1)
   {
      rotateAxis(n-1);
      return;
   }
   updatePointInfo();
   updateColors();
}
void dvDataPoints::setUpDirection(int n)
{
   if(n>2 || n==updir) return;
   int numrot=updir-n;
   if(numrot<0) numrot+=3;
   rotateAxis(numrot);
   updir=n;
}

void dvDataPoints::modColorCode(int mod)
{
   color_code+=mod;
   color_code=modulo(color_code,4);
   if(color_code>=stride) color_code=0;
}

void dvDataPoints::setColorCode(int c)
{
   color_code=c;
   color_code=modulo(color_code,4);
   if(color_code>=stride) color_code=0;
}

void dvDataPoints::setViewProperties(dvDataPointsViewProperties * p)
{
   view_properties=p;
}
dvDataPointsViewProperties* dvDataPoints::getViewProperties()
{
   return view_properties;
}
dvDataPoints& dvDataPoints::operator = (const dvDataPoints & d)
{
   if(&d==this) return *this;
   stride=d.stride;
   poffset=d.poffset;
   min=d.min;
   max=d.max;
   mean=d.mean;
   colormap=d.colormap;
   data_constraint=d.data_constraint;
   npoints=d.npoints;
   points=d.points;
   points_to_draw=d.points_to_draw;
   n_elements=d.n_elements;
   updir=d.updir;
   view_properties=d.view_properties;
   lookx=d.lookx;
   looky=d.looky;
   lookz=d.lookz;
   locx=d.locx;
   locy=d.locy;
   locz=d.locz;
   int i;
   for(i=0;i<colors.size();i++)
   {
      delete [] colors[i];
      delete [] indices[i];
   }
   colors.clear();
   for(i=0;i<points.size();i++)
   {
      colors.push_back(new CM_DATA_TYPE[COLOR_LENGTH*npoints[i]]);
      indices.push_back(new long[npoints[i]]);
   }
   return *this;
}
void dvDataPoints::printInfo() const
{
   int showcolors=color_code;
   printf("min=%f max=%f mean=%f npts=%d\n",min[showcolors],max[showcolors],mean[showcolors],n_elements);
}
