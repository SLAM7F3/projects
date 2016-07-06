#ifndef _ABSTRACTOSGCALLBACK_H
#define _ABSTRACTOSGCALLBACK_H

#include <osg/Node>
#include <osg/NodeCallback>
#include <osg/NodeVisitor>

template<class T>
class AbstractOSGCallback : public osg::NodeCallback
{
  public:
   typedef void * (T::*FuncPointerNoArgs)();
   typedef void * (T::*FuncPointerOneArg)(void *);
   typedef void (T::*FuncPointerNoArgsNoReturn)();
   typedef void (T::*FuncPointerOneArgNoReturn)(void *);

   /* copy constructor */
   inline AbstractOSGCallback(const AbstractOSGCallback & t):
      m_parameter(), fp_NoArgs(t.fp_NoArgs), fp_OneArg(t.fp_OneArg),
      fp_NoArgsNoRet(t.fp_NoArgsNoRet), fp_OneArgNoRet(t.fp_OneArgNoRet),
      m_instance(t,m_instance) {};
    
   /* overloaded constructor for using a member method of type as a
      Callback: void * method() */
   inline AbstractOSGCallback( T * p_instance, FuncPointerNoArgs p_F):
      m_parameter(), fp_NoArgs(p_F), fp_OneArg(0),
      fp_NoArgsNoRet(0), fp_OneArgNoRet(0), m_instance(p_instance)
      {};

   /* overloaded constructor for using a member method of type as a
      Callback: void * method(void *) */
   inline AbstractOSGCallback( 
      T * p_instance, FuncPointerOneArg p_F, void *p_param):
      m_parameter(p_param), fp_NoArgs(0), fp_OneArg(p_F),
      fp_NoArgsNoRet(0), fp_OneArgNoRet(0), m_instance(p_instance)
      {};

   /* overloaded constructor for using a member method of type as a
      Callback: void method() */
   inline AbstractOSGCallback( T * p_instance, FuncPointerNoArgsNoReturn p_F):
      m_parameter(), fp_NoArgs(0), fp_OneArg(0),
      fp_NoArgsNoRet(p_F), fp_OneArgNoRet(0), m_instance(p_instance)
      {};

   /* overloaded constructor for using a member method of type as a
      Callback: void method(void *) */
   inline AbstractOSGCallback( T * p_instance, FuncPointerOneArgNoReturn p_F,void *p_param):

      m_parameter(p_param), fp_NoArgs(0), fp_OneArg(0),
      fp_NoArgsNoRet(0), fp_OneArgNoRet(p_F), m_instance(p_instance)
      {};


   /* callback used by NodeVisitor, calls the member method of
      m_instance which was registered via the constructor */
   virtual void operator()(osg::Node * p_node, osg::NodeVisitor * p_nv)
     {
//       void* result =
        callClientFunction();
       traverse( p_node, p_nv );
     }
   
  protected:

   ~AbstractOSGCallback() {};

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


   void * m_parameter;
   FuncPointerNoArgs fp_NoArgs;
   FuncPointerOneArg fp_OneArg;
   FuncPointerNoArgsNoReturn fp_NoArgsNoRet;
   FuncPointerOneArgNoReturn fp_OneArgNoRet;
   T * m_instance;


  private:

   inline AbstractOSGCallback():
      m_parameter(), fp_NoArgs(0), fp_OneArg(0),
      fp_NoArgsNoRet(0), fp_OneArgNoRet(0), m_instance(0)
      {};
};


#endif
