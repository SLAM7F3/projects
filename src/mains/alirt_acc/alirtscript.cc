// ==========================================================================
// Program ALIRTSCRIPT
// ==========================================================================
// Last updated on 6/24/03
// ==========================================================================

#include "myinclude.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::ostream;
   using std::cout;
   using std::ios;
   using std::endl;
   using std::string;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

   bool input_param_file;
   int n,nscripts;
   int ninputlines,currlinenumber=0;
   string program_name,scriptdir_name,script_prefix,metadir_name;
   string script_name,batch_name;
   string unixcommandstr;

   string inputline[200],outputline[5];
   string PWD=get_prefix();
   std::ofstream scriptstream,batchstream;

   parameter_input(argc,argv,input_param_file,inputline,ninputlines);
   cout.precision(3);
   cout.setf(ios::showpoint);

   outputfunc::newline();
   outputline[0]="Enter name of ALIRT main program to be run:";
   program_name=mygetstring(1,outputline,input_param_file,inputline,
                            currlinenumber);

   outputfunc::newline();
   outputline[0]="Enter full pathname for directory where scripts";
   outputline[1]="will be written:";
   scriptdir_name=mygetstring(2,outputline,input_param_file,inputline,
                              currlinenumber);

// Make sure output directory exists.  If not, create it...

   if (!direxist(scriptdir_name))
   {
      unixcommandstr="mkdir -p "+scriptdir_name;
      unix_command(unixcommandstr);
   }

   outputfunc::newline();
   outputline[0]="Enter script filename prefix:";
   script_prefix=mygetstring(1,outputline,input_param_file,inputline,
                             currlinenumber);

   outputfunc::newline();
   outputline[0]="Enter name of imagedir subdirectory where program's";
   outputline[1]="metafile output will be written:";
   metadir_name=mygetstring(2,outputline,input_param_file,inputline,
                            currlinenumber);

   outputfunc::newline();
   outputline[0]="Enter maximum number of scripts to be generated:";
   nscripts=mygetinteger(1,outputline,input_param_file,inputline,
                        currlinenumber);

// Write out individual pass batch files:

   for (n=0; n<nscripts; n++)
   {
      batch_name=scriptdir_name+"/"+script_prefix+"."+number_to_string(n+1);
      openfile(batch_name,batchstream);

      batchstream << "#######################################################################" << endl;
      batchstream << "# Script to run program "+program_name << endl;
      batchstream << "#######################################################################" << endl;
      batchstream << "# Last updated on "+getcurrdate() << endl;
      batchstream << "#######################################################################" << endl;
      batchstream << endl;

      batchstream << "# Enter name of imagedir subdirectory " << endl;
      batchstream << "# where metafile image will be written:" << endl;
      batchstream << endl;
      batchstream << metadir_name+"."+number_to_string(n+1) << endl;
      batchstream << endl;

//      batchstream << "# Enter pass number:" << endl;
//      batchstream << endl;
//      batchstream << n+1 << endl;
//      batchstream << endl;

      batchstream << "# Enter starting image number:" << endl;
      batchstream << endl;
      batchstream << 1 << endl;
      batchstream << endl;

      closefile(batch_name,batchstream);
   } // loop over batch file index n 

// Write out script to run all batch files:

   script_name=scriptdir_name+"/"+"run_"+program_name;
   openfile(script_name,scriptstream);

   for (n=0; n<nscripts; n++)
   {
      batch_name=scriptdir_name+"/"+script_prefix+"."+number_to_string(n+1);
      scriptstream << program_name+" "+batch_name << endl;
   }
   closefile(script_name,scriptstream);

   unixcommandstr="chmod a+x "+script_name;
   unix_command(unixcommandstr);
}




