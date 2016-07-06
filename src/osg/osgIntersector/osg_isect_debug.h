/* -*-c++-*- osgIntersect - Copyright (C) 2005 IGEOSS (www.igeoss.com)
 *                                             frederic.marmond@igeoss.com
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
#ifdef OSGI_DEBUG_COLOR
#ifndef DBGCOLOR_FILE
#define DBGCOLOR_FILE "[0;37;44m"
#endif
#else
#define DBGCOLOR_FILE ""
#endif

#ifndef DEBUG_H
#define DEBUG_H


#define DEBUG
#include <iostream>

#ifdef OSGI_DEBUG_COLOR
#define DBGCOLOR_NORM "[0;0;0m"
#define DBGCOLOR_TRC  "[1;37;44m"
#define DBGCOLOR_MSG  "[0;0;32m"
#define DBGCOLOR_WARN "[0;0;33m"
#define DBGCOLOR_ERR  "[0;0;31m"
#else
#define DBGCOLOR_NORM ""
#define DBGCOLOR_TRC  ""
#define DBGCOLOR_MSG  ""
#define DBGCOLOR_WARN ""
#define DBGCOLOR_ERR  ""
#endif

#ifdef OSGI_DEBUG

#define DBG_LOG(msg) {std::cerr <<DBGCOLOR_FILE<< __FILE__ <<DBGCOLOR_NORM<< "(" << __LINE__ << "):     "<<msg << std::endl ;}

#define DBG_TRC(msg) {DBG_LOG(DBGCOLOR_TRC<<"MSG"<<DBGCOLOR_NORM<<":    "<<msg);}
#define DBG_MSG(msg) {DBG_LOG(DBGCOLOR_MSG<<"MSG"<<DBGCOLOR_NORM<<":    "<<msg);}
#define DBG_WAR(msg) {DBG_LOG(DBGCOLOR_WARN<<"WAR"<<DBGCOLOR_NORM<<":   "<<msg);}
#define DBG_ERR(msg) {DBG_LOG(DBGCOLOR_ERR<<"ERR"<<DBGCOLOR_NORM<<":    "<<msg);exit(1);}

#define DBG_PTR(ptr) {if (!ptr) DBG_ERR("Pointer not valid: "<<#ptr<<"="<<(void*)ptr);}
#define DBG_ASSNO(var1,cond,var2)         \
	        {if (!((var1) cond (var2)))                              \
			                DBG_ERR(#var1 << " " << #cond << " " << #var2 << " FAILED (sorry, I can't show values..." ) ;}
#define DBG_ASS(var1,cond,var2)         \
	        {if (!((var1) cond (var2)))                              \
			                DBG_ERR(#var1 << " " << #cond << " " << #var2 << " FAILED (" << var1 <<" "<< #cond<<" " << var2 << ")" ) ;}


#else

#define DBG_LOG(msg) {}

#define DBG_TRC(msg) {}
#define DBG_MSG(msg) {}
#define DBG_WAR(msg) {}
#define DBG_ERR(msg) {}

#define DBG_PTR(ptr) {}
#define DBG_ASSNO(var1,cond,var2) {}        
#define DBG_ASS(var1,cond,var2) {}


#endif //OSGI_DEBUG

#endif //DEBUG_H
