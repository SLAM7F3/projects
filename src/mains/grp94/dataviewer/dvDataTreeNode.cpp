/********************************************************************
 *
 *
 * Name: dvDataTreeNode.cpp
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * A data container to organize the data into a tree.
 * A fancier alternative to dvDataPoints.
 * 
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *********************************************************************/
#include "dvDataTreeNode.h"
#include "dvDataTreeLeaf.h"

#include "base_file.h"
#include <cmath>
#define doAll(f) for(int i=0;i<collection.size();i++) (shared_polymorphic_downcast< dvDataTreeNode >(collection[i]))->f;
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void dvDataTreeNode::setViewProperties(dvDataPointsViewProperties * p)
{
	view_properties=p;
	doAll(setViewProperties(p))
}
dvDataPointsViewProperties* dvDataTreeNode::getViewProperties()
{
	return view_properties;
}
dvDataTreeNode::dvDataTreeNode()
{
	colors=NULL;
}
dvDataTreeNode::~dvDataTreeNode()
{
	delete colors;
}
void dvDataTreeNode::setConstraint(dvDataConstraint* d)
{
	dvData::setConstraint(d);
	doAll(setConstraint(d))
	for(int j=0;j<minbox.size();j++)
	{
		data_constraint->setMin(j,minbox[j]);
		data_constraint->setMax(j,maxbox[j]);
	}
}
void dvDataTreeNode::resetConstraint()
{
	setConstraint(data_constraint);
}
void dvDataTreeNode::getFirstPoint(float &x, float &y, float &z) const
{
	if(abs((long)minbox[0])<abs((long)maxbox[0])) x=minbox[0];
	else x=maxbox[0];
	if(abs((long)minbox[1])<abs((long)maxbox[1])) y=minbox[1];
	else y=maxbox[1];
	z=maxbox[2];
}
dvDataTreeNode::dvDataTreeNode(float * points, int npoints, int stride_, int poffset, dvDataPointsViewProperties * p,int updir)
{
	stride=stride_;
	minbox.resize(stride);
	maxbox.resize(stride);
	int i,j;
	if(points==NULL) return;
	if(npoints<=0) return;
	vector<INDEX_TYPE> newindices;
	newindices.reserve(npoints);
	long index;
	for(i=0;i<stride;i++)
	{
		maxbox[i]=minbox[i]=points[i];
		
		for(int k=0;k<npoints;k++)
		{
			index=k;
			minbox[i]=MIN(minbox[i],points[index*stride+i]);
			maxbox[i]=MAX(maxbox[i],points[index*stride+i]);
			
		}
	}
	for(int k=0;k<npoints;k++)
	{
		newindices.push_back(k);
	}
	colors=new CM_DATA_TYPE[npoints*COLOR_LENGTH];
	branch(minbox,maxbox,points,colors,stride,poffset,p,updir,newindices);
	
}

dvDataTreeNode::dvDataTreeNode(vector<float> min, vector<float> max, float * points,CM_DATA_TYPE * colors,  int stride_, int poffset, dvDataPointsViewProperties * p,int updir, vector<long> & indices)
{

	stride=stride_;
	minbox.resize(stride);
	maxbox.resize(stride);
	branch(min,max,points,colors,stride,poffset,p,updir,indices);
}
void dvDataTreeNode::branch(vector<float> min, vector<float> max, float * points, CM_DATA_TYPE * colors, int stride, int poffset, dvDataPointsViewProperties * p,int updir, vector<long> & indices)
{
	view_properties=p;
	minbox=min;
	maxbox=max;
	// Create Storage
	
	int i,j,k,h;
	long tree_spread=3;
	vector<float> diff(tree_spread);
	for(i=0;i<tree_spread;i++)
	{
		diff[i]=max[i]-min[i];
	//	printf("%f\n",diff[i]);
	}
	float mindiff=*(min_element(diff.begin(),diff.end())); // smallest dimension will be broken into 2
	vector<long> dim_order(diff.size());
	vector<float> delta(diff.size());
	for(i=0;i<tree_spread;i++)
	{
		if(diff[i]<=0) dim_order[i]=1;
		else dim_order[i]=(long)ceil(diff[i]/mindiff)+1;
		delta[i]=diff[i]/(dim_order[i]);
	}
	long tree_order=1;
	for(i=0;i<tree_spread;i++)
	{
		tree_order*=dim_order[i];
	}
	vector< vector<INDEX_TYPE> > newindices(tree_order); 
	vector< vector<float> > newmin(tree_order);	// new sets of min and max
	vector< vector<float> > newmax(tree_order);
	for(i=0;i<tree_order;i++)
	{
		newmin[i].resize(stride);
		newmax[i].resize(stride);
	}
	h=0;
	// Setup new constraints
	for(i=0;i<dim_order[0];i++)
	{
		for(j=0;j<dim_order[1];j++)
		{
			for(k=0;k<dim_order[2];k++)
			{
				newmin[h][0]=minbox[0]+delta[0]*i;
				newmax[h][0]=minbox[0]+delta[0]*(i+1);
				newmin[h][1]=minbox[1]+delta[1]*j;
				newmax[h][1]=minbox[1]+delta[1]*(j+1);
				newmin[h][2]=minbox[2]+delta[2]*k;
				newmax[h][2]=minbox[2]+delta[2]*(k+1);
				h++;
			}
		}
	}
	
	
	// Setup indices for new nodes/leaves
	int numpoints=indices.size();
	float * point;
	long numz=dim_order[2];
	long numyz=numz*dim_order[1];
	long xindex,yindex,zindex,index;
	for(i=0;i<numpoints;i++)
	{
		point=points+indices[i]*stride;
		xindex=(long)((point[0]-minbox[0])/delta[0]);
		yindex=(long)((point[1]-minbox[1])/delta[1]);
		zindex=(long)((point[2]-minbox[2])/delta[2]);
		if(xindex==dim_order[0]) xindex-=1;
		if(yindex==dim_order[1]) yindex-=1;
		if(zindex==dim_order[2]) zindex-=1;
		index=xindex*numyz+yindex*numz+zindex;
#ifdef _DEBUG
		if(index>=tree_order)
		{
			printf("point=[%f,%f,%f]\nminbox=[%f,%f,%f]\nmaxbox=[%f,%f,%f]\ndelta=[%f,%f,%f]\n",point[0],point[1],point[2],minbox[0],minbox[1],minbox[2],maxbox[0],maxbox[1],maxbox[2],delta[0],delta[1],delta[2]);
			printf("i=%d j=%d k=%d\n",(long)((point[0]-minbox[0])/delta[0]),(long)((point[1]-minbox[1])/delta[1]),(long)((point[2]-minbox[2])/delta[2]));
			printf("dim_order=[%d,%d,%d]\n",dim_order[0],dim_order[1],dim_order[2]);
		}
		if(!(point[0]>=newmin[index][0] && point[0]<=newmax[index][0] && point[1]>=newmin[index][1] && point[1]<=newmax[index][1] && point[2]>=newmin[index][2] && point[2]<=newmax[index][2]))
		{
			printf("point=[%f,%f,%f]\nnewmin=[%f,%f,%f]\nnewmax=[%f,%f,%f]\n",point[0],point[1],point[2],newmin[index][0],newmin[index][1],newmin[index][2],newmax[index][0],newmax[index][1],newmax[index][2]);
		}
#endif
		newindices[index].push_back(indices[i]);
	}
	shared_ptr<dvDataTreeNode> newleaf;
	// Create Valid Nodes/Leafs
	for(i=0;i<tree_order;i++)
	{
		if(newindices[i].size()<=0) continue;
		if(newindices[i].size()<MAX_LEAF_SIZE)
		{
			newleaf.reset(new dvDataTreeLeaf(newmin[i],newmax[i],points,colors,stride,poffset,p,updir,newindices[i]));
			add(newleaf);
		}
		else if(newindices[i].size()>0)
		{
			newleaf.reset(new dvDataTreeNode(newmin[i],newmax[i],points,colors,stride,poffset,p,updir,newindices[i]));
			add(newleaf);
		}
	}
	
	setSuggestedCameraLookPoint((minbox[0]+maxbox[0])/2,(minbox[1]+maxbox[1])/2,(minbox[2]+maxbox[2])/2);
}

int dvDataTreeNode::glDraw(bool force_redraw,clock_t deadline,CFrustum * f)
{
	int i;
	bool local_changed=isChanged();
	if(local_changed || force_redraw)
	{
		force();
	}
	if(finished_draw) return 1;
	int ret;
	if(f->BoxInFrustum(minbox[0],minbox[1],minbox[2],maxbox[0],maxbox[1],maxbox[2]))
	{
		ret= dvDataCollection::glDraw(force_redraw || local_changed,deadline,f);
		if(ret==0)
		{
			return 0;
		}
	}
	return finished_draw=true;
}
int dvDataTreeNode::glDraw(vector<float> & camera_position,bool force_redraw,clock_t deadline,CFrustum * f)
{
	int i;
	bool local_changed=isChanged();
	if(local_changed || force_redraw)
	{
		force();
	}
	if(finished_draw) return 1;
	int ret;
	if(f->BoxInFrustum(minbox[0],minbox[1],minbox[2],maxbox[0],maxbox[1],maxbox[2]))
	{
		ret= dvDataCollection::glDraw(camera_position,force_redraw || local_changed,deadline,f);
		if(ret==0)
		{
			return 0;
		}
	}
	return finished_draw=true;
}
bool dvDataTreeNode::isChanged()
{
	long id=(long)this;
	bool propchange=view_properties->isChanged(id);
	return dvData::isChanged() || propchange;
}

