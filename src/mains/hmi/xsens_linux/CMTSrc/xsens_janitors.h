/*! \file
	\brief	Contains the Janitor class-interfaces and implementations

	This file contains a number of janitor classes. These classes can be used to perform
	simple actions upon leaving scope, such as deleting an object.
	This greatly simplifies exit code. Functions that have lots of exit points can benefit
	greatly from janitors.
	
	Each janitor is named after its main functionality, eg Restore, Free, Delete...

	\section FileCopyright Copyright Notice 
	Copyright (C) Xsens Technologies B.V., 2006.  All rights reserved.

	This source code is intended for use only by Xsens Technologies BV and
	those that have explicit written permission to use it from
	Xsens Technologies BV.

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.
*/

#ifndef _JANITORS_H_2006_05_01
#define _JANITORS_H_2006_05_01

// required for older gnu c++ compiler versions due to difference in attribute declarations
#if defined(__GNUC__) && !defined(HAVE_CDECL)
#   define __cdecl __attribute__((cdecl))
#   define __stdcall __attribute__((stdcall))
#endif

namespace xsens {

//////////////////////////////////////////////////////////////////////////////////////////
/*! \brief Value restoring janitor class

	This class can be used to make sure that the value that is in the variable at the time
	the janitor is created will be in it again when the janitor leaves scope.
*/
template <class T>
class JanitorRestore {
private:
	T& m_control;
	T  m_value;
	bool m_enabled;
public:
	JanitorRestore<T>(T& control, bool enabl = true) :
		m_control(control), m_value(control), m_enabled(enabl) {}
	~JanitorRestore<T>()
	{
		if (m_enabled)
			m_control = m_value;
	}
	
	void disable(void)
		{ m_enabled = false; }
	
	void enable(void)
		{ m_enabled = true; }
};

//////////////////////////////////////////////////////////////////////////////////////////
/*! \brief Memory releasing janitor class

	This class can be used to make sure that the associated pointer is freed when the
	janitor leaves scope.
*/
template <class T>
class JanitorFree {
private:
	T* m_control;
	bool m_enabled;
public:
	JanitorFree<T>(T* control, bool enabl = true) :
		m_control(control), m_enabled(enabl) {}
	~JanitorFree<T>()
	{
		if (m_enabled)
			free(m_control);
	}
	
	void disable(void)
		{ m_enabled = false; }
	
	void enable(void)
		{ m_enabled = true; }
};

//////////////////////////////////////////////////////////////////////////////////////////
/*! \brief Memory releasing janitor class

	This class can be used to make sure that the associated object is deleted when the
	janitor leaves scope.
*/
template <class T>
class JanitorDelete {
private:
	T* m_control;
	bool m_enabled;
public:
	JanitorDelete<T>(T* control, bool enabl = true) :
		m_control(control), m_enabled(enabl) {}
	~JanitorDelete<T>()
	{
		if (m_enabled)
			delete m_control;
	}
	
	void disable(void)
		{ m_enabled = false; }
	
	void enable(void)
		{ m_enabled = true; }
};

//////////////////////////////////////////////////////////////////////////////////////////
/*! \brief Memory releasing janitor class

	This class can be used to make sure that the associated object is deleted when the
	janitor leaves scope.
*/
template <class T>
class JanitorDeleteArray {
private:
	T* m_control;
	bool m_enabled;
public:
	JanitorDeleteArray<T>(T* control, bool enabl = true) :
		m_control(control), m_enabled(enabl) {}
	~JanitorDeleteArray<T>()
	{
		if (m_enabled)
			delete[] m_control;
	}
	
	void disable(void)
		{ m_enabled = false; }
	
	void enable(void)
		{ m_enabled = true; }
};

//////////////////////////////////////////////////////////////////////////////////////////
/*! \brief Class function calling janitor class

	This class can be used to make sure that the given class function is called when the
	janitor leaves scope.
*/
template <class T, typename R = void>
class JanitorClassFunc {
public:
	typedef R (T::*t_func_JanitorClasssFunc)(void);
private:
	T& m_control;
	t_func_JanitorClasssFunc m_funcJCF;
	bool m_enabled;
public:
	
	JanitorClassFunc<T,R>(T& control, t_func_JanitorClasssFunc func, bool enabl = true) :
		m_control(control), m_funcJCF(func), m_enabled(enabl)
	{
	}
	~JanitorClassFunc<T,R>()
	{
		if (m_enabled)
			(m_control.*m_funcJCF)();
	}
	
	void disable(void)
		{ m_enabled = false; }
	
	void enable(void)
		{ m_enabled = true; }
};

//////////////////////////////////////////////////////////////////////////////////////////
/*! \brief Function calling janitor class for function with 0 parameters

	This class can be used to make sure that the given function is called on the given
	object when the janitor leaves scope. Take care that the object is not of a type that
	is destroyed before the function unrolling begins.
*/
template <typename ResultType = void>
class JanitorFunc0 {
public:
	typedef ResultType (__cdecl * t_func_JanitorFunc)(void);
private:
	t_func_JanitorFunc m_funcJF;
	bool m_enabled;
public:
	
	JanitorFunc0<ResultType>(t_func_JanitorFunc func, bool enabl = true) :
		m_funcJF(func), m_enabled(enabl) {}
	~JanitorFunc0<ResultType>()
	{
		if (m_enabled)
			(*m_funcJF)();
	}
	
	void disable(void)
		{ m_enabled = false; }
	
	void enable(void)
		{ m_enabled = true; }
};

//////////////////////////////////////////////////////////////////////////////////////////
/*! \brief Function calling janitor class for function with 1 parameter

	This class can be used to make sure that the given function is called on the given
	object when the janitor leaves scope. Take care that the object is not of a type that
	is destroyed before the function unrolling begins.
*/
template <class ParamType, typename ResultType = void>
class JanitorFunc1 {
public:
	typedef ResultType (__cdecl * t_func_JanitorFunc)(ParamType);
private:
	ParamType& m_control;
	t_func_JanitorFunc m_funcJF;
	bool m_enabled;
public:
	
	JanitorFunc1<ParamType,ResultType>(t_func_JanitorFunc func, ParamType& control, bool enabl = true) :
		m_funcJF(func), m_control(control), m_enabled(enabl) {}
	~JanitorFunc1<ParamType,ResultType>()
	{
		if (m_enabled)
			(*m_funcJF)(m_control);
	}
	
	void disable(void)
		{ m_enabled = false; }
	
	void enable(void)
		{ m_enabled = true; }
};
#define JanitorFunc	JanitorFunc1

//////////////////////////////////////////////////////////////////////////////////////////
/*! \brief Function calling janitor class for function with 2 parameters

	This class can be used to make sure that the given function is called on the given
	object when the janitor leaves scope. Take care that the object is not of a type that
	is destroyed before the function unrolling begins.
*/
template <class Param1Type, class Param2Type, typename ResultType = void>
class JanitorFunc2 {
public:
	typedef ResultType (__cdecl * t_func_JanitorFunc)(Param1Type,Param2Type);
private:
	Param1Type& m_control1;
	Param2Type& m_control2;
	t_func_JanitorFunc m_funcJF;
	bool m_enabled;
public:
	
	JanitorFunc2<Param1Type,Param2Type,ResultType>(t_func_JanitorFunc func, Param1Type& control1, Param2Type& control2, bool enabl = true) :
		m_funcJF(func), m_control1(control1), m_control1(control2), m_enabled(enabl) {}
	~JanitorFunc2<Param1Type,Param2Type,ResultType>()
	{
		if (m_enabled)
			(*m_funcJF)(m_control1,m_control2);
	}
	
	void disable(void)
		{ m_enabled = false; }
	
	void enable(void)
		{ m_enabled = true; }
};

//////////////////////////////////////////////////////////////////////////////////////////
/*! \brief Log / printf-like function calling janitor class

	This class can be used to make sure that the given printf-like function is called with the
	supplied parameter when the janitor leaves scope. Take care that the object is not of a type that
	is destroyed before the function unrolling begins.
*/
template <class T, class C, typename R = void>
class JanitorLogFunc {
public:
	typedef R (__cdecl * t_func_JanitorLogFunc)(const char*,...);
private:
	const char* m_str;
	T& m_control;
	t_func_JanitorLogFunc m_funcJF;
	bool m_enabled;
public:
	
	JanitorLogFunc<T,C,R>(t_func_JanitorLogFunc func, const char* str, T& control, bool enable = true) :
		m_funcJF(func), m_str(str), m_control(control), m_enabled(enable) {}
	~JanitorLogFunc<T,C,R>()
	{
		if (m_enabled)
			(*m_funcJF)(m_str,(C) m_control);
	}
	
	void disable(void)
		{ m_enabled = false; }
	
	void enable(void)
		{ m_enabled = true; }
};

//////////////////////////////////////////////////////////////////////////////////////////
/*! \brief Function calling janitor class

	This class can be used to make sure that the given function is called on the given
	object when the janitor leaves scope. Take care that the object is not of a type that
	is destroyed before the function unrolling begins.
*/
template <class ParamType, typename ResultType = void>
class JanitorFuncStdCall {
public:
	typedef ResultType (__stdcall * t_func_JanitorFuncStdCall)(ParamType);
private:
	ParamType& m_control;
	t_func_JanitorFuncStdCall m_funcJFSC;
	bool m_enabled;
public:
	
	JanitorFuncStdCall<ParamType,ResultType>(t_func_JanitorFuncStdCall func, ParamType& control, bool enabl = true) :
		m_funcJFSC(func), m_control(control), m_enabled(enabl) {}
	~JanitorFuncStdCall<ParamType,ResultType>()
	{
		if (m_enabled)
			(*m_funcJFSC)(m_control);
	}
	
	void disable(void)
		{ m_enabled = false; }
	
	void enable(void)
		{ m_enabled = true; }
};

//////////////////////////////////////////////////////////////////////////////////////////
/*! \brief Value restoring janitor class

	This class can be used to make sure that the value that is in the variable at the time
	the janitor is created will be in it again when the janitor leaves scope.
*/
template <class T>
class JanitorSet {
private:
	T& m_control;
	T  m_value;
	bool m_enabled;
public:
	JanitorSet<T>(T& control, const T& val, bool enabl = true) :
		m_control(control), m_value(val), m_enabled(enabl) {}
	~JanitorSet<T>()
	{
		if (m_enabled)
			m_control = m_value;
	}
	
	void disable(void)
		{ m_enabled = false; }
	
	void enable(void)
		{ m_enabled = true; }
};

}	// end of xsens namespace

#endif	// _JANITORS_H_2006_05_01
