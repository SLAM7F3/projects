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
#ifndef OSG_GEN_INDEX_FUNCTOR_H
#define OSG_GEN_INDEX_FUNCTOR_H

#include "osg/osgIntersector/intersector.h"


#include <osg/Drawable>
#include <osg/Geometry>
#include <osg/PrimitiveSet>
#include <osg/Version>
using namespace osg;

namespace osgIsect
{

#if (OSG_VERSION_MAJOR >= 0 ) && (OSG_VERSION_MINOR >= 9) && (OSG_VERSION_RELEASE >= 8)
template<class T>
class GenIndexFunctor: public osg::PrimitiveIndexFunctor, public T
#else
template<class T>
class GenIndexFunctor: public osg::Drawable::PrimitiveIndexFunctor, public T
#endif
{
public:

    GLenum               _modeCache;
    std::vector<GLuint>  _indexCache;

    virtual void setVertexArray(unsigned int,const Vec2*)
    {
    }

    virtual void setVertexArray(unsigned int ,const Vec3* )
    {
    }

    virtual void setVertexArray(unsigned int,const Vec4* )
    {
    }

    virtual void begin(GLenum mode)
    {
        _modeCache = mode;
        _indexCache.clear();
    }

    virtual void vertex(unsigned int vert)
    {
        _indexCache.push_back(vert);
    }

    virtual void end()
    {
        if (!_indexCache.empty())
        {
            drawElements(_modeCache,_indexCache.size(),&_indexCache.front());
        }
    }

    virtual void drawArrays(GLenum mode,GLint first,GLsizei count)
    {
    	//DBG_MSG(IGS_TRC,IGS_VLOW,"1 drawArrays(GLenum mode,GLint first,GLsizei count)");
        switch(mode)
        {
            case(GL_TRIANGLES):
            {
                unsigned int pos=first;
                for(GLsizei i=2;i<count;i+=3,pos+=3)
                {
                    this->operator()(pos,pos+1,pos+2);
                }
                break;
            }
            case(GL_TRIANGLE_STRIP):
             {
                unsigned int pos=first;
                for(GLsizei i=2;i<count;++i,++pos)
                {
		    if ((i%2)) this->operator()(pos,pos+2,pos+1);
		    else       this->operator()(pos,pos+1,pos+2);
                }
                break;
            }
            case(GL_QUADS):
            {
                unsigned int pos=first;
                for(GLsizei i=3;i<count;i+=4,pos+=4)
                {
                    this->operator()(pos,pos+1,pos+2);
                    this->operator()(pos,pos+2,pos+3);
                }
                break;
            }
            case(GL_QUAD_STRIP):
            {
                unsigned int pos=first;
                for(GLsizei i=3;i<count;i+=2,pos+=2)
                {
                    this->operator()(pos,pos+1,pos+2);
                    this->operator()(pos+1,pos+3,pos+2);
                }
                break;
            }
            case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
            case(GL_TRIANGLE_FAN):
            {
                unsigned int pos=first+1;
                for(GLsizei i=2;i<count;++i,++pos)
                {
                    this->operator()(first,pos,pos+1);
                }
                break;
            }
            case(GL_POINTS):
            {
//	    	DBG_MSG(IGS_TRC,IGS_VLOW,"GL_POINTS");
                for(int i=first;i<first+count;i++)
                    this->operator()(i);
                break;
            }
            case(GL_LINES):
            {
//	    	DBG_MSG(IGS_TRC,IGS_VLOW,"GL_LINES");
                for(int i=first;i<first+count;i+=2)
                    this->operator()(i,i+1);
                break;
            }
            case(GL_LINE_STRIP):
            {
//	    	DBG_MSG(IGS_TRC,IGS_VLOW,"GL_LINES_STRIP");
                for(int i=first;i<first+count-1;i++)
                    this->operator()(i,i+1);
                break;
	    }

	    case(GL_LINE_LOOP):
            {
//	    	DBG_MSG(IGS_TRC,IGS_VLOW,"GL_LINES_LOOP");
                for( int i=first;i<first+count-1;i++)
                    this->operator()(i,i+1);
		this->operator()(first+count-1,first);
                break;
	    }
            default:
	    	DBG_WAR("Not handled");
                // can't be converted into to triangles.
                break;
        }
    }

    virtual void drawElements(GLenum mode,GLsizei count,const GLubyte* indices)
    {
    	DBG_MSG("2 drawElements(GLenum mode,GLsizei count,const GLubyte* indices)");
        if (indices==0 || count==0) return;

        typedef const GLubyte* IndexPointer;

        switch(mode)
        {
            case(GL_TRIANGLES):
            {
	    	DBG_TRC("not tested/implemented");
                IndexPointer ilast = &indices[count];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr+=3)
                    this->operator()(*iptr,*(iptr+1),*(iptr+2));
                break;
            }
            case(GL_TRIANGLE_STRIP):
            {
	    	DBG_TRC("not tested/implemented");
                IndexPointer iptr = indices;
                for(GLsizei i=2;i<count;++i,++iptr)
                {
		    if ((i%2)) this->operator()(*(iptr),*(iptr+2),*(iptr+1));
		    else       this->operator()(*(iptr),*(iptr+1),*(iptr+2));
                }
                break;
            }
            case(GL_QUADS):
            {
	    	DBG_TRC("not tested/implemented");
                IndexPointer iptr = indices;
                for(GLsizei i=3;i<count;i+=4,iptr+=4)
                {
                    this->operator()(*(iptr),*(iptr+1),*(iptr+2));
                    this->operator()(*(iptr),*(iptr+2),*(iptr+3));
                }
                break;
            }
            case(GL_QUAD_STRIP):
            {
	    	DBG_TRC("not tested/implemented");
                IndexPointer iptr = indices;
                for(GLsizei i=3;i<count;i+=2,iptr+=2)
                {
                    this->operator()(*(iptr),*(iptr+1),*(iptr+2));
                    this->operator()(*(iptr+1),*(iptr+3),*(iptr+2));
                }
                break;
            }
            case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
            case(GL_TRIANGLE_FAN):
            {
	    	DBG_TRC("not tested/implemented");
                IndexPointer iptr = indices;
                unsigned int first = *iptr;
                ++iptr;
                for(GLsizei i=2;i<count;++i,++iptr)
                {
                    this->operator()(first,*(iptr),*(iptr+1));
                }
                break;
            }
            case(GL_POINTS):
	    	DBG_TRC("not tested/implemented");
            case(GL_LINES):
	    	DBG_TRC("not tested/implemented");
            case(GL_LINE_STRIP):
	    	DBG_TRC("not tested/implemented");
            case(GL_LINE_LOOP):
	    	DBG_TRC("not tested/implemented");
            default:
                // can't be converted into to triangles.
                break;
        }
    }

    virtual void drawElements(GLenum mode,GLsizei count,const GLushort* indices)
    {
    	DBG_MSG("3 drawElements(GLenum mode,GLsizei count,const GLushort* indices)");
        if (indices==0 || count==0) return;

        typedef const GLushort* IndexPointer;

        switch(mode)
        {
            case(GL_TRIANGLES):
            {
	    	DBG_TRC("not tested/implemented");
                IndexPointer ilast = &indices[count];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr+=3)
                    this->operator()(*iptr,*(iptr+1),*(iptr+2));
                break;
            }
            case(GL_TRIANGLE_STRIP):
            {
	    	DBG_TRC("not tested/implemented");
                IndexPointer iptr = indices;
                for(GLsizei i=2;i<count;++i,++iptr)
                {
		    if ((i%2)) this->operator()(*(iptr),*(iptr+2),*(iptr+1));
		    else       this->operator()(*(iptr),*(iptr+1),*(iptr+2));
                }
                break;
            }
            case(GL_QUADS):
            {
	    	DBG_TRC("not tested/implemented");
                IndexPointer iptr = indices;
                for(GLsizei i=3;i<count;i+=4,iptr+=4)
                {
                    this->operator()(*(iptr),*(iptr+1),*(iptr+2));
                    this->operator()(*(iptr),*(iptr+2),*(iptr+3));
                }
                break;
            }
            case(GL_QUAD_STRIP):
            {
	    	DBG_TRC("not tested/implemented");
                IndexPointer iptr = indices;
                for(GLsizei i=3;i<count;i+=2,iptr+=2)
                {
                    this->operator()(*(iptr),*(iptr+1),*(iptr+2));
                    this->operator()(*(iptr+1),*(iptr+3),*(iptr+2));
                }
                break;
            }
            case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
            case(GL_TRIANGLE_FAN):
            {
	    	DBG_TRC("not tested/implemented");
                IndexPointer iptr = indices;
                unsigned int first = *iptr;
                ++iptr;
                for(GLsizei i=2;i<count;++i,++iptr)
                {
                    this->operator()(first,*(iptr),*(iptr+1));
                }
                break;
            }
            case(GL_POINTS):
            case(GL_LINES):
            case(GL_LINE_STRIP):
            case(GL_LINE_LOOP):
            default:
	    	DBG_TRC("not tested/implemented");
                // can't be converted into to triangles.
                break;
        }
    }

    virtual void drawElements(GLenum mode,GLsizei count,const GLuint* indices)
    {
//    	DBG_MSG(IGS_TRC,IGS_VLOW,"4 drawElements(GLenum mode,GLsizei count,const GLuint* indices)");
        if (indices==0 || count==0) return;

        typedef const GLuint* IndexPointer;

        switch(mode)
        {
            case(GL_TRIANGLES):
            {
                IndexPointer ilast = &indices[count];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr+=3)
                    this->operator()(*iptr,*(iptr+1),*(iptr+2));
                break;
            }
            case(GL_TRIANGLE_STRIP):
            {
                IndexPointer iptr = indices;
                for(GLsizei i=2;i<count;++i,++iptr)
                {
		    if ((i%2)) this->operator()(*(iptr),*(iptr+2),*(iptr+1));
		    else       this->operator()(*(iptr),*(iptr+1),*(iptr+2));
                }
                break;
            }
            case(GL_QUADS):
            {
	    	DBG_TRC("not tested/implemented");
                IndexPointer iptr = indices;
                for(GLsizei i=3;i<count;i+=4,iptr+=4)
                {
                    this->operator()(*(iptr),*(iptr+1),*(iptr+2));
                    this->operator()(*(iptr),*(iptr+2),*(iptr+3));
                }
                break;
            }
            case(GL_QUAD_STRIP):
            {
	    	DBG_TRC("not tested/implemented");
                IndexPointer iptr = indices;
                for(GLsizei i=3;i<count;i+=2,iptr+=2)
                {
                    this->operator()(*(iptr),*(iptr+1),*(iptr+2));
                    this->operator()(*(iptr+1),*(iptr+3),*(iptr+2));
                }
                break;
            }
            case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
            case(GL_TRIANGLE_FAN):
            {
	    	DBG_TRC("not tested/implemented");
                IndexPointer iptr = indices;
                unsigned int first = *iptr;
                ++iptr;
                for(GLsizei i=2;i<count;++i,++iptr)
                {
                    this->operator()(first,*(iptr),*(iptr+1));
                }
                break;
            }
            case(GL_POINTS):
            {
	    	DBG_TRC("not tested/implemented");
	    	DBG_TRC("GL_POINTS");
                IndexPointer ilast = &indices[count];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr++)
                    this->operator()(*iptr);
                break;
            }
            case(GL_LINES):
            {
	    	DBG_TRC("not tested/implemented");
	    	DBG_TRC("GL_LINES");
                IndexPointer ilast = &indices[count];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr+=2)
                    this->operator()(*iptr,*(iptr+1));
                break;
            }
            case(GL_LINE_STRIP):
            {
	    	DBG_TRC("not tested/implemented");
	    	DBG_TRC("GL_LINES_STRIP");

                IndexPointer ilast = &indices[count];
                for(IndexPointer  iptr=indices;iptr<ilast-2;iptr++)
                    this->operator()(*iptr,(*iptr+1));
                break;
            }

            case(GL_LINE_LOOP):
            {
	    	DBG_TRC("not tested/implemented");
	    	DBG_TRC("GL_LINES_LOOP not implemented");
/*
                IndexPointer ilast = &indices[count];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr++)
                    this->operator()(*iptr);
                break;
*/
            }

            default:
	    	DBG_WAR("Not handled");
                break;
        }
    }


};



}
#endif
