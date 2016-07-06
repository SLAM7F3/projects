// ========================================================================
// Program POPULATE_GRAPH_DIRS generates a set of subdirectories of
// bundler_IO_subdir/graphs/ labeled by coarse and fine topic IDs.  It
// then fills these subdirectories with sift_edgelist.dat =
// doc_edgelist.dat files.  POPULATE_GRAPH_DIRS subsequently runs
// OGDF_layout, extract_OGDF_layout, kmeans_clusters and
// generate_component_hierarchy scripts on each subdirectory's
// sift_edgelist.dat file.
// ========================================================================
// Last updated on 3/4/13; 5/25/13; 5/26/13; 5/29/13
// ========================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "graphs/graphdbfuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "math/mathfuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
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

   timefunc::initialize_timeofday_clock();

   bool modify_IMAGERY_database_flag=true;
//   bool modify_IMAGERY_database_flag=false;
   if (modify_IMAGERY_database_flag)
   {
      cout << "modify_IMAGERY_database_flag = true" << endl;
   }
   else
   {
      cout << "modify_IMAGERY_database_flag = false" << endl;
   }
//   outputfunc::enter_continue_char(); // comment out for Reuters 43K

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   string bundle_filename=passes_group.get_bundle_filename();
//   cout << " bundle_filename = " << bundle_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(bundle_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string photosynth_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/";
   string components_filename=bundler_IO_subdir+"components.dat";

   string graphs_basedir=bundler_IO_subdir+"graphs_50K/";
//   string graphs_basedir=bundler_IO_subdir+"graphs_43K/";
   cout << "graphs_basedir = " << graphs_basedir << endl;
   filefunc::dircreate(graphs_basedir);

   string reuters_subdir=
      "/data_third_disk/text_docs/reuters/export/";
   string text_subdir=reuters_subdir+"text/50K_docs/";
   string topic_docs_subdir=text_subdir+"topic_docs/";

//   string arXiv_subdir="/media/66368D22368CF3F9/visualization/arXiv/";
//   string reuters_subdir=arXiv_subdir+"reuters/export/";
//   string text_subdir=reuters_subdir+"text/";
//   string mallet_subdir=text_subdir+"mallet/";
//   string topic_docs_subdir=mallet_subdir+"topic_docs/";

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

// Generate topic subdirectories within bundler_IO_dir/graphs which
// will eventually hold files needed to populate tables within imagery
// database:

   int max_child_node_ID=0;
   int n_coarse_topics=-1;
   int n_fine_topics=-1;
   for (int i=0; i<filefunc::text_line.size(); i++)
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

      for (int d=2; d<substrings.size(); d++)
      {
         int curr_doc_ID=stringfunc::string_to_number(substrings[d]);
         max_child_node_ID=basic_math::max(max_child_node_ID,curr_doc_ID);
      }

      string graphs_topic_doc_subdir=graphs_basedir+
         "topic_"+stringfunc::integer_to_string(coarse_topic_ID,3)+"_"+
         stringfunc::integer_to_string(fine_topic_ID,4)+"/";
      filefunc::dircreate(graphs_topic_doc_subdir);

      string curr_topic_doc_subdir=topic_docs_subdir+
         "topic_"+stringfunc::integer_to_string(coarse_topic_ID,3)+"_"+
         stringfunc::integer_to_string(fine_topic_ID,4)+"/";
      string unix_cmd="cp "+curr_topic_doc_subdir+"docs_edgelist.dat "+
         graphs_topic_doc_subdir+"sift_edgelist.dat";
      sysfunc::unix_command(unix_cmd);
   } // loop over index i labeling lines in topics_docs_filename

   n_coarse_topics++;
   n_fine_topics++;
   cout << "n_coarse_topics = " << n_coarse_topics << endl;
   cout << "n_fine_topics = " << n_fine_topics << endl;
   cout << "max_child_node_ID = " << max_child_node_ID << endl;

   int hierarchy_ID=-1;
//   hierarchy_ID=15;	// Reuters 50K on BEAST
   cout << "Enter ID for new graph hierarchy to be entered into IMAGERY database:" << endl;
   cin >> hierarchy_ID;

   int campaign_ID,mission_ID;
   campaign_ID=6;	// Text documents
//   mission_ID=3;	// Reuters 43K
   mission_ID=4;	// Reuters 50K
   cout << "Enter campaign_ID:" << endl;
//   cin >> campaign_ID;
   cout << "Enter mission_ID:" << endl;
//   cin >> mission_ID;

   if (!imagesdatabasefunc::images_in_database(
      postgis_db_ptr,campaign_ID,mission_ID))
   {
      cout << "ERROR: No images corresponding to campaign_ID = "
           << campaign_ID << " and mission_ID = " << mission_ID
           << " exist in images table of IMAGERY database!" << endl;
      exit(-1);
   }

   const int n_levels=3;
   int n_total_nodes=0;
   int n_total_links=0;
   for (int level=0; level < n_levels; level++)
   {
      int graph_ID=level;
      int parent_graph_ID=graph_ID+1;
      if (parent_graph_ID==n_levels) parent_graph_ID=-1;
      
      string SQL_command=graphdbfunc::generate_insert_graph_SQL_command(
         hierarchy_ID,graph_ID,level,parent_graph_ID,
         n_total_nodes,n_total_links);
      vector<string> insert_commands;
      insert_commands.push_back(SQL_command);

      if (modify_IMAGERY_database_flag)
      {
         postgis_db_ptr->set_SQL_commands(insert_commands);
         postgis_db_ptr->execute_SQL_commands();
      }
   }
   
// Parse components_filename to retrieve connected components metadata
// for specified graph hierarchy:

   const int n_lines_per_fine_topic=6;
   const int n_labels_per_fine_topic=n_lines_per_fine_topic-1;

   filefunc::ReadInfile(components_filename);
   int n_lines=filefunc::text_line.size();

   n_fine_topics=n_lines/n_lines_per_fine_topic;
   cout << "n_fine_topics = number graph components " 
        << n_fine_topics << endl;

   int line_number=0;
   int n_total_child_nodes=0;
   for (int f=0; f<n_fine_topics; f++)
   {
      string curr_line=filefunc::text_line[line_number++];
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         curr_line);
      int cc_ID=stringfunc::string_to_number(substrings[0]);
      int n_nodes=stringfunc::string_to_number(substrings[1]);
      int cc_row=stringfunc::string_to_number(substrings[2]);
      int cc_column=stringfunc::string_to_number(substrings[3]);
      string cc_label=substrings[4];
      n_total_child_nodes += n_nodes;

      cout << "cc_ID = " << cc_ID
           << " n_nodes = " << n_nodes
           << " n_total_child_nodes = " << n_total_child_nodes
           << " cc_row = " << cc_row
           << " cc_column = " << cc_column
           << " cc_label = " << cc_label << endl;

      vector<string> topic_labels;
      for (int t=0; t<n_labels_per_fine_topic; t++)
      {
         topic_labels.push_back(filefunc::text_line[line_number++]);
//         cout << "t = " << t << " topic_label = " << topic_labels.back()
//              << endl;
      }

      if (modify_IMAGERY_database_flag)
      {
         for (int level=0; level < n_levels; level++)
         {
            int graph_ID=level;
            graphdbfunc::insert_connected_component(
               postgis_db_ptr,hierarchy_ID,graph_ID,level,cc_ID,
               cc_row,cc_column,cc_label);
            for (int t=0; t<topic_labels.size(); t++)
            {
               graphdbfunc::update_connected_component_topic(
                  postgis_db_ptr,hierarchy_ID,graph_ID,cc_ID,
                  t,topic_labels[t]);
            } // loop over index t labeling topic labels 
         } // loop over index level
      } // modify_IMAGERY_database_flag conditional

   } // loop over index f labeling fine topics
   
// Start processing graph information within each
// graphs_topic_doc_subdir:

   string scriptname;
   ofstream scriptstream;

   filefunc::ReadInfile(topics_docs_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_line=filefunc::text_line[i];
//      cout << "curr_line = " << curr_line << endl;
      vector<string> substrings=
         stringfunc::decompose_string_into_substrings(curr_line);
      int coarse_topic_ID=stringfunc::string_to_number(substrings[0]);
      int fine_topic_ID=stringfunc::string_to_number(substrings[1]);
//      cout << "coarse ID = " << coarse_topic_ID
//           << " fine ID = " << fine_topic_ID << endl;

      string graphs_topic_doc_subdir=
         "./"+filefunc::getbasename(graphs_basedir)+
         "topic_"+stringfunc::integer_to_string(coarse_topic_ID,3)+"_"+
         stringfunc::integer_to_string(fine_topic_ID,4)+"/";
      string unix_cmd="/bin/rm "+bundler_IO_subdir+"graphs";
      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      unix_cmd="ln -s "+graphs_topic_doc_subdir+" "+bundler_IO_subdir+"graphs";
      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

// Generate and run OGDF_layout script:

      scriptname=photosynth_subdir+"run_OGDF_layout";
      filefunc::openfile(scriptname,scriptstream);
      scriptstream << "../graphs/OGDF_layout \\" << endl;
      scriptstream << "--region_filename ./bundler/textdocs/reuters/packages/peter_inputs.pkg \\" << endl;
      scriptstream << "--graph_component_ID " 
                   << stringfunc::number_to_string(i) << endl;
      filefunc::closefile(scriptname,scriptstream);
      unix_cmd="chmod a+x "+scriptname;
      sysfunc::unix_command(unix_cmd);
      unix_cmd=scriptname;
      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

// Generate and run extract_OGDF_layout script:

      scriptname=photosynth_subdir+"run_extract_OGDF_layout";
      filefunc::openfile(scriptname,scriptstream);
      scriptstream << "../graphs/extract_OGDF_layout \\" << endl;
      scriptstream << "--region_filename ./bundler/textdocs/reuters/packages/peter_inputs.pkg \\" << endl;
      scriptstream << "--graph_component_ID " 
                   << stringfunc::number_to_string(i) << endl;
      filefunc::closefile(scriptname,scriptstream);
      unix_cmd="chmod a+x "+scriptname;
      sysfunc::unix_command(unix_cmd);
      unix_cmd=scriptname;
      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

// Generate and run kmeans_clusters script:

      scriptname=photosynth_subdir+"run_kmeans_clusters";
      filefunc::openfile(scriptname,scriptstream);
      scriptstream << "../graphs/kmeans_clusters \\" << endl;
      scriptstream << "--region_filename ./bundler/textdocs/reuters/packages/peter_inputs.pkg \\" << endl;
      scriptstream << "--graph_component_ID " 
                   << stringfunc::number_to_string(i) << endl;
      filefunc::closefile(scriptname,scriptstream);
      unix_cmd="chmod a+x "+scriptname;
      sysfunc::unix_command(unix_cmd);
      unix_cmd=scriptname;
      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      cout << "graphs_topic_doc_subdir = " 
           << graphs_topic_doc_subdir << endl;

// Generate and run generate_component_hierarchy script:

      scriptname=photosynth_subdir+"run_generate_component_hierarchy";
      filefunc::openfile(scriptname,scriptstream);
      scriptstream << "./generate_component_hierarchy \\" << endl;
      scriptstream << "--region_filename ./bundler/textdocs/reuters/packages/peter_inputs.pkg \\" << endl;
      
      scriptstream << "--GIS_layer ./packages/imagery_metadata.pkg \\" 
                   << endl;
      scriptstream << "--campaign_ID " 
                   << stringfunc::number_to_string(campaign_ID)+" \\" << endl;
      scriptstream << "--mission_ID " 
                   << stringfunc::number_to_string(mission_ID)+" \\" << endl;
      scriptstream << "--graph_hierarchy_ID " 
                   << stringfunc::number_to_string(hierarchy_ID)+" \\" << endl;
      scriptstream << "--graph_component_ID " 
                   << stringfunc::number_to_string(i)+" \\" << endl;
      scriptstream << "--max_child_node_ID " 
                   << stringfunc::number_to_string(max_child_node_ID) << endl;
      filefunc::closefile(scriptname,scriptstream);
      unix_cmd="chmod a+x "+scriptname;
      sysfunc::unix_command(unix_cmd);

      unix_cmd=scriptname;
      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      outputfunc::print_elapsed_time();

//       outputfunc::enter_continue_char();
   } // loop over index i labeling lines in coarse_fine_topic_docs.dat

}
