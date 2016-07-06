/********************************************************************
 *
 *
 * Name: fif_file.cpp
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * File Information File (fif)
 * fif files store the number of points per frame in a dynamic sequence
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/
#include "base_file.h"
class fif_file:public base_file{
public:
	fif_file(const char * filename);
	~fif_file();
	long getNumberOfPointsAt(int f);
	long getNext();
	int getNumberOfFrames();
private:
	int n_frames;
	long * n_points;
	int frameat;
};
