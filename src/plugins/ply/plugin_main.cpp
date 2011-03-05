#include <ggfx/ggfx>
#include "rply.h"

using namespace std;
using namespace gmath;
using namespace ggfx;

// ---

struct Vertex {
  Vector3 pos;
  Vector3 nrm;
};
typedef vector<Vertex> VertexList;

struct Tri {
  size_t v0, v1, v2;
  Vector3 nrm;
};
typedef vector<Tri> TriList;

struct Quad {
  size_t v0, v1, v2, v3;
  Vector3 nrm;
};
typedef vector<Quad> QuadList;

struct Edge {
  size_t from, to;
  size_t fdirect;
  size_t findirect;
  Edge(size_t v0, size_t v1, size_t fd)
    :from(v0), to(v1), fdirect(fd), findirect((size_t)~0) {
  }
};
typedef vector<Edge> EdgeList;
typedef map<size_t, EdgeList> EdgeMap;

struct PlyMesh {
  TriList tris;
  EdgeMap tedgeMap;
  QuadList quads;
  EdgeMap qedgeMap;
  VertexList vertices;
  AABox box;
  bool reverse;
};

// ---

static int read_vertex(p_ply_argument arg) {
  long comp;
  long idx;
  void *data;
  ply_get_argument_user_data(arg, &data, &comp);
  PlyMesh &pm = *((PlyMesh*)data);
  ply_get_argument_element(arg, NULL, &idx);
  pm.vertices[idx].pos[comp] = float(ply_get_argument_value(arg));
  if (comp == 2) {
    if (idx == 0) {
      pm.box.setMin(pm.vertices[idx].pos);
      pm.box.setMax(pm.vertices[idx].pos);
    } else {
      pm.box.merge(pm.vertices[idx].pos);
    }
  }
  return 1;
}

static int read_face(p_ply_argument arg) {
  long idx;
  long len;
  long comp;
  void *data;
  size_t vidx;
  ply_get_argument_user_data(arg, &data, NULL);
  PlyMesh &pm = *((PlyMesh*)data);
  ply_get_argument_element(arg, NULL, &idx); // not needed
  ply_get_argument_property(arg, NULL, &len, &comp);
  if (comp == -1) {
    // ply_get_argument_value(arg) == len
    if (len == 3) {
      pm.tris.push_back(Tri());
    } else if (len == 4) {
      pm.quads.push_back(Quad());
    }
  } else {
    vidx = size_t(ply_get_argument_value(arg));
    if (len == 3) {
      Tri &t = pm.tris.back();
      if (comp == 0) {
        t.v0 = vidx;
      } else if (comp == 1) {
        t.v1 = vidx;
      } else {
        t.v2 = vidx;
        if (pm.reverse) {
          // v0, v1, v2 --> v0, v2, v1
          size_t tmp = t.v1;
          t.v1 = t.v2;
          t.v2 = tmp;
        }
        // compute normal
        Vector3 e0 = (pm.vertices[t.v1].pos - pm.vertices[t.v0].pos).normalize();
        Vector3 e1 = (pm.vertices[t.v2].pos - pm.vertices[t.v0].pos).normalize();
        t.nrm = e0.cross(e1).normalize();
        // add edges
        size_t fidx = pm.tris.size() - 1;
        pm.tedgeMap[t.v0].push_back(Edge(t.v0, t.v1, fidx));
        pm.tedgeMap[t.v1].push_back(Edge(t.v1, t.v2, fidx));
        pm.tedgeMap[t.v2].push_back(Edge(t.v2, t.v0, fidx));
      }
    } else if (len == 4) {
      Quad &q = pm.quads.back();
      if (comp == 0) {
        q.v0 = vidx;
      } else if (comp == 1) {
        q.v1 = vidx;
      } else if (comp == 2) {
        q.v2 = vidx;
      } else {
        q.v3 = vidx;
        if (pm.reverse) {
          // v0, v1, v2, v3 --> v0, v3, v2, v1
          size_t tmp = q.v1;
          q.v1 = q.v3;
          q.v3 = tmp;
        }
        // compute normal
        Vector3 e0 = (pm.vertices[q.v1].pos - pm.vertices[q.v0].pos).normalize();
        Vector3 e1 = (pm.vertices[q.v3].pos - pm.vertices[q.v0].pos).normalize();
        q.nrm = e0.cross(e1).normalize();
        // add edges
        size_t fidx = pm.quads.size() - 1;
        pm.qedgeMap[q.v0].push_back(Edge(q.v0, q.v1, fidx));
        pm.qedgeMap[q.v1].push_back(Edge(q.v1, q.v2, fidx));
        pm.qedgeMap[q.v2].push_back(Edge(q.v2, q.v3, fidx));
        pm.qedgeMap[q.v3].push_back(Edge(q.v3, q.v0, fidx));
      }
    }
  }
  return 1;
}

template <typename IdxType>
void FillTriIndicesData(const PlyMesh &mesh, BufferObject *buffer, BufferObject::Usage usage) {
  size_t dataSize = mesh.tris.size() * 3 * sizeof(IdxType);
  IdxType *data = (IdxType*)malloc(dataSize);
  IdxType *ci = data;
  for (size_t i=0; i<mesh.tris.size(); ++i, ci+=3) {
    ci[0] = IdxType(mesh.tris[i].v0);
    ci[1] = IdxType(mesh.tris[i].v1);
    ci[2] = IdxType(mesh.tris[i].v2);
  }
  buffer->upload(dataSize, data, usage);
  free(data);
}

template <typename IdxType>
void FillQuadIndicesData(const PlyMesh &mesh, BufferObject *buffer, BufferObject::Usage usage) {
  size_t dataSize = mesh.quads.size() * 4 * sizeof(IdxType);
  IdxType *data = (IdxType*)malloc(dataSize);
  IdxType *ci = data;
  for (size_t i=0; i<mesh.quads.size(); ++i, ci+=4) {
    ci[0] = IdxType(mesh.quads[i].v0);
    ci[1] = IdxType(mesh.quads[i].v1);
    ci[2] = IdxType(mesh.quads[i].v2);
    ci[3] = IdxType(mesh.quads[i].v3);
  }
  buffer->upload(dataSize, data, usage);
  free(data);
}

typedef void (*IndexFillFunc)(const PlyMesh&, BufferObject*, BufferObject::Usage);

Mesh* PlyLoad(const std::string &fileName, ggfx::BufferObject::Usage usage, const std::string &name, float scl) {
  long nv, nf;
  p_ply ply = ply_open(fileName.c_str(), NULL);
  if (!ply) {
    return NULL;
  }
  if (!ply_read_header(ply)) {
    return NULL;
  }
  
  PlyMesh plymesh;
  
  nv = ply_set_read_cb(ply, "vertex", "x", read_vertex, &plymesh, 0);
  ply_set_read_cb(ply, "vertex", "y", read_vertex, &plymesh, 1);
  ply_set_read_cb(ply, "vertex", "z", read_vertex, &plymesh, 2);
  nf = ply_set_read_cb(ply, "face", "vertex_indices", read_face, &plymesh, 0);
  plymesh.vertices.resize(nv);
  plymesh.reverse = false;
  
  const char* objInfo = ply_get_next_obj_info(ply, NULL);
  
  Matrix4 rot = Matrix4::IDENTITY;
  Vector3 trans = Vector3::ZERO;
  float scale = 1.0f;
  
  while (objInfo) {
    std::string info(objInfo);
    if (info == "CW") {
      plymesh.reverse = true;
    } else if (info.find("rot") != std::string::npos) {
      size_t p0, p1;
      p0 = info.find(' ');
      if (p0 != std::string::npos) {
        p1 = info.find(' ', p0+1);
        if (p1 != std::string::npos) {
          std::string axis = info.substr(p0+1, p1-p0-1);
          Vector3 a;
          bool processRotation = true;
          if (axis == "x") {
            a = Vector3::UNIT_X;
          } else if (axis == "y") {
            a = Vector3::UNIT_Y;
          } else if (axis == "z") {
            a = Vector3::UNIT_Z;
          } else {
            processRotation = false;
          }
          if (processRotation) {
            float val;
            p0 = p1+1;
            p1 = info.find(' ', p0);
            if (p1 != std::string::npos) {
              std::string value = info.substr(p0, p1-p0);
              if (sscanf(value.c_str(), "%g", &val) != 1) {
                processRotation = false;
              }
            } else {
              std::string value = info.substr(p0);
              if (sscanf(value.c_str(), "%g", &val) != 1) {
                processRotation = false;
              }
            }
            if (processRotation) {
              rot *= Matrix4::MakeRotate(val, a);
            }
          }
        }
      }
    } else if (info.find("scl") != std::string::npos) {
      size_t p0 = info.find(' ');
      if (p0 != std::string::npos) {
        size_t p1 = info.find(' ', p0+1);
        if (p1 != std::string::npos) {
          std::string value = info.substr(p0+1, p1-p0-1);
          if (sscanf(value.c_str(), "%g", &scale) != 1) {
            scale = 1.0f;
          }
        } else {
          std::string value = info.substr(p0+1);
          if (sscanf(value.c_str(), "%g", &scale) != 1) {
            scale = 1.0f;
          }
        }
      }
    } else if (info.find("trans") != std::string::npos) {
      size_t p0 = info.find(' ');
      if (p0 != std::string::npos) {
        std::string value = info.substr(p0+1);
        if (sscanf(value.c_str(), "%g %g %g", &(trans.x), &(trans.y), &(trans.z)) != 3) {
          trans = Vector3::ZERO;
        }
      }
    }
    objInfo = ply_get_next_obj_info(ply, objInfo);
  }
  if (!ply_read(ply)) {
    return NULL;
  }
  ply_close(ply);
  
  
  size_t i;
  Vector3 center = 0.5f * (plymesh.box.getMin() + plymesh.box.getMax());
  
  // transform vertices pos and face normals around box center ?
  for (i=0; i<plymesh.vertices.size(); ++i) {
    Vector4 v = Vector4(plymesh.vertices[i].pos - center, 0.0f);
    v = rot * v;
    plymesh.vertices[i].pos = center + Vector3(v);
  }
  for (i=0; i<plymesh.tris.size(); ++i) {
    Vector4 v = Vector4(plymesh.tris[i].nrm, 0.0f);
    v = rot * v;
    plymesh.tris[i].nrm = Vector3(v);
  }
  for (i=0; i<plymesh.quads.size(); ++i) {
    Vector4 v = Vector4(plymesh.quads[i].nrm, 0.0f);
    v = rot * v;
    plymesh.quads[i].nrm = Vector3(v);
  }
  
  // scale then translate vertices
  plymesh.box.reset();
  scale *= scl;
  Vector3 tmp = plymesh.vertices[0].pos - center;
  tmp = (center + (scale * tmp)) + trans;
  plymesh.box.setMin(tmp);
  plymesh.box.setMax(tmp);
  for (i=1; i<plymesh.vertices.size(); ++i) {
    Vector3 v = plymesh.vertices[i].pos - center;
    plymesh.vertices[i].pos = (center + (scale * v)) + trans;
    plymesh.box.merge(plymesh.vertices[i].pos);
  }
  
  for (i=0; i<plymesh.vertices.size(); ++i) {
    Vertex &vtx = plymesh.vertices[i];
    vtx.nrm = Vector3::ZERO;
    if (plymesh.tedgeMap.find(i) != plymesh.tedgeMap.end()) {
      EdgeList &tedges = plymesh.tedgeMap[i];
      for (size_t j=0; j<tedges.size(); ++j) {
        vtx.nrm += plymesh.tris[tedges[j].fdirect].nrm;
      }
      vtx.nrm.normalize();
    }
    if (plymesh.qedgeMap.find(i) != plymesh.qedgeMap.end()) {
      EdgeList &qedges = plymesh.qedgeMap[i];
      for (size_t j=0; j<qedges.size(); ++j) {
        vtx.nrm += plymesh.quads[qedges[j].fdirect].nrm;
      }
      vtx.nrm.normalize();
    }
  }
  
  VertexFormat *fmt = VertexFormat::Get("position-float-3|normal-float-3");
  
  if (fmt) {
    
    Mesh *mesh = new Mesh(name);
    
    size_t dataSize = nv*6*sizeof(GLfloat);
    float *data = (float*)malloc(dataSize);
    float *cv = data;
    for (long v=0; v<nv; ++v, cv+=6) {
      Vertex &vtx = plymesh.vertices[v];
      cv[0] = vtx.pos.x;
      cv[1] = vtx.pos.y;
      cv[2] = vtx.pos.z;
      cv[3] = vtx.nrm.x;
      cv[4] = vtx.nrm.y;
      cv[5] = vtx.nrm.z;
    }
    
    SubMesh::IndexType itype;
    IndexFillFunc fillTriIndices;
    IndexFillFunc fillQuadIndices;
    
    if (nv < 256) {
      itype = SubMesh::IT_8;
      fillTriIndices = &FillTriIndicesData<GLubyte>;
      fillQuadIndices = &FillQuadIndicesData<GLubyte>;
    } else if (nv < 65536) {
      itype = SubMesh::IT_16;
      fillTriIndices = &FillTriIndicesData<GLushort>;
      fillQuadIndices = &FillQuadIndicesData<GLushort>;
    } else {
      itype = SubMesh::IT_32;
      fillTriIndices = &FillTriIndicesData<GLuint>;
      fillQuadIndices = &FillQuadIndicesData<GLuint>;
    }
    
    mesh->createSharedVertices(fmt, nv);
    mesh->getSharedVertexBuffer()->upload(dataSize, data, usage);
    
    if (plymesh.tris.size() > 0) {
      SubMesh *tsm = new SubMesh(mesh);
      tsm->createShared(SubMesh::PT_TRI, itype, plymesh.tris.size()*3);
      fillTriIndices(plymesh, tsm->getIndexBuffer(), usage);
    }
    
    if (plymesh.quads.size() > 0) {
      SubMesh *qsm = new SubMesh(mesh);
      qsm->createShared(SubMesh::PT_QUAD, itype, plymesh.quads.size()*4);
      fillQuadIndices(plymesh, qsm->getIndexBuffer(), usage);
    }
    
    free(data);
    
    return mesh;
  }
  
  return NULL;
}

// ---

// for win32 DllMain
//MODULE_DUMMY_MAIN

class PlyFactory : public ggfx::MeshFactory {
  public:
  
    static std::string NAME;
    static std::string EXT;
    
    PlyFactory() {
    }
    
    virtual ~PlyFactory() {
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
      return PlyLoad(fileName, usage, name, scl);
    }
    
    virtual void destroy(Mesh *mesh) {
      if (mesh && mesh->getFactory() == this) {
        delete mesh;
      }
    }
};

std::string PlyFactory::NAME = "PlyMeshFactory";
std::string PlyFactory::EXT = "ply";

static PlyFactory *gFactory = NULL;


MODULE_API void initialize() {
  ggfx::MeshManager &mm = ggfx::MeshManager::Instance();
  if (!gFactory) {
    gFactory = new PlyFactory();
    mm.registerFactory(gFactory);
  }
  cout << "PLY Mesh Plugin loaded" << endl;
}

MODULE_API void deInitialize() {
  ggfx::MeshManager &mm = ggfx::MeshManager::Instance();
  mm.unregisterFactory(gFactory);
  delete gFactory;
  gFactory = NULL;
  cout << "PLY Mesh Plugin unloaded" << endl;
}
