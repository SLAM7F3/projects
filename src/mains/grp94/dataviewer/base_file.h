/********************************************************************
 *
 *
 * Name: base_file.h
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

#ifndef BASE_FILE
#define BASE_FILE

#include <cstdio>
#include <cstdlib>                               //for exit function

#include <string>                                 // STL string class

#define BASE_FILE_VERSION 0.6

//#define _BASE_FILE_DEBUG_

#define DEFAULT_FILE_EXTENSION "asc"
#define DIR_ENIVRONMENT_VARIABLE "LUCY_DIR"
#define DEFAULT_PATH_LENGTH 5000

using namespace std;

enum endian_t{little,big};

class base_file
{
  public:
   FILE *fp;
   string filename;

  private:
   int file_is_open;
   int file_is_initialized;

   int read_result;
  protected:
   int little_endian;
   void (*read_function) 
      (void* dest ,size_t element_size,size_t num_elements,FILE* source);
   void (*write_function)	
      (const void* source, size_t element_size, size_t num_elements, 
       FILE* dest);

  public:
   base_file(void);
   base_file(const char *filename_);
   base_file(const string& filename_);

   ~base_file(void);

   int	 is_little_endian();
   virtual void set_endian(endian_t e);
   void write(const void* source, size_t element_size, size_t num_elements);

   inline void read_binary(
      void* dest ,size_t element_size,size_t num_elements,FILE* source)
      {(*read_function)(dest,element_size,num_elements,source);}

   inline void write_binary(
      const void* source, size_t element_size, size_t num_elements, 
      FILE* dest){(*write_function)(source,element_size,num_elements,dest);}

   void move_to(long offset);
   void skip_forward(long offset);
   long get_current_pos(void);
   void move_to_beginning(void);
   void move_to_end(void);
        
   long file_size(void);   // returns file size in bytes
   void close(void);

   int is_eof(void);

//        void set_filename(const char[]);
   void set_filename(const char* filename_);

   const char * get_filename(void) const 
      { 
         return filename.c_str();
      }
   //id get_filename_no_path_no_extension(char * filename_);

//  void initialize(const char *full_name_);
//  void initialize(const char *dir_name_, const char *base_name_,
//			 const char *ext_name_);

   int file_exists();
   void error(char error_text[]) const;
   void warning(char error_text[]) const;
   int check_io_error(void) const;

       
   void open(const char mode[]);

   void open_output_does_not_exist(void) ;
   void open_output_overwrite(void);
   void open_output_overwrite_binary(void);
   void open_output_append(void);
   void open_input_read_only(void);
   void open_input_read_only_binary(void);
   float read_single_float_at_BOL(void);
   void fileparts(const string& full_file, 
                  string& path, 
                  string& basename,
                  string& ext);
   void get_line(void);

//int write_data_to_output_file(void);

// returns bytes free 
   long free_disk_space(void) const;

    
};




void fileparts(const string& fullfile, 
               string& path, 
               string& name,
               string& ext);
 
void append_extension(string& outfile_name, const string& new_extension);
void replace_extension(string& my_name, const string& new_extension);

void env2string(const char var_str[], string& my_env);
void env2string(const string& var_str, string& my_env);

#endif
