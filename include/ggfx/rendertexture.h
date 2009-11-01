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

#ifndef __ggfx_RenderTexture_h_
#define __ggfx_RenderTexture_h_

#include <ggfx/config.h>

namespace ggfx {
  
  class GGFX_API FrameBufferObject {
    
    public:
    
      enum Attachment {
        AT_COLOR0 = GL_COLOR_ATTACHMENT0_EXT,
        AT_DEPTH = GL_DEPTH_ATTACHMENT_EXT,
        AT_STENCIL = GL_STENCIL_ATTACHMENT_EXT,
        AT_MAX
      };
    
      inline FrameBufferObject() : mId(0) {
      }
    
      inline ~FrameBufferObject() {
      
      }
    
      inline void create() {
        glGenFramebuffersEXT(1, &mId);
      }
    
      inline void destroy() {
        glDeleteFramebuffersEXT(1, &mId);
        mId = 0;
      }
    
      inline void bind() {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
      }
      
      inline void unbind() {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
      }
    
      inline void attach(Texture *tex, Attachment to=AT_COLOR0, int mipmapLevel=0) {
        bind();
        if (tex->is1D()) {
          glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT, (GLenum)to,
            tex->target(), tex->id(), mipmapLevel);
        } else if (tex->is2D()) {
          glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, (GLenum)to,
            tex->target(), tex->id(), mipmapLevel);
        } else {
          glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, (GLenum)to,
            tex->target(), tex->id(), mipmapLevel);
        }
      }
    
    protected:
      
      GLuint mId;
  };
  
}

#endif


