/********************************************************************
 *
 *
 * Name: xyz_file.h
 *
 *
 * Author: Joseph Adams
 *
 * Description:
 * xyzp file class
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/

#ifndef XYZP_FILE
#define XYZP_FILE

#include <cstdio>
//#include <cstring>
#include <cstdlib>                               //for exit function

#include <string>                                 // STL string class

#define XYZP_FILE_VERSION 0.5

//#define _XYZP_FILE_DEBUG_


#include "base_file.h"

//using namespace std;

class xyzp_file : public base_file
{
public:
	xyzp_file(const char * filename, int type_size);   //constructor
    long getNumberOfPoints();
	virtual int getStride();
    virtual float * read_data(long max_points=0);
	virtual void move_to_data_beginning();
	virtual int getDefaultPOffset();
	virtual int getUpDirection();
	virtual void getViewPoint(float &x, float &y, float &z);
	void setStride(int stride_);
protected:
	xyzp_file();
	int n_data_points;
	int stride;
    void calculate_sizes();
	int type_size;
	long data_size;
	long header_size;
	int default_poffset;
	int updir;
};

#endif
