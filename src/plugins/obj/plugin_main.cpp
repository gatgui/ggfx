#include <ggfx/ggfx>
#include <set>
#include "objmesh.h"

using namespace std;
using namespace gmath;
using namespace ggfx;

struct VertexEntry {
  long v;
  long n;
  long t;
  VertexEntry()
    :v(-1), n(-1), t(-1) {
  }
  VertexEntry(const ObjTri &tri, unsigned int idx) {
    v = long(tri.v[idx]);
    n = tri.has_norm ? long(tri.n[idx]) : -1;
    t = tri.has_texc ? long(tri.t[idx]) : -1;
  }
  VertexEntry(const VertexEntry &rhs)
    :v(rhs.v), n(rhs.n), t(rhs.t) {
  }
  ~VertexEntry() {
  }
  VertexEntry& operator=(const VertexEntry &rhs) {
    v = rhs.v;
    n = rhs.n;
    t = rhs.t;
    return *this;
  }
  bool operator == (const VertexEntry &rhs) const {
    return ((v == rhs.v) && (n == rhs.n) && (t == rhs.t));
  }
  bool operator != (const VertexEntry &rhs) const {
    return !operator==(rhs);
  }
  bool operator < (const VertexEntry &rhs) const {
    return ((v < rhs.v) && (n < rhs.n) && (t < rhs.t));
  }
};

static Mesh* ObjLoad(const std::string &fileName, BufferObject::Usage usage, const std::string &name, float scl=1.0f) {
  ObjMesh mesh;
  
  if (mesh.load(fileName)) {
    
    size_t i;
    
    std::string vfmt = "position-float-3";
    if (mesh.normals.size() > 0) {
      vfmt += "|normal-float-3";
    }
    if (mesh.texcoords.size() > 0) {
      vfmt += "|texcoord-float-2";
    }
    VertexFormat *fmt = VertexFormat::Get(vfmt);
    
    // compute bbox
    AABox box;
    for (i=0; i<mesh.vertices.size(); ++i) {
      if (i==0) {
        box.setMin(mesh.vertices[i]);
        box.setMax(box.getMin());
      } else {
        box.merge(mesh.vertices[i]);
      }
    }
    Vector3 center = 0.5f * (box.getMin() + box.getMax());
    
    // scale vertices [does not affect normal as it is a uniform scale]
    for (i=0; i<mesh.vertices.size(); ++i) {
      mesh.vertices[i] = center + scl * (mesh.vertices[i] - center);
    }
    box.setMin(center+scl*(box.getMin()-center));
    box.setMax(center+scl*(box.getMax()-center));
    
    // figure out unique vertices
    vector<VertexEntry> uniqVertices;
    for (i=0; i<mesh.tris.size(); ++i) {
      ObjTri &tri = mesh.tris[i];
      for (int j=0; j<3; ++j) {
        VertexEntry ve(tri, j);
        if (find(uniqVertices.begin(), uniqVertices.end(), ve) == uniqVertices.end()) {
          uniqVertices.push_back(ve);
        }
      }
    }
    
    //map<VertexEntry, size_t> indexMap;
    
    Mesh *m = new Mesh(name);
    m->createSharedVertices(fmt, uniqVertices.size());
    size_t dataSize = uniqVertices.size() * fmt->getBytesSize();
    void *data = malloc(dataSize);
    float *vals = (float*)data;
    for (i=0; i<uniqVertices.size(); ++i) {
      long v = uniqVertices[i].v;
      long n = uniqVertices[i].n;
      long t = uniqVertices[i].t;
      *vals++ = mesh.vertices[v].x;
      *vals++ = mesh.vertices[v].y;
      *vals++ = mesh.vertices[v].z;
      if (fmt->hasAttribute(VAS_NORMAL)) {
        *vals++ = mesh.normals[n].x;
        *vals++ = mesh.normals[n].y;
        *vals++ = mesh.normals[n].z;
      }
      if (fmt->hasAttribute(VAS_TEXCOORD)) {
        if (uniqVertices[i].t == -1) {
          *vals++ = 0.0f;
          *vals++ = 0.0f;
        } else {
          *vals++ = mesh.texcoords[t].x;
          *vals++ = mesh.texcoords[t].y;
        }
      }
    }
    m->getSharedVertexBuffer()->upload(dataSize, data, usage);
    free(data);
    
    SubMesh::IndexType itype;
    if (uniqVertices.size() < 256) {
      itype = SubMesh::IT_8;
    } else if (uniqVertices.size() < 65536) {
      itype = SubMesh::IT_16;
    } else {
      itype = SubMesh::IT_32;
    }
    
    map<long, vector<size_t> > matGroups;
    
    for (i=0; i<mesh.tris.size(); ++i) {
      long midx = mesh.tris[i].midx;
      matGroups[midx].push_back(i);
    }
    
    map<long, vector<size_t> >::iterator it = matGroups.begin();
    
    while (it != matGroups.end()) {
      
      long midx = it->first;
      vector<size_t> &tris = it->second;
      
      size_t ntri = tris.size();
      
      size_t dataSize = 0;
      void *data = NULL;
      VertexEntry ve;
      size_t vidx;
      
      if (itype == SubMesh::IT_8) {
        
        dataSize = ntri * 3 * sizeof(GLubyte);
        data = malloc(dataSize);
        GLubyte *vals = (GLubyte*)data;
        
        for (i=0; i<tris.size(); ++i) {
          size_t triIdx = tris[i];
          ObjTri &tri = mesh.tris[triIdx];
          ve = VertexEntry(tri, 0);
          vidx = find(uniqVertices.begin(), uniqVertices.end(), ve) - uniqVertices.begin();
          *vals++ = (GLubyte) vidx;
          ve = VertexEntry(tri, 1);
          vidx = find(uniqVertices.begin(), uniqVertices.end(), ve) - uniqVertices.begin();
          *vals++ = (GLubyte) vidx;
          ve = VertexEntry(tri, 2);
          vidx = find(uniqVertices.begin(), uniqVertices.end(), ve) - uniqVertices.begin();
          *vals++ = (GLubyte) vidx;
        }
      
      } else if (itype == SubMesh::IT_16) {
        
        dataSize = ntri * 3 * sizeof(GLushort);
        data = malloc(dataSize);
        GLushort *vals = (GLushort*)data;
        
        for (i=0; i<tris.size(); ++i) {
          size_t triIdx = tris[i];
          ObjTri &tri = mesh.tris[triIdx];
          ve = VertexEntry(tri, 0);
          vidx = find(uniqVertices.begin(), uniqVertices.end(), ve) - uniqVertices.begin();
          *vals++ = (GLushort) vidx;
          ve = VertexEntry(tri, 1);
          vidx = find(uniqVertices.begin(), uniqVertices.end(), ve) - uniqVertices.begin();
          *vals++ = (GLushort) vidx;
          ve = VertexEntry(tri, 2);
          vidx = find(uniqVertices.begin(), uniqVertices.end(), ve) - uniqVertices.begin();
          *vals++ = (GLushort) vidx;
        }
        
      } else {
        
        dataSize = ntri * 3 * sizeof(GLuint);
        data = malloc(dataSize);
        GLuint *vals = (GLuint*)data;
        
        for (i=0; i<tris.size(); ++i) {
          size_t triIdx = tris[i];
          ObjTri &tri = mesh.tris[triIdx];
          ve = VertexEntry(tri, 0);
          vidx = find(uniqVertices.begin(), uniqVertices.end(), ve) - uniqVertices.begin();
          *vals++ = (GLuint) vidx;
          ve = VertexEntry(tri, 1);
          vidx = find(uniqVertices.begin(), uniqVertices.end(), ve) - uniqVertices.begin();
          *vals++ = (GLuint) vidx;
          ve = VertexEntry(tri, 2);
          vidx = find(uniqVertices.begin(), uniqVertices.end(), ve) - uniqVertices.begin();
          *vals++ = (GLuint) vidx;
        }
        
      }
      
      SubMesh *sm = new SubMesh(m);
      sm->createShared(SubMesh::PT_TRI, itype, ntri*3);
      sm->getIndexBuffer()->upload(dataSize, data, usage);
      
      if (midx != -1) {
        Material *mat = new Material();
        mat->setDiffuse(Vector4(mesh.materials[midx].Kd));
        mat->setAmbient(Vector4(mesh.materials[midx].Ka));
        mat->setSpecular(Vector4(mesh.materials[midx].Ks));
        mat->setShininess(mesh.materials[midx].Ns);
        sm->setMaterial(mat);
      }
      // Tr -> alpha
      // illum -> 1=nospec, 2=spec
      // map_Ka -> file name
      
      free(data);
      
      ++it;
    }
    
    return m;
  }
  return NULL;
}


// ---

class ObjFactory : public ggfx::MeshFactory {
  public:
  
    static std::string NAME;
    static std::string EXT;
    
    ObjFactory() {
    }
    
    virtual ~ObjFactory() {
    }
    
    virtual const std::string& getName() const {
      return NAME;
    }
    
    virtual const std::string& getExtension() const {
      return EXT;
    }
    
    virtual bool canLoad(const std::string &fileName) const {
      return gcore::Path(fileName).checkExtension(EXT);
    }
    
    virtual Mesh* create(const std::string &fileName, BufferObject::Usage usage, const std::string &name, float scl=1.0f) {
      return ObjLoad(fileName, usage, name, scl);
    }
    
    virtual void destroy(Mesh *mesh) {
      if (mesh && mesh->getFactory() == this) {
        delete mesh;
      }
    }
};

std::string ObjFactory::NAME = "ObjMeshFactory";
std::string ObjFactory::EXT = "obj";

static ObjFactory *gFactory = NULL;


GCORE_MODULE_API void initialize() {
  ggfx::MeshManager &mm = ggfx::MeshManager::Instance();
  if (!gFactory) {
    gFactory = new ObjFactory();
    mm.registerFactory(gFactory);
  }
  cout << "OBJ Mesh Plugin loaded" << endl;
}

GCORE_MODULE_API void deInitialize() {
  ggfx::MeshManager &mm = ggfx::MeshManager::Instance();
  mm.unregisterFactory(gFactory);
  delete gFactory;
  gFactory = NULL;
  cout << "OBJ Mesh Plugin unloaded" << endl;
}
