#ifndef __objmesh_h_
#define __objmesh_h_

#include <ggfx/ggfx>

// starts with newmtl
struct ObjMaterial {
	std::string name;
	float Kd[4];
	float Ka[4];
	float Ks[4];
	float Ns; // Shininess
	float Tr; // Alpha [can be "d" instead of Tr]
	int illum; // 1=nospec, 2=withspec [kS required]
	std::string map_Ka; //Texture map file, ASCII dump of RGB 
};

struct ObjTri {
  gmath::Vector3 face_norm;
	unsigned long v[3];
	unsigned long n[3];
	unsigned long t[3];
	bool has_norm;
	bool has_texc;
	long midx;
};

struct ObjEdge {
  unsigned long from;
  unsigned long to;
  size_t fdirect;
};

typedef std::vector<size_t> ObjSmoothGroup;

typedef std::vector<ObjEdge> ObjEdgeList;
typedef std::map<unsigned long, ObjEdgeList> ObjEdgeMap;

class ObjMesh {
  public:
	  
	  ObjMesh(const std::string &fname = "");
	  ~ObjMesh();
	  
	  bool load(const std::string &file_name);
	  bool load_material(const std::string &file_name);
    
    void scale(float amount);
    
    void compute_face_normal(ObjTri &tri);
    void add_face_edges(size_t triIdx);
    
    long find_material(const std::string &name) const;
    void smooth();
    void reset();
    
    std::string name;
    std::vector<gmath::Vector3> vertices;
    std::vector<gmath::Vector3> normals;
	  std::vector<gmath::Vector3> texcoords;
    std::vector<ObjTri> tris;
    ObjEdgeMap edgemap;
	  std::vector<ObjMaterial> materials;
	  std::map<int, ObjSmoothGroup> smoothgrps;
};

#endif

