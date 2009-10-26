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

#include <ggfx/shader.h>

using namespace gmath;

namespace ggfx {

  static std::string GetLog(GLhandleARB mObj) {
    GLint logLen = 0;
    glGetObjectParameterivARB(mObj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLen);
    if (logLen > 0) {
      GLchar *buffer = new GLchar[logLen+1];
      GLsizei len = 0;
      glGetInfoLogARB(mObj, logLen, &len, buffer);
      buffer[len] = 0;
      std::string log = std::string(buffer);
      delete[] buffer;
      return log;
    }
    return "";
  }

  // ---

  Parameter::Parameter()
    :mProgObj(0), mName(""), mType(PT_MAX), mSize(0), mId(-1) {
  }
  
  Parameter::Parameter(
    GLhandleARB prog, const std::string &name, Parameter::Type t, GLint sz
  ) :mProgObj(prog), mName(name), mType(t), mSize(sz), mId(-1) {
    mId = glGetUniformLocationARB(mProgObj, name.c_str());
  }
  
  Parameter::Parameter(const Parameter &rhs)
    :mProgObj(rhs.mProgObj), mName(rhs.mName),
     mType(rhs.mType), mSize(rhs.mSize), mId(rhs.mId) {
  }
  
  Parameter::~Parameter() {
  }
  
  Parameter& Parameter::operator=(const Parameter &rhs) {
    if (this != &rhs) {
      mProgObj = rhs.mProgObj;
      mName = rhs.mName;
      mType = rhs.mType;
      mSize = rhs.mSize;
      mId = rhs.mId;
    }
    return *this;
  }
  
  Parameter::Type Parameter::type() const {
    return mType;
  }
  
  GLint Parameter::size() const {
    return mSize;
  }
  
  GLint Parameter::id() const {
    return mId;
  }
  
  const std::string& Parameter::name() const {
    return mName;
  }
  
  void Parameter::set(float v) {
    if (mId >= 0 && mType == PT_FLOAT) {
      glGetError();
      glUniform1fARB(mId, v);
      if (glGetError() != GL_NONE) {
        std::cout << "Failed to set " << TypeToString(mType) << " uniform \"" << mName << "\"" << std::endl;
      }
    }
  }
  
  void Parameter::set(float v0, float v1) {
    if (mId >= 0 && mType == PT_FLOAT2) {
      glGetError();
      glUniform2fARB(mId, v0, v1);
      if (glGetError() != GL_NONE) {
        std::cout << "Failed to set " << TypeToString(mType) << " uniform \"" << mName << "\"" << std::endl;
      }
    }
  }
  
  void Parameter::set(float v0, float v1, float v2) {
    if (mId >= 0 && mType == PT_FLOAT3) {
      glGetError();
      glUniform3fARB(mId, v0, v1, v2);
      if (glGetError() != GL_NONE) {
        std::cout << "Failed to set " << TypeToString(mType) << " uniform \"" << mName << "\"" << std::endl;
      }
    }
  }
  
  void Parameter::set(float v0, float v1, float v2, float v3) {
    if (mId >= 0 && mType == PT_FLOAT4) {
      glGetError();
      glUniform4fARB(mId, v0, v1, v2, v3);
      if (glGetError() != GL_NONE) {
        std::cout << "Failed to set " << TypeToString(mType) << " uniform \"" << mName << "\"" << std::endl;
      }
    }
  }
  
  void Parameter::set(const float *v, int count, bool transpose) {
    if (mId >= 0 && count > 0) {
      glGetError();
      if (mType == PT_FLOAT) {
        glUniform1fvARB(mId, count, v);
      } else if (mType == PT_FLOAT2) {
        glUniform2fvARB(mId, count, v);
      } else if (mType == PT_FLOAT3) {
        glUniform3fvARB(mId, count, v);
      } else if (mType == PT_FLOAT4) {
        glUniform4fvARB(mId, count, v);
      } else if (mType == PT_MATRIX2) {
        glUniformMatrix2fvARB(mId, count, transpose, v);
      } else if (mType == PT_MATRIX3) {
        glUniformMatrix3fvARB(mId, count, transpose, v);
      } else if (mType == PT_MATRIX4) {
        glUniformMatrix4fvARB(mId, count, transpose, v);
      }
      if (glGetError() != GL_NONE) {
        std::cout << "Failed to set " << TypeToString(mType) << " uniform \"" << mName << "\"" << std::endl;
      }
    }
  }
  
  void Parameter::get(float *v) {
    if (mId >= 0) {
      glGetError();
      glGetUniformfvARB(mProgObj, mId, v);
      if (glGetError() != GL_NONE) {
        std::cout << "Failed to get " << TypeToString(mType) << " uniform \"" << mName << "\"" << std::endl;
      }
    }
  }
 
  void Parameter::set(GLint v) {
    if (mId >= 0 && (mType == PT_INT || mType >= PT_SAMPLER_1D)) {
      glGetError();
      glUniform1iARB(mId , v);
      if (glGetError() != GL_NONE) {
        std::cout << "Failed to set " << TypeToString(mType) << " uniform \"" << mName << "\"" << std::endl;
      }
    }
  }
  
  void Parameter::set(GLint v0, GLint v1) {
    if (mId >= 0 && mType == PT_INT2) {
      glGetError();
      glUniform2iARB(mId , v0, v1);
      if (glGetError() != GL_NONE) {
        std::cout << "Failed to set " << TypeToString(mType) << " uniform \"" << mName << "\"" << std::endl;
      }
    }
  }
  
  void Parameter::set(GLint v0, GLint v1, GLint v2) {
    if (mId >= 0 && mType == PT_INT3) {
      glGetError();
      glUniform3iARB(mId , v0, v1, v2);
      if (glGetError() != GL_NONE) {
        std::cout << "Failed to set " << TypeToString(mType) << " uniform \"" << mName << "\"" << std::endl;
      }
    }
    
  }
  
  void Parameter::set(GLint v0, GLint v1, GLint v2, GLint v3) {
    if (mId >= 0 && mType == PT_INT4) {
      glGetError();
      glUniform4iARB(mId , v0, v1, v2, v3);
      if (glGetError() != GL_NONE) {
        std::cout << "Failed to set " << TypeToString(mType) << " uniform \"" << mName << "\"" << std::endl;
      }
    }
  }
  
  void Parameter::set(const GLint *v, int count) {
    if (mId >= 0 && count > 0) {
      glGetError();
      if (mType == PT_INT) {
        glUniform1ivARB(mId, count, v);
      } else if (mType == PT_INT2) {
        glUniform2ivARB(mId, count, v);
      } else if (mType == PT_INT3) {
        glUniform3ivARB(mId, count, v);
      } else if (mType == PT_INT4) {
        glUniform4ivARB(mId, count, v);
      }
      if (glGetError() != GL_NONE) {
        std::cout << "Failed to set " << TypeToString(mType) << " uniform \"" << mName << "\"" << std::endl;
      }
    }
  }
  
  void Parameter::get(GLint *v) {
    if (mId >= 0) {
      glGetError();
      glGetUniformivARB(mProgObj, mId, v);
      if (glGetError() != GL_NONE) {
        std::cout << "Failed to get " << TypeToString(mType) << " uniform \"" << mName << "\"" << std::endl;
      }
    }
  }
  
  Parameter& Parameter::operator=(float f) {
    set(f);
    return *this;
  }
  
  Parameter& Parameter::operator=(GLint i) {
    set(i);
    return *this;
  }
  
  Parameter& Parameter::operator=(const Vector2 &rhs) {
    set(rhs.x, rhs.y);
    return *this;
  }
  
  Parameter& Parameter::operator=(const Vector3 &rhs) {
    set(rhs.x, rhs.y, rhs.z);
    return *this;
  }
  
  Parameter& Parameter::operator=(const Vector4 &rhs) {
    set(rhs.x, rhs.y, rhs.z, rhs.w);
    return *this;
  }
  
  Parameter& Parameter::operator=(const Matrix4 &rhs) {
    if (mId >= 0 && mType == PT_MATRIX4) {
      glGetError();
      glUniformMatrix4fvARB(mId, 1, false, (const float*)rhs);
      if (glGetError() != GL_NONE) {
        std::cout << "Failed to set " << TypeToString(mType) << " uniform \"" << mName << "\"" << std::endl;
      }
    }
    return *this;
  }
  
  GLenum Parameter::TypeToGL(Parameter::Type t) {
    static GLenum GLconv[Parameter::PT_MAX] = {
      GL_FLOAT,
      GL_FLOAT_VEC2_ARB,
      GL_FLOAT_VEC3_ARB,
      GL_FLOAT_VEC4_ARB,
      GL_INT,
      GL_INT_VEC2_ARB,
      GL_INT_VEC3_ARB,
      GL_INT_VEC4_ARB,
      GL_BOOL_ARB,
      GL_BOOL_VEC2_ARB,
      GL_BOOL_VEC3_ARB,
      GL_BOOL_VEC4_ARB,
      GL_FLOAT_MAT2_ARB,
      GL_FLOAT_MAT3_ARB,
      GL_FLOAT_MAT4_ARB,
      GL_SAMPLER_1D_ARB,
      GL_SAMPLER_2D_ARB,
      GL_SAMPLER_3D_ARB,
      GL_SAMPLER_CUBE_ARB,
      GL_SAMPLER_1D_SHADOW_ARB,
      GL_SAMPLER_2D_SHADOW_ARB,
      GL_SAMPLER_2D_RECT_ARB,
      GL_SAMPLER_2D_RECT_SHADOW_ARB
    };
    return GLconv[t];
  }
  
  std::string Parameter::TypeToString(Parameter::Type t) {
    static const char* STRconv[Parameter::PT_MAX] = {
      "float",
      "float2",
      "float3",
      "float4",
      "int",
      "int2",
      "int3",
      "int4",
      "bool",
      "bool2",
      "bool3",
      "bool4",
      "matrix2",
      "matrix3",
      "matrix4",
      "sampler1D",
      "sampler2D",
      "sampler3D",
      "samplerCube",
      "sampler1DShadow",
      "sampler2DShadow",
      "samplerRect",
      "samplerRectShadow"
    };
    return std::string(STRconv[t]);
  }
  
  Parameter::Type Parameter::TypeFromGL(GLenum e) {
    switch (e) {
      case GL_FLOAT: return PT_FLOAT;
      case GL_FLOAT_VEC2_ARB: return PT_FLOAT2;
      case GL_FLOAT_VEC3_ARB: return PT_FLOAT3;
      case GL_FLOAT_VEC4_ARB: return PT_FLOAT4;
      case GL_INT: return PT_INT;
      case GL_INT_VEC2_ARB: return PT_INT2;
      case GL_INT_VEC3_ARB: return PT_INT3;
      case GL_INT_VEC4_ARB: return PT_INT4;
      case GL_BOOL_ARB: return PT_BOOL;
      case GL_BOOL_VEC2_ARB: return PT_BOOL2;
      case GL_BOOL_VEC3_ARB: return PT_BOOL3;
      case GL_BOOL_VEC4_ARB: return PT_BOOL4;
      case GL_FLOAT_MAT2_ARB: return PT_MATRIX2;
      case GL_FLOAT_MAT3_ARB: return PT_MATRIX3;
      case GL_FLOAT_MAT4_ARB: return PT_MATRIX4;
      case GL_SAMPLER_1D_ARB: return PT_SAMPLER_1D;
      case GL_SAMPLER_2D_ARB: return PT_SAMPLER_2D;
      case GL_SAMPLER_3D_ARB: return PT_SAMPLER_3D;
      case GL_SAMPLER_CUBE_ARB: return PT_SAMPLER_CUBE;
      case GL_SAMPLER_1D_SHADOW_ARB: return PT_SAMPLER_SHADOW_1D;
      case GL_SAMPLER_2D_SHADOW_ARB: return PT_SAMPLER_SHADOW_2D;
      case GL_SAMPLER_2D_RECT_ARB: return PT_SAMPLER_RECT;
      case GL_SAMPLER_2D_RECT_SHADOW_ARB: return PT_SAMPLER_SHADOW_RECT;
      default: return PT_MAX;
    }
  }

  // ---

  Shader::Shader(Shader::Type t)
    : mShaderObj(0), mType(t), mRefCount(0) {
    mShaderObj = glCreateShaderObjectARB(
      t == ST_VERTEX ? GL_VERTEX_SHADER_ARB : GL_FRAGMENT_SHADER_ARB);
  }
  
  Shader::~Shader() {
    glDeleteObjectARB(mShaderObj);
  }
  
  Shader::Type Shader::type() const {
    return mType;
  }
  
  std::string Shader::log() const {
    return GetLog(mShaderObj);
  }
  
  bool Shader::loadFromFile(const std::string &fileName) {
    FILE *file = fopen(fileName.c_str(), "r");
    if (file) {
      std::string code = "";
      char line[256];
      while (fgets(line, 256, file)) {
        code += std::string(line);
      }
      fclose(file);
      //std::cout << "=== Source code (\"" << fileName << "\"): " << std::endl << code << std::endl;
      return loadFromSource(code);
    }
    return false;
  }
  
  bool Shader::loadFromSource(const std::string &code) {
    GLsizei len = (GLsizei) code.length();
    const GLchar *pcode = code.c_str();
    glShaderSourceARB(mShaderObj, 1, &pcode, &len);
    glCompileShaderARB(mShaderObj);
    GLint status;
    glGetObjectParameterivARB(mShaderObj, GL_OBJECT_COMPILE_STATUS_ARB, &status);
    return (status != 0);
  }
   
  GLhandleARB Shader::handle() const {
    return mShaderObj;
  }

  // ---
  
  Parameter Program::NullParam = Parameter();

  Program::Program(const std::string &name)
    : mName(name), mProgObj(0), mVert(0), mFrag(0) {
    mProgObj = glCreateProgramObjectARB();
  }
  
  Program::~Program() {
    glDeleteObjectARB(mProgObj);
    if (mVert) {
      mVert->mRefCount -= 1;
    }
    if (mFrag) {
      mFrag->mRefCount -= 1;
    }
  }
  
  const std::string& Program::name() const {
    return mName;
  }
  
  GLhandleARB Program::handle() const {
    return mProgObj;
  }
  
  void Program::bind() {
    glUseProgramObjectARB(mProgObj);
  }
  
  void Program::unbind() {
    glUseProgramObjectARB(0);
  }
  
  Shader* Program::vertex() const {
    return mVert;
  }
  
  Shader* Program::fragment() const {
    return mFrag;
  }
  
  void Program::attach(Shader *shader) {
    if (!shader) {
      return;
    }
    if (shader->type() == Shader::ST_VERTEX) {
      if (mVert) {
        detach(mVert);
      }
      mVert = shader;
      mVert->mRefCount += 1;
    } else {
      if (mFrag) {
        detach(mFrag);
      }
      mFrag = shader;
      mFrag->mRefCount += 1;
    }
    glAttachObjectARB(mProgObj, shader->handle());
  }
  
  void Program::detach(Shader *shader) {
    if (!shader) {
      return;
    }
    if (shader == mVert) {
      mVert->mRefCount -= 1;
      mVert = 0;
    } else if (shader == mFrag){
      mFrag->mRefCount -= 1;
      mFrag = 0;
    } else {
      return;
    }
    glDetachObjectARB(mProgObj, shader->handle());
  }
  
  bool Program::link() {
    glLinkProgramARB(mProgObj);
    GLint status;
    glGetObjectParameterivARB(mProgObj, GL_OBJECT_LINK_STATUS_ARB, &status);
    if (status != 0) {
      fillUniforms();
      return true;
    } else {
      return false;
    }
  }
  
  std::string Program::log() const {
    return GetLog(mProgObj);
  }
  
  void Program::fillUniforms() {
    std::cout << "Program \"" << mName << "\" lookup uniforms..." << std::endl;
    mParams.clear();
    GLint n = 0;
    GLsizei nlen = 0;
    GLint sz = 0;
    GLenum type;
    GLint maxLen;
    glGetObjectParameterivARB(mProgObj, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &n);
    glGetObjectParameterivARB(mProgObj, GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB, &maxLen);
    char *name = new char[maxLen+1];
    for (GLint i=0; i<n; ++i) {
      glGetActiveUniformARB(mProgObj, i, maxLen, &nlen, &sz, &type, name);
      name[nlen] = 0;
      Parameter::Type pt = Parameter::TypeFromGL(type);
      Parameter p(mProgObj, name, pt, sz);
      if (p.id() != -1) { // non-active uniform
        mParams[name] = p;
        std::cout << "  " << name << ", type = " << Parameter::TypeToString(pt)
                  << ", size = " << sz << ", id = " << p.id() << std::endl;
        if (sz > 1) {
          std::string baseName = name;
          size_t pos = baseName.find('[');
          if (pos != std::string::npos) {
            baseName = baseName.substr(0, pos);
          }
          for (GLint i=0; i<sz; ++i) {
            std::ostringstream oss;
            oss << baseName << "[" << i << "]";
            std::string eltName = oss.str();
            p = Parameter(mProgObj, eltName, pt, sz);
            std::cout << "  Array element: " << eltName << ", id = " << p.id() << std::endl;
            mParams[eltName] = p;
          }
        }
      }
    }
    delete[] name;
  }
  
  GLint Program::numUniforms() const {
    return (GLint)(mParams.size());
  }
  
  Parameter* Program::uniform(const std::string &name, GLint n) {
    std::string lookupName = name;
    if (n >= 0) {
      std::ostringstream oss;
      oss << name << "[" << n << "]";
      lookupName = oss.str();
    }
    std::map<std::string, Parameter>::iterator it = mParams.find(lookupName);
    if (it != mParams.end()) {
      return &(it->second);
    } else {
      return &NullParam;
    }
  }
  
  bool Program::hasUniform(const std::string &name, GLint n) {
    return (uniform(name,n) != &NullParam);
  }
  
  Parameter& Program::operator[](const std::string &name) {
    std::map<std::string, Parameter>::iterator it = mParams.find(name);
    if (it != mParams.end()) {
      return it->second;
    } else {
      return NullParam;
    }
  }
  
  const Parameter& Program::operator[](const std::string &name) const {
    std::map<std::string, Parameter>::const_iterator it = mParams.find(name);
    if (it != mParams.end()) {
      return it->second;
    } else {
      return NullParam;
    }
  }
  
  Parameter* Program::uniform(GLint idx) {
    if (idx<0 || idx>=numUniforms()) {
      return &NullParam;
    }
    std::map<std::string, Parameter>::iterator it = mParams.begin();
    for (GLint i=0; i<idx; ++i, ++it);
    return &(it->second);
  }
  
  // ---
  
  ProgramManager* ProgramManager::msInstance = NULL;
  
  ProgramManager& ProgramManager::Instance() {
    if (!msInstance) {
      new ProgramManager();
    }
    return *msInstance;
  }
  
  ProgramManager* ProgramManager::InstancePtr() {
    return msInstance;
  }
  
  ProgramManager::ProgramManager() {
    msInstance = this;
  }
  
  ProgramManager::~ProgramManager() {
    msInstance = NULL;
    ProgramIt pit = mProgramMap.begin();
    while (pit != mProgramMap.end()) {
      delete pit->second;
      ++pit;
    }
    ShaderIt sit = mShaderMap.begin();
    while (sit != mShaderMap.end()) {
      delete sit->second;
      ++sit;
    }
    mProgramMap.clear();
    mShaderMap.clear();
  }
  
  Shader* ProgramManager::getShader(const std::string &name) {
    ShaderIt it = mShaderMap.find(name);
    if (it != mShaderMap.end()) {
      return it->second;
    } else {
      return NULL;
    }
  }
  
  Shader* ProgramManager::createShader(const std::string &name, Shader::Type type, const std::string &srcFile) {
    Shader *sh = getShader(name);
    if (!sh) {
      sh = createShaderFromFile(type, srcFile);
      if (sh) {
        mShaderMap[name] = sh;
        return sh;
      }
    } 
    return NULL;
  }
  
  void ProgramManager::deleteShader(Shader *sh) {
    if (sh && sh->mRefCount == 0) {
      ShaderIt it = mShaderMap.begin();
      while (it != mShaderMap.end()) {
        if (it->second == sh) {
          delete it->second;
          mShaderMap.erase(it);
          return;
        }
        ++it;
      }
    }
  }
  
  void ProgramManager::deleteShader(const std::string &name) {
    ShaderIt it = mShaderMap.find(name);
    if (it != mShaderMap.end() && it->second->mRefCount == 0) {
      delete it->second;
      mShaderMap.erase(it);
    }
  }
  
  Program* ProgramManager::getProgram(const std::string &name) {
    ProgramIt it = mProgramMap.find(name);
    if (it != mProgramMap.end()) {
      return it->second;
    } else {
      return NULL;
    }
  }
  
  Program* ProgramManager::createProgram(const std::string &name, Shader *vs, Shader *fs) {
    if (!vs || !fs) {
      return NULL;
    }
    if (vs->type() == fs->type()) {
      return NULL;
    }
    if (getProgram(name) != NULL) {
      return NULL;
    }
    Program *p = new Program(name);
    p->attach(vs);
    p->attach(fs);
    if (!p->link()) {
      std::cout << "*** Failed to link program \"" << name << "\"" << std::endl;
      std::cout << p->log() << std::endl;
      delete p;
      return NULL;
    }
    mProgramMap[name] = p;
    return p;
  }
  
  Program* ProgramManager::createProgram(const std::string &name,
    const std::string &vsName, const std::string &vsSrcFile,
    const std::string &fsName, const std::string &fsSrcFile)
  {
    Program *prg = getProgram(name);
    if (!prg) {
      Shader *vs = getShader(vsName);
      if (!vs) {
        vs = createShader(vsName, Shader::ST_VERTEX, vsSrcFile);
        if (!vs) {
          return NULL;
        }
      } else if (vs->type() != Shader::ST_VERTEX) {
        return NULL;
      }
      Shader *fs = getShader(fsName);
      if (!fs) {
        fs = createShader(fsName, Shader::ST_FRAGMENT, fsSrcFile);
        if (!fs) {
          return NULL;
        }
      } else if (fs->type() != Shader::ST_FRAGMENT) {
        return NULL;
      }
      // even if program creation failed, keep the created vs and fs)
      return createProgram(name, vs, fs);
    }
    return NULL;
  }
  
  Program* ProgramManager::createProgram(const std::string &name, const std::string &vsSrc, const std::string &fsSrc) {
    std::string vsName = name+"_vs";
    std::string fsName = name+"_fs";
    return createProgram(name, vsName, vsSrc, fsName, fsSrc);
  }
  
  void ProgramManager::deleteProgram(Program *prg) {
    if (prg) {
      ProgramIt it = mProgramMap.begin();
      while (it != mProgramMap.end()) {
        if (it->second == prg) {
          delete it->second;
          mProgramMap.erase(it);
          return;
        }
        ++it;
      }
    }
  }
  
  void ProgramManager::deleteProgram(const std::string &name) {
    ProgramIt it = mProgramMap.find(name);
    if (it != mProgramMap.end()) {
      delete it->second;
      mProgramMap.erase(it);
    }
  }
  
  Shader* ProgramManager::createShaderFromFile(Shader::Type type, const std::string &fileName) {
    Shader *s = new Shader(type);
    if (!s->loadFromFile(fileName)) {
      std::cout << "*** Failed to compile \"" << fileName << "\"" << std::endl;
      std::cout << s->log() << std::endl;
      delete s;
      return NULL;
    }
    return s;
  }
  
}

