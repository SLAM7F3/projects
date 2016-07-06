 /*
  AbstractCallbackObject.h was taken from Vadim Grinshpun's
  AbstractThreadedCallback class.

  See CallbackObject.h for usage details.


 */
#ifndef ABSTRACTCALLBACKOBJECT_H
#define ABSTRACTCALLBACKOBJECT_H


//=========================================

class AbstractCallbackObject
{
 public:
    AbstractCallbackObject();
    AbstractCallbackObject( void *p_param );
    virtual ~AbstractCallbackObject() {};

    static void *Callback( void * p_CallbackInstance );
    static void VoidCallback( void * p_CallbackInstance );
 
 protected:
    virtual void* callClientFunction()=0;
    void * m_parameter;
};


//=======================================

inline AbstractCallbackObject::AbstractCallbackObject():
    m_parameter( 0 )
{};

//-----------------

inline AbstractCallbackObject::AbstractCallbackObject( void *p_param ):
    m_parameter( p_param )
{};

//------------------

inline void * AbstractCallbackObject::Callback( void *p_CallbackInstance )
{
    AbstractCallbackObject * callbackInstance =
        static_cast<AbstractCallbackObject *>( p_CallbackInstance );

    void *result = callbackInstance->callClientFunction();

    return result;
};

//-----------------

inline void AbstractCallbackObject::VoidCallback( void *p_CallbackInstance )
{
    AbstractCallbackObject * callbackInstance =
        static_cast<AbstractCallbackObject *>( p_CallbackInstance );

    callbackInstance->callClientFunction();
};


#endif
