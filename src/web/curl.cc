// ==========================================================================
// Curl class member function definitions
// ==========================================================================
// Last modified on 2/16/07
// ==========================================================================

#include "web/curl.h"
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cerr;
using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ==========================================================================

// custom data type to be used in callback function

class XferInfo {

  private:
   int bytesTransferred_ ;
   int invocations_ ;

  protected:
   // empty

  public:
   XferInfo(){

      reset() ;

   } // ctor

   /// reset counters
   void reset(){

      bytesTransferred_ = 0 ;
      invocations_ = 0 ;

      return ;

   } // reset()

   /// add the number of bytes transferred in this call
   void add( int more ){

      bytesTransferred_ += more ;
      ++invocations_ ;

      return ;

   } // add()	

   /// get the amount of data transferred, in bytes
   int getBytesTransferred() const {
      return( bytesTransferred_ ) ;
   } // getBytesTransferred()

   /// get the number of times add() has been called
   int getTimesCalled(){
      return( invocations_ ) ;
   } // getTimesCalled()
} ;



// C++ programmers, take note of the "extern" call for C-style linkage
extern "C"
size_t showSize( void *source , size_t size , size_t nmemb , void *userData )
{
   cout << "inside showSize(), userData = " << userData << endl;

   // this function may be called any number of times for even a single
   // transfer; be sure to write it accordingly.

   // source is the actual data fetched by libcURL; cast it to whatever
   // type you need (usually char*).  It has NO terminating NULL byte.

   // we don't touch the data here, so the cast is commented out
   // const char* data = static_cast< const char* >( source ) ;

   // userData is called "stream" in the docs, which is misleading:
   // that parameter can be _any_ data type, not necessarily a FILE*
   // Here, we use it to save state between calls to this function
   // and track number of times this callback is invoked.
   XferInfo* info = static_cast< XferInfo* >( userData ) ;

   const int bufferSize = size * nmemb ;

   cout << "size = " << size << " nmemb = " << nmemb << endl;
   cout << '\t' << "showSize() called: " << bufferSize 
        << " bytes passed" << endl ;

   // ... pretend real data processing on *source happens here ...

   info->add( bufferSize ) ;

        
   /*
     return some number less than bufferSize to indicate an
     error (xfer abort)
	
     nicer code would also set a status var (in userData) for the
     calling function
   */

   return( bufferSize ) ;

} // showSize()

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void curl::allocate_member_objects()
{

// Create a context, sometimes known as a handle.  Think of it as a
// lookup table, or a source of config data:

   ctx_ptr = curl_easy_init() ;

}		       

void curl::initialize_member_objects()
{
}

curl::curl()
{
   allocate_member_objects();
   initialize_member_objects();
}

curl::curl(string url_name)
{
   allocate_member_objects();
   initialize_member_objects();

   set_url(url_name);
}

curl::~curl()
{
   curl_easy_cleanup( ctx_ptr );
   curl_global_cleanup();

   outstream.close();
   fclose(file_ptr);
}

// ---------------------------------------------------------------------
void curl::docopy(const curl& p)
{
}

// Overload = operator:

curl& curl::operator= (const curl& c)
{
   if (this==&c) return *this;
   docopy(c);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const curl& c)
{
   outstream << endl;
   return(outstream);
}

// =====================================================================
// =====================================================================

void curl::configure_handle()
{

// global libcURL init:

   curl_global_init( CURL_GLOBAL_ALL ) ;
   
// Configure the handle:

   // handy for debugging: see *everything* that goes on
   // curl_easy_setopt( ctx , CURLOPT_VERBOSE, OPTION_TRUE ) ;

   // target url:

   curl_easy_setopt( ctx_ptr , CURLOPT_URL,  url_name.c_str() ) ;

   // no progress bar:
   curl_easy_setopt( ctx_ptr , CURLOPT_NOPROGRESS , OPTION_TRUE ) ;

   // send response headers to stderr:

   curl_easy_setopt( ctx_ptr , CURLOPT_WRITEHEADER , stdout ) ;
}

// ---------------------------------------------------------------------
size_t curl::write_data(
   void* source,size_t size, size_t nmemb, void* userData)
{
   cout << "inside curl::write_data, userData = " << userData << endl;

   // this member function may be called any number of times for even
   // a single transfer; be sure to write it accordingly.

   // source is the actual data fetched by libcURL; cast it to whatever
   // type you need (usually char*).  It has NO terminating NULL byte.

   // we don't touch the data here, so the cast is commented out
   // const char* data = static_cast< const char* >( source ) ;

   // userData is called "stream" in the docs, which is misleading:
   // that parameter can be _any_ data type, not necessarily a FILE*
   // Here, we use it to save state between calls to this function
   // and track number of times this callback is invoked.

   XferInfo* info = static_cast< XferInfo* >( userData ) ;

   const int bufferSize = size * nmemb ;

   cout << "size = " << size << " nmemb = " << nmemb << endl;
   cout << '\t' << "showSize() called: " << bufferSize 
        << " bytes passed" << endl ;

   // ... pretend real data processing on *source happens here ...

   info->add( bufferSize ) ;

   /*
     return some number less than bufferSize to indicate an
     error (xfer abort)
	
     nicer code would also set a status var (in userData) for the
     calling function
   */

   return( bufferSize ) ;
} 

// ---------------------------------------------------------------------
void curl::retrieve_URL_contents()
{
//   cout << "inside curl::retrieve_URL_contents()" << endl;
   
   tmp_filename=filefunc::generate_tmpfilename();
   file_ptr=fopen(tmp_filename.c_str(),"w");

   curl_easy_setopt( ctx_ptr , CURLOPT_WRITEDATA , file_ptr ) ;
//   curl_easy_setopt( ctx_ptr , CURLOPT_WRITEFUNCTION , showSize ) ;
//   curl_easy_setopt( ctx_ptr , CURLOPT_WRITEDATA , curl::write_data ) ;

   cout << "Before executing URL call" << endl;
   const CURLcode rc = curl_easy_perform(ctx_ptr);
   cout << "After executing URL call" << endl;

   if (rc != CURLE_OK)
   {
      cerr << "Error from cURL: " << curl_easy_strerror( rc ) << endl ;
   }
   else
   {
//      long response_code;
//      if (CURLE_OK == curl_easy_getinfo(
//         ctx_ptr,CURLINFO_HTTP_CODE,&response_code))
//      {
//         cout << "Response code:  " << statLong << endl;
//      }

      char* content_type_cstr=NULL;
      string content_type;
      if (CURLE_OK == curl_easy_getinfo(
         ctx_ptr,CURLINFO_CONTENT_TYPE,&content_type_cstr))
      {
         content_type=content_type_cstr;
         cout << "Content type = " << content_type << endl;
         rename_tmp_image(content_type);
      }

      double download_size;
      if (CURLE_OK == curl_easy_getinfo( 
         ctx_ptr,CURLINFO_SIZE_DOWNLOAD,&download_size))
      {
         cout << "Download size:  " << download_size << " bytes" << endl;
      }

      double download_speed;
      if (CURLE_OK == curl_easy_getinfo(
         ctx_ptr,CURLINFO_SPEED_DOWNLOAD,&download_speed))
      {
         cout << "Download speed: " << download_speed << " bytes/sec" << endl;
      }

      double download_time=download_size/download_speed;
      cout << "Download time = " << download_time << endl;

   } // rc != CURLE_OK conditional
}

// ---------------------------------------------------------------------
void curl::rename_tmp_image(string content_type)
{
   string prefix=stringfunc::prefix(content_type,"/");
   if (prefix=="image")
   {
      string suffix=stringfunc::suffix(content_type,"/");
      string new_tmp_filename="/tmp/"+prefix+"."+suffix;
//   cout << "new_tmp_filename = " << new_tmp_filename << endl;
      string unix_command="mv "+tmp_filename+" "+new_tmp_filename;
//   cout << "unix_command = " << unix_command << endl;
      sysfunc::unix_command(unix_command);
      tmp_filename=new_tmp_filename;
   }
}

