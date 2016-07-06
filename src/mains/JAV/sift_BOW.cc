// ====================================================================
// Program SIFT_BOW
// ====================================================================
// Last updated on 12/12/13
// ====================================================================

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <dlib/svm_threaded.h>
#include <dlib/gui_widgets.h>
#include <dlib/array.h>
#include <dlib/array2d.h>
#include <dlib/image_keypoint.h>
#include <dlib/image_processing.h>
#include <dlib/cmd_line_parser.h>
#include <dlib/data_io.h>

#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

using namespace dlib;
using std::cin;
using std::cout;
using std::endl;
using std::exception;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;

typedef hog_image<4,4,1,8,hog_signed_gradient,hog_full_interpolation> feat_type;

void get_feats (
    const array2d<unsigned char>& img,
    const projection_hash& h,
    matrix<double,0,1>& hist
)
{
    feat_type feat;
    feat.load(img);
    for (long r = 0; r < feat.nr(); ++r)
    {
        for (long c = 0; c < feat.nc(); ++c)
        {
            hist(h(feat(r,c)))++;
        }
    }
}


int main(int argc, char** argv)
{  
    bool build_dictionary = false;
    if (!build_dictionary)
    {
        projection_hash h;
        ifstream fin("hog_bow_hash.dat", ios::binary);
        deserialize(h, fin);

        for (int i = 1; i < argc; ++i)
        {
            array2d<unsigned char> img, temp;
            load_image(img, argv[i]);

            matrix<double,0,1> hist(h.num_hash_bins());
            hist = 0;

            while (img.size() > 50*50)
            {
                get_feats(img,h,hist);
                pyramid_down<4> pyr;
                pyr(img,temp); temp.swap(img);
            }

            // print out BoW histogram
            cout << argv[i] << " " << trans(hist) << flush;
        }
    }
    else
    {
        // This section creates the dictionary based on the files given on the command line
        dlib::array<array2d<unsigned char> > images;
        for (int i = 1; i < argc; ++i)
        {
            array2d<unsigned char> img;
            load_image(img, argv[i]);
            images.push_back(img);
        }

        feat_type feat;
        random_subset_selector<feat_type::descriptor_type> samps = randomly_sample_image_features(images, pyramid_down<4>(),feat, 200000);

        projection_hash h = create_random_projection_hash(samps, 10);

        ofstream fout("hog_bow_hash.dat", ios::binary);
        serialize(h, fout);
    }
}

