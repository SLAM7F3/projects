// ==========================================================================
// caffe_classifier class member function definitions
// ==========================================================================
// Last modified on 6/16/16; 7/30/16; 7/31/16; 8/1/16
// ==========================================================================


#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <memory>

#include "classification/caffe_classifier.h"
#include "image/graphicsfuncs.h"
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "video/texture_rectangle.h"

using namespace caffe; // NOLINT(build/namespaces)

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// Pair (label, confidence) representing a prediction:

typedef std::pair<string, float> Prediction;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void caffe_classifier::allocate_member_objects()
{
}		       

void caffe_classifier::initialize_member_objects()
{
   feature_descriptor = NULL;
   label_tr_ptr = NULL;
   score_tr_ptr = NULL;
   cc_tr_ptr = NULL;

// By default, we assume caffe_classifier object is used for discrete
// classification rather than semantic segmentation:

   segmentation_flag = false; 
}		       

// ---------------------------------------------------------------------
caffe_classifier::caffe_classifier(
   const string& deploy_prototxt_filename,
   const string& trained_caffe_model_filename)
{
   allocate_member_objects();
   initialize_member_objects();

   test_prototxt_filename = deploy_prototxt_filename;
   this->trained_caffe_model_filename = trained_caffe_model_filename;
   load_trained_network();
}

// ---------------------------------------------------------------------
caffe_classifier::caffe_classifier(
   const string& deploy_prototxt_filename,
   const string& trained_caffe_model_filename,
   const string& mean_bgr_filename, 
   const string& labels_filename,
   bool imagenet_classification_flag)
{
   allocate_member_objects();
   initialize_member_objects();

   test_prototxt_filename = deploy_prototxt_filename;
   this->trained_caffe_model_filename = trained_caffe_model_filename;
   load_trained_network();

   ifstream labels(labels_filename.c_str());
   CHECK(labels) << "Unable to open labels file " << labels_filename;
   string line;
   while (std::getline(labels, line))
   {
      labels_.push_back(string(line));
      cout << "label = " << labels_.back() << endl;
   }

   if(imagenet_classification_flag) 
   {

// Load the binaryproto mean file:

      SetMean(mean_bgr_filename);

// Load labels:

      Blob<float>* output_layer = net_->output_blobs()[0];
      cout << "labels.size() = " << labels_.size() << endl;
      cout << "output_layer->channels() = " << output_layer->channels()
           << endl;

      CHECK_EQ(labels_.size(), output_layer->channels())
         << "Number of labels is different from the output layer dimension.";
   }
   else
   {
      filefunc::ReadInfile(mean_bgr_filename);
      vector<double> color_values = 
         stringfunc::string_to_numbers(filefunc::text_line[0]);
      mean_bgr.first = color_values[0];	   // Blue
      mean_bgr.second = color_values[1];   // Green
      mean_bgr.third = color_values[2];	   // Red

      cout << "mean_bgr.first = " << mean_bgr.first << endl;
      cout << "mean_bgr.second = " << mean_bgr.second << endl;
      cout << "mean_bgr.third = " << mean_bgr.third << endl;
   }
}

// ---------------------------------------------------------------------
caffe_classifier::~caffe_classifier()
{
   delete [] feature_descriptor;
   delete label_tr_ptr;
   delete score_tr_ptr;
   delete cc_tr_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

/*
  ostream& operator<< (ostream& outstream,const caffe_classifier& training)
  {
  outstream << endl;
  outstream << "Example " << training.get_ID() << endl;

  for(unsigned int f = 0; f < training.feature_values.size(); f++)
  {
  outstream << "  Feature " << f << " : " 
  << training.feature_labels[f] << " = "
  << training.feature_values[f] << endl;
  }
   
  outstream << "  Classification value = " 
  << training.classification_value << endl;
  return outstream;
  }
*/

// ==========================================================================

static bool PairCompare(const pair<float, int>& lhs,
                        const pair<float, int>& rhs) 
{
   return lhs.first > rhs.first;
}

// ---------------------------------------------------------------------
// Method Argmax returns the indices of the top N values of vector v. 

static vector<int> Argmax(const vector<float>& v, int N) 
{
   vector<pair<float, int> > pairs;
   for (size_t i = 0; i < v.size(); ++i)
      pairs.push_back(std::make_pair(v[i], i));
   std::partial_sort(pairs.begin(), pairs.begin() + N, pairs.end(), 
                     PairCompare);

   vector<int> result;
   for (int i = 0; i < N; ++i)
      result.push_back(pairs[i].second);
   return result;
}

// ---------------------------------------------------------------------
// Member function Classify returns the top N predictions

vector<Prediction> caffe_classifier::Classify(const cv::Mat& img, int N) 
{
   cout << "inside caffe_classifier::Classify()" << endl;
   vector<float> output = Predict(img);
   
   N = std::min<int>(labels_.size(), N);
   vector<int> maxN = Argmax(output, N);
   vector<Prediction> predictions;
   for (int i = 0; i < N; ++i) {
      int idx = maxN[i];
      predictions.push_back(std::make_pair(labels_[idx], output[idx]));
   }

   return predictions;
}

// ---------------------------------------------------------------------
// Member function load_trained_network() imports a test.prototxt file
// as well as a trained caffe model.  

void caffe_classifier::load_trained_network()
{
   cout << "Loading trained network:" << endl;

#ifdef CPU_ONLY
   Caffe::set_mode(Caffe::CPU);
#else
   Caffe::set_mode(Caffe::GPU);
#endif

// Load the network and print out its metadata:

   net_.reset(new Net<float>(test_prototxt_filename, caffe::TEST));
   net_->CopyTrainedLayersFrom(trained_caffe_model_filename);
   print_network_metadata();   

   Blob<float>* input_blob = net_->input_blobs()[0];
   num_data_channels_ = input_blob->channels();
   CHECK(num_data_channels_ == 3 || num_data_channels_ == 1)
      << "Input layer should have 1 or 3 color channels.";

   int datawidth_ = input_blob->width();
   int dataheight_ = input_blob->height();
   input_geometry_ = cv::Size(datawidth_, dataheight_);
}

// ---------------------------------------------------------------------
// Member function print_network_metadata() prints out metadata for the
// trained network's layers, blobs, etc.

void caffe_classifier::print_network_metadata()
{
   cout << "..........................................." << endl;
   cout << "Network metadata" << endl;
   
   cout << "Trained network name = " << net_->name() << endl;
   cout << "Trained caffe model filename = "
        << trained_caffe_model_filename << endl;
   cout << "Test prototxt filename = " << test_prototxt_filename << endl;

   int n_layers = net_->layer_names().size();
   cout << "n_layers = " << n_layers << endl;
   int n_blobs = net_->blob_names().size();
   cout << "n_blobs = " << n_blobs << endl;

   int n_input_blobs = net_->input_blobs().size();
   cout << "n_input_blobs = " << n_input_blobs << endl;
   CHECK_EQ(net_->num_inputs(), 1) 
      << "Network should have exactly one input blob.";

   int n_output_blobs = net_->output_blobs().size();
   cout << "n_output_blobs = " << n_output_blobs << endl;
   CHECK_EQ(net_->num_outputs(), 1) 
      << "Network should have exactly one output blob.";

   // On 8/1/16, we empirically confirmed that caffe::TRAIN = 0 and
   // caffe::TEST = 1.

   int network_phase = net_->phase();
   if(network_phase == caffe::TRAIN)
   {
      cout << "network_phase = caffe::TRAIN" << endl;
   }
   else if (network_phase == caffe::TEST)
   {
      cout << "network_phase = caffe::TEST" << endl;
   }
   
   for(int l = 0; l < n_layers; l++)
   {
      cout << "Layer l = " << l 
           << " layer_name = " << net_->layer_names().at(l)
           << endl;
   }
   cout << endl;

   for(int b = 0; b < n_blobs; b++)
   {
      cout << "Blob b = " << b
           << " blob_name = " << net_->blob_names().at(b)
           << " : " << flush;
      int num_axes = net_->blobs().at(b)->num_axes();

      for(int a = 0; a < num_axes; a++)
      {
         int curr_shape = net_->blobs().at(b)->shape(a);
         cout << curr_shape << " " << flush;
         if(a < num_axes - 1) cout << "x " << flush;
      }
      cout << endl;
   }

   cout << "..........................................." << endl;
}

// ---------------------------------------------------------------------
// Member function SetMean() loads the mean file in binaryproto format

void caffe_classifier::SetMean(const string& mean_file) 
{
   BlobProto blob_proto;
   ReadProtoFromBinaryFileOrDie(mean_file.c_str(), &blob_proto);

   // Convert from BlobProto to Blob<float> 
   Blob<float> mean_blob;
   mean_blob.FromProto(blob_proto);
   CHECK_EQ(mean_blob.channels(), num_data_channels_)
      << "Number of channels of mean file doesn't match input layer.";

   // The format of the mean file is planar 32-bit float BGR or grayscale. 
   vector<cv::Mat> channels;
   float* data = mean_blob.mutable_cpu_data();
   for (int i = 0; i < num_data_channels_; ++i) {
      // Extract an individual channel. 
      cv::Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
      channels.push_back(channel);
      data += mean_blob.height() * mean_blob.width();
   }

   // Merge the separate channels into a single image. 
   cv::Mat mean;
   cv::merge(channels, mean);

   // Compute the global mean pixel value and create a mean image
   // filled with this value. 
   cv::Scalar channel_mean = cv::mean(mean);
   mean_ = cv::Mat(input_geometry_, mean.type(), channel_mean);
}

// ---------------------------------------------------------------------

vector<float> caffe_classifier::Predict(const cv::Mat& img) 
{
   cout << "inside caffe_classifier::Predict()" << endl;
   Blob<float>* input_blob = net_->input_blobs()[0];
   input_blob->Reshape(1, num_data_channels_,
                        input_geometry_.height, input_geometry_.width);

   // Forward dimension change to all layers
   net_->Reshape();

   vector<cv::Mat> input_channels;
   WrapInputLayer(&input_channels);

   Preprocess(img, &input_channels);

   net_->ForwardPrefilled();

   // Copy the output layer to a vector 
   Blob<float>* output_layer = net_->output_blobs()[0];
   const float* begin = output_layer->cpu_data();
   const float* end = begin + output_layer->channels();
   return vector<float>(begin, end);
}

// ---------------------------------------------------------------------
// Member function WrapInputLayer() wraps the input layer of the
// network in separate cv::Mat objects (one per channel). This way we
// save one memcpy operation and we don't need to rely on
// cudaMemcpy2D. The last preprocessing operation will write the
// separate channels directly to the input layer.

void caffe_classifier::WrapInputLayer(vector<cv::Mat>* input_channels) 
{
   Blob<float>* input_blob = net_->input_blobs()[0];

   int width = input_blob->width();
   int height = input_blob->height();
   float* input_data = input_blob->mutable_cpu_data();
   for (int i = 0; i < input_blob->channels(); ++i) 
   {
      cv::Mat channel(height, width, CV_32FC1, input_data);
      input_channels->push_back(channel);
      input_data += width * height;
   }
}

// ---------------------------------------------------------------------

void caffe_classifier::Preprocess(const cv::Mat& img,
                                  vector<cv::Mat>* input_channels) 
{
   cout << "inside caffe_classifier::Preprocess()" << endl;
   cout << "img.channels() = " << img.channels() << endl;
   cout << "num_data_channels_ = " << num_data_channels_ << endl;
   
   /* Convert the input image to the input image format of the network. */
   cv::Mat sample;
   if (img.channels() == 3 && num_data_channels_ == 1)
      cv::cvtColor(img, sample, CV_BGR2GRAY);
   else if (img.channels() == 4 && num_data_channels_ == 1)
      cv::cvtColor(img, sample, CV_BGRA2GRAY);
   else if (img.channels() == 4 && num_data_channels_ == 3)
      cv::cvtColor(img, sample, CV_BGRA2BGR);
   else if (img.channels() == 1 && num_data_channels_ == 3)
      cv::cvtColor(img, sample, CV_GRAY2BGR);
   else
      sample = img;


   cv::Mat sample_resized;

   cout << "sample.size() = " << sample.size() << endl;
   
   if (sample.size() != input_geometry_)
      cv::resize(sample, sample_resized, input_geometry_);
   else
      sample_resized = sample;


   cv::Mat sample_float;
   if (num_data_channels_ == 3)
      sample_resized.convertTo(sample_float, CV_32FC3);
   else
      sample_resized.convertTo(sample_float, CV_32FC1);

   cv::Mat sample_normalized;
   cout << "before cv::subtract" << endl;
   cv::subtract(sample_float, mean_, sample_normalized);

   // This operation will write the separate BGR planes directly to the
   // input layer of the network because it is wrapped by the cv::Mat
   // objects in input_channels. 
   cv::split(sample_normalized, *input_channels);

   CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
         == net_->input_blobs()[0]->cpu_data())
      << "Input channels are not wrapping the input layer of the network.";

   cout << "At end of caffe_classifier::Preprocess()" << endl;
}

// ---------------------------------------------------------------------
// Member function rgb_img_to_bgr_fvec() subtracts off mean BGR values
// from the input RGB image.  It then converts the RGB values within
// the renormalized two-dimensional RGB image into a one-dimensional
// feature vector.  The 1D vector looks like

//             B1 B2 B3 .... G1 G2 G3 .... R1 R2 R3...

void caffe_classifier::rgb_img_to_bgr_fvec(texture_rectangle& curr_img)
{
//  cout << "inside caffe_classifier::rgb_img_to_bgr_fvec()" << endl;
   
   input_img_xdim = curr_img.getWidth();
   input_img_ydim = curr_img.getHeight();
   int n_dims = num_data_channels_ * input_img_ydim * input_img_xdim;
   feature_descriptor = new float[n_dims];

   int R, G, B;
   int index = 0;
   for(int c = 0; c < num_data_channels_; c++){
      for(int py = 0; py < input_img_ydim; py++){

         for(int px = 0; px < input_img_xdim; px++){

            float curr_value;
            if(c == 0)   // blue channel first
            {
               if(segmentation_flag)
               {
                  B = curr_img.fast_get_pixel_B_value(
                     px, input_img_ydim - 1 - py);
               }
               else
               {
                  B = curr_img.fast_get_pixel_B_value(px, py);
               }
               curr_value = B - mean_bgr.first;
            }
            else if (c == 1) // green channel second
            {
               if(segmentation_flag)
               {
                  G = curr_img.fast_get_pixel_R_value(
                     px, input_img_ydim - 1 - py);
               }
               else
               {
                  G = curr_img.fast_get_pixel_G_value(px, py);
               }
               curr_value = G - mean_bgr.second;
            }
            else  // red channel third
            {
               if(segmentation_flag)
               {
                  R = curr_img.fast_get_pixel_R_value(
                     px, input_img_ydim - 1 - py);
               }
               else
               {
                  R = curr_img.fast_get_pixel_R_value(px, py);
               }
               curr_value = R - mean_bgr.third;
            }
            
            feature_descriptor[index] = curr_value;
            index++;
         } // loop over index px
      } // loop over index py
   } // loop over index c labeling color channels
}

// ---------------------------------------------------------------------
// Member function generate_dense_map() checks whether the "data" for
// the loaded trained network corresponds to a Caffe layer or a Caffe
// blob.

void caffe_classifier::generate_dense_map()
{
   if(net_->has_layer("data"))
   {
      cout << "Testing network has a 'data' layer" << endl;
   }
   else if(net_->has_blob("data"))
   {
//      cout << "Testing network has a 'data' blob" << endl;
      generate_dense_map_data_blob();
   }
   else 
   {
      cout << "Network needs either an input layer or input blob with name data" << endl;
   }
}


//**** dl_dnn_classify_prob ***********************************************
// Description : Classify a feature vector. Return the probability vector of all
//               classes. Assume that there is a softmax layer called "prob" in
//               the model definition.
//
// Input  : dl_dnn  the deep neural net
//          feature_vec  the feature vector to be classified
//          cls  output class
//         score  array where the i:th element contains the score for the i:th
//                 class
//          num_classes  number of classes used in the training
//          job_tag  tag for logging job performance, NULL for not logging
//
// Output : 0 if OK, non-zero otherwise


/*
int dl_dnn_classify_prob(dl_dnn_t *dl_dnn, float *feature_vec, int *cls,
    float *score, int num_classes, char *job_tag)
{
  caffe::Net<float> *net = static_cast<caffe::Net<float> *>(dl_dnn->net);
  int num_classes_net; // Number of classes used in the net during training
  const float *argmaxs;
  int ret = 0;

  // Get the input data layer
  const shared_ptr<caffe::MemoryDataLayer<float> > memory_data_layer =
      static_pointer_cast<caffe::MemoryDataLayer<float> >(
          net->layer_by_name("data"));
  // Add data to the input layer
  memory_data_layer->AddRawDataAndLabels(feature_vec, NULL, 1);
  // Perform the forward pass
  int64_t t = tic();
  const vector<caffe::Blob<float> *> &result = net->ForwardPrefilled();
  if(job_tag){
    char tag[MAX_PATH];
    sprintf(tag, "%s_dl_dnn_classify_prob", job_tag);
    ps_job_stat(tag, (u_timel() - t) / 1000000.0,
        (ps_flag_t)(PS_MEAN | PS_SUM | PS_COUNT | PS_MIN | PS_MAX));
  }
  argmaxs = result[1]->cpu_data();
  // Get the class
  *cls = argmaxs[0];
  // Get the softmax probability array
  const shared_ptr<const caffe::Blob<float> > probs =
      net->blob_by_name("prob");
  // Check if the number of classes is the same as used in training
  num_classes_net = probs->count() / probs->shape(0);
  if(num_classes_net != num_classes){
    log_error("%s Something is wrong. num_classes %i != nr_results %i",
        __FUNCTION__, num_classes, num_classes_net);
    ret = 1;
  }
  else{
    // Get the probabilities
    const float *probs_out = probs->cpu_data();
    memcpy(score, probs_out, num_classes * sizeof(float));
  }
  return ret;
}
*/


// ---------------------------------------------------------------------
// Member function generate_dense_map_data_blob() reshapes a new data
// blob based upon input image's number of channels and pixel
// dimensions.  It then copies vectorized data from the input image
// into the new data blob.  The data blob is then passed into the
// trained network.  

void caffe_classifier::generate_dense_map_data_blob()
{
//   cout << "inside caffe_classifier::generate_dense_map_data_blob() " << endl;

/*
   // Get the input data layer
   const shared_ptr<caffe::MemoryDataLayer<float> > memory_data_layer =
      std::static_pointer_cast<caffe::MemoryDataLayer<float> >(
         net_->layer_by_name("data"));

   // Add data to the input layer
   memory_data_layer->AddRawDataAndLabels(feature_descriptor, NULL, 1);
   // Perform the forward pass

   const vector<caffe::Blob<float> *>& result_blobs = net->ForwardPrefilled();
*/

// Create the data blob:

   caffe::Blob<float> data_blob;
   vector<int> shape;
   shape.push_back(1);
   shape.push_back(num_data_channels_);
   shape.push_back(input_img_ydim);
   shape.push_back(input_img_xdim);
   data_blob.Reshape(shape);
   float *data = data_blob.mutable_cpu_data();

// Copy new test image data into data blob:

   memcpy(data, feature_descriptor, 
          shape[0] * shape[1] * shape[2] * shape[3] * sizeof(float));

/*
   for(int c = 0; c < 3; c++)
   {
      for(int py = 47; py <= 49; py++)
      {
         for(int px = 47; px <= 49; px++)
         {
            int index = c * input_img_ydim * input_img_xdim + 
               py * input_img_xdim + px;
            cout << "c = " << c << " py = " << py << " px = " << px
                 << " index = " << index
                 << " data[index] = " << data[index] << endl;
         }
      }
   }
   outputfunc::enter_continue_char();
*/


// Perform the forward pass on the data blob:

//    cout << "Performing forward inference pass" << endl;
   vector<caffe::Blob<float>*> input_blobs;
   input_blobs.push_back(&data_blob);

   const vector<caffe::Blob<float> *>& result_blobs = 
      net_->Forward(input_blobs);

   const caffe::Blob<float>* result_blob = result_blobs[0];

   if(segmentation_flag)
   {
      export_segmentation_mask(result_blob);
   }
   else
   {
      retrieve_classification_results(result_blob);
   }


   delete [] feature_descriptor;
   feature_descriptor=NULL;
}

// ---------------------------------------------------------------------
// Member function retrieve_classification_results() extracts
// n_classes softmax probability values from the "prob" blob.  The
// maximal softmax probability is returned as the classification
// score.  The label corresponding to the class with the highest
// classification probability is returned as the classification
// result.

void caffe_classifier::retrieve_classification_results(
   const caffe::Blob<float>* result_blob)
{
//   cout << "inside caffe_classifier::retrieve_classification_results()"
//        << endl;

// Softmax "prob" blob should have shape = 1 x n_classes :

   const shared_ptr<const caffe::Blob<float> > probs_blob =
      net_->blob_by_name("prob");

   int n_classes = probs_blob->shape(1);
   const float *probs_out = probs_blob->cpu_data();

   vector<float> class_probs;
   for(int c = 0; c < n_classes; c++)
   {
      class_probs.push_back(probs_out[c]);
//      cout << "c = " << c << " class_probs = " << class_probs.back()
//           << endl;
   }

   const float *class_argmaxes = result_blob->cpu_data();
   classification_result = class_argmaxes[0];
   classification_score = class_probs[classification_result];

//   cout << "classification_result = " << classification_result
//        << " classification_score = " << classification_score
//        << endl << endl;
}

// ---------------------------------------------------------------------
// Member function export_segmentation_mask()

void caffe_classifier::export_segmentation_mask(
   const caffe::Blob<float>* result_blob)
{
//   cout << "inside caffe_classifier::export_segmentation_mask()"
//        << endl;

   const float *argmaxs = result_blob->cpu_data();
//   cout << "argmaxs = " << argmaxs << endl;

   int height = result_blob->shape(2);
   int width = result_blob->shape(3);

   if(height != input_img_ydim || width != input_img_xdim)
   {
      cout << "Warning: Input and output image pixel dimensions are not equal" 
           << endl;
      cout << "   Output width = " << width 
           << " input_img_xdim = " << input_img_xdim << endl;
      cout << "   Output height = " << height 
           << " input_img_ydim = " << input_img_ydim << endl;
//       exit(-1);
   }

   label_tr_ptr=new texture_rectangle(width, height, 1, 1, NULL);
   score_tr_ptr=new texture_rectangle(width, height, 1, 1, NULL);

   label_tr_ptr->instantiate_ptwoDarray_ptr();
   score_tr_ptr->instantiate_ptwoDarray_ptr();

   // Fill label map:

   twoDarray* ptwoDarray_ptr = label_tr_ptr->get_ptwoDarray_ptr();
   for(int py = 0; py < height; py++)
   {
      for(int px = 0; px < width; px++)
      {
         float curr_label = argmaxs[py * width + px];
         ptwoDarray_ptr->put(px, height - 1 - py, curr_label);
      } // loop over px
   } // loop over py

//   string mask_filename="mask.jpg";
//   label_tr_ptr->initialize_twoDarray_image(ptwoDarray_ptr, 3, false);
//   label_tr_ptr->write_curr_frame(mask_filename);

//  Fill score map:
   ptwoDarray_ptr = score_tr_ptr->get_ptwoDarray_ptr();
   for(int py = 0; py < height; py++)
   {
      for(int px = 0; px < width; px++)
      {
         double curr_score = argmaxs[height * width + py * width + px];
         ptwoDarray_ptr->put(px, height - 1 - py, curr_score);
      } // loop over px
   } // loop over py

}

// ---------------------------------------------------------------------
// Member function display_segmentation_labels() loops over all
// pixels within *label_tr_ptr.  It resets the RGB values for any
// pixel labeled as background to greyscale.  

string caffe_classifier::display_segmentation_labels(
   texture_rectangle& curr_img, string output_subdir)
{
   const double SMALL = 0.001;

   double h, s, v;
   twoDarray* ptwoDarray_ptr = label_tr_ptr->get_ptwoDarray_ptr();
   for(unsigned int py = 0; py < ptwoDarray_ptr->get_ydim(); py++)
   {
      for(unsigned int px = 0; px < ptwoDarray_ptr->get_xdim(); px++)
      {
         curr_img.get_pixel_hsv_values(px, py, h, s, v);
         float curr_label = ptwoDarray_ptr->get(px, py);

         if(curr_label < SMALL)
         {
            s = 0;
            v *= 0.85;
         }
         else
         {
            double smin = 0.66;
            double smax = 1.0;
            double vmin = 0.5;
            double vmax = 1.0;

            h = 77 * (curr_label - 1);
//            h = 220 * curr_label; // for vertical segmentation charts only

            s = smin + s * (smax - smin);
            v = vmin + v * (vmax - vmin);
         }
         curr_img.set_pixel_hsv_values(px, py, h, s, v);
      } // loop over px
   } // loop over py

   string segmented_filename=curr_img.get_video_filename();
   string basename = filefunc::getbasename(segmented_filename);
   string basename_prefix=stringfunc::prefix(basename);
   segmented_filename=output_subdir+basename_prefix+"_segmented.png";

   curr_img.write_curr_frame(segmented_filename);
   cout << "Exported " << segmented_filename << endl;
   return segmented_filename;
}

// ---------------------------------------------------------------------
// Member function display_segmentation_scores() imports curr_img
// which we assume has previously been colored via a call to
// display_segmentation_labels().  It loops over all pixels within
// *score_tr_ptr.  The labeled pixels are recolored on a hot/red -
// cold/purple hue scale to indicate segmentation confidences.
// Coloring for background-class pixels remain as greyscale.

string caffe_classifier::display_segmentation_scores(
   texture_rectangle& curr_img, string output_subdir)
{
//   cout << "inside caffe_classifier::display_segmentation_scores()" << endl;
   twoDarray* ptwoDarray_ptr = score_tr_ptr->get_ptwoDarray_ptr();

   double minvalue, maxvalue;
   ptwoDarray_ptr->minmax_values(minvalue, maxvalue);
//   cout << "       minvalue = " << minvalue << " maxvalue = " << maxvalue 
//        << endl;
   double min_score = 0.3;	 // Empirically determined on 6/16/16
   double max_score = 1.0;

   for(unsigned int py = 0; py < ptwoDarray_ptr->get_ydim(); py++)
   {
      for(unsigned int px = 0; px < ptwoDarray_ptr->get_xdim(); px++)
      {
         double h,s,v;
         curr_img.get_pixel_hsv_values(px, py, h, s, v);
         float curr_score = ptwoDarray_ptr->get(px, py);

         double renorm_score = 
            (curr_score - min_score) / (max_score - min_score);
         h = 300 * (1 - renorm_score);

         curr_img.set_pixel_hsv_values(px, py, h, s, v);
      } // loop over px
   } // loop over py

   string score_filename=curr_img.get_video_filename();
   string basename = filefunc::getbasename(score_filename);
   string basename_prefix=stringfunc::prefix(basename);
   score_filename=output_subdir+basename_prefix+"_score.png";

   curr_img.write_curr_frame(score_filename);
   cout << "Exported " << score_filename << endl;
   return score_filename;
}

// ---------------------------------------------------------------------
// Member function cleanup_memory()

void caffe_classifier::cleanup_memory()
{
   delete label_tr_ptr;
   label_tr_ptr = NULL;

   delete score_tr_ptr;
   score_tr_ptr = NULL;

   delete cc_tr_ptr;
   cc_tr_ptr = NULL;
}
