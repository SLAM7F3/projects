// ==========================================================================
// Header file for curl class 
// ==========================================================================
// Last modified on 2/16/07
// ==========================================================================

#ifndef CURL_H
#define CURL_H

#include <fstream>
#include <iostream>
#include <string>

extern "C" 
{
#include<curl/curl.h>
}

class curl
{

  public:

   enum {
      ERROR_ARGS = 1 ,
      ERROR_CURL_INIT = 2
   };
   
   enum {
      OPTION_FALSE = 0 ,
      OPTION_TRUE = 1
   };

   enum {
      FLAG_DEFAULT = 0 
   };

   curl();
   curl(std::string url_name);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~curl();
   curl& operator= (const curl& c);
   friend std::ostream& operator<< (std::ostream& outstream, const curl& c);

// Set and get member functions:

   void set_url(std::string url_name);

   void configure_handle();
   void retrieve_URL_contents();
   void rename_tmp_image(std::string content_type);

  private: 

   CURL* ctx_ptr;
   std::string url_name,tmp_filename;
   std::ofstream outstream;
   FILE* file_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const curl& c);

   static size_t write_data(
      void* source,size_t size, size_t nmemb, void* userData);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void  curl::set_url(std::string url_name) 
{
   this->url_name=url_name;
}

#endif  // web/curl.h






