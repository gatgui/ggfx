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

#ifndef __ggfx_Mesh_h_
#define __ggfx_Mesh_h_

#include <ggfx/vertexformat.h>
#include <ggfx/bufferobject.h>
#include <ggfx/object.h>

namespace ggfx {
  
  // change int with size_t for indices/vertices count
  
  class GGFX_API SubMesh : public RenderableObject {
    
    public:
    
      enum DrawMode {
        DM_ARRAY = 0,
        DM_INDEXED,
        DM_UNKNOWN
      };
      
      enum PrimitiveType {
        PT_QUAD = 0,
        PT_QUAD_STRIP,
        PT_TRI,
        PT_TRI_FAN,
        PT_TRI_STRIP,
        PT_LINE,
        PT_LINE_STRIP,
        PT_LINE_LOOP,
        PT_POINT,
        PT_MAX
      };
      
      enum IndexType {
        IT_8 = 0,
        IT_16,
        IT_32,
        IT_MAX
      };
      
      friend class Mesh;
      
      SubMesh(class Mesh *parent = NULL);
      virtual ~SubMesh();
      
      bool create(
        PrimitiveType ptype, //BufferObject::Usage usage,
        VertexFormat *fmt, size_t nVertices,
        bool allocClientSideData=false);
      
      bool createIndexed(
        PrimitiveType ptype, //BufferObject::Usage usage,
        VertexFormat *fmt, size_t nVertices,
        IndexType itype, size_t nIndices,
        bool allocClientSideData=false);
      
      bool createShared(
        PrimitiveType ptype, //BufferObject::Usage usage,
        IndexType itype, size_t nIndices,
        bool allocClientSideData=false);
      
      virtual void render(class Renderer *r) const;
      
      // will dissapear
      // because we need to know through renderer if a shaders are bound
      // which texcoord set for which unit etc
      void render() const;
      
      inline BufferObject* getVertexBuffer() const {
        return mVBuffer;
      }
    
      inline BufferObject* getIndexBuffer() const {
        return mIBuffer;
      }
    
      inline void* getClientVertexBuffer() const {
        return mVData;
      }
    
      inline void* getClientIndexBuffer() const {
        return mIData;
      }

      inline VertexFormat* getFormat() const {
        return mFormat;
      }
      
      inline DrawMode getDrawMode() const {
        return mDrawMode;
      }
      
      inline bool isIndexed() const {
        return (mIdxType != IT_MAX);
      }
      
      inline IndexType getIndexType() const {
        return mIdxType;
      }
      
      inline PrimitiveType getPrimitiveType() const {
        return mPrimType;
      }
      
      inline class Mesh* getParentMesh() const {
        return mParentMesh;
      }
      
      inline bool usesSharedVertices() const {
        return mSharedVertices;
      }
      
      inline size_t getNumVertices() const {
        return mNumVertices;
      }
      
      inline size_t getNumIndices() const {
        return mNumIndices;
      }
      
      // add some utility functions probably

    protected:
      
      void cleanup();
      
      DrawMode mDrawMode;
      BufferObject *mVBuffer;
      BufferObject *mIBuffer;
      void *mVData;
      void *mIData;
      VertexFormat *mFormat;
      size_t mNumVertices;
      size_t mNumIndices;
      IndexType mIdxType;
      PrimitiveType mPrimType;
      class Mesh *mParentMesh;
      bool mSharedVertices;
      bool mInvalidSharedVertices;
  };
  
  // Mesh == a collection of sub mesh
  // each sub mesh can have a different material
  // if sub mesh do not have a material assigned
  // use the material from the mesh
  // in the same way if no material assigned to mesh
  // look up in node
  // for nodes look in parent
  // finaly, for root, get scene default material
  // scene will sort object using their material (ambient, diffuse, specular, alpha etc)
  
  // Mesh owns its sub-mesh
  
  class GGFX_API Mesh : public AttachableObject {
    
    public:
      
      friend class MeshManager;
      
      typedef std::vector<SubMesh*> SubMeshList;
      typedef SubMeshList::iterator SubMeshIt;
      typedef SubMeshList::const_iterator SubMeshConstIt;
      
      Mesh();
      Mesh(const std::string &name);
      virtual ~Mesh();
      
      void setMaterial(Material *mat, bool applyToSubMeshes=false);
      
      bool createSharedVertices(
        VertexFormat *fmt, size_t nVertices,
        bool allocClientSideData=false);
      
      void addSubMesh(SubMesh *sm);
      void removeSubMesh(SubMesh *sm);
      
      inline size_t getNumSubMeshes() const {
        return mSubMeshes.size();
      }
      
      inline SubMesh* getSubMesh(size_t idx) {
        return (idx < getNumSubMeshes() ? mSubMeshes[idx] : NULL);
      }
      
      inline Material* getMaterial() const {
        return mMaterial;
      }
      
      inline const std::string& getName() const {
        return mName;
      }
      
      inline VertexFormat* getSharedFormat() const {
        return mFormat;
      }
      
      inline BufferObject* getSharedVertexBuffer() const {
        return mVBuffer;
      }
      
      inline void* getClientSharedVertexBuffer() const {
        return mVData;
      }
      
      inline size_t getNumSharedVertices() const {
        return mNumVertices;
      }
      
      inline class MeshFactory* getFactory() const {
        return mFactory;
      }
      
    protected:
      
      void cleanupSharedVertices();
      
      SubMeshList mSubMeshes;
      Material *mMaterial;
      std::string mName;
      
      VertexFormat *mFormat;
      BufferObject *mVBuffer;
      void *mVData;
      size_t mNumVertices;
      class MeshFactory *mFactory;
  };
  
  
  class GGFX_API MeshFactory {
    public:
      
      MeshFactory();
      virtual ~MeshFactory();
      
      virtual const std::string& getName() const = 0;
      virtual bool canLoad(const std::string &fileName) const = 0;
      virtual const std::string& getExtension() const = 0;
      virtual Mesh* create(const std::string &fileName, BufferObject::Usage usage, const std::string &name, float scl=1.0f) = 0;
      virtual void destroy(Mesh *mesh) = 0;
  };
  
  
  class GGFX_API MeshManager { // Maybe a super class Manager
    public:
      
      MeshManager();
      virtual ~MeshManager();
      
      static MeshManager& Instance();
      static MeshManager* InstancePtr();
      
      void registerFactory(MeshFactory *factory);
      void unregisterFactory(MeshFactory *factory);
      size_t getNumFactories() const;
      MeshFactory* getFactory(size_t idx) const;
      MeshFactory* getFactoryByName(const std::string &name) const;
      MeshFactory* getFactoryByExtension(const std::string &ext) const;
      
      Mesh* create(const std::string &fileName, BufferObject::Usage usage, const std::string &name, float scl=1.0f);
      void destroy(Mesh *mesh);
      
    private:
      
      std::vector<MeshFactory*> mFactories;
      
      static MeshManager *msInstance;
  };
}

#endif


