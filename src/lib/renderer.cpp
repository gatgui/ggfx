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

#include <ggfx/renderer.h>

using namespace gmath;
using namespace std;

namespace ggfx {
  
  static GLenum GL_CompareFunc[CF_MAX] = {
    GL_ALWAYS,
    GL_NEVER,
    GL_LESS,
    GL_LEQUAL,
    GL_EQUAL,
    GL_GEQUAL,
    GL_GREATER,
    GL_NOTEQUAL
  };
  
  static GLenum GL_StencilOp[SO_MAX] = {
    GL_KEEP,
    GL_ZERO,
    GL_REPLACE,
    GL_INCR,
    GL_DECR,
    GL_INVERT
  };
  
  static GLenum GL_FrontFace[FF_MAX] = {
    GL_CCW,
    GL_CW
  };
  
  static GLenum GL_PolygonFace[PF_MAX] = {
    GL_FRONT,
    GL_BACK,
    GL_FRONT_AND_BACK
  };
  
  // ---
  
  Renderer* Renderer::msInstance = NULL;
  
  Renderer& Renderer::Instance() {
    if (!msInstance) {
      new  Renderer();
    }
    return *msInstance;
  }
  
  Renderer* Renderer::InstancePtr() {
    return msInstance;
  }
  
  Renderer::Renderer()
    : mStencilFunc(CF_ALWAYS), mStencilRef(0), mStencilMask(0xFFFFFFFF),
      mStencilFail(SO_KEEP), mStencilDepthFail(SO_KEEP), mStencilDepthPass(SO_KEEP) {
    assert(msInstance == NULL);
    msInstance = this;
  }
  
  Renderer::~Renderer() {
    msInstance = NULL;
  }
  
  void Renderer::initialize() {
    glewInit();
    setClearDepth(1.0f);
    setClearStencil(0);
    setClearColor(Vector4(0,0,0,1));
    setLightingEnabled(true);
    setDepthTestEnabled(true);
    setDepthWriteEnabled(true);
    setStencilTestEnabled(false);
    setFrontFace(FF_CCW);
    setCulledFace(PF_BACK);
    setFaceCullingEnabled(true);
    setNormalizeEnabled(false);
  }
  
  void Renderer::setClearStencil(long val) {
    glClearStencil(GLint(val));
  }
  
  void Renderer::setClearDepth(float val) {
    glClearDepth(val);
  }
  
  void Renderer::setClearColor(const Vector4 &val) {
    glClearColor(val.x, val.y, val.z, val.w);
  }
  
  void Renderer::setDepthTestEnabled(bool on) {
    if (on) {
      glEnable(GL_DEPTH_TEST);
    } else {
      glDisable(GL_DEPTH_TEST);
    }
  }
  
  void Renderer::setDepthWriteEnabled(bool on) {
    glDepthMask(GLboolean(on));
  }
  
  void Renderer::setStencilTestEnabled(bool on) {
    if (on) {
      glEnable(GL_STENCIL_TEST);
    } else {
      glDisable(GL_STENCIL_TEST);
    }
  }
  
  void Renderer::setDepthFunc(CompareFunc f) {
    glDepthFunc(GL_CompareFunc[f]);
  }
  
  void Renderer::setStencilFunc(CompareFunc f) {
    mStencilFunc = f;
    glStencilFunc(GL_CompareFunc[f], GLint(mStencilRef), GLuint(mStencilMask));
  }
  
  void Renderer::setStencilRef(long val) {
    mStencilRef = val;
    glStencilFunc(GL_CompareFunc[mStencilFunc], GLint(val), GLuint(mStencilMask));
  }
  
  void Renderer::setStencilMask(unsigned long val) {
    mStencilMask = val;
    glStencilFunc(GL_CompareFunc[mStencilFunc], GLint(mStencilRef), GLuint(val));
  }
  
  void Renderer::setStencilFail(StencilOp op) {
    mStencilFail = op;
    glStencilOp(
      GL_StencilOp[op],
      GL_StencilOp[mStencilDepthFail],
      GL_StencilOp[mStencilDepthPass]);
  }
  
  void Renderer::setStencilDepthFail(StencilOp op) {
    mStencilDepthFail = op;
    glStencilOp(
      GL_StencilOp[mStencilFail],
      GL_StencilOp[op],
      GL_StencilOp[mStencilDepthPass]);
  }
  
  void Renderer::setStencilDepthPass(StencilOp op) {
    mStencilDepthPass = op;
    glStencilOp(
      GL_StencilOp[mStencilFail],
      GL_StencilOp[mStencilDepthFail],
      GL_StencilOp[op]);
  }
  
  void Renderer::setLightingEnabled(bool on) {
    if (on) {
      glEnable(GL_LIGHTING);
    } else {
      glDisable(GL_LIGHTING);
    }
  }
  
  void Renderer::setupLight(int i, const Light *light) {
    if (!light || light->isVirtual() || i<0 || i>8) {
      return;
    }
    if (light->isEnabled()) {
      GLenum id = GLenum(GL_LIGHT0 + i);
      glEnable(id);
      if (light->getType() == LT_SPOT) {
        glLightf(id, GL_SPOT_CUTOFF, light->getSpotCutOff());
        glLightf(id, GL_SPOT_EXPONENT, light->getSpotExponent());
        glLightfv(id, GL_SPOT_DIRECTION, light->getWorldDirection());
        glLightfv(id, GL_POSITION, Vector4(light->getWorldPosition(), 1.0));
      } else {
        glLightf(id, GL_SPOT_CUTOFF, 180.0f);
        glLightf(id, GL_SPOT_EXPONENT, 0.0f);
        glLightfv(id, GL_SPOT_DIRECTION, Vector3(0,0,-1));
        if (light->getType() == LT_DIR) {
          glLightfv(id, GL_POSITION, Vector4(light->getWorldDirection(), 0.0));
        } else {
          glLightfv(id, GL_POSITION, Vector4(light->getWorldPosition(), 1.0));
        }
      }
      glLightfv(id, GL_DIFFUSE, light->getDiffuse());
      glLightfv(id, GL_AMBIENT, light->getAmbient());
      glLightfv(id, GL_SPECULAR, light->getSpecular());
      glLightf(id, GL_CONSTANT_ATTENUATION, light->getConstantAttenuation());
      glLightf(id, GL_LINEAR_ATTENUATION, light->getLinearAttenuation());
      glLightf(id, GL_QUADRATIC_ATTENUATION, light->getQuadraticAttenuation());
    } else {
      glDisable(GLenum(GL_LIGHT0+i));
    }
  }
  
  void Renderer::setupMaterial(PolygonFace pf, const Material *m) {
    if (!m) {
      return;
    }
    GLenum id = GL_PolygonFace[pf];
    glMaterialfv(id, GL_DIFFUSE, m->getDiffuse());
    glMaterialfv(id, GL_AMBIENT, m->getAmbient());
    glMaterialfv(id, GL_SPECULAR, m->getSpecular());
    glMaterialfv(id, GL_EMISSION, m->getEmissive());
    glMaterialf(id, GL_SHININESS, m->getShininess());
  }
  
  void Renderer::setFrontFace(FrontFace ff) {
    glFrontFace(GL_FrontFace[ff]);
  }
  
  void Renderer::setCulledFace(PolygonFace  pf) {
    glCullFace(GL_PolygonFace[pf]);
  }
  
  void Renderer::setFaceCullingEnabled(bool on) {
    if (on) {
      glEnable(GL_CULL_FACE);
    } else {
      glDisable(GL_CULL_FACE);
    }
  }
  
  void Renderer::setNormalizeEnabled(bool on) {
    if (on) {
      glEnable(GL_NORMALIZE);
    } else {
      glDisable(GL_NORMALIZE);
    }
  }
  
}

