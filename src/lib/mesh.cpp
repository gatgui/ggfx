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

#include <ggfx/mesh.h>
#include <ggfx/renderer.h>

using namespace std;

namespace ggfx {
  
  // ---
  
  static GLenum GLprim[SubMesh::PT_MAX] = {
    GL_QUADS,
    GL_QUAD_STRIP,
    GL_TRIANGLES,
    GL_TRIANGLE_FAN,
    GL_TRIANGLE_STRIP,
    GL_LINES,
    GL_LINE_STRIP,
    GL_LINE_LOOP,
    GL_POINTS
  };
  
  static GLenum GLidxType[SubMesh::IT_MAX] = {
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_SHORT,
    GL_UNSIGNED_INT
  };
  
  static size_t GLidxSize[SubMesh::IT_MAX] = {
    sizeof(GLubyte),
    sizeof(GLushort),
    sizeof(GLuint)
  };
  
  static GLenum GLtype[VAT_MAX] = {
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_SHORT,
    GL_UNSIGNED_INT,
    GL_BYTE,
    GL_SHORT,
    GL_INT,
    GL_FLOAT,
    GL_DOUBLE
  };
    
  // ---

  SubMesh::SubMesh(Mesh *parent)
    : mDrawMode(DM_UNKNOWN),
      mVBuffer(0), mIBuffer(0), mVData(0), mIData(0),
      mFormat(0), mNumVertices(0), mNumIndices(0),
      mIdxType(IT_MAX), mPrimType(PT_MAX), mParentMesh(NULL),
      mSharedVertices(false), mInvalidSharedVertices(false) {
    if (parent) {
      parent->addSubMesh(this);
    }
  }
  
  SubMesh::~SubMesh() {
    cleanup();
    if (mVBuffer) {
      delete mVBuffer;
    }
    if (mIBuffer) {
      delete mIBuffer;
    }
  }
  
  void SubMesh::cleanup() {
    if (mFormat) {
      mFormat->destroy(mVData);
    }
    if (mIData) {
      free(mIData);
    }
    // Do not delete VBOs !
    //if (mVBuffer) {
    //  delete mVBuffer;
    //  mVBuffer = 0;
    //}
    //if (mIBuffer) {
    //  delete mIBuffer;
    //  mIBuffer = 0;
    //}
    mNumVertices = 0;
    mNumIndices = 0;
    mFormat = 0;
    mVData = 0;
    mIData = 0;
    mIdxType = IT_MAX;
    mPrimType = PT_MAX;
    mDrawMode = DM_UNKNOWN;
    mSharedVertices = false;
    mInvalidSharedVertices = false;
  }
  
  bool SubMesh::create(
    SubMesh::PrimitiveType ptype,
    VertexFormat *fmt, size_t nVertices, bool allocClientSideData)
  {
    if (!fmt || ptype<0 || ptype>=PT_MAX) {
      return false;
    }
    cleanup();
    mPrimType = ptype;
    mFormat = fmt;
    mNumVertices = nVertices;
    mDrawMode = DM_ARRAY;
    if (allocClientSideData) {
      mVData = mFormat->allocate(nVertices);
    }
    if (!mVBuffer) {
      mVBuffer = new BufferObject(BufferObject::BOT_VERTEX);
    }
    return true;
  }
  
  bool SubMesh::createIndexed(
    SubMesh::PrimitiveType ptype,
    VertexFormat *fmt, size_t nVertices,
    SubMesh::IndexType itype, size_t nIndices, bool allocClientSideData)
  {
    if (!fmt || ptype<0 || ptype>=PT_MAX || itype<0 || itype>=IT_MAX) {
      return false;
    }
    cleanup();
    mIdxType = itype;
    mPrimType = ptype;
    mFormat = fmt;
    mNumVertices = nVertices;
    mNumIndices = nIndices;
    mDrawMode = DM_INDEXED;
    if (allocClientSideData) {
      mVData = mFormat->allocate(nVertices);
      mIData = malloc(nIndices*GLidxSize[mIdxType]);
    }
    if (!mVBuffer) {
      mVBuffer = new BufferObject(BufferObject::BOT_VERTEX);
    }
    if (!mIBuffer) {
      mIBuffer = new BufferObject(BufferObject::BOT_INDEX);
    }
    return true;
  }
  
  bool SubMesh::createShared(
    SubMesh::PrimitiveType ptype,
    SubMesh::IndexType itype, size_t nIndices,
    bool allocClientSideData)
  {
    if (ptype<0 || ptype>=PT_MAX || itype<0 || itype>=IT_MAX) {
      return false;
    }
    cleanup();
    mIdxType = itype;
    mPrimType = ptype;
    mNumIndices = nIndices;
    mSharedVertices = true;
    mDrawMode = DM_INDEXED;
    if (allocClientSideData) {
      mIData = malloc(nIndices*GLidxSize[mIdxType]);
    }
    if (!mIBuffer) {
      mIBuffer = new BufferObject(BufferObject::BOT_INDEX);
    }
    return true;
  }
  
  void SubMesh::render(Renderer *) const {
    // for now
    render();
  }
  
  void SubMesh::render() const {
    
    if (mDrawMode == DM_UNKNOWN || (mDrawMode == DM_INDEXED && !mIBuffer)) {
      return;
    }
    
    VertexFormat *fmt = NULL;
    BufferObject *vbo = NULL;
    size_t nv = 0;
    
    if (mSharedVertices == true) {
      if (mParentMesh != NULL && !mInvalidSharedVertices) {
        fmt = mParentMesh->getSharedFormat();
        vbo = mParentMesh->getSharedVertexBuffer();
        nv = mParentMesh->getNumSharedVertices();
      }
    } else {
      fmt = mFormat;
      vbo = mVBuffer;
      nv = mNumVertices;
    }
    
    if (!fmt || !vbo || !fmt->isValid()) {
      return;
    }
    
    vbo->bind();

    GLvoid *ptr = NULL;

    GLsizei stride = (GLsizei) fmt->getBytesSize();

    size_t i;

    // this is now how it should works, we might use the same coord set with
    // different texture units ... isn't it ? does it makes sense in fact ?
    // for fixed pipeline yes: tex unit 0 uses tex coords 0
    
    //GLint ntc=0;
    //glGetIntegerv(GL_MAX_TEXTURE_COORDS, &ntc);
    //cout << "Max texcoords = " << ntc << endl;
    
    for (i=0; i<8; ++i) {
      glClientActiveTextureARB(GLenum(GL_TEXTURE0_ARB+i));
      if (i < fmt->getNumTexcoords()) {
        const VertexAttribute &ta = fmt->getAttribute(VAS_TEXCOORD, i);
        ptr = (GLvoid*)(ta.getBytesOffset());
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(ta.getNumElements(), GLtype[ta.getType()], stride, ptr);
      } else {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      }
    }
    
    if (fmt->hasAttribute(VAS_COLOR0)) {
      const VertexAttribute &ca = fmt->getAttribute(VAS_COLOR0);
      ptr = (GLvoid*)(ca.getBytesOffset());
      glEnableClientState(GL_COLOR_ARRAY);
      glColorPointer(ca.getNumElements(), GLtype[ca.getType()], stride, ptr);
    } else {
      glDisableClientState(GL_COLOR_ARRAY);
    }
    
    if (fmt->hasAttribute(VAS_COLOR1)) {
      const VertexAttribute &ca = fmt->getAttribute(VAS_COLOR1);
      ptr = (GLvoid*)(ca.getBytesOffset());
      glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
      glSecondaryColorPointer(ca.getNumElements(), GLtype[ca.getType()], stride, ptr);
    } else {
      glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    }

    if (fmt->hasAttribute(VAS_NORMAL)) {
      const VertexAttribute &na = fmt->getAttribute(VAS_NORMAL);
      ptr = (GLvoid*)(na.getBytesOffset());
      glEnableClientState(GL_NORMAL_ARRAY);
      glNormalPointer(GLtype[na.getType()], stride, ptr);
    } else {
      glDisableClientState(GL_NORMAL_ARRAY);
    }
    
    // for now skip Generic attributes [requires shader knowledge]

    const VertexAttribute &va = fmt->getAttribute(VAS_POSITION);
    ptr = (GLvoid*)(va.getBytesOffset());
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(va.getNumElements(), GLtype[va.getType()], stride, ptr);
    
    if (mDrawMode == DM_INDEXED) {
      mIBuffer->bind();
      glDrawElements(GLprim[mPrimType], GLuint(mNumIndices), GLidxType[mIdxType], NULL);
      mIBuffer->unbind();
    } else {
      glDrawArrays(GLprim[mPrimType], 0, GLsizei(nv));
    }
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    for (i=0; i<8; ++i) {
      glClientActiveTextureARB(GLenum(GL_TEXTURE0_ARB+i));
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    
    vbo->unbind();
  }
  
  // ---
  
  Mesh::Mesh()
    : AttachableObject(), mName("aMesh"),
      mFormat(NULL), mVBuffer(NULL), mVData(NULL), mNumVertices(0),
      mFactory(NULL) {
  }
  
  Mesh::Mesh(const std::string &name)
    : AttachableObject(), mName(name), 
      mFormat(NULL), mVBuffer(NULL), mVData(NULL), mNumVertices(0),
      mFactory(NULL) {
  }

  Mesh::~Mesh() {
    for (size_t i=0; i<mSubMeshes.size(); ++i) {
      delete mSubMeshes[i];
    }
    mSubMeshes.clear();
    if (mFormat) {
      mFormat->destroy(mVData);
    }
    if (mVBuffer) {
      delete mVBuffer;
    }
  }
  
  void Mesh::cleanupSharedVertices() {
    if (mFormat) {
      mFormat->destroy(mVData);
    }
    // Do not destroy VBO
    //if (mVBuffer) {
    //  delete mVBuffer;
    //  mVBuffer = 0;
    //}
    mFormat = NULL;
    mNumVertices = 0;
    mVData = NULL;
    SubMeshIt it = mSubMeshes.begin();
    while (it != mSubMeshes.end()) {
      if ((*it)->usesSharedVertices()) {
        (*it)->mInvalidSharedVertices = true;
      } else {
        ++it;
      }
    }
  }
  
  bool Mesh::createSharedVertices(
    VertexFormat *fmt, size_t nVertices,
    bool allocClientSideData)
  {
    if (!fmt) {
      return false;
    }
    cleanupSharedVertices();
    mFormat = fmt;
    mNumVertices = nVertices;
    if (allocClientSideData) {
      mVData = mFormat->allocate(nVertices);
    }
    if (!mVBuffer) {
      mVBuffer = new BufferObject(BufferObject::BOT_VERTEX);
    }
    return true;
  }
  
  void Mesh::setMaterial(Material *mat, bool applyToSubMeshes) {
    mMaterial = mat;
    if (applyToSubMeshes) {
      for (size_t i=0; i<mSubMeshes.size(); ++i) {
        mSubMeshes[i]->setMaterial(mat);
      }
    }
  }
  
  void Mesh::addSubMesh(SubMesh *sm) {
    if (sm->getParentMesh()) {
      if (sm->getParentMesh() == this) {
        return;
      }
      sm->getParentMesh()->removeSubMesh(sm);
    }
    sm->mParentMesh = this;
    mSubMeshes.push_back(sm);
  }
  
  void Mesh::removeSubMesh(SubMesh *sm) {
    if (sm->getParentMesh() == this) {
      SubMeshIt it = std::find(mSubMeshes.begin(), mSubMeshes.end(), sm);
      if (it != mSubMeshes.end()) {
        mSubMeshes.erase(it);
        sm->mParentMesh = NULL;
      }
    }
  }
  
  // ---
  
  MeshFactory::MeshFactory() {
  }
  
  MeshFactory::~MeshFactory() {
  }
  
  // ---
  
  MeshManager *MeshManager::msInstance = NULL;
  
  MeshManager::MeshManager() {
    assert(msInstance == NULL);
    msInstance = this;
  }
  
  MeshManager::~MeshManager() {
    msInstance = NULL;
  }
      
  MeshManager& MeshManager::Instance() {
    if (!msInstance) {
      new MeshManager();
    }
    return *msInstance;
  }
  
  MeshManager* MeshManager::InstancePtr() {
    return msInstance;
  }
  
  void MeshManager::registerFactory(MeshFactory *factory) {
    if (factory) {
      if (getFactoryByExtension(factory->getExtension()) != 0) {
        return;
      }
      mFactories.push_back(factory);
    }
  }
  
  void MeshManager::unregisterFactory(MeshFactory *factory) {
    for (size_t i=0; i<mFactories.size(); ++i) {
      if (mFactories[i] == factory) {
        mFactories.erase(mFactories.begin()+i);
      }
    }
  }
  
  size_t MeshManager::getNumFactories() const {
    return mFactories.size();
  }
  
  MeshFactory* MeshManager::getFactory(size_t idx) const {
    return (idx < getNumFactories() ? mFactories[idx] : NULL);
  }
  
  MeshFactory* MeshManager::getFactoryByName(const std::string &name) const {
    for (size_t i=0; i<mFactories.size(); ++i) {
      if (mFactories[i]->getName() == name) {
        return mFactories[i];
      }
    }
    return NULL;
  }

  MeshFactory* MeshManager::getFactoryByExtension(const std::string &ext) const {
    for (size_t i=0; i<mFactories.size(); ++i) {
#ifdef WIN32
      if (stricmp(mFactories[i]->getExtension().c_str(), ext.c_str()) == 0) {
#else
      if (strcasecmp(mFactories[i]->getExtension().c_str(), ext.c_str()) == 0) {
#endif
        return mFactories[i];
      }
    }
    return NULL;
  }
  
  Mesh* MeshManager::create(const std::string &fileName, BufferObject::Usage usage, const std::string &name, float scl) {
    for (size_t i=0; i<mFactories.size(); ++i) {
      if (mFactories[i]->canLoad(fileName)) {
        Mesh *m = mFactories[i]->create(fileName, usage, name, scl);
        if (m) {
          m->mFactory = mFactories[i];
        } else {
          cout << "Factory(" << mFactories[i]->getName()
               << "): Failed to load file \"" << fileName << "\"" << endl;
        }
        return m;
      }
    }
    return NULL;
  }
  
  void MeshManager::destroy(Mesh *mesh) {
    if (mesh) {
      if (mesh->getFactory()) {
        mesh->getFactory()->destroy(mesh);
      } else {
        delete mesh;
      }
    }
  }
  
}

