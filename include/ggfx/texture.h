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

#ifndef __ggfx_Texture_h_
#define __ggfx_Texture_h_

#include <ggfx/config.h>

namespace ggfx {

  class GGFX_API Texture {
    public:
    
      friend class RenderTexture;
    
      enum Format {
        TIF_RGBA8,
        TIF_RGB8,
        TIF_R8,
        TIF_L8,
        TIF_RGBA16F,
        TIF_RGB16F,
        TIF_R16F,
        TIF_L16F,
        TIF_RGBA32F,
        TIF_RGB32F,
        TIF_R32F,
        TIF_L32F,
        TIF_DEPTH24,
        TIF_DEPTH32
      };
      
      // mip maps ?
  
      Texture() : mId(0) {
        create();
      }
      ~Texture() {
        destroy();
      }
    
      void create() {
        glGenTextures(1, &mId);
      }
    
      void destroy() {
        glDeleteTextures(1, &mId);
      }
    
      void bind() {
        glBindTexture(mTarget, mId);
      }
      
      void unbind() {
        glBindTexture(mTarget, 0);
      }
    
      GLenum target() const {
        return mTarget;
      }
    
      GLenum format() const {
        return mFormat;
      }
    
      GLenum internalFormat() const {
        return mInternalFormat;
      }
    
      bool is1D() const {
        return mHeight==1 && mDepth==1;
      }
      
      bool is2D() const {
        return mHeight>1 && mDepth==1;
      }
      
      bool is3D() const {
        return mHeight>1 && mDepth>1;
      }
    
      // pixel data up/down
      bool upload(int offx, int offy, int w, int h, GLenum pixelFormat, void *data, int mipmapLevel=0) {
        if (!is2D()) {
          return false;
        }
        if (offx<0 || offy<0 || offx+w>mWidth || offy+h>mHeight) {
          return false;
        }
        bind();
        glTexSubImage2D(mTarget,offx,offy,mipmapLevel,w,h,mFormat,pixelFormat,data);
        // glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        // glRasterPos2i(0,0);
        // glDrawPixels(w,h,mFormat,pixelFormat,data);
        return true;
      }
    
      void download(GLenum pixelFormat, void *data, int mipmapLevel=0) {
        bind();
        glGetTexImage(mTarget,mipmapLevel,mFormat,pixelFormat,data);
        // glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
        // glReadPixels(x,y,w,h,mFormat,pixelFormat,data);
      }
    
    protected:
      
      GLenum mTarget;
      GLenum mFormat;
      GLenum mInternalFormat;
      GLuint mId;
      unsigned int mWidth, mHeight, mDepth;
  };
  
  // applicable to any texture in fact
  // this is just a parameter set
  
  class GGFX_API TextureUnit {
    public:
      
      enum Filter {
        TF_LINEAR,
        TF_NEAREST
      };
      
      enum MipmapFilter {
        TMF_LINEAR,
        TMF_NEAREST,
        TMF_NONE
      };
      
      enum Wrap {
        TW_CLAMP,
        TW_CLAMP_TO_EDGE,
        TW_REPEAT,
        TW_MIRROR
      };
      
      enum Env {
        TE_REPLACE, // set color to texel color
        TE_MODULATE, // modulate incoming fragment color with texel color
        TE_COLOR_BLEND, // use texel rgb as blend factor for incoming fragment color, and env color
        TE_ALPHA_BLEND, // as above but use texel alpha to blend incoming fragment color and texel color
        TE_ADD, // add incoming fragment color and texel color
        TE_COMBINE // advanced
      };
      
      enum CombineComp {
        RGB=0,
        ALPHA
      };
      
      enum CombineArg {
        ARG0 = 0,
        ARG1,
        ARG2
      };
      
      enum CombineScale {
        SCALE_1X = 0,
        SCALE_2X,
        SCALE_4X
      };
      
      enum CombineCmd {
        REPLACE,     //  Src0
        MODULATE,    //  Src0 * Src1
        ADD,         //  Src0 + Src1
        ADD_SIGNED,  //  Src0 + Src1 - 0.5
        SUBTRACT,    //  Src0 - Src1
        INTERPOLATE, //  Src0*Src2 + Src1*(1-Src2),
        DOT3_RGB,    //  Src0 <dot> Src1
        DOT3_RGBA    //  Src0 <dot> Src1
      };
      
      enum CombineSrc {
        PRIMARY_COLOR,
        TEXTURE,
        CONSTANT,
        PREVIOUS  // MAP to PRIMARY COLOR on first Texture unit
      };
      
      enum CombineOp {
        SRC_COLOR,
        SRC_ALPHA,
        INV_SRC_COLOR, // 1-SRC_COLOR
        INV_SRC_ALPHA  // 1-SRC_ALPHA
      };
      
      // Op give us which component of the source to use
      // Src: Constant --> TEXTURE_ENV_COLOR
      
      
      
      
      TextureUnit(unsigned int unit=0) :mUnit(unit) {
        reset();
      }
      
      virtual ~TextureUnit() {
      }
      
      void setWrap(Wrap s, Wrap t=TW_REPEAT, Wrap r=TW_REPEAT) {
        static GLenum GLconv[] = {
          GL_CLAMP, GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRROR
        };
        mWrap[0] = GLconv[s];
        mWrap[1] = GLconv[t];
        mWrap[2] = GLconv[r];
      }
      
      void setEnv(Env e) {
        static GLenum GLconv[] = {
          GL_REPLACE, GL_MODULATE, GL_BLEND, GL_DECAL, GL_ADD, GL_COMBINE
        };
        mEnv = GLconv[e];
      }
      
      void setEnvColor(float r, float g, float b, float a=1.0f) {
        mEnvColor[0] = r;
        mEnvColor[1] = g;
        mEnvColor[2] = b;
        mEnvColor[3] = a;
      }
      
      void setCombineCmd(CombineComp comp, CombineCmd cmd) {
        static GLenum GLconv[] = {
          GL_REPLACE, GL_MODULATE, GL_ADD, GL_ADD_SIGNED, GL_SUBTRACT, GL_INTERPOLATE, GL_DOT3_RGB, GL_DOT3_RGBA
        };
        mCombineCmd[comp] = GLconv[cmd];
      }
      
      void setCombineArg(CombineComp comp, CombineArg i, CombineSrc src, CombineOp op) {
        if (mUnit == 0 && src==PREVIOUS) {
          src = PRIMARY_COLOR;
        }
        if (comp == ALPHA && (op==SRC_COLOR || op==INV_SRC_COLOR) {
          return;
        }
        static GLenum GLsrcConv[] = {
          GL_PRIMARY_COLOR, GL_TEXTURE, GL_CONSTANT, GL_PREVIOUS
        };
        static GLenum GLopConv[] = {
          GL_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA
        };
        mCombineSrc[comp][i] = GLsrcConv[src];
        mCombineOp[comp][i] = GLopConv[op];
      }
      
      void setCombineColor(float r, float g, float b, float a) {
        setEnvColor(r,g,b,a);
      }
      
      void setCombineScale(CombineComp comp, CombineScale scl) {
        GLfloat f = 1.0f;
        if (scl == SCALE_2X) {
          f = 2.0f;
        } else if (scl == SCALE_4X) {
          f = 4.0f;
        }
        mCombineScale[comp] = f;
      }
      
      void setFilter(Filter minf, Filter magf, MipmapFilter mip) {
        mMagFilter = mag;
        if (mip == TM_LINEAR) {
          if (min == TF_LINEAR) {
            mMinFilter = GL_LINEAR_MIPMAP_LINEAR;
          } else {
            mMinFilyer = GL_LINEAR_MIPMAP_NEAREST;
          }
        } else if (mip == TM_NEAREST) {
          if (min == TF_LINEAR) {
            mMinFilter = GL_NEAREST_MIPMAP_LINEAR;
          } else {
            mMinFilyer = GL_NEAREST_MIPMAP_NEAREST;
          }
        } else {
          if (min == TF_LINEAR) {
            mMinFilter = GL_LINEAR;
          } else {
            mMinFilyer = GL_NEAREST;
          }
        }
      }
      
      void setupFor(Texture *tex) {
        glActiveTextureARB(GLenum(GL_TEXTURE0_ARB+mUnit));
        tex->bind();
        glTexParameteri(tex->target(), GL_TEXTURE_MIN_FILTER, mMinFilter);
        glTexParameteri(tex->target(), GL_TEXTURE_MAG_FILTER, mMagFilter);
        glTexParameteri(tex->target(), GL_TEXTURE_WRAP_S, mWrap[0]);
        if (tex->is2D()) {
          glTexParameteri(tex->target(), GL_TEXTURE_WRAP_T, mWrap[1]);
        }
        if (tex->is3D()) {
          glTexParameteri(tex->target(), GL_TEXTURE_WRAP_R, mWrap[2]);
        }
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mEnv);
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, mEnvColor);
        if (mEnv == TE_COMBINE) {
          glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB,    mCombineCmd[0]);
          glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,  mCombineCmd[1]);
          glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE,      mCombineScale[0]);
          glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE,    mCombineScale[1]);
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB,    mCombineSrc[0][0]);
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,  mCombineSrc[1][0]);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB,   mCombineOp[0][0]);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, mCombineOp[1][0]);
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB,    mCombineSrc[0][1]);
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA,  mCombineSrc[1][1]);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB,   mCombineOp[0][1]);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, mCombineOp[1][1]);
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB,    mCombineSrc[0][2]);
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA,  mCombineSrc[1][2]);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB,   mCombineOp[0][2]);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, mCombineOp[1][2]);
        }
      }
      
      void reset() {
        // GLuint mUnit = 0; ??
        mWrap[0] = mWrap[1] = mWrap[2] = GL_REPEAT;
        mMinFilter = mMagFilter = GL_NEAREST;
        mEnv = GL_MODULATE;
        mEnvColor[0] = mEnvColor[1] = mEnvColor[2] = 0;
        mEnvColor[1] = 1;
        mCombineCmd[0] = mCombineCmd[1] = GL_MODULATE;
        mCombineSrc[0][0] = mCombineSrc[1][0] = GL_TEXTURE;
        mCombineSrc[0][1] = mCombineSrc[1][1] = GL_PREVIOUS;
        mCombineSrc[0][2] = mCombineSrc[1][2] = GL_CONSTANT;
        mCombineScale[0] = mCombineScale[1] = 1;
        mCombineOp[0][0] = mCombineOp[0][1] = mCombineOp[0][2] = GL_SRC_COLOR;
        mCombineOp[1][0] = mCombineOp[1][1] = mCombineOp[1][2] = GL_SRC_ALPHA;
      }
      
    protected:
      
      GLenum mWrap[3];
      GLenum mMinFilter;
      GLenum mMagFilter;
      GLenum mEnv;
      GLfloat mEnvColor;
      GLuint mUnit;
      
      GLenum mCombineCmd[2];
      GLenum mCombineSrc[2][3];
      GLenum mCombineOp[2][3];
      GLfloat mCombineScale[2];
  };
}

#endif

