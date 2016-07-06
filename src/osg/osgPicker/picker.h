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
#if defined(_WIN32) || defined(WIN32)
#ifdef OSGINTERSECT_EXPORTS
#define OSGINTERSECT __declspec(dllexport)
#else
#define OSGINTERSECT __declspec(dllimport)
#endif
#else
#define OSGINTERSECT
#endif

#include "osg/osgIntersector/osg_isect_debug.h"

