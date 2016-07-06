// ========================================================================
// InputFile member function definitions
// ========================================================================
// Last updated on 1/7/08; 1/8/08; 1/11/08
// ========================================================================

#include <iostream>
#include "ffmpeg/InputFile.h"

using std::cout;
using std::endl;

extern "C"
{
   void av_register_all(void);
   int av_open_input_file(
      AVFormatContext **ic_ptr, const char *filename,
      AVInputFormat *fmt,int buf_size,AVFormatParameters *ap);
   void av_close_input_file(AVFormatContext *s);
}

using namespace FFWrapper;


// ========================================================================
InputFile::InputFile() throw()
   : m_Context(0)
{
//   cout << "inside void InputFile constructor" << endl;
   av_register_all();
}
    
InputFile::InputFile(
   const char *p_Filename,
   AVInputFormat *p_ForceFormat,
   int p_BufferSize,
   AVFormatParameters *p_AdditionalParameters,
   int *p_Error) throw()
{
//   cout << "inside nontrivial InputFile constructor" << endl;

   av_register_all();
   int err = av_open_input_file(
      &m_Context, p_Filename, p_ForceFormat,
      p_BufferSize, p_AdditionalParameters);
   if (p_Error)
   {
      *p_Error = err;
   }
}

InputFile::~InputFile() throw()
{
   close();
}

int InputFile::open_nothrow(
   const char *p_Filename,
   AVInputFormat *p_ForceFormat,
   int p_BufferSize,
   AVFormatParameters *p_AdditionalParameters) throw()
{
   close();

   int ret = av_open_input_file(
      &m_Context, p_Filename, p_ForceFormat,
      p_BufferSize, p_AdditionalParameters);

   return ret;
}

void InputFile::close() throw()
{
   if(m_Context)
   {
      av_close_input_file(m_Context);
      m_Context = 0;
   }
}
