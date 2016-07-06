// ==========================================================================
// Kakadu functions
// ==========================================================================
// Last updated on 10/16/07; 4/16/11
// ==========================================================================

#include <fstream>
#include <iostream>
#include <math.h>
#include "kdu_stripe_decompressor.h"
#include "kakadu/kakadufuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::ofstream;
using std::string;

namespace kakadufunc
{

// Set up messaging services                         

   class kdu_stream_message : public kdu_message {
     public: // Member classes
      kdu_stream_message(std::ostream *stream)
         { this->stream = stream; }
      void put_text(const char *string)
         { (*stream) << string; }
      void flush(bool end_of_message=false)
         { stream->flush(); }
     private: // Data
      std::ostream *stream;
   };

   static kdu_stream_message cout_message(&std::cout);
   static kdu_stream_message cerr_message(&std::cerr);
   static kdu_message_formatter pretty_cout(&cout_message);
   static kdu_message_formatter pretty_cerr(&cerr_message);

   void print_version()
      {
         kdu_message_formatter out(&cout_message);
         out.start_message();
         out << "This is Kakadu's \"kdu_expand\" application.\n";
         out << "\tCompiled against the Kakadu core system, version "
             << KDU_CORE_VERSION << "\n";
         out << "\tCurrent core system version is "
             << kdu_get_core_version() << "\n";
         out.flush(true);
         exit(0);
      }

// --------------------------------------------------------------------------
   void print_usage(char* prog, bool comprehensive)
      {
         kdu_message_formatter out(&cout_message);

         out << "Usage:\n  \"" << prog << " ...\n";
         out.set_master_indent(3);
         out << "-i <compressed file>\n";
         if (comprehensive)
            out << "\tCurrently accepts raw code-stream files and code-streams "
               "wrapped in the JP2 or JPX file formats.  We no longer identify "
               "the file type based on its suffix.  Instead, we first try "
               "to open the file as a JP2-family file.  If this fails, we open "
               "it as a raw code-stream.  The only drawback of this approach "
               "is that raw code-streams are opened and closed twice.\n";
         out << "-o <file 1>,...\n";
         if (comprehensive)
            out << "\tOne or more output files. If multiple files are provided, "
               "they must be separated by commas. Any spaces will be treated as "
               "part of the file name.  This argument is not mandatory; if "
               "no output files are given, the decompressor will run completely "
               "but produce no image output.  This can be useful for timing "
               "purposes.  Currently accepted image file formats are: "
               "TIFF, RAW (big-endian), RAWL (little-endian), BMP, PGM and "
               "PPM, as determined by the suffix.  There need not be sufficient "
               "image files to represent all image components in the "
               "code-stream.  Raw files are written with the sample bits in the "
               "least significant bit positions of an 8, 16, 24 or 32 bit word, "
               "depending on the bit-depth.  For signed data, the word is sign "
               "extended. The word organization is big-endian if the file "
               "suffix is \".raw\" and little-endian if the file suffix is "
               "\".rawl\".\n";
         out << "-jpx_layer <compositing layer index>\n";
         if (comprehensive)
            out << "\tBy default, the first compositing layer of a JPX file is "
               "used to determine the colour channels to decompress.  This "
               "argument allows you to identify a different compositing layer "
               "(note that layer indices start from 0).  Plain JP2 files are "
               "treated as having only one compositing layer.  This argument "
               "is ignored if the input is a raw code-stream.  It is also "
               "ignored if `-raw_components' is used.\n";
         out << "-raw_components [<codestream index>]\n";
         if (comprehensive)
            out << "\tBy default, when a JP2/JPX file is decompressed, only "
               "those components which contribute to colour channels in the "
               "first compositing layer (or the layer identified by "
               "`-jpx_layer') are decompressed (one for luminance, three "
               "for RGB, etc.), applying any required palette mapping along "
               "the way.  In some cases, however, it may be desirable to "
               "decompress all of the raw image components available from a "
               "code-stream.  Use this switch to accomplish this.  There will "
               "then be no palette mapping in this case.  The optional "
               "parameter may be used to specify the index of the code-stream "
               "to be decompressed.  This is relevant only for JPX files which "
               "may contain multiple code-streams.  By default, the first "
               "code-stream (index 0) is decompressed.\n";
         out << "-codestream_components -- suppress multi-component/colour xforms\n";
         if (comprehensive)
            out << "\tThis flag allows you to gain access to the decompressed "
               "codestream image components, as they appear after inverse "
               "spatial wavelet transformation, prior to the inversion of any "
               "Part 1 colour transform (RCT/ICT decorrelation transforms) or "
               "Part 2 multi-component transform.  To understand this flag, it "
               "is helpful to know that there are two types of image components: "
               "\"codestream image components\" and \"output image components\".  "
               "The output image components are the ones which the content "
               "creator intended you to reconstruct, but it is sometimes "
               "interesting to reconstruct the codestream image components.  "
               "Thus, in the normal output-oriented mode, if you ask for the "
               "second image component (e.g., using `-skip_components') of a "
               "colour image, you will receive the green channel, even if this "
               "has to be recovered by first decompressing luminance and "
               "chrominance channels.  If you ask for codestream components, "
               "however, you will receive the Cb (blue colour difference) "
               "component, assuming that the ICT (YCbCr) transform was used "
               "during compression.  It should be noted that the "
               "`-codestream_components' flag cannot be used in conjunction with "
               "JP2/JPX files unless `-raw_components' is also specified.  "
               "`-raw_components' by itself, however, recovers only the "
               "\"output image components\" defined by the codestream, as opposed "
               "to its \"codestream image components\".\n";
         out << "-no_seek\n";
         if (comprehensive)
            out << "\tBy default, the system will try to exploit pointer information "
               "in the code-stream to seek over irrelevant elements.  This flag "
               "turns this behaviour off, which may result in higher memory "
               "consumption or even failure to read non-linear file formats, "
               "but avoids the potential for wasted disk accesses.\n";
         out << "-rotate <degrees>\n";
         if (comprehensive)
            out << "\tRotate source image prior to compression. "
               "Must be multiple of 90 degrees.\n";
         out << "-rate <bits per pixel>\n";
         if (comprehensive)
            out << "\tMaximum bit-rate, expressed in terms of the ratio between the "
               "total number of compressed bits (including headers) and the "
               "product of the largest horizontal and  vertical image component "
               "dimensions. Note that we use the original dimensions of the "
               "compressed image, regardless or resolution scaling and regions "
               "of interest.  Note CAREFULLY that the file is simply truncated "
               "to the indicated limit, so that the effect of the limit will "
               "depend strongly upon the packet sequencing order used by the "
               "code-stream.  The effect of the byte limit may be modified by "
               "supplying the `-simulate_parsing' flag, described below.\n";
         out << "-simulate_parsing\n";
         if (comprehensive)
            out << "\tIf this flag is supplied, discarded resolutions, image "
               "components or quality layers (see `-reduce' and `-layers') will "
               "not be counted when applying any rate limit supplied via "
               "`-rate' and when reporting overall bit-rates.  Also, if a "
               "reduced spatial region of the image is required (see `-region'), "
               "only those bytes which are relevant to that region are counted "
               "when applying bit-rate limits and reporting overall bit-rates.  "
               "The effect is intended to be the same as if the code-stream were "
               "first parsed to remove the resolutions, components, quality "
               "layers or precincts which are not being used.  Note, however, "
               "that this facility might not currently work as expected if the "
               "image happens to be tiled.\n";
         out << "-skip_components <num initial image components to skip>\n";
         if (comprehensive)
            out << "\tSkips over one or more initial image components, reconstructing "
               "as many remaining image components as can be stored in the "
               "output image file(s) specified with \"-o\" (or all remaining "
               "components, if no \"-o\" argument is supplied).  This argument "
               "is not meaningful if the input is a JP2/JPX file, unless "
               "the `-raw_components' switch is also selected.\n";
         out << "-no_alpha -- do not decompress alpha channel\n";
         if (comprehensive)
            out << "\tBy default, an alpha channel which is described by the "
               "source file will be decompressed and written to the output "
               "file(s) if possible.  This is relevant only for output file "
               "formats which can store alpha.  The present argument may be "
               "used to suppress the decompression of alpha channels.\n";
         out << "-layers <max layers to decode>\n";
         if (comprehensive)
            out << "\tSet an upper bound on the number of quality layers to actually "
               "decode.\n";
         out << "-reduce <discard levels>\n";
         if (comprehensive)
            out << "\tSet the number of highest resolution levels to be discarded.  "
               "The image resolution is effectively divided by 2 to the power of "
               "the number of discarded levels.\n";
         out << "-region {<top>,<left>},{<height>,<width>}\n";
         if (comprehensive)
            out << "\tEstablish a region of interest within the original compressed "
               "image.  Only the region of interest will be decompressed and the "
               "output image dimensions will be modified accordingly.  The "
               "coordinates of the top-left corner of the region are given first, "
               "separated by a comma and enclosed in curly braces, after which "
               "the dimensions of the region are given in similar fashion.  The "
               "two coordinate pairs must be separated by a comma, with no "
               "intervening spaces.  All coordinates and dimensions are expressed "
               "relative to the origin and dimensions of the high resolution "
               "grid, using real numbers in the range 0 to 1.\n";
         out << "-precise -- forces the use of 32-bit representations.\n";
         if (comprehensive)
            out << "\tBy default, 16-bit data representations will be employed for "
               "sample data processing operations (colour transform and DWT) "
               "whenever the image component bit-depth is sufficiently small.\n";
         out << "-fussy\n";
         if (comprehensive)
            out << "\tEncourage fussy code-stream parsing, in which most code-stream "
               "compliance failures will terminate execution, with an appropriate "
               "error message.\n";
         out << "-resilient\n";
         if (comprehensive)
            out << "\tEncourage error resilient processing, in which an attempt is "
               "made to recover from errors in the code-stream with minimal "
               "degradation in reconstructed image quality.  The current "
               "implementation should avoid execution failure so long as only "
               "a single tile-part was used and no errors are found in the main "
               "or tile header.  The implementation recognizes tile-part headers "
               "only if the first 4 bytes of the marker segment are correct, "
               "which makes it extremely unlikely that a code-stream with only "
               "one tile-part will be mistaken for anything else.  Multiple "
               "tiles or tile-parts can create numerous problems for an error "
               "resilient decompressor; complete failure may occur if a "
               "multi-tile-part code-stream is corrupted.\n";
         out << "-resilient_sop\n";
         if (comprehensive)
            out << "\tSame as \"-resilient\" except that the error resilient code-"
               "stream parsing algorithm is informed that it can expect SOP "
               "markers to appear in front of every single packet, whenever "
               "the relevant flag in the Scod style byte of the COD marker is "
               "set.  The JPEG2000 standard interprets this flag as meaning "
               "that SOP markers may appear; however, this does not give the "
               "decompressor any idea where it can expect SOP markers "
               "to appear.  In most cases, SOP markers, if used, will be placed "
               "in front of every packet and knowing this a priori can "
               "improve the performance of the error resilient parser.\n";
         out << "-num_threads <0, or number of parallel threads to use>\n";
         if (comprehensive)
            out << "\tUse this argument to gain explicit control over "
               "multi-threaded or single-threaded processing configurations.  "
               "The special value of 0 may be used to specify that you want "
               "to use the conventional single-threaded processing "
               "machinery -- i.e., you don't want to create or use a "
               "threading environment.  Otherwise, you must supply a "
               "positive integer here, and the object will attempt to create "
               "a threading environment with that number of concurrent "
               "processing threads.  The actual number of created threads "
               "may be smaller than the number requested, if your "
               "request exceeds internal resource limits.  It is worth "
               "noting that \"-num_threads 1\" and \"-num_threads 0\" "
               "both result in single-threaded processing, although the "
               "former creates an explicit threading environment and uses "
               "it to schedule the processing steps, even if there is only "
               "one actual thread of execution.\n"
               "\t   For effective use of parallel processing resources, you "
               "should consider creating at least one thread for each CPU; you "
               "should also consider using the `-double_buffering' option to "
               "minimize the amount of time threads might potentially sit idle.\n"
               "\t   If the `-num_threads' argument is not supplied explicitly, "
               "the default behaviour is to create a threading environment only "
               "if the system offers multiple CPU's (or virtual CPU's), with "
               "one thread per CPU.  However, this default behaviour depends "
               "upon knowledge of the number of CPU's which are available -- "
               "something which cannot always be accurately determined through "
               "system calls.  The default value might also not yield the "
               "best possible throughput.\n";
         out << "-double_buffering <stripe height>\n";
         if (comprehensive)
            out << "\tThis option is intended to be used in conjunction with "
               "`-num_threads'.  If `-double_buffering' is not specified, the "
               "DWT operations will all be performed by the "
               "single thread which \"owns\" the multi-threaded processing "
               "group.  For a small number of processors, this may be acceptable, "
               "since the DWT is generally quite a bit less CPU intensive than "
               "block decoding (which is always spread across multiple threads, "
               "if available).  However, even for a small number of threads, the "
               "amount of thread idle time can be reduced by specifying the "
               "`-double_buffering' option.  In this case, only a small number "
               "of image rows in each image component are actually double "
               "buffered, so that one set can be processed by colour "
               "transformation and sample writing operations, while the other "
               "set is generated by the DWT synthesis engines, which themselves "
               "feed off the block decoding engines.  The number of rows in "
               "each component which are to be double buffered is known as the "
               "\"stripe height\", supplied as a parameter to this argument.  The "
               "stripe height can be as small as 1, but this may add quite a bit "
               "of thread context switching overhead.  For this reason, a stripe "
               "height in the range 4 to 16 is recommended.  If you are working "
               "with small horizontal tiles, you may find that an even larger "
               "stripe height is required for maximum throughput.  In the "
               "extreme case of very small tile widths, you may find that the "
               "`-double_buffering' option hurts throughput.  In any case, the "
               "message is that for maximum throughput on a multi-processor "
               "platform, you should be prepared to play with both the "
               "`-num_threads' and `-double_buffering' options.\n"
               "\t   You may find the `-double_buffering' option to be useful "
               "even when working with only one thread, or when no threading "
               "environment is created at all (see `-num_threads').  In these "
               "cases, synthesized DWT lines are only single buffered, but "
               "increasing the size of this buffer (stripe height) allows the "
               "thread to do more work within a single tile-component, before "
               "moving to another tile-component.\n";
         out << "-cpu <coder-iterations>\n";
         if (comprehensive)
            out << "\tTimes end-to-end execution and, optionally, the block decoding "
               "operation, reporting throughput statistics.  If "
               "`coder-iterations' is 0, the block decoder will not be timed, "
               "leading to the most accurate end-to-end system execution "
               "times.  Otherwise, `coder-iterations' must be a positive "
               "integer -- larger values will result in more accurate "
               "estimates of the block decoder processing time, but "
               "degrade the accuracy of end-to-end execution times.  "
               "Note that end-to-end times include image file writing, which "
               "can have a dominant impact.  To avoid this, you may specify "
               "no output files at all.  Note also that timing information may "
               "not be at all reliable unless `-num_threads' is 1.  Since the "
               "default value for the `-num_threads' argument may be greater "
               "than 1, you should explicitly set the number of threads to 1 "
               "before collecting timing information.\n";
         out << "-mem -- Report memory usage\n";
         out << "-s <switch file>\n";
         if (comprehensive)
            out << "\tSwitch to reading arguments from a file.  In the file, argument "
               "strings are separated by whitespace characters, including spaces, "
               "tabs and new-line characters.  Comments may be included by "
               "introducing a `#' or a `%' character, either of which causes "
               "the remainder of the line to be discarded.  Any number of "
               "\"-s\" argument switch commands may be included on the command "
               "line.\n";
         out << "-record <file>\n";
         if (comprehensive)
            out << "\tRecord code-stream parameters in a file, using the same format "
               "which is accepted when specifying the parameters to the "
               "compressor. Parameters specific to tiles which do not intersect "
               "with the region of interest will not generally be recorded.\n";
         out << "-quiet -- suppress informative messages.\n";
         out << "-version -- print core system version I was compiled against.\n";
         out << "-v -- abbreviation of `-version'\n";
         out << "-usage -- print a comprehensive usage statement.\n";
         out << "-u -- print a brief usage statement.\"\n\n";
         out.flush();
         exit(0);
      }

// --------------------------------------------------------------------------
   kde_file_binding *parse_simple_args(
      kdu_args &args, char * &ifname,
      std::ostream * &record_stream,
      float &max_bpp, bool &transpose, bool &vflip, bool &hflip,
      bool &allow_shorts, int &skip_components,
      bool &want_alpha, int &jpx_layer, int &raw_codestream,
      kdu_component_access_mode &component_access_mode,
      bool &no_seek, int &max_layers, int &discard_levels,
      int &num_threads, int &double_buffering_height,
      int &cpu_iterations, bool &simulate_parsing,bool &mem, bool &quiet)

      /* Parses most simple arguments (those involving a dash). Most
      parameters are returned via the reference arguments, with the
      exception of the input file names, which are returned via a
      linked list of `kde_file_binding' objects.  Only the `fname'
      field of each `kde_file_binding' record is filled out here.
      Note that `max_bpp' is returned as negative if the bit-rate is
      not explicitly set.  Note also that the function may return NULL
      if no output files are specified; in this case, the decompressor
      is expected to run completely, but not output anything. The
      value returned via `cpu_iterations' is negative unless CPU times
      are required.  The `raw_codestream' variable will be set to the
      index of the code-stream to be decompressed if the
      `-raw_components' argument was used.  Otherwise it will be set
      to a negative integer.  Note that `num_threads' is set to 0 if
      no multi-threaded processing group is to be created, as distinct
      from a value of 1, which means that a multi-threaded processing
      group is to be used, but this group will involve only one
      thread. */
      {
         int rotate;
         kde_file_binding *files, *last_file, *new_file;

         if ((args.get_first() == NULL) || (args.find("-u") != NULL))
            kakadufunc::print_usage(args.get_prog_name());
         if (args.find("-usage") != NULL)
            kakadufunc::print_usage(args.get_prog_name(),true);
         if ((args.find("-version") != NULL) || (args.find("-v") != NULL))
            kakadufunc::print_version();      

         files = last_file = NULL;
         ifname = NULL;
         record_stream = NULL;
         rotate = 0;
         max_bpp = -1.0F;
         allow_shorts = true;
         skip_components = 0;
         want_alpha = true;
         jpx_layer = 0;
         raw_codestream = -1;
         component_access_mode = KDU_WANT_OUTPUT_COMPONENTS;
         no_seek = false;
         max_layers = 0;
         discard_levels = 0;
         num_threads = 0; // This is not actually the default -- see below.
         double_buffering_height = 0; // i.e., no double buffering
         cpu_iterations = -1;
         simulate_parsing = false;
         mem = false;
         quiet = false;

         if (args.find("-o") != NULL)
         {
            char *string, *cp;
            int len;

            if ((string = args.advance()) == NULL)
            { kdu_error e; e << "\"-o\" argument requires a file name!"; }
            while ((len = (int) strlen(string)) > 0)
            {
               cp = strchr(string,',');
               if (cp == NULL)
                  cp = string+len;
               new_file = new kde_file_binding(string,(int)(cp-string));
               if (last_file == NULL)
                  files = last_file = new_file;
               else
                  last_file = last_file->next = new_file;
               if (*cp == ',') cp++;
               string = cp;
            }
            args.advance();
         }

         if (args.find("-i") != NULL)
         {
            if ((ifname = args.advance()) == NULL)
            { kdu_error e; e << "\"-i\" argument requires a file name!"; }
            args.advance();
         }

         if (args.find("-jpx_layer") != NULL)
         {
            char *string = args.advance();
            if ((string == NULL) || (sscanf(string,"%d",&jpx_layer) != 1) ||
                (jpx_layer < 0))
            { kdu_error e; e << "\"-jpx_layer\" argument requires a "
                              "non-negative integer parameter!"; }
            args.advance();
         }

         if (args.find("-raw_components") != NULL)
         {
            raw_codestream = 0;
            char *string = args.advance();
            if ((string != NULL) && (*string != '-') &&
                (sscanf(string,"%d",&raw_codestream) == 1))
               args.advance();
         }

         if (args.find("-codestream_components") != NULL)
         {
            component_access_mode = KDU_WANT_CODESTREAM_COMPONENTS;
            args.advance();
         }

         if (args.find("-rate") != NULL)
         {
            char *string = args.advance();
            if ((string == NULL) || (sscanf(string,"%f",&max_bpp) != 1) ||
                (max_bpp <= 0.0F))
            { kdu_error e; e << "\"-rate\" argument requires a positive "
                              "numeric parameter!"; }
            args.advance();
         }

         if (args.find("-simulate_parsing") != NULL)
         {
            args.advance();
            simulate_parsing = true;
         }

         if (args.find("-skip_components") != NULL)
         {
            char *string = args.advance();
            if ((string == NULL) || (sscanf(string,"%d",&skip_components) != 1) ||
                (skip_components < 0))
            { kdu_error e; e << "\"-skip_components\" argument requires a "
                              "non-negative integer parameter!"; }
            args.advance();
         }

         if (args.find("-no_alpha") != NULL)
         {
            want_alpha = false;
            args.advance();
         }

         if (args.find("-no_seek") != NULL)
         {
            no_seek = true;
            args.advance();
         }

         if (args.find("-layers") != NULL)
         {
            char *string = args.advance();
            if ((string == NULL) || (sscanf(string,"%d",&max_layers) != 1) ||
                (max_layers < 1))
            { kdu_error e; e << "\"-layers\" argument requires a positive "
                              "integer parameter!"; }
            args.advance();
         }

         if (args.find("-reduce") != NULL)
         {
            char *string = args.advance();
            if ((string == NULL) || (sscanf(string,"%d",&discard_levels) != 1) ||
                (discard_levels < 0))
            { kdu_error e; e << "\"-reduce\" argument requires a non-negative "
                              "integer parameter!"; }
            args.advance();
         }

         if (args.find("-rotate") != NULL)
         {
            char *string = args.advance();
            if ((string == NULL) || (sscanf(string,"%d",&rotate) != 1) ||
                ((rotate % 90) != 0))
            { kdu_error e; e << "\"-rotate\" argument requires an integer "
                              "multiple of 90 degrees!"; }
            args.advance();
            rotate /= 90;
         }

         if (args.find("-precise") != NULL)
         {
            args.advance();
            allow_shorts = false;
         }

         if (args.find("-num_threads") != NULL)
         {
            char *string = args.advance();
            if ((string == NULL) || (sscanf(string,"%d",&num_threads) != 1) ||
                (num_threads < 0))
            { kdu_error e; e << "\"-num_threads\" argument requires a "
                              "non-negative integer."; }
            args.advance();
         }
         else if ((num_threads = kdu_get_num_processors()) < 2)
            num_threads = 0;

         if (args.find("-double_buffering") != NULL)
         {
            char *string = args.advance();
            if ((string == NULL) ||
                (sscanf(string,"%d",&double_buffering_height) != 1) ||
                (double_buffering_height < 1))
            { kdu_error e; e << "\"-double_buffering\" argument requires a "
                              "positive integer, specifying the number of rows from each "
                              "component which are to be double buffered."; }
            args.advance();
         }

         if (args.find("-cpu") != NULL)
         {
            char *string = args.advance();
            if ((string == NULL) || (sscanf(string,"%d",&cpu_iterations) 
                                     != 1) ||
                (cpu_iterations < 0))
            { kdu_error e; 
            e << "\"-cpu\" argument requires a non-negative "
               "integer, specifying the number of times to execute the block "
               "coder within a timing loop."; }
            args.advance();
         }

         if (args.find("-mem") != NULL)
         {
            mem = true;
            args.advance();
         }

         if (args.find("-quiet") != NULL)
         {
            quiet = true;
            args.advance();
         }

         if (args.find("-record") != NULL)
         {
            char *fname = args.advance();
            if (fname == NULL)
            { kdu_error e; e << "\"-record\" argument requires a file name!"; }
            record_stream = new std::ofstream(fname);
            if (record_stream->fail())
            { kdu_error e; e << "Unable to open record file, \"" << fname << "\"."; }
            args.advance();
         }

         if (ifname == NULL)
         { kdu_error e; e << "Must provide an input file name!"; }
         while (rotate >= 4)
            rotate -= 4;
         while (rotate < 0)
            rotate += 4;
         switch (rotate) {
            case 0: transpose = false; vflip = false; hflip = false; break;
            case 1: transpose = true; vflip = false; hflip = true; break;
            case 2: transpose = false; vflip = true; hflip = true; break;
            case 3: transpose = true; vflip = true; hflip = false; break;
         }

         return(files);
      }

// --------------------------------------------------------------------------

   kdu_long get_bpp_dims(siz_params *siz)
      {
         int comps, max_width, max_height, n;

         siz->get(Scomponents,0,0,comps);
         max_width = max_height = 0;
         for (n=0; n < comps; n++)
         {
            int width, height;
            siz->get(Sdims,n,0,height);
            siz->get(Sdims,n,1,width);
            if (width > max_width)
               max_width = width;
            if (height > max_height)
               max_height = height;
         }
         return ((kdu_long) max_height) * ((kdu_long) max_width);
      }

// --------------------------------------------------------------------------
   
   void set_region_of_interest(
      kdu_args &args, kdu_dims &region, siz_params *siz)
      {
         if (!(siz->get(Sorigin,0,0,region.pos.y) &&
               siz->get(Sorigin,0,1,region.pos.x) &&
               siz->get(Ssize,0,0,region.size.y) &&
               siz->get(Ssize,0,1,region.size.x)))
            assert(0);
         region.size.y -= region.pos.y;
         region.size.x -= region.pos.x;
         if (args.find("-region") == NULL)
            return;
         char *string = args.advance();
         if (string != NULL)
         {
            double top, left, height, width;

            if (sscanf(string,"{%lf,%lf},{%lf,%lf}",
                       &top,&left,&height,&width) != 4)
               string = NULL;
            else if ((top < 0.0) || (left < 0.0) || (height < 0.0) || 
                     (width < 0.0))
               string = NULL;
            else
            {
               region.pos.y += (int) floor(region.size.y * top);
               region.pos.x += (int) floor(region.size.x * left);
               region.size.y = (int) ceil(region.size.y * height);
               region.size.x = (int) ceil(region.size.x * width);
            }
         }
         if (string == NULL)
         { kdu_error e; 
         e << "The `-region' argument requires a set of coordinates "
            "of the form, \"{<top>,<left>},{<height>,<width>}\". All quantities "
            "must be real numbers in the range 0 to 1."; }
         args.advance();
      }

// --------------------------------------------------------------------------

   void extract_jp2_resolution_info(
      kdu_image_dims &idims, jp2_resolution resolution,
      kdu_coords ref_size, bool transpose)
      {
         if ((ref_size.x <= 0) || (ref_size.y <= 0))
            return;

         // Start by seeing whether we have display resolution info,
         // or only capture resolution info.

         bool for_display=true, have_absolute_res=true;
         float ypels_per_metre = resolution.get_resolution(for_display);
         if (ypels_per_metre <= 0.0F)
         {
            for_display = false;
            ypels_per_metre = resolution.get_resolution(for_display);
            if (ypels_per_metre <= 0.0F)
            { have_absolute_res = false; ypels_per_metre = 1.0F; }
         }
         float xpels_per_metre =
            ypels_per_metre * resolution.get_aspect_ratio(for_display);
         assert(xpels_per_metre > 0.0F);
         if (transpose)
            idims.set_resolution(ref_size.y,ref_size.x,have_absolute_res,
                                 ypels_per_metre,xpels_per_metre);
         else
            idims.set_resolution(ref_size.x,ref_size.y,have_absolute_res,
                                 xpels_per_metre,ypels_per_metre);
      }

// --------------------------------------------------------------------------

   void extract_jp2_colour_info(kdu_image_dims &idims, jp2_channels channels,
                                jp2_colour colour, bool have_alpha,
                                bool alpha_is_premultiplied)
      {
         int num_colours = channels.get_num_colours();
         bool have_premultiplied_alpha = have_alpha && alpha_is_premultiplied;
         bool have_unassociated_alpha = have_alpha && !alpha_is_premultiplied;
         int colour_space_confidence = 1;
         jp2_colour_space space = colour.get_space();
         if ((space == JP2_iccLUM_SPACE) ||
             (space == JP2_iccRGB_SPACE) ||
             (space == JP2_iccANY_SPACE) ||
             (space == JP2_vendor_SPACE))
            colour_space_confidence = 0;
         idims.set_colour_info(
            num_colours,have_premultiplied_alpha,
            have_unassociated_alpha,colour_space_confidence,space);
      }

// --------------------------------------------------------------------------
   void set_error_behaviour(kdu_args &args, kdu_codestream codestream)
      {
         bool fussy = false;
         bool resilient = false;
         bool ubiquitous_sops = false;
         if (args.find("-fussy") != NULL)
         { args.advance(); fussy = true; }
         if (args.find("-resilient") != NULL)
         { args.advance(); resilient = true; }
         if (args.find("-resilient_sop") != NULL)
         { args.advance(); resilient = true; ubiquitous_sops = true; }
         if (resilient)
            codestream.set_resilient(ubiquitous_sops);
         else if (fussy)
            codestream.set_fussy();
         else
            codestream.set_fast();
      }

// --------------------------------------------------------------------------
   void convert_samples_to_palette_indices(
      kdu_line_buf &src, kdu_line_buf &dst,
      int bit_depth, bool is_signed,int palette_bits)
      {
         int i=src.get_width();
         kdu_sample16 *dp = dst.get_buf16();
         assert(dp != NULL);
         if (src.get_buf32() != NULL)
         {
            kdu_sample32 *sp = src.get_buf32();
            if (src.is_absolute())
            {
               kdu_int32 offset = (is_signed)?0:((1<<bit_depth)>>1);
               kdu_int32 mask = ((kdu_int32)(-1))<<palette_bits;
               kdu_int32 val;
               for (; i > 0; i--, sp++, dp++)
               {
                  val = sp->ival + offset;
                  if (val & mask)
                     val = (val<0)?0:(~mask);
                  dp->ival = (kdu_int16) val;
               }
            }
            else
            {
               float scale = (float)(1<<(palette_bits+10)); 
				// 10 temp fraction bits
               kdu_int32 val;
               kdu_int32 offset = (is_signed)?0:(1<<(palette_bits+9));
               offset += (1<<9); // Rounding offset.
               kdu_int32 mask = ((kdu_int32)(-1))<<palette_bits;
               for (; i > 0; i--, sp++, dp++)
               {
                  val = (kdu_int32)(sp->fval * scale);
                  val = (val+offset)>>10;
                  if (val & mask)
                     val = (val<0)?0:(~mask);
                  dp->ival = (kdu_int16) val;
               }
            }
         }
         else
         {
            kdu_sample16 *sp = src.get_buf16();
            if (src.is_absolute())
            {
               kdu_int16 offset=(kdu_int16)((is_signed)?0:((1<<bit_depth)>>1));
               kdu_int16 mask = ((kdu_int16)(-1))<<palette_bits;
               kdu_int16 val;
               for (; i > 0; i--, sp++, dp++)
               {
                  val = sp->ival + offset;
                  if (val & mask)
                     val = (val<0)?0:(~mask);
                  dp->ival = val;
               }
            }
            else
            {
               kdu_int16 offset=(kdu_int16)((is_signed)?0:((1<<KDU_FIX_POINT)>>1));
               int downshift = KDU_FIX_POINT-palette_bits; assert(downshift > 0);
               offset += (kdu_int16)((1<<downshift)>>1);
               kdu_int32 mask = ((kdu_int16)(-1))<<palette_bits;
               kdu_int16 val;
               for (; i > 0; i--, sp++, dp++)
               {
                  val = (sp->ival + offset) >> downshift;
                  if (val & mask)
                     val = (val<0)?0:(~mask);
                  dp->ival = val;
               }
            }
         }
      }

// --------------------------------------------------------------------------
   kdu_long expand_single_threaded(
      kdu_codestream codestream, kdu_dims tile_indices,
      kde_file_binding *outputs, int num_output_channels,
      bool last_output_channel_is_alpha,
      bool alpha_is_premultiplied,
      int num_used_components, int *used_component_indices,
      jp2_channels channels, jp2_palette palette,
      bool allow_shorts, bool skip_ycc,int dwt_stripe_height)

      /* This function wraps up the operations required to actually
     decompress the image samples.  It is called directly from `main'
     after setting up the output files (passed in via the `outputs'
     list), configuring the `codestream' object and parsing relevant
     command-line arguments.  This particular function implements all
     decompression processing using a single thread of execution.
     This is the simplest approach.  From version 5.1 of Kakadu, the
     processing may also be efficiently distributed across multiple
     threads, which allows for the exploitation of multiple physical
     processors.  The implementation in that case is only slightly
     different from the multi-threaded case, but we encapsulate it in
     a separate version of this function, `expand_multi_threaded',
     mainly for illustrative purposes.  The function returns the
     amount of memory allocated for sample processing, including all
     intermediate line buffers managed by the DWT engines associated
     with each active tile-component and the block decoding machinery
     associated with each tile-component-subband.  The implementation
     here processes image lines one-by-one, maintaining W complete
     tile processing engines, where W is the number of tiles which
     span the width of the image (or the image region which is being
     reconstructed).  There are a variety of alternate processing
     paradigms which can be used.  The "kdu_buffered_expand"
     application demonstrates a different strategy, managed by the
     higher level `kdu_stripe_decompressor' object, in which whole
     image stripes are decompressed into a memory buffer.  If the
     stripe height is equal to the tile height, only one tile
     processing engine need be active at any given time in that model.
     Yet another model is used by the `kdu_region_decompress' object,
     which decompresses a specified image region into a memory buffer.
     In that case, processing is done tile-by-tile. */

      {
         int x_tnum;
         kde_flow_control **tile_flows = new kde_flow_control *[
            tile_indices.size.x];
         for (x_tnum=0; x_tnum < tile_indices.size.x; x_tnum++)
            tile_flows[x_tnum] = new
               kde_flow_control(
                  outputs,num_output_channels,
                  last_output_channel_is_alpha,alpha_is_premultiplied,
                  num_used_components,used_component_indices,
                  codestream,x_tnum,allow_shorts,channels,palette,
                  skip_ycc,dwt_stripe_height);
         bool done = false;
         while (!done)
         {
            while (!done)
            { // Process a row of tiles line by line.
               done = true;
               for (x_tnum=0; x_tnum < tile_indices.size.x; x_tnum++)
               {
                  if (tile_flows[x_tnum]->advance_components())
                  {
                     done = false;
                     tile_flows[x_tnum]->process_components();
                  }
               }
            }
            for (x_tnum=0; x_tnum < tile_indices.size.x; x_tnum++)
               if (tile_flows[x_tnum]->advance_tile())
                  done = false;
         }
         kdu_long processing_sample_bytes = 0;
         for (x_tnum=0; x_tnum < tile_indices.size.x; x_tnum++)
         {
            processing_sample_bytes += tile_flows[x_tnum]->
               get_buffer_memory();
            delete tile_flows[x_tnum];
         }
         delete[] tile_flows;

         return processing_sample_bytes;
      }

// --------------------------------------------------------------------------
   kdu_long expand_multi_threaded(
      kdu_codestream codestream, kdu_dims tile_indices,
      kde_file_binding *outputs, int num_output_channels,
      bool last_output_channel_is_alpha,
      bool alpha_is_premultiplied,
      int num_used_components, int *used_component_indices,
      jp2_channels channels, jp2_palette palette,
      bool allow_shorts, bool skip_ycc, int &num_threads,
      bool dwt_double_buffering, int dwt_stripe_height)

      /* This function provides exactly the same functionality as
     `expand_single_threaded', except that it uses Kakadu's
     multi-threaded processing features.  By and large,
     multi-threading does not substantially complicate the
     implementation, since Kakadu's threading framework conceal almost
     all of the details.  However, the application does have to create
     a multi-threaded environment, assigning it a suitable number of
     threads.  It must also be careful to close down the
     multi-threaded environment, which incorporates all required
     synchronization.  Upon return, `num_threads' is set to the actual
     number of threads which were created -- this value could be
     smaller than the value supplied on input, if insufficient
     internal resources exist.  For other aspects of the present
     function, refer to the comments found with
     `expand_single_threaded'. */
      {
         // Construct multi-threaded processing environment if required
         kdu_thread_env env;
         env.create();
         for (int nt=1; nt < num_threads; nt++)
            if (!env.add_thread())
            { num_threads = nt; break; }

         // Now set up the tile processing objects
         int x_tnum;
         kde_flow_control **tile_flows = new kde_flow_control *[
            tile_indices.size.x];
         for (x_tnum=0; x_tnum < tile_indices.size.x; x_tnum++)
         {
            kdu_thread_queue *tile_queue =
               env.add_queue(NULL,NULL,"tile expander");
            tile_flows[x_tnum] = new
               kde_flow_control(
                  outputs,num_output_channels,
                  last_output_channel_is_alpha,alpha_is_premultiplied,
                  num_used_components,used_component_indices,
                  codestream,x_tnum,allow_shorts,channels,palette,
                  skip_ycc,dwt_stripe_height,dwt_double_buffering,
                  &env,tile_queue);
         }

         // Now run the tile processing engines
         bool done = false;
         while (!done)
         {
            while (!done)
            { // Process a row of tiles line by line.
               done = true;
               for (x_tnum=0; x_tnum < tile_indices.size.x; x_tnum++)
               {
                  if (tile_flows[x_tnum]->advance_components(&env))
                  {
                     done = false;
                     tile_flows[x_tnum]->process_components();
                  }
               }
            }

            for (x_tnum=0; x_tnum < tile_indices.size.x; x_tnum++)
               if (tile_flows[x_tnum]->advance_tile(&env))
                  done = false;
         }

         // Cleanup processing environment
         env.terminate(NULL,true);
         env.destroy();
         kdu_long processing_sample_bytes = 0;
         for (x_tnum=0; x_tnum < tile_indices.size.x; x_tnum++)
         {
            processing_sample_bytes += tile_flows[x_tnum]->
               get_buffer_memory();
            delete tile_flows[x_tnum];
         }
         delete[] tile_flows;

         return processing_sample_bytes;
      }

} // kakadufunc namespace


