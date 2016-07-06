/********************************************************************
 *
 *
 * Name: dataviewermain.c
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * main entry point for OpenGL data viewer for xyz data
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 * $Log: dataviewermain.cpp,v $
 * Revision 1.2  2003/05/19 15:25:55  jadams
 * Fixed to now work with gcc3
 *
 * 
 * 36    2/27/03 5:13p Skelly
 * changed Octree to DataTree
 * 
 * 35    2/24/03 4:17p Skelly
 * 
 * 34    2/23/03 4:50p Skelly
 * cross_platform stuff
 * 
 * 33    2/20/03 3:26p Skelly
 * use of boost::shared_ptr
 * 
 * 32    2/20/03 2:54p Skelly
 * 
 * 31    2/16/03 8:29a Skelly
 * minor detail
 * 
 * 30    2/15/03 4:12p Skelly
 * added full octree support
 * 
 * 29    2/09/03 5:35p Skelly
 * octree stuff
 * 
 * 28    1/21/03 6:44p Skelly
 * fixed JIGSAW viewer to start with proper z constraints
 * 
 * 27    1/09/03 6:20p Skelly
 * command line options are re-enabled and stride=3
 * 
 * 26    12/03/02 3:09p Skelly
 * autoplay dynamic data
 * .3dp files use sensor_position as default camera location
 * 
 * 25    11/25/02 11:31a Skelly
 * 
 * 24    11/21/02 5:14p Skelly
 * 
 * 23    11/20/02 1:32p Skelly
 * 
 * 22    11/19/02 2:11p Skelly
 * 
 * 21    11/11/02 7:09p Skelly
 * 
 * 20    11/08/02 1:25p Skelly
 * v 2.4.3
 * 
 * 19    10/31/02 8:51p Skelly
 * 
 * 18    10/15/02 5:31p Skelly
 * Changed dvDataPoints to store pointer to data rather than a copy
 * 
 * 17    10/14/02 6:20p Skelly
 * v2.4.0
 * Complete Reorganization of code into C++ classes
 * main stuff is still working, but some options disabled
 * 
 * 16    9/10/02 1:26p Skelly
 * 
 * 15    9/09/02 6:07p Skelly
 * 
 * 14    8/28/02 8:32p Skelly
 * 
 * 13    8/20/02 5:36p Skelly
 * Joe Do's GLUI
 * 
 * 12    8/20/02 2:16p Skelly
 * 
 * 11    8/16/02 6:30p Skelly
 * 
 * 10    8/13/02 7:03p Skelly
 * 
 * 9     8/07/02 1:28p Skelly
 * 
 * 8     7/27/02 6:19p Skelly
 * Revision 1.4  2002/07/11 21:41:22  jadams
 * fixed casts to explicit call type on malloc (for C++)
 *
 * Revision 1.3  2002/06/28 18:53:31  jadams
 * added endian flags for MAC OS X
 *
 * Revision 1.2  2002/06/25 18:49:53  jadams
 * added usage help
 *
 * Revision 1.1  2002/06/25 18:12:48  jadams
 * *** empty log message ***
 *
 * Revision 1.3  2002/06/25 18:00:18  jadams
 * Incorporated all of lukes new changes to make version 2
 *
 * have a good .pr file for tmake (we should be able to cross compile!)
 *
 * Revision 1.2  2002/04/03 17:46:59  jadams
 * added colormap functions so you no longger need to read the colormap file
 *
 * Revision 1.1.1.1  2002/04/01 20:21:37  jadams
 * Luke's kickin' 3D data viewer
 *
 *
 **********************************************************************/

#include "dataviewer.h"
#include "dvDataCollection.h"
#include "dvDataPoints.h"
#include "dvDataPointsViewProperties.h"
#include "fif_file.h"
#include "harris_file.h"
#define DEFAULT_POINTS_PER_FRAME 1024

///////////////////////////////// MAIN \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	The Main function.  Loads data, initializes OpenGL
/////
///////////////////////////////// MAIN \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

int main(int argc, char **argv)
{
   double pthresh=0.0;
   char *xyzpfilename=NULL;
   char *colormap=NULL;
   char *framesfilename=NULL;
   int nframe=1;
   int i;
   int framesperdraw=0;
   int stride=-1;
   int datatypesize=sizeof(float);
   int poffset=-1;
   int updir=-1;
   int force_xyzp=0;
   long max_points=0;
   glutInit(&argc, argv);
   printf ("\n %s v%s\n\n", argv[0],DATAVIEWER_VERSION);
//	std::cout << '\n' << argv[0] << ' ' << DATAVIEWER_VERSION << '\n';
//	std::cout << CLOCKS_PER_SEC << endl;

   printf ("Number of arguments = %d\n", (argc-1));
   if(argc<2)
   {
      printf("You must specify data file!\n");
      exit(1);
   }
	
   for(i=1;i<argc;i++){
      //	printf("i=%d\n",i);
      if(strcmp(argv[i],"-pdet")==0){
         pthresh=atof(argv[++i]);
      }
      else if(strcmp(argv[i],"-movie")==0){
         framesperdraw=atoi(argv[++i]);
      }
      else if(strcmp(argv[i],"-fif")==0){
         framesfilename=argv[++i];
         if(framesperdraw==0) framesperdraw=1;
      }
      else if(strcmp(argv[i],"-double")==0){
         datatypesize=sizeof(double);
      }
      else if(strcmp(argv[i],"-float")==0){
         datatypesize=sizeof(float);
      }
      else if(strcmp(argv[i],"-xyzpph")==0){
         stride=6;
         datatypesize=sizeof(float);
      }
      else if(strcmp(argv[i],"-poffset")==0){
         poffset=atoi(argv[++i]);
      }
      else if(strcmp(argv[i],"-y")==0){
         updir=1;
      }
      else if(strcmp(argv[i],"-xyzp")==0){
         force_xyzp=1;
      }
      else if(strcmp(argv[i],"-stride")==0){
         stride=atoi(argv[++i]);
      }
      else if(strcmp(argv[i],"-max_points")==0){
         max_points=atoi(argv[++i]);
      }
      else{
         //		printf("Unknown parameter %d = %s",i,argv[i]);
         break;printf("Unknown parameter %d = %s",i,argv[i]);
      }
   }

   // ============================================================
   // Read XYZP Data

   dvDataSequence datacollection;
   dvDataPointsViewProperties datapointsprop;
   if(argc-1==i)  // Only one data file
   {
      printf("1\n");
      xyzpfilename=argv[i];
      harris_file* harrisfile=new harris_file(xyzpfilename);
      xyzp_file* datafile=harrisfile;
      harrisfile->read_header();
      if(harrisfile->isGood() && !force_xyzp)
      {
         printf("3dp file detected\n");
      }
      else
      {
         printf("xyzp file detected\n");
         delete datafile;
         datafile=new xyzp_file(xyzpfilename,datatypesize);
      }
      if(stride!=-1) datafile->setStride(stride);
      float * floatdata=datafile->read_data(max_points);
      int n_points=datafile->getNumberOfPoints();
      //printf("npoints=%d\n",n_points);
      if(max_points>0 && n_points>max_points) n_points=max_points;
      printf("npoints=%d\n",n_points);
      if(updir==-1) updir=datafile->getUpDirection();
      if(poffset==-1) poffset=datafile->getDefaultPOffset();
      if(stride==-1) stride=datafile->getStride();
      float x,y,z;
      // ============================================================
      //  Read Frame Information
      if(framesperdraw>0)  // dynamic scene
      {
         if(framesfilename==NULL)  // NO FIF FILE AVAILABLE
         {
				//dvData * localdatapoints;
            shared_ptr<dvData> localdatapoints;
            int curpoint=0;
            int nframes=n_points/DEFAULT_POINTS_PER_FRAME;
            for(i=0;i<nframes;i++)
            {
#ifdef OCTREE
               localdatapoints.reset(new dvDataTreeNode(floatdata+curpoint*stride,DEFAULT_POINTS_PER_FRAME,stride,poffset,&datapointsprop,updir));
#else
               localdatapoints.reset(new dvDataPoints(stride,poffset,&datapointsprop,updir));
               ((dvDataPoints*)localdatapoints)->add(floatdata+curpoint*stride,DEFAULT_POINTS_PER_FRAME);
#endif
               datacollection.add(localdatapoints);
               //		delete localdatapoints;
               curpoint+=DEFAULT_POINTS_PER_FRAME;
            }
         }
         else
         {	
            fif_file * fiffile=new fif_file(framesfilename);
				//dvData * localdatapoints;
            shared_ptr<dvData> localdatapoints;
            int curpoint=0;
            for(i=0;i<fiffile->getNumberOfFrames();i++)
            {
#ifdef OCTREE
               localdatapoints.reset(new dvDataTreeNode(floatdata+curpoint*stride,fiffile->getNumberOfPointsAt(i),stride,poffset,&datapointsprop,updir));
#else
               localdatapoints.reset(new dvDataPoints(stride,poffset,&datapointsprop,updir));
               ((dvDataPoints*)localdatapoints)->add(floatdata+curpoint*stride,fiffile->getNumberOfPointsAt(i));
#endif
               datacollection.add(localdatapoints);
               //			delete localdatapoints;
               curpoint+=fiffile->getNumberOfPointsAt(i);
            }
         }
         datacollection.setFPS(10);
      }
      else  // static scene
      {
#ifdef OCTREE
         shared_ptr<dvDataTreeNode> datapoints(new dvDataTreeNode(floatdata,n_points,stride,poffset,&datapointsprop,updir));
         //dvDataTreeNode* datapoints=new dvDataTreeNode(floatdata,n_points,stride,poffset,&datapointsprop,updir);
#else
         //dvDataPoints * datapoints;
         shared_ptr<dvDataPoints> datapoints(new dvDataPoints(stride,poffset,&datapointsprop,updir));
         //datapoints=new dvDataPoints(stride,poffset,&datapointsprop,updir);
         datapoints->add(floatdata,n_points);
#endif
         datacollection.add(datapoints);
         datapoints->getSuggestedCameraLookPoint(x,y,z);
         datacollection.setSuggestedCameraLookPoint(x,y,z);
         datapoints->getFirstPoint(x,y,z);
         datacollection.setSuggestedCameraLocationPoint(x,y,z);
         //	delete datapoints;
      }
		
      datafile->getViewPoint(x,y,z);
      if(!(x==y && y==z && z==0))
      {
         datacollection.setSuggestedCameraLocationPoint(x,y,z);
      }

   }
   else  // List of files
   {
      printf("2\n");
      harris_file * harrisfile;
      xyzp_file * datafile;
      float * floatdata;
      //dvData * datapoints;
      shared_ptr<dvData> datapoints;
      float x,y,z;
      for(;i<argc;i++)
      {
         harrisfile=new harris_file(argv[i]);
         datafile=harrisfile;
         harrisfile->read_header();
         if(!harrisfile->isGood() || force_xyzp)
         {
            delete datafile;
            datafile=new xyzp_file(argv[i],datatypesize);
         }
         if(updir==-1) updir=datafile->getUpDirection();
         if(poffset==-1) poffset=datafile->getDefaultPOffset();
         if(stride==-1) stride=datafile->getStride();
         floatdata=datafile->read_data();
#ifdef OCTREE
         datapoints.reset(new dvDataTreeNode(floatdata,datafile->getNumberOfPoints(),stride,poffset,&datapointsprop,updir));
         shared_polymorphic_downcast< dvDataTreeNode >(datapoints)->getFirstPoint(x,y,z);
         datacollection.setSuggestedCameraLocationPoint(x,y,z);
         datapoints->setSuggestedCameraLocationPoint(x,y,z);
#else
         datapoints.reset(new dvDataPoints(stride,poffset,&datapointsprop,updir));
         //datapoints=new dvDataPoints(datafile->getStride(),datafile->getDefaultPOffset(),updir);
         ((dvDataPoints*)datapoints)->add(floatdata,datafile->getNumberOfPoints());
#endif
			
         datafile->getViewPoint(x,y,z);
         if(!(x==y && y==z && z==0))
         {
            datapoints->setSuggestedCameraLocationPoint(x,y,z);
         }
         datacollection.add(datapoints);
         //	delete datapoints;
      }
		
//		datacollection.ResetConstraints();
//		float x,y,z;
//		datafile->getViewPoint(x,y,z);
//		datacollection.setViewPoint(x,y,z);
      datacollection.setFPS(4);
      //	datacollection.resetConstraint();
   }

   // ============================================================
   // Give Data to Viewer

   setpoints(&datacollection,&datapointsprop);
//	setUpDirection(2);
   // ============================================================
   // Start Viewer
    
   startDataViewer();
   return 0;                                     /* ANSI C requires main to return int. */
}


