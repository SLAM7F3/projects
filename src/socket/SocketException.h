// =======================================================================
// Header file for SocketException class.  Source code comes from
// "Linux socket programming in C++" by Rob Tougher in issue 74 of
// online Linux Gazette, Jan 2002.
// =======================================================================
// Last updated on 3/11/04
// =======================================================================

#ifndef SocketException_class
#define SocketException_class

#include <string>

class SocketException
{
  public:
   SocketException ( std::string s ) : m_s ( s ) {};
   ~SocketException (){};

   std::string description() { return m_s; }

  private:

   std::string m_s;
};

#endif
