// ==========================================================================
// Program GENERATE_MCL_SCRIPTS creates an executable script which
// calls the Markov Cluster Algorithm (MCL) for each connected
// component of the current graph.  The executable script is called
// run_mcl and is written to mains/photosynth/.  

// ==========================================================================
// Last updated on 2/22/12; 3/10/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ofstream;
   using std::ostream;
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
   string graphs_subdir=bundler_IO_subdir+"graphs/";

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("gml");
   vector<string> gml_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,graphs_subdir);

   string script_filename="./run_mcl";
   ofstream script_stream;
   filefunc::openfile(script_filename,script_stream);

   double inflation_01_value=8.0;
   double inflation_02_value=1.3;

   string banner="Does input imagery correspond to video frames (y/n)?";
   outputfunc::write_big_banner(banner);
   string answer;
   cin >> answer;
   string video_char=answer.substr(0,1);
   if (video_char=="y" || video_char=="Y")
   {
      inflation_01_value=2.0;
      inflation_02_value=1.025;
   }

   int n_connected_components=gml_filenames.size();
   for (int connected_component=0; connected_component<
           n_connected_components; connected_component++)
   {
      string connected_component_label="_C"+
         stringfunc::number_to_string(connected_component);
      string edgelist_filename=graphs_subdir+"connected_edgelist"+
         connected_component_label
         +".dat";

      const int n_threads=10;	// BEAST

      vector<double> level_0n_I_value;
      level_0n_I_value.push_back(inflation_01_value);	
      level_0n_I_value.push_back(inflation_02_value);	
//      level_0n_I_value.push_back(8.0);	
//      level_0n_I_value.push_back(1.3);	
      for (unsigned int n=1; n<=level_0n_I_value.size(); n++)
      {
         string n_str=stringfunc::number_to_string(n);
         string level_mcl_clusters_filename=
            graphs_subdir+"level_0"+n_str+"_clusters_mcl"+
            connected_component_label+".dat";
         string mcl_cmd="mcl "+edgelist_filename
            +" -t "+stringfunc::number_to_string(n_threads)
            +" -I "+stringfunc::number_to_string(level_0n_I_value[n-1])
            +" --abc "
            +"-o "+level_mcl_clusters_filename;
         script_stream << mcl_cmd << endl;

         string clusters_filename=
            graphs_subdir+"level_0"+n_str+"_clusters"+
            connected_component_label+".dat";
         string unix_cmd="cp "+level_mcl_clusters_filename+" "+
            clusters_filename;
         script_stream << unix_cmd << endl;

      } // loop over index n labeling MCL levels

   } // loop over connected_component index 

   filefunc::closefile(script_filename,script_stream);
   string unix_command="chmod a+x "+script_filename;
   sysfunc::unix_command(unix_command);

   banner="Wrote run_mcl script";
   outputfunc::write_big_banner(banner);
}
