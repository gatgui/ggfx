#include "objmesh.h"
using namespace std;
using namespace gmath;
using namespace ggfx;

// ---

struct normal_info {
	unsigned long index;
	std::vector<Vector3> normals;
};

enum IndicesFlags {
  NORMAL_INDEX = 0x01,
  TEXCOORD_INDEX = 0x02
};

static const char * face_patterns[4] = {
  "%d",
  "%d//%d", // n
  "%d/%d", // t
  "%d/%d/%d" // n + t
};

static int get_face_pattern(const char *token) {
  int tmp0, tmp1, tmp2;
  if (sscanf(token, face_patterns[3], &tmp0, &tmp1, &tmp2) == 3) {
    return 3;
  } else if (sscanf(token, face_patterns[1], &tmp0, &tmp1) == 2) {
    return 1;
  } else if (sscanf(token, face_patterns[2], &tmp0, &tmp1) == 2) {
    return 2;
  } else if (sscanf(token, face_patterns[0], &tmp0) == 1) {
    return 0;
  } else {
    return -1;
  }
}

static bool read_face_indices(const char *token, int pattern, int &v, int &n, int &t) {
  if (pattern == 0) {
    n = t = 0;
    return sscanf(token, face_patterns[pattern], &v) == 1;
  } else if (pattern == 1) {
    t = 0;
    return sscanf(token, face_patterns[pattern], &v, &n) == 2;
  } else if (pattern == 2) {
    n = 0;
    return sscanf(token, face_patterns[pattern], &v, &t) == 2;
  } else if (pattern == 3) {
    return sscanf(token, face_patterns[pattern], &v, &t, &n) == 3;
  } else {
    return false;
  }
}

// ---

ObjMesh::ObjMesh(const std::string &fname) {
  if (fname.length() > 0) {
    load(fname);
  }
}

ObjMesh::~ObjMesh() {
  reset();
}

void ObjMesh::reset() {
  name = "";
  vertices.clear();
  normals.clear();
  texcoords.clear();
  tris.clear();
  edgemap.clear();
  materials.clear();
  smoothgrps.clear();
}

bool ObjMesh::load_material(const string &file_name) {
	bool failed = false;
	bool empty = true;
	FILE *mfile = fopen(file_name.c_str(), "r");
	if (mfile) {
		char line[256];
		ObjMaterial m;
		while (!failed && fgets(line, 256, mfile)) {
			// skip white lines
			if (strlen(line) == 0) {
				continue;
			}
			// skip comment line
			else if (line[0] == '#') {
				continue;
			}
			// begin a new material
			else if (!strncmp(line, "newmtl", 6)) {
				if (!empty) {
					materials.push_back(m);
					empty = true;
				}
				char name[256];
				if (sscanf(line, "newmtl%*[ \t]%s", name) == 1) {
					m.name = name;
					m.Ka[0] = m.Ka[1] = m.Ka[2] = 0.2f;
					m.Kd[0] = m.Kd[1] = m.Kd[2] = 0.0f;
					m.Ks[0] = m.Ks[1] = m.Ks[2] = 1.0f;
					m.Ka[3] = m.Kd[3] = m.Ks[3] = 1.0f;
					m.Tr = 1.0f;
					m.Ns = 50.0f;
					m.illum = 2;
				} else {
					fprintf(stderr, "could not read material name\n");
					failed = true;
				}
			}
			// read diffuse
			else if (!strncmp(line, "Ka", 2)) {
				if (sscanf(line, "Ka%*[ \t]%f%*[ \t]%f%*[ \t]%f", &(m.Ka[0]), &(m.Ka[1]), &(m.Ka[2])) != 3) {
					fprintf(stderr, "could not read material ambient\n");
					failed = true;
				}
				empty = false;
			}
			// read ambiant
			else if (!strncmp(line, "Kd", 2)) {
				if (sscanf(line, "Kd%*[ \t]%f%*[ \t]%f%*[ \t]%f", &(m.Kd[0]), &(m.Kd[1]), &(m.Kd[2])) != 3) {
					fprintf(stderr, "could not read material diffuse\n");
					failed = true;
				}
				empty = false;
			}
			// read specular
			else if (!strncmp(line, "Ks", 2)) {
				if (sscanf(line, "Ks%*[ \t]%f%*[ \t]%f%*[ \t]%f", &(m.Ks[0]), &(m.Ks[1]), &(m.Ks[2])) != 3) {
					fprintf(stderr, "could not read material specular\n");
					failed = true;
				}
				empty = false;
			}
			// read alpha
			else if (line[0] == 'd') {
				if (sscanf(line, "d%*[ \t]%f", &m.Tr) == 1) {
					m.Ka[3] = m.Tr;
					m.Kd[3] = m.Tr;
					m.Ks[3] = m.Tr;
				} else {
					fprintf(stderr, "could not read material alpha\n");
					failed = true;
				}
				empty = false;
			}
			else if (!strncmp(line, "Tr", 2)) {
				if (sscanf(line, "Tr%*[ \t]%f", &m.Tr) == 1) {
					m.Ka[3] = m.Tr;
					m.Kd[3] = m.Tr;
					m.Ks[3] = m.Tr;
				} else {
					fprintf(stderr, "could not read material alpha\n");
					failed = true;
				}
				empty = false;
			}
			// Read shininess
			else if (!strncmp(line, "Ns", 2)) {
				if (sscanf(line, "Ns%*[ \t]%f", &m.Ns) != 1) {
					fprintf(stderr, "could not read material shininess\n");
					failed = true;
				}
				empty = false;
			}
			// read illum
			else if (!strncmp(line, "illum", 2)) {
				if (sscanf(line, "illum%*[ \t]%d", &m.illum) != 1) {
					fprintf(stderr, "could not read material illum\n");
					failed = true;
				}
				empty = false;
			}
			// read map_Ka (ignored)
			// other maps
			// Ni --> refaction index
		}
		if (!failed && !empty) {
			materials.push_back(m);
		}
		fclose(mfile);
	} else {
		failed = false;
		fprintf(stderr, "could not find material file\n");
	}
	return !failed;
}

long ObjMesh::find_material(const string &name) const {
	for (size_t i=0; i<materials.size(); ++i) {
		if (materials[i].name == name) {
			return (long)i;
		}
	}
	return -1;
}

void ObjMesh::smooth() {
  
  // when smoothing if vertex has normal, do not override
  
  // use edge list
  
  // if no vertex normal --> average face normal of neighbouring face in group
  
  map<int, ObjSmoothGroup>::iterator it = smoothgrps.begin();
  
  while (it != smoothgrps.end()) {
    
    ObjSmoothGroup &facelist = it->second;
    
    for (size_t i=0; i<facelist.size(); ++i) {
      
      ObjTri &tri = tris[facelist[i]];
      
      if (tri.has_norm == false) {
                
        tri.has_norm = true;
        unsigned long faceNrmIdx = 0;
        bool faceNrmAdded = false;
      
        for (int j=0; j<3; ++j) {
          
          Vector3 n = Vector3::ZERO;
        
          ObjEdgeList &edges = edgemap[tri.v[j]];
        
          for (size_t k=0; k<edges.size(); ++k) {
            if (find(facelist.begin(), facelist.end(), edges[k].fdirect) != facelist.end()) {
              n += tris[edges[k].fdirect].face_norm;
            }
          }
        
          if (n != Vector3::ZERO) {
            n.normalize();
            normals.push_back(n);
            tri.n[j] = (unsigned long)(normals.size() - 1);
          } else {
            if (!faceNrmAdded) {
              normals.push_back(tri.face_norm);
              faceNrmIdx = (unsigned long)(normals.size() - 1);
              faceNrmAdded = true;
            }
            tri.n[j] = faceNrmIdx;
          }
        }
      }
    }
    it++;
  }
}

void ObjMesh::compute_face_normal(ObjTri &tri) {
  Vector3 e0 = (vertices[tri.v[1]] - vertices[tri.v[0]]).normalize();
  Vector3 e1 = (vertices[tri.v[2]] - vertices[tri.v[1]]).normalize();
  Vector3 e2 = (vertices[tri.v[0]] - vertices[tri.v[2]]).normalize();
  float dp0 = Abs(e0.dot(e1));
  float dp1 = Abs(e1.dot(e2));
  float dp2 = Abs(e2.dot(e0));
  if (dp0 < dp1) {
    if (dp0 < dp2) {
      tri.face_norm = e0.cross(e1).normalize(); 
    } else {
      tri.face_norm = e2.cross(e0).normalize(); 
    }
  } else {
    if (dp1 < dp2) {
      tri.face_norm = e1.cross(e2).normalize(); 
    } else {
      tri.face_norm = e2.cross(e0).normalize(); 
    }
  }
}

void ObjMesh::add_face_edges(size_t triIdx) {
  ObjTri &tri = tris[triIdx];
  ObjEdge e0 = {tri.v[0], tri.v[1], triIdx};
  ObjEdge e1 = {tri.v[1], tri.v[2], triIdx};
  ObjEdge e2 = {tri.v[2], tri.v[0], triIdx};
  edgemap[tri.v[0]].push_back(e0);
  edgemap[tri.v[1]].push_back(e1);
  edgemap[tri.v[2]].push_back(e2);
}

bool ObjMesh::load(const string &file_name) {
	char line[256];
	bool failed = false;
  long curmat = -1; // should be white
	int sgidx = -1; // --> no smoothing [use face normal]
	int lineno = 0;
	FILE *file = fopen(file_name.c_str(), "r");
	if (file) {
		while (!failed && fgets(line, 256, file)) {
		  ++lineno;
			// skip white line
			if (strlen(line) == 0) {
				continue;
			}
			// skip comment
			if (line[0] == '#') {
				continue;
			}
			// read material if any
			else if (!strncmp(line, "mtllib", 6)) {
				char fname[256];
				if (sscanf(line, "mtllib%*[ \t]%s", fname) == 1) {
					size_t p = file_name.find_last_of("/\\");
					string mfile = fname;
					if (p != string::npos) {
						mfile = file_name.substr(0, p+1) + mfile;
					}
					if (!load_material(mfile)) {
						fprintf(stderr, "failed to read material library: \"%s\"\n", mfile.c_str());
					}
				}
			}
			// use material if any
			else if (!strncmp(line, "usemtl", 6)) {
				char mname[256];
				if (sscanf(line, "usemtl%*[ \t]%s", mname) == 1) {
          curmat = find_material(mname);
				}
			}
			// read a vertex or a vertex normal
			else if (line[0] == 'v') {
				if (line[1] == 'n') {
					// a vertex normal [always 3]
					Vector3 n;
					if (sscanf(line, "vn%*[ \t]%f%*[ \t]%f%*[ \t]%f", &n.x, &n.y, &n.z) == 3) {
						normals.push_back(n);
					} else {
					  fprintf(stderr, "Could not read normal (%d)\n", lineno);
						failed = true;
					}
				} else if (line[1] == 't') {
					// a texture coord [can be up to 3D !!!]
					Vector3 t;
					if (sscanf(line, "vt%*[ \t]%f%*[ \t]%f%*[ \t]%f", &t.x, &t.y, &t.z) == 3) {
						texcoords.push_back(t);
					} else if (sscanf(line, "vt%*[ \t]%f%*[ \t]%f", &t.x, &t.y) == 2) {
					  t.z = 0.0f;
				    texcoords.push_back(t);
				  } else if (sscanf(line, "vt%*[ \t]%f", &t.x) == 1) {
            t.y = t.z = 0.0f;
            texcoords.push_back(t);
			    } else {
				    fprintf(stderr, "Could not read texcoord (%d)\n", lineno);
					  failed = true;
				  }
				} else if (line[1] == ' ' || line[1] == '\t') {
					// a vertex [can be up to 4D or down to 1D]
					Vector3 v;
					if (sscanf(line, "v%*[ \t]%f%*[ \t]%f%*[ \t]%f", &v.x, &v.y, &v.z) == 3) {
						vertices.push_back(v);
					} else {
					  fprintf(stderr, "Could not read vertex (%d)\n", lineno);
						failed = true;
					}
				}
			}
			// read smooth group
			// [if no smooth -> use face normal !]
			else if (line[0] == 's') {
				char name[256] = {0};
				if (sscanf(line, "s%*[ \t]%s", name) == 1) {
					if (!strncmp(name, "off", 3)) {
						sgidx = -1;
					} else if (sscanf(name, "%d", &sgidx) == 1) {
						if (sgidx == 0) {
							sgidx = -1;
						}
					} else {
						fprintf(stderr, "Failed to read smooth group (%d)\n", lineno);
						failed = true;
					}
				} else {
					sgidx = -1;
				}
				if (sgidx != -1) {
					if (smoothgrps.find(sgidx) == smoothgrps.end()) {
						smoothgrps[sgidx] = ObjSmoothGroup();
					}
				}
			}
			// read a face
			else if (line[0] == 'f') {
				// will triangulate as triangle fans
				// first separate tokens !
				ObjTri tri;
				tri.midx = curmat;
				char *cc = line + 1;
				int ci = 0;
				int ntoken = 0;
				char tokens[64][64]; // not more than 64 tokens
				int t=0, v=0, n=0;
				while (*cc != '\0') {
					// skip white space
					while (*cc == ' ' || *cc == '\t' || *cc == '\n' || *cc == '\r') {
						++cc;
					}
					ci = 0;
					// keep characters
					while (*cc!=' ' && *cc!='\t' && *cc!='\n' && *cc!='\r' && *cc!='\0') {
						tokens[ntoken][ci++] = *cc;
						++cc;
					}
					tokens[ntoken][ci] = '\0';
					// not more than 64 tokens
					if (ci>0 && ++ntoken==64) {
						// to many tokens [up to 64]
						fprintf(stderr, "Polygon contains more than 64 vertices (%d)\n", lineno);
						failed = true;
						break;
					}
				}
				if (ntoken < 3) {
			    fprintf(stderr, "Polygon should have at least 3 vertices (%d)\n", lineno);
					failed = true;
			  }
				if (!failed) {
				  // if more than 3 --> fanning
          int pattern = get_face_pattern(tokens[0]);
          if (pattern < 0) {
            fprintf(stderr, "Unrecognize face pattern \"%s\" (%d)\n", line, lineno);
						failed = true;
          }
          tri.has_norm = ((pattern & NORMAL_INDEX) != 0);
          tri.has_texc = ((pattern & TEXCOORD_INDEX) != 0);
          if (read_face_indices(tokens[0], pattern, v, n, t)) {
            tri.v[0] = v - 1;
            tri.n[0] = n - 1;
            tri.t[0] = t - 1;
            if (read_face_indices(tokens[1], pattern, v, n, t)) {
              tri.v[1] = v - 1;
              tri.n[1] = n - 1;
              tri.t[1] = t - 1;
              if (read_face_indices(tokens[2], pattern, v, n, t)) {
                tri.v[2] = v - 1;
                tri.n[2] = n - 1;
                tri.t[2] = t - 1;
                compute_face_normal(tri);
                tris.push_back(tri);
                if (sgidx != -1) {
                  smoothgrps[sgidx].push_back(tris.size()-1);
                }
                add_face_edges(tris.size()-1);
                for (int i=3; i<ntoken; ++i) {
                  if (read_face_indices(tokens[i], pattern, v, n, t)) {
                    tri.v[1] = tri.v[2];
                    tri.n[1] = tri.n[2];
                    tri.t[1] = tri.t[2];
                    tri.v[2] = v - 1;
                    tri.n[2] = n - 1;
                    tri.t[2] = t - 1;
                    compute_face_normal(tri);
                    tris.push_back(tri);
                    if (sgidx != -1) {
                      smoothgrps[sgidx].push_back(tris.size()-1);
                    }
                    add_face_edges(tris.size()-1);
                  } else {
                    fprintf(stderr, "Unrecognize face token \"%s\" (%d)\n", tokens[i], lineno);
										failed = true;
										break;
                  }
                }
              } else {
                fprintf(stderr, "Unrecognize face token \"%s\" (%d)\n", tokens[2], lineno);
    						failed = true;
              }
            } else {
              fprintf(stderr, "Unrecognize face token \"%s\" (%d)\n", tokens[1], lineno);
  						failed = true;
            }
          } else {
            fprintf(stderr, "Unrecognize face token \"%s\" (%d)\n", tokens[0], lineno);
						failed = true;
          }
				}
			}
		}
		if (failed) {
			fprintf(stderr, "failed to read .obj file\n");
			reset();
			return false;
		} else {
			// smooth groups
      smooth();
      // set remaning vertex normals to face index
      for (size_t i=0; i<tris.size(); ++i) {
        if (tris[i].has_norm == false) {
          tris[i].has_norm = true;
          normals.push_back(tris[i].face_norm);
          tris[i].n[0] = (unsigned long)(normals.size() - 1);
          tris[i].n[1] = tris[i].n[0];
          tris[i].n[2] = tris[i].n[0];
        }
      }
		}
		fclose(file);
	}
	return true;
}

void ObjMesh::scale(float amount) {
  for (size_t i=0; i<vertices.size(); ++i) {
    float len = vertices[i].getLength();
    vertices[i].normalize() *= (len * amount);
  }
}

