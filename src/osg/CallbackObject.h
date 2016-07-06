#ifndef _CALLBACKOBJECT_H
#define _CALLBACKOBJECT_H

#include "AbstractCallbackObject.h"

template<class T>
class CallbackObject : public AbstractCallbackObject
{
  public:
   typedef void * (T::*FuncPointerNoArgs)();
   typedef void * (T::*FuncPointerOneArg)(void *);
   typedef void (T::*FuncPointerNoArgsNoReturn)();
   typedef void (T::*FuncPointerOneArgNoReturn)(void *);

   inline CallbackObject(const CallbackObject & t):

      AbstractCallbackObject(), fp_NoArgs(t.fp_NoArgs), 
      fp_OneArg(t.fp_OneArg),
      fp_NoArgsNoRet(t.fp_NoArgsNoRet), fp_OneArgNoRet(t.fp_OneArgNoRet),
      m_instance(t,m_instance) {};
    
   inline CallbackObject( T * p_instance, FuncPointerNoArgs p_F):

      AbstractCallbackObject(), fp_NoArgs(p_F), fp_OneArg(0),
      fp_NoArgsNoRet(0), fp_OneArgNoRet(0), m_instance(p_instance)
      {};

   inline CallbackObject( T * p_instance, FuncPointerOneArg p_F, 
                          void *p_param):

      AbstractCallbackObject(p_param), fp_NoArgs(0), fp_OneArg(p_F),
      fp_NoArgsNoRet(0), fp_OneArgNoRet(0), m_instance(p_instance)
      {};


   inline CallbackObject( T * p_instance, FuncPointerNoArgsNoReturn p_F):

      AbstractCallbackObject(), fp_NoArgs(0), fp_OneArg(0),
      fp_NoArgsNoRet(p_F), fp_OneArgNoRet(0), m_instance(p_instance)
      {};


   inline CallbackObject( T * p_instance, FuncPointerOneArgNoReturn p_F,
                          void *p_param):

      AbstractCallbackObject(p_param), fp_NoArgs(0), fp_OneArg(0),
      fp_NoArgsNoRet(0), fp_OneArgNoRet(p_F), m_instance(p_instance)
      {};

   ~CallbackObject() {};

  protected:

   inline void *callClientFunction()
      {
         if (fp_NoArgs != 0)
            return (m_instance->*fp_NoArgs)();
         else if (fp_OneArg != 0)
            return (m_instance->*fp_OneArg)(m_parameter);
         else if (fp_NoArgsNoRet != 0)
            (m_instance->*fp_NoArgsNoRet)();
         else if (fp_OneArgNoRet != 0)
            (m_instance->*fp_OneArgNoRet)(m_parameter);
            
         return 0;
      };


   FuncPointerNoArgs fp_NoArgs;
   FuncPointerOneArg fp_OneArg;
   FuncPointerNoArgsNoReturn fp_NoArgsNoRet;
   FuncPointerOneArgNoReturn fp_OneArgNoRet;
   T * m_instance;

  private:
   inline CallbackObject():
      AbstractCallbackObject(), fp_NoArgs(0), fp_OneArg(0),
      fp_NoArgsNoRet(0), fp_OneArgNoRet(0), m_instance(0)
      {};
};


#endif
