/********************************************************************
 *
 *
 * Name: base_file.cc
 *
 *
 * Author: Joseph Adams
 *
 * Description:
 * simple file class
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/

#include "base_file.h"
#include "swapbyte.h"
base_file::base_file(void)
{

   // filename=new char[1024];

   filename="default.txt";
   file_is_open=0;
   fp=NULL;
   set_endian(little);

};

base_file::base_file(const char *filename_)
{

   // filename=new char[1024];

   filename=filename_;
   file_is_open=0;
   fp=NULL;
   set_endian(little);
};

base_file::base_file(const string& filename_)
{

   // filename=new char[1024];

   filename=filename_;
   file_is_open=0;
   fp=NULL;
   set_endian(little);
};

base_file::~base_file(void)
{

#ifdef _BASE_FILE_DEBUG_
   fprintf(stderr,"\nEntering ~base_file()..");
#endif

   if (file_is_open)  close();

#ifdef _BASE_FILE_DEBUG_
   fprintf(stderr,"...done");
#endif

}


/*void base_file::set_filename(const char new_name[] )
{

    filename=new_name;

 
    return;

}*/

void base_file::set_filename(const char* filename_)
{

  
   filename=filename_;
   return;

};

/*id base_file::get_filename_no_path_no_extension(char * filename_)
{
	char * start=strrchr(filename,'\\')+1;
	char * end=strchr(filename,'.');
	strncpy(filename_,start,end-start);
}*/
int base_file::file_exists(void)
{

   if (file_is_initialized)
   {
      // open the file in read mode to see if it exists
      if ((fp = fopen(filename.c_str(),"r")))
      {
         fclose(fp);
         return 1;
      }
      else return 0;
   }
   else
      error("Call to base_file::file_exists(void) with unitialized file");

   // this point cannot be reached
   return -1;
}


void base_file::open_output_does_not_exist(void)
{

   if (file_is_open) close();

   if (file_exists())
   {
      fprintf(stderr,"\n\nERROR: The file %s already exists",filename.c_str());
      error("Either delete the old file by hand or change the filename");
      return;
   }

   if ((fp = fopen(filename.c_str(),"w"))==0)
   {
      fprintf(stderr,"\n\nERROR: problem opening file %s",filename.c_str());
      error("check permissions, directories, etc...");
      check_io_error();
   }

   file_is_open=1;

#ifdef _BASE_FILE_DEBUG_
   fprintf(stderr,"\n Opening new file <%s>...",filename.c_str());
#endif

}

void base_file::open(const char mode[])
{

   if (file_is_open) close();

   if ((fp = fopen(filename.c_str(),mode))==0)
   {
      fprintf(stderr,"\n\nERROR: problem opening file %s",filename.c_str());
      error("check permissions, directories, etc...");
      check_io_error();
   }

#ifdef _BASE_FILE_DEBUG_
   fprintf(stderr,"\n Opening file <%s> in mode %s",filename.c_str(),mode);
#endif
   file_is_open=1;

}


void base_file::open_output_overwrite(void)
{

   if (file_is_open) close();

   if ((fp = fopen(filename.c_str(),"w+"))==0)
   {
      fprintf(stderr,"\n\nERROR: problem opening file %s",filename.c_str());
      error("check permissions, directories, etc...");
      check_io_error();
   }

#ifdef _BASE_FILE_DEBUG_
   fprintf(stderr,"\n Opening file <%s> for overwrite",filename.c_str());
#endif
   file_is_open=1;

}

/*
the ``b''  is  ignored  on
       all  POSIX  conforming  systems,  including Linux.

*/
void base_file::open_output_overwrite_binary(void)
{

   if (file_is_open) close();

   if ((fp = fopen(filename.c_str(),"wb+"))==0)
   {
      fprintf(stderr,"\n\nERROR: problem opening file %s",filename.c_str());
      error("check permissions, directories, etc...");
      check_io_error();
   }

#ifdef _BASE_FILE_DEBUG_
   fprintf(stderr,"\n Opening file <%s> for overwrite",filename.c_str());
#endif
   file_is_open=1;

}


void base_file::open_output_append(void)
{

   if (file_is_open) close();

   if ((fp = fopen(filename.c_str(),"a+"))==0)
   {
      fprintf(stderr,"\n\nERROR: problem opening file %s",filename.c_str());
      error("check permissions, directories, etc...");
      check_io_error();
   }

#ifdef _BASE_FILE_DEBUG_
   fprintf(stderr,"\n Opening file <%s> for append",filename.c_str());
#endif
   file_is_open=1;

}


void base_file::open_input_read_only(void)
{
   if (file_is_open) close();

   if ((fp = fopen(filename.c_str(),"r"))==0)
   {
      fprintf(stderr,"\n\nERROR: problem opening file %s",filename.c_str());
      error("check permissions, directories, etc...");
      check_io_error();
   }

   file_is_open=1;

#ifdef _BASE_FILE_DEBUG_
   fprintf(stderr,"\n Opening file <%s> for reading",filename.c_str());
#endif

}

void base_file::open_input_read_only_binary(void)
{
   if (file_is_open) close();

   if ((fp = fopen(filename.c_str(),"rb"))==0)
   {
      fprintf(stderr,"\n\nERROR: problem opening file %s as input read only binary",filename.c_str());
      error("check permissions, directories, etc...");
      check_io_error();
   }

   file_is_open=1;

#ifdef _BASE_FILE_DEBUG_
   fprintf(stderr,"\n Opening file <%s> for reading",filename.c_str());
#endif

}


void base_file::close(void)
{
#ifdef _BASE_FILE_DEBUG_
   fprintf(stderr,"\nbase_file::close()");
#endif

   if (fp != NULL)
   {
      fclose(fp);
#ifdef _BASE_FILE_DEBUG_
      fprintf(stderr,"\n Closing file <%s>...",filename.c_str());
#endif

   }
#ifdef _BASE_FILE_DEBUG_
   else
      fprintf(stderr,"\n File <%s> Already closed...",filename.c_str());
#endif

   file_is_open=0;
   fp=NULL;

#ifdef _BASE_FILE_DEBUG_
   fprintf(stderr,"done");
#endif

}


/// error handling--crash&burn
void base_file::error(char error_text[]) const
{
   //  check_io_error();
   fprintf(stderr,"\n%s\n\n",error_text);
   fflush(stderr);

   exit(1);
}


void base_file::warning(char error_text[]) const
{
   fprintf(stderr,"\n%s",error_text);

}


int  base_file::check_io_error(void) const
{
   int n;

   n=ferror(fp);

#ifdef _BASE_FILE_DEBUG_
   //  if (n)
   //    {
   //      perror("Error in class base_file");
   //
   //    }
#endif

   return n;

}


float base_file::read_single_float_at_BOL(void)
{
   int buffer_length=1024;
   char buffer[1024];

   //  	int max_buffer_length;
   //    int buffer_length;
   //char *buffer;

   if (fgets(buffer,buffer_length,fp)==NULL)
      buffer_length=0;
   else
      buffer_length=strlen(buffer);

#ifdef _BASE_FILE_DEBUG_
   fprintf(stderr,"\n<%s>",buffer);
#endif

   float x;

   read_result=sscanf(buffer,"%f",&x);

   return x;

}



// file positioning commands
void base_file::move_to(long offset)
{
   long whence=SEEK_SET;  //  position relatie to start of file
   fseek(fp, offset, whence);

}

// file positioning commands
void base_file::skip_forward(long offset)
{
   long whence=SEEK_CUR;  //  position relative to current position
   fseek(fp, offset, whence);

}

// file positioning commands
long base_file::get_current_pos(void)
{

   long cur_pos;
   fflush(fp);
   cur_pos=ftell(fp);
  
   return(cur_pos);
}

// file positioning commands
int base_file::is_eof(void)
{
  
   return feof(fp);

}

// file positioning commands
void base_file::move_to_beginning(void)
{
   rewind(fp);

   return;
}

// file positioning commands
void base_file::move_to_end(void)
{
 
   // long whence=SEEK_END;  //  position relative to end
   fseek(fp, 0L, SEEK_END);

   return;
}

// gets size of file, file must be open
long base_file::file_size(void)
{

   long initial_pos;

   long c1;
   long c2;

   initial_pos=get_current_pos();

   move_to_beginning();
   c1=get_current_pos();
 

   move_to_end();
   c2=get_current_pos();

   move_to(initial_pos);

#ifdef _BASE_FILE_DEBUG_
   fprintf(stderr,"\nEntering base_file::file_size()..");
   fprintf(stderr,"\nBOF=%ld",c1);
   fprintf(stderr,"\nEOF=%ld",c2);
   fprintf(stderr,"\n");
   fflush(stderr);
#endif

   // when checking this with ls be careful!! 
   // ls uses weird units sometimes (ie blocks)
   // ls reports the size allocated on the disk for the file
   // not necessarily the file size
   // use ls -l for exact file size
   return(c2-c1);
}




/*
 long  base_file::free_disk_space(void) const
{
 
  // if the file does not exist this routine will barf

  //  int n=0;

  FILE *MY_PIPE;

  // allocate a charater array to query the df utility
  char *cmd;

  cmd=(char *)calloc(5000,sizeof(char));

  if (cmd==0) 
   {
     fprintf(stderr,"\n Failed to allocate memory for command string");
     exit(1);
   }

  // the -P gives posix compliant output!!!
  sprintf(cmd,"df -P");

  if (get_string_length(dir_name)==0)  // use CWD
      sprintf(cmd, "%s %s", cmd, "./");
  else
      sprintf(cmd, "%s %s", cmd, dir_name);

#ifdef _BASE_FILE_DEBUG_
    fprintf(stdout, "\nEvaluating the command: %s",cmd);
#endif
 
  MY_PIPE=popen(cmd,"r");

  
  // skip the first line
  char c='\0';

  while (c != '\n')
    c=fgetc(MY_PIPE);  

  // parse the next line
  char mount_point[256];
  long  size, used,bytes_free;

  fscanf(MY_PIPE, "%s %ld %ld %ld", mount_point, &size, &used, &bytes_free);

  

  // pclose the pipe
  pclose(MY_PIPE); 
#ifdef _BASE_FILE_DEBUG_
 fprintf(stdout, "\nSize %ld \nUsed %ld \nFree %ld",size,used,bytes_free);
 fprintf(stdout, "\ndone");
#endif
  free(cmd);



  
  
  // return bytes_free*1024;

 return bytes_free;

} 
*/

void base_file::set_endian(endian_t e){
   if(e==little){
      little_endian=1;
      read_function=&little_endian_fread;
      write_function=&little_endian_fwrite;
   }
   else{
      little_endian=0;
      read_function=&big_endian_fread;
      write_function=&big_endian_fwrite;
   }
}

int	base_file::is_little_endian(){
   return little_endian;
}

void fileparts(const string& full_file, 
               string& path, 
               string& basename,
               string& ext)
{


   // printf("\n_____________________________________________________________");

   string c_env;

   string file;

   file=full_file;

   unsigned int pos1,pos2;


   ////////////////////////////////////search for the path
   /*                                  this probably will not work on win32
    *                                  \ and : need to be handled - - ugh
    */

   string filesep="\\";
#ifdef _BASE_FILE_DEBUG_
   printf("\n-------------------------------------");
   printf("\nFull file <%s>",full_file.c_str());
   printf("\n     path <%s>",path.c_str());
   printf("\n basename <%s>",basename.c_str());
   printf("\n      ext <%s>",ext.c_str());
   printf("\n-------------------------------------");
#endif

   pos1=0; 
   pos2 = file.find_last_of(filesep);
 
   if (pos2==string::npos)
   {
      path="";
       
   }
   else
   {
      path=file.substr(0,pos2+1);

      if (pos2+1<=file.length())
      {
         file=file.substr(pos2+1,file.length());
      }
      else
      {
         printf("\nfileparts is confused");
      }
   }

#ifdef _BASE_FILE_DEBUG_
   printf("\nString \"%s\" found at pos %d",filesep.c_str(),pos2);

   ////////////////////////////////////search for an extension

   printf("\n-------------------------------------");
   printf("\nFull file <%s>",full_file.c_str());
   printf("\n     path <%s>",path.c_str());
   printf("\n basename <%s>",basename.c_str());
   printf("\n      ext <%s>",ext.c_str());
   printf("\n-------------------------------------");
#endif

   pos1=0; 
   pos2 = file.rfind (".",file.length());

   if (pos2==string::npos)
   {
      ext="";
       
   }
   else
   {

      basename =file.substr(pos1,pos2);

      if (pos2+1<=file.length())
      {
         ext=file.substr(pos2+1,file.length());
      }
      else
      {
         printf("\nfileparts is confused");
         exit(1);
      }
   }

#ifdef _BASE_FILE_DEBUG_
   printf("\n-------------------------------------");
   printf("\nFull file <%s>",full_file.c_str());
   printf("\n     path <%s>",path.c_str());
   printf("\n basename <%s>",basename.c_str());
   printf("\n      ext <%s>",ext.c_str());
   printf("\n-------------------------------------");
#endif
   // c_env is a pointer in the enviroment!!
   // it is not malloc'ed or free'd  
   // c_env=getenv(DIR_ENIVRONMENT_VARIABLE);


 
   //printf("\n_____________________________________________________________");


}




void append_extension(string& my_name, const string& new_extension)
{

   unsigned int pos;
   // search and remove . in the filename
   pos = my_name.rfind (".",my_name.length());
   if ((pos != string::npos) && (pos > 0))
   {
      my_name[pos]='_';
      // cout << "extension found" << endl;
   }

   my_name.append(new_extension);


   return;

}
 
void replace_extension(string& my_name, const string& new_extension)
{

   unsigned int pos;
   string my_ext;


   my_ext.assign(new_extension);

   // search and remove . in the filename
   pos = my_name.rfind (".",my_name.length());
   if ((pos != string::npos) && (pos > 0))
   {
      my_name=my_name.substr(0,pos);	 
   }


// if extension does not have a dot, add one
   pos = my_ext.find (".");      

   if ((pos == string::npos))
   {
      my_ext="."+my_ext;
   }

   my_name.append(my_ext);


   return;

}
 
void env2string(const char var_name[], string& my_env)
{

   char * c_env;

   // c_env is a pointer in the enviroment!!
   // it is not malloc'ed or free'd  
  
   c_env=getenv(var_name);

   my_env.assign(c_env);

   return;


}

void env2string(const string& var_name, string& my_env)
{

   env2string(var_name.c_str(),my_env);
   return;

}

void base_file::write(const void* source, size_t element_size, size_t num_elements)
{
   (*write_function)(source,element_size,num_elements,fp);
}
