// ========================================================================
// Program TRACKFEATURES implements the KLT tracking algorithm for
// video features.
// ========================================================================
// Last updated on 11/10/05; 12/30/05; 6/18/06
// ========================================================================

//   written by: Hyrum Anderson
//   (c) 2005 MIT Lincoln Laboratory

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/prob_distribution.h"
#include "KLT/klt.h"
#include "video/VidFile.h"

using std::cout;
using std::cerr;
using std::endl;
using std::ofstream;
using std::setw;
using std::string;
using std::vector;

const int NEWFEATUREVAL = 1; // must be a positive
int nFeatures;
int nFrames;
int nTotalFrames;
int nCols;
int nRows;
int nChannels;
int passNumber;

struct CommandLineOptions
{
      string featureInFile;
      string featureOutFile;
      int numFeatures;
      int imageInterval;
      int startFrame;
      int endFrame;
      string videoFilename;
};

// ==============================================
void read_image_RGB2Gray( 
   VidFile & video, const int frame, unsigned char *image)
{
   unsigned char * RGBimage = 
      new unsigned char[ video.image_size_in_bytes() ];
   const int Nentries = video.getWidth() * video.getHeight();
   // these values suggeted at http://www.devx.com/vb2themax/Tip/19661
   static double F_RED = 0.3;
   static double F_GREEN = 0.59;
   static double F_BLUE = 0.11;

   video.read_image( frame, RGBimage );

   for (int n=0; n<Nentries; n++)
   {
      image[n] = static_cast<char>( 
         RGBimage[ 3*n ] * F_RED +
         RGBimage[ 3*n + 1 ] * F_GREEN +
         RGBimage[ 3*n + 2 ] * F_BLUE
         );
   }

   delete[] RGBimage;
}

// ==============================================
void readFeatureFile( KLT_FeatureTable ft, CommandLineOptions * op )
{
   const int normalFactor = nRows;
    
   cout << "Reading feature file " << op->featureInFile << endl;
   std::ifstream feature_file( op->featureInFile.c_str(), std::ios::in );
   std::map< int, bool > feature_map;

   int startFrame = ( op->startFrame < op->endFrame ? op->startFrame : 
                      op->endFrame );
   int endFrame = ( op->startFrame < op->endFrame ? op->endFrame : 
                    op->startFrame );

   int parse_count=0;
   int add_count=0;
   while (feature_file && !feature_file.eof() )
   {
      double imagenumber;
      int featurenumber;
      int passnumber;
      double x;
      double y;
      feature_file >> imagenumber;
      feature_file >> featurenumber;
      feature_file >> passnumber;
      feature_file >> x;
      feature_file >> y;
      if (!feature_file.eof()  )
      {
         parse_count++;

         if ( imagenumber >= startFrame && imagenumber <= endFrame && 
              add_count < op->numFeatures && featurenumber < op->numFeatures )
         {
            add_count++;
            KLT_Feature feat = ft->feature[ featurenumber ][ 
               static_cast<int>(imagenumber) - startFrame ];
            feat->x = x * normalFactor;
            feat->y = nRows - y * normalFactor;

            if ( feature_map[ featurenumber ] )
            {
               feat->val = 0;   // already exists (being tracked)
            }
            else
            {
               // new feature
               feat->val = NEWFEATUREVAL;
               feature_map[ featurenumber ] = true;
            }
         }
      }
   }

   feature_file.close();
   cout << "Parsed " << parse_count << ", added " << add_count 
        << " feature entries" << endl;
};

// ==============================================
void writeFeatureFile( KLT_FeatureTable ft,
                       CommandLineOptions *op )
{

// Initialize feature_map look-up table:

   const int normalFactor = nRows;
   std::map< int, int > feature_map;
   for (int feat=0; feat < op->numFeatures; feat++)
   {
      feature_map[ feat ] = feat;
   }
   
   vector<float> feature_values;
   feature_values.reserve(10*ft->nFeatures);

   cout << "Writing features to " << op->featureOutFile << endl;
   int last_feature_ID = ft->nFeatures - 1; // last feature_ID

   std::ofstream feature_file( op->featureOutFile.c_str(), std::ios::out );

   int step = ( op->startFrame < op->endFrame ? 1 : -1 );
   int base = ( op->startFrame < op->endFrame ? op->startFrame : 
                op->endFrame );
 
   for (int frame = op->startFrame; 
        (step==-1 && frame >= op->endFrame) || 
           (step==1 && frame <= op->endFrame); frame+=step)
   {
      for (int feat=0; feat < op->numFeatures; feat++)
      {
         KLT_Feature f = ft->feature[feat][frame - base];

         if ( f->val > 0 )  feature_values.push_back(f->val);
         if ( f->val > 0 && frame != op->startFrame ) // new feature
         {
            last_feature_ID++;
            feature_map[feat] = last_feature_ID;
         }

         const int narrow_column_width=5;
         const int wide_column_width=15;
//         float curr_u=f->x;
//         float curr_v=(nRows-f->y);
         float curr_u=f->x/normalFactor;
         float curr_v=(nRows-f->y)/normalFactor;
         feature_file << setw(narrow_column_width)
                      << frame << "\t" // time (image number)
                      << setw(narrow_column_width)
                      << feature_map[ feat ] // ID number
                      << setw(narrow_column_width)
                      << passNumber  	// pass number, video=1
                      << setw(wide_column_width)
                      << curr_u 
                      << setw(wide_column_width)
                      << curr_v 
//                          << f->x / normalFactor << "\t" // y-location (v)
//                          << (nRows - f->y) / normalFactor << "\t" // x-location (u)
                      << setw(wide_column_width)
                      << f->val
                      << endl;
      }
   }
   feature_file.close();

// Print out all feature values:

   string values_filename="KLT_values.txt";
   ofstream valuestream;
   filefunc::openfile(values_filename,valuestream);
   cout << "feature_values.size() = " << feature_values.size() << endl;
   for (unsigned int f=0; f<feature_values.size(); f++)
   {
      valuestream << feature_values[f] << endl;
//      cout << "Feature " << f << " value = " << feature_values[f]
//           << endl;
   }
   filefunc::closefile(values_filename,valuestream);

//   prob_distribution prob(feature_values,500);
//   prob_distribution prob(feature_values,250);
   prob_distribution prob(feature_values,100);
   prob.set_freq_histogram(true);
   prob.set_densityfilenamestr("values.meta");
   prob.set_xlabel("Intensity Eigenvalue");
   prob.write_density_dist();
}

// ==============================================

void getUsage(char *argv[])
{
   cerr << "usage: " << argv[0] << " [options] vidfile" << endl;
   cerr << "\t --writefeatures=filename \t output file for writing tracked features" << endl;
   cerr << "\t --numfeatures=<#> \t number of features to track" << endl;
   cerr << "\t --writeimage=<#> \t write an image every <#> frames.  Off by default" << endl;
   cerr << "\t --readfeatures=filename \t track the features specified in the filename" << endl;
   cerr << "\t --startframe=<#> \t frame number to begin tracking" << endl;
   cerr << "\t --endframe=<#> \t frame number to stop tracking" << endl;
   exit(1);
}

// ==============================================
void parseArguments(CommandLineOptions * op, int argc, char * argv[])
{
   // set defaults
   op->featureInFile="";
   op->featureOutFile="";
   op->numFeatures=150;
   op->imageInterval=0; // don't write out an image
   op->startFrame=0; // start at the beginning
   op->endFrame=-1; // (this will be changed later to maxFrames)
   op->videoFilename="";

   bool fileset = false;

   for (int argNum=1; argNum < argc; argNum++ )
   {
      std::string thisArg = argv[ argNum ];
      if ( thisArg.substr(0,2) == "--" )
      {
         // get option
         unsigned int pos=thisArg.find("=",0);
         if (pos == std::string::npos)
         {
            cerr << "Improperly formatted option: " << thisArg << endl;
            getUsage(argv);
         }

         std::string option = thisArg.substr(2, pos-2);
         std::string value = thisArg.substr(pos+1, thisArg.length() - pos );

         if (option=="writefeatures")
            op->featureOutFile = value;

         else
            if (option=="numfeatures")
               op->numFeatures = atoi(value.c_str());

            else
               if (option=="writeimage")
                  op->imageInterval = atoi(value.c_str());

               else
                  if (option=="readfeatures")
                     op->featureInFile = value;

                  else
                     if (option=="startframe")
                        op->startFrame = atoi(value.c_str() );

                     else
                        if (option=="endframe")
                           op->endFrame = atoi( value.c_str() );

                        else
                        {
                           cerr << "Invalid option: " << option << endl;
                           getUsage( argv );
                        }
            
      }
      else
      {
         // get filename
         if (fileset)
         {
            cerr << "Invalid option: " << thisArg << endl;
            getUsage(argv);
         }
         // else
         op->videoFilename = argv[ argNum ];
         fileset = true;
      }
   }
   if (!fileset)
   {
      cerr << "No video file specified!" << endl;
      getUsage( argv );
   }
};

// ==============================================
void trackFeatures( KLT_FeatureTable ft, CommandLineOptions * op, 
                    VidFile & video, const bool replace=true )
{
   KLT_FeatureList fl = KLTCreateFeatureList( nFeatures );

   KLT_TrackingContext tc = KLTCreateTrackingContext();

// Experiment with altering KLT parameters in immediate lines below:

// Group 99 parameter settings:

   tc->mindist=15;
   tc->nSkippedPixels=1;
   tc->window_height=11;
   tc->window_width=11;
   tc->grad_sigma=1;
   tc->smooth_sigma_fact=0.1f;
   tc->sequentialMode=TRUE;
   
//   tc->smoothBeforeSelecting = TRUE;
   tc->smoothBeforeSelecting = FALSE;

   tc->writeInternalImages = FALSE;
   tc->affineConsistencyCheck = -1;  // set this to 2 to turn on affine
   // consistency check
//   tc->affineConsistencyCheck = 2;  // set this to 2 to turn on affine

   // get the first image

   cout << endl;
   cout << "============================================================"
        << endl;
   cout << "Tracking features in image " 
        << op->startFrame << " of " << nTotalFrames-1 << endl;

   unsigned char * thisimage = new unsigned char[video.image_size_in_bytes()];
   unsigned char * lastimage = 0;

   if (nChannels==1)
      video.read_image( op->startFrame, thisimage );
   else
      if (nChannels==3)
         read_image_RGB2Gray( video, op->startFrame, thisimage );

   if (op->featureInFile != "") // we've read an input feature file
   {
      KLTExtractFeatureList( fl, ft, op->startFrame );
      for (int feat=0; feat < ft->nFeatures; feat++)
         if (fl->feature[feat]->val != NEWFEATUREVAL )
            fl->feature[feat]->val = -1; // replace this feature 

      // KLTReplaceLostFeatures replaces all features 
      // whose "val" component is a negative number

      KLTReplaceLostFeatures( tc, thisimage, nCols, nRows, fl );
   }
   else
   {
      KLTSelectGoodFeatures( tc, thisimage, nCols, nRows, fl );
   }

   int step = ( op->startFrame < op->endFrame ? 1 : -1 );
   int base = ( op->startFrame < op->endFrame ? op->startFrame : 
                op->endFrame );

   KLTStoreFeatureList(fl, ft, op->startFrame - base );

   if (op->imageInterval)
   {
      char filename[80];
      sprintf( filename, "frames/frame%05u.ppm", op->startFrame );
      KLTWriteFeatureListToPPM( fl, thisimage, nCols, nRows, filename );
   }


   for (int i = op->startFrame+step; 
        (step==-1 && i >= op->endFrame) || (step==1 && i <= op->endFrame); 
        i+=step)
   {
      if ( lastimage )
         delete[] lastimage;

      lastimage = thisimage;

      cout << endl;
      cout << "============================================================"
           << endl;
      cout << "Tracking features in image " 
           << i  << " of " << nTotalFrames-1 << endl;

      // get next image
      thisimage = new unsigned char[ video.image_size_in_bytes() ];

      if (nChannels==1)
         video.read_image( i, thisimage );
      else
         if (nChannels==3)
            read_image_RGB2Gray( video, i, thisimage );

      KLTTrackFeatures( tc, lastimage, thisimage, nCols, nRows, fl ); 
      if (replace)
         KLTReplaceLostFeatures( tc, thisimage, nCols, nRows, fl );

      KLTStoreFeatureList( fl, ft, i - base );
        
      if (op->imageInterval)
      {
         if (i % op->imageInterval == 0)
         {
            char filename[80];
            sprintf( filename, "frames/frame%05u.ppm",i );
            KLTWriteFeatureListToPPM( fl, thisimage, nCols, nRows, filename );
         }
      }
   }
   delete[] thisimage; 
   delete[] lastimage; 
}

// ==============================================
void printOpts( CommandLineOptions * op)
{
   // print out options
   cout << "\n===========================================================" <<endl;
   cout << "Video filename: " << op->videoFilename << endl;
   if (op->featureInFile != "" )
      cout << "Feature input file: " << op->featureInFile << endl;
   if (op->featureOutFile != "")
      cout << "Feature output file: " << op->featureOutFile << endl;

   cout << "Features to track: " << op->numFeatures << endl;
   cout << "Starting frame: " << op->startFrame << endl;
   cout << "End frame: " << op->endFrame << endl;

   if (op->imageInterval)
      cout << "Output ppm file every " << op->imageInterval << " frames" << endl;
   cout << "===========================================================\n" <<endl;


}

// ==============================================


int main(int argc, char * argv[] )
{

   // parse arguments
   if (argc<2)
   {
      getUsage(argv);
   }

   CommandLineOptions op;
   parseArguments(&op, argc, argv);

   VidFile video( op.videoFilename );
   video.query_structure_values(); // print out video information

   if ( video.getNumChannels()!=1 && video.getNumChannels()!=3 )
   {
      cerr << "Current file has unsupported " << video.getNumChannels() 
           << " channels" << endl;
   }

   // check options against what video provides
   if ( op.startFrame < 0 )
      op.startFrame = 0;

   if ( op.endFrame >= video.getNumFrames() || op.endFrame <0 )
      op.endFrame = video.getNumFrames() - 1;

   // print out command line options
   printOpts( &op );
    
   // initialize global variables
   nFeatures = op.numFeatures;

   if ( op.startFrame < op.endFrame)
      nFrames = op.endFrame - op.startFrame + 1;
   else
      nFrames = op.startFrame - op.endFrame + 1;

   nTotalFrames = video.getNumFrames();
   nCols = video.getWidth();
   nRows = video.getHeight();
   nChannels = video.getNumChannels();
   passNumber = 0;

   // set up KLT tracking
   cout << "Creating table to store " << nFrames << " frames x " 
        << nFeatures << " features" << endl;
   KLT_FeatureTable ft;
   ft = KLTCreateFeatureTable( nFrames, nFeatures );

   // read input features file, if provided
   if (op.featureInFile != "")
      readFeatureFile( ft, &op );

   // track features
   trackFeatures( ft, &op, video );

   // write out the feature table
   if (op.featureOutFile != "" ) 
      writeFeatureFile( ft, &op ); 

   return 0;  
}
