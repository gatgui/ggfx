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

#ifndef __ggfx_bufferobject_h_
#define __ggfx_bufferobject_h_

#include <ggfx/config.h>

namespace ggfx {
  
#ifndef __APPLE__
  typedef unsigned int ID; // on windows/linux GLuint is defined as unsigned int
#else
  typedef unsigned long ID; // on OSX GLuint is defined as unsigned long
#endif
  
  class GGFX_API BufferObject {

    public:
    
      enum Type {
        BOT_VERTEX = 0,
        BOT_INDEX,
        BOT_PIXEL_DST, // GL -> Buffer [PACK]
        BOT_PIXEL_SRC  // Buffer -> GL [UNPACK]
      };
    
      enum Access {
        BOA_READ = 0x01,
        BOA_WRITE = 0x02,
        BOA_READ_WRITE = BOA_READ|BOA_WRITE
      };
    
      enum Usage {
        BOU_STATIC = 0x01,
        BOU_DYNAMIC = 0x02,
        BOU_STREAM = BOU_STATIC|BOU_DYNAMIC,
        //BOU_DRAW = 0x10,
        //BOU_READ = 0x20,
        //BOU_COPY = BOU_DRAW|BOU_READ
      };
    
      BufferObject(Type type);
      
      virtual ~BufferObject();
    
      bool bind() const;
      
      bool bindAs(Type type);
      
      void unbind() const;
    
      bool upload(size_t sz, const void *data, Usage u);
      
      bool upload(size_t offset, size_t sz, const void *data);
      
      bool download(size_t offset, size_t sz, void *data);
    
      Usage getUsage() const;
      
      Access getAccess() const;
    
      size_t getSize() const;
    
      bool isLocked() const;
    
      bool lock(Access access, void **ptr, bool discard=false);
      
      void unlock();
      
      inline ID getId() const {
        return mVBO;
      }
      
      inline Type getType() const {
        return mType;
      }
    
    protected:
    
      bool checkValid() const;
    
      ID mVBO;
      Type mType;
  };
  
  // ---
  
  class GGFX_API RenderbufferObject {
    public:
      
      enum Format {
        RBF_RGB_8,
        RBF_RGBA_8,
        RBF_DEPTH_16,
        RBF_DEPTH_24,
        RBF_DEPTH_32,
        RBF_STENCIL,
        RBF_STENCIL_1,
        RBF_STENCIL_4,
        RBF_STENCIL_8,
        RBF_STENCIL_16,
        RBF_MAX
      };
      
      RenderbufferObject();
      ~RenderbufferObject();
      
      void bind();
      void unbind();
      
      void setup(Format fmt, int w, int h);
      
      inline int getWidth() const {
        return mWidth;
      }
      
      inline int getHeight() const {
        return mHeight;
      }
      
      inline Format getFormat() const {
        return mFormat;
      }
      
      inline ID getId() const {
        return mRBO;
      }
      
    protected:
      
      ID mRBO;
      Format mFormat;
      int mWidth;
      int mHeight;
  };
  
  // ---
  
  class GGFX_API FramebufferObject {
    public:
      
      FramebufferObject();
      ~FramebufferObject();
      
      void bind();
      void unbind();
      
      bool isValid() const;
      
      inline ID getId() const {
        return mFBO;
      }
      
      // in order +x +y +z -x -y -z for cube textures
      
      void attachColor1D(int n, unsigned int id, int lvl=0);
      void attachColor2D(int n, unsigned int id, int lvl=0);
      void attachColorRect(int n, unsigned int id);
      void attachColorCube(int n, int f, unsigned int id, int lvl=0);
      void attachColor3D(int n, int z, unsigned int id, int lvl=0);
      
      void attachDepth(unsigned int id);
      void attachDepth(const RenderbufferObject &rbo);
      
      void attachStencil(unsigned int id);
      void attachStencil(const RenderbufferObject &rbo);
      
      void setDrawBuffer(int n);
      void setDrawBuffers(int n0, int n1);
      void setDrawBuffers(int n0, int n1, int n2);
      void setDrawBuffers(int n0, int n1, int n2, int n3);
      
    protected:
      
      static FramebufferObject *msLastBoundFBO;
      
      void storeDrawBuffers();
      void restoreDrawBuffers();
      
      ID mFBO;
      bool mDrawBuffersChanged;
      long mInitialDrawBuffers[4];
  };
  
  
  
}

#endif

