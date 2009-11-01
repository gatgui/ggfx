/*

Copyright (C) 2009  Gaetan Guidet

This file is part of ggfx.

ggfx is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

ggfx is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

*/

#include <ggfx/bufferobject.h>

using namespace std;

namespace ggfx {
  
  static GLenum GL_BOtype[] = {
    GL_ARRAY_BUFFER_ARB,
    GL_ELEMENT_ARRAY_BUFFER_ARB,
    GL_PIXEL_PACK_BUFFER_ARB, // write to buffer  [GL -> buffer]
    GL_PIXEL_UNPACK_BUFFER_ARB //read from buffer [buffer -> GL]
  };
  
  static bool GLusage(BufferObject::Usage u, GLenum &e) {
    if ((u & BufferObject::BOU_STREAM) == BufferObject::BOU_STREAM) {
      e = GL_STREAM_DRAW_ARB;
      //if ((u & BOU_COPY) == BOU_COPY) {
      //  e = GL_STREAM_COPY_ARB;
      //} else if (u & BOU_READ) {
      //  e = GL_STREAM_READ_ARB;
      //} else if (u & BOU_DRAW) {
      //  e = GL_STREAM_DRAW_ARB;
      //} else {
      //  return false;
      //}
    } else if (u & BufferObject::BOU_STATIC) {
      e = GL_STATIC_DRAW_ARB;
      //if ((u & BOU_COPY) == BOU_COPY) {
      //  e = GL_STATIC_COPY_ARB;
      //} else if (u & BOU_READ) {
      //  e = GL_STATIC_READ_ARB;
      //} else if (u & BOU_DRAW) {
      //  e = GL_STATIC_DRAW_ARB;
      //} else {
      //  return false;
      //}
    } else if (u & BufferObject::BOU_DYNAMIC) {
      e = GL_DYNAMIC_DRAW_ARB;
      //if ((u & BOU_COPY) == BOU_COPY) {
      //  e = GL_DYNAMIC_COPY_ARB;
      //} else if (u & BOU_READ) {
      //  e = GL_DYNAMIC_READ_ARB;
      //} else if (u & BOU_DRAW) {
      //  e = GL_DYNAMIC_DRAW_ARB;
      //} else {
      //  return false;
      //}
    } else {
      return false;
    }
    return true;
  }

  BufferObject::BufferObject(BufferObject::Type type)
    : mVBO(0), mType(type) {
    glGenBuffersARB(1, &mVBO);
  }
  
  BufferObject::~BufferObject() {
    glDeleteBuffersARB(1, &mVBO);
  }
  
  bool BufferObject::bind() const {
    glBindBufferARB(GL_BOtype[mType], mVBO);
    return checkValid();
  }
  
  void BufferObject::unbind() const {
    glBindBufferARB(GL_BOtype[mType], 0);
  }
  
  bool BufferObject::bindAs(Type type) {
    mType = type;
    return bind();
  }
  
  bool BufferObject::upload(size_t sz, const void *data, BufferObject::Usage u) {
    bind();
    GLenum usage;
    if (!GLusage(u, usage)) {
      return false;
    }
    glBufferDataARB(GL_BOtype[mType], sz, data, usage);
    return checkValid();
  }
  
  bool BufferObject::upload(size_t offset, size_t sz, const void *data) {
    if (!isLocked()) {
      glBufferSubDataARB(GL_BOtype[mType], offset, sz, data);
      return checkValid();
    }
    return false;
  }
  
  bool BufferObject::download(size_t offset, size_t sz, void *data) {
    if (!isLocked()) {
      glGetBufferSubDataARB(GL_BOtype[mType], offset, sz, data);
      return checkValid();
    }
    return false;
  }
  
  BufferObject::Usage BufferObject::getUsage() const {
    GLint i;
    bind();
    glGetBufferParameterivARB(GL_BOtype[mType], GL_BUFFER_USAGE_ARB, &i);
    checkValid();
    switch (i) {
      case GL_DYNAMIC_READ_ARB: //return Usage(BOU_DYNAMIC|BOU_READ);
      case GL_DYNAMIC_DRAW_ARB: //return Usage(BOU_DYNAMIC|BOU_DRAW);
      case GL_DYNAMIC_COPY_ARB: return Usage(BOU_DYNAMIC); //|BOU_COPY);
      case GL_STATIC_READ_ARB: //return Usage(BOU_STATIC|BOU_READ);
      case GL_STATIC_DRAW_ARB: //return Usage(BOU_STATIC|BOU_DRAW);
      case GL_STATIC_COPY_ARB: return Usage(BOU_STATIC); //|BOU_COPY);
      case GL_STREAM_READ_ARB: //return Usage(BOU_STREAM|BOU_READ);
      case GL_STREAM_DRAW_ARB: //return Usage(BOU_STREAM|BOU_DRAW);
      case GL_STREAM_COPY_ARB: return Usage(BOU_STREAM); //|BOU_COPY);
      default:
        return Usage(0);
    }
  }
  
  BufferObject::Access BufferObject::getAccess() const {
    if (!isLocked()) {
      GLint i;
      bind();
      glGetBufferParameterivARB(GL_BOtype[mType], GL_BUFFER_ACCESS_ARB, &i);
      if (checkValid()) {
        switch (i) {
          case GL_READ_ONLY_ARB: return Access(BOA_READ);
          case GL_WRITE_ONLY_ARB: return Access(BOA_WRITE);
          case GL_READ_WRITE_ARB: return Access(BOA_READ_WRITE);
          default:
            return Access(0);
        }
      }
    }
    return Access(0);
  }
  
  size_t BufferObject::getSize() const {
    GLint i;
    bind();
    glGetBufferParameterivARB(GL_BOtype[mType], GL_BUFFER_SIZE_ARB, &i);
    checkValid();
    return (size_t)i;
  }
  
  bool BufferObject::isLocked() const {
    GLint i;
    bind();
    glGetBufferParameterivARB(GL_BOtype[mType], GL_BUFFER_MAPPED_ARB, &i);
    checkValid();
    return (i != 0);
  }
  
  bool BufferObject::lock(BufferObject::Access access, void **ptr, bool discard) {
    if (!ptr) {
      return false;
    }
    if (!isLocked()) {
      GLenum a;
      if ((access & BOA_READ_WRITE) == BOA_READ_WRITE) {
        a = GL_READ_WRITE_ARB;
      } else if (access & BOA_READ) {
        a = GL_READ_ONLY_ARB;
      } else if (access & BOA_WRITE) {
        a = GL_WRITE_ONLY_ARB;
      } else {
        *ptr = NULL;
        return false;
      }
      if (discard) {
        GLenum usage;
        if (!GLusage(getUsage(), usage)) {
          *ptr = NULL;
          return false;
        }
        glBufferDataARB(GL_BOtype[mType], getSize(), NULL, usage);
      }
      *ptr = glMapBufferARB(GL_BOtype[mType], a);
      if (checkValid()) {
        return true;
      }
    }
    *ptr = NULL;
    return false;
  }
  
  /*
  void* BufferObject::bufferPtr() {
    void *ptr;
    glGetBufferPointervARB(mTarget, GL_BUFFER_MAP_POINTER_ARB, &ptr);
    return ptr;
  }
  */
  
  void BufferObject::unlock() {
    bind();
    glUnmapBufferARB(GL_BOtype[mType]);
  }
  
  bool BufferObject::checkValid() const {
    GLenum e = glGetError();
    if (e != GL_NO_ERROR) {
      switch (e) {
        case GL_INVALID_ENUM:
          cout << "BufferObject ERROR: invalid target" << endl;
          break;
        case GL_INVALID_OPERATION:
          cout << "BufferObject ERROR: invalid operation" << endl;
          break;
        default:
          cout << "BufferObject ERROR: unknown error" << endl;
          break;
      }
      return false;
    }
    return true;
    //return (glGetError() == GL_NO_ERROR);
  }
  
  // ---
  
  RenderbufferObject::RenderbufferObject()
    : mFormat(RBF_MAX), mWidth(0), mHeight(0) {
    glGenRenderbuffersEXT(1, &mRBO);
  }
  
  RenderbufferObject::~RenderbufferObject() {
    if (mRBO != 0) {
      glDeleteRenderbuffersEXT(1, &mRBO);
    }
  }
  
  void RenderbufferObject::bind() {
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mRBO);
  }
  
  void RenderbufferObject::unbind() {
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
  }
  
  void RenderbufferObject::setup(RenderbufferObject::Format fmt, int w, int h) {
    static GLenum GLfmt[] = {
      GL_RGB,
      GL_RGBA,
      GL_DEPTH_COMPONENT16,
      GL_DEPTH_COMPONENT24,
      GL_DEPTH_COMPONENT32,
      GL_STENCIL_INDEX,
      GL_STENCIL_INDEX1_EXT,
      GL_STENCIL_INDEX4_EXT,
      GL_STENCIL_INDEX8_EXT,
      GL_STENCIL_INDEX16_EXT
    };
    mWidth = w;
    mHeight = h;
    mFormat = fmt;
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GLfmt[fmt], w, h);
  }
  
  // ---
  
  FramebufferObject* FramebufferObject::msLastBoundFBO = NULL;
  
  FramebufferObject::FramebufferObject()
    : mDrawBuffersChanged(false) {
    glGenFramebuffersEXT(1, &mFBO);
  }
  
  FramebufferObject::~FramebufferObject() {
    if (msLastBoundFBO == this) {
      unbind();
    }
    glDeleteFramebuffersEXT(1, &mFBO);
  }
  
  void FramebufferObject::storeDrawBuffers() {
    if (!mDrawBuffersChanged) {
      GLint val;
      glGetIntegerv(GL_DRAW_BUFFER0_ARB, &val);
      mInitialDrawBuffers[0] = val;
      glGetIntegerv(GL_DRAW_BUFFER1_ARB, &val);
      mInitialDrawBuffers[1] = val;
      glGetIntegerv(GL_DRAW_BUFFER2_ARB, &val);
      mInitialDrawBuffers[2] = val;
      glGetIntegerv(GL_DRAW_BUFFER4_ARB, &val);
      mInitialDrawBuffers[3] = val;
      mDrawBuffersChanged = true;
    }
  }
  
  void FramebufferObject::restoreDrawBuffers() {
    if (mDrawBuffersChanged) {
      GLenum db[4] = {
        mInitialDrawBuffers[0], mInitialDrawBuffers[1],
        mInitialDrawBuffers[2], mInitialDrawBuffers[2]
      };
      glDrawBuffersARB(4, db);
      mDrawBuffersChanged = false;
    }
  }
  
  void FramebufferObject::setDrawBuffer(int n) {
    storeDrawBuffers();
    GLenum db[4] = {
      GLenum(GL_COLOR_ATTACHMENT0_EXT+n),
      GL_NONE, GL_NONE, GL_NONE
    };
    glDrawBuffersARB(4, db);
  }
  
  void FramebufferObject::setDrawBuffers(int n0, int n1) {
    storeDrawBuffers();
    GLenum db[4] = {
      GLenum(GL_COLOR_ATTACHMENT0_EXT+n0),
      GLenum(GL_COLOR_ATTACHMENT0_EXT+n1),
      GL_NONE, GL_NONE
    };
    glDrawBuffersARB(4, db);
  }
  
  void FramebufferObject::setDrawBuffers(int n0, int n1, int n2) {
    storeDrawBuffers();
    GLenum db[4] = {
      GLenum(GL_COLOR_ATTACHMENT0_EXT+n0),
      GLenum(GL_COLOR_ATTACHMENT0_EXT+n1),
      GLenum(GL_COLOR_ATTACHMENT0_EXT+n2),
      GL_NONE
    };
    glDrawBuffersARB(4, db);
  }
  
  void FramebufferObject::setDrawBuffers(int n0, int n1, int n2, int n3) {
    storeDrawBuffers();
    GLenum db[4] = {
      GLenum(GL_COLOR_ATTACHMENT0_EXT+n0),
      GLenum(GL_COLOR_ATTACHMENT0_EXT+n1),
      GLenum(GL_COLOR_ATTACHMENT0_EXT+n2),
      GLenum(GL_COLOR_ATTACHMENT0_EXT+n3)
    };
    glDrawBuffers(4, db);
  }
  
  void FramebufferObject::bind() {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBO);
    if (msLastBoundFBO != NULL) {
      msLastBoundFBO->restoreDrawBuffers();
    }
    msLastBoundFBO = this;
  }
  
  void FramebufferObject::unbind() {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    restoreDrawBuffers();
    msLastBoundFBO = NULL;
  }
  
  void FramebufferObject::attachColor1D(int n, unsigned int id, int lvl) {
    glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT, GLenum(GL_COLOR_ATTACHMENT0_EXT+n),
      GL_TEXTURE_1D, id, lvl);
  }
  
  void FramebufferObject::attachColor2D(int n, unsigned int id, int lvl) {
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GLenum(GL_COLOR_ATTACHMENT0_EXT+n),
      GL_TEXTURE_2D, id, lvl);
  }
  
  void FramebufferObject::attachColorRect(int n, unsigned int id) {
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GLenum(GL_COLOR_ATTACHMENT0_EXT+n),
      GL_TEXTURE_RECTANGLE_ARB, id, 0);
  }
  
  void FramebufferObject::attachColorCube(int n, int f, unsigned int id, int lvl) {
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GLenum(GL_COLOR_ATTACHMENT0_EXT+n),
      GLenum(GL_TEXTURE_CUBE_MAP_POSITIVE_X+f), id, lvl);
  }
  
  void FramebufferObject::attachColor3D(int n, int z, unsigned int id, int lvl) {
    glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, GLenum(GL_COLOR_ATTACHMENT0_EXT+n),
      GL_TEXTURE_3D, id, lvl, z);
  }
  
  void FramebufferObject::attachDepth(const RenderbufferObject &rbo) {
    attachDepth(rbo.getId());
  }
  
  void FramebufferObject::attachDepth(unsigned int id) {
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
      GL_RENDERBUFFER_EXT, id);
  }
  
  void FramebufferObject::attachStencil(const RenderbufferObject &rbo) {
    attachStencil(rbo.getId());
  }
  
  void FramebufferObject::attachStencil(unsigned int id) {
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
      GL_RENDERBUFFER_EXT, id);
  }
  
  bool FramebufferObject::isValid() const {
    GLenum fbErr = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (fbErr != GL_FRAMEBUFFER_COMPLETE_EXT) {
      switch (fbErr) {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
          cout << "FramebufferObject: Incomplete attachment" << endl;
          break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
          cout << "FramebufferObject: Missing attachment" << endl;
          break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
          cout << "FramebufferObject: Invalid dimensions" << endl;
          break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
          cout << "FramebufferObject: Invalid formats" << endl;
          break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
          cout << "FramebufferObject: Invalid draw buffer" << endl;
          break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
          cout << "FramebufferObject: Invalid read buffer" << endl;
          break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
          cout << "FramebufferObject: Unsupported" << endl;
          break;
        default:
          cout << "FramebufferObject: Unknown error" << endl;
      }
      return false;
    }
    return true;
  }

}

