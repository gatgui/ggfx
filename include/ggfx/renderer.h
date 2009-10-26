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

#ifndef __ggfx_renderer_h_
#define __ggfx_renderer_h_

#include <ggfx/config.h>
#include <ggfx/light.h>
#include <ggfx/material.h>

namespace ggfx {
  
  enum CompareFunc {
    CF_ALWAYS = 0,
    CF_NEVER,
    CF_LESS,
    CF_LEQUAL,
    CF_EQUAL,
    CF_GEQUAL,
    CF_GREATER,
    CF_NOTEQUAL,
    CF_MAX
  };
  
  enum StencilOp {
    SO_KEEP = 0,
    SO_ZERO,
    SO_REPLACE,
    SO_INCR,
    SO_DECR,
    SO_INV,
    SO_MAX
  };
  
  enum FrontFace {
    FF_CCW = 0,
    FF_CW,
    FF_MAX
  };
  
  enum PolygonFace {
    PF_FRONT = 0,
    PF_BACK,
    PF_FRONT_AND_BACK,
    PF_MAX
  };
  
  class GGFX_API Renderer {
    public:
      
      Renderer();
      virtual ~Renderer();
      
      static Renderer& Instance();
      static Renderer* InstancePtr();
      
      void initialize();
      
      void setClearColor(const gmath::Vector4 &val);
      
      void setClearStencil(long val);
      void setDepthTestEnabled(bool on);
      void setDepthWriteEnabled(bool on);
      void setDepthFunc(CompareFunc f);
      
      void setClearDepth(float val);
      void setStencilTestEnabled(bool on);
      void setStencilFunc(CompareFunc f);
      void setStencilRef(long val);
      void setStencilMask(unsigned long val);
      void setStencilFail(StencilOp op);
      void setStencilDepthFail(StencilOp op);
      void setStencilDepthPass(StencilOp op);
      
      void setFrontFace(FrontFace ff);
      void setCulledFace(PolygonFace  pf);
      void setFaceCullingEnabled(bool on);
      
      void setNormalizeEnabled(bool on);
      
      void setLightingEnabled(bool on);
      void setupLight(int i, const Light *light);
    
      void setupMaterial(PolygonFace pf, const Material *m);
      
      // blending
      // texture
      // alpha
      // culling
      // polygon offset
      // clear
      // matrices
      // vbo
      // fbo -> rendertarget !
      // pixelformat
      
    protected:
      
      static Renderer *msInstance;
      
      CompareFunc mStencilFunc;
      long mStencilRef;
      unsigned long mStencilMask;
      StencilOp mStencilFail;
      StencilOp mStencilDepthFail;
      StencilOp mStencilDepthPass;
      CompareFunc mDepthFunc;
  };
  
}



#endif

