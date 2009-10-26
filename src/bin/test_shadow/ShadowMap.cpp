#include <ggfx/config.h>
#ifdef __APPLE__
# include <GLUT/glut.h>
#else
# ifdef WIN32
#   pragma warning(disable: 4505)
# endif
# include <GL/glut.h>
#endif
#include <ggfx/bufferobject.h>
#include <ggfx/vertexformat.h>
#include <ggfx/shader.h>
#include <ggfx/mesh.h>
#include <ggfx/renderer.h>
#include <ggfx/system.h>

#include "Shadow.h"
#include "Shadow2.h"

#if 1
# define NEW_SHADOW
#endif

using namespace gmath;
using namespace ggfx;
using namespace std;

// --- Structures

struct Vertex {
  float pos[3];
  float nrm[3];
  float texc[2];
};
struct Tri {
  GLubyte v0;
  GLubyte v1;
  GLubyte v2;
};
struct MaterialData {
  float diff[4];
  float ambi[4];
  float spec[4];
  float shininess;
};

// --- Data

bool useAdvShadows = false;
LightType ltype = LT_SPOT;
#ifndef NEW_SHADOW
  Shadow::Type stype = Shadow::ST_UNIFORM;
  Shadow shadow;
#else
  Shadow2 shadow;
  enum AdvType {
    FOCUSED = 0,
    LiSPSM,
    TSM
  };
  AdvType advShadowType = FOCUSED;
#endif

int win_w = 640;
int win_h = 480;
int mouse_btn = -1;
int mouse_state = -1;
int key_mod = -1;
int scr_width = 0;
int scr_hwidth = 0;
int scr_height = 0;
int last_x = -1;
int last_y = -1;
int cur_x = -1;
int cur_y = -1;
enum MouseOperation {
  MO_CAM_ROTATE = 0,
  MO_CAM_TRANSLATE,
  MO_CAM_DOLLY,
  MO_LIGHT_TRANSLATE,
  MO_NONE
};
MouseOperation mouse_op = MO_NONE;
enum DebugView {
  DV_HULLS,
#ifdef NEW_SHADOW
  DV_NT,
#endif
  DV_PPS
};
DebugView dbview = DV_HULLS;

Program *cprog = NULL;
Program *rprog = NULL;
Shader *cvert = NULL;
Shader *cfrag = NULL;
Shader *rvert = NULL;
Shader *rfrag = NULL;
Shader *rfrag_no = NULL;
Shader *rfrag_2x2 = NULL;
Shader *rfrag_4x4 = NULL;
Shader *rfrag_disc = NULL;
// for tsm
Shader *cvert_tsm = NULL;
Shader *cfrag_tsm = NULL;
Shader *rvert_tsm = NULL;
Shader *rfrag_tsm = NULL;

SubMesh *CubeMesh = NULL;
SubMesh *PlaneMesh = NULL;

GLuint fbo;
GLuint dbo;
GLuint shadowMap;
GLuint colorMap0;
GLuint colorMap1;
GLsizei shadowSize = 1024;

float cam_dtt2 = 10.0f;
Vector3 cam_aim2(0.0f, 2.5f, 0.0f);
Vector3 cam_ypr2(0, 0, 0);
Matrix4 cam_mv2;
Matrix4 cam_imv2;
Matrix4 cam_proj2;

float cam_dtt = 10;
Vector3 cam_aim(0, 2.5f, 0);
Vector3 cam_ypr(0, 0, 0);
Matrix4 cam_mv;
Matrix4 cam_imv;
Matrix4 cam_proj;

struct Cam {
  Vector3 *aim;
  Vector3 *ypr;
  float *dtt;
  Matrix4 *mv;
  Matrix4 *imv;
  Matrix4 *proj;
};

Cam cams[2] = {
  {&cam_aim, &cam_ypr, &cam_dtt, &cam_mv, &cam_imv, &cam_proj},
  {&cam_aim2, &cam_ypr2, &cam_dtt2, &cam_mv2, &cam_imv2, &cam_proj2}
};
int cur_cam = 0;

Vector3 light_pos_eye;
Vector4 light_pos(20, 20, 20, 1);
Vector3 light_aim(0, 0, 0);
Vector3 light_dir(0, 0, 0);
Matrix4 light_mv;
Matrix4 light_imv;
Matrix4 light_proj;

float fovy = 56.0f;
float aspect = 640.f / 480.f;
float nplane = 1.0f;
float fplane = 1000.0f;

Vector4 white(1,1,1,1);
Vector4 black(0,0,0,1);

float planeSize = 30.f;
Vertex PlaneVertexData[] = {
  {{-planeSize, 0,  planeSize}, {0, 1, 0}, {0, 0}},
  {{ planeSize, 0,  planeSize}, {0, 1, 0}, {1, 0}},
  {{ planeSize, 0, -planeSize}, {0, 1, 0}, {1, 1}},
  {{-planeSize, 0, -planeSize}, {0, 1, 0}, {0, 1}},
};
Tri PlaneFaceData[] = {
  {0, 1, 2},
  {0, 2, 3}
};
unsigned int PlaneVertexCount(sizeof(PlaneVertexData) / sizeof(Vertex));
unsigned int PlaneFaceCount(sizeof(PlaneFaceData) / sizeof(Tri));
MaterialData PlaneMaterial = {
  {0.8f, 0.2f, 0.5f, 1.0f},
  {0, 0, 0, 1.0f},
  {1, 1, 1, 1.0f},
  12
};
MaterialData NoneMaterial = {
  {1, 1, 1, 1},
  {0, 0, 0, 1},
  {1, 1, 1, 1},
  0
};

float cubeSize = 2.5f;
Vertex CubeVertexData[] = {
  // Front face
  {{-cubeSize, 0,           cubeSize}, { 0,  0,  1}, {0, 0}},
  {{ cubeSize, 0,           cubeSize}, { 0,  0,  1}, {1, 0}},
  {{ cubeSize, 2*cubeSize,  cubeSize}, { 0,  0,  1}, {1, 1}},
  {{-cubeSize, 2*cubeSize,  cubeSize}, { 0,  0,  1}, {0, 1}},
  // Back face
  {{ cubeSize, 0,          -cubeSize}, { 0,  0, -1}, {0, 0}},
  {{-cubeSize, 0,          -cubeSize}, { 0,  0, -1}, {1, 0}},
  {{-cubeSize, 2*cubeSize, -cubeSize}, { 0,  0, -1}, {1, 1}},
  {{ cubeSize, 2*cubeSize, -cubeSize}, { 0,  0, -1}, {0, 1}},
  // Left face
  {{ cubeSize, 0,           cubeSize}, { 1,  0,  0}, {0, 0}},
  {{ cubeSize, 0,          -cubeSize}, { 1,  0,  0}, {1, 0}},
  {{ cubeSize, 2*cubeSize, -cubeSize}, { 1,  0,  0}, {1, 1}},
  {{ cubeSize, 2*cubeSize,  cubeSize}, { 1,  0,  0}, {0, 1}},
  // Right face
  {{-cubeSize, 0,          -cubeSize}, {-1,  0,  0}, {0, 0}},
  {{-cubeSize, 0,           cubeSize}, {-1,  0,  0}, {1, 0}},
  {{-cubeSize, 2*cubeSize,  cubeSize}, {-1,  0,  0}, {1, 1}},
  {{-cubeSize, 2*cubeSize, -cubeSize}, {-1,  0,  0}, {0, 1}},
  // Top face
  {{-cubeSize, 2*cubeSize,  cubeSize}, { 0,  1,  0}, {0, 0}},
  {{ cubeSize, 2*cubeSize,  cubeSize}, { 0,  1,  0}, {1, 0}},
  {{ cubeSize, 2*cubeSize, -cubeSize}, { 0,  1,  0}, {1, 1}},
  {{-cubeSize, 2*cubeSize, -cubeSize}, { 0,  1,  0}, {0, 1}},
  // Bottom face
  {{ cubeSize,          0,  cubeSize}, { 0, -1,  0}, {0, 0}},
  {{-cubeSize,          0,  cubeSize}, { 0, -1,  0}, {1, 0}},
  {{-cubeSize,          0, -cubeSize}, { 0, -1,  0}, {1, 1}},
  {{ cubeSize,          0, -cubeSize}, { 0, -1,  0}, {0, 1}}
};
Tri CubeFaceData[] = {
  { 0,  1,  2},
  { 0,  2,  3},
  { 4,  5,  6},
  { 4,  6,  7},
  { 8,  9, 10},
  { 8, 10, 11},
  {12, 13, 14},
  {12, 14, 15},
  {16, 17, 18},
  {16, 18, 19},
  {20, 21, 22},
  {20, 22, 23}
};
unsigned int CubeVertexCount(sizeof(CubeVertexData) / sizeof(Vertex));
unsigned int CubeFaceCount(sizeof(CubeFaceData) / sizeof(Tri));
MaterialData CubeMaterial = {
  {0.4f, 0.2f, 0.8f, 1.0f},
  {0, 0, 0, 1.0f},
  {1, 1, 1, 1.0f},
  36
};

// --- Shader

void SHADERsetup() {

#ifdef _DEBUG
  std::cout << "Setup shaders" << std::endl;
#endif

  ProgramManager &mgr = ProgramManager::Instance();
  
  cvert = mgr.createShader("SM_Caster_vp", Shader::ST_VERTEX, "share/shaders/sc_vp.glsl");
  cfrag = mgr.createShader("SM_Caster_fp", Shader::ST_FRAGMENT, "share/shaders/sc_fp.glsl");
  cvert_tsm = mgr.createShader("TSM_Caster_vp", Shader::ST_VERTEX, "share/shaders/tsm_cast.vert");
  cfrag_tsm = mgr.createShader("TSM_Caster_fp", Shader::ST_FRAGMENT, "share/shaders/tsm_cast.frag");
  cprog = mgr.createProgram("Caster", cvert, cfrag);
  
  rvert = mgr.createShader("SM_Receiver_vp", Shader::ST_VERTEX, "share/shaders/sr_vp.glsl");
  rfrag = rfrag_no = mgr.createShader("SM_Receiver_basic_fp", Shader::ST_FRAGMENT, "share/shaders/sr_fp.glsl");
  rfrag_2x2 = mgr.createShader("SM_Receiver_2x2_fp", Shader::ST_FRAGMENT, "share/shaders/sr_pcf2x2_fp.glsl");
  rfrag_4x4 = mgr.createShader("SM_Receiver_4x4_fp", Shader::ST_FRAGMENT, "share/shaders/sr_pcf4x4_fp.glsl");
  rfrag_disc = mgr.createShader("SM_Receiver_disc_fp", Shader::ST_FRAGMENT, "share/shaders/sr_pcfDisc_fp.glsl");
  rvert_tsm = mgr.createShader("TSM_Receiver_vp", Shader::ST_VERTEX, "share/shaders/tsm_recv.vert");
  rfrag_tsm = mgr.createShader("TSM_Receiver_fp", Shader::ST_FRAGMENT, "share/shaders/tsm_recv.frag");
  rprog = mgr.createProgram("Receiver", rvert, rfrag);
}

void SHADERdelete() {
  delete ProgramManager::InstancePtr();
}

// --- Mesh

void MESHsetMaterial(MaterialData *material) {
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material->diff);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material->spec);
  glMaterialfv(GL_FRONT, GL_AMBIENT, material->ambi);
  glMaterialf(GL_FRONT, GL_SHININESS, material->shininess);
}

void MESHdelete() {
  delete CubeMesh;
  delete PlaneMesh;
}

void MESHsetup() {
#ifdef _DEBUG
  std::cout << "Setup meshes" << std::endl;
#endif
  
  VertexFormat *fmt;
  {
    VertexFormat tmp;
    tmp.addAttribute(VertexAttribute(VAS_POSITION, VAT_FLOAT, 3));
    tmp.addAttribute(VertexAttribute(VAS_NORMAL, VAT_FLOAT, 3));
    tmp.addAttribute(VertexAttribute(VAS_TEXCOORD, VAT_FLOAT, 2));
    cout << tmp.toString() << endl;
    cout << "Size in bytes: " << (unsigned long)(tmp.getBytesSize()) << endl;
    fmt = VertexFormat::Register(tmp);
  }
  //BufferObject::Usage usage =
    //BufferObject::Usage(BufferObject::BOU_STATIC|BufferObject::BOU_DRAW);
  BufferObject *vbo = NULL;
  CubeMesh = new SubMesh();
  //if (CubeMesh->createIndexed(usage, fmt, 24, 36)) { //nvert, nindex
  if (CubeMesh->createIndexed(SubMesh::PT_TRI, fmt, 24, SubMesh::IT_8, 36)) { //nvert, nindex
    vbo = CubeMesh->getVertexBuffer();
    vbo->bind();
    //vbo->upload(0, sizeof(CubeVertexData), CubeVertexData);
    vbo->upload(sizeof(CubeVertexData), CubeVertexData, BufferObject::BOU_STATIC);
    vbo = CubeMesh->getIndexBuffer();
    vbo->bind();
    //vbo->upload(0, sizeof(CubeFaceData), CubeFaceData);
    vbo->upload(sizeof(CubeFaceData), CubeFaceData, BufferObject::BOU_STATIC);
  }
  PlaneMesh = new SubMesh();
  //if (PlaneMesh->createIndexed(usage, fmt, 4, 6)) { // nvert, nindex
  if (PlaneMesh->createIndexed(SubMesh::PT_TRI, fmt, 4, SubMesh::IT_8, 6)) { // nvert, nindex
    vbo = PlaneMesh->getVertexBuffer();
    vbo->bind();
    //vbo->upload(0, sizeof(PlaneVertexData), PlaneVertexData);
    vbo->upload(sizeof(PlaneVertexData), PlaneVertexData, BufferObject::BOU_STATIC);
    vbo = PlaneMesh->getIndexBuffer();
    vbo->bind();
    //vbo->upload(0, sizeof(PlaneFaceData), PlaneFaceData);
    vbo->upload(sizeof(PlaneFaceData), PlaneFaceData, BufferObject::BOU_STATIC);
  }
  /*
  std::string fmtStr = "P-3-N-3-T0-2";
  VertexFormat *fmt = VertexFormat::Get(fmtStr);
  BufferObject::Usage usage =
    BufferObject::Usage(BufferObject::BOU_STATIC|BufferObject::BOU_DRAW);
  BufferObject *vbo = NULL;
  CubeMesh = new SubMesh();
  if (CubeMesh->createIndexed(usage, fmt, 24, 36)) { //nvert, nindex
    vbo = CubeMesh->vertexBuffer();
    vbo->bind();
    vbo->upload(0, sizeof(CubeVertexData), CubeVertexData);
    vbo = CubeMesh->indexBuffer();
    vbo->bind();
    vbo->upload(0, sizeof(CubeFaceData), CubeFaceData);
  }
  PlaneMesh = new SubMesh();
  if (PlaneMesh->createIndexed(usage, fmt, 4, 6)) { // nvert, nindex
    vbo = PlaneMesh->vertexBuffer();
    vbo->bind();
    vbo->upload(0, sizeof(PlaneVertexData), PlaneVertexData);
    vbo = PlaneMesh->indexBuffer();
    vbo->bind();
    vbo->upload(0, sizeof(PlaneFaceData), PlaneFaceData);
  }
  */
}

// --- FBO

void FBOsetup() {
#ifdef _DEBUG
  std::cout << "Setup FBO" << std::endl;
#endif
  glGenFramebuffersEXT(1, &fbo);
}

void FBObind() {
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
}

void FBOunbind() {
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void FBOattachColor2D(GLuint texId, int idx=0) {
  FBObind();
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT+idx,
    GL_TEXTURE_2D, texId, 0);
}

void FBOattachDepth(GLuint texId) {
  FBObind();
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
    GL_TEXTURE_2D, texId, 0);
}

void FBOdelete() {
  glDeleteFramebuffersEXT(1, &fbo);
}

// --- Texture

void TEXsetup(GLsizei sz) {

#ifdef _DEBUG
  std::cout << "Setup textures" << std::endl;
#endif

  FBOunbind();
  
  glGenTextures(1, &colorMap0);
  glBindTexture(GL_TEXTURE_2D, colorMap0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, sz, sz, 0, GL_RGBA, GL_FLOAT, NULL);
  
  //glGenTextures(1, &shadowMap);
  //glBindTexture(GL_TEXTURE_2D, shadowMap);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24_ARB, sz, sz, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
  
  glGenRenderbuffersEXT(1, &dbo);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, dbo);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, sz, sz);
  
  FBObind();
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, colorMap0, 0);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, dbo);
  //glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, shadowMap, 0);
  GLenum fbErr = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if (fbErr != GL_FRAMEBUFFER_COMPLETE_EXT) {
    switch (fbErr) {
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT: cout << "FBO: Incomplete attachment" << endl; break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT: cout << "FBO: Missing attachment" << endl; break;
      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: cout << "FBO: Invalid dimensions" << endl; break;
      case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: cout << "FBO: Invalid formats" << endl; break;
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT: cout << "FBO: Invalid draw buffer" << endl; break;
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT: cout << "FBO: Invalid read buffer" << endl; break;
      case GL_FRAMEBUFFER_UNSUPPORTED_EXT: cout << "FBO: Unsupported" << endl; break;
      default: cout << "Unknown error" << endl;
    }
  }
  FBOunbind();
}

void TEXdelete() {
  glDeleteTextures(1, &colorMap0);
  glDeleteRenderbuffersEXT(1, &dbo);
  //glDeleteTextures(1, &colorMap1);
  //glDeleteTextures(1, &shadowMap);
}

// --- Init/Cleanup

void init() {
#ifdef _DEBUG
  std::cout << "Initialize" << std::endl;
#endif
  //ggfx::InitExtensions();
  
#ifdef _DEBUG
  std::cout << "Create system" << std::endl;
#endif
  ggfx::System *s = new ggfx::System(); // load plugins
  s->initialize(".");
  
#ifdef _DEBUG
  std::cout << "Create renderer" << std::endl;
#endif
  ggfx::Renderer *r = new ggfx::Renderer(); // create renderer
  r->initialize();
  
#ifdef _DEBUG
  std::cout << "Basic GL setup" << std::endl;
#endif
  glClearColor(0, 0, 0, 1);
  glClearDepth(1);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT, GL_FILL);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
  glEnable(GL_TEXTURE_2D);
  FBOsetup();
  MESHsetup();
  TEXsetup(shadowSize);
  SHADERsetup();
  light_dir = (light_aim - Vector3(light_pos)).normalize();
}

void cleanup() {
  cout << "Cleanup" << endl;
  FBOdelete();
  TEXdelete();
  MESHdelete();
  SHADERdelete();
}

// --- GLUT callbacks

static void renderHull(
  const ConvexHull3D &b, const Vector3 &color,
  const Matrix4 &toWorld = Matrix4::IDENTITY) {
  
  int i, j;
  
  Vector4 c0 = Vector4(color, 0.25f);
  Vector3 c1 = color - Vector3(0.25f);
  
  glPushMatrix();
  glMultMatrixf(toWorld);
  
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (i=0; i<b.getNumPolygons(); ++i) {
    const ConvexHull3D::Polygon &p = b.getPolygon(i);
    glBegin(GL_POLYGON);
      glColor4fv(c0);
      for (j=0; j<p.getNumVertices(); ++j) {
        glVertex3fv(p.getVertex(j));
      }
    glEnd();
  }
  glDisable(GL_BLEND);
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  for (i=0; i<b.getNumPolygons(); ++i) {
    const ConvexHull3D::Polygon &p = b.getPolygon(i);
    glBegin(GL_POLYGON);
      glColor3fv(c1);
      for (j=0; j<p.getNumVertices(); ++j) {
        glVertex3fv(p.getVertex(j));
      }
    glEnd();
  }
  
  glPolygonMode(GL_FRONT, GL_FILL);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  
  glPopMatrix();
}

#ifdef NEW_SHADOW

static void drawLine(const Vector2 &v0, const Vector2 &v1, bool verbose = false) {
  Vector2 e = (v1 - v0).normalize();
  Vector2 i0, i1;
  // check intersection with left edge
  if (fabs(e.dot(Vector2::UNIT_X)) < 0.000001) {
    // vector is // to X axis
    float t0 = (-(-1) - v0.dot( Vector2::UNIT_Y)) / (e.dot( Vector2::UNIT_Y));
    float t1 = (-( 1) - v0.dot(-Vector2::UNIT_Y)) / (e.dot(-Vector2::UNIT_Y));
    i0 = v0 + t0*e;
    i1 = v0 + t1*e;
  } else {
    if (fabs(e.dot(Vector2::UNIT_Y)) < 0.000001) {
      // vector is // to Y axis
      float t0 = (1 - v0.dot( Vector2::UNIT_X)) / (e.dot( Vector2::UNIT_X));
      float t1 = (1 - v0.dot(-Vector2::UNIT_X)) / (e.dot(-Vector2::UNIT_X));
      i0 = v0 + t0*e;
      i1 = v0 + t1*e;
    } else {
      // general case, test all 4 unit square edges
      bool done0 = false;
      bool done1 = false;
      float t;
      Vector2 tmp;
      t = (1 - v0.dot( Vector2::UNIT_X)) / (e.dot( Vector2::UNIT_X));
      tmp = v0 + t*e;
      if (tmp.y >= -1 && tmp.y <= 1) {
        if (verbose) {
          cout << "Intersect right edge at: " << tmp << endl;
        }
        done0 = true;
        i0 = tmp;
      }
      t = (1 - v0.dot(-Vector2::UNIT_X)) / (e.dot(-Vector2::UNIT_X));
      tmp = v0 + t*e;
      if (tmp.y >= -1 && tmp.y <= 1) {
        if (verbose) {
          cout << "Intersect left edge at: " << tmp << endl;
        }
        if (done0) {
          done1 = true;
          i1 = tmp;
        } else {
          done0 = true;
          i0 = tmp;
        }
      }
      if (!done1) {
        t = (1 - v0.dot( Vector2::UNIT_Y)) / (e.dot( Vector2::UNIT_Y));
        tmp = v0 + t*e;
        if (tmp.x >= -1 && tmp.x <= 1) {
          if (verbose) {
            cout << "Intersect top edge at: " << tmp << " (t=" << t << ")" << endl;
          }
          if (done0) {
            done1 = true;
            i1 = tmp;
          } else {
            done0 = true;
            i0 = tmp;
          }
        }
        if (!done1) {
          t = (1 - v0.dot(-Vector2::UNIT_Y)) / (e.dot(-Vector2::UNIT_Y));
          tmp = v0 + t*e;
          if (tmp.x >= -1 && tmp.x <= 1) {
            if (verbose) {
              cout << "Intersect bottom edge at: " << i1 << " (t=" << t << ")" << endl;
            }
            i1 = tmp;
          } else {
            if (verbose) {
                cout << "Line lies outside unit square" << endl;
            }
            return;
          }
        }
      }
    }
  }
  if (verbose) {
    cout << "Line from " << i0 << " to " << i1 << endl;
  }
  glBegin(GL_LINES);
    glVertex2fv(i0);
    glVertex2fv(i1);
  glEnd();
}

static void drawPerpendicular(const Vector2 &org, const Vector2 &n, float d, bool verbose=false) {
  Vector2 pt0 = org + d*n;
  Vector2 pt1 = pt0 + Vector2(-n.y, n.x);
  drawLine(pt0, pt1, verbose);
}

float orthoRange = 2.0f;

static void drawTSM(const Shadow2::TSMData &data) {
  std::cout << "data.top = " << data.top << std::endl;
  std::cout << "data.bottom = " << data.bottom << std::endl;
  std::cout << "data.n = " << data.n << std::endl;
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glViewport(scr_hwidth, 0, scr_hwidth, scr_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-orthoRange, orthoRange, -orthoRange, orthoRange, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glPushMatrix();
    glTranslatef(0, 0, -1);
    // Draw unit square
    glColor3f(1,1,1);
    glBegin(GL_LINE_LOOP);
      glVertex2f(-1, -1);
      glVertex2f( 1, -1);
      glVertex2f( 1,  1);
      glVertex2f(-1,  1);
    glEnd();
    // drawFrustm --> draw B
    // drawHull
    glColor3f(0,0,1);
    glBegin(GL_LINE_LOOP);
      for (size_t i=0; i<data.hull.size(); ++i) {
        glVertex2fv(data.hull[i]);
      }
    glEnd();
    glColor3f(1,1,1);
    drawLine(data.nMid, data.fMid);
    // draw output
    glColor3f(1,1,1);
    drawPerpendicular(data.nMid, data.l, data.top);
    //drawPerpendicular(Vector2::ZERO, data.l, data.top);
    glColor3f(0.7f, 0.7f, 0.7f);
    drawPerpendicular(data.nMid, data.l, data.bottom);
    //drawPerpendicular(Vector2::ZERO, data.l, data.bottom);
    glColor3f(0.4f, 0.4f, 0.4f);
    drawPerpendicular(data.nMid, data.l, (data.top - data.n));
    //drawPerpendicular(Vector2::ZERO, data.l, data.n);
    glColor3f(1,1,1);
    drawLine(data.q, data.left);
    drawLine(data.q, data.right);
    /* --> Maps to Unit square
    glColor3f(0,0.7f,0);
    glPushMatrix();
      if (data.step >= 0 && data.step <= 8) {
        glMultMatrixf(data.Nt_step[data.step]);
      }
      glBegin(GL_LINE_LOOP);
        glVertex2fv(data.t0);
        glVertex2fv(data.t1);
        glVertex2fv(data.t2);
        glVertex2fv(data.t3);
      glEnd();
    glPopMatrix();
    */
    // draw points
    glEnable(GL_POINT_SMOOTH);
    glPointSize(4);
    glBegin(GL_POINTS);
      glColor3f(1,0,0);
      glVertex2fv(data.nMid);
      glColor3f(0.7f,0,0);
      glVertex2fv(data.fFoc);
      glColor3f(0.4f,0,0);
      glVertex2fv(data.fMid);
    glEnd();
    glDisable(GL_POINT_SMOOTH);
    glPointSize(1);
  glPopMatrix();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_LIGHTING);
}

#endif

static void display() {
  
  // Compute test camera [the one we will use for shadowing]
  Frustum cf(PM_PERSPECTIVE, fovy, aspect, nplane, fplane);
  cf.update();
  
  cam_mv  = Matrix4::MakeTranslate(Vector3(0, 0, -cam_dtt));
  cam_mv *= Matrix4::MakeRotate(-cam_ypr[2], Vector3::UNIT_Z);
  cam_mv *= Matrix4::MakeRotate(-cam_ypr[1], Vector3::UNIT_X);
  cam_mv *= Matrix4::MakeRotate(-cam_ypr[0], Vector3::UNIT_Y);
  cam_mv *= Matrix4::MakeTranslate(-cam_aim);
  cam_imv = cam_mv.getFastInverse();
  cam_proj = cf.getProjectionMatrix();
  
  cam_mv2  = Matrix4::MakeTranslate(Vector3(0, 0, -cam_dtt2));
  cam_mv2 *= Matrix4::MakeRotate(-cam_ypr2[2], Vector3::UNIT_Z);
  cam_mv2 *= Matrix4::MakeRotate(-cam_ypr2[1], Vector3::UNIT_X);
  cam_mv2 *= Matrix4::MakeRotate(-cam_ypr2[0], Vector3::UNIT_Y);
  cam_mv2 *= Matrix4::MakeTranslate(-cam_aim2);
  cam_imv2 = cam_mv2.getFastInverse();
  cam_proj2 = cf.getProjectionMatrix();
  
  // Compute Scene bounding box
  AABox sceneBB;
  unsigned int i;
  Vector3 off1(-10.0f, 0, -5.0f);
  Vector3 off2(2.0f, 0, 10.0f);
  Vector3 off3(10.0, 2.0f, -2.0f);
  for (i=0; i<CubeVertexCount; ++i) {
    sceneBB.merge(Vector3(CubeVertexData[i].pos));
    sceneBB.merge(Vector3(CubeVertexData[i].pos) + off1);
    sceneBB.merge(Vector3(CubeVertexData[i].pos) + off2);
    sceneBB.merge(Vector3(CubeVertexData[i].pos) + off3);
  }
  for (i=0; i<PlaneVertexCount; ++i) {
    sceneBB.merge(Vector3(PlaneVertexData[i].pos));
  }
  
  // Compute light params
  light_pos_eye = Vector3(cam_mv * light_pos);
  
  if (!useAdvShadows) {
    light_mv = Matrix4::MakeLookAt(Vector3(light_pos), light_aim, Vector3::UNIT_Y);
    light_imv = light_mv.getFastInverse();
    Frustum lf(PM_PERSPECTIVE, 120, 1, nplane, fplane);
    lf.update();
    light_proj = lf.getProjectionMatrix();
  } else {
#ifndef NEW_SHADOW
    shadow.setType(stype);
#endif
    shadow.setSceneParameters(sceneBB, sceneBB);
    shadow.setShadowParameters(100.0f, 0.1f, shadowSize, 10000.0f);
    shadow.setCameraParameters(fovy, aspect, nplane, fplane, cam_mv);
    if (ltype == LT_DIR) {
      shadow.setDirLightParameters(light_dir);
    } else if (ltype == LT_SPOT) {
      shadow.setSpotLightParameters(Vector3(light_pos), light_dir, 80, 89);
    } else {
      shadow.setPointLightParameters(Vector3(light_pos));
    }
    shadow.compute();
    light_proj = shadow.getLightProj();
    light_mv = shadow.getLightView();
    light_imv = light_mv.getFastInverse();
  }
  
  // RTT
  FBObind();
  glClearColor(1,1,1,1);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glViewport(0, 0, shadowSize, shadowSize);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMultMatrixf(light_proj);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMultMatrixf(light_mv);
  cprog->bind();
#ifdef NEW_SHADOW
  //cprog->uniform("LS")->set(shadow.getLightView()); as set in light view, do not need to pass it
  cprog->uniform("depthOffset")->set(0.0f);
#endif
  CubeMesh->render();
  glPushMatrix();
    glTranslatef(off1.x, off1.y, off1.z);
    CubeMesh->render();
  glPopMatrix();
  glPushMatrix();
    glTranslatef(off2.x, off2.y, off2.z);
    CubeMesh->render();
  glPopMatrix();
  glPushMatrix();
    glTranslatef(off3.x, off3.y, off3.z);
    CubeMesh->render();
  glPopMatrix();
  cprog->unbind();
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_LIGHTING);
  FBOunbind();
  
  // RTW
  glClearColor(0,0,0,1);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  // DRAW FIRST VIEWPORT
  //
  glViewport(0, 0, scr_hwidth, scr_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMultMatrixf(cam_proj);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMultMatrixf(cam_mv);
  // show light
  glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
  glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.8, 0.8, 0);
    glTranslatef(light_pos[0], light_pos[1], light_pos[2]);
    glutSolidSphere(1, 16, 16);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
  glPopMatrix();
  // render cube
  MESHsetMaterial(&CubeMaterial);
  glBindTexture(GL_TEXTURE_2D, 0);
  CubeMesh->render();
  glPushMatrix();
    glTranslatef(off1.x, off1.y, off1.z);
    CubeMesh->render();
  glPopMatrix();
  glPushMatrix();
    glTranslatef(off2.x, off2.y, off2.z);
    CubeMesh->render();
  glPopMatrix();
  glPushMatrix();
    glTranslatef(off3.x, off3.y, off3.z);
    CubeMesh->render();
  glPopMatrix();
  // render plane width shadows
  Matrix4 eyeToLightProj = light_proj * light_mv * cam_imv;
  MESHsetMaterial(&PlaneMaterial);
  rprog->bind();
  glBindTexture(GL_TEXTURE_2D, colorMap0);
  rprog->uniform("shadowMap")->set(GLint(0));
  rprog->uniform("depthBias")->set(0.0f); // should be on caster !!!
  rprog->uniform("shadowWidth")->set((GLint)shadowSize);
  rprog->uniform("lightPosEye")->set(light_pos_eye);
  rprog->uniform("eyeToLightProj")->set(eyeToLightProj);
#ifdef NEW_SHADOW
  rprog->uniform("LS")->set(light_mv * cam_imv);
#endif
  PlaneMesh->render();
  /*
  MESHsetMaterial(&CubeMaterial);
  glBindTexture(GL_TEXTURE_2D, 0);
  CubeMesh->render();
  glPushMatrix();
    glTranslatef(off1.x, off1.y, off1.z);
    CubeMesh->render();
  glPopMatrix();
  glPushMatrix();
    glTranslatef(off2.x, off2.y, off2.z);
    CubeMesh->render();
  glPopMatrix();
  glPushMatrix();
    glTranslatef(off3.x, off3.y, off3.z);
    CubeMesh->render();
  glPopMatrix();
  */
  rprog->unbind(); 
  
  // Draw Second viewport
  if (dbview == DV_HULLS) {
    glViewport(scr_hwidth, 0, scr_hwidth, scr_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMultMatrixf(cam_proj2);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixf(cam_mv2);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glPushMatrix();
      glDisable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D);
      glColor3f(0.8, 0.8, 0);
      glTranslatef(light_pos[0], light_pos[1], light_pos[2]);
      glutSolidSphere(1, 16, 16);
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_LIGHTING);
    glPopMatrix();
    MESHsetMaterial(&CubeMaterial);
    glBindTexture(GL_TEXTURE_2D, 0);
    CubeMesh->render();
    glPushMatrix();
      glTranslatef(off1.x, off1.y, off1.z);
      CubeMesh->render();
    glPopMatrix();
    glPushMatrix();
      glTranslatef(off2.x, off2.y, off2.z);
      CubeMesh->render();
    glPopMatrix();
    glPushMatrix();
      glTranslatef(off3.x, off3.y, off3.z);
      CubeMesh->render();
    glPopMatrix();
    MESHsetMaterial(&PlaneMaterial);
    PlaneMesh->render();
    // render hulls / frustums
#ifndef NEW_SHADOW
    if (useAdvShadows) {
      Vector3 fc = Vector3(0.8f, 0.5f, 0.2f);
      Vector3 bc = Vector3(0.2f, 0.2f, 0.7f);
      renderHull(shadow.getBodyFocus(), bc, Matrix4::IDENTITY);
      renderHull(shadow.getBodyB(), fc, Matrix4::IDENTITY);
    }
    glPushMatrix();
      glColor3f(0.7f, 0.2f, 0.2f);
      glTranslatef(shadow.getE().x, shadow.getE().y, shadow.getE().z);
      glutSolidSphere(0.1, 16, 16);
    glPopMatrix();
#else
    // new shadow draw stuff
    Vector3 fc = Vector3(0.8f, 0.5f, 0.2f);
    Vector3 bc = Vector3(0.2f, 0.2f, 0.7f);
    ConvexHull3D ch;
    shadow.calcB(NULL, &ch);
    renderHull(ch, bc, Matrix4::IDENTITY);
    shadow.calcLVS(NULL, &ch);
    renderHull(ch, fc, Matrix4::IDENTITY);
    glPushMatrix();
      glColor3f(0.7f, 0.2f, 0.2f);
      glTranslatef(shadow.getE().x, shadow.getE().y, shadow.getE().z);
      glutSolidSphere(0.1, 16, 16);
    glPopMatrix();
#endif
    
  } else if (dbview == DV_PPS) {
    glViewport(scr_hwidth, 0, scr_hwidth, scr_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
      glMultMatrixf(light_proj);
      glMultMatrixf(light_mv);
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST); // in PPS, z is inversed !!!
      glDisable(GL_TEXTURE_2D);
      glColor3fv(PlaneMaterial.diff);
      PlaneMesh->render();
      glPushMatrix();
        glColor3f(0.7f, 0.2f, 0.2f);
        glTranslatef(cam_imv(0,3), cam_imv(1,3), cam_imv(2,3));
        glutSolidSphere(0.1, 16, 16);
      glPopMatrix();
      glColor3fv(CubeMaterial.diff);
      CubeMesh->render();
      glPushMatrix();
        glTranslatef(off1.x, off1.y, off1.z);
        CubeMesh->render();
      glPopMatrix();
      glPushMatrix();
        glTranslatef(off2.x, off2.y, off2.z);
        CubeMesh->render();
      glPopMatrix();
      glPushMatrix();
        glTranslatef(off3.x, off3.y, off3.z);
        CubeMesh->render();
      glPopMatrix();
#ifndef NEW_SHADOW
      Vector3 fc = Vector3(0.8f, 0.5f, 0.2f);
      Vector3 bc = Vector3(0.2f, 0.2f, 0.7f);
      renderHull(shadow.getBodyLVS(), fc, Matrix4::IDENTITY);
      renderHull(shadow.getBodyB(), bc, Matrix4::IDENTITY);
#else
      // new shadow stuff
      Vector3 fc = Vector3(0.8f, 0.5f, 0.2f);
      Vector3 bc = Vector3(0.2f, 0.2f, 0.7f);
      ConvexHull3D ch;
      shadow.calcB(NULL, &ch);
      renderHull(ch, bc, Matrix4::IDENTITY);
#endif
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_LIGHTING);
    glPopMatrix();
  }
#ifdef NEW_SHADOW
  else if (dbview == DV_NT) {
    std::cout << "Debug TSM" << std::endl;
    drawTSM(shadow.getTSMData());
  }
#endif
  
  /* RENDER QUAD
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_LIGHTING);
  glActiveTextureARB(GL_TEXTURE0_ARB);
  glBindTexture(GL_TEXTURE_2D, colorMap0);
  float top  = 10 * tan(DegToRad * 0.5 * fovy);
  float left = top * aspect;
  glBegin(GL_QUADS);
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);
    glVertex3f(-left, -top, -10);
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 0);
    glVertex3f(left, -top, -10);
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 1);
    glVertex3f(left, top, -10);
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 1);
    glVertex3f(-left, top, -10);
  glEnd();
  glEnable(GL_LIGHTING);
  glPopMatrix();
  */
  
  glutSwapBuffers();
}

static void reshape(int w, int h) {
  scr_width = w;
  scr_hwidth = w >> 1; // * 0.5
  scr_height = h;
  aspect = float(scr_hwidth) / float(scr_height);
}

static void keyboard(unsigned char key, int, int) {
  switch (key) {
    case 27:
      exit(0);
    //case 'p':
    //  useAdvShadows = !useAdvShadows;
    //  cout << (useAdvShadows ? "Advanced shadow mode" : "Simple shadow mode") << endl;
    // glutPostRedisplay();
    //  break;
#ifdef NEW_SHADOW
    case '+':    
      if (useAdvShadows && advShadowType == TSM) {
        shadow.setTSMStep(shadow.getTSMStep()+1);
        glutPostRedisplay();
      }
      break;
    case '-':
      if (useAdvShadows && advShadowType == TSM) {
        shadow.setTSMStep(shadow.getTSMStep()-1);
        glutPostRedisplay();
      }
      break;
    case 'a':
      if (useAdvShadows && advShadowType == TSM) {
        shadow.tryTSMCorrection(!shadow.tryTSMCorrection());
        glutPostRedisplay();
      }
      break;
    case 'z':
      orthoRange *= 0.5f;
      if (orthoRange <= 0.0f) {
        orthoRange = 1.0f;
      }
      glutPostRedisplay();
      break;
    case 'Z':
      orthoRange *= 2.0f;
      glutPostRedisplay();
      break;
#endif
    case 's':
#ifndef NEW_SHADOW
      if (!useAdvShadows) {
        useAdvShadows = true;
        stype = Shadow::ST_CUSTOM;
        cout << "Shadow algorithm: Custom" << endl;
      } else {
        if (stype == Shadow::ST_CUSTOM) {
          stype = Shadow::ST_UNIFORM;
          cout << "Shadow algorithm: Uniform" << endl;
        } else if (stype == Shadow::ST_UNIFORM) {
          stype = Shadow::ST_LiSPSM;
          cout << "Shadow algorithm: LiSPSM" << endl;
        } else if (stype == Shadow::ST_LiSPSM) {
          stype = Shadow::ST_UNIFORM2;
          cout << "Shadow algorithm: Uniform v2" << endl;
        } else if (stype == Shadow::ST_UNIFORM2) {
          stype = Shadow::ST_LiSPSM2;
          cout << "Shadow algorithm: LiSPSM v2" << endl;
        } else if (stype == Shadow::ST_LiSPSM2) {
          useAdvShadows = false;
          cout << "Shadow algorithm: Basic" << endl;
        }
      }
#else
      if (useAdvShadows) {
        if (advShadowType == FOCUSED) {
          advShadowType = LiSPSM;
          shadow.useLiSPSM(true);
          shadow.useTSM(false);
          cout << "Shadow algorithm: LiSPSM" << endl;
        } else if (advShadowType == LiSPSM) {
          advShadowType = TSM;
          shadow.useLiSPSM(false);
          shadow.useTSM(true);
          cout << "Shadow algorithm: TSM" << endl;
        } else {
          useAdvShadows = false;
          cout << "Shadow algorithm: Default" << endl;
        }
      } else {
        useAdvShadows = true;
        advShadowType = FOCUSED;
        shadow.useLiSPSM(false);
        shadow.useTSM(false);
        cout << "Shadow algorithm: Focused" << endl;
      }
      if (useAdvShadows && advShadowType == TSM) {
        cprog->detach(cvert);
        cprog->detach(cfrag);
        cprog->attach(cvert_tsm);
        cprog->attach(cfrag_tsm);
        cprog->link();
        
        rprog->detach(rvert);
        rprog->detach(rfrag);
        rprog->attach(rvert_tsm);
        rprog->attach(rfrag_tsm);
        rprog->link();
        
      } else {
        if (useAdvShadows == false) {
          // just leave TSM mode
          cprog->detach(cvert_tsm);
          cprog->detach(cfrag_tsm);
          cprog->attach(cvert);
          cprog->attach(cfrag);
          cprog->link();
          
          rprog->detach(rvert_tsm);
          rprog->detach(rfrag_tsm);
          rprog->attach(rvert);
          rprog->attach(rfrag);
          rprog->link();
          
        }
      }
      //if (!useAdvShadows) {
      //  useAdvShadows = true;
      //  cout << "Shadow algorithm: Custom" << endl;
      //} else {
      //  useAdvShadows = false;
      //  cout << "Shadow algorithm: Basic" << endl;
      //}
#endif
      glutPostRedisplay();
      break;
    case 'd':
      if (dbview == DV_HULLS) {
        dbview = DV_PPS;
      } else if (dbview == DV_PPS) {
#ifdef NEW_SHADOW
        dbview = DV_NT;
      } else if (dbview == DV_NT) {
#endif
        dbview = DV_HULLS;
      }
      
      glutPostRedisplay();
      break;
    case 'l':
      switch (ltype) {
        case LT_DIR:
          ltype = LT_POINT;
          cout << "POINT LIGHT" << endl;
          break;
        case LT_POINT:
          ltype = LT_SPOT;
          cout << "SPOT LIGHT" << endl;
          break;
        case LT_SPOT:
          ltype = LT_DIR;
          cout << "DIR LIGHT" << endl;
        default:
          break;
      }
      glutPostRedisplay();
      break;
    case 'f':
      rprog->detach(rfrag);
      if (rfrag == rfrag_no) {
        rfrag = rfrag_2x2;
        cout << "PCF 2x2" << endl;
      } else if (rfrag == rfrag_2x2) {
        rfrag = rfrag_4x4;
        cout << "PCF 4x4" << endl;
      } else if (rfrag == rfrag_4x4) {
        rfrag = rfrag_disc;
        cout << "PCF: Poisson disc 12 samples" << endl;
      } else if (rfrag == rfrag_disc) {
        rfrag = rfrag_no;
        cout << "No PCF" << endl;
      }
      rprog->attach(rfrag);
      if (!rprog->link()) {
        cout << "Error changing fragment program: " << rprog->log() << endl;
      }
      glutPostRedisplay();
      break;
    default:
      return;
  }
}

static void mouseclick(int button, int state, int x, int y) {
  mouse_btn = button;
  mouse_state = state;
  key_mod = glutGetModifiers();
  cur_x = x;
  cur_y = y;
  last_x = x;
  last_y = y;
  if (mouse_state == GLUT_DOWN) {
    if (x > scr_hwidth) {
      cur_cam = 1;
    } else {
      cur_cam = 0;
    }
    if (key_mod & GLUT_ACTIVE_ALT) {      
      switch (mouse_btn) {
        case GLUT_LEFT_BUTTON:
          mouse_op = MO_CAM_ROTATE;
          break;
        case GLUT_MIDDLE_BUTTON:
          mouse_op = MO_CAM_TRANSLATE;
          break;
        case GLUT_RIGHT_BUTTON:
          mouse_op = MO_CAM_DOLLY;
          break;
        default:
          mouse_op = MO_NONE;
      }
    } else {
      //if (cur_cam == 1 && button == GLUT_LEFT_BUTTON) {
      //  x -= src_hwidth;
      if (button == GLUT_LEFT_BUTTON) {
        if (cur_cam == 1) {
          x -= scr_hwidth;
        }
        float sy = nplane * Tand(0.5f * fovy);
        float sx = aspect * sy;
        float nx = ((2.0f * x) / float(scr_hwidth)) - 1.0f;
        float ny = ((2.0f * (scr_height - y)) / float(scr_height)) - 1.0f;
        Vector3 rorg = Vector3(0,0,0);
        Vector3 rdir = (Vector3(sx*nx, sy*ny, -nplane) - rorg).normalize();
        //Vector3 lpos = Vector3(cam_mv2 * light_pos);
        Vector3 lpos = Vector3(*(cams[cur_cam].mv) * light_pos);
        Plane pl(Vector3::UNIT_Z, lpos.z);
        Ray ray(rorg, rdir);
        float t;
        ray.intersect(pl, &t);
        Vector3 tmp = rorg - (t * rdir);
        float d = (tmp - lpos).getLength();
        if (d <= 1.0f) {
          mouse_op = MO_LIGHT_TRANSLATE;
        }
      }
    }
  } else {
    mouse_op = MO_NONE;
  }
}

static void mousemove(int x, int y) {
  last_x = cur_x;
  last_y = cur_y;
  cur_x = x;
  cur_y = y;
}

static void mousedrag(int x, int y) {
  last_x = cur_x;
  last_y = cur_y;
  cur_x = x;
  cur_y = y;
  float dx = (float)(cur_x - last_x);
  float dy = (float)(last_y - cur_y);
  if (mouse_op == MO_CAM_ROTATE) {
    cams[cur_cam].ypr->y += 0.25f * dy;
    cams[cur_cam].ypr->x -= 0.25f * dx;
    glutPostRedisplay();
  }
  else if (mouse_op == MO_CAM_TRANSLATE) {
    float ny = *(cams[cur_cam].dtt) * Tand(0.5f * fovy);
    float piy = ny / (0.5f * (float)win_h);
    float pix = piy * aspect;
    dx *= -pix;
    dy *= -piy;
    Vector3 x(cams[cur_cam].imv->getColumn(0));
    Vector3 y(cams[cur_cam].imv->getColumn(1));
    x *= dx;
    y *= dy;
    *(cams[cur_cam].aim) += (x + y);
    glutPostRedisplay();
  }
  else if (mouse_op == MO_CAM_DOLLY) {
    float dolly = (Abs(dx) > Abs(dy) ? dx : dy);
    if (*(cams[cur_cam].dtt) > 0.000001) {
      dolly *= -1 * *(cams[cur_cam].dtt) / 500;
    } else {
      dolly *= -0.001;
    }
    *(cams[cur_cam].dtt) += dolly;
    if (*(cams[cur_cam].dtt) < 0) {
      *(cams[cur_cam].dtt) = 0;
    }
    glutPostRedisplay();
  }
  else if (mouse_op == MO_LIGHT_TRANSLATE) {
    //Vector3 lpos = Vector3(cam_mv2 * light_pos);
    Vector3 lpos = Vector3(*(cams[cur_cam].mv) * light_pos);
    float sy = -lpos.z * Tand(0.5f * fovy);
    float sx = aspect * sy;
    
    float nx0 = ((2.0f * (last_x - scr_hwidth)) / float(scr_hwidth)) - 1.0f;
    float ny0 = ((2.0f * (scr_height - last_y)) / float(scr_height)) - 1.0f;
    
    float nx1 = ((2.0f * (cur_x - scr_hwidth)) / float(scr_hwidth)) - 1.0f;
    float ny1 = ((2.0f * (scr_height - cur_y)) / float(scr_height)) - 1.0f;
    
    float dx = sx * (nx1 - nx0);
    float dy = sy * (ny1 - ny0);
    
    //Vector3 x(cam_imv2.getColumn(0));
    //Vector3 y(cam_imv2.getColumn(1));
    Vector3 x(cams[cur_cam].imv->getColumn(0));
    Vector3 y(cams[cur_cam].imv->getColumn(1));
    
    light_pos += Vector4(((dx * x) + (dy * y)), 0);
    light_dir = (light_aim - Vector3(light_pos)).normalize();
    
    glutPostRedisplay();
  }
}

static void on_exit() {
  cleanup();
}

// ---

int main(int argc, char **argv) {
  atexit(on_exit);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE);
  glutInitWindowPosition(50,50);
  glutInitWindowSize(1024,512);
  glutCreateWindow("ShadowMaps");
  init();
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouseclick);
  glutMotionFunc(mousedrag);
  glutPassiveMotionFunc(mousemove);
  glutMainLoop();
  return 0;
}

/* 

 NOTES:
  - auto-adjust bias for self shadowing
  - PCF -> PCSS (percentage-closer soft shadows: smartly varying PCF kernel size [penumbra])
  - Blur pass
  - Light space perspective 
  - Use an edge map + dynamic branching to PCF only on edge regions

*/



