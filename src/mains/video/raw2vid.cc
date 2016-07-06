// ========================================================================
// Program RAW2VID converts raw Group 99 video data into Group 99 .vid
// files.
// ========================================================================
// Last updated on 8/24/05
// ========================================================================

#include <iostream>
#include "video/G99_raw.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

int main(int argc, char* argv[])
{

// Open raw input video file and read its header:

   RawFile raw_video(argv[1]);
   
   raw_video.parse_input_arguments(argc,argv);
   raw_video.read_file_header();
   raw_video.compute_n_raw_images();

   int start_frame,stop_frame,nframes_to_skip;
   cout << endl;
   cout << "Enter starting frame number:" << endl;
   cin >> start_frame;
   cout << "Enter stopping frame number:" << endl;
   cin >> stop_frame;
   cout << "Enter number of raw frames to skip for each greyscale image:" 
        << endl;
   cin >> nframes_to_skip;

   raw_video.set_start_frame(start_frame);
   raw_video.set_stop_frame(stop_frame);
   raw_video.set_nframes_to_skip(nframes_to_skip);

// Initialize output .vid file:

   raw_video.compute_output_video_image_params();
   raw_video.write_output_header();
   raw_video.advance_raw_file_counter();

// Write converted video data to output .vid file:

   raw_video.write_output_video_images();
}

