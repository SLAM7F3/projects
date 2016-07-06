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
#include "fif_file.h"
fif_file::fif_file(const char * filename):base_file(filename)
{
	open_input_read_only_binary();
	n_frames=file_size()/sizeof(long);
	n_points=new long[n_frames];
	fread(n_points,n_frames,sizeof(long),fp);
	close();
	frameat=0;
}
fif_file::~fif_file()
{
	delete [] n_points;
}
long fif_file::getNumberOfPointsAt(int f)
{
	return n_points[f];
}
long fif_file::getNext()
{
	if(frameat>=n_frames) frameat=0;
	return n_points[frameat++];
}
int fif_file::getNumberOfFrames()
{
	return n_frames;
}