// ========================================================================
// ========================================================================
// Last updated on 1/7/08
// ========================================================================

#ifndef INCLUDE_FFWRAPPER_INPUTFILE
#define INCLUDE_FFWRAPPER_INPUTFILE

struct AVFormatContext;
struct AVFormatParameters;
struct AVInputFormat;

namespace FFWrapper
{
   class InputFile
      {
        public:
         // Possible TODO: add throwing versions of methods
        
         InputFile() throw();
         /**
          * p_Error assigned 0 on success, error code on failure.
          * p_Error is ignored if NULL.
          * */
         InputFile(
            const char *p_Filename,
            AVInputFormat* p_ForceFormat,
            int m_BufferSize,
            AVFormatParameters *m_AdditionalParameters,
            int *p_Error) throw();
         ~InputFile() throw();

         /// Return 0 on success, error code on failure
         int open_nothrow(
            const char *p_Filename,
            AVInputFormat *p_ForceFormat,
            int m_BufferSize,
            AVFormatParameters *m_AdditionalParameters) throw();

         void close() throw();

         AVFormatContext *get_context() throw();
         const AVFormatContext *get_context() const throw();

        private:
         /// Copy constructor disabled
         InputFile(const InputFile&);
         /// Assignment operator disabled
         InputFile& operator=(const InputFile&);
        
         AVFormatContext *m_Context;
      };
}

inline AVFormatContext* FFWrapper::InputFile::get_context() throw()
{
   return m_Context;
}

inline const AVFormatContext* FFWrapper::InputFile::get_context() 
   const throw()
{
   return m_Context;
}

#endif
