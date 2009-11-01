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

#ifndef __ggfx_shader_h_
#define __ggfx_shader_h_

#include <ggfx/config.h>

namespace ggfx {
  
  // AUTO PARAMS ? not all needed for GL
  // by can provides some others ^^
  
  class GGFX_API Parameter {
    
    public:
      
      enum Type {
        PT_FLOAT = 0,
        PT_FLOAT2,
        PT_FLOAT3,
        PT_FLOAT4,
        PT_INT,
        PT_INT2,
        PT_INT3,
        PT_INT4,
        PT_BOOL,
        PT_BOOL2,
        PT_BOOL3,
        PT_BOOL4,
        PT_MATRIX2,
        PT_MATRIX3,
        PT_MATRIX4,
        PT_SAMPLER_1D,
        PT_SAMPLER_2D,
        PT_SAMPLER_3D,
        PT_SAMPLER_CUBE,
        PT_SAMPLER_SHADOW_1D,
        PT_SAMPLER_SHADOW_2D,
        PT_SAMPLER_RECT,
        PT_SAMPLER_SHADOW_RECT,
        PT_MAX
      };
      
      
      Parameter();
      Parameter(GLhandleARB prog, const std::string &name, Type t, GLint sz);
      Parameter(const Parameter &rhs);
      ~Parameter();
      
      Parameter& operator=(const Parameter &rhs);
      
      Type type() const;
      GLint size() const;
      GLint id() const;
      const std::string& name() const;
      
      void set(float v);
      void set(float v0, float v1);
      void set(float v0, float v1, float v2);
      void set(float v0, float v1, float v2, float v3);
      void set(const float *v, int count=1, bool transpose=false);
      
      void set(GLint v);
      void set(GLint v0, GLint v1);
      void set(GLint v0, GLint v1, GLint v2);
      void set(GLint v0, GLint v1, GLint v2, GLint v3);
      void set(const GLint *v, int count=1);
      
      void get(float *v);
      void get(GLint *v);
      
      Parameter& operator=(float f);
      Parameter& operator=(GLint i);
      Parameter& operator=(const gmath::Vector2 &rhs);
      Parameter& operator=(const gmath::Vector3 &rhs);
      Parameter& operator=(const gmath::Vector4 &rhs);
      Parameter& operator=(const gmath::Matrix4 &rhs);
      
      static GLenum TypeToGL(Type t);
      static std::string TypeToString(Type t);
      static Type TypeFromGL(GLenum e);
      
    protected:
      
      GLhandleARB mProgObj;
      std::string mName;
      Type mType;
      GLint mSize;
      GLint mId;
  };

  class GGFX_API Shader {
    
    public:
      
      friend class Program;
      friend class ProgramManager;
      
      enum Type {
        ST_VERTEX,
        ST_FRAGMENT
      };
    
    protected:
      
      Shader(Type t);
      virtual ~Shader();
      
    public:
      
      Type type() const;
      std::string log() const;
      GLhandleARB handle() const;
      
      bool loadFromFile(const std::string &fileName);
      bool loadFromSource(const std::string &code);
      
    protected:
      
      GLhandleARB mShaderObj;
      Type mType;
      unsigned int mRefCount;
  };
  
  class GGFX_API Program {
    
    protected:
      
      Program(const std::string &name);
      virtual ~Program();
      
    public:
      
      friend class ProgramManager;
      
      void bind();
      void unbind();
      
      void attach(Shader *shader);
      void detach(Shader *shader);
      bool link();
      void fillUniforms();
      
      const std::string& name() const;
      GLhandleARB handle() const;
      Shader* vertex() const;
      Shader* fragment() const;
      std::string log() const;
      
      GLint numUniforms() const;
      Parameter* uniform(const std::string &name, GLint n=-1);
      bool hasUniform(const std::string &name, GLint n=-1);
      Parameter* uniform(GLint idx);
      Parameter& operator[](const std::string &name);
      const Parameter& operator[](const std::string &name) const;
      
    protected:
    
      std::string mName;
      GLhandleARB mProgObj;
      Shader *mVert;
      Shader *mFrag;
      std::map<std::string, Parameter> mParams;
      
      static Parameter NullParam;
  };
  
  class GGFX_API ProgramManager {
    
    protected:
    
      ProgramManager();
    
    public:
      
      typedef std::map<std::string, Program*> ProgramMap;
      typedef ProgramMap::iterator ProgramIt;
      typedef ProgramMap::const_iterator ProgramConstIt;
      
      typedef std::map<std::string, Shader*> ShaderMap;
      typedef ShaderMap::iterator ShaderIt;
      typedef ShaderMap::const_iterator ShaderConstIt;
      
      static ProgramManager& Instance();
      static ProgramManager* InstancePtr();
      
      virtual ~ProgramManager();
      
      Shader* getShader(const std::string &name);
      Shader* createShader(const std::string &name, Shader::Type type, const std::string &srcFile);
      void deleteShader(Shader *sh);
      void deleteShader(const std::string &name);
      
      Program* getProgram(const std::string &name);
      Program* createProgram(const std::string &name, Shader *vs, Shader *fs);
      Program* createProgram(const std::string &name,
        const std::string &vsSrc, const std::string &fsSrc);
      Program* createProgram(const std::string &name,
        const std::string &vsName, const std::string &vsSrcFile,
        const std::string &fsName, const std::string &fsSrcFile);
      void deleteProgram(Program *prg);
      void deleteProgram(const std::string &name);
      
    protected:
    
      static ProgramManager *msInstance;
      
      Shader* createShaderFromFile(Shader::Type type, const std::string &fileName);
      
      ProgramMap mProgramMap;
      ShaderMap mShaderMap;
  };


}

#endif

