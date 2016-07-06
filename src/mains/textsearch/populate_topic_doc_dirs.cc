// ========================================================================
// Program POPULATE_TOPIC_DOC_DIRS generates a set of subdirectories
// labeled by coarse and fine topic IDs.  It then fills these
// subdirectories with soft links to document files for each coarse
// and fine topic.

// ========================================================================
// Last updated on 3/3/13; 5/27/13; 5/29/13
// ========================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::map;
   using std::ofstream;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string bundle_filename=passes_group.get_bundle_filename();
//   cout << " bundle_filename = " << bundle_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(bundle_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string image_list_filename=bundler_IO_subdir+"image_list.dat";

   string reuters_subdir=
      "/data_third_disk/text_docs/reuters/export/";
   string text_subdir=reuters_subdir+"text/50K_docs/";
   string topic_docs_subdir=text_subdir+"topic_docs/";

//   string arXiv_subdir="/media/66368D22368CF3F9/visualization/arXiv/";
//   string reuters_subdir=arXiv_subdir+"reuters/export/";
//   string text_subdir=reuters_subdir+"text/";
//   string mallet_subdir=text_subdir+"mallet/";
//   string topic_docs_subdir=mallet_subdir+"topic_docs/";

   filefunc::dircreate(topic_docs_subdir);
      
// Instantiate and populate STL map with relationships between
// text filenames and bundler image/document IDs:

   typedef map<std::string, int > FILENAME_DOCID_MAP;
// independent var = text filename prefix
// dependent var = bundler image/document ID

   FILENAME_DOCID_MAP filename_docid_map;
   FILENAME_DOCID_MAP::iterator filename_docid_iter;

// Store text filenames as function of bundler image/document ID
// within STL vector doc_filenames:

   vector<string> doc_filenames;

   filefunc::ReadInfile(image_list_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_image_filename=filefunc::text_line[i];
      string basename=filefunc::getbasename(curr_image_filename);
      string prefix=stringfunc::prefix(basename);
      filename_docid_map[prefix]=i;
//      cout << "bundler ID = " << i << " prefix = " << prefix << endl;

      string curr_doc_filename=text_subdir+prefix+".txt";
      doc_filenames.push_back(curr_doc_filename);
   }

// Store coarse and fine topic ID assignments for each document within
// STL map:

   typedef map<int,pair<int,int> > DOC_VS_TOPIC_IDS_MAP;
   DOC_VS_TOPIC_IDS_MAP doc_vs_topic_ids_map;
   DOC_VS_TOPIC_IDS_MAP::iterator iter;
   
// independent var = document ID
// dependent vars: coarse topic ID, fine topic ID

   vector<int> document_IDs,coarse_topic_IDs,fine_topic_IDs;

   string topics_docs_filename=bundler_IO_subdir+"coarse_fine_topic_docs.dat";
   filefunc::ReadInfile(topics_docs_filename);

   int n_coarse_topics=-1;
   int n_fine_topics=-1;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_line=filefunc::text_line[i];
//      cout << "curr_line = " << curr_line << endl;
      vector<string> substrings=
         stringfunc::decompose_string_into_substrings(curr_line);
      int coarse_topic_ID=stringfunc::string_to_number(substrings[0]);
      int fine_topic_ID=stringfunc::string_to_number(substrings[1]);
//      cout << "coarse ID = " << coarse_topic_ID
//           << " fine ID = " << fine_topic_ID << endl;
      n_coarse_topics=basic_math::max(n_coarse_topics,coarse_topic_ID);
      n_fine_topics=basic_math::max(n_fine_topics,fine_topic_ID);

      string curr_topic_doc_subdir=topic_docs_subdir+
         "topic_"+stringfunc::integer_to_string(coarse_topic_ID,3)+"_"+
         stringfunc::integer_to_string(fine_topic_ID,4);
//      cout << "curr_topic_doc_subdir = "
//           << curr_topic_doc_subdir << endl;
      filefunc::dircreate(curr_topic_doc_subdir);

      vector<int> doc_IDs;
      for (unsigned int j=2; j<substrings.size(); j++)
      {
         document_IDs.push_back(stringfunc::string_to_number(substrings[j]));
         int curr_doc_ID=document_IDs.back();
         pair<int,int> P(coarse_topic_ID,fine_topic_ID);
         doc_vs_topic_ids_map[curr_doc_ID]=P;
      } // loop over index j labeling substrings
   } // loop over index i labeling lines in topics_docs_filename

   n_coarse_topics++;
   n_fine_topics++;
   cout << "n_coarse_topics = " << n_coarse_topics << endl;
   cout << "n_fine_topics = " << n_fine_topics << endl;

// Iterate over all documents within doc_vs_topic_ids_map.  Create
// soft links between document filenames and new files within topic
// subdirectories labeled by coarse and fine topic IDs:

   int counter=0;
   int n_docs=doc_vs_topic_ids_map.size();
   cout << "n_docs = " << n_docs << endl;
   for (iter=doc_vs_topic_ids_map.begin(); iter !=
           doc_vs_topic_ids_map.end(); iter++)
   {
      if (counter%1000==0) 
      {
         double doc_frac=double(counter)/n_docs;
         cout << doc_frac << " " << flush;
      }
      counter++;
      
      int doc_ID=iter->first;
      string curr_doc_filename=doc_filenames[doc_ID];
      int coarse_topic_ID=iter->second.first;
      int fine_topic_ID=iter->second.second;
      string curr_topic_doc_subdir=topic_docs_subdir+
         "topic_"+stringfunc::integer_to_string(coarse_topic_ID,3)+"_"+
         stringfunc::integer_to_string(fine_topic_ID,4);

//      cout << " coarse ID = " << coarse_topic_ID
//           << " fine ID = " << fine_topic_ID 
 //          << " doc = " << doc_filename 
//           << endl;
      
      string unix_cmd="ln -s "+curr_doc_filename+" "+curr_topic_doc_subdir;
//      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
   }
   cout << endl;
}
